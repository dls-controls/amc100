TOP=../..

include $(TOP)/configure/CONFIG

# -------------------------------
# Build a Diamond Support Module
# -------------------------------

USR_CXXFLAGS_Linux += -std=c++11

# ------------------------
# Build an IOC Application
# ------------------------

LIBRARY_IOC = amc100

# amc100.dbd will be installed into <top>/dbd
DBD += amc100.dbd

# amc100.dbd will be created from these files
amc100_DBD += base.dbd
amc100_DBD += asyn.dbd

amc100_SRCS += amc100Driver.cpp

# amc100_registerRecordDeviceDriver.cpp will be created
# amc100.dbd
amc100_SRCS += amc100_registerRecordDeviceDriver.cpp

# These two lines are needed for non-vxWorks builds, such as Linux
amc100_SRCS_DEFAULT += amc100Main.cpp
amc100_SRCS_vxWorks += -nil-

# Add locally compiled object code
#amc100_SRCS +=

# This line says that this IOC Application depends on the
# xxx Support Module
amc100_LIBS += asyn

# We need to link this IOC Application against the EPICS Base libraries
amc100_LIBS += $(EPICS_BASE_IOC_LIBS)

# ---------------------------------------------------

include $(TOP)/configure/RULES
