CPP = g++ 
dir_path := $(abspath $(dir $(lastword $(MAKEFILE_LIST))))
SRCS_path := $(dir_path)/sources
OBJ_path := $(dir_path)/obj
INC_path := $(dir_path)/include
#CFLAGS+=-pthread
CFLAGS+=-I${fileDirname}
#CFLAGS+=-lws2_32
#LIBS= $(dir_path)/libs

APPINCLUDE:=-I$(dir_path) \
			-I$(dir_path)/include

SRCPP+=$(wildcard *.cpp)

SRCOBJ := $(patsubst %.c,%.o,$(SRC))

INCOBJ=$(patsubst %.c,%$(MULT).$(CROSS).o,$(SRCINC))
EXE=$(patsubst %.c,%.o,$(SRC))
EXEPP=$(patsubst %.cpp,%.o,$(SRCPP))

OBJS = $(wildcard **/*.o)

all: run

#.PHONY: exe

# Run all executables for the matrix multiplication program
run: headers exe 
	./app.exe

exe::
#	 Building testcase
	$(V) $(CPP) $(CFLAGS) \
		-c $(SRCPP) \
		$(APPINCLUDE) $(OBJS) -o app.exe

	

headers::
	$(CPP) -c $(SRCS_path)/netInt.cpp $(APPINCLUDE) -o $(OBJ_path)/netInt.o

#	$(CPP) -o simNet $(OBJ_path)/LList.o

.PHONY: clean

clean:
	rm -rf *.o *.exe
