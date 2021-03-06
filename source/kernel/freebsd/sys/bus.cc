/*
 *
 * Copyright (c) 2016 Raphine Project
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

#include <mem/physmem.h>
#include <sys/kernel.h>
#include <sys/bus-raph.h>
#include <sys/types-raph.h>
#include <sys/rman.h>
#include <sys/errno.h>
#include <tty.h>
#include <global.h>

extern "C" {

  uint8_t bus_space_read_1(bus_space_tag_t space, bus_space_handle_t handle, bus_size_t offset) {
    switch(space) {
    case BUS_SPACE_MEMIO:
      return reinterpret_cast<volatile uint8_t *>(handle)[offset / sizeof(uint8_t)];
    case BUS_SPACE_PIO:
      return inb(handle + offset);
    default:
      kassert(false);
    }
  }

  uint16_t bus_space_read_2(bus_space_tag_t space, bus_space_handle_t handle, bus_size_t offset) {
    switch(space) {
    case BUS_SPACE_MEMIO:
      return reinterpret_cast<volatile uint16_t *>(handle)[offset / sizeof(uint16_t)];
    case BUS_SPACE_PIO:
      return inw(handle + offset);
    default:
      kassert(false);
    }
  }

  uint32_t bus_space_read_4(bus_space_tag_t space, bus_space_handle_t handle, bus_size_t offset) {
    switch(space) {
    case BUS_SPACE_MEMIO:
      return reinterpret_cast<volatile uint32_t *>(handle)[offset / sizeof(uint32_t)];
    case BUS_SPACE_PIO:
      return inl(handle + offset);
    default:
      kassert(false);
    }
  }

  void bus_space_write_1(bus_space_tag_t space, bus_space_handle_t handle, bus_size_t offset, uint8_t value) {
    switch(space) {
    case BUS_SPACE_MEMIO:
      reinterpret_cast<volatile uint8_t *>(handle)[offset / sizeof(uint8_t)] = value;
      return;
    case BUS_SPACE_PIO:
      outb(handle + offset, value);
      return;
    default:
      kassert(false);
    }
  }

  void bus_space_write_2(bus_space_tag_t space, bus_space_handle_t handle, bus_size_t offset, uint16_t value) {
    switch(space) {
    case BUS_SPACE_MEMIO:
      reinterpret_cast<volatile uint16_t *>(handle)[offset / sizeof(uint16_t)] = value;
      return;
    case BUS_SPACE_PIO:
      outw(handle + offset, value);
      return;
    default:
      kassert(false);
    }
  }

  void bus_space_write_4(bus_space_tag_t space, bus_space_handle_t handle, bus_size_t offset, uint32_t value) {
    switch(space) {
    case BUS_SPACE_MEMIO:
      reinterpret_cast<volatile uint32_t *>(handle)[offset / sizeof(uint32_t)] = value;
      return;
    case BUS_SPACE_PIO:
      outl(handle + offset, value);
      return;
    default:
      kassert(false);
    }
  }

  int bus_generic_attach(device_t dev) {
    dev->ProbeAndAttachChildren();
    return 0;
  }

  struct resource *bus_alloc_resource_any(device_t dev, int type, int *rid, u_int flags) {
    return dev->GetBusClass()->DevMethodBusAllocResource(type, rid, 0, ~0, 1, flags);
  }

  int bus_release_resource(device_t dev, int type, int rid, struct resource *r) {
    return dev->GetBusClass()->DevMethodBusReleaseResource(type, rid, r);
  }
  
  int bus_setup_intr(device_t dev, struct resource *r, int flags, driver_filter_t filter, driver_intr_t ithread, void *arg, void **cookiep) {
    return dev->GetBusClass()->DevMethodBusSetupIntr(r, flags, filter, ithread, arg, cookiep);
  }

  int     bus_teardown_intr(device_t dev, struct resource *r, void *cookie) {
    // TODO impl
    return -1;
  }

  int     bus_bind_intr(device_t dev, struct resource *r, int cpu) {
    // TODO impl
    return -1;
  }


  int device_printf(device_t dev, const char *fmt, ...) {
    gtty->Cprintf("[pci device]:");
    va_list args;
    va_start(args, fmt);
    gtty->Cvprintf(fmt, args);
    va_end(args);
    gtty->Cprintf("\n");
    return 0;  // TODO fix this
  }

  void device_set_desc_copy(device_t dev, const char* desc) {
  }

  void *device_get_softc(device_t dev) {
    return dev->softc;
  }
  
  device_t	device_get_parent(device_t dev) {
    return dev->GetParent();
  }
  
  int	resource_int_value(const char *name, int unit, const char *resname,
                         int *result) {
    return ENOENT;
  }

  void *device_get_ivars(device_t dev) {
    return dev->ivar;
  }

  void device_set_ivars(device_t dev, void *ivar) {
    dev->ivar = ivar;
  }

  int	device_get_unit(device_t dev) {
    return dev->unit;
  }
  
  const	char *device_get_name(device_t dev) {
    return dev->name;
  }

}
