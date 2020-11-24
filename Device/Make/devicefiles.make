
# Please insert new source files into this list alphabetically.
# Do not forget the "\" as a line continuation character 
# on ALL BUT THE LAST lines.

# !!!
# Please keep an alphabetical sorting within each group!
# !!!

# Common source files
SRCS_CPP += \
Source/Unit/Board/StandardBoard.cpp \
Source/Unit/Datapath/Generic/ChainLink.cpp \
Source/Unit/Datapath/Generic/StreamMixer.cpp \
Source/Unit/Datapath/Specific/MPEG/DVDPESSplitter.cpp \
Source/Unit/Datapath/Specific/MPEG/DVDPESStreamUnpacker.cpp \
Source/Unit/Datapath/Specific/MPEG/DVDStreamDemux.cpp \
Source/Unit/Memory/HeapMemoryPool.cpp \
Source/Unit/Memory/LinearMemoryPool.cpp 

# OS21 specific source files
ifeq (os21,$(findstring os21,$(TARGET)))
SRCS_CPP += 
endif

# OS20 specific source files
ifeq (os20,$(findstring os20,$(TARGET)))
SRCS_CPP += 
endif

# Linux PC User specific source files
ifeq (linuxpcusr,$(findstring linuxpcusr,$(TARGET)))
SRCS_CPP += \
Source/Unit/Video/Decoder/Specific/MPEGVideoDecoder.cpp \
Source/Unit/Audio/Decoder/Specific/AC3Decoder.cpp \
Source/Unit/Audio/Renderer/Specific/SDL2AudioRenderer.cpp \
Source/Unit/Audio/Renderer/Specific/PulseAudioRenderer.cpp
endif

# Linux ST40 User specific source files
ifeq (linuxst40usr,$(findstring linuxst40usr,$(TARGET)))
SRCS_CPP += 
endif


SRCS_C := 

SRCS := $(SRCS_CPP) $(SRCS_C)

# If a new directory is used under Source/, add it to the
# correct search path:

# Common search paths
VPATH=\
$(DEVICEBASE):\
$(DEVICEBASE)/Source:\
$(DEVICEBASE)/Source/Unit/Board:\
$(DEVICEBASE)/Source/Unit/Datapath/Generic:\
$(DEVICEBASE)/Source/Unit/Datapath/Specific:\
$(DEVICEBASE)/Source/Unit/Datapath/Specific/MPEG:\
$(DEVICEBASE)/Source/Unit/Memory:

# OS21 specific search paths
ifeq (os21,$(findstring os21,$(TARGET)))
VPATH+=
endif


# OS20 specific search paths
ifeq (os20,$(findstring os20,$(TARGET)))
VPATH+=
endif

# LinuxPCUser specific search paths
ifeq (linuxpcusr,$(findstring linuxpcusr,$(TARGET)))
VPATH+=\
$(DEVICEBASE)/Source/Unit/Video/Decoder/Specific:\
$(DEVICEBASE)/Source/Unit/Audio/Decoder/Generic:\
$(DEVICEBASE)/Source/Unit/Audio/Decoder/Specific:\
$(DEVICEBASE)/Source/Unit/Audio/Renderer/Specific:
endif

# LinuxST40User specific search paths
ifeq (linuxst40usr,$(findstring linuxst40usr,$(TARGET)))
VPATH+=
endif


# Only add if you really need this.
# Source/Unit/Support/Performance/PerformanceMeasurement.cpp
# $(DEVICEBASE)/Source/Unit/Support/Performance:\

