/*
 *
 * Copyright (c) 2015 Project Raphine
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 *
 * Author: Liva
 * 
 */

#include <global.h>
#include <spinlock.h>
#include <acpi.h>
#include <apic.h>
#include <multiboot.h>
#include <task.h>
#include <mem/physmem.h>
#include <mem/paging.h>
#include <gdt.h>
#include <idt.h>
#include <timer.h>
#include <tty.h>

#include <dev/hpet.h>
#include <dev/vga.h>
#include <dev/pci.h>
#include <dev/keyboard.h>

#include "net/netctrl.h"
#include "net/socket.h"


SpinLockCtrl *spinlock_ctrl;
MultibootCtrl *multiboot_ctrl;
AcpiCtrl *acpi_ctrl;
ApicCtrl *apic_ctrl;
PhysmemCtrl *physmem_ctrl;
PagingCtrl *paging_ctrl;
VirtmemCtrl *virtmem_ctrl;
TaskCtrl *task_ctrl;
Gdt *gdt;
Idt *idt;
Timer *timer;

Tty *gtty;
Keyboard *keyboard;

PCICtrl *pci_ctrl;

static uint32_t rnd_next = 1;

#include <dev/e1000/bem.h>
bE1000 *eth;
uint64_t cnt;
int time;

#include <callout.h>
Callout tt;

extern "C" int main() {
  SpinLockCtrl _spinlock_ctrl;
  spinlock_ctrl = &_spinlock_ctrl;
  
  MultibootCtrl _multiboot_ctrl;
  multiboot_ctrl = &_multiboot_ctrl;

  AcpiCtrl _acpi_ctrl;
  acpi_ctrl = &_acpi_ctrl;

  ApicCtrl _apic_ctrl;
  apic_ctrl = &_apic_ctrl;

  Gdt _gdt;
  gdt = &_gdt;
  
  Idt _idt;
  idt = &_idt;

  VirtmemCtrl _virtmem_ctrl;
  virtmem_ctrl = &_virtmem_ctrl;
  
  PhysmemCtrl _physmem_ctrl;
  physmem_ctrl = &_physmem_ctrl;
  
  PagingCtrl _paging_ctrl;
  paging_ctrl = &_paging_ctrl;

  TaskCtrl _task_ctrl;
  task_ctrl = &_task_ctrl;
  
  Hpet _htimer;
  timer = &_htimer;

  Vga _vga;
  gtty = &_vga;

  Keyboard _keyboard;
  keyboard = &_keyboard;
  
  PhysAddr paddr;
  physmem_ctrl->Alloc(paddr, PagingCtrl::kPageSize * 2);
  extern int kKernelEndAddr;
  kassert(paging_ctrl->MapPhysAddrToVirtAddr(reinterpret_cast<virt_addr>(&kKernelEndAddr) - PagingCtrl::kPageSize * 4, paddr, PagingCtrl::kPageSize * 2, PDE_WRITE_BIT, PTE_WRITE_BIT | PTE_GLOBAL_BIT));

  multiboot_ctrl->Setup();
  
  // acpi_ctl->Setup() は multiboot_ctrl->Setup()から呼ばれる

  if (timer->Setup()) {
    gtty->Printf("s","[timer] info: HPET supported.\n");
  } else {
    kernel_panic("timer", "HPET not supported.\n");
  }


  // timer->Sertup()より後
  apic_ctrl->Setup();

  rnd_next = timer->ReadMainCnt();

  // apic_ctrl->Setup()より後
  task_ctrl->Setup();

  idt->SetupGeneric();
  
  apic_ctrl->BootBSP();

  gdt->SetupProc();

  idt->SetupProc();

  InitNetCtrl();

  InitDevices<PCICtrl, Device>();


  extern int kKernelEndAddr;
  // stackは16K
  kassert(paging_ctrl->IsVirtAddrMapped(reinterpret_cast<virt_addr>(&kKernelEndAddr)));
  kassert(paging_ctrl->IsVirtAddrMapped(reinterpret_cast<virt_addr>(&kKernelEndAddr) - (4096 * 1) + 1));
  kassert(paging_ctrl->IsVirtAddrMapped(reinterpret_cast<virt_addr>(&kKernelEndAddr) - (4096 * 2) + 1));
  kassert(paging_ctrl->IsVirtAddrMapped(reinterpret_cast<virt_addr>(&kKernelEndAddr) - (4096 * 3) + 1));
  kassert(paging_ctrl->IsVirtAddrMapped(reinterpret_cast<virt_addr>(&kKernelEndAddr) - (4096 * 4) + 1));
  kassert(paging_ctrl->IsVirtAddrMapped(reinterpret_cast<virt_addr>(&kKernelEndAddr) - (4096 * 5) + 1));
  kassert(!paging_ctrl->IsVirtAddrMapped(reinterpret_cast<virt_addr>(&kKernelEndAddr) - 4096 * 6));

  cnt = 0;

  gtty->Printf("s", "[cpu] info: #", "d", apic_ctrl->GetApicId(), "s", " started.\n");
  apic_ctrl->SetupTimer(32 + 10);

  apic_ctrl->StartAPs();

  gtty->Printf("s", "\n\n[kernel] info: initialization completed\n");

  // print keyboard_input
  PollingFunc _keyboard_polling;
  keyboard->Setup(0); //should we define kDefaultLapicid = 0 ?
  _keyboard_polling.Init([](void *) {
    while(keyboard->Count() > 0) {
      char ch[2] = {'\0','\0'};
      ch[0] = keyboard->GetCh();
      gtty->Printf("s", ch);
    }
  }, nullptr);
  _keyboard_polling.Register();
  
  while(true) {
    task_ctrl->Run();
    asm volatile("hlt");
  }

  DismissNetCtrl();

  return 0;
}

#define FLAG 0
#if FLAG == 3
#define IP1 192, 168, 100, 117
#define IP2 192, 168, 100, 254
#elif FLAG == 2
#define IP1 192, 168, 100, 117
#define IP2 192, 168, 100, 104
#elif FLAG == 1
#define IP1 192, 168, 100, 104
#define IP2 192, 168, 100, 117
#elif FLAG == 0
#define IP1 10, 0, 2, 5
#define IP2 10, 0, 2, 15
#endif

uint8_t ip1[] = {IP1};
uint8_t ip2[] = {IP2};

extern "C" int main_of_others() {
// according to mp spec B.3, system should switch over to Symmetric I/O mode
  apic_ctrl->BootAP();

  gdt->SetupProc();
  idt->SetupProc();

  gtty->Printf("s", "[cpu] info: #", "d", apic_ctrl->GetApicId(), "s", " started.\n");

  apic_ctrl->SetupTimer(32 + 10);
  // ループ性能測定用
  // if (apic_ctrl->GetApicId() == 3) {
  //   PollingFunc p;
  //   static int hoge = 0;
  //   p.Init([](void *){
  //       int hoge2 = timer->GetUsecFromCnt(timer->ReadMainCnt()) - hoge;
  //       gtty->Printf("d",hoge2,"s"," ");
  //       hoge = timer->GetUsecFromCnt(timer->ReadMainCnt());
  //     }, nullptr);
  //   p.Register();
  // }

  
  if (apic_ctrl->GetApicId() == 1) {
    static ARPSocket socket;
    if(socket.Open() < 0) {
      gtty->Printf("s", "[error] failed to open socket\n");
    }
    socket.SetIPAddr(inet_atoi(ip1));

    PollingFunc p;
    p.Init([](void *){
      uint32_t ipaddr;
      uint8_t macaddr[6];
      int32_t rval = socket.ReceivePacket(0, &ipaddr, macaddr);
      if(rval == ARPSocket::kOpARPReply) {
        uint64_t l = ((uint64_t)(timer->ReadMainCnt() - cnt) * (uint64_t)timer->GetCntClkPeriod()) / 1000;
        cnt = 0;
        gtty->Printf(
                     "s", "[arp] reply received; ",
                     "x", macaddr[0], "s", ":",
                     "x", macaddr[1], "s", ":",
                     "x", macaddr[2], "s", ":",
                     "x", macaddr[3], "s", ":",
                     "x", macaddr[4], "s", ":",
                     "x", macaddr[5], "s", " is ",
                     "d", (ipaddr >> 24) & 0xff, "s", ".",
                     "d", (ipaddr >> 16) & 0xff, "s", ".",
                     "d", (ipaddr >> 8) & 0xff, "s", ".",
                     "d", (ipaddr >> 0) & 0xff, "s", " (");
        gtty->Printf("s", "latency:", "d", l, "s", "us)\n");
      } else if(rval == ARPSocket::kOpARPRequest) {
        gtty->Printf(
                     "s", "[arp] request received; ",
                     "x", macaddr[0], "s", ":",
                     "x", macaddr[1], "s", ":",
                     "x", macaddr[2], "s", ":",
                     "x", macaddr[3], "s", ":",
                     "x", macaddr[4], "s", ":",
                     "x", macaddr[5], "s", " is ",
                     "d", (ipaddr >> 24) & 0xff, "s", ".",
                     "d", (ipaddr >> 16) & 0xff, "s", ".",
                     "d", (ipaddr >> 8) & 0xff, "s", ".",
                     "d", (ipaddr >> 0) & 0xff, "s", "\n");

        if(socket.TransmitPacket(ARPSocket::kOpARPReply, ipaddr, macaddr) < 0) {
	        gtty->Printf("s", "[arp] failed to transmit reply\n");
	      } else {
	        gtty->Printf("s", "[arp] reply sent\n");
	      }
      }
    }, nullptr);
    p.Register();
  } else if (apic_ctrl->GetApicId() == 2) {
    cnt = 0;
    new(&tt) Callout;
    time = 10;
    tt.Init([](void *){
        if (!apic_ctrl->IsBootupAll()) {
          tt.SetHandler(1000);
          return;
        }
        apic_ctrl->SendIpi(3);
        kassert(eth != nullptr);
        eth->UpdateLinkStatus();
        if (eth->GetStatus() != bE1000::LinkStatus::Up) {
          tt.SetHandler(1000);
          return;
        }
        if (cnt != 0) {
          tt.SetHandler(10);
          return;
        }

        ARPSocket socket;
        if(socket.Open() < 0) {
          gtty->Printf("s", "[error] failed to open socket\n");
        } else {
          socket.SetIPAddr(inet_atoi(ip1));
          cnt = timer->ReadMainCnt();
          if(socket.TransmitPacket(ARPSocket::kOpARPRequest, inet_atoi(ip2), nullptr) < 0) {
    	    gtty->Printf("s", "[arp] failed to transmit request\n");
    	  } else {
    	    gtty->Printf("s", "[arp] request sent\n");
    	  }
        }

        time--;
        if (time != 0) {
          tt.SetHandler(3000);
        }
      }, nullptr);
    tt.SetHandler(10);
  }
  while(true) {
    task_ctrl->Run();
    asm volatile("hlt");
  }
  return 0;
}

void kernel_panic(const char *class_name, const char *err_str) {
  gtty->PrintfRaw("s", "\n[","s",class_name,"s","] error: ","s",err_str);
  while(true) {
    asm volatile("hlt;");
  }
}

extern "C" void __cxa_pure_virtual() {
  kernel_panic("", "");
}

extern "C" void __stack_chk_fail() {
  kernel_panic("", "");
}

#define RAND_MAX 0x7fff

uint32_t rand() {
  rnd_next = rnd_next * 1103515245 + 12345;
  /* return (unsigned int)(rnd_next / 65536) % 32768;*/
  return (uint32_t)(rnd_next >> 16) & RAND_MAX;
}
