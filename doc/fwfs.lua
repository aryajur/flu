#!/usr/bin/env lua
local fuse = require 'luse'
local udata = require 'luse.userdata'
local errno = require 'luse.errno'
local pio = require 'luse.posixio'

local red = "\27[31m"
local green = "\27[32m"
local yellow = "\27[33m"
local clean = "\27[0m"

local verbose = false

function info(msg)
	if verbose then
		print(green.."(!) "..msg..clean)
	end
end

function warning(msg)
	if verbose then
		print(yellow.."/!\\ "..msg..clean)
	end
end

function danger(msg)
	if verbose then
		print(red.."[!] "..msg..clean)
	end
end

local function mkset(array)
	local set = {}
	for _,flag in ipairs(array) do
		set[flag] = true
	end
	return set
end

local root = assert((...), "no root directory specified")

-- when the process is detached from the console, its directory changed, so we
-- make sure root is an absolute path
if root:sub(1,1)~='/' then
	local buf = udata.new(pio.PATH_MAX)
	assert(pio.getcwd(buf.data, buf.size)==buf.data)
	buf = tostring(buf)
	buf = buf:sub(1, buf:find('%z')-1)
	root = tostring(buf).."/"..root
	buf = nil
end

local fwfs = {}

local fields = {'dev', 'ino', 'mode', 'nlink', 'uid', 'gid', 'rdev', 'size', 'atime', 'mtime', 'ctime', 'blksize', 'blocks'}
function fwfs:getattr(path, st)
	info "mkdir"
	local pst = pio.new 'stat'
	if pio.stat(root..path, pst)~=0 then
		return -errno.errno
	end
	for _,k in ipairs(fields) do
		st[k] = pst[k]
	end
end

local descriptors = {}

function fwfs:mkdir(path, mode)
	info "mkdir"
	if pio.mkdir(root..path, mode)~=0 then
		return -errno.errno
	end
end

function fwfs:rmdir(path)
	info "rmdir"
	if pio.rmdir(root..path)~=0 then
		return -errno.errno
	end
end

function fwfs:opendir(path, fi)
	info "opendir"
	local dir = pio.opendir(root..path)
	if not dir then
		return -errno.errno
	end
	fi.fh = #descriptors+1
	descriptors[fi.fh] = {dir=dir}
end

function fwfs:releasedir(path, fi)
	info "releasedir"
	if fi.fh~=0 then
		descriptors[fi.fh].dir:closedir()
		descriptors[fi.fh] = nil
	else
		return -errno.EINVAL
	end
end

function fwfs:readdir(path, filler, offset, fi)
	info "readdir"
	if fi.fh~=0 then
		local dir = descriptors[fi.fh].dir
		repeat
			local entry = dir:readdir()
			if entry then
				filler(entry.name, nil, 0)
			end
		until not entry
	end
end

function fwfs:mknod(path, mode, dev)
	info "mknod"
	if pio.mknod(root..path, mode, dev)~=0 then
		return -errno.errno
	end
end

function fwfs:unlink(path)
	info "unlink"
	if pio.unlink(root..path)~=0 then
		return -errno.errno
	end
end

function fwfs:open(path, fi)
	info "open"
	local fd = pio.open(root..path, fi.flags)
	if fd==-1 then
		return -errno.errno
	end
	fi.fh = #descriptors+1
	descriptors[fi.fh] = {fd=fd, offset=0}
end

function fwfs:release(path, fi)
	info "release"
	if fi.fh~=0 then
		pio.close(descriptors[fi.fh].fd)
		descriptors[fi.fh] = nil
	else
		return -errno.EINVAL
	end
end

function fwfs:read(path, buf, size, offset, fi)
	info("read: offset="..offset.." size="..size)
	if fi.fh~=0 then
		local descriptor = descriptors[fi.fh]
		if descriptor.offset~=offset then
			pio.lseek(descriptor.fd, offset, 'SET')
			descriptor.offset = offset
		end
		size = pio.read(descriptor.fd, buf, size)
		descriptor.offset = descriptor.offset + size
		return size
	else
		return -errno.EINVAL
	end
end

function fwfs:write(path, buf, size, offset, fi)
	info("write: offset="..offset.." size="..size)
	if fi.fh~=0 then
		local descriptor = descriptors[fi.fh]
		if descriptor.offset~=offset then
			warning("seeking")
			pio.lseek(descriptor.fd, offset, 'SET')
			descriptor.offset = offset
		end
		size = pio.write(descriptor.fd, buf, size)
		descriptor.offset = descriptor.offset + size
		return size
	else
		return -errno.EINVAL
	end
end

local fields = {'bsize', 'frsize', 'blocks', 'bfree', 'bavail', 'files', 'ffree', 'favail', 'fsid', 'flag', 'namemax'}
function fwfs:statfs(path, st)
	local pst = pio.new 'statvfs'
	if pio.statvfs(root..path, pst)~=0 then
		return -errno.errno
	end
	local pbsize = pst.bsize
	for _,k in ipairs(fields) do
		st[k] = pst[k]
	end
end

function fwfs:utimens(path, time)
	info("utimens('"..path.."', {{sec="..time[1].sec..", nsec="..time[1].nsec.."}, {sec="..time[2].sec..", nsec="..time[2].nsec.."}})")
	local times = pio.new('timeval', 2)
	times[1].sec = time[1].sec
	times[1].usec = time[1].nsec / 1000
	times[2].sec = time[2].sec
	times[2].usec = time[2].nsec / 1000
	return pio.utimes(root..path, times)
end

function fwfs:rename(oldpath, newpath)
	return pio.rename(root..oldpath, root..newpath)
end

local args = {"fwfs", select(2, ...)}
for _,arg in ipairs(args) do
	if arg=='-d' then
		verbose = true
	end
end
fuse.main(args, fwfs)

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
