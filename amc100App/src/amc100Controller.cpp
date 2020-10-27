/*
 * amc100Controller.cpp
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
#include "amc100Controller.h"
#include "amc100Axis.h"
#include "asynOctetSyncIO.h"
#include "asynCommonSyncIO.h"


/*******************************************************************************
*
*   The amc100 controller class
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
amc100Controller::amc100Controller(const char* portName, int controllerNum,
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
        printf("amc100Controller: Failed to connect to serial port %s\n", serialPortName);
        asynPrint(this->pasynUserSelf, ASYN_TRACE_ERROR,
                "amc100Controller: Failed to connect to serial port %s\n",
                                serialPortName);
    }
    else
    {
        printf("amc100Controller: connected to serial port %s\n", serialPortName);
        asynPrint(this->pasynUserSelf, ASYN_TRACE_FLOW,
                "amc100Controller: connected to serial port %s\n", serialPortName);
    }

    // Create the poller thread
    startPoller(movingPollPeriod, idlePollPeriod, /*forcedFastPolls-*/10);
}

/** Destructor
 */
amc100Controller::~amc100Controller()
{
}

/** Polls the controller
 */
asynStatus amc100Controller::poll()
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
        result = readFirmwareVer();
        for(int pollAxis=0; pollAxis < numAxes; pollAxis++) {
            amc100Axis *axis = dynamic_cast<amc100Axis *>(this->getAxis(pollAxis));
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

bool amc100Controller::readFirmwareVer()
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

// bool amc100Controller::readAmplitude(int axisNum) {
//     bool result = false;

//     rapidjson::StringBuffer string_buffer;
//     rapidjson::Writer<rapidjson::StringBuffer> writer(string_buffer);
//     char recvBuffer[256];
//     writer.StartObject();
//     writer.String("jsonrpc");
//     writer.String("2.0");
//     writer.String("method");
//     writer.String("com.attocube.amc.control.getControlAmplitude");
//     writer.String("params");
//     writer.StartArray();
//     writer.Uint64(axisNum);
//     writer.EndArray();
//     writer.String("id");
//     writer.Uint64(idReq);
//     idReq++;
//     writer.EndObject();

//     result = sendReceive(string_buffer.GetString(), string_buffer.GetSize(), recvBuffer, sizeof(recvBuffer));
//     if (!result) {
//         printf("sendReceive failed\n");
//         return false;
//     }

//     rapidjson::Document recvDocument;
//     recvDocument.Parse(recvBuffer);
//     if (recvDocument.Parse(recvBuffer).HasParseError()) {
//         printf("Could not parse recvBuffer json\n");
//         return false;
//     }

//     rapidjson::Value& response = recvDocument["result"];
//     if (!response.IsArray() || response.Size() != 2) {
//         printf("Didn't return expected type\n");
//         return false;
//     }

//     int errorNum = response[0].GetInt();
//     setIntegerParam(indexError, errorNum);
//     int amplitude = response[1].GetDouble();
//     setIntegerParam(indexAmplitude, amplitude);
    
//     return result;
// }

/** Decode the controller's binary representation of a 32 bit int into a human readable form
 * \param[in] encoded a pointer to 4 bytes encoding the binary representation
 * \param[out] string a string representing the decoded int
*/
void amc100Controller::IntToString(const unsigned char* encoded, char* string)
{
	// for(int i = 0; i<4; i++) sprintf(string+2*i, "%02x",encoded[i]);
}

/** Transmits binary data to the controller and waits for a return result.
 * \param[in] tx the data to transmit
 * \param[in] txSize the length of tx
 * \param[out] rx the receive buffer - allocated by the caller
 * \param[in] rxSize the size of the recieve buffer
 */
bool amc100Controller::sendReceive(const char* tx, size_t txSize,
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
        // printf(rx);
    }

    return result;
}

/** first Time initializatin for future possible requirements
 *
 */
bool amc100Controller::firstTimeInit()
{
	// No first time init required for the controller
	return true;
}

// /** Transmits a command to the controller
//  * \param[in] axis the axis number to send the command to
//  * \param[in] command the command string to send
//  * \param[in] parms pointer to encoded parameter buffer
//  * \param[in] pLen the length of parms buffer
//  * \param[out] response buffer for encoded response - allocated by caller
//  * \param[in] rLen length of response buffer
//  */
// bool amc100Controller::command(unsigned char axis, unsigned char command,
// 		const unsigned char* parms, size_t pLen, unsigned char* response, size_t rLen)
// {
// 	unsigned char txBuffer[TXBUFFERSIZE];
//     bool result = false;

//     txBuffer[0] = axis;
//     txBuffer[1] = command;
//     for(int i = 0; i< pLen; i++) txBuffer[2+i] = parms[i];

//     // add one to rLen for the leading ACK
//     result = sendReceive(txBuffer, pLen+2, response, rLen+1);

//     // check for the ACK
//     if(response[0] != 0x06)
//     {
//         asynPrint(this->pasynUserSelf, ASYN_TRACE_ERROR,
//                 "Error on axis %02x, command %02x, response %02x\n",axis, command, response[0]);
//     }

//     // remove the ACK
//     for(int i = 0; i< rLen; i++)
//     	response[i]=response[i+1];

//     return result;
// }



/** An integer parameter has been written
 * \param[in] pasynUser Handle of the user writing the paramter
 * \param[in] value The value written
 */
asynStatus amc100Controller::writeInt32(asynUser *pasynUser, epicsInt32 value)
{
	/*
    int function = pasynUser->reason;

    amc100Axis* axis = dynamic_cast<amc100Axis*>(getAxis(pasynUser));
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
asynStatus amc100ControllerConfig(const char *portName, int controllerNum,
        const char* serialPortName, int serialPortAddress,
        int numAxes, int movingPollPeriod, int idlePollPeriod)
{
    amc100Controller* ctlr = new amc100Controller(portName, controllerNum,
            serialPortName, serialPortAddress, numAxes,
            movingPollPeriod/1000.0, idlePollPeriod/1000.0);
    ctlr = NULL;   // To avoid compiler warning
    return asynSuccess;
}

/** Create an axis
 * param[in] ctlrName Asyn port name of the controller
 * param[in] axisNum The number of this axis
 */
asynStatus amc100AxisConfig(const char* ctlrName, int axisNum)
{
    asynStatus result = asynSuccess;
    amc100Controller* ctlr = (amc100Controller*)findAsynPortDriver(ctlrName);
    if(ctlr == NULL)
    {
        result = asynError;
    }
    else
    {
        amc100Axis* axis = new amc100Axis(ctlr, axisNum);
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

static const iocshArg amc100ControllerConfigArg0 = {"port name", iocshArgString};
static const iocshArg amc100ControllerConfigArg1 = {"controller number", iocshArgInt};
static const iocshArg amc100ControllerConfigArg2 = {"serial port name", iocshArgString};
static const iocshArg amc100ControllerConfigArg3 = {"serial port address", iocshArgInt};
static const iocshArg amc100ControllerConfigArg4 = {"number of axes", iocshArgInt};
static const iocshArg amc100ControllerConfigArg5 = {"moving poll period (ms)", iocshArgInt};
static const iocshArg amc100ControllerConfigArg6 = {"idle poll period (ms)", iocshArgInt};

static const iocshArg *const amc100ControllerConfigArgs[] =
{
    &amc100ControllerConfigArg0, &amc100ControllerConfigArg1,
    &amc100ControllerConfigArg2, &amc100ControllerConfigArg3,
    &amc100ControllerConfigArg4, &amc100ControllerConfigArg5,
    &amc100ControllerConfigArg6
};
static const iocshFuncDef amc100ControllerConfigDef =
    {"amc100ControllerConfig", 7, amc100ControllerConfigArgs};

static void amc100ControllerConfigCallFunc(const iocshArgBuf *args)
{
    amc100ControllerConfig(args[0].sval, args[1].ival, args[2].sval, args[3].ival,
            args[4].ival, args[5].ival, args[6].ival);
}

static const iocshArg amc100AxisConfigArg0 = {"controller port name", iocshArgString};
static const iocshArg amc100AxisConfigArg1 = {"axis number", iocshArgInt};

static const iocshArg *const amc100AxisConfigArgs[] =
{
    &amc100AxisConfigArg0, &amc100AxisConfigArg1,
};
static const iocshFuncDef amc100AxisConfigDef =
    {"amc100AxisConfig", 2, amc100AxisConfigArgs};

static void amc100AxisConfigCallFunc(const iocshArgBuf *args)
{
    amc100AxisConfig(args[0].sval, args[1].ival);
}

static void amc100Register(void)
{
    iocshRegister(&amc100ControllerConfigDef, amc100ControllerConfigCallFunc);
    iocshRegister(&amc100AxisConfigDef, amc100AxisConfigCallFunc);
}
epicsExportRegistrar(amc100Register);


