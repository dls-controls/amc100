/*
 * AMC100Controller.h
 *
 */

#ifndef AMC_100_CONTROLLER_H_
#define AMC_100_CONTROLLER_H_

#include <string.h>
#include <pthread.h>
#include "asynMotorController.h"
#include "asynMotorAxis.h"

#include "AMC100Axis.h"

// Strings for extra parameters
#define indexConnectedString "CONNECTED"
#define indexAmplitudeString "AMPLITUDE"
#define indexAmplitudeRbvString "AMPLITUDE_RBV"
#define indexErrorString "ERROR"
#define indexFirmwareString "FIRMWARE"
#define indexFrequencyString "FREQUENCY"
#define indexFrequencyRbvString "FREQUENCY_RBV"
#define indexAxisEnabledString "ST_ENABLED"
#define indexAxisEnabledRbvString "ST_ENABLED_RBV"
#define indexAxisConnectedString "ST_CONNECTED"
#define indexAxisPositionString "POSITION"
#define indexAxisRefPositionString "REF_POSITION"
#define indexStatusReferenceString "STAT_REF"
#define indexAxisAutoResetString "AUTO_RESET"
#define indexAxisAutoResetRbvString "AUTO_RESET_RBV"
#define indexAxisRefAutoUpdateString "REF_AUTO_UPDATE"
#define indexAxisRefAutoUpdateRbvString "REF_AUTO_UPDATE_RBV"
#define indexAxisResetString "AXIS_RESET"


#define RECV_BUFFER_LEN (256)
#define COMMAND_MOVE_REQID (0)
#define COMMAND_GET_FIRMWARE_REQID (1)
#define COMMAND_GET_STATUS_REQID (2)
#define COMMAND_GET_POSITION_REQID (3)
#define COMMAND_GET_REF_POSITION_REQID (4)
#define COMMAND_SET_CONTROL_MOVE_REQID (5)
#define COMMAND_GET_CONTROL_OUTPUT_REQID (6)
#define COMMAND_SET_CONTROL_OUTPUT_REQID (7)
#define COMMAND_GET_REF_AUTO_UPDATE_REQID (8)
#define COMMAND_SET_REF_AUTO_UPDATE_REQID (9)
#define COMMAND_GET_AUTO_RESET_REQID (10)
#define COMMAND_SET_AUTO_RESET_REQID (11)
#define COMMAND_ERROR_NUM_TO_STRING_REQID (12)
#define COMMAND_GET_AMPLITUDE_REQID (13)
#define COMMAND_GET_FREQUENCY_REQID (14)
#define COMMAND_SET_AMPLITUDE_REQID (15)
#define COMMAND_SET_FREQUENCY_REQID (16)
#define COMMAND_GET_AXIS_CXN_REQID (17)
#define COMMAND_GET_STATUS_REF_REQID (18)
#define COMMAND_STOP_MOVE_REQID (19)
#define COMMAND_GET_STATUS_EOT_FWD_REQID (20)
#define COMMAND_GET_STATUS_EOT_BKWD_REQID (21)
#define COMMAND_GET_CONTROL_EOT_OUTPUT_REQID (22)
#define COMMAND_SET_AXIS_RESET_REQID (23)

#define MAX_N_REPLIES (99)


class AMC100Controller : public asynMotorController {
    friend class AMC100Axis;
private:
    // Constants
	enum {INTSTRINGLEN=5,RXBUFFERSIZE=10, TXBUFFERSIZE=10, RESBUFFERSIZE=30, CONNECTIONPOLL=10};
public:
    unsigned int idReq;
	AMC100Controller(const char *portName, int controllerNum,
	        const char* lowlevelPortName, int lowlevelPortAddress, int numAxes,
	        double movingPollPeriod, double idlePollPeriod);
    virtual ~AMC100Controller();

    // Overridden from asynMotorController
    virtual asynStatus poll();
    virtual asynStatus writeInt32(asynUser *pasynUser, epicsInt32 value);
    bool sendReceive(const char* tx, size_t txSize, char* rx, size_t rxSize);
    bool receive(int reqId, char *buffer);
    void receivingTask();
    bool parseReqId(char *buffer, int *reqId);
    asynStatus lowlevelWrite(const char *buffer, size_t buffer_len);
    asynStatus lowlevelRead(char *buffer, size_t buffer_len);
    bool sendCommand(const char *command, int reqId);
    bool sendCommand(const char *command, int reqId, int val1);
    bool sendCommand(const char *command, int reqId, int val1, int val2);
    bool sendCommand(const char *command, int reqId, int val1, bool val2);
    bool sendCommand(const char *command, int reqId, int val1, double val2);
protected:
    int firstParam;
    int indexConnected;
    int indexFirmware;
    int indexAmplitude;
    int indexAmplitudeRbv;
    int indexError;
    int indexFrequency;
    int indexFrequencyRbv;
    int indexAxisEnabled;
    int indexAxisEnabledRbv;
    int indexAxisConnected;
    int indexAxisRefPosition;
    int indexStatusReference;
    int indexAxisStopEot;
    int indexAxisReset;
    int indexAxisAutoReset;
    int indexAxisAutoResetRbv;
    int indexAxisRefUpdate;
    int indexAxisRefUpdateRbv;
    int lastParam;

private:
    /* Data */
    asynUser* lowlevelPortUser;
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
    char replyBuffers[MAX_N_REPLIES][256];
    epicsEventId replyEvents[MAX_N_REPLIES];
    epicsMutexId replyLocks[MAX_N_REPLIES];
    epicsMutexId sendingLock;
    epicsMutexId printLock;
};

#endif /* INCLUDE_AMC_100_CONTROLLER_H_ */
