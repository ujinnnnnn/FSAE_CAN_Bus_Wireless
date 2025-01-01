#include <stdint.h>
#include <string.h>

#include "freertos/freertos.h"
#include "freertos/task.h"
#include "freertos/semphr.h"

#include "esp_system.h"
#include "driver/uart.h"

#include "config.h"
#include "CAN.h"

#include "LoRa.h"


extern can_DTI_HV_500_t Inv_data;
extern can_OrionBMS2_t BMS_data;
extern can_PLEX_t Plex_data;
extern int8_t Thermistor_data[TEM_MAX_THERMISTORS];
extern can_CellData_t Cell_data[120];

extern SemaphoreHandle_t setup_sem;

extern SemaphoreHandle_t transmit_sem;

/*
    memcpy((void*) &lora_tx_buffer[], (void*) & , );
*/

void LoRa_rx_check(void * arg){
  while (uxSemaphoreGetCount(setup_sem) == 0);
  static char lora_rx_buffer[LORA_RX_BUFFER_SIZE + 1] = {0};
  while (1){
    if (uart_read_bytes(UART_NUM,(void* ) lora_rx_buffer,LORA_RX_BUFFER_SIZE,0) > 0){
      for (int i = 0;i < LORA_RX_BUFFER_SIZE;i++){
        if (lora_rx_buffer[i] == '$'){ // found start byte
          switch (lora_rx_buffer[i+1]){
            case 0xAA: // start LoRa transmit
              xSemaphoreGive(transmit_sem);
            break;
            case 0xBB: // stop LoRa transmit
              xSemaphoreTake(transmit_sem,0);
            break;
            default:
            break;
          }
        }else{
          continue;
        }
      }
    }
    vTaskDelay(pdMS_TO_TICKS(LORA_RX_CHECK_PERIOD_MS));
  }
}

void slow_1_transmit_task(void *arg){

  uint8_t lora_tx_buffer[42] = {0};
  lora_tx_buffer[0] = '$';
  lora_tx_buffer[1] = SLOW_1_ID;


  while (uxSemaphoreGetCount(setup_sem) == 0);

  while (uxSemaphoreGetCount(transmit_sem) == 1){
    memcpy((void*) &lora_tx_buffer[3], (void*) &Inv_data.inverter_temp,4);
    memcpy((void*) &lora_tx_buffer[7], (void*) &Inv_data.motor_temp, 4);
    memcpy((void*) &lora_tx_buffer[11], (void*) &Plex_data.vBat, 4);
    memcpy((void*) &lora_tx_buffer[15], (void*) &Plex_data.RadiatorIN, 4);
    memcpy((void*) &lora_tx_buffer[19], (void*) &Plex_data.RadiatorOut, 4);
    memcpy((void*) &lora_tx_buffer[23], (void*) &BMS_data.PackCCL, 4);
    memcpy((void*) &lora_tx_buffer[27], (void*) &BMS_data.PackDCL, 2);
    memcpy((void*) &lora_tx_buffer[29], (void*) &Inv_data.input_voltage, 2);
    memcpy((void*) &lora_tx_buffer[31], (void*) &BMS_data.PackAbsCurrent, 2);
    memcpy((void*) &lora_tx_buffer[33], (void*) &BMS_data.AverageCurrent, 2);
    memcpy((void*) &lora_tx_buffer[35], (void*) &BMS_data.MaxPackVoltage, 2);
    memcpy((void*) &lora_tx_buffer[37], (void*) &BMS_data.PackSOC, 1);
    memcpy((void*) &lora_tx_buffer[38], (void*) &BMS_data.HighTemperature, 1);
    memcpy((void*) &lora_tx_buffer[39], (void*) &BMS_data.InternalTemperature, 1);

    uint16_t crc = crc_16((unsigned char *) &lora_tx_buffer[3],37);
    memcpy((void*) &lora_tx_buffer[39],(void *) &crc,2);

    lora_tx_buffer[41] = '#';

    uart_write_bytes(UART_NUM,lora_tx_buffer,42);
    vTaskDelay(pdMS_TO_TICKS(SLOW_1_TRANSMIT_PERIOD_MS));
  }
}

void slow_2_transmit_task(void *arg){
  uint8_t lora_tx_buffer[28] = {0};
  lora_tx_buffer[0] = '$';
  lora_tx_buffer[1] = SLOW_2_ID;

  while (uxSemaphoreGetCount(setup_sem) == 0);

  while (uxSemaphoreGetCount(transmit_sem) == 1){
    memcpy((void*) &lora_tx_buffer[3], (void*) &Inv_data.FOC_Id,4);
    memcpy((void*) &lora_tx_buffer[7], (void*) &Inv_data.FOC_Iq, 4);
    memcpy((void*) &lora_tx_buffer[11], (void*) &Plex_data.GPS_Fix ,2);
    memcpy((void*) &lora_tx_buffer[13], (void*) &Plex_data.CAN1_Load , 2);
    memcpy((void*) &lora_tx_buffer[15], (void*) &Plex_data.CAN1_Errors , 2);
    memcpy((void*) &lora_tx_buffer[17], (void*) &Inv_data.DigitalIO , 1);
    memcpy((void*) &lora_tx_buffer[18], (void*) &Inv_data.DriveEN , 1);
    memcpy((void*) &lora_tx_buffer[19], (void*) &Inv_data.CAN_MapVers , 1);
    memcpy((void*) &lora_tx_buffer[20], (void*) &BMS_data.bitfield1 , 1);
    memcpy((void*) &lora_tx_buffer[21], (void*) &BMS_data.bitfield2 , 1);
    memcpy((void*) &lora_tx_buffer[22], (void*) &BMS_data.bitfield3, 1);
    memcpy((void*) &lora_tx_buffer[23], (void*) &BMS_data.bitfield4, 1);
    memcpy((void*) &lora_tx_buffer[24], (void*) &BMS_data.DischargeEnableInverted , 1);
    memcpy((void*) &lora_tx_buffer[25], (void*) &BMS_data.ChargerSafetyRelayFault , 1);

    uint16_t crc = crc_16((unsigned char *) &lora_tx_buffer[3],23);
    memcpy((void*) &lora_tx_buffer[25],(void *) &crc,2);

    lora_tx_buffer[27] = '#';

    uart_write_bytes(UART_NUM,lora_tx_buffer,28);
    vTaskDelay(pdMS_TO_TICKS(SLOW_2_TRANSMIT_PERIOD_MS));
  }
}

void fast_critical_transmit_task(void *arg){
  uint8_t lora_tx_buffer[35] = {0};
  lora_tx_buffer[0] = '$';
  lora_tx_buffer[1] = FAST_CRITICAL_ID;

  while (uxSemaphoreGetCount(setup_sem) == 0);

  while (uxSemaphoreGetCount(transmit_sem) == 1){
    memcpy((void*) &lora_tx_buffer[3], (void*) &Inv_data.duty_cycle , 4);
    memcpy((void*) &lora_tx_buffer[7], (void*) &Inv_data.ac_current , 4);
    memcpy((void*) &lora_tx_buffer[11], (void*) &Inv_data.dc_current , 4);
    memcpy((void*) &lora_tx_buffer[15], (void*) &BMS_data.MaxCellVoltage , 4);
    memcpy((void*) &lora_tx_buffer[19], (void*) &Inv_data.erpm , 4);
    memcpy((void*) &lora_tx_buffer[23], (void*) &Plex_data.Throttle_1 , 2);
    memcpy((void*) &lora_tx_buffer[25], (void*) &Plex_data.Throttle_2, 2);
    memcpy((void*) &lora_tx_buffer[27], (void*) &Plex_data.Brake , 2);
    memcpy((void*) &lora_tx_buffer[29], (void*) &Inv_data.throttle_in , 1);
    memcpy((void*) &lora_tx_buffer[30], (void*) &Inv_data.ActiveLimitsByte4 , 1);
    memcpy((void*) &lora_tx_buffer[31], (void*) &Inv_data.ActiveLimitsByte5 , 1);
    memcpy((void*) &lora_tx_buffer[32], (void*) &Inv_data.FAULT_CODE , 1);

    uint16_t crc = crc_16((unsigned char *) &lora_tx_buffer[3],30);
    memcpy((void*) &lora_tx_buffer[32],(void *) &crc,2);

    lora_tx_buffer[34] = '#';

    uart_write_bytes(UART_NUM,lora_tx_buffer,35);
    vTaskDelay(pdMS_TO_TICKS(FAST_CRITICAL_TRANSMIT_PERIOD_MS));
  }
}
void fast_information_transmit_task(void *arg){
  uint8_t lora_tx_buffer[19] = {0};
  lora_tx_buffer[0] = '$';
  lora_tx_buffer[1] = FAST_CRITICAL_ID;

  while (uxSemaphoreGetCount(setup_sem) == 0);

  while (uxSemaphoreGetCount(transmit_sem) == 1){
    memcpy((void*) &lora_tx_buffer[3], (void*) &BMS_data.PackCurrent , 2);
    memcpy((void*) &lora_tx_buffer[5], (void*) &Plex_data.accLong , 2);
    memcpy((void*) &lora_tx_buffer[7], (void*) &Plex_data.accLat , 2);
    memcpy((void*) &lora_tx_buffer[9], (void*) &Plex_data.accVert , 2);
    memcpy((void*) &lora_tx_buffer[11], (void*) &Plex_data.yawRate , 2);
    memcpy((void*) &lora_tx_buffer[13], (void*) &Plex_data.Pitch , 2);
    memcpy((void*) &lora_tx_buffer[15], (void*) &Plex_data.Roll , 2);

    uint16_t crc = crc_16((unsigned char *) &lora_tx_buffer[3],14);
    memcpy((void*) &lora_tx_buffer[16],(void *) &crc,2);

    lora_tx_buffer[18] = '#';

    uart_write_bytes(UART_NUM,lora_tx_buffer,19);
    vTaskDelay(pdMS_TO_TICKS(FAST_INFORMATION_TRANSMIT_PERIOD_MS));
  }
}

void cell_voltage_transmit_task(void *arg){
  uint8_t lora_tx_buffer[53] = {0};
  lora_tx_buffer[0] = '$';

  uint8_t segment_id = 0;

  while (uxSemaphoreGetCount(setup_sem) == 0);

  while (uxSemaphoreGetCount(transmit_sem) == 1){
    lora_tx_buffer[1] = 0xF0 | segment_id;

    memcpy((void*) &lora_tx_buffer[3], (void*) &Cell_data[segment_id + 0] , 4);
    memcpy((void*) &lora_tx_buffer[7], (void*) &Cell_data[segment_id + 1] , 4);
    memcpy((void*) &lora_tx_buffer[11], (void*) &Cell_data[segment_id + 2] , 4);
    memcpy((void*) &lora_tx_buffer[15], (void*) &Cell_data[segment_id + 3] , 4);
    memcpy((void*) &lora_tx_buffer[19], (void*) &Cell_data[segment_id + 4] , 4);
    memcpy((void*) &lora_tx_buffer[23], (void*) &Cell_data[segment_id + 5] , 4);
    memcpy((void*) &lora_tx_buffer[27], (void*) &Cell_data[segment_id + 6] , 4);
    memcpy((void*) &lora_tx_buffer[31], (void*) &Cell_data[segment_id + 7] , 4);
    memcpy((void*) &lora_tx_buffer[35], (void*) &Cell_data[segment_id + 8] , 4);
    memcpy((void*) &lora_tx_buffer[39], (void*) &Cell_data[segment_id + 9] , 4);
    memcpy((void*) &lora_tx_buffer[43], (void*) &Cell_data[segment_id + 10] , 4);
    memcpy((void*) &lora_tx_buffer[47], (void*) &Cell_data[segment_id + 11] , 4);

    uint16_t crc = crc_16((unsigned char *) &lora_tx_buffer[3],48);
    memcpy((void*) &lora_tx_buffer[50],(void *) &crc,2);

    lora_tx_buffer[52] = '#';
    uart_write_bytes(UART_NUM,lora_tx_buffer,53);
    
    segment_id = (segment_id+1) % 12;

    vTaskDelay(pdMS_TO_TICKS(CELL_VOLTAGE_TRANSMIT_PERIOD_MS));
  }
}