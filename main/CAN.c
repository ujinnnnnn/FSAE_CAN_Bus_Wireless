#include "CAN.h"

#include "freertos/freertos.h"
#include "freertos/task.h"
#include "freertos/semphr.h"

#include "esp_log.h"

extern can_DTI_HV_500_t Inv_data;
extern can_OrionBMS2_t BMS_data;
extern can_PLEX_t Plex_data;
extern int8_t Thermistor_data[TEM_MAX_THERMISTORS];
extern can_CellData_t Cell_data[120];

extern SemaphoreHandle_t setup_sem;

static char TAG[] = "CAN.c";

void twai_receive_task(void *arg)
{
  esp_err_t ret;

  twai_message_t rx_msg;
  while (1)
  {
    ret = twai_receive(&rx_msg, pdMS_TO_TICKS(1000));

    switch (ret)
    {
    case ESP_OK:
      //print_can_data_hex(&rx_msg);
      switch (rx_msg.identifier)
      {
      case Inverter_Message_1_ID:
        Inv_data.erpm = GET_INT32_BE_BYTE_(rx_msg.data, 0);  
        Inv_data.duty_cycle = GET_INT16_BE_BYTE_(rx_msg.data, 4); // 1 : 10
        Inv_data.input_voltage = GET_INT16_BE_BYTE_(rx_msg.data, 6);
        break;
      case Inverter_Message_2_ID:
        Inv_data.ac_current = GET_INT16_BE_BYTE_(rx_msg.data, 0);  // 1 : 10
        Inv_data.dc_current = GET_INT16_BE_BYTE_(rx_msg.data, 2) ; // 1 : 10
        break;
      case Inverter_Message_3_ID:
        Inv_data.inverter_temp = GET_INT16_BE_BYTE_(rx_msg.data, 0) ; // 1 : 10
        Inv_data.motor_temp = GET_INT16_BE_BYTE_(rx_msg.data, 2) ;    // 1 : 10
        Inv_data.FAULT_CODE = rx_msg.data[4];
        break;
      case Inverter_Message_4_ID:
        Inv_data.FOC_Id = GET_INT32_BE_BYTE_(rx_msg.data, 0);   // 1 : 100
        Inv_data.FOC_Iq = GET_INT32_BE_BYTE_(rx_msg.data, 4);   // 1 ; 100
        break;
      case Inverter_Message_5_ID:
        Inv_data.throttle_in = (int8_t) rx_msg.data[0];
        Inv_data.brake_in = (int8_t) rx_msg.data[1];
        Inv_data.DigitalIO = (uint8_t) rx_msg.data[2];
        Inv_data.DriveEN = (uint8_t) rx_msg.data[3];
        Inv_data.ActiveLimitsByte4 = (uint8_t) rx_msg.data[4];
        Inv_data.ActiveLimitsByte5 = (uint8_t) rx_msg.data[5];
        Inv_data.CAN_MapVers = (uint8_t) rx_msg.data[7];
        break;

      case BMS_Cell_Broadcast_ID:
        uint8_t checkSum = 0xFF & (BMS_Cell_Broadcast_ID + 8 + rx_msg.data[0] + rx_msg.data[1] + rx_msg.data[2] + rx_msg.data[3] + rx_msg.data[4] + rx_msg.data[5] + rx_msg.data[6]);
        if (checkSum == rx_msg.data[7])
        {
          Cell_data[(uint8_t)rx_msg.data[0]].InstantVoltage = GET_UINT16_BE_BYTE_(rx_msg.data, 1);      // 1 : 10
          Cell_data[(uint8_t)rx_msg.data[0]].InternalResistance = (uint16_t) (rx_msg.data[3] << 8 | (rx_msg.data[3+1] & 0x7F));  // 1 : 100
          Cell_data[(uint8_t)rx_msg.data[0]].Shunted = (bool) (rx_msg.data[4] >> 7);
          Cell_data[(uint8_t)rx_msg.data[0]].OpenVoltage = GET_UINT16_BE_BYTE_(rx_msg.data, 5);         // 1 : 10
        }
        else
        {
          ESP_LOGE(TAG, "\n\n\n\n\nCell checksum error\nExpected: %x\nGot: %x", rx_msg.data[7], checkSum);
        }
        break;

      case BMS_Message_1_ID:
        BMS_data.PackSOC = (uint8_t)rx_msg.data[0];
        BMS_data.PackCurrent = GET_INT16_BE_BYTE_(rx_msg.data, 1);
        BMS_data.HighTemperature = (int8_t)rx_msg.data[3];
        BMS_data.InternalTemperature = (int8_t)rx_msg.data[4];
        BMS_data.PackDCL = GET_UINT16_BE_BYTE_(rx_msg.data, 5);
        BMS_data.DischargeEnableInverted = rx_msg.data[7] & 0b1;
        BMS_data.BalancingEnabled = (rx_msg.data[7] >> 1) & 0b1;
        break;

      case BMS_Message_2_ID:
        BMS_data.PackAbsCurrent = GET_UINT16_BE_BYTE_(rx_msg.data, 0);
        BMS_data.AverageCurrent = GET_INT16_BE_BYTE_(rx_msg.data, 2);

        break;

      case TEM_Broadcast_ID:
        Thermistor_data[GET_UINT16_BE_BYTE_(rx_msg.data, 0)] = (int8_t)rx_msg.data[2];
        break;
      case PLEX_Message_1_ID:
        Plex_data.Brake = GET_UINT16_BE_BYTE_(rx_msg.data,0);
        Plex_data.yawRate = GET_INT16_BE_BYTE_(rx_msg.data,2);
        Plex_data.Throttle_1 = GET_UINT16_BE_BYTE_(rx_msg.data,4);
        Plex_data.Throttle_2 = GET_UINT16_BE_BYTE_(rx_msg.data,6);
        break;
      case PLEX_Message_2_ID:
        Plex_data.accLong = GET_INT16_BE_BYTE_(rx_msg.data,0);
        Plex_data.accLat = GET_INT16_BE_BYTE_(rx_msg.data,2);
        Plex_data.Pitch = GET_INT16_BE_BYTE_(rx_msg.data,4);
        Plex_data.Roll = GET_INT16_BE_BYTE_(rx_msg.data,6); 
        break;
      case PLEX_Message_3_ID:
        Plex_data.vBat = GET_INT16_BE_BYTE_(rx_msg.data,0);
        Plex_data.GPS_Fix = GET_UINT16_BE_BYTE_(rx_msg.data,2);
        Plex_data.CAN1_Load = GET_UINT16_BE_BYTE_(rx_msg.data,4);
        Plex_data.CAN1_Errors = GET_UINT16_BE_BYTE_(rx_msg.data,6);
        break;

      default:
        //ESP_LOGW(TAG,"Unexpected CAN ID %lu",rx_msg.identifier);
        // TODO: Indicate unexpected CAN ID through LoRa
        break;
      }
      break;

    case ESP_ERR_TIMEOUT:
      // ESP_LOGE(TAG,"\n\n\n\n\nCAN Rx Timed out\n\n\n\n\n");
      break;

    default:
      ESP_LOGE(TAG, "\n\n\n\n\n%s\n\n\n\n\n", esp_err_to_name(ret));
      break;
    }
  vTaskDelay(1);
  }
}



void print_can_data_hex(twai_message_t * msg){
  printf("\n0x%lx",msg->identifier);
  for (int i = 0 ; i < msg->data_length_code; i ++){
    printf(",%2hhx",msg->data[i]);
  }

  //printf("\n");
}