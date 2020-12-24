This fork takes from the original [Mercurial repository](http://hg.piratery.net/flu/) and made it compatible with the latest version of [libfuse](https://github.com/libfuse/libfuse)
It also adds a Code:Blocks project for easy compilation

# 1 - About Flu

Flu is a Lua binding for [FUSE](http://fuse.sourceforge.net/), which is a library allowing creation of filesystem drivers run in userspace. Flu is a high level binding, using basic Lua types rather than userdata whenever possible.

Flu tries to be complete, but is not supporting obsolete APIs. The binding is closely following the FUSE API, so in most case you can use FUSE documentation if the present page is not clear enough. The missing functions are missing because I've not used them yet. I can add them on request (a use case could be helpful for non-trivial ones).

## Support

All support is done through the [Lua mailing list](http://www.lua.org/lua-l.html).

Feel free to ask for further developments. I can't guarantee that I'll develop everything you ask, but I want my code to be as useful as possible, so I'll do my best to help you. You can also send me request or bug reports (for code and documentation) directly at [jerome.vuarand@gmail.com](mailto:jerome.vuarand@gmail.com).


## Credits

This module is written and maintained by [Jérôme Vuarand](mailto:jerome.vuarand@gmail.com). It is a fork of [LUSE](http://piratery.net/luse/), a previous Lua-FUSE binding from the same author. LUSE was low level and rather complicated to use, while Flu tries its best to be simple.

Flu is available under a [MIT-style license](LICENSE.txt).

# 2 - Download

Flu sources are available in its [Mercurial repository](http://hg.piratery.net/flu/):

    hg clone http://hg.piratery.net/flu

Tarballs of the latest code can be downloaded directly from there: as [gz](http://hg.piratery.net/flu/get/tip.tar.gz), [bz2](http://hg.piratery.net/flu/get/tip.tar.bz2) or [zip](http://hg.piratery.net/flu/get/tip.zip).

Finally, I published some rockspecs:

    luarocks install flu

# 3 - Installation

To build Flu from sources, edit the Makefile and then run make in the top directory:

    $ vi Makefile
    $ make
    $ sudo make install

Flu obviously depends on FUSE. It currently supports FUSE API versions 25 (FUSE 2.5.x and later) and 26 (FUSE 2.6.x and later). You can select a FUSE API version in the Makefile.

# 4 - Manual

Here you can find a list of the functions present in the module and how to use them. Flu main module follows Lua package system, see the [Lua 5.1 manual](http://www.lua.org/manual/5.1/) or [Lua 5.2 manual](http://www.lua.org/manual/5.2/) for further explanations.

Quick links:

- [flu module](#markdown-header-41-flu-module)
    - [flu.get_context](#markdown-header-fluget_context-tbl)
    - [flu.main](#markdown-header-flumain-argv-fs)
- [Filesystem methods](#markdown-header-42-filesystem-methods)
    - [fs.access](#markdown-header-fsaccess-path-mask)
    - [fs.chmod](#markdown-header-fschmod-path-mode)
    - [fs.chown](#markdown-header-fschown-path-uid-gid)
    - [fs.create](#markdown-header-fscreate-path-mode-fi)
    - [fs.fgetattr](#markdown-header-fsfgetattr-path-st-fi)
    - [fs.flush](#markdown-header-fsflush-path-fi)
    - [fs.fsync](#markdown-header-fsfsync-path-datasync-fi)
    - [fs.fsyncdir](#markdown-header-fsfsyncdir-path-datasync-fi)
    - [fs.ftruncate](#markdown-header-fsftruncate-path-size-fi)
    - [fs.getattr](#markdown-header-stat-fsgetattr-path)
    - [fs.getxattr](#markdown-header-value-fsgetxattr-path-name)
    - [fs.link](#markdown-header-fslink-oldpath-newpath)
    - [fs.listxattr](#markdown-header-list-fslistxattr-path)
    - [fs.mkdir](#markdown-header-fsmkdir-path-mode)
    - [fs.mknod](#markdown-header-fsmknod-path-mode-rdev)
    - [fs.open](#markdown-header-fsopen-path-fi)
    - [fs.opendir](#markdown-header-fsopendir-path-fi)
    - [fs.read](#markdown-header-data-fsread-path-size-offset-fi)
    - [fs.readdir](#markdown-header-fsreaddir-path-filler-fi)
    - [fs.readlink](#markdown-header-target-fsreadlink-path-size)
    - [fs.release](#markdown-header-fsrelease-path-fi)
    - [fs.releasedir](#markdown-header-fsreleasedir-path-fi)
    - [fs.removexattr](#markdown-header-fsremovexattr-path-name)
    - [fs.rename](#markdown-header-fsrename-oldpath-newpath)
    - [fs.rmdir](#markdown-header-fsrmdir-path)
    - [fs.setxattr](#markdown-header-fssetxattr-path-name-value-flags)
    - [fs.statfs](#markdown-header-stat-fsstatfs-path)
    - [fs.symlink](#markdown-header-fssymlink-linkname-path)
    - [fs.truncate](#markdown-header-fstruncate-path-size)
    - [fs.unlink](#markdown-header-fsunlink-path)
    - [fs.utimens](#markdown-header-fsutimens-path-access-modification)
    - [fs.write](#markdown-header-fswrite-path-buf-offset-fi)

---

## 4.1 - flu module

The `flu` module has very few functions, most of the magic happens inside the `fs` table passed to [`flu.main`](#flu.main).

---

### `flu.main (argv, fs)`

Starts a new filesystem daemon. `argv` is an array containing additionnal parameters to pass to the FUSE library. `fs` is a table (or any indexable object) containing methods of the FUSE filesystem you are trying to create.

    local luafs = {}
    function luafs.getattr(path, stat)
		error(flu.errno.ENOSYS)
	end
    local argv = {"luafs", select(2, ...)}
    flu.main(argv, luafs)


---

### `flu.get_context ([tbl])`

Obtain information about the context of the current FUSE request. A table is returned containing fields `uid`, `gid` and `pid` (user, group and thread id of the calling process, respectively). If no arguments are given, a new table is created. If a table is passed as an argument, it is reused and fields are set in it instead.

---

## 4.2 - Filesystem methods

These methods may be present in the `fs` object passed to [`flu.main`](#flu.main). They are all optional, though without them your filesystem may not work properly. See example filesystems available in Flu packages for minimal requirements.

Below each functon is described with the parameters they are called with, and the expected return values on normal operation. Most functions though have no return value. To express error conditions, regular Lua error should be thrown with `error`. The thrown error though have to be errno userdata values as found in the flu.errno table. For example:

    error(flu.errno.EINVAL)

They are named methods here, but they are called as Lua function, without a self parameter. To maintain state, use upvalues.

As of FUSE 2.6, there are six FUSE methods that are not bound by Flu: and `init`, `destroy`, `lock` and `bmap` which I haven't used yet and are a bit complicated to bind, and `getdir` and `utime` which are obsolete.

---

### `stat = fs.getattr (path)`

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


---

### `target = fs.readlink (path, size)`

Read the target of a symbolic link. The target should be returned as a string, which size should not exceed `size`.

    function luafs.readlink(path, size)
        return "/foo/"..path
    end


---

### `fs.mknod (path, mode, rdev)`

Create a file node. This is called for creation of all non-directory, non-symlink nodes. If the filesystem defines a `create` method, then for regular files that will be called instead. `mode` is a set, see [`fs.getattr`](#fs.getattr) for a description of the recognized flags.

---

### `fs.mkdir (path, mode)`

Create a directory. `mode` is a set, see [`fs.getattr`](#fs.getattr) for a description of the recognized flags.

---

### `fs.unlink (path)`

Remove a file.

---

### `fs.rmdir (path)`

Remove a directory.

---

### `fs.symlink (linkname, path)`

Create a symbolic link. `path` is the name of the link itself in the Flu filesystem, `linkname` is the linked file.

---

### `fs.rename (oldpath, newpath)`

Rename a file.

---

### `fs.link (oldpath, newpath)`

Create a hard link to a file.

---

### `fs.chmod (path, mode)`

Change the permission bits of a file. `mode` is a set, see [`fs.getattr`](#fs.getattr) for a description of the recognized flags.

---

### `fs.chown (path, uid, gid)`

Change the owner and group of a file. `uid` and `gid` are numbers.

---

### `fs.truncate (path, size)`

Change the size of a file.

---

### `fs.open (path, fi)`

File open operation. `fi` is a `fuse_file_info` structure (see below). No creation, or truncation flags (`creat`, `excl`, `trunc`) will be passed to [`fs.open`](#fs.open). Open should check if the operation is permitted for the given flags. Optionally open may also return an arbitrary file handle in the `fh` field of the `fuse_file_info` structure, which will be passed to all subsequent operations on that file.

The `fuse_file_info` structure has the following members:

- `flags`, set, with the following flags recognized: `append`, `async`, `creat`, `direct`, `directory`, `excl`, `largefile`, `noatime`, `noctty`, `nofollow`, `nonblock`, `ndelay`, `sync`, `trunc`, `rdwr`, `rdonly`, `wronly`
- `writepage`, number
- `direct_io`, boolean
- `keep_cache`, boolean
- `flush`, boolean
- `fh`, number, the file handle that the filesystem can write to
- `lock_owner`, number


---

### `data = fs.read (path, size, offset, fi)`

Read data from an open file. `size` is the amount of data required in bytes. `offset` is a position inside the file. See [`fs.open`](#fs.open) for a description of `fi`.

Read should return a string of `size` length exactly except on end of file or on error, otherwise the rest of the data will be substituted with zeroes. An exception to this is when the `direct_io` mount option is specified, in which case the return value of the read system call will reflect the return value of this operation.

---

### `fs.write (path, buf, offset, fi)`

Write data to an open file. `buf` is a string containing the written data. `offset` is a position inside the file. See [`fs.open`](#fs.open) for a description of `fi`.

Write should return exactly the number of bytes requested except on error. An exception to this is when the `direct_io` mount option is specified (see read operation).

---

### `stat = fs.statfs (path)`

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

The `frsize`, `favail`, `fsid` and `flag` fields are ignored.

---

### `fs.flush (path, fi)`

Possibly flush cached data. See [`fs.open`](#fs.open) for a description of `fi`.

BIG NOTE: This is not equivalent to fsync(). It's not a request to sync dirty data.

Flush is called on each [`fs.close`](#fs.close) of a file descriptor. So if a filesystem wants to return write errors in [`fs.close`](#fs.close) and the file has cached dirty data, this is a good place to write back data and raise any errors. Since many applications ignore [`fs.close`](#fs.close) errors this is not always useful.

NOTE: The [`fs.flush`](#fs.flush) method may be called more than once for each [`fs.open`](#fs.open). This happens if more than one file descriptor refers to an opened file due to dup(), dup2() or fork() calls. It is not possible to determine if a flush is final, so each flush should be treated equally. Multiple write-flush sequences are relatively rare, so this shouldn't be a problem.

Filesystems shouldn't assume that [`fs.flush`](#fs.flush) will always be called after some writes, or that it will be called at all.

---

### `fs.release (path, fi)`

Release an open file. See [`fs.open`](#fs.open) for a description of `fi`.

Release is called when there are no more references to an open file: all file descriptors are closed and all memory mappings are unmapped.

For every open() call there will be exactly one release() call with the same flags and file descriptor. It is possible to have a file opened more than once, in which case only the last release will mean, that no more reads/writes will happen on the file. Errors in release are ignored.

---

### `fs.fsync (path, datasync, fi)`

Synchronize file contents. If the `datasync` parameter is `true`, then only the user data should be flushed, not the meta data. See [`fs.open`](#fs.open) for a description of `fi`.

---

### `fs.setxattr (path, name, value, flags)`

Set an extended attribute on the file. `flags` is a set with the following recognized flags: `create`, `replace`. If `create` is set the method should throw an error if the attribute exists. If `replace` the method should throw when the attribute doesn't exist.

---

### `value = fs.getxattr (path, name)`

Get an extended attribute. If the attribute doesn't exist `ENODATA` should be thrown.

---

### `list = fs.listxattr (path)`

Return a list of extended attribute names. `list` should be an array of strings.

---

### `fs.removexattr (path, name)`

Remove an extended attribute. If the attribute doesn't exist `ENODATA` should be thrown.

---

### `fs.opendir (path, fi)`

Open directory. This method should check if the open operation is permitted for this directory. See [`fs.open`](#fs.open) for a description of `fi`.

---

### `fs.readdir (path, filler, fi)`

Read directory. `filler` is a function with the prototype `filler(name)` that must be called for each directory entry, and that will return `false` when the output buffer is full, `true` otherwise. See [`fs.open`](#fs.open) for a description of `fi`.

---

### `fs.releasedir (path, fi)`

Release a directory. See [`fs.open`](#fs.open) for a description of `fi`.

---

### `fs.fsyncdir (path, datasync, fi)`

Synchronize directory contents. If the `datasync` parameter is `true`, then only the user data should be flushed, not the meta data. See [`fs.open`](#fs.open) for a description of `fi`.

---

### `fs.access (path, mask)`

Check file access permissions. This will be called for the `access()` system call. If the `'default_permissions'` mount option is given, this method is not called.

This method is not called under Linux kernel versions 2.4.x.

---

### `fs.create (path, mode, fi)`

Create and open a file. `mode` is a set, see [`fs.getattr`](#fs.getattr) for a description of the recognized flags. See [`fs.open`](#fs.open) for a description of `fi`.

If the file does not exist, first create it with the specified `mode`, and then open it.

If this method is not implemented or under Linux kernel versions earlier than 2.6.15, the mknod() and open() methods will be called instead.

---

### `fs.ftruncate (path, size, fi)`

Change the size of an open file. See [`fs.open`](#fs.open) for a description of `fi`.

This method is called instead of the [`fs.truncate`](#fs.truncate) method if the truncation was invoked from an `ftruncate()` system call.

If this method is not implemented or under Linux kernel versions earlier than 2.6.15, the [`fs.truncate`](#fs.truncate) method will be called instead.

---

### `fs.fgetattr (path, st, fi)`

Get attributes from an open file.

This method is called instead of the [`fs.getattr`](#fs.getattr) method if the file information is available.

Currently this is only called after the [`fs.create`](#fs.create) method if that is implemented (see above). Later it may be called for invocations of `fstat()` too.

---

### `fs.utimens (path, access, modification)`

Change the access and modification times of a file with nanosecond resolution. `access` and `modification` are Lua tables with two fields each:

- `sec`, number, time in seconds
- `nsec`, number, the sub-second portion of the time, in nanoseconds



# 5 - Examples

Here are some filesystem examples. [`hellofs`](#hellofs) is just a minimal example. [`luafs`](#luafs) is a basic filesystem that exposes a Lua table as a directory. It can be used as a model to expose some data from a Lua state in your own applications.

## 5.1 - hellofs

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



## 5.2 - luafs

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


