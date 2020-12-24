#ifndef LUSE_POSIX_STRUCTS_H
#define LUSE_POSIX_STRUCTS_H

#define _GNU_SOURCE 1

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/statvfs.h>
#include <fcntl.h>
#include <unistd.h>
#include <dirent.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <lua.h>

void lua__push__stat_mode(lua_State* L, mode_t m);
mode_t lua__to__stat_mode(lua_State* L, int index);
void lua__push__stat(lua_State* L, struct stat* st);
void lua__push__open_flags(lua_State* L, int f);
int lua__to__open_flags(lua_State* L, int index);
void lua__push__dirent(lua_State* L, struct dirent* ent);
void lua__push__statvfs_flag(lua_State* L, int flags);
int lua__to__statvfs_flag(lua_State* L, int index);
void lua__push__statvfs(lua_State* L, struct statvfs* st);
void lua__push__timeval(lua_State* L, struct timeval* time);
void lua__push__timeval_array(lua_State* L, struct timeval* times, size_t size);
void lua__push__timespec(lua_State* L, const struct timespec* tv);
void lua__push__timespec_array(lua_State* L, const struct timespec* tv);

#endif

/*
Copyright (c) Jérôme Vuarand

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.
*/

// vi: ts=4 sts=4 sw=4
