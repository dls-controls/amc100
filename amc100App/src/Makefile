TOP=../..

include $(TOP)/configure/CONFIG

# -------------------------------
# Build a Diamond Support Module
# -------------------------------

USR_CXXFLAGS_Linux += -std=c++11

# ------------------------
# Build an IOC Application
# ------------------------

LIBRARY_IOC = AMC100

# AMC100.dbd will be installed into <top>/dbd
DBD += AMC100.dbd

# AMC100.dbd will be created from these files
AMC100_DBD += base.dbd
AMC100_DBD += asyn.dbd
AMC100_DBD += motorSupport.dbd
AMC100_DBD += AMC100.dbd

# AMC100_registerRecordDeviceDriver.cpp will be created
# AMC100.dbd
AMC100_SRCS += AMC100_registerRecordDeviceDriver.cpp

# These two lines are needed for non-vxWorks builds, such as Linux
AMC100_SRCS_DEFAULT += AMC100Main.cpp
AMC100_SRCS_vxWorks += -nil-

# Add locally compiled object code
AMC100_SRCS += AMC100Controller.cpp
AMC100_SRCS += AMC100Axis.cpp

# This line says that this IOC Application depends on the
# xxx Support Module
AMC100_LIBS += asyn
AMC100_LIBS += motor

# We need to link this IOC Application against the EPICS Base libraries
AMC100_LIBS += $(EPICS_BASE_IOC_LIBS)

# ---------------------------------------------------

include $(TOP)/configure/RULES
