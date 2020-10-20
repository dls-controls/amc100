/*
 * amc100Axis.cpp
 *
 *  Created on: 4 Oct 2011
 *      Author: hgv27681
 */

#include <stdlib.h>
#include "amc100Axis.h"
#include "amc100Controller.h"
#include <epicsThread.h>

/** Constructor
 * \param[in] portName Asyn port name
 */
amc100Axis::amc100Axis(amc100Controller* ctlr, int axisNum)
    : asynMotorAxis(ctlr, axisNum)
    , controller(ctlr)
    , axisNum(axisNum)
	, initialized(false)
{
}

/** Destructor
 */
amc100Axis::~amc100Axis()
{
}

/** Move axis command
 * \param[in] position Where to move to
 * \param[in] relative Non-zero for a relative move
 * \param[in] minVelocity The minimum velocity during the move
 * \param[in] maxVelocity The maximum velocity during the move
 * \param[in] acceleration The acceleration to use
 */
asynStatus amc100Axis::move(double position, int relative,
        double minVelocity, double maxVelocity, double acceleration)
{
	bool result = false;
	unsigned char txBuffer[controller->TXBUFFERSIZE];
	unsigned char rxBuffer[controller->RXBUFFERSIZE];

	// Brute force attempt to ensure that the controller is in a state to accept the
	// move command even if it has been rebooted or reset
	// TODO a better solution would be to check status and perform this step if required
	//  controller->command(amc100AxesNumbers[axisNum],cmdSetEncLimits,txBuffer,8,rxBuffer,0);
	//  controller->command(amc100AxesNumbers[axisNum],cmdSetSilentPark,txBuffer,1,rxBuffer,0);
	controller->command(amc100AxesNumbers[axisNum],cmdTargetModeOn,txBuffer,0,rxBuffer,0);

	// request the new Target position
	controller->EncodeInt((int)position,txBuffer);
	result = controller->command(amc100AxesNumbers[axisNum],cmdSetPos,txBuffer,4,rxBuffer,0);

    return asynSuccess;
}

/** Jog axis command
 * \param[in] minVelocity The minimum velocity during the move
 * \param[in] maxVelocity The maximum velocity during the move
 * \param[in] acceleration The acceleration to use
 */
asynStatus amc100Axis::moveVelocity(double minVelocity,
        double maxVelocity, double acceleration)
{
    return asynSuccess;
}

/** Home axis command
 * \param[in] minVelocity The minimum velocity during the move
 * \param[in] maxVelocity The maximum velocity during the move
 * \param[in] acceleration The acceleration to use
 * \param[in] forwards Set to TRUE to home forwards
 */
asynStatus amc100Axis::home(double minVelocity, double maxVelocity,
        double acceleration, int forwards)
{
    return asynSucces;
}

/** Stop axis command
 * \param[in] acceleration The acceleration to use
 */
asynStatus amc100Axis::stop(double acceleration)
{
	bool result = true;
	unsigned char rxBuffer[controller->RXBUFFERSIZE];

    return result ? asynSuccess : asynError;
}

/** Poll the axis, start moves when possible, etc.  This function is
 * entered with the lock already on.
 * \param[out] moving Set to TRUE if the axis is moving
 */
asynStatus amc100Axis::poll(bool* moving)
{
    unsigned char rxBuffer[controller->RXBUFFERSIZE];

    if(!initialized)
    {
    	initialized = firstTimeInit();
    }

    if(controller->command(amc100AxesNumbers[axisNum],cmdReadPos,NULL,0,rxBuffer,4))
    {
		curPosition = controller->DecodeInt32(rxBuffer);

		setDoubleParam(controller->motorEncoderPosition_, (double)curPosition);
		setDoubleParam(controller->motorPosition_, (double)curPosition);
    }

    if(controller->command(amc100AxesNumbers[axisNum],cmdReadStatus,NULL,0,rxBuffer,1))
    {
    	// TODO - stuff like this should probably be defined in controller class
    	// AND the command function should be overloaded - not require the caller to use Decode etc.
    	char status = rxBuffer[0];
    	bool running = (status & 1);
    	// bool targetPass = (status & 4);
    	// bool encoderLimit = (status & 64);
    	// bool error = (status & 128);

        setIntegerParam(controller->motorStatusDone_, !running);
        setIntegerParam(controller->motorStatusMoving_, running);
    }

    setIntegerParam(controller->motorStatusHighLimit_, 0);
    setIntegerParam(controller->motorStatusLowLimit_, 0);
    setIntegerParam(controller->motorStatusHasEncoder_, 0);
    setDoubleParam(controller->motorVelocity_, 0.0);
    setIntegerParam(controller->motorStatusSlip_, 0);
    setIntegerParam(controller->motorStatusCommsError_, 0);
    setIntegerParam(controller->motorStatusFollowingError_, 0);
    setIntegerParam(controller->motorStatusProblem_, 0);

    callParamCallbacks();

    return asynSuccess;
}

/** First time initialization
 * sets up the following settings
 *  Clear all errors
 *  Encoder Limits - to widest range possible (required for motion to be possible before homing)
 *  read back encoder limits to verify they have been set
 *  unpark
 *  switch to closed loop target position mode
 */
bool amc100Axis::firstTimeInit()
{
	bool result = true;

	return result;
}

