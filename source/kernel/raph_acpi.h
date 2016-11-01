/*
 *
 * Copyright (c) 2015 Raphine Project
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

#ifndef __RAPH_KERNEL_ACPI_H__
#define __RAPH_KERNEL_ACPI_H__

#include <stdint.h>
#include <string.h>
#include <task.h>
#include <global.h>
#include <mem/physmem.h>
#include <_raph_acpi.h>

struct MCFG;
struct HPETDT;

class DevPci;

class AcpiCtrl {
public:
  AcpiCtrl() {}
  void Setup();
  MCFG *GetMCFG();
  HPETDT *GetHPETDT();
  FADT *GetFADT();
  void SetupAcpica();
  void Reset();
  void Shutdown();
private:
  friend class AcpicaPciCtrl;
  int GetPciIntNum(DevPci *device);
  void GlobalEventHandler(void *);
  Task _global_event_task;
};

#endif /* __RAPH_KERNEL_ACPI_H__ */
