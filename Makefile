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
	./app.exe | tee logs/log.txt

exe::
#	 Building testcase
	$(V) $(CPP) $(CFLAGS) \
		-c $(SRCPP) \
		$(APPINCLUDE) $(OBJS) -o app.exe

	

headers::
	$(CPP) -c $(SRCS_path)/netData.cpp $(APPINCLUDE) -o $(OBJ_path)/netData.o
	$(CPP) -c $(SRCS_path)/netInt.cpp $(APPINCLUDE) -o $(OBJ_path)/netInt.o
	$(CPP) -c $(SRCS_path)/LList.cpp $(APPINCLUDE) -o $(OBJ_path)/LList.o
	$(CPP) -c $(SRCS_path)/netOpt.cpp $(APPINCLUDE) -o $(OBJ_path)/netOpt.o
	$(CPP) -c $(SRCS_path)/lightOptimiser.cpp $(APPINCLUDE) -o $(OBJ_path)/lightOptimiser.o
	$(CPP) -c $(SRCS_path)/speakerOptimiser.cpp $(APPINCLUDE) -o $(OBJ_path)/speakerOptimiser.o
	$(CPP) -c $(SRCS_path)/light2light.cpp $(APPINCLUDE) -o $(OBJ_path)/light2light.o
	$(CPP) -c $(SRCS_path)/light2tv.cpp $(APPINCLUDE) -o $(OBJ_path)/light2tv.o
	$(CPP) -c $(SRCS_path)/light2speaker.cpp $(APPINCLUDE) -o $(OBJ_path)/light2speaker.o
	$(CPP) -c $(SRCS_path)/light2recordPlayer.cpp $(APPINCLUDE) -o $(OBJ_path)/light2recordPlayer.o
	$(CPP) -c $(SRCS_path)/tv2tv.cpp $(APPINCLUDE) -o $(OBJ_path)/tv2tv.o
	$(CPP) -c $(SRCS_path)/tv2speaker.cpp $(APPINCLUDE) -o $(OBJ_path)/tv2speaker.o
	$(CPP) -c $(SRCS_path)/tv2recordPlayer.cpp $(APPINCLUDE) -o $(OBJ_path)/tv2recordPlayer.o
	$(CPP) -c $(SRCS_path)/speaker2speaker.cpp $(APPINCLUDE) -o $(OBJ_path)/speaker2speaker.o
	$(CPP) -c $(SRCS_path)/speaker2recordPlayer.cpp $(APPINCLUDE) -o $(OBJ_path)/speaker2recordPlayer.o
	$(CPP) -c $(SRCS_path)/recordPlayer2recordPlayer.cpp $(APPINCLUDE) -o $(OBJ_path)/recordPlayer2recordPlayer.o


#	$(CPP) -o simNet $(OBJ_path)/LList.o

.PHONY: clean

clean:
	rm -rf *.o *.exe
