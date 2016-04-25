/*
 *
 * Copyright (c) 2016 Project Raphine
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
 * Author: Yuchiki
 * 
 */


/*this shell-like program is substituted later*/
#include<global.h>
#include<tty.h>
#include<string.h>
#include<shell.h>

void Shell::Setup() {
  _liner.Setup(this);
  gtty->Printf("s", ">");
}

void Shell::Register(const char *name, void (*func)(void)) {
  if (_next_buf == kNameSize){
    //error
  }else{
    int slen = strlen(name);
    if (slen < kNameSize){
      strncpy(_name_func_mapping[_next_buf].name, name, slen);
      _name_func_mapping[_next_buf].func = func;
      _next_buf++;
    }
  }
}

void Shell::Exec(const char *name) {
  for(int i = 0; i < kBufSize; i++){
    if (strncmp(name, _name_func_mapping[i].name ,strlen(name)) == 0){
      _name_func_mapping[i].func();
      return;
    }
  }

  gtty->Printf("s","unknown command:", "s", name, "s", "\n");
}

void Shell::ReadCh(char c) {
  _liner.ReadCh(c);
}

void Shell::Liner::ReadCh(char c) {
  if(c == '\n'){
    if(strlen(_command) > 0){
      _shell->Exec(_command);
      _command[0] = '\0';
      _next_command = 0;
    }
    gtty->Printf("s", ">");
  }else if(_next_command != kCommandSize - 1){
    _command[_next_command] = c;
    _next_command++;
    _command[_next_command] = '\0';
  }
}
