#define _GNU_SOURCE 1
#define _FILE_OFFSET_BITS 64

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
#include "compat.h"

/****************************************************************************/

void lua__push__stat_mode(lua_State* L, mode_t m)
{
	lua_newtable(L);
	if (S_ISBLK(m))		{ lua_pushboolean(L, 1); lua_setfield(L, -2, "blk"); }
	if (S_ISCHR(m))		{ lua_pushboolean(L, 1); lua_setfield(L, -2, "chr"); }
	if (S_ISFIFO(m))	{ lua_pushboolean(L, 1); lua_setfield(L, -2, "fifo"); }
	if (S_ISREG(m))		{ lua_pushboolean(L, 1); lua_setfield(L, -2, "reg"); }
	if (S_ISDIR(m))		{ lua_pushboolean(L, 1); lua_setfield(L, -2, "dir"); }
	if (S_ISLNK(m))		{ lua_pushboolean(L, 1); lua_setfield(L, -2, "lnk"); }
	if (S_ISSOCK(m))	{ lua_pushboolean(L, 1); lua_setfield(L, -2, "sock"); }
	if (m & S_IRUSR)	{ lua_pushboolean(L, 1); lua_setfield(L, -2, "rusr"); }
	if (m & S_IWUSR)	{ lua_pushboolean(L, 1); lua_setfield(L, -2, "wusr"); }
	if (m & S_IXUSR)	{ lua_pushboolean(L, 1); lua_setfield(L, -2, "xusr"); }
	if (m & S_IRGRP)	{ lua_pushboolean(L, 1); lua_setfield(L, -2, "rgrp"); }
	if (m & S_IWGRP)	{ lua_pushboolean(L, 1); lua_setfield(L, -2, "wgrp"); }
	if (m & S_IXGRP)	{ lua_pushboolean(L, 1); lua_setfield(L, -2, "xgrp"); }
	if (m & S_IROTH)	{ lua_pushboolean(L, 1); lua_setfield(L, -2, "roth"); }
	if (m & S_IWOTH)	{ lua_pushboolean(L, 1); lua_setfield(L, -2, "woth"); }
	if (m & S_IXOTH)	{ lua_pushboolean(L, 1); lua_setfield(L, -2, "xoth"); }
	if (m & S_ISUID)	{ lua_pushboolean(L, 1); lua_setfield(L, -2, "suid"); }
	if (m & S_ISGID)	{ lua_pushboolean(L, 1); lua_setfield(L, -2, "sgid"); }
	if (m & S_ISVTX)	{ lua_pushboolean(L, 1); lua_setfield(L, -2, "svtx"); }
}

mode_t lua__to__stat_mode(lua_State* L, int index)
{
	mode_t m;
	if (lua_istable(L, index))
	{
		if (index < 0)
			index = lua_gettop(L) + 1 + index;
		m = 0;
		lua_pushnil(L);
		while (lua_next(L, index))
		{
			if (lua_type(L, -2)==LUA_TSTRING)
			{
				const char* k = lua_tostring(L, -2);
				if (!strcmp(k, "blk"))			{ if (lua_toboolean(L, -1)!=0) m |= S_IFBLK; }
				else if (!strcmp(k, "chr"))		{ if (lua_toboolean(L, -1)!=0) m |= S_IFCHR; }
				else if (!strcmp(k, "fifo"))	{ if (lua_toboolean(L, -1)!=0) m |= S_IFIFO; }
				else if (!strcmp(k, "reg"))		{ if (lua_toboolean(L, -1)!=0) m |= S_IFREG; }
				else if (!strcmp(k, "dir"))		{ if (lua_toboolean(L, -1)!=0) m |= S_IFDIR; }
				else if (!strcmp(k, "lnk"))		{ if (lua_toboolean(L, -1)!=0) m |= S_IFLNK; }
				else if (!strcmp(k, "sock"))	{ if (lua_toboolean(L, -1)!=0) m |= S_IFSOCK; }
				else if (!strcmp(k, "rusr"))	{ if (lua_toboolean(L, -1)!=0) m |= S_IRUSR; }
				else if (!strcmp(k, "wusr"))	{ if (lua_toboolean(L, -1)!=0) m |= S_IWUSR; }
				else if (!strcmp(k, "xusr"))	{ if (lua_toboolean(L, -1)!=0) m |= S_IXUSR; }
				else if (!strcmp(k, "rgrp"))	{ if (lua_toboolean(L, -1)!=0) m |= S_IRGRP; }
				else if (!strcmp(k, "wgrp"))	{ if (lua_toboolean(L, -1)!=0) m |= S_IWGRP; }
				else if (!strcmp(k, "xgrp"))	{ if (lua_toboolean(L, -1)!=0) m |= S_IXGRP; }
				else if (!strcmp(k, "roth"))	{ if (lua_toboolean(L, -1)!=0) m |= S_IROTH; }
				else if (!strcmp(k, "woth"))	{ if (lua_toboolean(L, -1)!=0) m |= S_IWOTH; }
				else if (!strcmp(k, "xoth"))	{ if (lua_toboolean(L, -1)!=0) m |= S_IXOTH; }
				else if (!strcmp(k, "suid"))	{ if (lua_toboolean(L, -1)!=0) m |= S_ISUID; }
				else if (!strcmp(k, "sgid"))	{ if (lua_toboolean(L, -1)!=0) m |= S_ISGID; }
				else if (!strcmp(k, "svtx"))	{ if (lua_toboolean(L, -1)!=0) m |= S_ISVTX; }
			}
			lua_pop(L, 1);
		}
	}
	else if (lua_type(L, index)==LUA_TSTRING)
	{
		const char* value = lua_tostring(L, -1);
		if (!strcmp(value, "file")) m = S_IFREG | 0660;
		else if (!strcmp(value, "directory")) m = S_IFDIR | 0770;
		else if (!strcmp(value, "link")) m = S_IFLNK | 0660;
		else if (!strcmp(value, "socket")) m = S_IFSOCK | 0660;
		else if (!strcmp(value, "named pipe")) m = S_IFIFO | 0660;
		else if (!strcmp(value, "char device")) m = S_IFCHR | 0660;
		else if (!strcmp(value, "block device")) m = S_IFBLK | 0660;
		else if (!strcmp(value, "other")) m = 0;
		else m = 0;
	}
	else
		m = 0;
	return m;
}

/****************************************************************************/

static int lua__stat__index(lua_State* L)
{
	struct stat** pst;
	pst = lua_touserdata(L, 1);
	if (*pst)
	{
		struct stat* st = *pst;
		if (lua_type(L, 2)==LUA_TSTRING)
		{
			const char* k = lua_tostring(L, 2);
			if (!strcmp(k, "dev")) lua_pushnumber(L, st->st_dev);
			else if (!strcmp(k, "ino")) lua_pushnumber(L, st->st_ino);
			else if (!strcmp(k, "mode")) lua__push__stat_mode(L, st->st_mode);
			else if (!strcmp(k, "nlink")) lua_pushnumber(L, st->st_nlink);
			else if (!strcmp(k, "uid")) lua_pushnumber(L, st->st_uid);
			else if (!strcmp(k, "gid")) lua_pushnumber(L, st->st_gid);
			else if (!strcmp(k, "rdev")) lua_pushnumber(L, st->st_rdev);
			else if (!strcmp(k, "size")) lua_pushnumber(L, st->st_size);
			else if (!strcmp(k, "atime")) lua_pushnumber(L, st->st_atime);
			else if (!strcmp(k, "mtime")) lua_pushnumber(L, st->st_mtime);
			else if (!strcmp(k, "ctime")) lua_pushnumber(L, st->st_ctime);
			else if (!strcmp(k, "blksize")) lua_pushnumber(L, st->st_blksize);
			else if (!strcmp(k, "blocks")) lua_pushnumber(L, st->st_blocks);
			else lua_pushnil(L);
			return 1;
		}
	}
	lua_pushnil(L);
	return 1;
}

static int lua__stat__newindex(lua_State* L)
{
	struct stat** pst;
	pst = lua_touserdata(L, 1);
	if (*pst)
	{
		struct stat* st = *pst;
		if (lua_type(L, 2)==LUA_TSTRING)
		{
			const char* k = lua_tostring(L, 2);
			if (!strcmp(k, "dev")) { if (lua_isnumber(L, 3)) st->st_dev = lua_tonumber(L, 3); }
			else if (!strcmp(k, "ino")) { if (lua_isnumber(L, 3)) st->st_ino = lua_tonumber(L, 3); }
			else if (!strcmp(k, "mode")) { if (lua_istable(L, 3)) st->st_mode = lua__to__stat_mode(L, 3); }
			else if (!strcmp(k, "nlink")) { if (lua_isnumber(L, 3)) st->st_nlink = lua_tonumber(L, 3); }
			else if (!strcmp(k, "uid")) { if (lua_isnumber(L, 3)) st->st_uid = lua_tonumber(L, 3); }
			else if (!strcmp(k, "gid")) { if (lua_isnumber(L, 3)) st->st_gid = lua_tonumber(L, 3); }
			else if (!strcmp(k, "rdev")) { if (lua_isnumber(L, 3)) st->st_rdev = lua_tonumber(L, 3); }
			else if (!strcmp(k, "size")) { if (lua_isnumber(L, 3)) st->st_size = lua_tonumber(L, 3); }
			else if (!strcmp(k, "atime")) { if (lua_isnumber(L, 3)) st->st_atime = lua_tonumber(L, 3); }
			else if (!strcmp(k, "mtime")) { if (lua_isnumber(L, 3)) st->st_mtime = lua_tonumber(L, 3); }
			else if (!strcmp(k, "ctime")) { if (lua_isnumber(L, 3)) st->st_ctime = lua_tonumber(L, 3); }
			else if (!strcmp(k, "blksize")) { if (lua_isnumber(L, 3)) st->st_blksize = lua_tonumber(L, 3); }
			else if (!strcmp(k, "blocks")) { if (lua_isnumber(L, 3)) st->st_blocks = lua_tonumber(L, 3); }
		}
	}
	return 0;
}

static int lua__stat__gc(lua_State* L)
{
	struct stat** pst;
	pst = lua_touserdata(L, 1);
	if (*pst)
	{
		free(*pst);
		*pst = 0;
	}
	return 0;
}

// See http://www.opengroup.org/onlinepubs/009695399/basedefs/sys/stat.h.html
void lua__push__stat(lua_State* L, struct stat* st)
{
	struct stat** pst;
	pst = (struct stat**)lua_newuserdata(L, sizeof(struct stat*));
	if (!st)
	{
		*pst = (struct stat*)malloc(sizeof(struct stat));
		memset(*pst, 0, sizeof(struct stat));
	}
	else
		*pst = st;
	lua_newtable(L);
	lua_pushcfunction(L, lua__stat__index);
	lua_setfield(L, -2, "__index");
	lua_pushcfunction(L, lua__stat__newindex);
	lua_setfield(L, -2, "__newindex");
	if (!st)
	{
		lua_pushcfunction(L, lua__stat__gc);
		lua_setfield(L, -2, "__gc");
	}
	lua_setmetatable(L, -2);
}

/****************************************************************************/

void lua__push__statvfs_flag(lua_State* L, int flags)
{
	lua_newtable(L);
	if (flags & ST_RDONLY)	{ lua_pushboolean(L, 1); lua_setfield(L, -2, "rdonly"); }
	if (flags & ST_NOSUID)	{ lua_pushboolean(L, 1); lua_setfield(L, -2, "nosuid"); }
}

int lua__to__statvfs_flag(lua_State* L, int index)
{
	int flags = 0;
	lua_pushnil(L);
	while (lua_next(L, index))
	{
		if (lua_type(L, -2)==LUA_TSTRING)
		{
			const char* k = lua_tostring(L, -2);
			if (!strcmp(k, "rdonly"))		{ if (lua_toboolean(L, -1)!=0) flags |= ST_RDONLY; }
			else if (!strcmp(k, "nosuid"))	{ if (lua_toboolean(L, -1)!=0) flags |= ST_NOSUID; }
		}
		lua_pop(L, 1);
	}
	return flags;
}

/****************************************************************************/

static int lua__statvfs__index(lua_State* L)
{
	struct statvfs** pst;
	pst = lua_touserdata(L, 1);
	if (*pst)
	{
		struct statvfs* st = *pst;
		if (lua_type(L, 2)==LUA_TSTRING)
		{
			const char* k = lua_tostring(L, 2);
			if (!strcmp(k, "bsize")) lua_pushnumber(L, st->f_bsize);
			else if (!strcmp(k, "frsize")) lua_pushnumber(L, st->f_frsize);
			else if (!strcmp(k, "blocks")) lua_pushnumber(L, st->f_blocks);
			else if (!strcmp(k, "bfree")) lua_pushnumber(L, st->f_bfree);
			else if (!strcmp(k, "bavail")) lua_pushnumber(L, st->f_bavail);
			else if (!strcmp(k, "files")) lua_pushnumber(L, st->f_files);
			else if (!strcmp(k, "ffree")) lua_pushnumber(L, st->f_ffree);
			else if (!strcmp(k, "favail")) lua_pushnumber(L, st->f_favail);
			else if (!strcmp(k, "fsid")) lua_pushnumber(L, st->f_fsid);
			else if (!strcmp(k, "flag")) lua__push__statvfs_flag(L, st->f_flag);
			else if (!strcmp(k, "namemax")) lua_pushnumber(L, st->f_namemax);
			else lua_pushnil(L);
			return 1;
		}
	}
	lua_pushnil(L);
	return 1;
}

static int lua__statvfs__newindex(lua_State* L)
{
	struct statvfs** pst;
	pst = lua_touserdata(L, 1);
	if (*pst)
	{
		struct statvfs* st = *pst;
		if (lua_type(L, 2)==LUA_TSTRING)
		{
			const char* k = lua_tostring(L, 2);
			if (!strcmp(k, "bsize")) { if (lua_isnumber(L, 3)) st->f_bsize = lua_tonumber(L, 3); }
			else if (!strcmp(k, "frsize")) { if (lua_isnumber(L, 3)) st->f_frsize = lua_tonumber(L, 3); }
			else if (!strcmp(k, "blocks")) { if (lua_isnumber(L, 3)) st->f_blocks = lua_tonumber(L, 3); }
			else if (!strcmp(k, "bfree")) { if (lua_isnumber(L, 3)) st->f_bfree = lua_tonumber(L, 3); }
			else if (!strcmp(k, "bavail")) { if (lua_isnumber(L, 3)) st->f_bavail = lua_tonumber(L, 3); }
			else if (!strcmp(k, "files")) { if (lua_isnumber(L, 3)) st->f_files = lua_tonumber(L, 3); }
			else if (!strcmp(k, "ffree")) { if (lua_isnumber(L, 3)) st->f_ffree = lua_tonumber(L, 3); }
			else if (!strcmp(k, "favail")) { if (lua_isnumber(L, 3)) st->f_favail = lua_tonumber(L, 3); }
			else if (!strcmp(k, "fsid")) { if (lua_isnumber(L, 3)) st->f_fsid = lua_tonumber(L, 3); }
			else if (!strcmp(k, "flag")) { if (lua_istable(L, 3)) st->f_flag = lua__to__statvfs_flag(L, 3); }
			else if (!strcmp(k, "namemax")) { if (lua_isnumber(L, 3)) st->f_namemax = lua_tonumber(L, 3); }
		}
	}
	return 0;
}

static int lua__statvfs__gc(lua_State* L)
{
	struct statvfs** pst;
	pst = lua_touserdata(L, 1);
	if (*pst)
	{
		free(*pst);
		*pst = 0;
	}
	return 0;
}

// See http://www.opengroup.org/onlinepubs/009695399/basedefs/sys/statvfs.h.html
void lua__push__statvfs(lua_State* L, struct statvfs* st)
{
	struct statvfs** pst;
	pst = (struct statvfs**)lua_newuserdata(L, sizeof(struct statvfs*));
	if (!st)
	{
		*pst = (struct statvfs*)malloc(sizeof(struct statvfs));
		memset(*pst, 0, sizeof(struct statvfs));
	}
	else
		*pst = st;
	lua_newtable(L);
	lua_pushcfunction(L, lua__statvfs__index);
	lua_setfield(L, -2, "__index");
	lua_pushcfunction(L, lua__statvfs__newindex);
	lua_setfield(L, -2, "__newindex");
	if (!st)
	{
		lua_pushcfunction(L, lua__statvfs__gc);
		lua_setfield(L, -2, "__gc");
	}
	lua_setmetatable(L, -2);
}

/****************************************************************************/

void lua__push__open_flags(lua_State* L, int f)
{
	lua_newtable(L);
	if (f & O_APPEND)		{ lua_pushboolean(L, 1); lua_setfield(L, -2, "append"); }
	if (f & O_ASYNC)		{ lua_pushboolean(L, 1); lua_setfield(L, -2, "async"); }
	if (f & O_CREAT)		{ lua_pushboolean(L, 1); lua_setfield(L, -2, "creat"); }
	if (f & O_DIRECT)		{ lua_pushboolean(L, 1); lua_setfield(L, -2, "direct"); }
	if (f & O_DIRECTORY)	{ lua_pushboolean(L, 1); lua_setfield(L, -2, "directory"); }
	if (f & O_EXCL)			{ lua_pushboolean(L, 1); lua_setfield(L, -2, "excl"); }
	if (f & O_LARGEFILE)	{ lua_pushboolean(L, 1); lua_setfield(L, -2, "largefile"); }
	if (f & O_NOATIME)		{ lua_pushboolean(L, 1); lua_setfield(L, -2, "noatime"); }
	if (f & O_NOCTTY)		{ lua_pushboolean(L, 1); lua_setfield(L, -2, "noctty"); }
	if (f & O_NOFOLLOW)		{ lua_pushboolean(L, 1); lua_setfield(L, -2, "nofollow"); }
	if (f & O_NONBLOCK)		{ lua_pushboolean(L, 1); lua_setfield(L, -2, "nonblock"); }
	if (f & O_NDELAY)		{ lua_pushboolean(L, 1); lua_setfield(L, -2, "ndelay"); }
	if (f & O_SYNC)			{ lua_pushboolean(L, 1); lua_setfield(L, -2, "sync"); }
	if (f & O_TRUNC)		{ lua_pushboolean(L, 1); lua_setfield(L, -2, "trunc"); }
	if (f & O_RDWR)			{ lua_pushboolean(L, 1); lua_setfield(L, -2, "rdwr"); }
	else if (f & O_RDONLY)	{ lua_pushboolean(L, 1); lua_setfield(L, -2, "rdonly"); }
	else if (f & O_WRONLY)	{ lua_pushboolean(L, 1); lua_setfield(L, -2, "wronly"); }
}

int lua__to__open_flags(lua_State* L, int index)
{
	int flags = 0;
	lua_pushnil(L);
	while (lua_next(L, index))
	{
		if (lua_type(L, -2)==LUA_TSTRING)
		{
			const char* k = lua_tostring(L, -2);
			if (!strcmp(k, "append"))			{ if (lua_toboolean(L, -1)!=0) flags |= O_APPEND; }
			else if (!strcmp(k, "async"))		{ if (lua_toboolean(L, -1)!=0) flags |= O_ASYNC; }
			else if (!strcmp(k, "creat"))		{ if (lua_toboolean(L, -1)!=0) flags |= O_CREAT; }
			else if (!strcmp(k, "direct"))		{ if (lua_toboolean(L, -1)!=0) flags |= O_DIRECT; }
			else if (!strcmp(k, "directory"))	{ if (lua_toboolean(L, -1)!=0) flags |= O_DIRECTORY; }
			else if (!strcmp(k, "excl"))		{ if (lua_toboolean(L, -1)!=0) flags |= O_EXCL; }
			else if (!strcmp(k, "largefile"))	{ if (lua_toboolean(L, -1)!=0) flags |= O_LARGEFILE; }
			else if (!strcmp(k, "noatime"))		{ if (lua_toboolean(L, -1)!=0) flags |= O_NOATIME; }
			else if (!strcmp(k, "noctty"))		{ if (lua_toboolean(L, -1)!=0) flags |= O_NOCTTY; }
			else if (!strcmp(k, "nofollow"))	{ if (lua_toboolean(L, -1)!=0) flags |= O_NOFOLLOW; }
			else if (!strcmp(k, "nonblock"))	{ if (lua_toboolean(L, -1)!=0) flags |= O_NONBLOCK; }
			else if (!strcmp(k, "ndelay"))		{ if (lua_toboolean(L, -1)!=0) flags |= O_NDELAY; }
			else if (!strcmp(k, "sync"))		{ if (lua_toboolean(L, -1)!=0) flags |= O_SYNC; }
			else if (!strcmp(k, "trunc"))		{ if (lua_toboolean(L, -1)!=0) flags |= O_TRUNC; }
			else if (!strcmp(k, "rdwr"))		{ if (lua_toboolean(L, -1)!=0) flags |= O_RDWR; }
			else if (!strcmp(k, "rdonly"))		{ if (lua_toboolean(L, -1)!=0) flags |= O_RDONLY; }
			else if (!strcmp(k, "wronly"))		{ if (lua_toboolean(L, -1)!=0) flags |= O_WRONLY; }
		}
		lua_pop(L, 1);
	}
	return flags;
}

/****************************************************************************/

static int lua__dirent__index(lua_State* L)
{
	struct dirent** pent;
	pent = lua_touserdata(L, 1);
	if (*pent)
	{
		struct dirent* ent = *pent;
		if (lua_type(L, 2)==LUA_TSTRING)
		{
			const char* k = lua_tostring(L, 2);
			if (!strcmp(k, "ino")) lua_pushnumber(L, ent->d_ino);
			else if (!strcmp(k, "off")) lua_pushnumber(L, ent->d_off);
			else if (!strcmp(k, "reclen")) lua_pushnumber(L, ent->d_reclen);
			else if (!strcmp(k, "type")) lua_pushnumber(L, ent->d_type);
			else if (!strcmp(k, "name")) lua_pushstring(L, ent->d_name);
			else lua_pushnil(L);
			return 1;
		}
	}
	lua_pushnil(L);
	return 1;
}

static inline int min(int a, int b)
{
	return a<b?a:b;
}

static int lua__dirent__newindex(lua_State* L)
{
	struct dirent** pent;
	pent = (struct dirent**)lua_touserdata(L, 1);
	if (*pent)
	{
		struct dirent* ent = *pent;
		if (lua_type(L, 2)==LUA_TSTRING)
		{
			const char* k = lua_tostring(L, 2);
			if (!strcmp(k, "ino")) { if (lua_isnumber(L, 3)) ent->d_ino = lua_tonumber(L, 3); }
			else if (!strcmp(k, "off")) { if (lua_isnumber(L, 3)) ent->d_off = lua_tonumber(L, 3); }
			else if (!strcmp(k, "reclen")) { if (lua_isnumber(L, 3)) ent->d_reclen = lua_tonumber(L, 3); }
			else if (!strcmp(k, "type")) { if (lua_isnumber(L, 3)) ent->d_reclen = lua_tonumber(L, 3); }
			else if (!strcmp(k, "name")) { if (lua_isstring(L, 3)) strncpy(ent->d_name, lua_tostring(L, 3), min(NAME_MAX, rawlen(L, 3))+1); }
		}
	}
	return 0;
}

static int lua__dirent__gc(lua_State* L)
{
	struct dirent** pent;
	pent = lua_touserdata(L, 1);
	if (*pent)
	{
		free(*pent);
		*pent = 0;
	}
	return 0;
}

void lua__push__dirent(lua_State* L, struct dirent* ent)
{
	struct dirent** pent;
	pent = (struct dirent**)lua_newuserdata(L, sizeof(struct dirent*));
	if (!ent)
	{
		*pent = (struct dirent*)malloc(sizeof(struct dirent));
		memset(*pent, 0, sizeof(struct dirent));
	}
	else
		*pent = ent;
	lua_newtable(L);
	lua_pushcfunction(L, lua__dirent__index);
	lua_setfield(L, -2, "__index");
	lua_pushcfunction(L, lua__dirent__newindex);
	lua_setfield(L, -2, "__newindex");
	if (!ent)
	{
		lua_pushcfunction(L, lua__dirent__gc);
		lua_setfield(L, -2, "__gc");
	}
	lua_setmetatable(L, -2);
}

/****************************************************************************/

static int lua__timeval__index(lua_State* L)
{
	struct timeval** ptime;
	ptime = lua_touserdata(L, 1);
	if (*ptime)
	{
		struct timeval* time = *ptime;
		if (lua_type(L, 2)==LUA_TSTRING)
		{
			const char* k = lua_tostring(L, 2);
			if (!strcmp(k, "sec")) lua_pushnumber(L, time->tv_sec);
			else if (!strcmp(k, "usec")) lua_pushnumber(L, time->tv_usec);
			else lua_pushnil(L);
			return 1;
		}
	}
	lua_pushnil(L);
	return 1;
}

static int lua__timeval__newindex(lua_State* L)
{
	struct timeval** ptime;
	ptime = lua_touserdata(L, 1);
	if (*ptime)
	{
		struct timeval* time = *ptime;
		if (lua_type(L, 2)==LUA_TSTRING)
		{
			const char* k = lua_tostring(L, 2);
			if (!strcmp(k, "sec")) { if (lua_isnumber(L, 3)) time->tv_sec = lua_tonumber(L, 3); }
			else if (!strcmp(k, "usec")) { if (lua_isnumber(L, 3)) time->tv_usec = lua_tonumber(L, 3); }
		}
	}
	return 0;
}

static int lua__timeval__gc(lua_State* L)
{
	struct timeval** ptime;
	ptime = lua_touserdata(L, 1);
	if (*ptime)
	{
		free(*ptime);
		*ptime = 0;
	}
	return 0;
}

void lua__push__timeval(lua_State* L, struct timeval* time)
{
	struct timeval** ptime;
	ptime = (struct timeval**)lua_newuserdata(L, sizeof(struct timeval*));
	if (!time)
	{
		*ptime = (struct timeval*)malloc(sizeof(struct timeval));
		memset(*ptime, 0, sizeof(struct timeval));
	}
	else
		*ptime = time;
	lua_newtable(L);
	lua_pushcfunction(L, lua__timeval__index);
	lua_setfield(L, -2, "__index");
	lua_pushcfunction(L, lua__timeval__newindex);
	lua_setfield(L, -2, "__newindex");
	if (!time)
	{
		lua_pushcfunction(L, lua__timeval__gc);
		lua_setfield(L, -2, "__gc");
	}
	lua_setmetatable(L, -2);
}

static int lua__timeval_array__index(lua_State* L)
{
	if (lua_isnumber(L, 2))
	{
		size_t size;
		size_t index;
		
		lua_getmetatable(L, 1);
		lua_getfield(L, -1, "size");
		size = lua_tonumber(L, -1);
		lua_pop(L, 2);
		index = lua_tonumber(L, 2) - 1;
		
		if (index<size)
		{
			struct timeval* times;
			times = *(struct timeval**)lua_touserdata(L, 1);
			lua__push__timeval(L, times+index);
			lua_createtable(L, 0, 1);
			lua_pushvalue(L, 1);
			lua_setfield(L, -2, "array");
			setuservalue(L, -2);
			return 1;
		}
		else
			lua_pushnil(L);
	}
	else
		lua_pushnil(L);
	return 1;
}

void lua__push__timeval_array(lua_State* L, struct timeval* times, size_t size)
{
	struct timeval** ptimes;
	ptimes = (struct timeval**)lua_newuserdata(L, sizeof(struct timeval*));
	if (!times)
	{
		*ptimes = (struct timeval*)malloc(sizeof(struct timeval) * size);
		memset(*ptimes, 0, sizeof(struct timeval) * size);
	}
	else
		*ptimes = times;
	lua_newtable(L);
	lua_pushnumber(L, size);
	lua_setfield(L, -2, "size");
	lua_pushcfunction(L, lua__timeval_array__index);
	lua_setfield(L, -2, "__index");
	if (!times)
	{
		lua_pushcfunction(L, lua__timeval__gc);
		lua_setfield(L, -2, "__gc");
	}
	lua_setmetatable(L, -2);
}

void lua__push__timespec(lua_State* L, const struct timespec* tv)
{
	lua_createtable(L, 0, 2);
	lua_pushnumber(L, tv->tv_sec); lua_setfield(L, -2, "sec");
	lua_pushnumber(L, tv->tv_nsec); lua_setfield(L, -2, "nsec");
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
