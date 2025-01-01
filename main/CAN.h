#ifndef CAN_H
#define CAN_H
#include <stdint.h>
#include <stdbool.h>

#include "driver/twai.h"

#include "dbc.h"

typedef struct {
  int32_t erpm;               // time sensitive critical
  float duty_cycle;           // time sensitive critical 
  int16_t input_voltage;      // monitoring 1
  float ac_current;           // time sensitive critical 
  float dc_current;           // time sensitive critical
  float inverter_temp;        // monitoring 1
  float motor_temp;           // monitoring 1
  uint8_t FAULT_CODE;         // time sensitive critical
  float FOC_Id;               // monitoring 2
  float FOC_Iq;               // monitoring 2
  int8_t throttle_in;         // time sensitive critical
  int8_t brake_in;            // X
  uint8_t DigitalIO;          // monitoring 2
  uint8_t DriveEN;            // monitoring 2
  uint8_t ActiveLimitsByte4;  // time sensitive critical
  uint8_t ActiveLimitsByte5;  // time sensitive critical
  uint8_t CAN_MapVers;        // monitoring 2
} can_DTI_HV_500_t;

typedef struct {
  uint16_t PackAbsCurrent;      // monitoring 1
  uint16_t AverageCurrent;      // monitoring 1
  float MaxCellVoltage;         // time sensitive critical
  float PackCCL;                // monitoring 1
  uint16_t PackDCL;             // monitoring 1
  uint16_t MaxPackVoltage;      // monitoring 1
  int16_t PackCurrent;          // time sensitive information
  uint8_t PackSOC;              // monitoring 1
  uint8_t bitfield1;            // monitoring 2
  uint8_t bitfield2;            // monitoring 2
  uint8_t bitfield3;            // monitoring 2
  uint8_t bitfield4;            // monitoring 2
  int8_t HighTemperature;       // monitoring 1
  int8_t InternalTemperature;   // monitoring 1
  bool DischargeEnableInverted; // monitoring 2
  bool ChargerSafetyRelayFault; // monitoring 2
} can_OrionBMS2_t;

typedef struct {
  float InstantVoltage;
  float InternalResistance;
  float OpenVoltage;
} can_CellData_t;

typedef struct {
  uint16_t RadiatorIN;    // monitoring 1
  uint16_t RadiatorOut;   // monitoring 1
  uint16_t Throttle_1;    // time sensitive critical
  uint16_t Throttle_2;    // time sensitive critical
  uint16_t Brake;         // time sensitive critical
  uint16_t vBat;          // monitoring 1
  int16_t accLong;        // time sensitive informational
  int16_t accLat;         // time sensitive informational
  int16_t accVert;        // time sensitive informational
  int16_t yawRate;        // time sensitive informational
  int16_t Pitch;          // time sensitive informational
  int16_t Roll;           // time sensitive informational
  uint16_t GPS_Fix;       // monitoring 2
  uint16_t CAN1_Load;     // monitoring 2
  uint16_t CAN1_Errors;   // monitoring 2
} can_PLEX_t;

#define GET_UINT32_LE_BYTE_(buffer, byte) (uint32_t) (buffer[byte + 3] << 3 | buffer[byte + 2] << 2 | buffer[byte + 1] << 1 | buffer[byte])

#define GET_INT32_LE_BYTE_(buffer, byte) (int32_t) (buffer[byte + 3] << 3 | buffer[byte + 2] << 2 | buffer[byte + 1] << 1 | buffer[byte])

#define GET_INT16_LE_BYTE_(buffer, byte) (int16_t) (buffer[byte + 1] << 1 | buffer[byte])

#define GET_UINT16_LE_BYTE_(buffer, byte) (uint16_t) (buffer[byte + 1] << 1 | buffer[byte])


#define GET_UINT32_BE_BYTE_(buffer, byte) (uint32_t) (buffer[byte] << 3 | buffer[byte + 1] << 2 | buffer[byte + 2] << 1 | buffer[byte+3])

#define GET_INT32_BE_BYTE_(buffer, byte) (int32_t) (buffer[byte] << 3 | buffer[byte + 1] << 2 | buffer[byte + 2] << 1 | buffer[byte+3])

#define GET_INT16_BE_BYTE_(buffer, byte) (int16_t) (buffer[byte] << 1 | buffer[byte+1])

#define GET_UINT16_BE_BYTE_(buffer, byte) (uint16_t) (buffer[byte] << 1 | buffer[byte+1])

void twai_receive_task(void *arg);



#endif  // CAN.h