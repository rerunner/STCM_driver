
# Please insert new source files into this list alphabetically.
# Do not forget the "\" as a line continuation character 
# on ALL BUT THE LAST lines.

# Common source files
SRCS_CPP += \
Source/Base/VDRBase.cpp \
Source/Base/VDRMessage.cpp \
Source/Construction/Main.cpp \
Source/Construction/UnitConstruction.cpp \
Source/Memory/MemoryPartition.cpp \
Source/Memory/MemoryPoolAllocator.cpp \
Source/Memory/MemOverlapDetector.cpp \
Source/Memory/ROMLayout/Specific/ManagerMulticoreLittleEndianNoUnit.cpp \
Source/Streaming/BaseStreamingChainUnit.cpp \
Source/Streaming/BaseStreamingUnit.cpp \
Source/Streaming/InfiniteStreamReplicator.cpp \
Source/Streaming/InfiniteStreamPacketDistributor.cpp \
Source/Streaming/Streaming.cpp \
Source/Streaming/StreamingClock.cpp \
Source/Streaming/StreamingConnectors.cpp \
Source/Streaming/StreamingDebug.cpp \
Source/Streaming/StreamingFormatter.cpp \
Source/Streaming/StreamingSupport.cpp \
Source/Streaming/StreamingUnit.cpp \
Source/Unit/MessageDispatcherUnit.cpp \
Source/Unit/PhysicalUnit.cpp \
Source/Unit/Tags.cpp \
Source/Unit/UnitCollection.cpp \
Source/Unit/VirtualUnit.cpp

# OS21 specific source files
ifeq (os21,$(findstring os21,$(TARGET)))
SRCS_CPP += \
Source/Startup/Specific/ST40OS21/KernelStartup.cpp \
Source/Startup/Specific/ST40OS21/MemoryStartup.cpp
endif

# OS20 specific source files
ifeq (os20,$(findstring os20,$(TARGET)))
SRCS_CPP += \
Source/Startup/Specific/ST20OS20/CacheStartup.cpp \
Source/Startup/Specific/ST20OS20/KernelStartup.cpp \
Source/Startup/Specific/ST20OS20/MemoryStartup.cpp \
Source/Startup/Specific/ST20OS20/OS20InternalMemoryPartition.cpp
endif

# Linux PC User Mode specific source files
ifeq (linuxpcusr,$(findstring linuxpcusr,$(TARGET)))
SRCS_CPP += \
Source/Startup/Specific/LinuxPCUser/KernelStartup.cpp \
Source/Startup/Specific/LinuxPCUser/MemoryStartup.cpp
endif

# Linux ST40 User Mode specific source files
ifeq (linuxst40usr,$(findstring linuxst40usr,$(TARGET)))
SRCS_CPP += \
Source/Startup/Specific/LinuxST40User/KernelStartup.cpp \
Source/Startup/Specific/LinuxST40User/MemoryStartup.cpp
endif

# OS21(ST200) specific source files
ifeq (st200,$(findstring st200,$(TARGET)))
SRCS_CPP += \

endif


# If a new directory is used under Source/, add it to the
# correct search path:

# Common search paths
VPATH=\
$(VDRBASE):\
$(VDRBASE)/Source:\
$(VDRBASE)/Source/Base:\
$(VDRBASE)/Source/Construction:\
$(VDRBASE)/Source/Memory:\
$(VDRBASE)/Source/Memory/ROMLayout/Specific:\
$(VDRBASE)/Source/Streaming:\
$(VDRBASE)/Source/Unit:\
$(VDRBASE)/Source/Unit/Drive:\
$(VDRBASE)/Source/Unit/Video/AncillaryData:

# OS21 specific search paths
ifeq (os21,$(findstring os21,$(TARGET)))
VPATH+=\
$(VDRBASE)/Source/Startup/Specific/ST40OS21:
endif

# OS20 specific search paths
ifeq (os20,$(findstring os20,$(TARGET)))
VPATH+= \
$(VDRBASE)/Source/Startup/Specific/ST20OS20:
endif

# Linux PC User Mode specific search paths
ifeq (linuxpcusr,$(findstring linuxpcusr,$(TARGET)))
VPATH+= \
$(VDRBASE)/Source/Startup/Specific/LinuxPCUser:
endif

# Linux ST40 User Mode specific search paths
ifeq (linuxst40usr,$(findstring linuxst40usr,$(TARGET)))
VPATH+= \
$(VDRBASE)/Source/Startup/Specific/LinuxST40User:
endif

# OS21(ST200) specific search paths
ifeq (st200,$(findstring st200,$(TARGET)))
VPATH+=\

endif

