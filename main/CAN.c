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

extern SemaphoreHandle_t start_sem;

static char TAG[] = "CAN.c";

void twai_receive_task(void *arg)
{
  xSemaphoreTake(start_sem, portMAX_DELAY);
  esp_err_t ret;

  twai_message_t rx_msg;
  while (1)
  {
    ret = twai_receive(&rx_msg, pdMS_TO_TICKS(1000));

    switch (ret)
    {
    case ESP_OK:
      switch (rx_msg.identifier)
      {
      case Inverter_Message_1_ID:
        Inv_data.erpm = GET_INT32_BE_BYTE_(rx_msg.data, 0);
        Inv_data.duty_cycle = (float)GET_INT16_BE_BYTE_(rx_msg.data, 4) / 10;
        Inv_data.input_voltage = GET_INT16_BE_BYTE_(rx_msg.data, 6);
        break;
      case Inverter_Message_2_ID:
        Inv_data.ac_current = (float)GET_INT16_BE_BYTE_(rx_msg.data, 0) / 10;
        Inv_data.dc_current = (float)GET_INT16_BE_BYTE_(rx_msg.data, 2) / 10;
        break;
      case Inverter_Message_3_ID:
        Inv_data.inverter_temp = (float)GET_INT16_BE_BYTE_(rx_msg.data, 0) / 10;
        Inv_data.motor_temp = (float)GET_INT16_BE_BYTE_(rx_msg.data, 2) / 10;
        Inv_data.FAULT_CODE = rx_msg.data[4];
        break;
      case Inverter_Message_4_ID:
        Inv_data.FOC_Id = (float)GET_INT32_BE_BYTE_(rx_msg.data, 0) / 100;
        Inv_data.FOC_Iq = (float)GET_INT32_BE_BYTE_(rx_msg.data, 4) / 100;
        break;

      case BMS_Cell_Broadcast_ID:
        uint8_t checkSum = 0xFF & (BMS_Cell_Broadcast_ID + 8 + rx_msg.data[0] + rx_msg.data[1] + rx_msg.data[2] + rx_msg.data[3] + rx_msg.data[4] + rx_msg.data[5] + rx_msg.data[6]);
        if (checkSum == rx_msg.data[7])
        {
          Cell_data[(uint8_t)rx_msg.data[0]].InstantVoltage = (float)GET_UINT16_BE_BYTE_(rx_msg.data, 1) / 10;
          Cell_data[(uint8_t)rx_msg.data[0]].InternalResistance = (float)GET_UINT16_BE_BYTE_(rx_msg.data, 3) / 100;
          Cell_data[(uint8_t)rx_msg.data[0]].OpenVoltage = (float)GET_UINT16_BE_BYTE_(rx_msg.data, 5) / 10;
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
        break;

      case BMS_Message_2_ID:
        BMS_data.PackAbsCurrent = GET_UINT16_BE_BYTE_(rx_msg.data, 0);
        BMS_data.AverageCurrent = GET_UINT16_BE_BYTE_(rx_msg.data, 2);
        break;

      case BMS_Message_3_ID:
        BMS_data.MaxCellVoltage = ((float)(GET_UINT16_BE_BYTE_(rx_msg.data, 0) * 1000)) / 120;
        BMS_data.PackCCL = (float)GET_UINT16_BE_BYTE_(rx_msg.data, 2) / 10;
        BMS_data.ChargerSafetyRelayFault = rx_msg.data[4] >> 7; // P0A08
        break;

      case BMS_Message_4_ID:
        BMS_data.MaxPackVoltage = GET_UINT16_BE_BYTE_(rx_msg.data, 0);
        BMS_data.PackCCL = (float)GET_UINT16_BE_BYTE_(rx_msg.data, 2) / 10;
        BMS_data.ChargerSafetyRelayFault = rx_msg.data[4] >> 7; // P0A08
        break;

      case BMS_Message_5_ID:
        BMS_data.MaxCellVoltage = ((float)(GET_UINT16_BE_BYTE_(rx_msg.data, 0) * 1000)) / 120;
        BMS_data.PackCCL = (float)GET_UINT16_BE_BYTE_(rx_msg.data, 2) / 10;
        BMS_data.ChargerSafetyRelayFault = rx_msg.data[4] >> 7; // P0A08
        break;

      case TEM_Message_3_ID:
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
        Plex_data.vBat = GET_UINT16_BE_BYTE_(rx_msg.data,0);
        Plex_data.GPS_Fix = GET_UINT16_BE_BYTE_(rx_msg.data,2);
        Plex_data.CAN1_Load = GET_UINT16_BE_BYTE_(rx_msg.data,4);
        Plex_data.CAN1_Errors = GET_UINT16_BE_BYTE_(rx_msg.data,6);
        break;

      default:
        ESP_LOGW(TAG,"Unexpected CAN ID %lu",rx_msg.identifier);
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
  }
}