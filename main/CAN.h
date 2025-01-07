#ifndef CAN_H
#define CAN_H
#include <stdint.h>
#include <stdbool.h>

#include "driver/twai.h"

#include "dbc.h"

typedef struct {
  int32_t erpm;               
  int16_t duty_cycle;          
  int16_t input_voltage;      
  int16_t ac_current;          
  int16_t dc_current;         
  int16_t inverter_temp;      
  int16_t motor_temp;         
  uint8_t FAULT_CODE;         
  int32_t FOC_Id;             
  int32_t FOC_Iq;             
  int8_t throttle_in;         
  int8_t brake_in;            
  uint8_t DigitalIO;          
  uint8_t DriveEN;            
  uint8_t ActiveLimitsByte4;  
  uint8_t ActiveLimitsByte5;  
  uint8_t CAN_MapVers;        
} can_DTI_HV_500_t;

typedef struct {
  int16_t PackCurrent;        
  int16_t AverageCurrent;      
  uint16_t PackOpenVoltage;   
  uint16_t PackAbsCurrent;    
  uint16_t PackDCL;           
  int8_t HighTemperature;     
  uint8_t PackSOC;            
  int8_t InternalTemperature; 
  uint8_t HighOpenCellVoltage;
  uint8_t HighOpenCellID;     
  uint8_t LowOpenCellVoltage; 
  uint8_t LowOpenCellID;      
  uint8_t DTC_Flags_1;
  uint8_t DTC_Flags_2;
  bool DischargeEnableInverted; 
  bool BalancingEnabled;
} can_OrionBMS2_t;

typedef struct {
  uint16_t InstantVoltage; 
  uint16_t InternalResistance;
  uint16_t OpenVoltage;
  bool Shunted;
} can_CellData_t;

typedef struct {
  uint16_t RadiatorIN;    
  uint16_t RadiatorOUT;   
  uint16_t Throttle_1;    
  uint16_t Throttle_2;    
  uint16_t Brake;         
  int16_t vBat;          
  int16_t accLong;       
  int16_t accLat;         
  int16_t accVert;        
  int16_t yawRate;        
  int16_t Pitch;          
  int16_t Roll;           
  uint16_t GPS_Fix;       
  uint16_t CAN1_Load;     
  uint16_t CAN1_Errors;   
} can_PLEX_t;

#define GET_UINT32_LE_BYTE_(buffer, byte) (uint32_t) (buffer[byte + 3] << 24 | buffer[byte + 2] << 16 | buffer[byte + 1] << 8 | buffer[byte])

#define GET_INT32_LE_BYTE_(buffer, byte) (int32_t) (buffer[byte + 3] << 24 | buffer[byte + 2] << 16 | buffer[byte + 1] << 8 | buffer[byte])

#define GET_INT16_LE_BYTE_(buffer, byte) (int16_t) (buffer[byte + 1] << 8 | buffer[byte])

#define GET_UINT16_LE_BYTE_(buffer, byte) (uint16_t) (buffer[byte + 1] << 8 | buffer[byte])


#define GET_UINT32_BE_BYTE_(buffer, byte) (uint32_t) (buffer[byte] << 24 | buffer[byte + 1] << 16 | buffer[byte + 2] << 8 | buffer[byte+3])

#define GET_INT32_BE_BYTE_(buffer, byte) (int32_t) (buffer[byte] << 24 | buffer[byte + 1] << 16 | buffer[byte + 2] << 8 | buffer[byte+3])

#define GET_INT16_BE_BYTE_(buffer, byte) (int16_t) (buffer[byte] << 8 | buffer[byte+1])

#define GET_UINT16_BE_BYTE_(buffer, byte) (uint16_t) (buffer[byte] << 8 | buffer[byte+1])

void twai_receive_task(void *arg);


void print_can_data_hex(twai_message_t * msg);


#endif  // CAN.h