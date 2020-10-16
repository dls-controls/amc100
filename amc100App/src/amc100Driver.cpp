/*
 * amc100Driver.cpp
 *      Author: sfx44126
 */

#include <string>
#include <iostream>
#include <cstdlib>
#include <ctime>
#include <stddef.h>
#include <stdlib.h>
#include <stdio.h>
// EPICS
#include <epicsTime.h>
#include <epicsThread.h>
#include <epicsMutex.h>
#include <epicsExport.h>
#include <epicsString.h>
#include <iocsh.h>
#include <drvSup.h>
#include <registryFunction.h>
// Asyn
#include "amc100Driver.h"
#include "asynOctetSyncIO.h"
#include "asynCommonSyncIO.h"

#define amc100_FirmwareString "AMC_FIRMWARE"
#define amc100_UserCommandString "AMC_USRCMD"
#define amc100_LastReplyString "AMC_LASTREPLY"
#define amc100_ErrorString "AMC_ERROR"
// #include "rapidjson.h"

// static const char *driverName = "amc100Driver";

/*
 * Constructor 
 */

amc100Driver::amc100Driver(const char* portName, const char* lowLevelPortName, int lowLevelPortAddress)
	: asynPortDriver(portName, // Name of the port we'll create
		1, // Maximum address
		asynOctetMask | asynInt32Mask | asynFloat64Mask | asynDrvUserMask | asynInt32ArrayMask | asynFloat64ArrayMask, // Interfaces
		asynOctetMask | asynInt32Mask | asynFloat64Mask | asynDrvUserMask | asynInt32ArrayMask | asynFloat64ArrayMask, // Interrupts
		1, // Automatically connect on startup
		0, // Default priority
		0 ){ // Default stack size
	// static const char *functionName = "amc100Driver";

	// Initialise data members
	lowLevelPortUser_ = NULL;

	// Set up asyn parameters
	createParam(amc100_FirmwareString, 			asynParamInt32, &amcFirmware_);
	createParam(amc100_UserCommandString,		asynParamOctet, &amcUserCommand_);
	createParam(amc100_LastReplyString,			asynParamOctet, &amcLastReply_);
	createParam(amc100_ErrorString,				asynParamInt32, &amcError_);

	// Connect to low level port
	const char * inputEos = "\r\n";
	const char * outputEos = "\r\n";
	if (lowLevelPortConnect(lowLevelPortName, lowLevelPortAddress, &lowLevelPortUser_, inputEos, outputEos) != asynSuccess)
	{
		asynPrint(this->pasynUserSelf, ASYN_TRACE_ERROR,
			      "lowLevelPortConnect failed\n");
	}
	else 
	{
		// Connected OK
	}

	callParamCallbacks();
}

/*
 * Destructor
 */
amc100Driver::~amc100Driver()
{
}

/** Polls the controller
 */
// asynStatus amc100Driver::poll()
// {
// 	asynStatus status = asynSuccess;

// 	// Ideally want to poll something that changes like motor position
// 	status = queryVersion();

//     return asynSuccess;
// }

/**
 * Connect to the underlying low level Asyn port that is used for comms.
 * This uses the asynOctetSyncIO interface, and also sets the input and output terminators.
**/

int amc100Driver::lowLevelPortConnect(const char* port, int addr, asynUser** ppasynUser, const char *inputEos, const char *outputEos)
{
  const char* functionName = "amc100Driver::lowLevelPortConnect";

  asynStatus status = asynSuccess;

  // Lock the driver while we open the connection
  this->lock();
  // Connect to the low level port
  status = pasynOctetSyncIO->connect( port, addr, ppasynUser, NULL);
  if (status)
  {
    this->unlock();
    asynPrint(this->pasynUserSelf, ASYN_TRACE_ERROR,
	      "%s: unable to connect to port %s\n",
	      functionName, port);
    return status;
  }
  // Set end of string terminator
  status = pasynOctetSyncIO->setInputEos(*ppasynUser, inputEos, strlen(inputEos));
  status = pasynOctetSyncIO->setOutputEos(*ppasynUser, outputEos, strlen(outputEos));

  // Read the firmware version by way of testing comms
  if (status == asynSuccess)
	status = queryVersion();

  // Unlock
  this->unlock();

  return status;
}

// asynStatus queryVersion() {
// 	asynStatus status = asynSuccess;

// 	char command[64] = {0};
// 	char response[64] = {0};
	
// 	const char json[] = " { \"hello\" : \"world\", \"t\" : true , \"f\" : false, \"n\": null, \"i\":123, \"pi\": 3.1416, \"a\":[1, 2, 3, 4] } ";
// 	MyHandler handler;
// 	Reader reader;
// 	StringStream ss(json);
// 	reader.Parse(ss, handler);
// 	// command = {"jsonrpc"}

// 	sprintf(command, "V");

// 	status = lowLevelWriteRead(command, response, sizeof(response));
// 	if (response[0] != 0 && status == asynSuccess) {
// 		setStringParam (paramFirmware_,  response);
// 	}
// 	return status;
// }

/**
 * Wrapper for asynOctetSyncIO write/read functions.
 * @param command String command to send.
 * @param response String response back.
 * @param maxlen Size of response buffer
 * @param dontLock Locking of driver will be controlled from a higher level. Default false.
 * @param quiet Don't give a communication error on failure
 */
asynStatus amc100Driver::lowLevelWriteRead(const char *command, char *response, size_t maxlen, bool dontLock, bool quiet)
{
	asynStatus status = asynSuccess;

	int eomReason;
	size_t nwrite = 0;
	size_t nread = 0;
	char sendString[2048];
	char recbuf[amc100_MAXBUF_];
	size_t strlen_recbuf;

	memset(recbuf, 0, sizeof(recbuf));
	sprintf(sendString, "%s", command);
	static const char *functionName = "amc100Driver::lowLevelWriteRead";

	if (!lowLevelPortUser_) {

		if (quiet != true){
			asynPrint(this->pasynUserSelf, ASYN_TRACE_ERROR,
					  "no lowLevelPortUser\n");
		}
		return asynError;
	}

	// Lock the driver during comms
	if (dontLock == false)
		this->lock();

	asynPrint(lowLevelPortUser_, ASYN_TRACEIO_DRIVER, "%s: command: %s\n", functionName, command);

	//Make sure the low level port is connected before we attempt comms
	//Use the controller-wide param amc100_C_CommsError_
	status = pasynOctetSyncIO->writeRead(lowLevelPortUser_ ,
				  sendString, strlen(sendString),
				  recbuf, sizeof(recbuf) - 1,
				  amc100_timeout_,
				  &nwrite, &nread, &eomReason );
	strlen_recbuf = strlen(recbuf);
	// Remove trailing newlines/carriage returns
	if (strlen_recbuf > 1) {
		if (recbuf[strlen_recbuf-1] == '\n') {
			--strlen_recbuf;
			recbuf[strlen_recbuf] = '\0';
		}
		if (strlen_recbuf > 1) {
			if (recbuf[strlen_recbuf-1] == '\r') {
				--strlen_recbuf;
				recbuf[strlen_recbuf] = '\0';
			}
		}
	}
	if (strlen_recbuf > maxlen - 1) {
		asynPrint(lowLevelPortUser_, ASYN_TRACE_ERROR, "%s: Buffer too short in pasynOctetSyncIO->writeRead. command: %s recbuff: %s\n", functionName, command, recbuf);
		memcpy(response, recbuf, maxlen - 1);
		response[maxlen - 1] = '\0';
	} else {
		memcpy(response, recbuf, strlen_recbuf + 1); // include '\0'
	}
	if (status) {
		if (quiet!=true) {
			asynPrint(lowLevelPortUser_, ASYN_TRACE_ERROR, "%s: Error (%i) from pasynOctetSyncIO->writeRead. command: %s, response: %s\n", functionName, status, command, response);

		}
	}

	asynPrint(lowLevelPortUser_, ASYN_TRACEIO_DRIVER, "%s: response: %s\n", functionName, response);

	if (dontLock == false)
		this->unlock();

	return status;
}

/**
 * Wrapper for asynOctetSyncIO read function.
 * Wait for and return a single input message.
 * @param response String message received.
 * @param maxlen Size of response buffer
 * @param dontLock Locking of the driver will be controlled from a higher level. Default false.
 */
asynStatus amc100Driver::lowLevelRead(char *response, size_t maxlen, bool dontLock)
{
	asynStatus status = asynSuccess;

	int eomReason;
	size_t nread = 0;
	char recbuf[amc100_MAXBUF_];
	size_t strlen_recbuf;

	memset(recbuf, 0, sizeof(recbuf));
	static const char *functionName = "amc100Driver::lowLevelRead";

	if (!lowLevelPortUser_)
	{
		return asynError;
	}

	// Lock driver for uninterrupted reading
	if (dontLock == false)
		this->lock();

	status = pasynOctetSyncIO->read(lowLevelPortUser_ ,
			recbuf, sizeof(recbuf) - 1,
			amc100_timeout_,
			&nread, &eomReason);
	strlen_recbuf = strlen(recbuf);

	// Strip trailing newline and carriage return characters
	if (strlen_recbuf > 1)
	{
		if (recbuf[strlen_recbuf-1] == '\n')
		{
			--strlen_recbuf;
			recbuf[strlen_recbuf] = '\0';
		}
		if (strlen_recbuf > 1)
		{
			if (recbuf[strlen_recbuf-1] == '\r')
			{
				--strlen_recbuf;
				recbuf[strlen_recbuf] = '\0';
			}
		}
	}
	if (strlen_recbuf > maxlen - 1)
	{
		asynPrint(lowLevelPortUser_, ASYN_TRACE_ERROR, "%s: Buffer too short in pasynOctetSyncIO->read. recbuff: %s\n", functionName, recbuf);
		memcpy(response, recbuf, maxlen - 1);
		response[maxlen - 1] = '\0';
	}
	else
	{
		memcpy(response, recbuf, strlen_recbuf + 1); /* inlcude '\0' */
	}
	if (status)
	{
		asynPrint(lowLevelPortUser_, ASYN_TRACE_FLOW/*ERROR*/, "%s: Error (%i) from pasynOctetSyncIO->read. Response: %s\n", functionName, status, response);
	}

	asynPrint(lowLevelPortUser_, ASYN_TRACEIO_DRIVER, "%s: response: %s\n", functionName, response);

	if (dontLock == false)
		this->unlock();

	return status;
}

extern "C" int amc100DriverCreate(const char *portName, const char* lowLevelPortName, int lowLevelPortAddress)
{
	new amc100Driver(portName, lowLevelPortName, lowLevelPortAddress);
	return(asynSuccess);
}

/* amc100DriverCreate */
static const iocshArg amc100DriverCreateArg0 = {"Controller port name", iocshArgString};
static const iocshArg amc100DriverCreateArg1 = {"Low level port name", iocshArgString};
static const iocshArg amc100DriverCreateArg2 = {"Low level port address", iocshArgInt};
static const iocshArg* const amc100DriverCreateArgs[] = {&amc100DriverCreateArg0,
														&amc100DriverCreateArg1,
														&amc100DriverCreateArg2};
static const iocshFuncDef configamc100Driver = {"amc100DriverCreate", 2, amc100DriverCreateArgs};
static void configamc100DriverCallFunc(const iocshArgBuf *args)
{
	amc100DriverCreate(args[0].sval, args[1].sval, args[2].ival);
}

static void amc100DriverRegister(void)
{
	iocshRegister(&configamc100Driver, configamc100DriverCallFunc);
}

extern "C"
{
	epicsExportRegistrar(amc100DriverRegister);
}