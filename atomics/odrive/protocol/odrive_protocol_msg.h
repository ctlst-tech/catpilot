#ifndef FSPEC_ODRIVE_PROTOCOL_MSG_H
#define FSPEC_ODRIVE_PROTOCOL_MSG_H

#define ODRIVE_GET_CMD_ID(axis, cmd) (axis * 0x20 + cmd)

#define ODRIVE_GET_VERSION 0x000
#define ODRIVE_HEARTBEAT 0x001
#define ODRIVE_ESTOP 0x002
#define ODRIVE_GET_ERROR 0x003
#define ODRIVE_SET_AXIS_NODE_ID 0x006
#define ODRIVE_SET_AXIS_STATE 0x007
#define ODRIVE_GET_ENCODER_ESTIMATES 0x009
#define ODRIVE_SET_CONTROLLER_MODE 0x00B
#define ODRIVE_SET_INPUT_POS 0x00C
#define ODRIVE_SET_INPUT_VEL 0x00D
#define ODRIVE_SET_INPUT_TORQUE 0x00E
#define ODRIVE_SET_LIMITS 0x00F
#define ODRIVE_START_ANTICOGGING 0x010
#define ODRIVE_SET_TRAJ_VEL_LIMIT 0x011
#define ODRIVE_SET_TRAJ_ACCEL_LIMITS 0x012
#define ODRIVE_SET_TRAJ_INERTIA 0x013
#define ODRIVE_GET_IQ 0x014
#define ODRIVE_GET_TEMPERATURE 0x015
#define ODRIVE_REBOOT 0x016
#define ODRIVE_GET_BUS_VOLTAGE_CURRENT 0x017
#define ODRIVE_CLEAR_ERRORS 0x018
#define ODRIVE_SET_ABSOLUTE_POSITION 0x019
#define ODRIVE_SET_POS_GAIN 0x01A
#define ODRIVE_SET_VEL_GAIN 0x01B
#define ODRIVE_GET_ADC_VOLTAGE 0x01C
#define ODRIVE_GET_CONTROLLER_ERROR 0x01D
#define ODRIVE_ENTER_DFU_MODE 0x01F

#endif  // FSPEC_ODRIVE_PROTOCOL_MSG_H
