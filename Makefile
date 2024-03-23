# Copyright 2024, Maik Merten
#
# Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
#
# The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

# build directories for release, debug and profiling
DIR_BUILDS=builds
DIR_REL=$(DIR_BUILDS)/release
DIR_DEB=$(DIR_BUILDS)/debug
DIR_PRF=$(DIR_BUILDS)/profile

# executable names for release and debug
EXE_REL=hmp3
EXE_DEB=hmp3
EXE_PRF=hmp3

# name of static library file
ALIB=libhmp3.a

# location of hmp3 source files
SRC_PREFIX=./hmp3/src

CC=gcc
CC_WIN32=i686-w64-mingw32-gcc
CC_WIN64=x86_64-w64-mingw32-gcc

CFLAGS_COMMON=-O3 -fno-math-errno -fno-trapping-math -c -I$(SRC_PREFIX)/pub -DIEEE_FLOAT
CFLAGS_REL=$(CFLAGS_COMMON)
CFLAGS_REL_WIN32=-march=pentium3 -mtune=pentium3 -mfpmath=sse $(CFLAGS_COMMON)
CFLAGS_REL_WIN64=-march=x86-64 $(CFLAGS_COMMON)

CFLAGS_DEB=-g -O0 -DDEBUG -c -I$(SRC_PREFIX)/pub -DIEEE_FLOAT
CFLAGS_PRF=$(CFLAGS_REL) -g -pg
LFLAGS=-lm -lstdc++

AR=ar

# source files for encoder library within SRC_PREFIX
SRC_LIB_C=amodini2.c cnts.c detect.c emap.c l3init.c l3pack.c mhead.c pcmhpm.c setup.c spdsmr.c xhead.c cnt.c emdct.c filter2.c hwin.c l3math.c pow34.c sbt.c xhwin.c xsbt.c
SRC_LIB_CPP=bitallo.cpp bitallo1.cpp bitallo3.cpp bitalloc.cpp bitallos.cpp bitallosc.cpp mp3enc.cpp srcc.cpp srccf.cpp srccfb.cpp

# source files for encoder application
SRC_APP=test/tomp3.cpp

OBJS_LIB = $(SRC_LIB_C:.c=.o)
OBJS_LIB +=$(SRC_LIB_CPP:.cpp=.o)

OBJS_LIB_REL = $(addprefix $(DIR_REL)/,$(OBJS_LIB))
OBJS_LIB_DEB = $(addprefix $(DIR_DEB)/,$(OBJS_LIB))
OBJS_LIB_PRF = $(addprefix $(DIR_PRF)/,$(OBJS_LIB))

OBJS_APP_REL = $(addprefix $(DIR_REL)/, $(SRC_APP:.cpp=.o))
OBJS_APP_DEB = $(addprefix $(DIR_DEB)/, $(SRC_APP:.cpp=.o))
OBJS_APP_PRF = $(addprefix $(DIR_PRF)/, $(SRC_APP:.cpp=.o))

.PHONY: clean prep
all: prep $(DIR_REL)/$(EXE_REL)
debug: prep $(DIR_DEB)/$(EXE_DEB)
profile: prep $(DIR_PRF)/$(EXE_PRF)

release-win32: CC=$(CC_WIN32)
release-win32: CFLAGS_REL=$(CFLAGS_REL_WIN32)
release-win32: clean all

release-win64: CC=$(CC_WIN64)
release-win64: CFLAGS_REL=$(CFLAGS_REL_WIN64)
release-win64: clean all


# Release

$(DIR_REL)/%.o: $(SRC_PREFIX)/%.c
	$(CC) $(CFLAGS_REL) -o $@ $<

$(DIR_REL)/%.o: $(SRC_PREFIX)/%.cpp
	$(CC) $(CFLAGS_REL) -o $@ $<

$(DIR_REL)/$(ALIB): $(OBJS_LIB_REL)
	$(AR) rcs $@ $^

$(DIR_REL)/$(EXE_REL): $(OBJS_LIB_REL) $(OBJS_APP_REL)
	$(CC) -o $@ $^ $(LFLAGS)


# Debug

$(DIR_DEB)/%.o: $(SRC_PREFIX)/%.c
	$(CC) $(CFLAGS_DEB) -o $@ $<

$(DIR_DEB)/%.o: $(SRC_PREFIX)/%.cpp
	$(CC) $(CFLAGS_DEB) -o $@ $<

$(DIR_DEB)/$(ALIB): $(OBJS_LIB_DEB)
	$(AR) rcs $@ $^

$(DIR_DEB)/$(EXE_DEB): $(OBJS_LIB_DEB) $(OBJS_APP_DEB)
	$(CC) -o $@ $^ $(LFLAGS)

# Profile

$(DIR_PRF)/%.o: $(SRC_PREFIX)/%.c
	$(CC) $(CFLAGS_PRF) -o $@ $<

$(DIR_PRF)/%.o: $(SRC_PREFIX)/%.cpp
	$(CC) $(CFLAGS_PRF) -o $@ $<

$(DIR_PRF)/$(ALIB): $(OBJS_LIB_PRF)
	$(AR) rcs $@ $^

$(DIR_PRF)/$(EXE_PRF): $(OBJS_LIB_PRF) $(OBJS_APP_PRF)
	$(CC) -o $@ $^ -pg $(LFLAGS)


prep:
	@mkdir -p $(DIR_REL)/test $(DIR_DEB)/test $(DIR_PRF)/test

clean:
	@rm -rf $(DIR_BUILDS)
