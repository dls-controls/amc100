/*
 * amc100Axis.h
 *
 */

#ifndef AMC_100_AXIS_H_
#define AMC_100_AXIS_H_

#include "asynMotorController.h"
#include "asynMotorAxis.h"
class amc100Controller;

// global constants
const char amc100AxesNumbers[] = {0, 1, 2};

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

#endif /* AMC_100_AXIS_H_ */