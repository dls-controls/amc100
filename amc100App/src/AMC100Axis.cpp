/*
 * AMC100Axis.cpp
 *
 */

#include <stdlib.h>
#include <epicsThread.h>
#include <cstring>
#include "rapidjson/document.h"
#include "rapidjson/writer.h"
#include "rapidjson/stringbuffer.h"
#include "AMC100Axis.h"
#include "AMC100Controller.h"

#define SLOW_POLL_FREQ_CONST (8)

/** Constructor
 * \param[in] portName Asyn port name
 */
AMC100Axis::AMC100Axis(AMC100Controller* ctlr, int axisNum)
    : asynMotorAxis(ctlr, axisNum)
    , controller(ctlr)
    , axisNum(axisNum)
	, initialized(false)
    , _pollCounter(0)
{
}

// Enabled axes, without this we can't move!
void AMC100Axis::reconfigure()
{
    if (!setControlOutput(true)) {
        printf("setControlOutput failed");
    }
    if (!setControlAutoReset(true)) {
        printf("setControlAutoReset failed");
    }
}

/** Destructor
 */
AMC100Axis::~AMC100Axis()
{
}

/** Poll the axis, start moves when possible, etc.  This function is
 * entered with the lock already on.
 * \param[out] moving Set to TRUE if the axis is moving
 */
asynStatus AMC100Axis::poll()
{
    bool result = false;
    result |= getPosition();
    result |= getStatusMoving();
    result |= getStatusEotFwd();
    result |= getStatusEotBkwd();
    if (_pollCounter % SLOW_POLL_FREQ_CONST == 0) {
        result |= getAmplitude();
        result |= getFrequency();
        result |= getReferencePosition();
        result |= getStatusConnected();
        result |= getStatusReference();
        result |= getControlOutput();
    }
    // TODO: To check for errors

    // setIntegerParam(controller->motorStatusHasEncoder_, 0);
    // setDoubleParam(controller->motorVelocity_, 0.0);
    // setIntegerParam(controller->motorStatusSlip_, 0);
    // setIntegerParam(controller->motorStatusCommsError_, 0);
    // setIntegerParam(controller->motorStatusFollowingError_, 0);
    // setIntegerParam(controller->motorStatusProblem_, 0);

    callParamCallbacks();
    _pollCounter += 1;
    return asynSuccess;
}

// Axis electrically connected to controller
bool AMC100Axis::getStatusConnected() {
    char recvBuffer[256];

    bool result = controller->sendCommand(
        "com.attocube.amc.status.getStatusConnected",
        COMMAND_GET_AXIS_CXN_REQID,
        axisNum);

    if (!result) {
        printf("sendCommand failed\n");
        return false;
    }

    result = controller->receive(COMMAND_GET_AXIS_CXN_REQID, recvBuffer);

    rapidjson::Document recvDocument;
    recvDocument.Parse(recvBuffer);
    if (recvDocument.Parse(recvBuffer).HasParseError()) {
        printf("Could not parse recvBuffer json: %s\n", recvBuffer);
        return false;
    }

    rapidjson::Value& response = recvDocument["result"];
    if (!response.IsArray() || response.Size() != 2) {
        printf("Didn't return expected type\n");
        return false;
    }

    int axisConnected = response[1].GetBool();
    setIntegerParam(controller->indexAxisConnected, axisConnected);
    return result;
}

bool AMC100Axis::getStatusReference() {
    char recvBuffer[256];

    bool result = controller->sendCommand(
        "com.attocube.amc.status.getStatusReference",
        COMMAND_GET_STATUS_REF_REQID,
        axisNum);

    if (!result) {
        printf("sendCommand failed\n");
        return false;
    }

    result = controller->receive(COMMAND_GET_STATUS_REF_REQID, recvBuffer);

    rapidjson::Document recvDocument;
    recvDocument.Parse(recvBuffer);
    if (recvDocument.Parse(recvBuffer).HasParseError()) {
        printf("Could not parse recvBuffer json: %s\n", recvBuffer);
        return false;
    }

    rapidjson::Value& response = recvDocument["result"];
    if (!response.IsArray() || response.Size() != 2) {
        printf("Didn't return expected type\n");
        return false;
    }

    int statusRef = response[1].GetBool();
    setIntegerParam(controller->indexStatusReference, statusRef);
    setIntegerParam(controller->motorStatusHomed_, statusRef);
    return result;

}

bool AMC100Axis::getStatusMoving() {
    char recvBuffer[256];

    bool result = controller->sendCommand(
        "com.attocube.amc.status.getStatusMoving",
        COMMAND_GET_STATUS_REQID,
        axisNum);

    if (!result) {
        printf("sendCommand failed\n");
        return false;
    }

    result = controller->receive(COMMAND_GET_STATUS_REQID, recvBuffer);

    rapidjson::Document recvDocument;
    recvDocument.Parse(recvBuffer);
    if (recvDocument.Parse(recvBuffer).HasParseError()) {
        printf("Could not parse recvBuffer json: %s\n", recvBuffer);
        return false;
    }

    rapidjson::Value& response = recvDocument["result"];
    if (!response.IsArray() || response.Size() != 2) {
        printf("Didn't return expected type\n");
        return false;
    }

    // int errorNum = response[0].GetInt();
    // setIntegerParam(indexError, error);
    int moving = response[1].GetInt();
    int moving_on_demand = moving == 1 ? 1 : 0;
    setIntegerParam(controller->motorStatusDone_, !moving_on_demand);
    setIntegerParam(controller->motorStatusMoving_, moving_on_demand);
    return result;

}

bool AMC100Axis::setControlMove(bool enable) {
    return controller->sendCommand(
        "com.attocube.amc.control.setControlMove",
        COMMAND_SET_CONTROL_MOVE_REQID,
        axisNum,
        enable);
}

bool AMC100Axis::setControlAutoReset(bool enable) {
    return controller->sendCommand(
        "com.attocube.amc.control.setControlAutoReset",
        COMMAND_SET_CONTROL_AUTO_RESET_REQID,
        axisNum,
        enable);
}

bool AMC100Axis::getControlOutput() {
    char recvBuffer[256];

    bool result = controller->sendCommand(
        "com.attocube.amc.control.getControlOutput", 
        COMMAND_GET_AMPLITUDE_REQID,
        axisNum);

    if (!result) {
        printf("sendCommand failed\n");
        return false;
    }

    result = controller->receive(COMMAND_GET_AMPLITUDE_REQID, recvBuffer);

    rapidjson::Document recvDocument;
    recvDocument.Parse(recvBuffer);
    if (recvDocument.Parse(recvBuffer).HasParseError()) {
        printf("Could not parse recvBuffer json\n");
        return false;
    }

    rapidjson::Value& response = recvDocument["result"];
    if (!response.IsArray() || response.Size() != 2) {
        printf("Didn't return expected type\n");
        return false;
    }

    // int errorNum = response[0].GetInt();
    // setIntegerParam(indexError, error);
    int axisEnabledStatus = response[1].GetBool();
    setIntegerParam(controller->indexAxisEnabled, axisEnabledStatus);
    return result;
}

bool AMC100Axis::setControlOutput(bool enable) {
    bool result = controller->sendCommand(
        "com.attocube.amc.control.setControlOutput",
        COMMAND_SET_CONTROL_OUTPUT_REQID,
        axisNum,
        enable);

    if (!result) {
        printf("sendReceive failed\n");
        return false;
    }
    return true;
}

bool AMC100Axis::getAmplitude() {
    char recvBuffer[256];

    bool result = controller->sendCommand(
        "com.attocube.amc.control.getControlAmplitude", 
        COMMAND_GET_AMPLITUDE_REQID,
        axisNum);

    if (!result) {
        printf("sendCommand failed\n");
        return false;
    }

    result = controller->receive(COMMAND_GET_AMPLITUDE_REQID, recvBuffer);

    rapidjson::Document recvDocument;
    recvDocument.Parse(recvBuffer);
    if (recvDocument.Parse(recvBuffer).HasParseError()) {
        printf("Could not parse recvBuffer json\n");
        return false;
    }

    rapidjson::Value& response = recvDocument["result"];
    if (!response.IsArray() || response.Size() != 2) {
        printf("Didn't return expected type\n");
        return false;
    }

    // int errorNum = response[0].GetInt();
    // setIntegerParam(indexError, error);
    int amplitude = response[1].GetInt();
    setIntegerParam(controller->indexAmplitude, amplitude / 1000);
    return result;
}

bool AMC100Axis::setAmplitude(int amplitude) {
    char recvBuffer[RECV_BUFFER_LEN];
    bool result = controller->sendCommand(
        "com.attocube.amc.control.setControlAmplitude",
        COMMAND_SET_AMPLITUDE_REQID,
        axisNum,
        amplitude * 1000);

    if (!result) {
        printf("sendCommand failed\n");
        return false;
    }

    result = controller->receive(COMMAND_SET_FREQUENCY_REQID, recvBuffer);

    rapidjson::Document recvDocument;
    recvDocument.Parse(recvBuffer);
    if (recvDocument.Parse(recvBuffer).HasParseError()) {
        printf("Could not parse recvBuffer json\n");
        return false;
    }

    rapidjson::Value& response = recvDocument["result"];
    if (!response.IsArray() || response.Size() != 1) {
        printf("Didn't return expected type\n");
        return false;
    }

    return result;
}

bool AMC100Axis::getFrequency() {
    char recvBuffer[256];

    bool result = controller->sendCommand(
        "com.attocube.amc.control.getControlFrequency", 
        COMMAND_GET_FREQUENCY_REQID,
        axisNum);

    if (!result) {
        printf("sendCommand failed\n");
        return false;
    }

    result = controller->receive(COMMAND_GET_FREQUENCY_REQID, recvBuffer);

    rapidjson::Document recvDocument;
    recvDocument.Parse(recvBuffer);
    if (recvDocument.Parse(recvBuffer).HasParseError()) {
        printf("Could not parse recvBuffer json\n");
        return false;
    }

    rapidjson::Value& response = recvDocument["result"];
    if (!response.IsArray() || response.Size() != 2) {
        printf("Didn't return expected type\n");
        return false;
    }

    // int errorNum = response[0].GetInt();
    // setIntegerParam(indexError, error);
    int frequency = response[1].GetInt();
    setIntegerParam(controller->indexFrequency, frequency / 1000);
    return result;
}

bool AMC100Axis::setFrequency(int frequency) {
    char recvBuffer[RECV_BUFFER_LEN];
    bool result = controller->sendCommand(
        "com.attocube.amc.control.setControlFrequency",
        COMMAND_SET_FREQUENCY_REQID,
        axisNum,
        frequency * 1000);

    if (!result) {
        printf("sendCommand failed\n");
        return false;
    }

    result = controller->receive(COMMAND_SET_FREQUENCY_REQID, recvBuffer);

    rapidjson::Document recvDocument;
    recvDocument.Parse(recvBuffer);
    if (recvDocument.Parse(recvBuffer).HasParseError()) {
        printf("Could not parse recvBuffer json\n");
        return false;
    }

    rapidjson::Value& response = recvDocument["result"];
    if (!response.IsArray() || response.Size() != 1) {
        printf("Didn't return expected type\n");
        return false;
    }

    return result;
}

/** Move axis command
 * \param[in] position Where to move to
 * \param[in] relative Non-zero for a relative move
 * \param[in] minVelocity The minimum velocity during the move
 * \param[in] maxVelocity The maximum velocity during the move
 * \param[in] acceleration The acceleration to use
 */
asynStatus AMC100Axis::move(double position, int relative,
        double minVelocity, double maxVelocity, double acceleration)
{
    bool result = setControlMove(true);
    result &= controller->sendCommand(
        "com.attocube.amc.move.setControlTargetPosition",
        COMMAND_MOVE_REQID,
        axisNum,
        (double) position);

    if (!result) {
        printf("Move failed\n");
        return asynError;
    }

    return result ? asynSuccess : asynError;
}

bool AMC100Axis::getPosition()
{
    char recvBuffer[RECV_BUFFER_LEN];
    bool result = controller->sendCommand(
    "com.attocube.amc.move.getPosition",
    COMMAND_GET_POSITION_REQID,
    axisNum);

    if (!result) {
        printf("sendCommand failed\n");
        return false;
    }

    controller->receive(COMMAND_GET_POSITION_REQID, recvBuffer);

    rapidjson::Document recvDocument;
    
    char *recvPtr = (char *) memchr(recvBuffer, '{', sizeof(recvBuffer));
    // skip spaces and new line characters at the beginin
    if (!recvPtr) {
        printf("Unexpected reply: %s\n", recvBuffer);
        return false;
    }
    rapidjson::ParseResult parseResult = recvDocument.Parse(recvPtr);
    //recvDocument.Parse(recvBuffer);
    if (!parseResult) {
        printf("Could not parse recvBuffer json: %s\n", recvPtr);
        return false;
    }

    rapidjson::Value& response = recvDocument["result"];
    if (!response.IsArray() || response.Size() != 2) {
        printf("Didn't return expected type\n");
        return false;
    }

    // int errorNum = response[0].GetInt();
    // setIntegerParam(indexError, error);
    double position = response[1].GetDouble();
    setDoubleParam(controller->motorEncoderPosition_, position);
    setDoubleParam(controller->motorPosition_, position);
    lastPosition = position;

    return result;
}

bool AMC100Axis::getReferencePosition()
{
    char recvBuffer[RECV_BUFFER_LEN];
    bool result = controller->sendCommand(
        "com.attocube.amc.control.getReferencePosition",
        COMMAND_GET_REF_POSITION_REQID,
        axisNum);

    if (!result) {
        printf("sendCommand failed\n");
        return false;
    }

    controller->receive(COMMAND_GET_REF_POSITION_REQID, recvBuffer);

    rapidjson::Document recvDocument;
    
    char *recvPtr = (char *) memchr(recvBuffer, '{', sizeof(recvBuffer));
    // skip spaces and new line characters at the beginning
    if (!recvPtr) {
        printf("Unexpected reply: %s\n", recvBuffer);
        return false;
    }
    rapidjson::ParseResult parseResult = recvDocument.Parse(recvPtr);
    //recvDocument.Parse(recvBuffer);
    if (!parseResult) {
        printf("Could not parse recvBuffer json: %s\n", recvPtr);
        return false;
    }

    rapidjson::Value& response = recvDocument["result"];
    if (!response.IsArray() || response.Size() != 2) {
        printf("Didn't return expected type\n");
        return false;
    }

    double refPosition = response[1].GetDouble();
    setDoubleParam(controller->indexAxisRefPosition, refPosition);

    return result;

}

bool AMC100Axis::getStatusEotFwd()
{
    char recvBuffer[RECV_BUFFER_LEN];
    bool result = controller->sendCommand(
        "com.attocube.amc.status.getStatusEotFwd",
        COMMAND_GET_STATUS_EOT_FWD_REQID,
        axisNum);

    if (!result) {
        printf("sendCommand failed\n");
        return false;
    }

    controller->receive(COMMAND_GET_STATUS_EOT_FWD_REQID, recvBuffer);

    rapidjson::Document recvDocument;
    
    char *recvPtr = (char *) memchr(recvBuffer, '{', sizeof(recvBuffer));
    // skip spaces and new line characters at the beginning
    if (!recvPtr) {
        printf("Unexpected reply: %s\n", recvBuffer);
        return false;
    }
    rapidjson::ParseResult parseResult = recvDocument.Parse(recvPtr);
    //recvDocument.Parse(recvBuffer);
    if (!parseResult) {
        printf("Could not parse recvBuffer json: %s\n", recvPtr);
        return false;
    }

    rapidjson::Value& response = recvDocument["result"];
    if (!response.IsArray() || response.Size() != 2) {
        printf("Didn't return expected type\n");
        return false;
    }

    bool fwd_limit_reached = response[1].GetBool();
    setIntegerParam(controller->motorStatusHighLimit_, fwd_limit_reached);

    return result;
}

bool AMC100Axis::getStatusEotBkwd()
{
        char recvBuffer[RECV_BUFFER_LEN];
    bool result = controller->sendCommand(
        "com.attocube.amc.status.getStatusEotBkwd",
        COMMAND_GET_STATUS_EOT_BKWD_REQID,
        axisNum);

    if (!result) {
        printf("sendCommand failed\n");
        return false;
    }

    controller->receive(COMMAND_GET_STATUS_EOT_BKWD_REQID, recvBuffer);

    rapidjson::Document recvDocument;
    
    char *recvPtr = (char *) memchr(recvBuffer, '{', sizeof(recvBuffer));
    // skip spaces and new line characters at the beginning
    if (!recvPtr) {
        printf("Unexpected reply: %s\n", recvBuffer);
        return false;
    }
    rapidjson::ParseResult parseResult = recvDocument.Parse(recvPtr);
    //recvDocument.Parse(recvBuffer);
    if (!parseResult) {
        printf("Could not parse recvBuffer json: %s\n", recvPtr);
        return false;
    }

    rapidjson::Value& response = recvDocument["result"];
    if (!response.IsArray() || response.Size() != 2) {
        printf("Didn't return expected type\n");
        return false;
    }

    bool bkwd_limit_reached = response[1].GetBool();
    setIntegerParam(controller->motorStatusLowLimit_, bkwd_limit_reached);

    return result;
}

/** Jog axis command
 * \param[in] minVelocity The minimum velocity during the move
 * \param[in] maxVelocity The maximum velocity during the move
 * \param[in] acceleration The acceleration to use
 */
asynStatus AMC100Axis::moveVelocity(double minVelocity,
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
asynStatus AMC100Axis::home(double minVelocity, double maxVelocity,
        double acceleration, int forwards)
{
    return asynSuccess;
}

/** Stop axis command
 * \param[in] acceleration The acceleration to use
 */
asynStatus AMC100Axis::stop(double acceleration)
{
    if (!setControlMove(false))
        return asynError;
    return asynSuccess;
}
