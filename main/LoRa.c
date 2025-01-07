#include <stdint.h>
#include <string.h>

#include "freertos/freertos.h"
#include "freertos/task.h"
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

SemaphoreHandle_t lora_uart_mutex;

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

void slow_1_transmit_task(void *arg)
{
#define DATA_BYTES 23
  uint8_t lora_data_buffer[DATA_BYTES + CRC_BYTES] = {0};
  uint8_t lora_tx_buffer[START_BYTES + END_BYTES + ID_BYTES + BASE64_ENCODE_OUT_SIZE(DATA_BYTES+CRC_BYTES)];

  lora_tx_buffer[0] = START_BYTE;
  lora_tx_buffer[1] = SLOW_1_ID;

  lora_tx_buffer[START_BYTES + END_BYTES + ID_BYTES + BASE64_ENCODE_OUT_SIZE(DATA_BYTES+CRC_BYTES) - 1] = END_BYTE;

  while (uxSemaphoreGetCount(transmit_sem) == 1)
  {
    memcpy((void *)&lora_data_buffer[0], (void *)&Inv_data.inverter_temp, 2);
    memcpy((void *)&lora_data_buffer[2], (void *)&Inv_data.motor_temp, 2);
    memcpy((void *)&lora_data_buffer[4], (void *)&Inv_data.input_voltage, 2);
    memcpy((void *)&lora_data_buffer[6], (void *)&BMS_data.AverageCurrent, 2);
    memcpy((void *)&lora_data_buffer[8], (void *)&BMS_data.PackOpenVoltage, 2);
    memcpy((void *)&lora_data_buffer[10], (void *)&BMS_data.PackDCL, 2);
    memcpy((void *)&lora_data_buffer[12], (void *)&BMS_data.PackAbsCurrent, 2);
    memcpy((void *)&lora_data_buffer[14], (void *)&Plex_data.RadiatorIN, 2);
    memcpy((void *)&lora_data_buffer[16], (void *)&Plex_data.RadiatorOUT, 2);
    memcpy((void *)&lora_data_buffer[18], (void *)&Plex_data.vBat, 2);
    memcpy((void *)&lora_data_buffer[20], (void *)&BMS_data.PackSOC, 1);
    memcpy((void *)&lora_data_buffer[21], (void *)&BMS_data.HighTemperature, 1);
    memcpy((void *)&lora_data_buffer[22], (void *)&BMS_data.InternalTemperature, 1);

    uint16_t crc = crc_16((unsigned char *) lora_data_buffer,DATA_BYTES);
    memcpy((void*) &lora_data_buffer[DATA_BYTES],(void *) &crc,START_BYTES + ID_BYTES);

    base64_encode(lora_data_buffer,DATA_BYTES+CRC_BYTES,(char *) &lora_tx_buffer[2]);

    xSemaphoreTake(lora_uart_mutex,portMAX_DELAY);
    while(gpio_get_level(AUX_PIN) == 0)vTaskDelay(1);
    uart_write_bytes(UART_NUM, (void *)lora_tx_buffer, START_BYTES + END_BYTES + ID_BYTES + BASE64_ENCODE_OUT_SIZE(DATA_BYTES+CRC_BYTES));
    xSemaphoreGive(lora_uart_mutex);

    vTaskDelay(pdMS_TO_TICKS(SLOW_1_TRANSMIT_PERIOD_MS));
#undef DATA_BYTES
  }
}

void slow_2_transmit_task(void *arg)
{
#define DATA_BYTES 21
  uint8_t lora_data_buffer[DATA_BYTES + CRC_BYTES] = {0};
  uint8_t lora_tx_buffer[START_BYTES + END_BYTES + ID_BYTES + BASE64_ENCODE_OUT_SIZE(DATA_BYTES+CRC_BYTES)];

  lora_tx_buffer[0] = START_BYTE;
  lora_tx_buffer[1] = SLOW_2_ID;

  lora_tx_buffer[START_BYTES + END_BYTES + ID_BYTES + BASE64_ENCODE_OUT_SIZE(DATA_BYTES+CRC_BYTES) - 1] = END_BYTE;

  while (uxSemaphoreGetCount(transmit_sem) == 1)
  {
    memcpy((void *)&lora_data_buffer[0], (void *)&Inv_data.FOC_Id, 4);
    memcpy((void *)&lora_data_buffer[4], (void *)&Inv_data.FOC_Iq, 4);
    memcpy((void *)&lora_data_buffer[8], (void *)&Plex_data.GPS_Fix, 2);
    memcpy((void *)&lora_data_buffer[10], (void *)&Plex_data.CAN1_Load, 2);
    memcpy((void *)&lora_data_buffer[12], (void *)&Plex_data.CAN1_Errors, 2);
    memcpy((void *)&lora_data_buffer[14], (void *)&Inv_data.DigitalIO, 1);
    memcpy((void *)&lora_data_buffer[15], (void *)&Inv_data.DriveEN, 1);
    memcpy((void *)&lora_data_buffer[16], (void *)&Inv_data.CAN_MapVers, 1);
    memcpy((void *)&lora_data_buffer[17], (void *)&BMS_data.DTC_Flags_1, 1);
    memcpy((void *)&lora_data_buffer[18], (void *)&BMS_data.DTC_Flags_2, 1);
    memcpy((void *)&lora_data_buffer[19], (void *)&BMS_data.BalancingEnabled, 1);
    memcpy((void *)&lora_data_buffer[20], (void *)&BMS_data.DischargeEnableInverted, 1);

    uint16_t crc = crc_16((unsigned char *) lora_data_buffer,DATA_BYTES);
    memcpy((void*) &lora_data_buffer[DATA_BYTES],(void *) &crc,START_BYTES + ID_BYTES);

    base64_encode(lora_data_buffer,DATA_BYTES+CRC_BYTES,(char *) &lora_tx_buffer[2]);

    xSemaphoreTake(lora_uart_mutex,portMAX_DELAY);
    while(gpio_get_level(AUX_PIN) == 0)vTaskDelay(1);
    uart_write_bytes(UART_NUM, (void *)lora_tx_buffer, START_BYTES + END_BYTES + ID_BYTES + BASE64_ENCODE_OUT_SIZE(DATA_BYTES+CRC_BYTES));
    xSemaphoreGive(lora_uart_mutex);
    
    vTaskDelay(pdMS_TO_TICKS(SLOW_2_TRANSMIT_PERIOD_MS));
#undef DATA_BYTES
  }
}

void fast_critical_transmit_task(void *arg)
{
#define DATA_BYTES 24
  uint8_t lora_data_buffer[DATA_BYTES + CRC_BYTES] = {0};
  uint8_t lora_tx_buffer[START_BYTES + END_BYTES + ID_BYTES + BASE64_ENCODE_OUT_SIZE(DATA_BYTES+CRC_BYTES)];

  lora_tx_buffer[0] = START_BYTE;
  lora_tx_buffer[1] = FAST_CRITICAL_ID;

  lora_tx_buffer[START_BYTES + END_BYTES + ID_BYTES + BASE64_ENCODE_OUT_SIZE(DATA_BYTES+CRC_BYTES) - 1] = END_BYTE;

  while (uxSemaphoreGetCount(transmit_sem) == 1)
  {
    memcpy((void *)&lora_data_buffer[0], (void *)&Inv_data.erpm, 4);
    memcpy((void *)&lora_data_buffer[4], (void *)&Inv_data.duty_cycle, 2);
    memcpy((void *)&lora_data_buffer[6], (void *)&Inv_data.ac_current, 2);
    memcpy((void *)&lora_data_buffer[8], (void *)&Inv_data.dc_current, 2);
    memcpy((void *)&lora_data_buffer[10], (void *)&Plex_data.Throttle_1, 2);
    memcpy((void *)&lora_data_buffer[12], (void *)&Plex_data.Throttle_2, 2);
    memcpy((void *)&lora_data_buffer[14], (void *)&Plex_data.Brake, 2);
    memcpy((void *)&lora_data_buffer[16], (void *)&BMS_data.HighOpenCellVoltage, 1);
    memcpy((void *)&lora_data_buffer[17], (void *)&BMS_data.LowOpenCellVoltage, 1);
    memcpy((void *)&lora_data_buffer[18], (void *)&BMS_data.HighOpenCellID, 1);
    memcpy((void *)&lora_data_buffer[19], (void *)&BMS_data.LowOpenCellID, 1);
    memcpy((void *)&lora_data_buffer[20], (void *)&Inv_data.throttle_in, 1);
    memcpy((void *)&lora_data_buffer[21], (void *)&Inv_data.ActiveLimitsByte4, 1);
    memcpy((void *)&lora_data_buffer[22], (void *)&Inv_data.ActiveLimitsByte5, 1);
    memcpy((void *)&lora_data_buffer[23], (void *)&Inv_data.FAULT_CODE, 1);

    uint16_t crc = crc_16((unsigned char *) lora_data_buffer,DATA_BYTES);
    memcpy((void*) &lora_data_buffer[DATA_BYTES],(void *) &crc,START_BYTES + ID_BYTES);

    base64_encode(lora_data_buffer,DATA_BYTES+CRC_BYTES,(char *) &lora_tx_buffer[2]);

    xSemaphoreTake(lora_uart_mutex,portMAX_DELAY);
    while(gpio_get_level(AUX_PIN) == 0)vTaskDelay(1);
    uart_write_bytes(UART_NUM, (void *)lora_tx_buffer, START_BYTES + END_BYTES + ID_BYTES + BASE64_ENCODE_OUT_SIZE(DATA_BYTES+CRC_BYTES));
    xSemaphoreGive(lora_uart_mutex);

    vTaskDelay(pdMS_TO_TICKS(FAST_CRITICAL_TRANSMIT_PERIOD_MS));
#undef DATA_BYTES
  }
}
void fast_information_transmit_task(void *arg)
{
#define DATA_BYTES 14
  uint8_t lora_data_buffer[DATA_BYTES + CRC_BYTES] = {0};
  uint8_t lora_tx_buffer[START_BYTES + END_BYTES + ID_BYTES + BASE64_ENCODE_OUT_SIZE(DATA_BYTES+CRC_BYTES)];

  lora_tx_buffer[0] = START_BYTE;
  lora_tx_buffer[1] = FAST_INFORMATION_ID;

  lora_tx_buffer[START_BYTES + END_BYTES + ID_BYTES + BASE64_ENCODE_OUT_SIZE(DATA_BYTES+CRC_BYTES) - 1] = END_BYTE;

  while (uxSemaphoreGetCount(transmit_sem) == 1)
  {
    memcpy((void *)&lora_data_buffer[0], (void *)&BMS_data.PackCurrent, 2);
    memcpy((void *)&lora_data_buffer[2], (void *)&Plex_data.accLong, 2);
    memcpy((void *)&lora_data_buffer[4], (void *)&Plex_data.accLat, 2);
    memcpy((void *)&lora_data_buffer[6], (void *)&Plex_data.accVert, 2);
    memcpy((void *)&lora_data_buffer[8], (void *)&Plex_data.yawRate, 2);
    memcpy((void *)&lora_data_buffer[10], (void *)&Plex_data.Pitch, 2);
    memcpy((void *)&lora_data_buffer[12], (void *)&Plex_data.Roll, 2);

    uint16_t crc = crc_16((unsigned char *) lora_data_buffer,DATA_BYTES);
    memcpy((void*) &lora_data_buffer[DATA_BYTES],(void *) &crc,START_BYTES + ID_BYTES);

    base64_encode(lora_data_buffer,DATA_BYTES+CRC_BYTES,(char *) &lora_tx_buffer[2]);

    xSemaphoreTake(lora_uart_mutex,portMAX_DELAY);
    while(gpio_get_level(AUX_PIN) == 0)vTaskDelay(1);
    uart_write_bytes(UART_NUM, (void *)lora_tx_buffer, START_BYTES + END_BYTES + ID_BYTES + BASE64_ENCODE_OUT_SIZE(DATA_BYTES+CRC_BYTES));
    xSemaphoreGive(lora_uart_mutex);

    vTaskDelay(pdMS_TO_TICKS(FAST_INFORMATION_TRANSMIT_PERIOD_MS));
#undef DATA_BYTES
  }
}

void cell_voltage_transmit_task(void *arg)
{
#define DATA_BYTES 24
  uint8_t lora_data_buffer[DATA_BYTES + CRC_BYTES] = {0};
  uint8_t lora_tx_buffer[START_BYTES + END_BYTES + ID_BYTES + BASE64_ENCODE_OUT_SIZE(DATA_BYTES+CRC_BYTES)];

  lora_tx_buffer[0] = START_BYTE;

  lora_tx_buffer[START_BYTES + END_BYTES + ID_BYTES + BASE64_ENCODE_OUT_SIZE(DATA_BYTES+CRC_BYTES) - 1] = END_BYTE;

  uint8_t segment_id = 0;

  while (uxSemaphoreGetCount(transmit_sem) == 1)
  {
    lora_tx_buffer[1] = 0xF0 | segment_id;

    memcpy((void *)&lora_data_buffer[0], (void *)&Cell_data[segment_id + 0].OpenVoltage, 2);
    memcpy((void *)&lora_data_buffer[2], (void *)&Cell_data[segment_id + 1].OpenVoltage, 2);
    memcpy((void *)&lora_data_buffer[4], (void *)&Cell_data[segment_id + 2].OpenVoltage, 2);
    memcpy((void *)&lora_data_buffer[6], (void *)&Cell_data[segment_id + 3].OpenVoltage, 2);
    memcpy((void *)&lora_data_buffer[8], (void *)&Cell_data[segment_id + 4].OpenVoltage, 2);
    memcpy((void *)&lora_data_buffer[10], (void *)&Cell_data[segment_id + 5].OpenVoltage, 2);
    memcpy((void *)&lora_data_buffer[12], (void *)&Cell_data[segment_id + 6].OpenVoltage, 2);
    memcpy((void *)&lora_data_buffer[14], (void *)&Cell_data[segment_id + 7].OpenVoltage, 2);
    memcpy((void *)&lora_data_buffer[16], (void *)&Cell_data[segment_id + 8].OpenVoltage, 2);
    memcpy((void *)&lora_data_buffer[18], (void *)&Cell_data[segment_id + 9].OpenVoltage, 2);
    memcpy((void *)&lora_data_buffer[20], (void *)&Cell_data[segment_id + 10].OpenVoltage, 2);
    memcpy((void *)&lora_data_buffer[22], (void *)&Cell_data[segment_id + 11].OpenVoltage, 2);

    uint16_t crc = crc_16((unsigned char *) lora_data_buffer,DATA_BYTES);
    memcpy((void*) &lora_data_buffer[DATA_BYTES],(void *) &crc,START_BYTES + ID_BYTES);

    base64_encode(lora_data_buffer,DATA_BYTES+CRC_BYTES,(char *) &lora_tx_buffer[2]);

    xSemaphoreTake(lora_uart_mutex,portMAX_DELAY);
    while(gpio_get_level(AUX_PIN) == 0)vTaskDelay(1);
    uart_write_bytes(UART_NUM, (void *)lora_tx_buffer, START_BYTES + END_BYTES + ID_BYTES + BASE64_ENCODE_OUT_SIZE(DATA_BYTES+CRC_BYTES));
    xSemaphoreGive(lora_uart_mutex);

    segment_id = (segment_id + 1) % 10;

    vTaskDelay(pdMS_TO_TICKS(CELL_VOLTAGE_TRANSMIT_PERIOD_MS));
#undef DATA_BYTES
  }
}


void thermistor_transmit_task(void *arg)
{
#define DATA_BYTES 8
  uint8_t lora_data_buffer[DATA_BYTES + CRC_BYTES] = {0};
  uint8_t lora_tx_buffer[START_BYTES + END_BYTES + ID_BYTES + BASE64_ENCODE_OUT_SIZE(DATA_BYTES+CRC_BYTES)];

  lora_tx_buffer[0] = START_BYTE;

  lora_tx_buffer[START_BYTES + END_BYTES + ID_BYTES + BASE64_ENCODE_OUT_SIZE(DATA_BYTES+CRC_BYTES) - 1] = END_BYTE;

  uint8_t thermsitor_group_id = 0;

  while (uxSemaphoreGetCount(transmit_sem) == 1)
  {
    lora_tx_buffer[1] = 0xE0 | thermsitor_group_id;

    memcpy((void *)&lora_data_buffer[0], (void *)&Thermistor_data[thermsitor_group_id+ 0], 1);
    memcpy((void *)&lora_data_buffer[1], (void *)&Thermistor_data[thermsitor_group_id+ 1], 1);
    memcpy((void *)&lora_data_buffer[2], (void *)&Thermistor_data[thermsitor_group_id+ 2], 1);
    memcpy((void *)&lora_data_buffer[3], (void *)&Thermistor_data[thermsitor_group_id+ 3], 1);
    memcpy((void *)&lora_data_buffer[4], (void *)&Thermistor_data[thermsitor_group_id+ 4], 1);
    memcpy((void *)&lora_data_buffer[5], (void *)&Thermistor_data[thermsitor_group_id+ 5], 1);
    memcpy((void *)&lora_data_buffer[6], (void *)&Thermistor_data[thermsitor_group_id+ 6], 1);
    memcpy((void *)&lora_data_buffer[7], (void *)&Thermistor_data[thermsitor_group_id+ 7], 1);

    uint16_t crc = crc_16((unsigned char *) lora_data_buffer,DATA_BYTES);
    memcpy((void*) &lora_data_buffer[DATA_BYTES],(void *) &crc,START_BYTES + ID_BYTES);

    base64_encode(lora_data_buffer,DATA_BYTES+CRC_BYTES,(char *) &lora_tx_buffer[2]);

    xSemaphoreTake(lora_uart_mutex,portMAX_DELAY);
    while(gpio_get_level(AUX_PIN) == 0)vTaskDelay(1);
    uart_write_bytes(UART_NUM, (void *)lora_tx_buffer, START_BYTES + END_BYTES + ID_BYTES + BASE64_ENCODE_OUT_SIZE(DATA_BYTES+CRC_BYTES));
    xSemaphoreGive(lora_uart_mutex);

    thermsitor_group_id = (thermsitor_group_id + 1) % 10;
    vTaskDelay(pdMS_TO_TICKS(CELL_VOLTAGE_TRANSMIT_PERIOD_MS));
#undef DATA_BYTES
  }
}