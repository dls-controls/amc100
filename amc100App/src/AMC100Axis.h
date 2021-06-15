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
    void reconfigure();
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
    bool setAmplitude(int amplitude);
    bool getPosition();
    bool getReferencePosition();
    bool getFrequency();
    bool setFrequency(int frequency);
    bool getStatusConnected();
    bool getStatusReference();
    bool getStatusMoving();
    bool setControlAutoReset(bool enable);
    bool setControlMove(bool enable);
    bool getControlOutput();
    bool setControlOutput(bool enable);

private:
    /* Data */
    int connectionPollRequired;
    AMC100Controller* controller;
    int axisNum;
    bool initialized;
    unsigned int _pollCounter;
    double lastPosition;
};

#endif /* AMC_100_AXIS_H_ */