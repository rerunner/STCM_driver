
# Please insert new source files into this list alphabetically.
# Do not forget the "\" as a line continuation character 
# on ALL BUT THE LAST lines.

# Common source files
SRCS_CPP += \
Source/STFDebug.cpp \
Source/STFGenericDebug.cpp \
Source/STFHeapMemoryManager.cpp \
Source/STFProfile.cpp \
Source/STFSemaphore.cpp \
Source/STFSignal.cpp \
Source/STFTimer.cpp \
Source/Types/STFBitField.cpp \
Source/Types/STFHash.cpp \
Source/Types/STFInt64.cpp \
Source/Types/STFList.cpp \
Source/Types/STFMessage.cpp \
Source/Types/STFQueues.cpp \
Source/Types/STFRangeManager.cpp \
Source/Types/STFResult.cpp \
Source/Types/STFSharedDataBlock.cpp \
Source/Types/STFString.cpp \
Source/Types/STFTime.cpp \
Source/Tools/STFCRC.cpp

# OS21 specific source files
ifeq (os21,$(findstring os21,$(TARGET)))
SRCS_CPP += \
Source/HAL/ST40/HALSTFDataManipulation.cpp \
Source/OSAL/OS20_OS21/OSSTFMemoryManagement.cpp \
Source/OSAL/OS20_OS21/OSSTFThread.cpp \
Source/OSAL/OS21/OSSTFMutex.cpp \
Source/OSAL/OS21/OSSTFDebug.cpp \
Source/OSAL/OS21/OSSTFSemaphore.cpp \
Source/OSAL/OS21/OSSTFTimer.cpp
endif

# OS20 specific source files
ifeq (os20,$(findstring os20,$(TARGET)))
SRCS_CPP += \
Source/HAL/ST20/HALSTFDataManipulation.cpp \
Source/OSAL/OS20_OS21/OSSTFMemoryManagement.cpp \
Source/OSAL/OS20_OS21/OSSTFThread.cpp \
Source/OSAL/OS20/OSSTFMutex.cpp \
Source/OSAL/OS20/OSSTFDebug.cpp \
Source/OSAL/OS20/OSSTFSemaphore.cpp \
Source/OSAL/OS20/OSSTFTimer.cpp 
endif


# Linux PC User Mode specific source files
ifeq (linuxpcusr,$(findstring linuxpcusr,$(TARGET)))
SRCS_CPP += \
Source/HAL/gcc386/HALSTFDataManipulation.cpp \
Source/OSAL/LinuxUser/OSSTFDebug.cpp \
Source/OSAL/LinuxUser/OSSTFMutex.cpp \
Source/OSAL/LinuxUser/OSSTFSemaphore.cpp \
Source/OSAL/LinuxUser/OSSTFTimer.cpp \
Source/OSAL/LinuxUser/OSSTFThread.cpp
endif


# Linux ST40 User Mode specific source files
ifeq (linuxst40usr,$(findstring linuxst40usr,$(TARGET)))
SRCS_CPP += \
Source/HAL/ST40/HALSTFDataManipulation.cpp \
Source/OSAL/LinuxUser/OSSTFDebug.cpp \
Source/OSAL/LinuxUser/OSSTFMutex.cpp \
Source/OSAL/LinuxUser/OSSTFSemaphore.cpp \
Source/OSAL/LinuxUser/OSSTFTimer.cpp \
Source/OSAL/LinuxUser/OSSTFThread.cpp
endif

# OS21(ST200) specific source files
ifeq (st200,$(findstring st200,$(TARGET)))
SRCS_CPP += \
Source/HAL/ST200/HALSTFDataManipulation.cpp \
Source/OSAL/OS20_OS21/OSSTFMemoryManagement.cpp \
Source/OSAL/OS20_OS21/OSSTFThread.cpp \
Source/OSAL/OS21/OSSTFDebug.cpp \
Source/OSAL/OS21/OSSTFSemaphore.cpp \
Source/OSAL/OS21/OSSTFTimer.cpp
endif


# Same for the VPATH, first the common part
VPATH=\
$(STFBASE):\
$(STFBASE)/Source:\
$(STFBASE)/Source/Types:\
$(STFBASE)/Source/Tools:\

# OS21 specific search paths
ifeq (os21,$(findstring os21,$(TARGET)))
VPATH+=\
$(STFBASE)/Source/OSAL/OS20_OS21:\
$(STFBASE)/Source/OSAL/OS21:\
$(STFBASE)/Source/HAL/ST40:
endif

# OS20 specific search paths
ifeq (os20,$(findstring os20,$(TARGET)))
VPATH+=\
$(STFBASE)/Source/OSAL/OS20_OS21:\
$(STFBASE)/Source/OSAL/OS20:\
$(STFBASE)/Source/HAL/ST20:
endif

# Linux PC User specific search paths
ifeq (linuxpcusr,$(findstring linuxpcusr,$(TARGET)))
VPATH+=\
$(STFBASE)/Source/OSAL/LinuxPCUser:\
$(STFBASE)/Source/OSAL/LinuxUser:\
$(STFBASE)/Source/HAL/gcc386:
endif

# Linux ST40 User specific search paths
ifeq (linuxst40usr,$(findstring linuxst40usr,$(TARGET)))
VPATH+=\
$(STFBASE)/Source/OSAL/LinuxST40User:\
$(STFBASE)/Source/OSAL/LinuxUser:\
$(STFBASE)/Source/HAL/ST40:
endif

# OS21(ST200) specific search paths
ifeq (st200,$(findstring st200,$(TARGET)))
VPATH+=\
$(STFBASE)/Source/OSAL/OS20_OS21:\
$(STFBASE)/Source/OSAL/OS21:\
$(STFBASE)/Source/HAL/ST200:
endif

