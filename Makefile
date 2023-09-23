#Makefile
#compiler=C++
PROJECT_NAME=CDCL_Solver
CXX=g++32
DEBUGGER=gdb32
SRC=src
INC=-I. -I./Include
LIB=-L.
OBJ=obj
BIN=bin
COMPILELOG=log/compile-log
LINKLOG=log/link-log

srcv:=src/

Flags=-g -Wall -fexceptions -std=c++17 #-lstdc++fs
links=

include_state=\#include

.PHONY= run,test,build,debug-run

#compile to object files
%.o: $(SRC)/%.cpp	
	$(CXX) $(Flags) $(INC) -c $< -o $(OBJ)/$@ 2>$(COMPILELOG)/$(patsubst %.o,%,$@).ccpp

#link all to exe
objfiles=$(wildcard $(OBJ)/*.o)
%.exe: $(objfiles)
	$(CXX) $(LIB) -o $(BIN)/$@ $(objfiles) $(links) 2>$(LINKLOG)/$(patsubst %.exe,%,$@).ccpp

build:
	$(MAKE) $(PROJECT_NAME).exe

#compile all
srcf=$(wildcard $(SRC)/*.cpp)
sources=$(patsubst %.cpp,%.o,$(srcf))
source=$(subst src/,,$(sources))

compileall: $(source)
	$($^)

build-dll:
	$(CXX) $(Flags) $(INC) -shared -o $(PROJECT_NAME).dll $(srcf) 2>log/DLL_export/dllexport.ccpp

#create .cpp file for the header with the same name 
%.h:
	echo $(patsubst \#,#,$(include_state))"$@" > $(SRC)/$(patsubst %.h,%.cpp,$@)

compilelist=$(wildcard $(COMPILELOG)/*.ccpp)
compiles=$(subst $(COMPILELOG)/,,$(compilelist))
#cleans all compile logs
%.ccpp:
	echo Log Cleared > $(COMPILELOG)/$@

clearcompile: $(compiles)
	$($^)

test:
	$(DEBUGGER) $(BIN)/$(PROJECT_NAME).exe