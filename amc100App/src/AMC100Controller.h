/*
 * AMC100Controller.h
 *
 */

#ifndef AMC_100_CONTROLLER_H_
#define AMC_100_CONTROLLER_H_

#include <string.h>
#include "asynMotorController.h"
#include "asynMotorAxis.h"

#define size_t int // temporary fix for eclipse indexer problem

#include "AMC100Axis.h"

// Strings for extra parameters
#define indexConnectedString "CONNECTED"
#define indexAmplitudeString "AMPLITUDE"
#define indexErrorString "ERROR"
#define indexFirmwareString "FIRMWARE"
#define indexFrequencyString "FREQUENCY"


class AMC100Controller : public asynMotorController {
    friend class AMC100Axis;
private:
    // Constants
	enum {INTSTRINGLEN=5,RXBUFFERSIZE=10, TXBUFFERSIZE=10, RESBUFFERSIZE=30, CONNECTIONPOLL=10};
public:
    unsigned int idReq;
	AMC100Controller(const char *portName, int controllerNum,
	        const char* serialPortName, int serialPortAddress, int numAxes,
	        double movingPollPeriod, double idlePollPeriod);
    virtual ~AMC100Controller();

    // Overridden from asynMotorController
    virtual asynStatus poll();
    virtual asynStatus writeInt32(asynUser *pasynUser, epicsInt32 value);
    bool sendReceive(const char* tx, size_t txSize, char* rx, size_t rxSize);

protected:
    int firstParam;
    int indexConnected;
    int indexFirmware;
    int indexAmplitude;
    int indexError;
    int indexFrequency;
    int lastParam;

private:
    /* Data */
    asynUser* serialPortUser;
    int controllerNum;
    int connectionPollRequired;
    bool initialized;
    int numAxes;

private:
    /* utility methods */
    int DecodeInt32(unsigned char* encoded);
    void EncodeInt(int value, unsigned char* encoded);
    void IntToString(const unsigned char* encoded, char* string);

    /* Communication Methods */
    bool firstTimeInit();
    bool getFirmwareVer();
    bool setError(int errorNum);
    // bool errorNumberToString(int errorNum);
    bool command(unsigned char axis, unsigned char command,
    		const unsigned char* parms, size_t pLen, unsigned char* response, size_t rLen);
};

#endif /* INCLUDE_AMC_100_CONTROLLER_H_ */