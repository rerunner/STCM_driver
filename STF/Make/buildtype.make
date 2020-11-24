
# buildtype.make - set build type, depending on selected build TARGET

### "all" targets for OS21 builds
ifeq ($(TARGET),os21stp)
BUILDTYPE:=DEBUG
all: os21stp
endif

ifeq ($(TARGET),os21dbg)
BUILDTYPE:=DEBUG
all: os21dbg
endif

ifeq ($(TARGET),os21rel)
BUILDTYPE:=RELEASE
all: os21rel
endif

ifeq ($(TARGET),os21prof)
BUILDTYPE:=PROFILED
all: os21prof
endif

ifeq ($(TARGET),os21relprof)
BUILDTYPE:=RELEASEPROFILED
all: os21relprof
endif

# $$$ user-defined OS21 build goes here..

### "all" targets for OS20 builds
ifeq ($(TARGET),os20dbg)
BUILDTYPE:=DEBUG
all: os20dbg
endif

ifeq ($(TARGET),os20rel)
BUILDTYPE:=RELEASE
all: os20rel
endif

ifeq ($(TARGET),os20prof)
BUILDTYPE:=PROFILED
all: os20prof
endif

# $$$ user-defined OS20 build goes here..


ifeq ($(TARGET),linuxpcusrdbg)
BUILDTYPE:=DEBUG
all: linuxpcusrdbg
endif

ifeq ($(TARGET),linuxst40usrdbg)
BUILDTYPE:=DEBUG
all: linuxst40usrdbg
endif


### "all" targets for ST200 builds
ifeq ($(TARGET),st200dbg)
BUILDTYPE:=DEBUG
all: st200dbg
endif

ifeq ($(TARGET),st200rel)
BUILDTYPE:=RELEASE
all: st200rel
endif

ifeq ($(TARGET),st200mix)
BUILDTYPE:=MIX
all: st200mix

# Profiling on ST200 not yet tested.
#ifeq ($(TARGET),st200prof)
#BUILDTYPE:=PROFILED
#all: st200prof



# $$$ to be moved when remaining targets below get added.
else
# endif    - commented out for fall-through to "default" case (usage:) below


### If no (or no correct) TARGET is given, print out usage
.PHONY: usage
usage:
	@echo "Incorrect 'make' invocation; possible configurations are:"
	@echo "make TARGET=os21stp     - build debug version (non-optimized) for OS21"
	@echo "make TARGET=os21dbg     - build debug version (-O1) for OS21"
	@echo "make TARGET=os21rel     - build release version for OS21"
	@echo "make TARGET=os21prof    - build profiled version for OS21"
	@echo "make TARGET=os21relprof - build release-profiled version for OS21"
	@echo "make TARGET=os21user    - build user-defined version for OS21 (not yet implemented)"
	@echo "make TARGET=os20dbg     - build debug version for OS20 (later)"
	@echo "make TARGET=os20rel     - build release version for OS20 (later)"
	@echo "make TARGET=os20prof    - build profiled version for OS20 (not yet implemented)"
	@echo "make TARGET=os20user    - build user-defined version for OS20 (not yet implemented)"
	@echo "make TARGET=st200dbg    - build debug version for ST200/OS21"
	@echo "make TARGET=st200rel    - build release version for ST200/OS21"
	@echo "make TARGET=linuxpcusrdbg    - build debug version for Linux PC User Mode"
	@echo "make TARGET=linuxst40usrdbg    - build debug version for Linux ST40 User Mode"
endif
