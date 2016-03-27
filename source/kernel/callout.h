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

#ifndef __RAPH_KERNEL_CALLOUT_H__
#define __RAPH_KERNEL_CALLOUT_H__

#include "callout.h"
#include "polling.h"
#include "timer.h"
#include "global.h"
#include "task.h"


class Callout : public Function, Polling {
 public:
  Callout() {
  }
  void SetHandler(uint32_t us) {
    if (this->CanExecute()) {
      _cnt = timer->GetCntAfterPeriod(timer->ReadMainCnt(), us);
      this->RegisterPolling();
    }
  }
  volatile bool IsHandling() {
    return (_status == 1);
  }
  virtual void Handle() override {
    if (timer->IsTimePassed(_cnt)) {
      this->Cancel();
      _status = 1;
      this->Execute();
      _status = 0;
    }
  }
  // calloutを登録したprocessorで実行する事
  void Cancel() {
    this->RemovePolling();
    this->Clear();
  }
 private:
  volatile int _status = 0;
  uint64_t _cnt;
};

#include "spinlock.h"

class LckCallout : public Callout {
 public:
  void SetLock(SpinLock *lock) {
    _lock = lock;
  }
  virtual void Handle() override {
    if (_lock != nullptr) {
      _lock->Lock();
    }
    Callout::Handle();
    if (_lock != nullptr) {
      _lock->Unlock();
    }
  }
 private:
  SpinLock *_lock = nullptr;
};

#endif // __RAPH_KERNEL_CALLOUT_H__