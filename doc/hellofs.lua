#!/usr/bin/env lua
local flu = require 'flu'
local errno = flu.errno

local msg = "Hello World!\n"

local function mkset(array)
	local set = {}
	for _,flag in ipairs(array) do
		set[flag] = true
	end
	return set
end

local hellofs = {}

function hellofs.getattr(path, st)
	if path=="/" then
		return {
			mode = mkset{ 'dir', 'rusr', 'wusr', 'xusr', 'rgrp', 'xgrp', 'roth', 'xoth' },
			nlink = 2,
		}
	elseif path=="/hello" then
		return {
			mode = mkset{ 'reg', 'rusr', 'wusr', 'rgrp', 'roth' },
			nlink = 1,
			size = #msg,
		}
	else
		error(errno.ENOENT)
	end
end

function hellofs.readdir(path, filler, fi)
	assert(path=="/", errno.ENOENT)
	filler(".")
	filler("..")
	filler("hello")
end

function hellofs.open(path, fi)
	assert(path=="/hello", errno.ENOENT)
	if fi.flags.O_WRONLY or fi.flags.O_RDWR then
		error(errno.EACCES)
	end
end

function hellofs.opendir(path, fi)
	assert(path=="/", errno.ENOENT)
	if fi.flags.O_WRONLY or fi.flags.O_RDWR then
		error(errno.EACCES)
	end
end

function hellofs.read(path, size, offset, fi)
	assert(path=="/hello", errno.ENOENT)
	local len = #msg
	if offset<len then
		if offset + size > len then
			size = len - offset
		end
		return msg:sub(offset+1, offset+size)
	else
		return ""
	end
end

local args = {"hellofs", ...}
flu.main(args, hellofs)

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
