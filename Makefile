# build directories for release and debug
DIR_BUILDS=builds
DIR_REL=$(DIR_BUILDS)/release
DIR_DEB=$(DIR_BUILDS)/debug

# executable names for release and debug
EXE_REL=hmp3
EXE_DEB=hmp3

# name of static library file
ALIB=libhmp3.a

# location of hmp3 source files
SRC_PREFIX=./hmp3/src

CC=gcc
AR=ar
CFLAGS_REL=-O2 -c -I$(SRC_PREFIX)/pub -DIEEE_FLOAT
CFLAGS_DEB=-g -O0 -DDEBUG -c -I$(SRC_PREFIX)/pub -DIEEE_FLOAT
LFLAGS=-lm -lstdc++

# source files for encoder library within SRC_PREFIX
SRC_LIB_C=amodini2.c cnts.c detect.c emap.c l3init.c l3pack.c mhead.c pcmhpm.c setup.c spdsmr.c xhead.c cnt.c emdct.c filter2.c hwin.c l3math.c pow34.c sbt.c xhwin.c xsbt.c
SRC_LIB_CPP=bitallo.cpp bitallo1.cpp bitallo3.cpp bitalloc.cpp bitallos.cpp bitallosc.cpp mp3enc.cpp srcc.cpp srccf.cpp srccfb.cpp

# source files for encoder application
SRC_APP=test/tomp3.cpp

OBJS_LIB = $(SRC_LIB_C:.c=.o)
OBJS_LIB +=$(SRC_LIB_CPP:.cpp=.o)

OBJS_LIB_REL = $(addprefix $(DIR_REL)/,$(OBJS_LIB))
OBJS_LIB_DEB = $(addprefix $(DIR_DEB)/,$(OBJS_LIB))

OBJS_APP_REL = $(addprefix $(DIR_REL)/, $(SRC_APP:.cpp=.o))
OBJS_APP_DEB = $(addprefix $(DIR_DEB)/, $(SRC_APP:.cpp=.o))

.PHONY: clean prep
all: prep $(DIR_REL)/$(EXE_REL)
debug: prep $(DIR_DEB)/$(EXE_DEB)	

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

prep:
	@mkdir -p $(DIR_REL)/test $(DIR_DEB)/test

clean:
	@rm -rf $(DIR_BUILDS)
