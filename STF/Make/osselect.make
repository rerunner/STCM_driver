
# osselect.make - makefile fragment to pick up the correct tools and
# variables for the selected operating system/toolkit

# Determine host operating system
HOSTSTR:=$(shell uname -o)
ifeq ($(findstring Linux, $(HOSTSTR)),Linux)
HOST_OS:=linux
endif

ifeq ($(findstring Cygwin, $(HOSTSTR)),Cygwin)
HOST_OS:=cygwin
else
HOST_OS:=undefined
endif

## ST40 compiler options to keep in mind (see "GettingStarted.pdf", p.182):
## fastest code: -O3 -fomit-frame-pointer -funroll-loops -funroll-all-loops -mrelax
## minimal code space: -Os -fomit-frame-pointer -mrelax

### OS21, single step version (_DEBUG build *without* optimisation)
ifeq ($(TARGET), os21stp)
CC:=sh4g++
AR:=sh4ar
LD:=sh4g++
RANLIB:=sh4ranlib
TARGETTYPE:=ST40OS21

## These are the original debug settings, which produce a very inefficient executable, but could sometimes be
## necessary to do line-by-line debugging:
##CFLAGS:=-g -ansi -pedantic -fno-exceptions -mruntime=os21 -mboard=board_mb379 -D_DEBUG=1 -DOSAL_OS21=1 -DHAL_ST40=1

## These settings are producing an executable with symbols and _DEBUG enabled:
CFLAGS:=-g -ansi -pedantic -fno-exceptions -mruntime=os21 -mboard=board_mb379 -D_DEBUG=1 -DOSAL_OS21=1 -DHAL_ST40=1

WARNINGFLAGS:=-Wall -Wno-unknown-pragmas
endif


### OS21, debug version	(*with* optimisation)
ifeq ($(TARGET), os21dbg)
CC:=sh4g++
AR:=sh4ar
LD:=sh4g++
RANLIB:=sh4ranlib
TARGETTYPE:=ST40OS21

## These are the original debug settings, which produce a very inefficient executable, but could sometimes be
## necessary to do line-by-line debugging:
CFLAGS:=-g -O1 -ansi -pedantic -fno-exceptions -mruntime=os21 -mboard=board_mb379 -D_DEBUG=1 -DOSAL_OS21=1 -DHAL_ST40=1

WARNINGFLAGS:=-Wall -Wno-unknown-pragmas
endif


### OS21, release version
ifeq ($(TARGET), os21rel)
CC:=sh4g++
AR:=sh4ar
# FN: -O should improve things but slows down the linker..to be evaluated if it's worth it.
LD:=sh4g++ -O
RANLIB:=sh4ranlib
TARGETTYPE:=ST40OS21

## Original CFLAGS for release:
##CFLAGS:=-g -O1 -ansi -pedantic -fno-exceptions -mruntime=os21 -mboard=board_mb379 -DNDEBUG=1 -DOSAL_OS21=1 -DHAL_ST40=1

## A bit more optimisation towards code size reduction:
## When MCDT/SuperH delivers ST40R2.1.4 patch, set this to -O3 again
## ATTENTION: Optimized builds CAN ONLY WORK with -fno-strict-aliasing!
CFLAGS:=-g -O1 -ansi -pedantic -mspace -fno-strict-aliasing -fno-exceptions -fno-implement-inlines -mruntime=os21 -mboard=board_mb379 -DNDEBUG=1 -DOSAL_OS21=1 -DHAL_ST40=1

## The below is problematic - can cause run-time crash and sometimes crash of linker:
##CFLAGS:=-g -Os -ansi -pedantic -mspace -fno-exceptions -fno-implement-inlines -fvtable-gc -mruntime=os21 -mboard=board_mb379 -DNDEBUG=1 -DOSAL_OS21=1 -DHAL_ST40=1
WARNINGFLAGS:=-Wall -Wno-unknown-pragmas
endif



### OS21, profiled version
ifeq ($(TARGET), os21prof)
CC:=sh4g++
AR:=sh4ar
LD:=sh4g++ -g -pg 
RANLIB:=sh4ranlib
TARGETTYPE:=ST40OS21
CFLAGS:=-pg -g -ansi -pedantic -fno-exceptions -mruntime=os21 -mboard=board_mb379 -D_DEBUG=1 -DOSAL_OS21=1 -DHAL_ST40=1
WARNINGFLAGS:=-Wall -Wno-unknown-pragmas
endif

### OS21, release-profiled version
ifeq ($(TARGET), os21relprof)
CC:=sh4g++
AR:=sh4ar
LD:=sh4g++ -pg
RANLIB:=sh4ranlib
TARGETTYPE:=ST40OS21
# FN: When toolkit is fixed, increase opt. level to -O3
#CFLAGS:=-pg -O1 -g -ansi -pedantic -fno-exceptions -mruntime=os21 -mboard=board_mb379 -DNDEBUG=1 -DOSAL_OS21=1 -DHAL_ST40=1

## Default is to profile a release build
CFLAGS:=-g -O1 -ansi -pedantic -mspace -fno-exceptions -fno-implement-inlines -mruntime=os21 -mboard=board_mb379 -DNDEBUG=1 -DOSAL_OS21=1 -DHAL_ST40=1

##CFLAGS:=-pg -O3 -ansi -pedantic -fno-exceptions -mruntime=os21 -mboard=board_mb379 -DOSAL_OS21=1 -DHAL_ST40=1
##CFLAGS:= -g -pg -ansi -pedantic -fno-exceptions -mruntime=os21 -mboard=board_mb379 -D_DEBUG=1 -DOSAL_OS21=1 -DHAL_ST40=1

WARNINGFLAGS:=-Wall -Wno-unknown-pragmas
endif


### OS20, debug version
# $$$ Careful here - usage of ST20 might differ from ST40..
ifeq ($(TARGET), os20dbg)
CC:=st20cc
AR:=st20libr
LD:=st20cc
RANLIB:=true
TARGETTYPE:=ST20OS20
# CFLAGS:=-g -c2 -D_DEBUG -DOSAL_OS20 -DST20LITE
WARNINGFLAGS:=
endif

### OS20, release version
# $$$ Careful here - usage of ST20 might differ from ST40..
ifeq ($(TARGET), os20rel)
CC:=st20cc
AR:=st20libr
LD:=st20cc
RANLIB:=true
TARGETTYPE:=ST20OS20
# CFLAGS:=RELEASE:NO CFLAGS SPECIFIED YET
# WARNINGFLAGS:=RELEASE:NO WARNINGFLAGS SPECIFIED YET
endif

### OS20, profiled version
# $$$ Careful here - usage of ST20 might differ from ST40..
ifeq ($(TARGET), os20prof)
CC:=st20cc
AR:=st20libr
LD:=st20cc
RANLIB:=true
TARGETTYPE:=ST20OS20
CFLAGS:=PROFILED:NO CFLAGS SPECIFIED YET
WARNINGFLAGS:=PROFILED:NO WARNINGFLAGS SPECIFIED YET
endif


### ST200, debug version
ifeq ($(TARGET), st200dbg)
CC:=st200cc
AR:=st200ar
LD:=st200cc
RANLIB:=st200ranlib
TARGETTYPE:=ST200OS21
# When debugging LX code, add "-keep" below to keep asm files etc.
CFLAGS=-g -O0 -mruntime=os21 -D_DEBUG=1 -DOSAL_OS21=1 $(LOCALFLAGS) $(LOCALFLAGS_DEBUG)
WARNINGFLAGS:=-Wall -Wno-unknown-pragmas
endif


### ST200, release version
ifeq ($(TARGET), st200rel)
CC:=st200cc
AR:=st200ar
LD:=st200cc -O3
RANLIB:=st200ranlib
TARGETTYPE:=ST200OS21
# When debugging LX code, add "-keep" below to keep asm files etc.
CFLAGS=-g -O3 -mruntime=os21 -DNDEBUG=1 -DOSAL_OS21=1 $(LOCALFLAGS) $(LOCALFLAGS_RELEASE)
WARNINGFLAGS:=-Wall -Wno-unknown-pragmas
endif


### ST200, mix version (for ACC debugging)
ifeq ($(TARGET), st200mix)
CC:=st200cc
AR:=st200ar
LD:=st200cc -O3
RANLIB:=st200ranlib
TARGETTYPE:=ST200OS21
# When debugging LX code, add "-keep" below to keep asm files etc.
CFLAGS=-g -O3 -mruntime=os21 -DNDEBUG=1 $(LOCALFLAGS) $(LOCALFLAGS_RELEASE)
WARNINGFLAGS:=-Wall -Wno-unknown-pragmas
endif


### Linux PC User, debug version	(no optimisation)
ifeq ($(TARGET), linuxpcusrdbg)
CC:=g++
AR:=ar
LD:=g++
RANLIB:=ranlib
TARGETTYPE:=LINUXPCUSR

CFLAGS:=-g -ansi -pedantic -fno-exceptions -pthread -fpermissive -D_DEBUG=1 -D_REENTRANT -D__USE_XOPEN2K=1
WARNINGFLAGS:=-Wall -Wno-unknown-pragmas
endif


### Linux ST40 User, debug version	(no optimisation)
ifeq ($(TARGET), linuxst40usrdbg)
CC:=sh4-linux-g++
AR:=sh4-linux-ar
LD:=sh4-linux-g++
RANLIB:=sh4-linux-ranlib
TARGETTYPE:=LINUXST40USR

CFLAGS:=-g -ansi -pedantic -fno-exceptions -pthread -D_DEBUG=1	-D_REENTRANT -D__USE_XOPEN2K=1
WARNINGFLAGS:=-Wall -Wno-unknown-pragmas
endif

