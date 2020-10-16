#include "asynPortDriver.h"

class amc100Driver : public asynPortDriver {
public:
    amc100Driver(const char* portName, const char* lowLevelPortName, int lowLevelPortAddress);
	virtual ~amc100Driver();

    int lowLevelPortConnect(const char *port, int addr, asynUser **ppasynUser, const char *inputEos, const char *outputEos);
    asynStatus lowLevelWriteRead(const char *command, char *response, size_t maxlen, bool dontLock = false, bool quiet = false);
    asynStatus lowLevelRead(char * response, size_t maxlen, bool dontLock = false);

    // AMC100 Commands
    // asynStatus queryVersion();

    // asynStatus poll();

protected:
    asynUser* lowLevelPortUser_;
    /// Check received repsonse against expected string
    asynStatus checkResponse(const std::string & command, const std::string & response, const std::string & expected);

int amcError_;
int amcFirmware_;
int amcUserCommand_;
int amcLastReply_;

const int amc_MAXBUF_ = 255; ///< Maximum buffer string size
const int amc_timeout_ = 1; //< Communications timeout, seconds
const int maxReconnectAttempts = 25; ///< Maximum reconnection attempts
const int isOK = 0;

};

