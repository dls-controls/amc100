/*
 * AMC100Axis.cpp
 *
 */

#include <stdlib.h>
#include <epicsThread.h>
#include "rapidjson/document.h"
#include "rapidjson/writer.h"
#include "rapidjson/stringbuffer.h"
#include "AMC100Axis.h"
#include "AMC100Controller.h"

/** Constructor
 * \param[in] portName Asyn port name
 */
AMC100Axis::AMC100Axis(AMC100Controller* ctlr, int axisNum)
    : asynMotorAxis(ctlr, axisNum)
    , controller(ctlr)
    , axisNum(axisNum)
	, initialized(false)
{
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
    // result = getAmplitude();
    // result |= getFrequency();
    result |= getPosition();
    result |= getStatusMoving();

    // TODO: To check for errors
    // setIntegerParam(controller->motorStatusHighLimit_, 0);
    // setIntegerParam(controller->motorStatusLowLimit_, 0);
    // setIntegerParam(controller->motorStatusHasEncoder_, 0);
    // setDoubleParam(controller->motorVelocity_, 0.0);
    // setIntegerParam(controller->motorStatusSlip_, 0);
    // setIntegerParam(controller->motorStatusCommsError_, 0);
    // setIntegerParam(controller->motorStatusFollowingError_, 0);
    // setIntegerParam(controller->motorStatusProblem_, 0);

    callParamCallbacks();

    return asynSuccess;
}

bool AMC100Axis::getStatusMoving() {

    bool result = false;

        rapidjson::StringBuffer string_buffer;
    rapidjson::Writer<rapidjson::StringBuffer> writer(string_buffer);
    char recvBuffer[256];
    writer.StartObject();
    writer.String("jsonrpc");
    writer.String("2.0");
    writer.String("method");
    writer.String("com.attocube.amc.status.getStatusMoving");
    writer.String("params");
    writer.StartArray();
    writer.Uint64(axisNum);
    writer.EndArray();
    writer.String("id");
    writer.Uint64(controller->idReq);
    (controller->idReq)++;
    writer.EndObject();

    result = controller->sendReceive(string_buffer.GetString(), string_buffer.GetSize(), recvBuffer, sizeof(recvBuffer));
    if (!result) {
        printf("sendReceive failed\n");
        return false;
    }

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
    int moving = response[1].GetInt();
    int moving_on_demand = moving == 1 ? 1 : 0;
    setIntegerParam(controller->motorStatusDone_, !moving_on_demand);
    setIntegerParam(controller->motorStatusMoving_, moving_on_demand);
    return result;

}

bool AMC100Axis::getAmplitude() {
    bool result = false;

    rapidjson::StringBuffer string_buffer;
    rapidjson::Writer<rapidjson::StringBuffer> writer(string_buffer);
    char recvBuffer[256];
    writer.StartObject();
    writer.String("jsonrpc");
    writer.String("2.0");
    writer.String("method");
    writer.String("com.attocube.amc.control.getControlAmplitude");
    writer.String("params");
    writer.StartArray();
    writer.Uint64(axisNum);
    writer.EndArray();
    writer.String("id");
    writer.Uint64(controller->idReq);
    (controller->idReq)++;
    writer.EndObject();

    result = controller->sendReceive(string_buffer.GetString(), string_buffer.GetSize(), recvBuffer, sizeof(recvBuffer));
    if (!result) {
        printf("sendReceive failed\n");
        return false;
    }

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

bool AMC100Axis::getFrequency() {
    bool result = false;

    rapidjson::StringBuffer string_buffer;
    rapidjson::Writer<rapidjson::StringBuffer> writer(string_buffer);
    char recvBuffer[256];
    writer.StartObject();
    writer.String("jsonrpc");
    writer.String("2.0");
    writer.String("method");
    writer.String("com.attocube.amc.control.getControlFrequency");
    writer.String("params");
    writer.StartArray();
    writer.Uint64(axisNum);
    writer.EndArray();
    writer.String("id");
    writer.Uint64(controller->idReq);
    (controller->idReq)++;
    writer.EndObject();

    result = controller->sendReceive(string_buffer.GetString(), string_buffer.GetSize(), recvBuffer, sizeof(recvBuffer));
    if (!result) {
        printf("sendReceive failed\n");
        return false;
    }

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
    bool result = false;

    rapidjson::StringBuffer string_buffer;
    rapidjson::Writer<rapidjson::StringBuffer> writer(string_buffer);
    char recvBuffer[256];
    writer.StartObject();
    writer.String("jsonrpc");
    writer.String("2.0");
    writer.String("method");
    writer.String("com.attocube.amc.control.setControlFrequency");
    writer.String("params");
    writer.StartArray();
    writer.Uint64(axisNum);
    writer.Uint64(frequency * 1000);
    writer.EndArray();
    writer.String("id");
    writer.Uint64(controller->idReq);
    (controller->idReq)++;
    writer.EndObject();

    result = controller->sendReceive(string_buffer.GetString(), string_buffer.GetSize(), recvBuffer, sizeof(recvBuffer));
    if (!result) {
        printf("sendReceive failed\n");
        return false;
    }

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
	bool result = false;

    rapidjson::StringBuffer string_buffer;
    rapidjson::Writer<rapidjson::StringBuffer> writer(string_buffer);
    char recvBuffer[256];
    writer.StartObject();
    writer.String("jsonrpc");
    writer.String("2.0");
    writer.String("method");
    writer.String("com.attocube.amc.move.setControlTargetPosition");
    writer.String("params");
    writer.StartArray();
    writer.Uint64(axisNum);
    writer.Double(position / 1000.0);
    writer.EndArray();
    writer.String("id");
    writer.Uint64(controller->idReq);
    (controller->idReq)++;
    writer.EndObject();

    result = controller->sendReceive(string_buffer.GetString(), string_buffer.GetSize(), recvBuffer, sizeof(recvBuffer));
    if (!result) {
        printf("sendReceive failed\n");
        return asynError;
    }

    rapidjson::Document recvDocument;
    recvDocument.Parse(recvBuffer);
    if (recvDocument.Parse(recvBuffer).HasParseError()) {
        printf("Could not parse recvBuffer json\n");
        return asynError;
    }

    rapidjson::Value& response = recvDocument["result"];
    if (!response.IsArray() || response.Size() != 1) {
        printf("Didn't return expected type\n");
        return asynError;
    }

    int errorNum = response[0].GetInt();
    if (errorNum) {
        controller->setError(errorNum);
    }

    return result ? asynSuccess : asynError;

}

bool AMC100Axis::getPosition() {
    bool result = false;

    rapidjson::StringBuffer string_buffer;
    rapidjson::Writer<rapidjson::StringBuffer> writer(string_buffer);
    char recvBuffer[256];
    writer.StartObject();
    writer.String("jsonrpc");
    writer.String("2.0");
    writer.String("method");
    writer.String("com.attocube.amc.move.getPosition");
    writer.String("params");
    writer.StartArray();
    writer.Uint64(axisNum);
    writer.EndArray();
    writer.String("id");
    writer.Uint64(controller->idReq);
    (controller->idReq)++;
    writer.EndObject();

    result = controller->sendReceive(string_buffer.GetString(), string_buffer.GetSize(), recvBuffer, sizeof(recvBuffer));
    if (!result) {
        printf("sendReceive failed\n");
        return false;
    }

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
    double position = response[1].GetDouble();
    setDoubleParam(controller->motorEncoderPosition_, position * 1000);
    setDoubleParam(controller->motorPosition_, position * 1000);

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
	bool result = true;

    // if ((!controller->command(pmAxesNumbers[axisNum],cmdStop,NULL,0,rxBuffer,0)) {
        // result = false;
    // }
    //     
    // else {
    //     setIntegerParam(controller->motorStatusDone_, 1);
    //     setIntegerParam(controller->motorStatusMoving_, 0);
    // }

    // return result ? asynSuccess : asynError;
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
bool AMC100Axis::firstTimeInit()
{
	bool result = true;

	return result;
}

