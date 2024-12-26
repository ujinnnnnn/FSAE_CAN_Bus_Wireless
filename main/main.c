#include <stdio.h>
#include <stdlib.h>
#include "string.h"

#include "freertos/freertos.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "freertos/semphr.h"
#include "esp_system.h"
#include "esp_err.h"
#include "esp_log.h"


#include "driver/uart.h"
#include "driver/twai.h"
#include "driver/gpio.h"

#include "config.h"
#include "CAN.h"
#include "checksum.h"


static char TAG[]  = "main.c";

static const twai_filter_config_t f_config = TWAI_FILTER_CONFIG_ACCEPT_ALL();
static const twai_timing_config_t t_config = TWAI_TIMING_CONFIG_250KBITS();
//Set TX queue length to 0 due to listen only mode
static const twai_general_config_t g_config = {.mode = TWAI_MODE_LISTEN_ONLY,
                                               .tx_io = CAN_TX_PIN, .rx_io = CAN_RX_PIN,
                                               .clkout_io = TWAI_IO_UNUSED, .bus_off_io = TWAI_IO_UNUSED,
                                               .tx_queue_len = 0, .rx_queue_len = 512,
                                               .alerts_enabled = TWAI_ALERT_RX_DATA,
                                               .clkout_divider = 0, .intr_flags = ESP_INTR_FLAG_IRAM
                                              };


static const gpio_config_t aux_config = {.intr_type = GPIO_INTR_DISABLE,
                              .mode = GPIO_MODE_INPUT,
                              .pin_bit_mask = (1ULL << AUX_PIN),
                              .pull_down_en = 0,
                              .pull_up_en = 0};


static const gpio_config_t M0M1_config = {.intr_type = GPIO_INTR_DISABLE,
                              .mode = GPIO_MODE_OUTPUT,
                              .pin_bit_mask = (1ULL << M0_PIN) | (1ULL << M1_PIN),
                              .pull_down_en = 0,
                              .pull_up_en = 0};
  

static uart_config_t lora_uart_config = {
      .baud_rate = 9600,
      .data_bits = UART_DATA_8_BITS,
      .parity = UART_PARITY_DISABLE,
      .stop_bits = UART_STOP_BITS_1,
      .flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
      .source_clk = UART_SCLK_DEFAULT};

SemaphoreHandle_t start_sem;

void init(){
  start_sem = xSemaphoreCreateBinary();
  //Install and start TWAI driver
  ESP_ERROR_CHECK(twai_driver_install(&g_config, &t_config, &f_config));
  ESP_LOGI(TAG, "Driver installed");
  ESP_ERROR_CHECK(twai_start());
  ESP_LOGI(TAG, "Driver started");

  gpio_config(&aux_config);

  gpio_config(&M0M1_config);
  
  ESP_ERROR_CHECK(gpio_set_level(M1_PIN,0));
  ESP_ERROR_CHECK(gpio_set_level(M0_PIN,0));


  // We won't use a buffer for sending data.
  uart_driver_install(UART_NUM_1, 2048, 0, 0, NULL, 0);
  uart_param_config(UART_NUM_1, &lora_uart_config);
  uart_set_pin(UART_NUM_1, TXD_PIN, RXD_PIN, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE);

  xSemaphoreGive(start_sem);                     //Start tasks
}


can_DTI_HV_500_t Inv_data = {0};

can_OrionBMS2_t BMS_data = {0};

int8_t Thermistor_data[TEM_MAX_THERMISTORS] = {0};

can_CellData_t Cell_data[120] = {0};


//uint16_t		crc_16(             const unsigned char *input_str, size_t num_bytes       );

void pack_and_send_message_1(uint8_t * lora_tx_buffer){
  uint8_t message_id = 1;

  memcpy((void *) &lora_tx_buffer[1],(void *) &message_id,1);
  memcpy((void*) &lora_tx_buffer[DATA_OFFSET_BITS + 0], (void *) &Inv_data.inverter_temp,4);
  memcpy((void*) &lora_tx_buffer[DATA_OFFSET_BITS + 4], (void *) &Inv_data.motor_temp,4);
  memcpy((void*) &lora_tx_buffer[DATA_OFFSET_BITS + 8], (void *) &BMS_data.MaxCellVoltage,4);
  memcpy((void*) &lora_tx_buffer[DATA_OFFSET_BITS + 12], (void *) &Inv_data.input_voltage,2);
  memcpy((void*) &lora_tx_buffer[DATA_OFFSET_BITS + 14], (void *) &BMS_data.PackAbsCurrent,2);
  memcpy((void*) &lora_tx_buffer[DATA_OFFSET_BITS + 16], (void *) &BMS_data.AverageCurrent,2);
  memcpy((void*) &lora_tx_buffer[DATA_OFFSET_BITS + 18], (void *) &BMS_data.MaxPackVoltage,2);
  memcpy((void*) &lora_tx_buffer[DATA_OFFSET_BITS + 20], (void *) &Inv_data.FAULT_CODE,1);
  memcpy((void*) &lora_tx_buffer[DATA_OFFSET_BITS + 21], (void *) &BMS_data.PackSOC,1);
  memcpy((void*) &lora_tx_buffer[DATA_OFFSET_BITS + 22], (void *) &BMS_data.HighTemperature,1);
  memcpy((void*) &lora_tx_buffer[DATA_OFFSET_BITS + 23], (void *) &BMS_data.InternalTemperature,1);

#define MESSAGE_1_DATA_BYTES 24

  uint16_t crc = crc_16((unsigned char *) &lora_tx_buffer[DATA_OFFSET_BITS],MESSAGE_1_DATA_BYTES);

  memcpy((void*) &lora_tx_buffer[DATA_OFFSET_BITS + MESSAGE_1_DATA_BYTES],(void *) &crc,2);

  lora_tx_buffer[DATA_OFFSET_BITS + MESSAGE_1_DATA_BYTES + CRC_LENGTH_BYTES - 1] = '#';

  uart_write_bytes(UART_NUM_1,lora_tx_buffer,DATA_OFFSET_BITS + DATA_OFFSET_BITS + MESSAGE_1_DATA_BYTES + CRC_LENGTH_BYTES);
}

void pack_and_send_message_2(uint8_t * lora_tx_buffer){
  uint8_t message_id = 2;

  memcpy((void *) &lora_tx_buffer[1],(void *) &message_id,1);
  memcpy((void*) &lora_tx_buffer[DATA_OFFSET_BITS + 0], (void *) &Inv_data.inverter_temp,4);
  memcpy((void*) &lora_tx_buffer[DATA_OFFSET_BITS + 4], (void *) &Inv_data.motor_temp,4);
  memcpy((void*) &lora_tx_buffer[DATA_OFFSET_BITS + 8], (void *) &BMS_data.MaxCellVoltage,4);
  memcpy((void*) &lora_tx_buffer[DATA_OFFSET_BITS + 12], (void *) &Inv_data.input_voltage,2);
  memcpy((void*) &lora_tx_buffer[DATA_OFFSET_BITS + 14], (void *) &BMS_data.PackAbsCurrent,2);
  memcpy((void*) &lora_tx_buffer[DATA_OFFSET_BITS + 16], (void *) &BMS_data.AverageCurrent,2);
  memcpy((void*) &lora_tx_buffer[DATA_OFFSET_BITS + 18], (void *) &BMS_data.MaxPackVoltage,2);
  memcpy((void*) &lora_tx_buffer[DATA_OFFSET_BITS + 20], (void *) &Inv_data.FAULT_CODE,1);
  memcpy((void*) &lora_tx_buffer[DATA_OFFSET_BITS + 21], (void *) &BMS_data.PackSOC,1);
  memcpy((void*) &lora_tx_buffer[DATA_OFFSET_BITS + 22], (void *) &BMS_data.HighTemperature,1);
  memcpy((void*) &lora_tx_buffer[DATA_OFFSET_BITS + 23], (void *) &BMS_data.InternalTemperature,1);

#define MESSAGE_2_DATA_BYTES 24

  uint16_t crc = crc_16((unsigned char *) &lora_tx_buffer[DATA_OFFSET_BITS],MESSAGE_2_DATA_BYTES);

  memcpy((void*) &lora_tx_buffer[DATA_OFFSET_BITS + MESSAGE_2_DATA_BYTES],(void *) &crc,2);

  lora_tx_buffer[DATA_OFFSET_BITS + MESSAGE_2_DATA_BYTES + CRC_LENGTH_BYTES - 1] = '#';

  uart_write_bytes(UART_NUM_1,lora_tx_buffer,DATA_OFFSET_BITS + DATA_OFFSET_BITS + MESSAGE_2_DATA_BYTES + CRC_LENGTH_BYTES);
}

static void lora_transmit_task(void *arg){
  uint8_t lora_tx_buffer[2 * LORA_PACKET_SIZE_BYTES ] = {0} ;
  lora_tx_buffer[0] = '$';


  Inv_data.inverter_temp = 34;
  Inv_data.motor_temp = 23;
  BMS_data.MaxCellVoltage = 3.45f;
  Inv_data.input_voltage = 12;
  BMS_data.PackAbsCurrent = 78;
  BMS_data.AverageCurrent = 24;
  BMS_data.MaxPackVoltage = 478;
  Inv_data.FAULT_CODE = 0xAA;
  BMS_data.PackSOC = 56;
  BMS_data.HighTemperature = 0;
  BMS_data.InternalTemperature = 28;



  while(1){
    vTaskDelay(pdMS_TO_TICKS(LORA_TRANSMIT_PERIOD_MS));
    pack_message_1(lora_tx_buffer);
    uart_write_bytes(UART_NUM_1,lora_tx_buffer,DATA_OFFSET_BITS + 25);

    vTaskDelay(pdMS_TO_TICKS(LORA_TRANSMIT_PERIOD_MS));
    //pack_message_2(lora_tx_buffer);
    uart_write_bytes(UART_NUM_1,lora_tx_buffer,DATA_OFFSET_BITS + 25);
  }
}

static void usb_debug_task(void *arg){

  while(1){
   vTaskDelay(pdMS_TO_TICKS(LORA_TRANSMIT_PERIOD_MS));
    
    ESP_LOGI(TAG,"\nInverter Data:\nerpm:\t%ld\nduty cycle:\t%f\nInput Voltage:\t%hd\nac current:\t%f\ndc current:\t%f\nInverter Temp:\t%f\nmotor temp:\t%f\nfault code:\t%x\n",
    Inv_data.erpm,Inv_data.duty_cycle,Inv_data.input_voltage,Inv_data.ac_current,Inv_data.dc_current,Inv_data.inverter_temp,Inv_data.motor_temp,Inv_data.FAULT_CODE);

    vTaskDelay(pdMS_TO_TICKS(LORA_TRANSMIT_PERIOD_MS));

    ESP_LOGI(TAG,"\nBMS Data:\nMaxCellVoltage:\t%f\nMaxPackVoltage:\t%hu\nPackSOC:\t%hhu\nInternalTemperature:\t%hhu\n",
    BMS_data.MaxCellVoltage,BMS_data.MaxPackVoltage,BMS_data.PackSOC,BMS_data.InternalTemperature);

    vTaskDelay(pdMS_TO_TICKS(LORA_TRANSMIT_PERIOD_MS));
    ESP_LOGI(TAG,"\nCell Data\n");
    for (int i = 0;i < 10;i++){
      ESP_LOGI(TAG,"%f %f %f %f %f %f %f %f %f %f %f %f",
      Cell_data[i*12 + 0].OpenVoltage,Cell_data[i*12 + 1].OpenVoltage,Cell_data[i*12 + 2].OpenVoltage,Cell_data[i*12 + 3].OpenVoltage,Cell_data[i*12 + 4].OpenVoltage,Cell_data[i*12 + 5].OpenVoltage,
      Cell_data[i*12 + 6].OpenVoltage,Cell_data[i*12 + 7].OpenVoltage,Cell_data[i*12 + 8].OpenVoltage,Cell_data[i*12 + 9].OpenVoltage,Cell_data[i*12 + 10].OpenVoltage,Cell_data[i*12 + 11].OpenVoltage);
      }
    }
}



void app_main(void)
{
  init();

  xTaskCreatePinnedToCore(twai_receive_task, "TWAI_rx", 4096, NULL, 9, NULL, 1);


  //xTaskCreatePinnedToCore(usb_debug_task, "usb debug",4096,NULL, 9,NULL,0);
  xTaskCreatePinnedToCore(lora_transmit_task, "lora transmit",4096,NULL, 9,NULL,0);

  vTaskDelete(NULL);
}
