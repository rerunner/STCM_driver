
# comp_vdrsupport.make - component-specific flags etc for VDR/Support component

# Some Makefile "above us" sets BUILDTYPE to one of
# "DEBUG", "RELEASE", "PROFILED" or "UDEFINED" (user-defined).
# It has also set TARGETTYPE to one of ST40OS21, ST20OS20,
# (+x86WIN32, ST200OS21 later)

VDRSUPPORTBASE:=$(VIEWBASE)/STCM_driver/VDR/Support
ifeq ($(HOST_OS), cygwin)
VDRSUPPORTBASE_W:=$(shell cygpath -m $(VDRSUPPORTBASE))
else
VDRSUPPORTBASE_W:=$(VDRSUPPORTBASE)
endif

ifeq ($(TARGETTYPE),ST40OS21)
INCLUDEPATHS += -I$(VDRSUPPORTBASE_W)/InofficialCode/SerialMenu/Interface/OSAL/OS21
endif

# Currently the same as above.
ifeq ($(TARGETTYPE),ST20OS20)
# Nothing to be done here yet.
INCLUDEPATHS += -I$(VDRSUPPORTBASE_W)/InofficialCode/SerialMenu/Interface/OSAL/OS20
endif

# OS21 on ST200: Nothing to be done yet.
ifeq ($(TARGETTYPE),ST200OS21)
INCLUDEPATHS += 
endif

# LX builds go here once ST200 support of VDR has been written

LIB_VDRSUPPORT_DEBUG:= $(VDRSUPPORTBASE_W)/Library/$(TARGETTYPE)/libvdrsupport_d.a
LIB_VDRSUPPORT_RELEASE:= $(VDRSUPPORTBASE_W)/Library/$(TARGETTYPE)/libvdrsupport.a
LIB_VDRSUPPORT_PROFILED:= $(VDRSUPPORTBASE_W)/Library/$(TARGETTYPE)/libvdrsupport_p.a
LIB_VDRSUPPORT_RELEASEPROFILED:= $(VDRSUPPORTBASE_W)/Library/$(TARGETTYPE)/libvdrsupport_rp.a
# $$$ UDEFINED still missing..

LIBS := $(LIB_VDRSUPPORT_$(BUILDTYPE)) $(LIBS)
