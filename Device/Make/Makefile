
# Makefile for Device component 

# Typical targets you will want to call are:
# make TARGET=os21dbg depend - create dependencies for this build
# make TARGET=os21dbg        - build debug version for OS21, ST40
# make TARGET=os21dbg clean  - clean up the mess
# make TARGET=os21stp        - build debug version for OS21 WITHOUT optimizer for single-stepping
# make TARGET=os21rel   ...dto..same for os20.

# When VOBs are moved around, this will have to be adapted.
# VIEWBASE is pointing to the view's base directory
# TOOLBASE points to the location of the Makefile snippets
# DRVBASE is used for references to other components in the driver vob - also for Makefile snippets
# SRCBASE is used for component-local files (sources, headers, library location...)
#
# DRVBASE is in Unix path notation, DRVBASE_W is the same in 'simplified Win notation'
# (Example: Unix: /e/ccviews/franky_dv/bla  Windows: e:/ccviews/franky_dv/bla)
# Unix notation is needed e.g. for VPATH, Windows notation is needed for the
# actual Compiler (sh4gcc etc) invocation.

VIEWBASE := $(shell echo `pwd` | sed -e 's/\/STCM_driver\/.*//')

# When the snippets get moved to their final destination, this might have to be changed.
TOOLBASE := $(VIEWBASE)/STCM_driver/STF/Make

# selection of toolkit/operating system
include $(TOOLBASE)/osselect.make

DRVBASE:=$(shell echo `pwd` | sed -e 's/\/STCM_driver\/.*//')/STCM_driver
ifeq ($(HOST_OS), cygwin)
DRVBASE_W:=$(shell cygpath -m $(DRVBASE))
VIEWBASE_W := $(shell cygpath -m $(VIEWBASE))
else
DRVBASE_W:=$(DRVBASE)
VIEWBASE_W := $(VIEWBASE)
endif

FILELIST:=devicefiles.make

# define the different directory paths for 8000 and 5700 
ifeq (os21,$(findstring os21,$(TARGET)))
STCOMPOINC:=Source/Unit/Video/Blitter/Specific/STCOMPO/STm8000
TOOLSETINC:=ST40
endif
ifeq (os20,$(findstring os20,$(TARGET)))
STCOMPOINC:=Source/Unit/Video/Blitter/Specific/STCOMPO/STm5700
TOOLSETINC:=ST20
endif


# We should really try to get rid of the include paths into the Device/Source directory!!
INCLUDEPATHS += -I$(DRVBASE_W) \
 -I$(PROJECTBASE_W)/Interface \
 -I$(VIEWBASE)/STCM_driverExt \
 -I/usr/include/directfb \
 -I$(DEVICEBASE_W)/Source/Unit/MemoryMappedIO/Specific/$(TOOLSETINC) 

# Bring in the list of source files and directories for this component
include $(FILELIST)

# generic build rules
include $(TOOLBASE)/rules.make

# build type selection
include $(TOOLBASE)/buildtype.make

# header include paths and libs specific to this component
# These will have to be changed when the snippets move to their final location
include $(TOOLBASE)/comp_stf.make
include $(TOOLBASE)/comp_vdr.make
include $(TOOLBASE)/comp_device.make

SUBDIRS:=

# $$$ Should split "TARGET" up into BUILDTYPE=.., TARGETTYPE=.., CPU=.. ?

THISTARGET:=$(LIB_DEVICE_$(BUILDTYPE))

### Targets for "full build" (source available)
# if source code is visible at some specific location...
ifneq ($(shell find . -maxdepth 1 -iname .bin_only), ./.bin_only)

os21stp: dirs $(THISTARGET)

os21dbg: dirs $(THISTARGET)

os21rel: dirs $(THISTARGET)

os21prof: dirs $(THISTARGET)

os21relprof: dirs $(THISTARGET)

os20dbg: dirs $(THISTARGET)

os20rel: dirs $(THISTARGET)

os20prof: dirs $(THISTARGET)

linuxpcusrdbg: dirs $(THISTARGET)

linuxst40usrdbg: dirs $(THISTARGET)


### Targets for "customer build" (no source available)
else
os21stp:
	@echo "No source code available - assuming binary distribution. Done."
os21dbg:
	@echo "No source code available - assuming binary distribution. Done."
os21rel:
	@echo "No source code available - assuming binary distribution. Done."
os21prof:
	@echo "No source code available - assuming binary distribution. Done."
os21relprof:
	@echo "No source code available - assuming binary distribution. Done."
os20dbg:
	@echo "No source code available - assuming binary distribution. Done."
os20rel:
	@echo "No source code available - assuming binary distribution. Done."
os20prof:
	@echo "No source code available - assuming binary distribution. Done."
linuxpcusrdbg:
	@echo "No source code available - assuming binary distribution. Done."
linuxst40usrdbg:
	@echo "No source code available - assuming binary distribution. Done."
endif

# Create the "build directory"
.PHONY: dirs

dirs:
	@mkdir -p $(OBJDIR)
ifeq ($(wildcard .depend.$(TARGET)),)
	@echo "==============================================================================="
	@echo "No .depend file for this config found - have you done 'make TARGET=xxx depend'?"
	@echo "==============================================================================="
endif

$(THISTARGET): $(OBJS) $(FILELIST)
	@echo Creating library $(THISTARGET)
	@rm -f $(THISTARGET)
ifeq (os21,$(findstring os21,$(TARGET)))
	@$(AR) rc $(THISTARGET) $(OBJS)
	@$(RANLIB) $(THISTARGET)
endif
ifeq (os20,$(findstring os20,$(TARGET)))
	$(AR) $(OBJS) -o $(THISTARGET)
endif
ifeq (linuxpcusr,$(findstring linuxpcusr,$(TARGET)))
	@$(AR) rc $(THISTARGET) $(OBJS)
	@$(RANLIB) $(THISTARGET)
endif
ifeq (linuxst40usr,$(findstring linuxst40usr,$(TARGET)))
	@$(AR) rc $(THISTARGET) $(OBJS)
	@$(RANLIB) $(THISTARGET)
endif

include $(TOOLBASE)/conflog.make
include $(TOOLBASE)/cleanrules.make

# auxiliary variable, needed for dependency file list
DEPBASE_W:= $(DEVICEBASE_W)
include $(TOOLBASE)/deprules.make

