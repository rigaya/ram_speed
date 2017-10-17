SRCDIR=.
PROGRAM=ram_speed64
#PROGRAM=ram_speed32
CXX=g++
AS=nasm
LD=gcc
SRCS=ram_speed/ram_speed.cpp ram_speed/cpu_info.cpp ram_speed/simd_util.cpp
ASMS=ram_speed/ram_speed_x64.asm
#ASMS=ram_speed/ram_speed_x86.asm

CXXFLAGS=-I./ram_speed -std=c++11 -DLINUX -DLINUX64 -m64 -O3 -DNDEBUG=1
#CXXFLAGS=-I./ram_speed -DLINUX -DLINUX32 -m32 -O3 -DNDEBUG=1
ASFLAGS=-I./ram_speed -DLINUX=1 -f elf64 -DARCH_X86_64=1
#ASFLAGS=-I./ram_speed -DLINUX=1 -f elf32
LDFLAGS=-L. -lstdc++ -lpthread -lm

vpath %.cpp $(SRCDIR)
vpath %.asm $(SRCDIR)

OBJS  = $(SRCS:%.cpp=%.o)
OBJASMS = $(ASMS:%.asm=%.o)

all: $(PROGRAM)

$(PROGRAM): .depend $(OBJS) $(OBJASMS)
	$(LD) $(OBJS) $(OBJASMS) $(LDFLAGS) -o $(PROGRAM)

%.o: %.cpp .depend
	$(CXX) -c $(CXXFLAGS) -o $@ $<

%.o: %.asm
	$(AS) $(ASFLAGS) $< -o $@
	
.depend:
	@rm -f .depend
	@echo 'generate .depend...'
	@$(foreach SRC, $(SRCS:%=$(SRCDIR)/%), $(CXX) $(SRC) $(CXXFLAGS) -g0 -MT $(SRC:$(SRCDIR)/%.cpp=%.o) -MM >> .depend;)
	
ifneq ($(wildcard .depend),)
include .depend
endif

clean:
	rm -f $(OBJS) $(OBJASMS) $(PROGRAM) .depend
