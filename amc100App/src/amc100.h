#include <string>
#include <iostream>
#include <cstdlib>
#include <ctime>
// EPICS
#include <epicsTime.h>
#include <epicsThread.h>
#include <epicsExport.h>
#include <epicsString.h>
#include <iocsh.h>
#include <drvSup.h>
#include <registryFunction.h>
// Asyn
#include "asynPortDriver.h"
#include "asynOctetSyncIO.h"

#define paramFirmwareString "AMC_FIRMWARE"
#define paramUserCommandString "AMC_COMMAND"
#define paramError "AMC_ERROR"

class amc100Driver : public asynPortDriver {
public:
    amc100Driver(const char *portName, const char *lowLevelPortName, int lowLevelPortAddress);
	virtual ~amc100Driver();

    int lowLevelPortConnect(const char *port, int addr, asynUser **ppasynUser, const char *inputEos, const char *outputEos);
    asynStatus lowLevelWriteRead(const char *command, char *response, size_t maxlen, bool dontLock = false, bool quiet = false);
    asynStatus lowLevelRead(char * response, size_t maxlen, bool dontLock = false);

    // AMC100 Commands
    asynStatus getFirmwareVersion();

    asynStatus poll();

protected:
    asynUser* lowLevelPortUser_;
    /// Check received repsonse against expected string
    asynStatus checkResponse(const std::string & command, const std::string & response, const std::string & expected);

static const int amc_MAXBUF_ = 255; ///< Maximum buffer string size
static const int amc_timeout_ = 1; //< Communications timeout, seconds
static const int maxReconnectAttempts = 25; ///< Maximum reconnection attempts
static const int isOK = 0;

};

