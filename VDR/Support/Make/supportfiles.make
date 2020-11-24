
# Please insert new source files into this list alphabetically.
# Do not forget the "\" as a line continuation character 
# on ALL BUT THE LAST lines.

# Common source files
SRCS_CPP += \
Profile/ProfileFactory.cpp \
Profile/ProfileWrapper.cpp \
Streaming/VDRStreamingFormatter.cpp

# OS21 specific source files
ifeq (os21,$(findstring os21,$(TARGET)))
SRCS_CPP += \
InofficialCode/SerialMenu/Source/SerialMenu.cpp \
InofficialCode/SerialMenu/Source/SystemSerialMenu.cpp \
InofficialCode/SerialMenu/Source/OSAL/OS21/OSSystemInspector.cpp
endif

# OS20 specific source files
ifeq (os20,$(findstring os20,$(TARGET)))
SRCS_CPP += \
InofficialCode/SerialMenu/Source/SerialMenu.cpp \
InofficialCode/SerialMenu/Source/SystemSerialMenu.cpp \
InofficialCode/SerialMenu/Source/OSAL/OS20/OSSystemInspector.cpp
endif

# OS21(ST200) specific source files
ifeq (st200,$(findstring st200,$(TARGET)))
SRCS_CPP += \

endif

SRCS_C := 

SRCS := $(SRCS_CPP) $(SRCS_C)

# If a new directory is created under Source/, add it to the
# following search path:
VPATH=\
$(VDRSUPPORTBASE):\
$(VDRSUPPORTBASE)/Profile \
$(VDRSUPPORTBASE)/Streaming

# OS21 specific search paths
ifeq (os21,$(findstring os21,$(TARGET)))
VPATH+=\
$(VDRSUPPORTBASE)/InofficialCode/SerialMenu/Source:\
$(VDRSUPPORTBASE)/InofficialCode/SerialMenu/Source/OSAL/OS21:
endif

# OS20 specific search paths
ifeq (os20,$(findstring os20,$(TARGET)))
VPATH+=\
$(VDRSUPPORTBASE)/InofficialCode/SerialMenu/Source:\
$(VDRSUPPORTBASE)/InofficialCode/SerialMenu/Source/OSAL/OS20:
endif

# OS21(ST200) specific search paths
ifeq (st200,$(findstring st200,$(TARGET)))
VPATH += \

endif

