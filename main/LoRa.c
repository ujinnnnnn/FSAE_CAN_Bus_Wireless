#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#include "freertos/freertos.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "freertos/semphr.h"

#include "esp_system.h"
#include "esp_log.h"
#include "driver/uart.h"

#include "config.h"
#include "CAN.h"
#include "checksum.h"
#include "base64.h"
#include "LoRa.h"

extern can_DTI_HV_500_t Inv_data;
extern can_OrionBMS2_t BMS_data;
extern can_PLEX_t Plex_data;
extern int8_t Thermistor_data[TEM_MAX_THERMISTORS];
extern can_CellData_t Cell_data[120];

extern SemaphoreHandle_t transmit_sem;
static char TAG[] = "LoRa.c";

/*
    memcpy((void*) &lora_data_buffer[], (void*) & , );
*/

QueueHandle_t LoRa_TX_Queue;
StaticQueue_t LoRa_TX_QueueStruct;

void LoRa_rx_check(void *arg)
{
  static char lora_rx_buffer[LORA_RX_BUFFER_SIZE + 1] = {0};
  while (1)
  {
    if (uart_read_bytes(UART_NUM, (void *)lora_rx_buffer, LORA_RX_BUFFER_SIZE, 0) > 0)
    {
      for (int i = 0; i < LORA_RX_BUFFER_SIZE; i++)
      {
        if (lora_rx_buffer[i] == '$')
        { // found start byte
          switch (lora_rx_buffer[i + 1])
          {
          case 0x10: // start LoRa transmit
            xSemaphoreGive(transmit_sem);
            break;
          case 0x20: // stop LoRa transmit
            xSemaphoreTake(transmit_sem, 0);
            break;
          default:
            break;
          }
        }
        else
        {
          continue;
        }
      }
    }
    vTaskDelay(pdMS_TO_TICKS(LORA_RX_CHECK_PERIOD_MS));
  }
}

uint8_t LoRa_TX_Queue_Storage[64 * sizeof(LoRa_message_t)];

void
LoRa_sender_task(void* arg)
{
  static char lora_tx_buffer[256] = {0};
  lora_tx_buffer[0] = START_BYTE;

  LoRa_TX_Queue = xQueueCreateStatic(
    64, // queue length
    sizeof(LoRa_message_t),
    LoRa_TX_Queue_Storage,
    &LoRa_TX_QueueStruct);

  LoRa_message_t message = {0};
  size_t encoded_str_len = 0;
  while (1){
    if (xQueueReceive(LoRa_TX_Queue,&message,0) == pdTRUE)  {
      lora_tx_buffer[START_BYTES] = message.id;
      encoded_str_len = base64_encode(message.data,message.length,&lora_tx_buffer[START_BYTES + ID_BYTES]);            

      lora_tx_buffer[START_BYTES + ID_BYTES + encoded_str_len] = END_BYTE;
      while(gpio_get_level(AUX_PIN) == 0)vTaskDelay(1);
      uart_write_bytes(UART_NUM, (void*)lora_tx_buffer,START_BYTES + ID_BYTES + encoded_str_len + END_BYTES);
      free(message.data);
    }else{
      vTaskDelay(1);
    }
  }
}

void slow_1_transmit_task(void *arg)
{
#define DATA_BYTES 23
  LoRa_message_t message = {.id = SLOW_1_ID,.length = DATA_BYTES + CRC_BYTES};

  while (uxSemaphoreGetCount(transmit_sem) == 1)
  {
    message.data = malloc(DATA_BYTES + CRC_BYTES);
    memcpy((void *)&message.data[0], (void *)&Inv_data.inverter_temp, 2);
    memcpy((void *)&message.data[2], (void *)&Inv_data.motor_temp, 2);
    memcpy((void *)&message.data[4], (void *)&Inv_data.input_voltage, 2);
    memcpy((void *)&message.data[4], (void *)&Inv_data.input_voltage, 2);
    memcpy((void *)&message.data[6], (void *)&BMS_data.AverageCurrent, 2);
    memcpy((void *)&message.data[8], (void *)&BMS_data.PackOpenVoltage, 2);
    memcpy((void *)&message.data[10], (void *)&BMS_data.PackDCL, 2);
    memcpy((void *)&message.data[12], (void *)&BMS_data.PackAbsCurrent, 2);
    memcpy((void *)&message.data[14], (void *)&Plex_data.RadiatorIN, 2);
    memcpy((void *)&message.data[16], (void *)&Plex_data.RadiatorOUT, 2);
    memcpy((void *)&message.data[18], (void *)&Plex_data.vBat, 2);
    memcpy((void *)&message.data[20], (void *)&BMS_data.PackSOC, 1);
    memcpy((void *)&message.data[21], (void *)&BMS_data.HighTemperature, 1);
    memcpy((void *)&message.data[22], (void *)&BMS_data.InternalTemperature, 1);

    uint16_t crc = crc_16((unsigned char *) message.data,DATA_BYTES);
    memcpy((void*) &message.data[DATA_BYTES],(void *) &crc,CRC_BYTES);

    while (xQueueSend(LoRa_TX_Queue,&message,0) == pdFALSE) ESP_LOGW(TAG,"queue full");

    vTaskDelay(pdMS_TO_TICKS(SLOW_1_TRANSMIT_PERIOD_MS));
#undef DATA_BYTES
  }
}

void slow_2_transmit_task(void *arg)
{
#define DATA_BYTES 21
  LoRa_message_t message = {.id = SLOW_2_ID,.length = DATA_BYTES + CRC_BYTES};

  while (uxSemaphoreGetCount(transmit_sem) == 1)
  {
    message.data = malloc(DATA_BYTES + CRC_BYTES);
    memcpy((void *)&message.data[0], (void *)&Inv_data.FOC_Id, 4);
    memcpy((void *)&message.data[4], (void *)&Inv_data.FOC_Iq, 4);
    memcpy((void *)&message.data[8], (void *)&Plex_data.GPS_Fix, 2);
    memcpy((void *)&message.data[10], (void *)&Plex_data.CAN1_Load, 2);
    memcpy((void *)&message.data[12], (void *)&Plex_data.CAN1_Errors, 2);
    memcpy((void *)&message.data[14], (void *)&Inv_data.DigitalIO, 1);
    memcpy((void *)&message.data[15], (void *)&Inv_data.DriveEN, 1);
    memcpy((void *)&message.data[16], (void *)&Inv_data.CAN_MapVers, 1);
    memcpy((void *)&message.data[17], (void *)&BMS_data.DTC_Flags_1, 1);
    memcpy((void *)&message.data[18], (void *)&BMS_data.DTC_Flags_2, 1);
    memcpy((void *)&message.data[19], (void *)&BMS_data.BalancingEnabled, 1);
    memcpy((void *)&message.data[20], (void *)&BMS_data.DischargeEnableInverted, 1);

    uint16_t crc = crc_16((unsigned char *) message.data,DATA_BYTES);
    memcpy((void*) &message.data[DATA_BYTES],(void *) &crc,CRC_BYTES);

    while (xQueueSend(LoRa_TX_Queue,&message,0) == pdFALSE) ESP_LOGW(TAG,"queue full");

    vTaskDelay(pdMS_TO_TICKS(SLOW_2_TRANSMIT_PERIOD_MS));
#undef DATA_BYTES
  }
}

void fast_critical_transmit_task(void *arg)
{
#define DATA_BYTES 24
  LoRa_message_t message = {.id = FAST_CRITICAL_ID,.length = DATA_BYTES + CRC_BYTES};

  while (uxSemaphoreGetCount(transmit_sem) == 1)
  {
    message.data = malloc(DATA_BYTES + CRC_BYTES);
    memcpy((void *)&message.data[0], (void *)&Inv_data.erpm, 4);
    memcpy((void *)&message.data[4], (void *)&Inv_data.duty_cycle, 2);
    memcpy((void *)&message.data[6], (void *)&Inv_data.ac_current, 2);
    memcpy((void *)&message.data[8], (void *)&Inv_data.dc_current, 2);
    memcpy((void *)&message.data[10], (void *)&Plex_data.Throttle_1, 2);
    memcpy((void *)&message.data[12], (void *)&Plex_data.Throttle_2, 2);
    memcpy((void *)&message.data[14], (void *)&Plex_data.Brake, 2);
    memcpy((void *)&message.data[16], (void *)&BMS_data.HighOpenCellVoltage, 1);
    memcpy((void *)&message.data[17], (void *)&BMS_data.LowOpenCellVoltage, 1);
    memcpy((void *)&message.data[18], (void *)&BMS_data.HighOpenCellID, 1);
    memcpy((void *)&message.data[19], (void *)&BMS_data.LowOpenCellID, 1);
    memcpy((void *)&message.data[20], (void *)&Inv_data.throttle_in, 1);
    memcpy((void *)&message.data[21], (void *)&Inv_data.ActiveLimitsByte4, 1);
    memcpy((void *)&message.data[22], (void *)&Inv_data.ActiveLimitsByte5, 1);
    memcpy((void *)&message.data[23], (void *)&Inv_data.FAULT_CODE, 1);

    uint16_t crc = crc_16((unsigned char *) message.data,DATA_BYTES);
    memcpy((void*) &message.data[DATA_BYTES],(void *) &crc,CRC_BYTES);

    while (xQueueSend(LoRa_TX_Queue,&message,0) == pdFALSE) ESP_LOGW(TAG,"queue full");

    vTaskDelay(pdMS_TO_TICKS(FAST_CRITICAL_TRANSMIT_PERIOD_MS));
#undef DATA_BYTES
  }
}




void fast_information_transmit_task(void *arg)
{
#define DATA_BYTES 14
  LoRa_message_t message = {.id = FAST_INFORMATION_ID ,.length = DATA_BYTES + CRC_BYTES};

  while (uxSemaphoreGetCount(transmit_sem) == 1)
  {
    message.data = malloc(DATA_BYTES + CRC_BYTES);
    memcpy((void *)&message.data[0], (void *)&BMS_data.PackCurrent, 2);
    memcpy((void *)&message.data[2], (void *)&Plex_data.accLong, 2);
    memcpy((void *)&message.data[4], (void *)&Plex_data.accLat, 2);
    memcpy((void *)&message.data[6], (void *)&Plex_data.accVert, 2);
    memcpy((void *)&message.data[8], (void *)&Plex_data.yawRate, 2);
    memcpy((void *)&message.data[10], (void *)&Plex_data.Pitch, 2);
    memcpy((void *)&message.data[12], (void *)&Plex_data.Roll, 2);

    uint16_t crc = crc_16((unsigned char *) message.data,DATA_BYTES);
    memcpy((void*) &message.data[DATA_BYTES],(void *) &crc,CRC_BYTES);

    while (xQueueSend(LoRa_TX_Queue,&message,0) == pdFALSE) ESP_LOGW(TAG,"queue full");

    vTaskDelay(pdMS_TO_TICKS(FAST_INFORMATION_TRANSMIT_PERIOD_MS));
#undef DATA_BYTES
  }
}

void cell_voltage_transmit_task(void *arg)
{
#define DATA_BYTES 24
  LoRa_message_t message = {.length = DATA_BYTES + CRC_BYTES};

  uint8_t segment_id = 0;
  while (uxSemaphoreGetCount(transmit_sem) == 1)
  {
    message.id = 0xF0 | segment_id;

    message.data = malloc(DATA_BYTES + CRC_BYTES);
    memcpy((void *)&message.data[0], (void *)&Cell_data[segment_id + 0].OpenVoltage, 2);
    memcpy((void *)&message.data[2], (void *)&Cell_data[segment_id + 1].OpenVoltage, 2);
    memcpy((void *)&message.data[4], (void *)&Cell_data[segment_id + 2].OpenVoltage, 2);
    memcpy((void *)&message.data[6], (void *)&Cell_data[segment_id + 3].OpenVoltage, 2);
    memcpy((void *)&message.data[8], (void *)&Cell_data[segment_id + 4].OpenVoltage, 2);
    memcpy((void *)&message.data[10], (void *)&Cell_data[segment_id + 5].OpenVoltage, 2);
    memcpy((void *)&message.data[12], (void *)&Cell_data[segment_id + 6].OpenVoltage, 2);
    memcpy((void *)&message.data[14], (void *)&Cell_data[segment_id + 7].OpenVoltage, 2);
    memcpy((void *)&message.data[16], (void *)&Cell_data[segment_id + 8].OpenVoltage, 2);
    memcpy((void *)&message.data[18], (void *)&Cell_data[segment_id + 9].OpenVoltage, 2);
    memcpy((void *)&message.data[20], (void *)&Cell_data[segment_id + 10].OpenVoltage, 2);
    memcpy((void *)&message.data[22], (void *)&Cell_data[segment_id + 11].OpenVoltage, 2);

    uint16_t crc = crc_16((unsigned char *) message.data,DATA_BYTES);
    memcpy((void*) &message.data[DATA_BYTES],(void *) &crc,CRC_BYTES);

    segment_id = (segment_id + 1) % 10;

    while (xQueueSend(LoRa_TX_Queue,&message,0) == pdFALSE) ESP_LOGW(TAG,"queue full");

    vTaskDelay(pdMS_TO_TICKS(CELL_VOLTAGE_TRANSMIT_PERIOD_MS));
#undef DATA_BYTES
  }
}


void thermistor_transmit_task(void *arg)
{
#define DATA_BYTES 8
  LoRa_message_t message = {.length = DATA_BYTES + CRC_BYTES};

  uint8_t thermsitor_group_id = 0;

  while (uxSemaphoreGetCount(transmit_sem) == 1)
  {
    message.data = malloc(DATA_BYTES + CRC_BYTES);
    message.id = 0xE0 | thermsitor_group_id;

    memcpy((void *)&message.data[0], (void *)&Thermistor_data[thermsitor_group_id + 0], 1);
    memcpy((void *)&message.data[1], (void *)&Thermistor_data[thermsitor_group_id + 1], 1);
    memcpy((void *)&message.data[2], (void *)&Thermistor_data[thermsitor_group_id + 2], 1);
    memcpy((void *)&message.data[3], (void *)&Thermistor_data[thermsitor_group_id + 3], 1);
    memcpy((void *)&message.data[4], (void *)&Thermistor_data[thermsitor_group_id + 4], 1);
    memcpy((void *)&message.data[5], (void *)&Thermistor_data[thermsitor_group_id + 5], 1);
    memcpy((void *)&message.data[6], (void *)&Thermistor_data[thermsitor_group_id + 6], 1);
    memcpy((void *)&message.data[7], (void *)&Thermistor_data[thermsitor_group_id + 7], 1);

    uint16_t crc = crc_16((unsigned char *) message.data,DATA_BYTES);
    memcpy((void*) &message.data[DATA_BYTES],(void *) &crc,CRC_BYTES);

    while (xQueueSend(LoRa_TX_Queue,&message,0) == pdFALSE) ESP_LOGW(TAG,"queue full");

    thermsitor_group_id = (thermsitor_group_id + 1) % 10;

    vTaskDelay(pdMS_TO_TICKS(CELL_VOLTAGE_TRANSMIT_PERIOD_MS));
#undef DATA_BYTES
  }
}
