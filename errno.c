#include <errno.h>
#include <string.h>
#include <lua.h>
#include <lauxlib.h>
#include <stdlib.h>
#include <sys/xattr.h>
#include "compat.h"

/****************************************************************************/

static void lua_pusherrno(lua_State* L, int err)
{
	int* udata;
	udata = (int*)lua_newuserdata(L, sizeof(int));
	*udata = err;
	luaL_getmetatable(L, "errno");
	lua_setmetatable(L, -2);
}

int lua_iserrno(lua_State* L, int index)
{
	int iserrno;
	if (lua_type(L, index)!=LUA_TUSERDATA)
		return 0;
	lua_getmetatable(L, index);
	luaL_getmetatable(L, "errno");
	iserrno = lua_rawequal(L, -1, -2);
	lua_pop(L, 2);
	return iserrno;
}

int lua_toerrno(lua_State* L, int index)
{
	return *(int*)lua_touserdata(L, index);
}

static int luaL_checkerrno(lua_State* L, int index)
{
	return *(int*)luaL_checkudata(L, index, "errno");
}

/****************************************************************************/

static int errno__tostring(lua_State* L)
{
	int err;
	const char* result;
	
	err = luaL_checkerrno(L, 1);
	
	result = strerror(err);
	
	lua_pushstring(L, result);
	return 1;
}

static const luaL_Reg errno_mt[] = {
	{"__tostring", errno__tostring},
	{0, 0},
};

void push_errno_table(lua_State* L)
{
	if (luaL_newmetatable(L, "errno"))
		setfuncs(L, errno_mt, 0);
	lua_pop(L, 1);
	
	lua_newtable(L);
	#define REGISTER_ERROR(error) \
		lua_pushstring(L, #error); lua_pusherrno(L, error); lua_settable(L, -3);
	REGISTER_ERROR(EPERM)
	REGISTER_ERROR(ENOENT)
	REGISTER_ERROR(ESRCH)
	REGISTER_ERROR(EINTR)
	REGISTER_ERROR(EIO)
	REGISTER_ERROR(ENXIO)
	REGISTER_ERROR(E2BIG)
	REGISTER_ERROR(ENOEXEC)
	REGISTER_ERROR(EBADF)
	REGISTER_ERROR(ECHILD)
	REGISTER_ERROR(EAGAIN)
	REGISTER_ERROR(ENOMEM)
	REGISTER_ERROR(EACCES)
	REGISTER_ERROR(EFAULT)
	REGISTER_ERROR(ENOTBLK)
	REGISTER_ERROR(EBUSY)
	REGISTER_ERROR(EEXIST)
	REGISTER_ERROR(EXDEV)
	REGISTER_ERROR(ENODEV)
	REGISTER_ERROR(ENOTDIR)
	REGISTER_ERROR(EISDIR)
	REGISTER_ERROR(EINVAL)
	REGISTER_ERROR(ENFILE)
	REGISTER_ERROR(EMFILE)
	REGISTER_ERROR(ENOTTY)
	REGISTER_ERROR(ETXTBSY)
	REGISTER_ERROR(EFBIG)
	REGISTER_ERROR(ENOSPC)
	REGISTER_ERROR(ESPIPE)
	REGISTER_ERROR(EROFS)
	REGISTER_ERROR(EMLINK)
	REGISTER_ERROR(EPIPE)
	REGISTER_ERROR(EDOM)
	REGISTER_ERROR(ERANGE)
	REGISTER_ERROR(ENOSYS)
	REGISTER_ERROR(ENODATA)
	#undef REGISTER_ERROR
	lua_pushstring(L, "ENOATTR"); lua_pusherrno(L, ENODATA); lua_settable(L, -3);
}

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
