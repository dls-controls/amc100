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


/*******************************************************************************
*
*   The AMC100 controller class
*
*******************************************************************************/

/** Constructor
 * \param[in] portName Asyn port name
 * \param[in] controllerNum The number (address) of the controller in the protocol
 * \param[in] serialPortName Asyn port name of the serial port
 * \param[in] serialPortAddress Asyn address of the serial port (usually 0)
 * \param[in] numAxes Maximum number of axes
 * \param[in] movingPollPeriod The period at which to poll position while moving
 * \param[in] idlePollPeriod The period at which to poll position while not moving
 */
AMC100Controller::AMC100Controller(const char* portName, int controllerNum,
        const char* serialPortName, int serialPortAddress, int numAxes,
        double movingPollPeriod, double idlePollPeriod)
    : asynMotorController(portName, numAxes, /*numParams=*/&lastParam-&firstParam-1,
            /*interfaceMask=*/0, /*interruptMask=*/0,
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
    createParam(indexFrequencyString, asynParamInt32, &indexFrequency);
    createParam(indexFirmwareString, asynParamOctet, &indexFirmware);

    // Initialise our parameters
    setIntegerParam(indexConnected, 0);
    setIntegerParam(indexError, 0);
    setIntegerParam(indexAmplitude, 0);
    setIntegerParam(indexFrequency, 0);
    setStringParam(indexFirmware, "");

    // Connect to the serial port
    asynStatus result = pasynOctetSyncIO->connect(serialPortName, serialPortAddress,
            &serialPortUser, NULL);

    if( result != asynSuccess)
    {
        printf("AMC100Controller: Failed to connect to serial port %s\n", serialPortName);
        asynPrint(this->pasynUserSelf, ASYN_TRACE_ERROR,
                "AMC100Controller: Failed to connect to serial port %s\n",
                                serialPortName);
    }
    else
    {
        printf("AMC100Controller: connected to serial port %s\n", serialPortName);
        asynPrint(this->pasynUserSelf, ASYN_TRACE_FLOW,
                "AMC100Controller: connected to serial port %s\n", serialPortName);
    }

    // Create the poller thread
    startPoller(movingPollPeriod, idlePollPeriod, /*forcedFastPolls-*/10);
}

/** Destructor
 */
AMC100Controller::~AMC100Controller()
{
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
        }
        result = getFirmwareVer();
        for(int pollAxis=0; pollAxis < numAxes; pollAxis++) {
            AMC100Axis *axis = dynamic_cast<AMC100Axis *>(this->getAxis(pollAxis));
            axis->poll();
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

    callParamCallbacks();

    return asynSuccess;
}

bool AMC100Controller::getFirmwareVer()
{
    bool result = false;

    rapidjson::StringBuffer string_buffer;
    rapidjson::Writer<rapidjson::StringBuffer> writer(string_buffer);
    char recvBuffer[256];
    writer.StartObject();
    writer.String("jsonrpc");
    writer.String("2.0");
    writer.String("method");
    writer.String("com.attocube.system.getFirmwareVersion");
    writer.String("params");
    writer.StartArray();
    writer.EndArray();
    writer.String("id");
    writer.Uint64(idReq);
    idReq++;
    writer.EndObject();

    result = sendReceive(string_buffer.GetString(), string_buffer.GetSize(), recvBuffer, sizeof(recvBuffer));
    if (!result) {
        printf("sendReceive firmware json failed\n");
        return false;
    }

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


    // pasynOctetSyncIO->flush(serialPortUser);
    asynStatus result = pasynOctetSyncIO->writeRead(serialPortUser, (char*)tx, txSize,
            (char*)rx, rxSize, /*timeout=*/0.1, &bytesOut, &bytesIn, &eomReason);
    
    if(!result) {
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

    rapidjson::StringBuffer string_buffer;
    rapidjson::Writer<rapidjson::StringBuffer> writer(string_buffer);
    char recvBuffer[256];
    writer.StartObject();
    writer.String("jsonrpc");
    writer.String("2.0");
    writer.String("method");
    writer.String("com.attocube.system.errorNumberToString");
    writer.String("params");
    writer.StartArray();
    writer.Uint64(1);
    writer.Uint64(errorNum);
    writer.EndArray();
    writer.String("id");
    writer.Uint64(idReq);
    idReq++;
    writer.EndObject();

    result = sendReceive(string_buffer.GetString(), string_buffer.GetSize(), recvBuffer, sizeof(recvBuffer));
    if (!result) {
        printf("sendReceive firmware json failed\n");
        return false;
    }

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

/** first Time initializatin for future possible requirements
 *
 */
bool AMC100Controller::firstTimeInit()
{
	// No first time init required for the controller
	return true;
}

/** An integer parameter has been written
 * \param[in] pasynUser Handle of the user writing the paramter
 * \param[in] value The value written
 */
asynStatus AMC100Controller::writeInt32(asynUser *pasynUser, epicsInt32 value)
{
	/*
    int function = pasynUser->reason;

    AMC100Axis* axis = dynamic_cast<AMC100Axis*>(getAxis(pasynUser));
    if(axis != NULL && function == indexCalibrateSensor)
    {
        // Calibrate the sensor of the specified axis
        axis->calibrateSensor(value);
    }
    */

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
 * \param[in] serialPortName Asyn port name of the serial port
 * \param[in] serialPortAddress Asyn address of the serial port (usually 0)
 * \param[in] numAxes Maximum number of axes
 * \param[in] movingPollPeriod The period at which to poll position while moving
 * \param[in] idlePollPeriod The period at which to poll position while not moving
 */
asynStatus AMC100ControllerConfig(const char *portName, int controllerNum,
        const char* serialPortName, int serialPortAddress,
        int numAxes, int movingPollPeriod, int idlePollPeriod)
{
    AMC100Controller* ctlr = new AMC100Controller(portName, controllerNum,
            serialPortName, serialPortAddress, numAxes,
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
static const iocshArg AMC100ControllerConfigArg2 = {"serial port name", iocshArgString};
static const iocshArg AMC100ControllerConfigArg3 = {"serial port address", iocshArgInt};
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


