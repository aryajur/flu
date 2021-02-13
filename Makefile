# FUSE version. You can compile for different FUSE versions. Supported
# versions are: 25, 26.
FUSE_VERSION=31
LUA_VERSION=5.3

PREFIX?=/usr
INSTALL_BIN=$(PREFIX)/lib/lua/$(LUA_VERSION)
CPPFLAGS+=-Wall -Wextra -Werror -O2 -Wno-unused-function -Iinc
CFLAGS+=-fPIC
LDFLAGS+=-fvisibility=hidden


build: flu.so

flu.so: CPPFLAGS+=-DFUSE_USE_VERSION=$(FUSE_VERSION)
# flu.so: LDLIBS+=-lfuse
flu.so: LDLIBS+= lib/libfuse3.so
flu.so: errno.o posix_structs.o compat.o

%.so: %.c
	$(LINK.c) -shared $^ $(LOADLIBES) $(LDLIBS) -o $@

install:build
	mkdir -p $(INSTALL_BIN)
	cp flu.so $(INSTALL_BIN)/

uninstall:
	rm -f $(INSTALL_BIN)/flu.so

clean:
	rm -f errno.o posix_structs.o flu.so

.PHONY:build install uninstall clean

# Copyright (c) Jérôme Vuarand
# 
# Permission is hereby granted, free of charge, to any person obtaining a copy
# of this software and associated documentation files (the "Software"), to deal
# in the Software without restriction, including without limitation the rights
# to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
# copies of the Software, and to permit persons to whom the Software is
# furnished to do so, subject to the following conditions:
# 
# The above copyright notice and this permission notice shall be included in
# all copies or substantial portions of the Software.
# 
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
# AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
# OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
# THE SOFTWARE.

