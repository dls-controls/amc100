/*
 * amc100Controller.h
 *
 */

#include <string.h>
#include "asynMotorController.h"
#include "asynMotorAxis.h"

#define size_t int // temporary fix for eclipse indexer problem

#include "amc100Axis.h"

// Strings for extra parameters
#define indexVersionHighString "VERSION_HIGH"
#define indexVersionLowString "VERSION_LOW"
#define indexStatusString "STATUS"
#define indexConnectedString "CONNECTED"
#define indexSystemIdString "SYSTEM_ID"
#define indexActiveHoldString "ACTIVE_HOLD"
#define indexAmplitudeString "AMPLITUDE"
#define indexErrorString "ERROR"
#define indexFirmwareString "FIRMWARE"
#define indexFrequencyString "FREQUENCY"
#define indexVoltageString "VOLTAGE"


class amc100Controller : public asynMotorController {
    friend class amc100Axis;
private:
    // Constants
	enum {INTSTRINGLEN=5,RXBUFFERSIZE=10, TXBUFFERSIZE=10, RESBUFFERSIZE=30, CONNECTIONPOLL=100};
public:
	amc100Controller(const char *portName, int controllerNum,
	        const char* serialPortName, int serialPortAddress, int numAxes,
	        double movingPollPeriod, double idlePollPeriod);
    virtual ~amc100Controller();

    // Overridden from asynMotorController
    virtual asynStatus poll();
    virtual asynStatus writeInt32(asynUser *pasynUser, epicsInt32 value);

protected:
    int firstParam;
    int indexVersionHigh;
    int indexVersionLow;
    int indexStatus;
    int indexConnected;
    int indexSystemId;
    int indexActiveHold;
    int indexAmplitude;
    int indexFirmware;
    int indexError;
    int indexFrequency;
    int lastParam;

private:
    /* Data */
    asynUser* serialPortUser;
    int controllerNum;
    int connectionPollRequired;
    bool initialized;
    unsigned int idReq;

private:
    /* utility methods */
    int DecodeInt32(unsigned char* encoded);
    void EncodeInt(int value, unsigned char* encoded);
    void IntToString(const unsigned char* encoded, char* string);

    /* Communication Methods */
    bool firstTimeInit();
    // bool readAmplitude(int axisNum);
    bool readFirmwareVer();
    // bool errorNumberToString(int errorNum);
    // bool readFrequency(int axisNum);
    bool sendReceive(const char* tx, size_t txSize, char* rx, size_t rxSize);
    bool command(unsigned char axis, unsigned char command,
    		const unsigned char* parms, size_t pLen, unsigned char* response, size_t rLen);
};
