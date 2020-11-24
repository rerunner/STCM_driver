
# comp_device.make - component-specific flags etc for Device component

# Some Makefile "above us" sets BUILDTYPE to one of
# "DEBUG", "RELEASE", "PROFILED" or "UDEFINED" (user-defined).
# It has also set TARGETTYPE to one of ST40OS21, ST20OS20,
# (+x86WIN32, ST200OS21 later)

DEVICEBASE:=$(VIEWBASE)/STCM_driver/Device
ifeq ($(HOST_OS), cygwin)
DEVICEBASE_W:=$(shell cygpath -m $(DEVICEBASE))
else
DEVICEBASE_W:=$(DEVICEBASE)
endif

ifeq ($(TARGETTYPE),ST40OS21)
INCLUDEPATHS += \
 -I$(DEVICEBASE_W)/Interface \
 -I$(DEVICEBASE_W)/Interface/Unit/Interrupt/Specific/ST40 \
 -I$(DEVICEBASE_W)/Interface/HAL/ST40


# $$$ Need to check this one - do we need more search paths, down into
# the subfolders below Device/Interface/Unit/...?
endif

# $$$ To be ported!
ifeq ($(TARGETTYPE),ST20OS20)
INCLUDEPATHS += \
 -I$(DEVICEBASE_W)/Interface \
 -I$(DEVICEBASE_W)/Interface/Unit/Interrupt/Specific/OS20 \
 -I$(DEVICEBASE_W)/Source/HAL/ST20

endif


ifeq ($(TARGETTYPE),LINUXPCUSR)
INCLUDEPATHS += \
 -I$(DEVICEBASE_W)/Interface \
 -I$(DEVICEBASE_W)/Interface/Unit/Interrupt/Specific/x86 \
 -I$(DEVICEBASE_W)/Interface/HAL/x86
# $$$ Need to check this one - do we need more search paths, down into
# the subfolders below Device/Interface/Unit/...?
endif


ifeq ($(TARGETTYPE),LINUXST40USR)
INCLUDEPATHS += \
 -I$(DEVICEBASE_W)/Interface \
 -I$(DEVICEBASE_W)/Interface/HAL/ST40

# -I$(DEVICEBASE_W)/Interface/Unit/Interrupt/Specific/ST40 
# $$$ Need to check this one - do we need more search paths, down into
# the subfolders below Device/Interface/Unit/...?
endif


LIB_DEVICE_DEBUG:= $(DEVICEBASE_W)/Library/$(TARGETTYPE)/libdevice_d.a
LIB_DEVICE_RELEASE:= $(DEVICEBASE_W)/Library/$(TARGETTYPE)/libdevice.a
LIB_DEVICE_PROFILED:= $(DEVICEBASE_W)/Library/$(TARGETTYPE)/libdevice_p.a
LIB_DEVICE_RELEASEPROFILED:= $(DEVICEBASE_W)/Library/$(TARGETTYPE)/libdevice_rp.a
# $$$ UDEFINED still missing..

LIBS := $(LIB_DEVICE_$(BUILDTYPE)) $(LIBS)
