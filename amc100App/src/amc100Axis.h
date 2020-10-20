/*
 * amc100Axis.h
 *
 *  Created on: 4 Oct 2011
 *      Author: hgv27681
 */

#ifndef MCSAXIS_H_
#define MCSAXIS_H_

#include "asynMotorController.h"
#include "asynMotorAxis.h"
class amc100Controller;

// global constants
const char amc100AxesNumbers[4] = {0x88,0x8c,0x90,0x94};
const int encoderUpperLimit = 0x00ffffff;
const int encoderLowerLimit = 0xFF000000;

class amc100Axis : public asynMotorAxis
{
private:
	// constants
	enum {CONNECTIONPOLL=5};
public:
	amc100Axis(amc100Controller* ctlr, int axisNum);
    virtual ~amc100Axis();

    // Overridden from asynMotorAxis
    virtual asynStatus move(double position, int relative,
            double minVelocity, double maxVelocity, double acceleration);
    virtual asynStatus moveVelocity(double minVelocity,
            double maxVelocity, double acceleration);
    virtual asynStatus home(double minVelocity,
            double maxVelocity, double acceleration, int forwards);
    virtual asynStatus stop(double acceleration);
    virtual asynStatus poll(bool* moving);

private:
    bool firstTimeInit();

private:
    /* Data */
    int connectionPollRequired;
    amc100Controller* controller;
    int axisNum;
    int curPosition;
    bool initialized;
};


#endif /* MCSAXIS_H_ */
