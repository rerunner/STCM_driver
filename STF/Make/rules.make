
# rules.make - generic compilation rules

# Rule that says whenever a .o file is needed compile the .cpp file
# with the same name (though the object file goes to a different dir)
# This is a bit of GNUMake magic, but it allows to store the object files
# in a flat directory while the sources are in a complex directory structure.

# This should speed up make's rule matching a LOT.
%.c:
%.cpp:
%.h:
%: %,v
%: RCS/%,v
%: RCS/%
%: s.%
%: SCCS/s.%

# FN: $(FEATURESTRING) will only be set (and used) by Dvd_Navigation/ components; it's empty for others
OBJDIR=obj_$(TARGET)$(FEATURESTRING)

$(OBJDIR)/$(notdir %).o: %.cpp
	@echo "Compiling $(shell basename $<)"
ifeq ($(HOST_OS), cygwin)
ifeq ($(VERBOSE_CC), 1)
	@echo "$(CC) $(INCLUDEPATHS) $(WARNINGFLAGS) $(CFLAGS) $(BOARDFLAGS) -c -o $@ $(shell cygpath -m $<)"
endif
	@$(CC) $(INCLUDEPATHS) $(WARNINGFLAGS) $(CFLAGS) $(BOARDFLAGS) -c -o $@ $(shell cygpath -m $<)
else
	@$(CC) $(INCLUDEPATHS) $(WARNINGFLAGS) $(CFLAGS) $(BOARDFLAGS) -c -o $@ $<
endif

$(OBJDIR)/$(notdir %).o: %.c
	@echo "Compiling $(shell basename $<)"
ifeq ($(HOST_OS), cygwin)
ifeq ($(VERBOSE_CC), 1)
	@echo "$(CC) $(INCLUDEPATHS) $(WARNINGFLAGS) $(CFLAGS) $(BOARDFLAGS) -c -o $@ $(shell cygpath -m $<)"
endif
	@$(CC) $(INCLUDEPATHS) $(WARNINGFLAGS) $(CFLAGS) $(BOARDFLAGS) -c -o $@ $(shell cygpath -m $<)
else
	@$(CC) $(INCLUDEPATHS) $(WARNINGFLAGS) $(CFLAGS) $(BOARDFLAGS) -c -o $@ $<
endif

OBJS1_CPP=$(SRCS_CPP:.cpp=.o)
OBJS1_C=$(SRCS_C:.c=.o)
ALL_SRCS=$(SRCS_C) $(SRCS_CPP)
OBJS1=$(OBJS1_CPP) $(OBJS1_C)
OBJS2=$(foreach file, $(OBJS1), $(notdir $(file)))
OBJS=$(foreach file,$(OBJS2),$(addprefix $(OBJDIR)/,$(file)))


