#!/usr/bin/env lua
local flu = require 'flu'
local errno = flu.errno

function assert(...)
	local value,err = ...
	if not value then
		error(err, 2)
	else
		return ...
	end
end

local function mkset(array)
	local set = {}
	for _,flag in ipairs(array) do
		set[flag] = true
	end
	return set
end

local root = {}

local function splitpath(path)
	local elements = {}
	for element in path:gmatch("[^/]+") do
		table.insert(elements, element)
	end
	return elements
end

local function getfile(path)
	path = splitpath(path)
	if #path == 0 then
		return root
	else
		local parent = root
		for i = 1,#path - 1 do
			local dir = path[i]
			parent = parent[dir]
			if type(parent)~='table' then return nil end
		end
		local node = parent[path[#path]]
		return node,parent
	end
end

local luafs = {}

function luafs.getattr(path, st)
	local file = getfile(path)
	if file then
		if type(file)=='table' then
			local st = {}
			st.mode = mkset{ 'dir', 'rusr', 'wusr', 'xusr', 'rgrp', 'wgrp', 'xgrp', 'roth', 'woth', 'xoth' }
			st.size = 4096
			-- link count is important for some apps like 'find'
			-- - one from the parent dir (:TODO: actually there should be one for each parent)
			-- - one from the current dir (.)
			-- - one from each subdir (..)
			st.nlink = 2
			for _,subfile in pairs(file) do
				if type(file)=='table' then
					st.nlink = st.nlink + 1
				end
			end
			return st
		elseif type(file)=='string' then
			return {
				mode = mkset{ 'reg', 'rusr', 'wusr', 'rgrp', 'wgrp', 'roth', 'woth' },
				size = #file,
				nlink = 1,
			}
		else
			error(errno.EINVAL)
		end
	else
		error(errno.ENOENT)
	end
end

local descriptors = {}

function luafs.mkdir(path, mode)
	local file,parent = getfile(path)
	assert(not file, errno.EEXIST)
	assert(parent, errno.ENOENT)
	local tpath = splitpath(path)
	local filename = assert(tpath[#tpath], errno.EINVAL)
	assert(type(parent)=='table', errno.ENOTDIR)
	local file = {}
	parent[filename] = file
end

function luafs.rmdir(path)
	local file,parent = getfile(path)
	assert(file, errno.ENOENT)
	assert(type(file)=='table', errno.ENOTDIR)
	local tpath = splitpath(path)
	local filename = assert(tpath[#tpath], errno.EINVAL)
	parent[filename] = nil
end

function luafs.opendir(path, fi)
	local file,parent = getfile(path)
	assert(file, errno.ENOENT)
	assert(type(file)=='table', errno.ENOTDIR)
	local tpath = splitpath(path)
	local filename = tpath[#tpath]
	if not filename and path~="/" then
		error(errno.EINVAL)
	end
	fi.fh = #descriptors+1
	descriptors[fi.fh] = {path=path, parent=parent, filename=filename, file=file}
end

function luafs.releasedir(path, fi)
	assert(fi.fh~=0, errno.EINVAL)
	descriptors[fi.fh] = nil
end

function luafs.readdir(path, filler, fi)
	assert(fi.fh~=0, errno.EINVAL)
	local file = descriptors[fi.fh].file
	filler(".")
	filler("..")
	for name in pairs(file) do
		filler(name)
	end
end

function luafs.mknod(path, mode, dev)
	local file,parent = getfile(path)
	assert(not file, errno.EEXIST)
	assert(parent, errno.ENOENT)
	local tpath = splitpath(path)
	local filename = assert(tpath[#tpath], errno.EINVAL)
	assert(type(parent)=='table', errno.ENOTDIR)
	local file = ""
	parent[filename] = file
end

function luafs.unlink(path)
	local file,parent = getfile(path)
	assert(file, errno.ENOENT)
	assert(type(file)~='table', errno.EISDIR)
	local tpath = splitpath(path)
	local filename = assert(tpath[#tpath], errno.EINVAL)
	parent[filename] = nil
end

function luafs.open(path, fi)
	local file,parent = getfile(path)
	assert(file, errno.ENOENT)
	assert(type(file)~='table', errno.EISDIR)
	local tpath = splitpath(path)
	local filename = assert(tpath[#tpath], errno.EINVAL)
	fi.fh = #descriptors+1
	descriptors[fi.fh] = {path=path, parent=parent, filename=filename, file=file}
end

function luafs.release(path, fi)
	assert(fi.fh~=0, errno.EINVAL)
	descriptors[fi.fh] = nil
end

function luafs.read(path, size, offset, fi)
	assert(fi.fh~=0, errno.EINVAL)
	local file = descriptors[fi.fh].file
	if type(file)=='string' then
		local filelen = #file
		if offset<filelen then
			if offset + size > filelen then
				size = filelen - offset
			end
			return file:sub(offset+1, offset+size)
		end
	end
	return ''
end

function luafs.write(path, buf, offset, fi)
	assert(fi.fh~=0, errno.EINVAL)
	local descriptor = descriptors[fi.fh]
	local file,filename,parent = descriptor.file,descriptor.filename,descriptor.parent
	if type(file)=='string' then
		local buflen = #buf
		local filelen = #file
		if offset<filelen then
			if offset + buflen >= filelen then
				file = file:sub(1,offset)..buf
			else
				file = file:sub(1,offset)..buf..file:sub(offset+buflen)
			end
		else
			file = file..string.rep('\0', offset-filelen)..buf
		end
		parent[filename] = file
		descriptor.file = file
		return #buf
	else
		return 0
	end
end

-- truncate is necessary to rewrite a file
function luafs.truncate(path, size)
	local file,parent = getfile(path)
	assert(file, errno.ENOENT)
	assert(type(file)~='table', errno.EISDIR)
	local tpath = splitpath(path)
	local filename = assert(tpath[#tpath], errno.EINVAL)
	if size > #file then
		parent[filename] = file..string.rep('\0', size - #file)
	else
		parent[filename] = file:sub(1, size)
	end
end

-- utimens necessary for 'touch'
function luafs.utimens(path, access, modification)
end

local xattrs = {}
-- :NOTE: since the filesystem is a pure tree (not a DAG), use the path to find attribs

function luafs.getxattr(path, name)
	local attrs = assert(xattrs[path], errno.ENODATA)
	return assert(attrs[name], errno.ENODATA)
end

function luafs.setxattr(path, name, value, flags)
	local attrs = xattrs[path]
	if not attrs then
		attrs = {}
		xattrs[path] = attrs
	end
	attrs[name] = value
end

function luafs.removexattr(path, name)
	local attrs = assert(xattrs[path], errno.ENODATA)
	attrs[name] = nil
	if next(attrs)==nil then
		xattrs[path] = nil
	end
end

function luafs.listxattr(path)
	local attrs = xattrs[path]
	if attrs then
		local names = {}
		for name in pairs(attrs) do
			table.insert(names, name)
		end
		return names
	else
		return {}
	end
end

local args = {"luafs", ...}
flu.main(args, luafs)

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
