
# comp_stf.make - component-specific flags etc for STF component

# Some Makefile "above us" sets BUILDTYPE to one of
# "DEBUG", "RELEASE", "PROFILED" or "UDEFINED" (user-defined).
# It has also set TARGETTYPE to one of ST40OS21, ST20OS20,
# (+x86WIN32, ST200OS21 later)

STFBASE:=$(VIEWBASE)/STCM_driver/STF
ifeq ($(HOST_OS), cygwin)
STFBASE_W:=$(shell cygpath -m $(STFBASE))
else
STFBASE_W:=$(STFBASE)
endif

ifeq ($(TARGETTYPE),ST40OS21)
INCLUDEPATHS += \
 -I$(STFBASE_W)/Interface/OSAL/OS20_OS21 \
 -I$(STFBASE_W)/Interface/OSAL/OS21 \
 -I$(STFBASE_W)/Interface/Types/OSAL/OS21
endif

ifeq ($(TARGETTYPE),ST20OS20)
INCLUDEPATHS += \
 -I$(STFBASE_W)/Interface/OSAL/OS20_OS21 \
 -I$(STFBASE_W)/Interface/OSAL/OS20 \
 -I$(STFBASE_W)/Interface/Types/OSAL/OS20
endif

ifeq ($(TARGETTYPE),LINUXPCUSR)
INCLUDEPATHS += \
 -I$(STFBASE_W)/Interface/OSAL/LinuxUser \
 -I$(STFBASE_W)/Interface/Types/OSAL/LinuxUser \
 -I$(STFBASE_W)/Interface/OSAL/LinuxPCUser \
 -I$(STFBASE_W)/Interface/Types/OSAL/LinuxPCUser
endif

ifeq ($(TARGETTYPE),LINUXST40USR)
INCLUDEPATHS += \
 -I$(STFBASE_W)/Interface/OSAL/LinuxUser \
 -I$(STFBASE_W)/Interface/Types/OSAL/LinuxUser
endif


# LX builds go here once ST200 support of STF has been written

ifeq ($(TARGETTYPE),ST200OS21)
INCLUDEPATHS += \
 -I$(STFBASE_W)/Interface/OSAL/OS20_OS21 \
 -I$(STFBASE_W)/Interface/OSAL/OS21 \
 -I$(STFBASE_W)/Interface/Types/OSAL/OS21
endif


LIB_STF_DEBUG:= $(STFBASE_W)/Library/$(TARGETTYPE)/libstf_d.a
LIB_STF_RELEASE:= $(STFBASE_W)/Library/$(TARGETTYPE)/libstf.a
LIB_STF_PROFILED:= $(STFBASE_W)/Library/$(TARGETTYPE)/libstf_p.a
LIB_STF_RELEASEPROFILED:= $(STFBASE_W)/Library/$(TARGETTYPE)/libstf_rp.a
# $$$ UDEFINED still missing..

LIBS := $(LIB_STF_$(BUILDTYPE)) $(LIBS)

