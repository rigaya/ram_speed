include config.mak

vpath %.cpp $(SRCDIR)

OBJS    = $(SRCS:%.cpp=%.cpp.o)
OBJASMS = $(SRCASMS:%.asm=%.o)

all: $(PROGRAM)

$(PROGRAM): .depend $(OBJS) $(OBJASMS)
	$(LD) $(OBJS) $(OBJASMS) $(LDFLAGS) -o $(PROGRAM)

%.cpp.o: %.cpp .depend
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
    
distclean: clean
	rm -f config.mak

install: all
	install -d $(PREFIX)/bin
	install -m 755 $(PROGRAM) $(PREFIX)/bin

uninstall:
	rm -f $(PREFIX)/bin/$(PROGRAM)
    
config.mak:
	./configure
