#ifndef CAN_H
#define CAN_H
#include <stdint.h>
#include <stdbool.h>

#include "driver/twai.h"

#include "dbc.h"

typedef struct {
  int32_t erpm;
  float duty_cycle;
  int16_t input_voltage;
  float ac_current;
  float dc_current;
  float inverter_temp;
  float motor_temp;
  uint8_t FAULT_CODE;
  float FOC_Id;
  float FOC_Iq;
  int8_t throttle_in;
  int8_t brake_in;
  uint8_t DigitalIO;
  uint8_t DriveEN;
  uint8_t ActiveLimitsByte4;
  uint8_t ActiveLimitsByte5;
  uint8_t CAN_MapVers;  
} can_DTI_HV_500_t;

typedef struct {
  uint16_t PackAbsCurrent;
  uint16_t AverageCurrent;
  float MaxCellVoltage;
  float PackCCL;
  uint16_t PackDCL;
  uint16_t MaxPackVoltage;
  int16_t PackCurrent;
  uint8_t PackSOC;
  uint8_t bitfield1;
  uint8_t bitfield2;
  uint8_t bitfield3;
  uint8_t bitfield4;
  int8_t HighTemperature;
  int8_t InternalTemperature;
  bool DischargeEnableInverted;
  bool ChargerSafetyRelayFault;
} can_OrionBMS2_t;

typedef struct {
  float InstantVoltage;
  float InternalResistance;
  float OpenVoltage;
} can_CellData_t;



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