
# deprules.make - rules for dependancy generation
# to depend in a single directory for 5700: make -s -f Makefile .depend TARGET=os20dbg
ifeq (os20,$(findstring os20,$(TARGET)))
DEPSRCS=$(foreach file, $(ALL_SRCS), $(addprefix $(DEPBASE_W)/,$(file)))
endif

export DEPFILE:=$(shell pwd)/.depend.$(TARGET)$(FEATURESTRING)
depend::
	@echo "Building dependency information..."
	@rm -f .depend
	@$(MAKE) -s -f Makefile .depend

# If the Makefile or *files.make are more recent than .depend,
# re-create .depend.
.depend: Makefile $(FILELIST)
#	@echo "No .depend file exists, or Makefile/filelist are more recent - rebuilding .depend"
ifeq (os21,$(findstring os21,$(TARGET)))
	@(cd $(DEPBASE_W) && $(CC) -MM $(INCLUDEPATHS) $(WARNINGFLAGS) $(CFLAGS) $(BOARDFLAGS) $(ALL_SRCS) | sed -e 's/^.*\.o:/$(OBJDIR)\/\0/' >$(DEPFILE))
endif

ifeq (st200,$(findstring st200,$(TARGET)))
	@(cd $(DEPBASE_W) && $(CC) -MM $(INCLUDEPATHS) $(WARNINGFLAGS) $(CFLAGS) $(BOARDFLAGS) $(ALL_SRCS) | sed -e 's/^.*\.o:/$(OBJDIR)\/\0/' >$(DEPFILE))
endif

ifeq (os20,$(findstring os20,$(TARGET)))
	@$(CC) -depend .depend.temp $(CFLAGS) $(INCLUDEPATHS) $(DEPSRCS)
	@cat .depend.temp | sed -e 's/^.*\.o:/$(OBJDIR)\/\0/' >.depend.$(TARGET)$(FEATURESTRING)
	@rm .depend.temp
endif

ifeq (linuxpcusr,$(findstring linuxpcusr,$(TARGET)))
	@(cd $(DEPBASE_W) && $(CC) -MM $(INCLUDEPATHS) $(WARNINGFLAGS) $(CFLAGS) $(BOARDFLAGS) $(ALL_SRCS) | sed -e 's/^.*\.o:/$(OBJDIR)\/\0/' >$(DEPFILE))
endif

ifeq (linuxst40usr,$(findstring linuxst40usr,$(TARGET)))
	@(cd $(DEPBASE_W) && $(CC) -MM $(INCLUDEPATHS) $(WARNINGFLAGS) $(CFLAGS) $(BOARDFLAGS) $(ALL_SRCS) | sed -e 's/^.*\.o:/$(OBJDIR)\/\0/' >$(DEPFILE))
endif


#
# include dependency files if they exist
#
#ifneq ($(wildcard .depend.$(TARGET)$(FEATURESTRING)),)
-include .depend.$(TARGET)$(FEATURESTRING)
#endif

