/*
 * AMC100Axis.h
 *
 */

#ifndef AMC_100_AXIS_H_
#define AMC_100_AXIS_H_

#include "asynMotorController.h"
#include "asynMotorAxis.h"
class AMC100Controller;

// global constants
const char AMC100AxesNumbers[] = {0, 1, 2};

class AMC100Axis : public asynMotorAxis
{
private:
	// constants
	enum {CONNECTIONPOLL=5};
public:
	AMC100Axis(AMC100Controller* ctlr, int axisNum);
    virtual ~AMC100Axis();

    // Overridden from asynMotorAxis
    virtual asynStatus move(double position, int relative,
            double minVelocity, double maxVelocity, double acceleration);
    virtual asynStatus moveVelocity(double minVelocity,
            double maxVelocity, double acceleration);
    virtual asynStatus home(double minVelocity,
            double maxVelocity, double acceleration, int forwards);
    virtual asynStatus stop(double acceleration);
    virtual asynStatus poll();
    bool getAmplitude();
    bool getPosition();
    bool getFrequency();
    bool setFrequency(int frequency);
    bool getControlContinuousFwd();
    bool getControlContinuousBkwd();

private:
    bool firstTimeInit();

private:
    /* Data */
    int connectionPollRequired;
    AMC100Controller* controller;
    int axisNum;
    int curPosition;
    bool initialized;
};

#endif /* AMC_100_AXIS_H_ */