/*
 * amc100Driver.cpp
 *      Author: sfx44126
 */

#include "amc100Driver.h"
#include "TakeLock.h"
#include "FreeLock.h"

static const char *driverName = "amc100Driver";

amc100Driver::amc100Driver(const char *portName)
: asynPortDriver(portName, // Name of the port we'll create
		1, // Maximum address
		asynOctetMask | asynInt32Mask | asynFloat64Mask | asynDrvUserMask | asynInt32ArrayMask | asynFloat64ArrayMask, // Interfaces
		asynOctetMask | asynInt32Mask | asynFloat64Mask | asynDrvUserMask | asynInt32ArrayMask | asynFloat64ArrayMask, // Interrupts
		1, // Automatically connect on startup
		0, // Default priority
		0 ){ // Default stack size

	static const char *functionName = "amc100Driver";

	// Initialise data members
	lowLevelPortUser_ = NULL;
	pollingCounter = 0;

	// Set up asyn parameters
	createParam(paramFirmwareString, 		asynParamOctet, &paramFirmware_);
	createParam(paramUserCommandString,		asynParamOctet, &paramUserCommand);
	createParam(paramError, 				asynParamInt32, &paramError);

	// Connect to low level port
	const char * inputEos = "\r\n";
	const char * outputEos = "\r\n";
	if (lowLevelPortConnect(lowLevelPortName, lowLevelPortAddress, &lowLevelPortUser_, inputEos, outputEos) != asynSuccess) {
		asynPrint(this->pasynUserSelf, ASYN_TRACE_ERROR,
			      "lowLevelPortConnect failed\n");
	} else {
		// Connected OK
	}

	callParamCallbacks();
}

/*
 * Destructor
 */
amc100Driver::~amc100Driver()

/** Polls the controller
 */
asynStatus amc100Driver::poll()
{
	asynStatus status = asynSuccess;

	// Ideally want to poll something that changes like motor position
	status = getFirmwareVer();

    return asynSuccess;
}

/**
 * Connect to the underlying low level Asyn port that is used for comms.
 * This uses the asynOctetSyncIO interface, and also sets the input and output terminators.
 *
 * @param port - Name of the port to connect to.
 * @param addr - Address to connect to.
 * @param ppasynUser - Pointer to the asyn user structure.
 * @param inputEos - String input EOS.
 * @param outputEos - String output EOS.
 */
int amc100Driver::lowLevelPortConnect(const char *port, int addr, asynUser **ppasynUser, const char *inputEos, const char *outputEos)
{
  const char * functionName = "amc100Driver::lowLevelPortConnect";

  asynStatus status = asynSuccess;

  // Lock the driver while we open the connection
  this->lock();
  // Connect to the low level port
  status = pasynOctetSyncIO->connect( port, addr, ppasynUser, NULL);
  if (status) {
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
//  if (status == asynSuccess)

  // Unlock
  this->unlock();

  return status;
}

asynStatus getFirmwareVer() {
	asynStatus status = asynSuccess;

	const char * functionName = "amc100Driver::getFirmwareVer";

	strcpy(firmwareVersion_, "Unknown");


}