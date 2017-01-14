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

#pragma once

#include <raph.h>
#include <spinlock.h>

template<class T>
class ObjectList {
public:
  struct Container {
    template<class... Arg>
    Container(Arg... args) : obj(args...) {
      next = nullptr;
    }
    Container *GetNext() {
      return next;
    }
    T *GetObject() {
      return &obj;
    }
  private:
    T obj;
    Container *next;
    friend ObjectList;
  };
  ObjectList() {
    _last = &_first;
    _first.obj = nullptr;
    _first.next = nullptr;
  }
  ~ObjectList() {
  }
  template<class... Arg>
  Container *PushBack(Arg... args) {
    Container *c = new Container(args...);
    Locker locker(_lock);
    kassert(_last->next == nullptr);
    _last->next = c;
    _last = c;
    return c;
  }
  // 帰るコンテナは番兵なので、オブジェクトを格納していない
  Container *GetBegin() {
    return &_first;
  }
  bool IsEmpty() {
    return &_first == _last;
  }
private:
  Container _first;
  Container *_last;
  SpinLock _lock;
};

