
# comp_vdr.make - component-specific flags etc for VDR component

# Some Makefile "above us" sets BUILDTYPE to one of
# "DEBUG", "RELEASE", "PROFILED" or "UDEFINED" (user-defined).
# It has also set TARGETTYPE to one of ST40OS21, ST20OS20,
# (+x86WIN32, ST200OS21 later)

VDRBASE:=$(VIEWBASE)/STCM_driver/VDR
ifeq ($(HOST_OS), cygwin)
VDRBASE_W:=$(shell cygpath -m $(VDRBASE))
else
VDRBASE_W:=$(VDRBASE)
endif

ifeq ($(TARGETTYPE),ST40OS21)
INCLUDEPATHS += \
 -I$(VDRBASE_W) \
 -I$(VDRBASE_W)/Interface
endif

# Currently the same as above.
ifeq ($(TARGETTYPE),ST20OS20)
INCLUDEPATHS += \
 -I$(VDRBASE_W) \
 -I$(VDRBASE_W)/Interface
endif

# OS21 on ST200: Same as above.
ifeq ($(TARGETTYPE),ST200OS21)
INCLUDEPATHS += \
 -I$(VDRBASE_W) \
 -I$(VDRBASE_W)/Interface
endif

# LX builds go here once ST200 support of VDR has been written

LIB_VDR_DEBUG:= $(VDRBASE_W)/Library/$(TARGETTYPE)/libvdr_d.a
LIB_VDR_RELEASE:= $(VDRBASE_W)/Library/$(TARGETTYPE)/libvdr.a
LIB_VDR_PROFILED:= $(VDRBASE_W)/Library/$(TARGETTYPE)/libvdr_p.a
LIB_VDR_RELEASEPROFILED:= $(VDRBASE_W)/Library/$(TARGETTYPE)/libvdr_rp.a
# $$$ UDEFINED still missing..

LIBS := $(LIB_VDR_$(BUILDTYPE)) $(LIBS)
