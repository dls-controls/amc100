/*
 * AMC100Controller.cpp
 * TODO: Capitalise class names to follow convention
 */

#include <stdlib.h>
#include <stddef.h>
#include <string.h>
#include <iocsh.h>
#include <math.h>
#include <epicsExport.h>
#include <epicsThread.h>
#include "rapidjson/document.h"
#include "rapidjson/writer.h"
#include "rapidjson/stringbuffer.h"
#include "AMC100Controller.h"
#include "AMC100Axis.h"
#include "asynOctetSyncIO.h"
#include "asynCommonSyncIO.h"


// HACK ALERT:
// stolen fron asynOctetSyncIO.c
// couldn't find a cleaner way to access
// the underlying pasynOctet
typedef struct ioPvt {
   asynCommon   *pasynCommon;
   void         *pcommonPvt;
   asynOctet    *pasynOctet;
   void         *octetPvt;
   asynDrvUser  *pasynDrvUser;
   void         *drvUserPvt;
} ioPvt;


extern "C" {
    static void receivingTaskC(void *arg)
    {
        AMC100Controller *controller = (AMC100Controller *) arg;
        controller->receivingTask();
    }
}

/*******************************************************************************
*
*   The AMC100 controller class
*
*******************************************************************************/

/** Constructor
 * \param[in] portName Asyn port name
 * \param[in] controllerNum The number (address) of the controller in the protocol
 * \param[in] lowlevelPortName Asyn port name of the low level port
 * \param[in] lowlevelPortAddress Asyn address of the low level port (usually 0)
 * \param[in] numAxes Maximum number of axes
 * \param[in] movingPollPeriod The period at which to poll position while moving
 * \param[in] idlePollPeriod The period at which to poll position while not moving
 */
AMC100Controller::AMC100Controller(const char* portName, int controllerNum,
        const char* lowlevelPortName, int lowlevelPortAddress, int numAxes,
        double movingPollPeriod, double idlePollPeriod)
    : asynMotorController(portName, numAxes, /*numParams=*/&lastParam-&firstParam-1,
            /*interfaceMask=*/ asynFloat64Mask, /*interruptMask=*/ asynFloat64Mask,
            /*asynFlags=*/ASYN_MULTIDEVICE | ASYN_CANBLOCK , /*autoConnect=*/1,
            /*priority=*/0, /*stackSize=*/0)
    , controllerNum(controllerNum)
    , connectionPollRequired(1)
	, initialized(false)
    , numAxes(numAxes)
{
    // Create our parameters
    createParam(indexConnectedString, asynParamInt32, &indexConnected);
    createParam(indexErrorString, asynParamInt32, &indexError);
    createParam(indexAmplitudeString, asynParamInt32, &indexAmplitude);
    createParam(indexAmplitudeRbvString, asynParamInt32, &indexAmplitudeRbv);
    createParam(indexFrequencyString, asynParamInt32, &indexFrequency);
    createParam(indexFrequencyRbvString, asynParamInt32, &indexFrequencyRbv);
    createParam(indexFirmwareString, asynParamOctet, &indexFirmware);
    createParam(indexAxisEnabledString, asynParamInt32, &indexAxisEnabled);
    createParam(indexAxisEnabledRbvString, asynParamInt32, &indexAxisEnabledRbv);
    createParam(indexAxisConnectedString, asynParamInt32, &indexAxisConnected);
    createParam(indexAxisRefPositionString, asynParamFloat64, &indexAxisRefPosition);
    createParam(indexStatusReferenceString, asynParamInt32, &indexStatusReference);

    // Initialise our parameters
    setIntegerParam(indexConnected, 0);
    setIntegerParam(indexError, 0);
    setIntegerParam(indexAmplitude, 0);
    setIntegerParam(indexAmplitudeRbv, 0);
    setIntegerParam(indexFrequency, 0);
    setIntegerParam(indexFrequencyRbv, 0);
    setStringParam(indexFirmware, "");
    setIntegerParam(indexAxisEnabled, 1);
    setIntegerParam(indexAxisEnabledRbv, 1);
    setIntegerParam(indexAxisConnected, 0);
    setDoubleParam(indexAxisRefPosition, 0.0);
    setIntegerParam(indexStatusReference, 0);


    // Connect to the low level port
    asynStatus result = pasynOctetSyncIO->connect(lowlevelPortName,
            lowlevelPortAddress, &lowlevelPortUser, NULL);
    pasynOctetSyncIO->setInputEos(lowlevelPortUser, "\n", 1);
    pasynOctetSyncIO->setOutputEos(lowlevelPortUser, "\n", 1);

    if( result != asynSuccess)
    {
        printf("AMC100Controller: Failed to connect to low level port %s\n",
               lowlevelPortName);
        asynPrint(this->pasynUserSelf, ASYN_TRACE_ERROR,
                  "AMC100Controller: Failed to connect to low level port %s\n",
                  lowlevelPortName);
    }
    else
    {
        printf("AMC100Controller: connected to low level port %s\n",
               lowlevelPortName);
        asynPrint(this->pasynUserSelf, ASYN_TRACE_FLOW,
                  "AMC100Controller: connected to low level port %s\n",
                  lowlevelPortName);
    }

    // Synchronization variables
    for (int i=0; i < MAX_N_REPLIES; i++) {
        replyEvents[i] = epicsEventCreate(epicsEventEmpty);
        replyLocks[i] = epicsMutexCreate();
    }
    sendingLock = epicsMutexCreate();
    printLock = epicsMutexCreate();

    // Create the poller thread
    startPoller(movingPollPeriod, idlePollPeriod, /*forcedFastPolls-*/10);
    epicsThreadCreate("receivingThread", 
                epicsThreadPriorityMedium, 
                epicsThreadGetStackSize(epicsThreadStackMedium), 
                (EPICSTHREADFUNC) receivingTaskC, 
                this); 
}

/** Destructor
 */
AMC100Controller::~AMC100Controller()
{
    for (int i=0; i < MAX_N_REPLIES; i++) {
        epicsEventDestroy(replyEvents[i]);
        epicsMutexDestroy(replyLocks[i]);
    }
    epicsMutexDestroy(sendingLock);
    epicsMutexDestroy(printLock);
}

bool AMC100Controller::sendCommand(const char *command, int reqId)
{
    rapidjson::StringBuffer string_buffer;
    rapidjson::Writer<rapidjson::StringBuffer> writer(string_buffer);
    writer.StartObject();
    writer.String("jsonrpc");
    writer.String("2.0");
    writer.String("method");
    writer.String(command);
    writer.String("params");
    writer.StartArray();
    writer.EndArray();
    writer.String("id");
    writer.Uint64(reqId);
    writer.EndObject();
    asynStatus result = lowlevelWrite(string_buffer.GetString(), string_buffer.GetSize());
    return result == asynSuccess;
}

bool AMC100Controller::sendCommand(const char *command, int reqId, int val1, int val2)
{
    rapidjson::StringBuffer string_buffer;
    rapidjson::Writer<rapidjson::StringBuffer> writer(string_buffer);
    writer.StartObject();
    writer.String("jsonrpc");
    writer.String("2.0");
    writer.String("method");
    writer.String(command);
    writer.String("params");
    writer.StartArray();
    writer.Uint64(val1);
    writer.Uint64(val2);
    writer.EndArray();
    writer.String("id");
    writer.Uint64(reqId);
    writer.EndObject();
    asynStatus result = lowlevelWrite(string_buffer.GetString(), string_buffer.GetSize());
    return result == asynSuccess;
}

bool AMC100Controller::sendCommand(const char *command, int reqId, int val1)
{
    rapidjson::StringBuffer string_buffer;
    rapidjson::Writer<rapidjson::StringBuffer> writer(string_buffer);
    writer.StartObject();
    writer.String("jsonrpc");
    writer.String("2.0");
    writer.String("method");
    writer.String(command);
    writer.String("params");
    writer.StartArray();
    writer.Uint64(val1);
    writer.EndArray();
    writer.String("id");
    writer.Uint64(reqId);
    writer.EndObject();
    asynStatus result = lowlevelWrite(string_buffer.GetString(), string_buffer.GetSize());
    return result == asynSuccess;
}

bool AMC100Controller::sendCommand(const char *command, int reqId, int val1, bool val2)
{
    rapidjson::StringBuffer string_buffer;
    rapidjson::Writer<rapidjson::StringBuffer> writer(string_buffer);
    writer.StartObject();
    writer.String("jsonrpc");
    writer.String("2.0");
    writer.String("method");
    writer.String(command);
    writer.String("params");
    writer.StartArray();
    writer.Uint64(val1);
    writer.Bool(val2);
    writer.EndArray();
    writer.String("id");
    writer.Uint64(reqId);
    writer.EndObject();
    asynStatus result = lowlevelWrite(string_buffer.GetString(), string_buffer.GetSize());
    return result == asynSuccess;
}

bool AMC100Controller::sendCommand(const char *command, int reqId, int val1,
                                   double val2)
{
    rapidjson::StringBuffer string_buffer;
    rapidjson::Writer<rapidjson::StringBuffer> writer(string_buffer);
    writer.StartObject();
    writer.String("jsonrpc");
    writer.String("2.0");
    writer.String("method");
    writer.String(command);
    writer.String("params");
    writer.StartArray();
    writer.Uint64(val1);
    writer.Double(val2);
    writer.EndArray();
    writer.String("id");
    writer.Uint64(reqId);
    writer.EndObject();
    asynStatus result = lowlevelWrite(string_buffer.GetString(), string_buffer.GetSize());
    return result == asynSuccess;
}

// this function doesn't block, in order to allow receiving and reading in parallel
asynStatus AMC100Controller::lowlevelWrite(const char *buffer, size_t buffer_len)
{
    const char *functionName = "AMC100Controller::lowlevelWrite";
    double timeout = 0.1;
    size_t nBytes;
    ioPvt      *pioPvt = (ioPvt *)lowlevelPortUser->userPvt;
    asynStatus status;
    lowlevelPortUser->timeout = timeout;
    
    epicsMutexLock(sendingLock);
    status = pioPvt->pasynOctet->write(
        pioPvt->octetPvt, lowlevelPortUser, buffer, buffer_len , &nBytes);
    epicsMutexUnlock(sendingLock);
    epicsMutexLock(printLock);
    //asynPrint(this->pasynUserSelf, ASYN_TRACEIO_DEVICE, 
    // printf("%s: buffer=%s status=%d nbytes=%d bufflen=%d\n",
    //        functionName, buffer, status, nBytes, buffer_len);
    epicsMutexUnlock(printLock);
    
err_out:
    return status;
}

// this function doesn't block, in order to allow receiving and reading in parallel
asynStatus AMC100Controller::lowlevelRead(char *buffer, size_t buffer_len)
{
    const char *functionName = "AMC100Controller::lowlevelRead";
    double timeout = 300.0;
    int eomReason;
    asynStatus status;
    size_t nBytes;
    ioPvt      *pioPvt = (ioPvt *)lowlevelPortUser->userPvt;

    lowlevelPortUser->timeout = timeout;

    status = pioPvt->pasynOctet->read(
        pioPvt->octetPvt,lowlevelPortUser, buffer, buffer_len, &nBytes, &eomReason);
    epicsMutexLock(printLock);
    //asynPrint(this->pasynUserSelf, ASYN_TRACEIO_DEVICE, 
    // printf("%s: buffer=%s status=%d eomReason=%d nbytes=%d\n",
    //        functionName, buffer, status,  eomReason, nBytes);
    epicsMutexUnlock(printLock);
    return status;
}

bool AMC100Controller::receive(int reqId, char *buffer)
{
    // cond_wait ... 
    const char *functionName = "AMC100Controller::receive";
    assert(reqId >= 0 && reqId < MAX_N_REPLIES);
    epicsEventWait(replyEvents[reqId]);
    epicsMutexLock(replyLocks[reqId]);
    strncpy(buffer, replyBuffers[reqId], RECV_BUFFER_LEN);
    epicsMutexUnlock(replyLocks[reqId]);
    epicsMutexLock(printLock);
    //asynPrint(this->pasynUserSelf, ASYN_TRACE_FLOW,
    // printf("%s: reqId=%d buffer=%s\n",
    //        functionName, reqId, replyBuffers[reqId]);
    epicsMutexUnlock(printLock);
    return true;
}


// Continuously runs in background
void AMC100Controller::receivingTask()
{
    const char *functionName = "receivingTask";
    char buffer[RECV_BUFFER_LEN];
    buffer[RECV_BUFFER_LEN - 1] = 0;
    asynStatus status;
    while (1) {
        status = lowlevelRead(buffer, RECV_BUFFER_LEN - 1);
        if (status == asynSuccess) {
            // parse the data to get req id
            int reqId;
            if (!parseReqId(buffer, &reqId)) {
                printf("%s: Error parsing req id\n", functionName);
                continue;
            }
            //printf("reqId: %d\n", reqId);
            assert(reqId >= 0 && reqId < MAX_N_REPLIES);
            // To protect the buffers
            epicsMutexLock(replyLocks[reqId]);
            strncpy(replyBuffers[reqId], buffer, RECV_BUFFER_LEN);
            epicsMutexUnlock(replyLocks[reqId]);
            epicsEventSignal(replyEvents[reqId]);
        }
    }
}

/** Polls the controller
 */
asynStatus AMC100Controller::poll()
{
	bool result;

    // Poll the connection if it is time
    if(--connectionPollRequired < 0)
    {
        connectionPollRequired = CONNECTIONPOLL;

        if(!initialized)
        {
            initialized = true;
            firstTimeInit();
            result = getFirmwareVer();
        }
        if(!result)
        {
            setIntegerParam(indexConnected, 0);
        }
        else
        {
            setIntegerParam(indexConnected, 1);
        }
    }

    for(int pollAxis=0; pollAxis < numAxes; pollAxis++) {
        AMC100Axis *axis = dynamic_cast<AMC100Axis *>(this->getAxis(pollAxis));
        // TODO: will it always return NULL for a non existent axis?
        if (axis)
            axis->poll();
    }

    callParamCallbacks();

    return asynSuccess;
}

bool AMC100Controller::parseReqId(char *buffer, int *reqId)
{
    rapidjson::Document recvDocument;
    recvDocument.Parse(buffer);
    if (recvDocument.Parse(buffer).HasParseError()) {
        printf("Could not parse buffer json\n");
        return false;
    }

    rapidjson::Value& response = recvDocument["id"];
    if (!response.IsInt()) {
        printf("Didn't return expected type\n");
        return false;
    }
    *reqId = response.GetInt();
    return true;
}

bool AMC100Controller::getFirmwareVer()
{
    bool result = false;
    char recvBuffer[RECV_BUFFER_LEN];
    result = sendCommand("com.attocube.system.getFirmwareVersion",                
                         COMMAND_GET_FIRMWARE_REQID);

    if (!result) {
        printf("write firmware json failed\n");
        return false;
    }

    this->receive(COMMAND_GET_FIRMWARE_REQID, recvBuffer);

    rapidjson::Document recvDocument;
    recvDocument.Parse(recvBuffer);
    if (recvDocument.Parse(recvBuffer).HasParseError()) {
        printf("Could not parse recvBuffer json\n");
        return false;
    }

    rapidjson::Value& response = recvDocument["result"];
    if (!response.IsArray()) {
        printf("Didn't return expected type\n");
        return false;
    }

    const char* firmware = response[0].GetString();
    setStringParam(indexFirmware, firmware);
    
    return result;

}

/** Decode the controller's binary representation of a 32 bit int into a human readable form
 * \param[in] encoded a pointer to 4 bytes encoding the binary representation
 * \param[out] string a string representing the decoded int
*/
void AMC100Controller::IntToString(const unsigned char* encoded, char* string)
{
	// for(int i = 0; i<4; i++) sprintf(string+2*i, "%02x",encoded[i]);
}

/** Transmits binary data to the controller and waits for a return result.
 * \param[in] tx the data to transmit
 * \param[in] txSize the length of tx
 * \param[out] rx the receive buffer - allocated by the caller
 * \param[in] rxSize the size of the recieve buffer
 */
bool AMC100Controller::sendReceive(const char* tx, size_t txSize,
		char* rx, size_t rxSize)
{
    int eomReason;
    size_t bytesOut;
    size_t bytesIn;

    // make sure the previous command was digested - this controller is
    // prone to buffer overrun
    epicsThreadSleep(0.05);


    // pasynOctetSyncIO->flush(lowlevelPortUser);
    asynStatus result = pasynOctetSyncIO->writeRead(lowlevelPortUser, (char*)tx, txSize,
            (char*)rx, rxSize, /*timeout=*/0.1, &bytesOut, &bytesIn, &eomReason);
    
    if(result != asynSuccess) {
        printf("Error calling writeRead, tx=%s result=%d bytesin=%d eomReason=%d inString=%s\n", tx, result, &bytesIn, eomReason, rx);
    }
    else {
        // printf("writeRead successful\n");
        printf("%s",tx);
        printf(rx);
    }

    return result;
}

bool AMC100Controller::setError(int errorNum) {
    bool result = false;
    char recvBuffer[RECV_BUFFER_LEN];
    result = sendCommand("com.attocube.system.errorNumberToString",
                         COMMAND_ERROR_NUM_TO_STRING_REQID, 1, errorNum);
    if (!result) {
        printf("sendCommand json failed\n");
        return false;
    }
    result = receive(COMMAND_ERROR_NUM_TO_STRING_REQID, recvBuffer);

    rapidjson::Document recvDocument;
    recvDocument.Parse(recvBuffer);
    if (recvDocument.Parse(recvBuffer).HasParseError()) {
        printf("Could not parse recvBuffer json\n");
        return false;
    }

    rapidjson::Value& response = recvDocument["result"];
    if (!response.IsArray()) {
        printf("Didn't return expected type\n");
        return false;
    }

    const char* error = response[0].GetString();
    setStringParam(indexError, error);

    return result;

}

/** first Time initialization for future possible requirements
 *
 */
bool AMC100Controller::firstTimeInit()
{
    for(int pollAxis=0; pollAxis < numAxes; pollAxis++) {
        AMC100Axis *axis = dynamic_cast<AMC100Axis *>(this->getAxis(pollAxis));
        // TODO: will it always return NULL for a non existent axis?
	if (axis)
            axis->reconfigure();
    }
	return true;
}

/** An integer parameter has been written
 * \param[in] pasynUser Handle of the user writing the paramter
 * \param[in] value The value written
 */
asynStatus AMC100Controller::writeInt32(asynUser *pasynUser, epicsInt32 value)
{
    int function = pasynUser->reason;

    AMC100Axis* axis = dynamic_cast<AMC100Axis*>(getAxis(pasynUser));
    if(axis != NULL) {
        if (function == indexAmplitude)
            return axis->setAmplitude(value) ? asynSuccess : asynError;
        else if (function == indexFrequency)
            return axis->setFrequency(value) ? asynSuccess : asynError;
        else if (function == indexAxisEnabled)
            return axis->setControlOutput(!!value) ? asynSuccess : asynError;
    }

    // no specific handling of parameter changes required for this class
    return asynMotorController::writeInt32(pasynUser, value);
}



/*******************************************************************************
*
*   The following functions have C linkage and can be called directly or from iocsh
*
*******************************************************************************/

extern "C"
{

/** Create a controller
 * \param[in] portName Asyn port name
 * \param[in] controllerNum The number (address) of the controller in the protocol
 * \param[in] lowlevelPortName Asyn port name of the physical port
 * \param[in] lowlevelPortAddress Asyn address of the physical port (usually 0)
 * \param[in] numAxes Maximum number of axes
 * \param[in] movingPollPeriod The period at which to poll position while moving
 * \param[in] idlePollPeriod The period at which to poll position while not moving
 */
asynStatus AMC100ControllerConfig(const char *portName, int controllerNum,
        const char* lowlevelPortName, int lowlevelPortAddress,
        int numAxes, int movingPollPeriod, int idlePollPeriod)
{
    AMC100Controller* ctlr = new AMC100Controller(portName, controllerNum,
            lowlevelPortName, lowlevelPortAddress, numAxes,
            movingPollPeriod/1000.0, idlePollPeriod/1000.0);
    ctlr = NULL;   // To avoid compiler warning
    return asynSuccess;
}

/** Create an axis
 * param[in] ctlrName Asyn port name of the controller
 * param[in] axisNum The number of this axis
 */
asynStatus AMC100AxisConfig(const char* ctlrName, int axisNum)
{
    asynStatus result = asynSuccess;
    AMC100Controller* ctlr = (AMC100Controller*)findAsynPortDriver(ctlrName);
    if(ctlr == NULL)
    {
        result = asynError;
    }
    else
    {
        AMC100Axis* axis = new AMC100Axis(ctlr, axisNum);
        axis = NULL; // To avoid compiler warning
    }
    return result;
}

}

/*******************************************************************************
*
*   Registering the IOC shell functions
*
*******************************************************************************/

static const iocshArg AMC100ControllerConfigArg0 = {"port name", iocshArgString};
static const iocshArg AMC100ControllerConfigArg1 = {"controller number", iocshArgInt};
static const iocshArg AMC100ControllerConfigArg2 = {"low level port name", iocshArgString};
static const iocshArg AMC100ControllerConfigArg3 = {"low level port address", iocshArgInt};
static const iocshArg AMC100ControllerConfigArg4 = {"number of axes", iocshArgInt};
static const iocshArg AMC100ControllerConfigArg5 = {"moving poll period (ms)", iocshArgInt};
static const iocshArg AMC100ControllerConfigArg6 = {"idle poll period (ms)", iocshArgInt};

static const iocshArg *const AMC100ControllerConfigArgs[] =
{
    &AMC100ControllerConfigArg0, &AMC100ControllerConfigArg1,
    &AMC100ControllerConfigArg2, &AMC100ControllerConfigArg3,
    &AMC100ControllerConfigArg4, &AMC100ControllerConfigArg5,
    &AMC100ControllerConfigArg6
};
static const iocshFuncDef AMC100ControllerConfigDef =
    {"AMC100ControllerConfig", 7, AMC100ControllerConfigArgs};

static void AMC100ControllerConfigCallFunc(const iocshArgBuf *args)
{
    AMC100ControllerConfig(args[0].sval, args[1].ival, args[2].sval, args[3].ival,
            args[4].ival, args[5].ival, args[6].ival);
}

static const iocshArg AMC100AxisConfigArg0 = {"controller port name", iocshArgString};
static const iocshArg AMC100AxisConfigArg1 = {"axis number", iocshArgInt};

static const iocshArg *const AMC100AxisConfigArgs[] =
{
    &AMC100AxisConfigArg0, &AMC100AxisConfigArg1,
};
static const iocshFuncDef AMC100AxisConfigDef =
    {"AMC100AxisConfig", 2, AMC100AxisConfigArgs};

static void AMC100AxisConfigCallFunc(const iocshArgBuf *args)
{
    AMC100AxisConfig(args[0].sval, args[1].ival);
}

static void AMC100Register(void)
{
    iocshRegister(&AMC100ControllerConfigDef, AMC100ControllerConfigCallFunc);
    iocshRegister(&AMC100AxisConfigDef, AMC100AxisConfigCallFunc);
}
epicsExportRegistrar(AMC100Register);


