#ifndef FUSE_USE_VERSION
#error FUSE_USE_VERSION is not defined.
#endif
#if FUSE_USE_VERSION > 26
//#error Versions newer than FUSE 2.6 (26) are not yet supported (FUSE 2.7.x and 2.8.x use APIv26).
#endif
#if FUSE_USE_VERSION < 25
#error Versions older than FUSE 2.5 (25) are not yet supported.
#endif

#define _GNU_SOURCE 1
#define _FILE_OFFSET_BITS 64

#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <fuse.h>
#include <sys/xattr.h>
#include <lua.h>
#include <lauxlib.h>

#include "compat.h"
#include "posix_structs.h"

/****************************************************************************/

int lua_iserrno(lua_State* L, int index);
int lua_toerrno(lua_State* L, int index);
void push_errno_table(lua_State* L);

#define PROLOGUE(method) \
	struct fuse_context* context; \
	lua_State* L; \
	int top; \
	int result = -ENOSYS; \
	 \
	context = fuse_get_context(); \
	L = (lua_State*)context->private_data; \
	 \
	top = lua_gettop(L); \
	lua_getfield(L, 2, #method);

#define CALL(nresult) \
	if (lua_pcall(L, lua_gettop(L)-top-1, nresult, 0)) \
	{ \
		/* if error is a small integer, return it */ \
		/* otherwise propagate the value */ \
		int err; \
		/* :KLUDGE: luaB_error is calling lua_isstring, and numbers get converted to strings, so we need to use userdata */ \
		if (!lua_iserrno(L, -1)) \
			lua_error(L); \
		err = lua_toerrno(L, -1); \
		return -err; \
	}

#define EPILOGUE() \
	lua_settop(L, top); \
	return result;

#define EPILOGUE1(method, resultname) \
	/* if result is invalid throw an error */ \
	if (result == -ENOSYS) \
	{ \
		lua_pushliteral(L, "bad result #1 from '"); \
		lua_pushliteral(L, #method); \
		lua_pushliteral(L, "' ("); \
		lua_pushliteral(L, resultname); \
		lua_pushliteral(L, " expected, got "); \
		lua_pushstring(L, luaL_typename(L, top+1)); \
		lua_pushliteral(L, ")"); \
		lua_concat(L, 7); \
		luaL_error(L, lua_tostring(L, -1)); \
	} \
	EPILOGUE()

#define EPILOGUE_I(method) \
	if (lua_type(L, -1)==LUA_TNUMBER) \
	{ \
		lua_Number n; \
		int i; \
		n = lua_tonumber(L, -1); \
		i = (int)n; \
		if (n==(lua_Number)i && i >= 0) \
			result = i; \
	} \
	EPILOGUE1(method, "positive integer")

#define EPILOGUE_S(method, buf, size) \
	if (lua_type(L, -1)==LUA_TSTRING) \
	{ \
		size_t len; \
		const char* str; \
		str = lua_tolstring(L, -1, &len); \
		if (len > size-1) \
			len = size-1; \
		memcpy(buf, str, len); \
		buf[len] = '\0'; \
		result = 0; \
	} \
	EPILOGUE1(method, "string")

#define EPILOGUE_BIN(method, buf, size) \
	if (lua_type(L, -1)==LUA_TSTRING) \
	{ \
		size_t len; \
		const char* str; \
		str = lua_tolstring(L, -1, &len); \
		if (buf) \
		{ \
			if (len > size) \
				len = size; \
			memcpy(buf, str, len); \
		} \
		result = len; \
	} \
	EPILOGUE1(method, "string")

#define DEFINE_STUB(method)

/****************************************************************************/

static int lua__fuse_file_info__index(lua_State* L)
{
	struct fuse_file_info** pfi;
	pfi = lua_touserdata(L, 1);
	if (*pfi)
	{
		struct fuse_file_info* fi = *pfi;
		if (lua_type(L, 2)==LUA_TSTRING)
		{
			const char* k = lua_tostring(L, 2);
			if (!strcmp(k, "flags")) lua__push__open_flags(L, fi->flags);
			else if (!strcmp(k, "writepage")) lua_pushnumber(L, fi->writepage);
			else if (!strcmp(k, "direct_io")) lua_pushboolean(L, fi->direct_io);
			else if (!strcmp(k, "keep_cache")) lua_pushboolean(L, fi->keep_cache);
			else if (!strcmp(k, "flush")) lua_pushboolean(L, fi->flush);
			else if (!strcmp(k, "fh")) lua_pushnumber(L, fi->fh);
			else if (!strcmp(k, "lock_owner")) lua_pushnumber(L, fi->lock_owner);
			else lua_pushnil(L);
			return 1;
		}
	}
	lua_pushnil(L);
	return 1;
}

static int lua__fuse_file_info__newindex(lua_State* L)
{
	struct fuse_file_info** pfi;
	pfi = lua_touserdata(L, 1);
	if (*pfi)
	{
		struct fuse_file_info* fi = *pfi;
		if (lua_type(L, 2)==LUA_TSTRING)
		{
			const char* k = lua_tostring(L, 2);
			if (!strcmp(k, "flags")) { if (lua_istable(L, 3)) { fi->flags = lua__to__open_flags(L, 3); } }
			else if (!strcmp(k, "writepage")) { if (lua_isnumber(L, 3)) fi->writepage = lua_tonumber(L, 3); }
			else if (!strcmp(k, "direct_io")) fi->direct_io = lua_toboolean(L, 3);
			else if (!strcmp(k, "keep_cache")) fi->keep_cache = lua_toboolean(L, 3);
			else if (!strcmp(k, "flush")) fi->flush = lua_toboolean(L, 3);
			else if (!strcmp(k, "fh")) { if (lua_isnumber(L, 3)) fi->fh = lua_tonumber(L, 3); }
			else if (!strcmp(k, "lock_owner")) { if (lua_isnumber(L, 3)) fi->lock_owner = lua_tonumber(L, 3); }
		}
	}
	return 0;
}

static void lua__push__fuse_file_info(lua_State* L, struct fuse_file_info* fi)
{
	struct fuse_file_info** pfi;
	pfi = (struct fuse_file_info**)lua_newuserdata(L, sizeof(struct fuse_file_info*));
	*pfi = fi;
	lua_newtable(L);
	lua_pushcfunction(L, lua__fuse_file_info__index);
	lua_setfield(L, -2, "__index");
	lua_pushcfunction(L, lua__fuse_file_info__newindex);
	lua_setfield(L, -2, "__newindex");
	lua_setmetatable(L, -2);
}

/****************************************************************************/

DEFINE_STUB(getattr)
static int fuse__getattr(const char* path, struct stat* st)
{
	PROLOGUE(getattr)
	lua_pushstring(L, path);
	CALL(1)
	if (lua_type(L, -1)==LUA_TTABLE)
	{
		lua_pushnil(L);
		while (lua_next(L, -2))
		{
			if (lua_type(L, -2)==LUA_TSTRING)
			{
				const char* k = lua_tostring(L, -2);
				if (!strcmp(k, "dev")) { if (lua_isnumber(L, -1)) st->st_dev = lua_tonumber(L, -1); }
				else if (!strcmp(k, "ino")) { if (lua_isnumber(L, -1)) st->st_ino = lua_tonumber(L, -1); }
				else if (!strcmp(k, "mode")) { if (lua_istable(L, -1) || lua_isstring(L, -1)) st->st_mode = lua__to__stat_mode(L, -1); }
				else if (!strcmp(k, "nlink")) { if (lua_isnumber(L, -1)) st->st_nlink = lua_tonumber(L, -1); }
				else if (!strcmp(k, "uid")) { if (lua_isnumber(L, -1)) st->st_uid = lua_tonumber(L, -1); }
				else if (!strcmp(k, "gid")) { if (lua_isnumber(L, -1)) st->st_gid = lua_tonumber(L, -1); }
				else if (!strcmp(k, "rdev")) { if (lua_isnumber(L, -1)) st->st_rdev = lua_tonumber(L, -1); }
				else if (!strcmp(k, "access")) { if (lua_isnumber(L, -1)) st->st_atime = lua_tonumber(L, -1); }
				else if (!strcmp(k, "modification")) { if (lua_isnumber(L, -1)) st->st_mtime = lua_tonumber(L, -1); }
				else if (!strcmp(k, "change")) { if (lua_isnumber(L, -1)) st->st_ctime = lua_tonumber(L, -1); }
				else if (!strcmp(k, "size")) { if (lua_isnumber(L, -1)) st->st_size = lua_tonumber(L, -1); }
				else if (!strcmp(k, "blocks")) { if (lua_isnumber(L, -1)) st->st_blocks = lua_tonumber(L, -1); }
				else if (!strcmp(k, "blksize")) { if (lua_isnumber(L, -1)) st->st_blksize = lua_tonumber(L, -1); }
			}
			lua_pop(L, 1);
		}
	}
	result = 0;
	EPILOGUE()
}

DEFINE_STUB(readlink)
static int fuse__readlink(const char* path, char* buf, size_t size)
{
	PROLOGUE(readlink)
	lua_pushstring(L, path);
	lua_pushnumber(L, size);
	CALL(1)
	EPILOGUE_S(readlink, buf, size)
}

DEFINE_STUB(mknod)
static int fuse__mknod(const char* path, mode_t mode, dev_t rdev)
{
	PROLOGUE(mknod)
	lua_pushstring(L, path);
	lua__push__stat_mode(L, mode);
	lua_pushnumber(L, rdev);
	CALL(0)
	result = 0;
	EPILOGUE()
}

DEFINE_STUB(mkdir)
static int fuse__mkdir(const char* path, mode_t mode)
{
	PROLOGUE(mkdir)
	lua_pushstring(L, path);
	lua__push__stat_mode(L, mode);
	CALL(0)
	result = 0;
	EPILOGUE()
}

DEFINE_STUB(unlink)
static int fuse__unlink(const char* path)
{
	PROLOGUE(unlink)
	lua_pushstring(L, path);
	CALL(0)
	result = 0;
	EPILOGUE()
}

DEFINE_STUB(rmdir)
static int fuse__rmdir(const char* path)
{
	PROLOGUE(rmdir)
	lua_pushstring(L, path);
	CALL(0)
	result = 0;
	EPILOGUE()
}

DEFINE_STUB(symlink)
static int fuse__symlink(const char* linkname, const char* path)
{
	PROLOGUE(symlink)
	lua_pushstring(L, linkname);
	lua_pushstring(L, path);
	CALL(0)
	result = 0;
	EPILOGUE()
}

DEFINE_STUB(rename)
static int fuse__rename(const char* oldpath, const char* newpath)
{
	PROLOGUE(rename)
	lua_pushstring(L, oldpath);
	lua_pushstring(L, newpath);
	CALL(0)
	result = 0;
	EPILOGUE()
}

DEFINE_STUB(link)
static int fuse__link(const char* oldpath, const char* newpath)
{
	PROLOGUE(link)
	lua_pushstring(L, oldpath);
	lua_pushstring(L, newpath);
	CALL(0)
	result = 0;
	EPILOGUE()
}

DEFINE_STUB(chmod)
static int fuse__chmod(const char* path, mode_t mode)
{
	PROLOGUE(chmod)
	lua_pushstring(L, path);
	lua__push__stat_mode(L, mode);
	CALL(0)
	result = 0;
	EPILOGUE()
}

DEFINE_STUB(chown)
static int fuse__chown(const char* path, uid_t uid, gid_t gid)
{
	PROLOGUE(chown)
	lua_pushstring(L, path);
	lua_pushnumber(L, uid);
	lua_pushnumber(L, gid);
	CALL(0)
	result = 0;
	EPILOGUE()
}

DEFINE_STUB(truncate)
static int fuse__truncate(const char* path, off_t size)
{
	PROLOGUE(truncate)
	lua_pushstring(L, path);
	lua_pushnumber(L, size);
	CALL(0)
	result = 0;
	EPILOGUE()
}

DEFINE_STUB(open)
static int fuse__open(const char* path, struct fuse_file_info* fi)
{
	PROLOGUE(open)
	lua_pushstring(L, path);
	lua__push__fuse_file_info(L, fi);
	CALL(0)
	result = 0;
	EPILOGUE()
}

DEFINE_STUB(read)
static int fuse__read(const char* path, char* buf, size_t size, off_t offset, struct fuse_file_info* fi)
{
	PROLOGUE(read)
	lua_pushstring(L, path);
	lua_pushnumber(L, size);
	lua_pushnumber(L, offset);
	lua__push__fuse_file_info(L, fi);
	CALL(1)
	if (lua_type(L, -1)==LUA_TSTRING)
	{
		size_t len;
		const char* str;
		str = lua_tolstring(L, -1, &len);
		if (len > size)
			len = size;
		memcpy(buf, str, len);
		result = len;
	}
	EPILOGUE1(read, "string")
}

DEFINE_STUB(write)
static int fuse__write(const char* path, const char* buf, size_t size, off_t offset, struct fuse_file_info* fi)
{
	PROLOGUE(write)
	lua_pushstring(L, path);
	lua_pushlstring(L, (const char*)buf, size);
	lua_pushnumber(L, offset);
	lua__push__fuse_file_info(L, fi);
	CALL(1)
	EPILOGUE_I(write)
}

DEFINE_STUB(statfs)
static int fuse__statfs(const char* path, struct statvfs* st)
{
	PROLOGUE(statfs)
	lua_pushstring(L, path);
	CALL(1)
	if (lua_type(L, -1)==LUA_TTABLE)
	{
		lua_pushnil(L);
		while (lua_next(L, -2))
		{
			if (lua_type(L, -2)==LUA_TSTRING)
			{
				const char* k = lua_tostring(L, -2);
				if (!strcmp(k, "bsize")) { if (lua_isnumber(L, -1)) st->f_bsize = lua_tonumber(L, -1); }
				else if (!strcmp(k, "frsize")) { if (lua_isnumber(L, -1)) st->f_frsize = lua_tonumber(L, -1); }
				else if (!strcmp(k, "blocks")) { if (lua_isnumber(L, -1)) st->f_blocks = lua_tonumber(L, -1); }
				else if (!strcmp(k, "bfree")) { if (lua_isnumber(L, -1)) st->f_bfree = lua_tonumber(L, -1); }
				else if (!strcmp(k, "bavail")) { if (lua_isnumber(L, -1)) st->f_bavail = lua_tonumber(L, -1); }
				else if (!strcmp(k, "files")) { if (lua_isnumber(L, -1)) st->f_files = lua_tonumber(L, -1); }
				else if (!strcmp(k, "ffree")) { if (lua_isnumber(L, -1)) st->f_ffree = lua_tonumber(L, -1); }
				else if (!strcmp(k, "favail")) { if (lua_isnumber(L, -1)) st->f_favail = lua_tonumber(L, -1); }
				else if (!strcmp(k, "fsid")) { if (lua_isnumber(L, -1)) st->f_fsid = lua_tonumber(L, -1); }
				else if (!strcmp(k, "flag")) { if (lua_istable(L, -1)) st->f_flag = lua__to__statvfs_flag(L, -1); }
				else if (!strcmp(k, "namemax")) { if (lua_isnumber(L, -1)) st->f_namemax = lua_tonumber(L, -1); }
			}
			lua_pop(L, 1);
		}
	}
	result = 0;
	EPILOGUE()
}

DEFINE_STUB(flush)
static int fuse__flush(const char* path, struct fuse_file_info* fi)
{
	PROLOGUE(flush)
	lua_pushstring(L, path);
	lua__push__fuse_file_info(L, fi);
	CALL(0)
	result = 0;
	EPILOGUE()
}

DEFINE_STUB(release)
static int fuse__release(const char* path, struct fuse_file_info* fi)
{
	PROLOGUE(release)
	lua_pushstring(L, path);
	lua__push__fuse_file_info(L, fi);
	CALL(0)
	result = 0;
	EPILOGUE()
}

DEFINE_STUB(fsync)
static int fuse__fsync(const char* path, int isdatasync, struct fuse_file_info* fi)
{
	PROLOGUE(fsync)
	lua_pushstring(L, path);
	lua_pushboolean(L, isdatasync!=0?1:0);
	lua__push__fuse_file_info(L, fi);
	CALL(0)
	result = 0;
	EPILOGUE()
}

static void push_xattr_flags(lua_State* L, int flags)
{
	lua_newtable(L);
#define ADD_FLAG(cflag, lflag) \
	if(flags & cflag) \
	{ \
		lua_pushboolean(L, 1); \
		lua_setfield(L, -2, #lflag); \
	}
	ADD_FLAG(XATTR_CREATE, create)
	ADD_FLAG(XATTR_REPLACE, replace)
#undef ADD_FLAG
}

DEFINE_STUB(setxattr)
static int fuse__setxattr(const char* path, const char* name, const char* value, size_t size, int flags)
{
	PROLOGUE(setxattr)
	lua_pushstring(L, path);
	lua_pushstring(L, name);
	lua_pushlstring(L, value, size);
	push_xattr_flags(L, flags);
	CALL(0)
	result = 0;
	EPILOGUE()
}

DEFINE_STUB(getxattr)
static int fuse__getxattr(const char* path, const char* name, char* value, size_t size)
{
	PROLOGUE(getxattr)
	lua_pushstring(L, path);
	lua_pushstring(L, name);
//	lua_pushnumber(L, size);
	CALL(1)
	EPILOGUE_BIN(getxattr, value, size)
}

DEFINE_STUB(listxattr)
static int fuse__listxattr(const char* path, char* list, size_t size)
{
	PROLOGUE(listxattr)
	lua_pushstring(L, path);
//	lua_pushnumber(L, size);
	CALL(1)
	if (lua_istable(L, -1))
	{
		size_t len, i;
		int ok;
		ok = 1;
		len = rawlen(L, -1);
		for (i=1; i<=len; ++i)
		{
			lua_rawgeti(L, -1, i);
			if (lua_type(L, -1)!=LUA_TSTRING)
				ok = 0;
			lua_pop(L, 1);
		}
		if (ok)
		{
			size_t len;
			const char* str;
			lua_getglobal(L, "table");
			lua_getfield(L, -1, "concat");
			lua_replace(L, -2);
			lua_insert(L, -2);
			lua_pushlstring(L, "\0", 1);
			lua_call(L, 2, 1);
			str = lua_tolstring(L, -1, &len);
			if(len > 0)
				++len; // add the trailing 0
			if (list)
			{
				if (len > size)
					len = size;
				memcpy(list, str, len);
			}
			result = len;
		}
	}
	EPILOGUE1(listxattr, "array of strings")
}

DEFINE_STUB(removexattr)
static int fuse__removexattr(const char* path, const char* name)
{
	PROLOGUE(removexattr)
	lua_pushstring(L, path);
	lua_pushstring(L, name);
	CALL(0)
	result = 0;
	EPILOGUE()
}

DEFINE_STUB(opendir)
static int fuse__opendir(const char* path, struct fuse_file_info* fi)
{
	PROLOGUE(opendir)
	lua_pushstring(L, path);
	lua__push__fuse_file_info(L, fi);
	CALL(0)
	result = 0;
	EPILOGUE()
}

static int lua__fuse_fill_dir(lua_State* L)
{
	void* buf;
	int result;
	fuse_fill_dir_t filler;
	buf = lua_touserdata(L, lua_upvalueindex(1));
	filler = lua_touserdata(L, lua_upvalueindex(2));
	luaL_checktype(L, 1, LUA_TSTRING);
	result = filler(buf, lua_tostring(L, 1), NULL,0, 0);
	lua_pushboolean(L, result==0);
	return 1;
}

DEFINE_STUB(readdir)
static int fuse__readdir(const char* path, void* buf, fuse_fill_dir_t filler, off_t off, struct fuse_file_info* fi)
{
	PROLOGUE(readdir)
	lua_pushstring(L, path);
	lua_pushlightuserdata(L, buf);
	lua_pushlightuserdata(L, filler);
	lua_pushcclosure(L, lua__fuse_fill_dir, 2);
	(void)off; /* unused parameter */
	lua__push__fuse_file_info(L, fi);
	CALL(0)
	result = 0;
	EPILOGUE()
}

DEFINE_STUB(releasedir)
static int fuse__releasedir(const char* path, struct fuse_file_info* fi)
{
	PROLOGUE(releasedir)
	lua_pushstring(L, path);
	lua__push__fuse_file_info(L, fi);
	CALL(0)
	result = 0;
	EPILOGUE()
}

DEFINE_STUB(fsyncdir)
static int fuse__fsyncdir(const char* path, int datasync, struct fuse_file_info* fi)
{
	PROLOGUE(fsyncdir)
	lua_pushstring(L, path);
	lua_pushnumber(L, datasync);
	lua__push__fuse_file_info(L, fi);
	CALL(0)
	result = 0;
	EPILOGUE()
}

/* intialize the state before calling fuse_main */
static lua_State* init__23__state = 0;
static void* fuse__init__23()
{
	lua_State* state = init__23__state;
	/* we reset global state pointer because we may have several ones */
	init__23__state = 0;
	return state;
}

static void* fuse__init(struct fuse_conn_info* conn)
{
	(void)conn;
	return 0;
}

static void fuse__destroy(void* data)
{
	(void)data;
}

DEFINE_STUB(access)
static int fuse__access(const char* path, int mask)
{
	PROLOGUE(access)
	lua_pushstring(L, path);
	lua_pushnumber(L, mask);
	CALL(0)
	result = 0;
	EPILOGUE()
}

DEFINE_STUB(create)
static int fuse__create(const char* path, mode_t mode, struct fuse_file_info* fi)
{
	PROLOGUE(create)
	lua_pushstring(L, path);
	lua__push__stat_mode(L, mode);
	lua__push__fuse_file_info(L, fi);
	CALL(0)
	result = 0;
	EPILOGUE()
}
/*
DEFINE_STUB(ftruncate)
static int fuse__ftruncate(const char* path, off_t size, struct fuse_file_info* fi)
{
	PROLOGUE(ftruncate)
	lua_pushstring(L, path);
	lua_pushnumber(L, size);
	lua__push__fuse_file_info(L, fi);
	CALL(0)
	result = 0;
	EPILOGUE()
}
*/
DEFINE_STUB(fgetattr)
static int fuse__fgetattr(const char* path, struct stat* st, struct fuse_file_info* fi)
{
	PROLOGUE(fgetattr)
	lua_pushstring(L, path);
	lua__push__stat(L, st);
	lua__push__fuse_file_info(L, fi);
	CALL(0)
	result = 0;
	EPILOGUE()
}
/*
DEFINE_STUB(lock)
static int fuse__lock(const char* path, struct fuse_file_info* fi, int cmd, struct flock* lock)
{
	PROLOGUE(lock)
	lua_pushstring(L, path);
	lua__push__fuse_file_info(L, fi);
	lua_pushnumber(L, cmd);
	lua_pushlightuserdata(L, lock);
	CALL(0)
	result = 0;
	EPILOGUE()
}
*/

DEFINE_STUB(utimens)
static int fuse__utimens(const char* path, const struct timespec tv[2])
{
	PROLOGUE(utimens)
	lua_pushstring(L, path);
	lua_createtable(L, 2, 0);
	lua__push__timespec(L, &tv[0]); lua_rawseti(L, -2, 1);
	lua__push__timespec(L, &tv[1]); lua_rawseti(L, -2, 2);
	CALL(0)
	result = 0;
	EPILOGUE()
}

/****************************************************************************/

static int lua__main(lua_State *L)
{
//	pthread_rwlock_t* lock;
	int largc, cargc;
	char** argv;
	int i;
	struct fuse_operations oper;

	// 1: argv
	// 2: method table

	luaL_checktype(L, 1, LUA_TTABLE);   // argv
	luaL_checktype(L, 2, LUA_TTABLE);   // methods
	lua_settop(L, 2);   // Keep only 2 elements

	largc = rawlen(L, 1);   // --------------------WHERE IS THIS DEFINED????
	cargc = 1;  // To store -s option
	lua_newtable(L);    // New table - Stack top(ST)=3
	argv = (char**)lua_newuserdata(L, sizeof(char*) * (largc + cargc + 1)); // To store user data for all strings in argv (largc) + -s option and finally NULL (ST=4)
	lua_rawseti(L, -2, 0);  // Put the userdata in 0 index of the new Lua Table at 3, ST=3
	for (i=0; i < largc; ++i)
	{
		size_t len;
		const char* str;
		char* udata;
		lua_rawgeti(L, 1, i+1); // Get the ith string from the argv Lua table to top of stack ST=4
		if (!lua_isstring(L, -1))
			return typeerror(L, 1, "array of strings");
		str = lua_tolstring(L, -1, &len);   // convert the Lua string to a C string on top of stack
		udata = (char*)lua_newuserdata(L, len+1);   // New user data on top of stack to store the string ST=5
		memcpy(udata, str, len+1);  // Copy the string data to the userdata data
		argv[i] = udata;    // Copy the userdata to the argv user data
		lua_rawseti(L, -3, i+1);    // Put the userdata in the new methods table at index i+1 ST=4
		lua_pop(L, 1);  // Pop the string from the stack ST=3
	}
	// Single threaded operation for the moment
	{
		size_t len;
		const char* str;
		char* udata;
		len = 2;
		str = "-s";
		udata = (char*)lua_newuserdata(L, len+1); //ST=4
		memcpy(udata, str, len+1);
		argv[i] = udata;
		lua_rawseti(L, -2, i+1); //ST=3
	}
	++i;
	argv[i] = NULL;
	lua_replace(L, 1);  // Replace argv with the new table containing all the strings in userdata and 0 index containing the argv userdata ST=2

	lua_newtable(L);    // ST=3
	memset(&oper, 0, sizeof(struct fuse_operations));   // initialize oper space with all 0s
	#define CHECK_METHOD(method) \
		lua_getfield(L, 2, #method); \
		if (lua_isfunction(L, -1)) \
		{ \
			oper.method = fuse__##method; \
			lua_setfield(L, -2, #method); \
		} \
		else \
			lua_pop(L, 1);
	CHECK_METHOD(getattr)   // adds the fuse__getattr method to the new table created
	CHECK_METHOD(readlink)
//	CHECK_METHOD(getdir) /* deprecated */
	CHECK_METHOD(mknod)
	CHECK_METHOD(mkdir)
	CHECK_METHOD(unlink)
	CHECK_METHOD(rmdir)
	CHECK_METHOD(symlink)
	CHECK_METHOD(rename)
	CHECK_METHOD(link)
	CHECK_METHOD(chmod)
	CHECK_METHOD(chown)
	CHECK_METHOD(truncate)
//	CHECK_METHOD(utime) /* deprecated */
#if FUSE_USE_VERSION >= 22
	CHECK_METHOD(open)
	CHECK_METHOD(read)
	CHECK_METHOD(write)
#endif
#if FUSE_USE_VERSION >= 25
	CHECK_METHOD(statfs)
#endif
#if FUSE_USE_VERSION >= 22
	CHECK_METHOD(flush)
	CHECK_METHOD(release)
	CHECK_METHOD(fsync)
#endif
	CHECK_METHOD(setxattr)
	CHECK_METHOD(getxattr)
	CHECK_METHOD(listxattr)
	CHECK_METHOD(removexattr)
#if FUSE_USE_VERSION >= 23
	CHECK_METHOD(opendir)
	CHECK_METHOD(readdir)
	CHECK_METHOD(releasedir)
	CHECK_METHOD(fsyncdir)
#endif
#if FUSE_USE_VERSION >= 26
//	CHECK_METHOD(init) /* :TODO: */
#elif FUSE_USE_VERSION >= 23
	oper.init = fuse__init__23;
#endif
#if FUSE_USE_VERSION >= 23
//	CHECK_METHOD(destroy) /* :TODO: */
#endif
#if FUSE_USE_VERSION >= 25
	CHECK_METHOD(access)
	CHECK_METHOD(create)
	//CHECK_METHOD(ftruncate)
	//CHECK_METHOD(fgetattr)
#endif
#if FUSE_USE_VERSION >= 26
//	CHECK_METHOD(lock) /* :TODO: */
	CHECK_METHOD(utimens)
//	CHECK_METHOD(bmap) /* :TODO: */
#endif
	#undef CHECK_METHOD
	lua_replace(L, 2);  // The new table containing all the identified functions

#if FUSE_USE_VERSION >= 26
	fuse_main(largc + cargc, argv, &oper, L);
#elif FUSE_USE_VERSION >= 23
	if (init__23__state!=0) return luaL_error(L, "another FUSE filesystem is being run, try again later");
	init__23__state = L;
	fuse_main(largc + cargc, argv, &oper);
#endif

	return 0;
}

static int lua__get_context(lua_State *L)
{
	struct fuse_context* context;
	context = fuse_get_context();
	if (lua_gettop(L) == 0)
		lua_createtable(L, 0, 3);
	else if (lua_type(L, 1)!=LUA_TTABLE)
		return typeerror(L, 1, "table or nil");
	lua_pushnumber(L, context->uid); lua_setfield(L, -2, "uid");
	lua_pushnumber(L, context->gid); lua_setfield(L, -2, "gid");
	lua_pushnumber(L, context->pid); lua_setfield(L, -2, "pid");
	return 1;
}

/****************************************************************************/

static const luaL_Reg functions[] = {
	{"main", lua__main},
	{"get_context", lua__get_context},
	{0, 0},
};

__attribute__((visibility ("default"))) int luaopen_flu(lua_State *L)
{
#if LUA_VERSION_NUM==502 || LUA_VERSION_NUM==503
	lua_newtable(L);
#elif LUA_VERSION_NUM==501
	{
		static luaL_Reg empty[] = { {0, 0} };
		luaL_register(L, lua_tostring(L, 1), empty);
	}
#else
#error unsupported Lua version
#endif

	setfuncs(L, functions, 0);

	lua_pushnumber(L, FUSE_USE_VERSION);
	lua_setfield(L, -2, "FUSE_USE_VERSION");

	push_errno_table(L);
	lua_setfield(L, -2, "errno");

#if LUA_VERSION_NUM==502 || LUA_VERSION_NUM==503
	return 1;
#else
	return 0;
#endif
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
