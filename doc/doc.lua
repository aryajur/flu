
------------------------------------------------------------------------------

readme '../README.md'
index {
	title = 'Flu',
	header = [[Filesystems in Lua Userspace]],
	logo = { alt = 'Flu' },
	index = {
		{page='index', title="home"},
		{page='index', section='download', title="download"},
		{page='index', section='installation', title="installation"},
		{page='manual', title="manual"},
		{page='examples', title="examples"},
	},
}

------------------------------------------------------------------------------

header('index')

chapter('about', "About Flu", [[
Flu is a Lua binding for [FUSE](http://fuse.sourceforge.net/), which is a library allowing creation of filesystem drivers run in userspace. Flu is a high level binding, using basic Lua types rather than userdata whenever possible.

Flu tries to be complete, but is not supporting obsolete APIs. The binding is closely following the FUSE API, so in most case you can use FUSE documentation if the present page is not clear enough. The missing functions are missing because I've not used them yet. I can add them on request (a use case could be helpful for non-trivial ones).

## Support

All support is done through the [Lua mailing list](http://www.lua.org/lua-l.html).

Feel free to ask for further developments. I can't guarantee that I'll develop everything you ask, but I want my code to be as useful as possible, so I'll do my best to help you. You can also send me request or bug reports (for code and documentation) directly at [jerome.vuarand@gmail.com](mailto:jerome.vuarand@gmail.com).

## Credits

This module is written and maintained by [Jérôme Vuarand](mailto:jerome.vuarand@gmail.com). It is a fork of [LUSE](http://piratery.net/luse/), a previous Lua-FUSE binding from the same author. LUSE was low level and rather complicated to use, while Flu tries its best to be simple.

Flu is available under a [MIT-style license](LICENSE.txt).
]])

chapter('download', "Download", [[
Flu sources are available in its [Mercurial repository](http://hg.piratery.net/flu/):

    hg clone http://hg.piratery.net/flu

Tarballs of the latest code can be downloaded directly from there: as [gz](http://hg.piratery.net/flu/get/tip.tar.gz), [bz2](http://hg.piratery.net/flu/get/tip.tar.bz2) or [zip](http://hg.piratery.net/flu/get/tip.zip).

Finally, I published some rockspecs:

    luarocks install flu
]])

chapter('installation', "Installation", [[
To build Flu from sources, edit the Makefile and then run make in the top directory:

    $ vi Makefile
    $ make
    $ sudo make install

Flu obviously depends on FUSE. It currently supports FUSE API versions 25 (FUSE 2.5.x and later) and 26 (FUSE 2.6.x and later). You can select a FUSE API version in the Makefile.
]])

footer()

------------------------------------------------------------------------------

header('manual')

local manual = [[
Here you can find a list of the functions present in the module and how to use them. Flu main module follows Lua package system, see the [Lua 5.1 manual](http://www.lua.org/manual/5.1/) or [Lua 5.2 manual](http://www.lua.org/manual/5.2/) for further explanations.

Quick links:
]]

local functions = {
	{
		name = "flu_functions";
		title = "flu module";
		doc = "The `flu` module has very few functions, most of the magic happens inside the `fs` table passed to [`flu.main`](#flu.main).";
		functions = {
			{
				name = "flu.main";
				parameters = {"argv", "fs"};
				doc = [[
Starts a new filesystem daemon. `argv` is an array containing additionnal parameters to pass to the FUSE library. `fs` is a table (or any indexable object) containing methods of the FUSE filesystem you are trying to create.

    local luafs = {}
    function luafs.getattr(path, stat)
		error(flu.errno.ENOSYS)
	end
    local argv = {"luafs", select(2, ...)}
    flu.main(argv, luafs)
]];
			},
			{
				name = "flu.get_context";
				parameters = {"[tbl]"};
				doc = [[Obtain information about the context of the current FUSE request. A table is returned containing fields `uid`, `gid` and `pid` (user, group and thread id of the calling process, respectively). If no arguments are given, a new table is created. If a table is passed as an argument, it is reused and fields are set in it instead.]];
			}
		}
	},
	{
		name = "fuse_functions";
		title = "Filesystem methods";
		doc = [[These methods may be present in the `fs` object passed to [`flu.main`](#flu.main). They are all optional, though without them your filesystem may not work properly. See example filesystems available in Flu packages for minimal requirements.

Below each functon is described with the parameters they are called with, and the expected return values on normal operation. Most functions though have no return value. To express error conditions, regular Lua error should be thrown with `error`. The thrown error though have to be errno userdata values as found in the flu.errno table. For example:

    error(flu.errno.EINVAL)

They are named methods here, but they are called as Lua function, without a self parameter. To maintain state, use upvalues.

As of FUSE 2.6, there are six FUSE methods that are not bound by Flu: and `init`, `destroy`, `lock` and `bmap` which I haven't used yet and are a bit complicated to bind, and `getdir` and `utime` which are obsolete.]];
		functions = {
			{
				name = "fs.getattr";
				parameters = {"path"};
				results = {"stat"};
				doc = [[
Get file attributes. `stat` should be a table, with some of the following fields set:

- `dev`, number, ID of device containing file
- `ino`, number, inode number
- `mode`, a set or a string, file type and permissions. If mode is a set, it's made of the recognized mode flags listed below. If it's a string, it's one of the predefined modes described below.
- `nlink`, number, number of hard links
- `uid`, number, user ID of owner
- `gid`, number, group ID of owner
- `rdev`, number, device ID (if special file)
- `access`, number, time of last access
- `modification`, number, time of last modification
- `change`, number, time of last status change
- `size`, number, total size, in bytes
- `blocks`, number, number of blocks allocated
- `blksize`, number, blocksize for filesystem I/O

This table is compatible with LuaFileSystem, though the default access rights for `mode` strings (as described below) may not suit you.

The `dev` and `blksize` fields are ignored. The `ino` field is ignored except if the `use_ino` mount option is given.

Note that for the *find* command line utility to work in your filesystem, the `nlink` field must be properly filled. See [this FUSE FAQ entry](http://fuse.sourceforge.net/wiki/index.php/FAQ#Why_doesnx27.t_find_work_on_my_filesystemx3f.) for more details.

Mode flags:

- `blk`, the file is a block device
- `chr`, the file is a character device
- `fifo`, the file is a FIFO
- `reg`, the file is a regular file
- `dir`, the file is a directory
- `lnk`, the file is a symbolic link
- `sock`, the file is a socket
- `rusr`, owner has read permission
- `wusr`, owner has write permission
- `xusr`, owner has execute permission
- `rgrp`, group has read permission
- `wgrp`, group has write permission
- `xgrp`, group has execute permission
- `roth`, others have read permission
- `woth`, others have write permission
- `xoth`, others have execute permission
- `suid`, set UID bit
- `sgid`, set-group-ID bit
- `svtx`, sticky bit

Predefined modes:

- `'file'`, equivalent to `reg`, `rusr`, `wusr`, `rgrp` and `wgrp`
- `'directory'`, equivalent to `dir`, `rusr`, `wusr`, `xusr`, `rgrp`, `wgrp` and `xgrp`
- `'link'`, equivalent to `lnk`, `rusr`, `wusr`, `rgrp` and `wgrp`
- `'socket'`, equivalent to `sock`, `rusr`, `wusr`, `rgrp` and `wgrp`
- `'named pipe'`, equivalent to `fifo`, `rusr`, `wusr`, `rgrp` and `wgrp`
- `'char device'`, equivalent to `chr`, `rusr`, `wusr`, `rgrp` and `wgrp`
- `'block device'`, equivalent to `blk`, `rusr`, `wusr`, `rgrp` and `wgrp`
- `'other'`, equivalent to an empty set
]];
			},
			{
				name = "fs.readlink";
				parameters = {"path", "size"};
				results = {"target"};
				doc = [[
Read the target of a symbolic link. The target should be returned as a string, which size should not exceed `size`.

    function luafs.readlink(path, size)
        return "/foo/"..path
    end
]];
			},
			{
				name = "fs.mknod";
				parameters = {"path", "mode", "rdev"};
				doc = [[Create a file node. This is called for creation of all non-directory, non-symlink nodes. If the filesystem defines a `create` method, then for regular files that will be called instead. `mode` is a set, see [`fs.getattr`](#fs.getattr) for a description of the recognized flags.]];
			},
			{
				name = "fs.mkdir";
				parameters = {"path", "mode"};
				doc = [[Create a directory. `mode` is a set, see [`fs.getattr`](#fs.getattr) for a description of the recognized flags.]];
			},
			{
				name = "fs.unlink";
				parameters = {"path"};
				doc = [[Remove a file.]];
			},
			{
				name = "fs.rmdir";
				parameters = {"path"};
				doc = [[Remove a directory.]];
			},
			{
				name = "fs.symlink";
				parameters = {"linkname", "path"};
				doc = [[Create a symbolic link. `path` is the name of the link itself in the Flu filesystem, `linkname` is the linked file.]];
			},
			{
				name = "fs.rename";
				parameters = {"oldpath", "newpath"};
				doc = [[Rename a file.]];
			},
			{
				name = "fs.link";
				parameters = {"oldpath", "newpath"};
				doc = [[Create a hard link to a file.]];
			},
			{
				name = "fs.chmod";
				parameters = {"path", "mode"};
				doc = [[Change the permission bits of a file. `mode` is a set, see [`fs.getattr`](#fs.getattr) for a description of the recognized flags.]];
			},
			{
				name = "fs.chown";
				parameters = {"path", "uid", "gid"};
				doc = [[Change the owner and group of a file. `uid` and `gid` are numbers.]];
			},
			{
				name = "fs.truncate";
				parameters = {"path", "size"};
				doc = [[Change the size of a file.]];
			},
			{
				name = "fs.open";
				parameters = {"path", "fi"};
				doc = [[
File open operation. `fi` is a `fuse_file_info` structure (see below). No creation, or truncation flags (`creat`, `excl`, `trunc`) will be passed to [`fs.open`](#fs.open). Open should check if the operation is permitted for the given flags. Optionally open may also return an arbitrary file handle in the `fh` field of the `fuse_file_info` structure, which will be passed to all subsequent operations on that file.

The `fuse_file_info` structure has the following members:

- `flags`, set, with the following flags recognized: `append`, `async`, `creat`, `direct`, `directory`, `excl`, `largefile`, `noatime`, `noctty`, `nofollow`, `nonblock`, `ndelay`, `sync`, `trunc`, `rdwr`, `rdonly`, `wronly`
- `writepage`, number
- `direct_io`, boolean
- `keep_cache`, boolean
- `flush`, boolean
- `fh`, number, the file handle that the filesystem can write to
- `lock_owner`, number
]];
			},
			{
				name = "fs.read";
				parameters = {"path", "size", "offset", "fi"};
				results = {"data"},
				doc = [[
Read data from an open file. `size` is the amount of data required in bytes. `offset` is a position inside the file. See [`fs.open`](#fs.open) for a description of `fi`.

Read should return a string of `size` length exactly except on end of file or on error, otherwise the rest of the data will be substituted with zeroes. An exception to this is when the `direct_io` mount option is specified, in which case the return value of the read system call will reflect the return value of this operation.]];
			},
			{
				name = "fs.write";
				parameters = {"path", "buf", "offset", "fi"};
				doc = [[
Write data to an open file. `buf` is a string containing the written data. `offset` is a position inside the file. See [`fs.open`](#fs.open) for a description of `fi`.

Write should return exactly the number of bytes requested except on error. An exception to this is when the `direct_io` mount option is specified (see read operation).]];
			},
			{
				name = "fs.statfs";
				parameters = {"path"};
				results = {"stat"};
				doc = [[
Get file system statistics. `stat` should be a table, with some of the following fields set:

- `bsize`, number, file system block size
- `frsize`, number, fragment size
- `blocks`, number, size of fs in frsize units
- `bfree`, number, # free blocks
- `bavail`, number, # free blocks for non-root
- `files`, number, # inodes
- `ffree`, number, # free inodes
- `favail`, number, # free inodes for non-root
- `fsid`, number, file system ID
- `flag`, table, mount flags as a Lua set, with the following recognized flags: `rdonly`, `nosuid`.
- `namemax`, number, maximum filename length

The `frsize`, `favail`, `fsid` and `flag` fields are ignored.]];
			},
			{
				name = "fs.flush";
				parameters = {"path", "fi"};
				doc = [[
Possibly flush cached data. See [`fs.open`](#fs.open) for a description of `fi`.

BIG NOTE: This is not equivalent to fsync(). It's not a request to sync dirty data.

Flush is called on each [`fs.close`](#fs.close) of a file descriptor. So if a filesystem wants to return write errors in [`fs.close`](#fs.close) and the file has cached dirty data, this is a good place to write back data and raise any errors. Since many applications ignore [`fs.close`](#fs.close) errors this is not always useful.

NOTE: The [`fs.flush`](#fs.flush) method may be called more than once for each [`fs.open`](#fs.open). This happens if more than one file descriptor refers to an opened file due to dup(), dup2() or fork() calls. It is not possible to determine if a flush is final, so each flush should be treated equally. Multiple write-flush sequences are relatively rare, so this shouldn't be a problem.

Filesystems shouldn't assume that [`fs.flush`](#fs.flush) will always be called after some writes, or that it will be called at all.]];
			},
			{
				name = "fs.release";
				parameters = {"path", "fi"};
				doc = [[
Release an open file. See [`fs.open`](#fs.open) for a description of `fi`.

Release is called when there are no more references to an open file: all file descriptors are closed and all memory mappings are unmapped.

For every open() call there will be exactly one release() call with the same flags and file descriptor. It is possible to have a file opened more than once, in which case only the last release will mean, that no more reads/writes will happen on the file. Errors in release are ignored.]];
			},
			{
				name = "fs.fsync";
				parameters = {"path", "datasync", "fi"};
				doc = [[Synchronize file contents. If the `datasync` parameter is `true`, then only the user data should be flushed, not the meta data. See [`fs.open`](#fs.open) for a description of `fi`.]];
			},
			{
				name = "fs.setxattr";
				parameters = {"path", "name", "value", "flags"};
				doc = [[Set an extended attribute on the file. `flags` is a set with the following recognized flags: `create`, `replace`. If `create` is set the method should throw an error if the attribute exists. If `replace` the method should throw when the attribute doesn't exist.]];
			},
			{
				name = "fs.getxattr";
				parameters = {"path", "name"};
				results = {"value"};
				doc = [[Get an extended attribute. If the attribute doesn't exist `ENODATA` should be thrown.]];
			},
			{
				name = "fs.listxattr";
				parameters = {"path"};
				results = {"list"};
				doc = [[Return a list of extended attribute names. `list` should be an array of strings.]];
			},
			{
				name = "fs.removexattr";
				parameters = {"path", "name"};
				doc = [[Remove an extended attribute. If the attribute doesn't exist `ENODATA` should be thrown.]];
			},
			{
				name = "fs.opendir";
				parameters = {"path", "fi"};
				doc = [[Open directory. This method should check if the open operation is permitted for this directory. See [`fs.open`](#fs.open) for a description of `fi`.]];
			},
			{
				name = "fs.readdir";
				parameters = {"path", "filler", "fi"};
				doc = [[Read directory. `filler` is a function with the prototype `filler(name)` that must be called for each directory entry, and that will return `false` when the output buffer is full, `true` otherwise. See [`fs.open`](#fs.open) for a description of `fi`.]];
			},
			{
				name = "fs.releasedir";
				parameters = {"path", "fi"};
				doc = [[Release a directory. See [`fs.open`](#fs.open) for a description of `fi`.]];
			},
			{
				name = "fs.fsyncdir";
				parameters = {"path", "datasync", "fi"};
				doc = [[Synchronize directory contents. If the `datasync` parameter is `true`, then only the user data should be flushed, not the meta data. See [`fs.open`](#fs.open) for a description of `fi`.]];
			},
			{
			--[=[
				name = "fs.init";
				parameters = {};
				doc = [[:TODO: Initialize filesystem. The return value will passed in the private\_data field of fuse\_context to all file operations and as a parameter to the fs.destroy method.]];
			},{
				name = "fs.destroy";
				parameters = {"conn"};
				doc = [[:TODO: Clean up filesystem. Called on filesystem exit.]];
			},{
			--]=]
				name = "fs.access";
				parameters = {"path", "mask"};
				doc = [[
Check file access permissions. This will be called for the `access()` system call. If the `'default_permissions'` mount option is given, this method is not called.

This method is not called under Linux kernel versions 2.4.x.]];
			},
			{
				name = "fs.create";
				parameters = {"path", "mode", "fi"};
				doc = [[
Create and open a file. `mode` is a set, see [`fs.getattr`](#fs.getattr) for a description of the recognized flags. See [`fs.open`](#fs.open) for a description of `fi`.

If the file does not exist, first create it with the specified `mode`, and then open it.

If this method is not implemented or under Linux kernel versions earlier than 2.6.15, the mknod() and open() methods will be called instead.]];
			},
			{
				name = "fs.ftruncate";
				parameters = {"path", "size", "fi"};
				doc = [[
Change the size of an open file. See [`fs.open`](#fs.open) for a description of `fi`.

This method is called instead of the [`fs.truncate`](#fs.truncate) method if the truncation was invoked from an `ftruncate()` system call.

If this method is not implemented or under Linux kernel versions earlier than 2.6.15, the [`fs.truncate`](#fs.truncate) method will be called instead.]];
			},
			{
				name = "fs.fgetattr";
				parameters = {"path", "st", "fi"};
				doc = [[
Get attributes from an open file.

This method is called instead of the [`fs.getattr`](#fs.getattr) method if the file information is available.

Currently this is only called after the [`fs.create`](#fs.create) method if that is implemented (see above). Later it may be called for invocations of `fstat()` too.]];
			--[=[
			},{
				name = "fs.lock";
				parameters = {"path", "fi", "cmd", "lock"};
				doc = [[:TODO:]];
			--]=]
			},
			{
				name = "fs.utimens";
				parameters = {"path", "access", "modification"};
				doc = [[
Change the access and modification times of a file with nanosecond resolution. `access` and `modification` are Lua tables with two fields each:

- `sec`, number, time in seconds
- `nsec`, number, the sub-second portion of the time, in nanoseconds
]];
			--[=[
			},{
				name = "fs.bmap";
				parameters = {};
				doc = [[:TODO:]];
			--]=]
			}
		}
	},
}

local function stitle(sectionid, section)
	return "%chapterid%."..tostring(sectionid).." - "..section.title
end

local function ftitle(func)
	local title = ""
	if func.results then
		title = title..table.concat(func.results, ", ").." = "
	end
	title = title..func.name
	if func.parameters then
		title = title..' ('..table.concat(func.parameters, ", ")..")"
	end
	return title
end

local hmanual = markdown(manual)
manual = manual..'\n'
hmanual = hmanual..[[
<ul>
]]
for sectionid,section in ipairs(functions) do
	local title = stitle(sectionid, section)
	local a = 'markdown-header-'..title:gsub('[^%w%s_%%]', ''):match('^%s*(.-)%s*$'):gsub('%s+', '-'):lower()
	manual = manual..'- ['..section.title..'](#'..a..')\n'
	hmanual = hmanual..[[
    <li><a href="#]]..section.name..[[">]]..section.title..[[</a>
    <ul>
]]
	local names = {}
	for _,func in ipairs(section.functions) do
		table.insert(names, func)
	end
	table.sort(names, function(a, b) return a.name < b.name end)
	for _,func in ipairs(names) do
		local title = ftitle(func)
		local a = 'markdown-header-'..title:gsub('[^%w%s_]', ''):match('^%s*(.-)%s*$'):gsub('%s+', '-'):lower()
		manual = manual..'    - ['..func.name..'](#'..a..')\n'
		hmanual = hmanual..[[
        <li><a href="#]]..func.name..[[">]]..func.name..[[</a></li>
]]
	end
	hmanual = hmanual..[[
    </ul></li>
]]
end
manual = manual..'\n'
hmanual = hmanual..[[
</ul>
]]

for sectionid,section in ipairs(functions) do
	local title = stitle(sectionid, section)
	manual = manual..'---\n\n## '..title..'\n\n'..section.doc..'\n\n'
	hmanual = hmanual..[[
	<div class="section">
	<h2><a name="]]..section.name..[[">]]..title..[[</a></h2>
]]..markdown(section.doc)..[[

]]
	for _,func in ipairs(section.functions) do
		local title = ftitle(func)
		manual = manual..'---\n\n### `'..title..'`\n\n'..func.doc..'\n\n'
		hmanual = hmanual..[[
		<div class="function">
		<h3><a name="]]..func.name..[["><code>]]..title..[[</code></a></h3>
]]..markdown(func.doc)..[[
		</div>

]]
	end
	hmanual = hmanual..[[
	</div>

]]
end

chapter('manual', "Manual", manual, nil, hmanual)

footer()

------------------------------------------------------------------------------

header('examples')

chapter('examples', "Examples", [[
Here are some filesystem examples. [`hellofs`](#hellofs) is just a minimal example. [`luafs`](#luafs) is a basic filesystem that exposes a Lua table as a directory. It can be used as a model to expose some data from a Lua state in your own applications.]]--[[ [`fwfs`](#fwfs) is a forwarding filesystem, that can be used as a base to make on the fly operations on file I/O.
]], { [[
## %chapterid%.1 - hellofs

`hellofs` is the Hello World! of FUSE filesystems. It creates a directory with a single file called `hello` that contain the string `"Hello World!"`.

Example:

    $ mkdir tmpdir
    $ ./hellofs.lua tmpdir
    $ ls tmpdir
    hello
    $ cat tmpdir/hello
    Hello World!
    $ fusermount -u tmpdir

Source code: [hellofs.lua](http://piratery.net/flu/hellofs.lua)

]], [[
## %chapterid%.2 - luafs

`luafs` expose a table as a directory. The subtables are exposed as subdirectories, while the string fields are exposed as regular files. You can create new files, and write to them: that will create new strings in the table hierarchy.

Example:

    $ mkdir tmpdir
    $ ./luafs.lua tmpdir
    $ ls tmpdir
    $ echo Hello World! > tmpdir/hello
    $ ls tmpdir
    hello
    $ cat tmpdir/hello
    Hello World!
    $ mkdir tmpdir/subdir
    $ ls tmpdir
    hello subdir/
    $ echo foo > tmpdir/subdir/bar
    $ ls tmpdir/subdir
    bar
    $ cat tmpdir/subdir/bar
    foo
    $ fusermount -u tmpdir

Source code: [luafs.lua](http://piratery.net/flu/luafs.lua)

]], --[[
## %chapterid%.3 - fwfs

`fwfs` creates a directory that will forward all I/O to another directory. It's meant to be used as a base for any filesystem that is backed on disk, and that is doing something to the files on the fly. For example it can do filesystem encryption, it can make several directories on different disks appear as a single one, etc.

Example:
    $ mkdir srcdir
    $ echo Hello World! > srcdir/hello
    $ mkdir dstdir
    $ ./fwfs.lua srcdir dstdir
    $ ls dstdir
    hello
    $ cat dstdir/hello
    Hello World!
    $ mkdir dstdir/subdir
    $ ls srcdir
    hello subdir/
    $ echo foo > dstdir/subdir/bar
    $ ls srcdir/subdir
    bar
    $ cat srcdir/subdir/bar
    foo
    $ fusermount -u dstdir

Source code: [fwfs.lua](http://piratery.net/flu/fwfs.lua)
]]})

footer()

------------------------------------------------------------------------------

--[[
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
]]

-- vi: ts=4 sts=4 sw=4
