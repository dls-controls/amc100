
/*
// get Lock status
{"jsonrpc": "2.0", "method": "getLockStatus", "id": 1}
{"jsonrpc": "2.0", "result": [0], "id": 1}

// Lock
{"jsonrpc": "2.0", "method": "lock", "params": [password], "id": 2}
{"jsonrpc": "2.0", "result": [error_number], "id": 2}

// grant access
{"jsonrpc": "2.0", "method": "grantAccess", "params": [password], "id": 3}
{"jsonrpc": "2.0", "result": [error_number], "id": 3}
*/

// error number to string
{"jsonrpc": "2.0", "method": "com.attocube.system.errorNumberToString", "params": [language, error_n], "id": 4}
{"jsonrpc": "2.0", "result": [error_number], "id": 4}

// unlock
//{"jsonrpc": "2.0", "method": "unlock", "id": 5}
//{"jsonrpc": "2.0", "result": [error_number], "id": 5}

// control output
//{"jsonrpc": "2.0", "method": "com.attocube.AMC.control.setControlOutput", "params": [axis_number, output], "id": 6}
//{"jsonrpc": "2.0", "result": [error_number], "id": 6}

//{"jsonrpc": "2.0", "method": "com.attocube.AMC.control.getControlOutput", "params": [axis_number], "id": 6.1}
//{"jsonrpc": "2.0", "result": [error_number, output], "id": 6.1}

// control amplitude
{"jsonrpc": "2.0", "method": "com.attocube.AMC.control.setControlAmplitude", "params": [axis_n, amplitude], "id": 8}
{"jsonrpc":"2.0","result":[0], "id":8}

// control frequency
{"jsonrpc": "2.0", "method": "com.attocube.AMC.control.setControlFrequency", "params": [axis_number, frequency], "id": 9}
{"json": "2.0", "result": [0], "id": 9}

{"jsonrpc": "2.0", "method": "com.attocube.AMC.control.getControlFrequency", "params": [axis_number], "id": 9.1}
{"jsonrpc": "2.0", "result": [error_number, frequency], "id": 9.1}
/*
// set controlActorSelection
{"jsonrpc": "2.0", "method": "com.attocube.AMC.control.setActorParametersByName", "params": [axis_number, positioner], "id": 10}
{"jsonrpc": "2.0", "result": [error_number], "id": 10}

// getActorName
{"jsonrpc": "2.0", "method": "com.attocube.AMC.control.getActorName", "params": [axis_number], "id": 11}
{"jsonrpc": "2.0", "result": [error_number, positioner_name], "id": 11}

// getActorType
{"jsonrpc": "2.0", "method": "com.attocube.AMC.control.getActorType", "params": [axis_number], "id": 12}
{"jsonrpc": "2.0", "result": [error_number, positioner_type], "id": 12}

// set Reset
{"jsonrpc": "2.0", "method": "com.attocube.AMC.control.setReset", "params": [axis_number], "id": 13}
{"jsonrpc": "2.0", "result": [error_number], "id": 13}
*/

// control Move
{"jsonrpc": "2.0", "method": "com.attocube.AMC.control.setControlMove", "params": [axis_number, enable], "id": 14}
{"jsonrpc": "2.0", "result": [error_number], "id": 14}

{"jsonrpc": "2.0", "method": "com.attocube.AMC.control.getControlMove", "params": [axis_number], "id": 14.1}
{"jsonrpc": "2.0", "result": [error_number, enable], "id": 14.1}

// setNsteps
{"jsonrpc": "2.0", "method": "com.attocube.AMC.move.setNSteps", "params": [axis_number, backward, steps_number], "id": 15}
{"jsonrpc": "2.0", "result": [error_number], "id": 15}

// getNsteps
{"jsonrpc": "2.0", "method": "com.attocube.AMC.move.getNSteps", "params": [axis_number], "id": 16}
{"jsonrpc": "2.0", "result": [error_number, number_of_steps], "id": 16}
/*
// control continuous Fwd
{"jsonrpc": "2.0", "method": "com.attocube.AMC.move.setControlContinousFwd", "params": [axis_number, enable], "id": 17}
{"jsonrpc": "2.0", "result": [error_number], "id": 17}

{"jsonrpc": "2.0", "method": "com.attocube.AMC.move.getControlContinousFwd", "params": [axis_number], "id": 17.1}
{"jsonrpc": "2.0", "result": [error_number, enable], "id": 17.1}

// control continuous Bkwd
{"jsonrpc": "2.0", "method": "com.attocube.AMC.move.setControlContinousBkwd", "params": [axis_number, enable], "id": 18}
{"jsonrpc": "2.0", "result": [error_number], "id": 18}

{"jsonrpc": "2.0", "method": "com.attocube.AMC.move.getControlContinousBkwd", "params": [axis_number], "id": 18.1}
{"jsonrpc": "2.0", "result": [error_number, enable], "id": 18.1}
*/

// control target position
{"jsonrpc": "2.0", "method": "com.attocube.AMC.move.setControlTargetPosition", "params": [axis_number, target_position], "id": 19}
{"jsonrpc": "2.0", "result": [error_number], "id": 19}

{"jsonrpc": "2.0", "method": "com.attocube.AMC.move.getControlTargetPosition", "params": [axis_number], "id": 19.1}
{"jsonrpc": "2.0", "result": [error_number, target_position], "id": 19.1}
/*
// get status reference
{"jsonrpc": "2.0", "method": "com.attocube.AMC.status.getStatusReference", "params": [axis_number], "id": 20}
{"jsonrpc": "2.0", "result": [error_number, status_reference], "id": 20}
*/
// get status moving
{"jsonrpc": "2.0", "method": "com.attocube.AMC.status.getStatusMoving", "params": [axis_number], "id": 21}
{"jsonrpc": "2.0", "result": [error_number, status_moving], "id": 21}

// get status connected
{"jsonrpc": "2.0", "method": "com.attocube.AMC.status.getStatusConnected", "params": [axis_number], "id": 22}
{"jsonrpc": "2.0", "result": [error_number, status_connecting], "id": 22}
/*
// get reference position
{"jsonrpc": "2.0", "method": "com.attocube.AMC.control.getReferencePosition", "params": [axis_number], "id": 23}
{"jsonrpc": "2.0", "result": [error_number, reference_position], "id": 23}
*/

// get position
{"jsonrpc": "2.0", "method": "com.attocube.AMC.move.getPosition", "params": [axis_number], "id": 24}
{"jsonrpc": "2.0", "result": [error_number, position], "id": 24}
/*
// control reference autoUpdate
{"jsonrpc": "2.0", "method": "com.attocube.AMC.control.setControlReferenceAutoUpdate", "params": [axis_number, enable], "id": 25}
{"jsonrpc": "2.0", "result": [error_number], "id": 25}

{"jsonrpc": "2.0", "method": "com.attocube.AMC.control.getControlReferenceAutoUpdate", "params": [axis_number], "id": 25.1}
{"jsonrpc": "2.0", "result": [error_number, enable], "id": 25.1}

// control autoReset
{"jsonrpc": "2.0", "method": "com.attocube.AMC.control.setControlAutoReset", "params": [axis_number, enable], "id": 26}
{"jsonrpc": "2.0", "result": [error_number], "id": 26}

{"jsonrpc": "2.0", "method": "com.attocube.AMC.control.getControlAutoReset", "params": [axis_number], "id": 26.1}
{"jsonrpc": "2.0", "result": [error_number, enable], "id": 26.1}

// control targetRange
{"jsonrpc": "2.0", "method": "com.attocube.AMC.control.setControlTargetRange", "params": [axis_number, target_range], "id": 27}
{"jsonrpc": "2.0", "result": [error_number], "id": 27}

{"jsonrpc": "2.0", "method": "com.attocube.AMC.control.getControlTargetRange", "params": [axis_number], "id": 27.1}
{"jsonrpc": "2.0", "result": [error_number, target_range], "id": 27.1}

// getStatusTargetRange
{"jsonrpc": "2.0", "method": "com.attocube.AMC.status.getStatusTargetRange", "params": [axis_number], "id": 28}
{"jsonrpc": "2.0", "result": [error_number, status_target_range], "id": 28}
*/

// getFirmwareVersion
{"jsonrpc": "2.0", "method": "com.attocube.system.getFirmwareVersion", "id": 29}
{"jsonrpc": "2.0", "result": [firmware_version_number], "id": 29}

/*
// getFpgaVersion
{"jsonrpc": "2.0", "method": "com.attocube.AMC.description.getFpgaVersion", "id": 30}
{"jsonrpc": "2.0", "result": [FPGA_version_number], "id": 30}

// rebootSystem
{"jsonrpc": "2.0", "method": "rebootSystem", "id": 31}
{"jsonrpc": "2.0", "result": [error_number], "id": 31}

// factoryReset
{"jsonrpc": "2.0", "method": "factoryReset", "id": 32}
{"jsonrpc": "2.0", "result": [error_number], "id": 32}

// getMacAddress
{"jsonrpc": "2.0", "method": "com.attocube.system.getMacAddress", "id": 33}
{"jsonrpc": "2.0", "result": [MAC_address], "id": 33}

// getIpAddress
{"jsonrpc": "2.0", "method": "com.attocube.system.network.getIpAddress", "id": 34}
{"jsonrpc": "2.0", "result": [IP_address], "id": 34}

// getDeviceType
{"jsonrpc": "2.0", "method": "com.attocube.AMC.description.getDeviceType", "id": 35}
{"jsonrpc": "2.0", "result": [device_type], "id": 35}

// getSerialNumber
{"jsonrpc": "2.0", "method": "com.attocube.system.getSerialNumber", "id": 36}
{"jsonrpc": "2.0", "result": [serial_number], "id": 36}

// getDeviceName
{"jsonrpc": "2.0", "method": "com.attocube.system.getDeviceName", "id": 37}
{"jsonrpc": "2.0", "result": [device_name], "id": 37}

// setDeviceName
{"jsonrpc": "2.0", "method": "com.attocube.system.setDeviceName", "params": [device_name], "id": 38}
{"jsonrpc": "2.0", "result": [error_number], "id": 38}

// getStatusEotFwd
{"jsonrpc": "2.0", "method": "com.attocube.AMC.status.getStatusEotFwd", "params": [axis_number], "id": 39}
{"jsonrpc": "2.0", "result": [error_number, end_of_travel_detected], "id": 39}

// getStatusEotBkwd
{"jsonrpc": "2.0", "method": "com.attocube.AMC.status.getStatusEotBkwd", "params": [axis_number], "id": 40}
{"jsonrpc": "2.0", "result": [error_number, end_of_travel_detected], "id": 40}

// controlEotOutputDeactive
{"jsonrpc": "2.0", "method": "com.attocube.AMC.move.setControlEotOutputDeactive", "params": [axis_number, enable], "id": 41}
{"jsonrpc": "2.0", "result": [error_number], "id": 41}

{"jsonrpc": "2.0", "method": "com.attocube.AMC.move.getControlEotOutputDeactive", "params": [axis_number], "id": 41.1}
{"jsonrpc": "2.0", "result": [error_number, enable], "id": 41.1}

// controlFixOutputVoltage
{"jsonrpc": "2.0", "method": "com.attocube.AMC.control.setControlFixOutputVoltage", "params": [axis_number, voltage], "id": 42}
{"jsonrpc": "2.0", "result": [error_number], "id": 42}

{"jsonrpc": "2.0", "method": "com.attocube.AMC.control.getControlFixOutputVoltage", "params": [axis_number], "id": 42.1}
{"jsonrpc": "2.0", "result": [error_number, voltage], "id": 42.1}

// getPositionersList
{"jsonrpc": "2.0", "method": "com.attocube.AMC.description.getPositionersList", "id": 43}
{"jsonrpc": "2.0", "result": [list], "id": 43}

// controlAQuadBInResolution
{"jsonrpc": "2.0", "method": "com.attocube.AMC.rtin.setControlAQuadBInResolution", "params": [axis_number, resolution] "id": 44}
{"jsonrpc": "2.0", "result": [error_number], "id": 44}

{"jsonrpc": "2.0", "method": "com.attocube.AMC.rtin.getControlAQuadBInResolution", "params": [axis_number], "id": 44.1}
{"jsonrpc": "2.0", "result": [error_number, resolution], "id": 44.1}

// controlAQuadBOut
{"jsonrpc": "2.0", "method": "com.attocube.AMC.rtout.setControlAQuadBOut", "params": [axis_number, enable] "id": 45}
{"jsonrpc": "2.0", "result": [error_number], "id": 45}

{"jsonrpc": "2.0", "method": "com.attocube.AMC.rtout.getControlAQuadBOut", "params": [axis_number], "id": 45.1}
{"jsonrpc": "2.0", "result": [error_number, enable], "id": 45.1}

// controlAQuadBOutResolution
{"jsonrpc": "2.0", "method": "com.attocube.AMC.rtout.setControlAQuadBOutResolution", "params": [axis_number, resolution] "id": 46}
{"jsonrpc": "2.0", "result": [error_number], "id": 46}

{"jsonrpc": "2.0", "method": "com.attocube.AMC.rtout.getControlAQuadBOutResolution", "params": [axis_number], "id": 46.1}
{"jsonrpc": "2.0", "result": [error_number, resolution], "id": 46.1}

// controlAQuadBOutClock
{"jsonrpc": "2.0", "method": "com.attocube.AMC.rtout.setControlAQuadBOutClock", "params": [axis_number, clock] "id": 47}
{"jsonrpc": "2.0", "result": [error_number], "id": 47}

{"jsonrpc": "2.0", "method": "com.attocube.AMC.rtout.getControlAQuadBOutClock", "params": [axis_number], "id": 47.1}
{"jsonrpc": "2.0", "result": [error_number, clock], "id": 47.1}

// controlRtOutSignalMode
{"jsonrpc": "2.0", "method": "com.attocube.AMC.rtout.setRtOutSignalMode", "params": [axis_number, mode] "id": 48}
{"jsonrpc": "2.0", "result": [error_number], "id": 48}

{"jsonrpc": "2.0", "method": "com.attocube.AMC.rtout.getRtOutSignalMode", "params": [axis_number], "id": 48.1}
{"jsonrpc": "2.0", "result": [error_number, mode], "id": 48.1}

// controlRealtimeInputMode
{"jsonrpc": "2.0", "method": "com.attocube.AMC.rtin.setRealTimeInMode", "params": [axis_number, mode] "id": 49}
{"jsonrpc": "2.0", "result": [error_number], "id": 49}

{"jsonrpc": "2.0", "method": "com.attocube.AMC.rtin.getRealTimeInMode", "params": [axis_number], "id": 49.1}
{"jsonrpc": "2.0", "result": [error_number, mode], "id": 49.1}

// controlRealtimeInputLoopMode
{"jsonrpc": "2.0", "method": "com.attocube.AMC.rtin.setRealTimeInFeebackLoopMode", "params": [axis_number, mode] "id": 50}
{"jsonrpc": "2.0", "result": [error_number], "id": 50}

{"jsonrpc": "2.0", "method": "com.attocube.AMC.rtin.getRealTimeInFeebackLoopMode", "params": [axis_number], "id": 50.1}
{"jsonrpc": "2.0", "result": [error_number, mode], "id": 50.1}

// controlRealtimeInputChangePerPulse
{"jsonrpc": "2.0", "method": "com.attocube.AMC.rtin.setRealTimeInChangePerPulse", "params": [axis_number, change_per_pulse] "id": 501
{"jsonrpc": "2.0", "result": [error_number], "id": 51}

{"jsonrpc": "2.0", "method": "com.attocube.AMC.rtin.getRealTimeInChangePerPulse", "params": [axis_number], "id": 51.1}
{"jsonrpc": "2.0", "result": [error_number, change_per_pulse], "id": 51.1}

//  controlRealtimeInputStepsPerPulse
{"jsonrpc": "2.0", "method": "com.attocube.AMC.rtin.setRealTimeInStepsPerPulse", "params": [axis_number, steps_per_pulse] "id": 52}
{"jsonrpc": "2.0", "result": [error_number], "id": 52}

{"jsonrpc": "2.0", "method": "com.attocube.AMC.rtin.getRealTimeInStepsPerPulse", "params": [axis_number], "id": 52.1}
{"jsonrpc": "2.0", "result": [error_number, steps_per_pulse], "id": 52.1}

// controlRealtimeInputMove
{"jsonrpc": "2.0", "method": "com.attocube.AMC.rtin.setControlMoveGPIO", "params": [axis_number, enable] "id": 53}
{"jsonrpc": "2.0", "result": [error_number], "id": 53}

{"jsonrpc": "2.0", "method": "com.attocube.AMC.rtin.getControlMoveGPIO", "params": [axis_number], "id": 53.1}
{"jsonrpc": "2.0", "result": [error_number, enable], "id": 53.1}

// rotationCompensation.setLUT
{"jsonrpc": "2.0", "method": "com.attocube.AMC.rotcomp.setLUT", "params": [lut_string] "id": 54}
{"jsonrpc": "2.0", "result": [error_number], "id": 54}

// rotationCompensation.getLUT
// rotationCompensation.setEnabled
// rotationCompensation.getEnabled
// rotationCompensation.updateOffsets
// getControlTargetRanges
*/