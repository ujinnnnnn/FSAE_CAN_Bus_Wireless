#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "freertos/freertos.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "freertos/semphr.h"
#include "esp_system.h"
#include "esp_err.h"
#include "esp_log.h"

#include "esp_intr_alloc.h"

#include "driver/uart.h"
#include "driver/twai.h"
#include "driver/gpio.h"

#include "config.h"
#include "LoRa.h"
#include "CAN.h"


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

SemaphoreHandle_t setup_sem;

SemaphoreHandle_t transmit_sem;

extern SemaphoreHandle_t lora_uart_mutex;
void init(){
  transmit_sem = xSemaphoreCreateBinary();
  lora_uart_mutex = xSemaphoreCreateMutex();
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
  uart_driver_install(UART_NUM, 2048, 0, 0, NULL, 0);
  uart_param_config(UART_NUM, &lora_uart_config);
  uart_set_pin(UART_NUM, TXD_PIN, RXD_PIN, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE);

  xSemaphoreGive(transmit_sem);                     //Start tasks
}


can_DTI_HV_500_t Inv_data = {0};

can_OrionBMS2_t BMS_data = {0};

can_PLEX_t Plex_data = {0};

int8_t Thermistor_data[TEM_MAX_THERMISTORS] = {0};

can_CellData_t Cell_data[120] = {0};


//uint16_t		crc_16(             const unsigned char *input_str, size_t num_bytes       );

static void usb_debug_task(void *arg){
  while (uxSemaphoreGetCount(setup_sem) == 0);

  while(1){
   vTaskDelay(pdMS_TO_TICKS(2000));
   /*
    ESP_LOGI(TAG,"\nInverter Data:\nerpm:\t%ld\nduty cycle:\t%f\nInput Voltage:\t%hd\nac current:\t%f\ndc current:\t%f\nInverter Temp:\t%f\nmotor temp:\t%f\nfault code:\t%x\n",
    Inv_data.erpm,Inv_data.duty_cycle,Inv_data.input_voltage,Inv_data.ac_current,Inv_data.dc_current,Inv_data.inverter_temp,Inv_data.motor_temp,Inv_data.FAULT_CODE);

    vTaskDelay(pdMS_TO_TICKS(2000));

    ESP_LOGI(TAG,"\nBMS Data:\nMaxCellVoltage:\t%f\nMaxPackVoltage:\t%hu\nPackSOC:\t%hhu\nInternalTemperature:\t%hhu\n",
    BMS_data.MaxCellVoltage,BMS_data.MaxPackVoltage,BMS_data.PackSOC,BMS_data.InternalTemperature);

    vTaskDelay(pdMS_TO_TICKS(2000));
    ESP_LOGI(TAG,"\nCell Data\n");
    for (int i = 0;i < 10;i++){
      ESP_LOGI(TAG,"%f %f %f %f %f %f %f %f %f %f %f %f",
      Cell_data[i*12 + 0].OpenVoltage,Cell_data[i*12 + 1].OpenVoltage,Cell_data[i*12 + 2].OpenVoltage,Cell_data[i*12 + 3].OpenVoltage,Cell_data[i*12 + 4].OpenVoltage,Cell_data[i*12 + 5].OpenVoltage,
      Cell_data[i*12 + 6].OpenVoltage,Cell_data[i*12 + 7].OpenVoltage,Cell_data[i*12 + 8].OpenVoltage,Cell_data[i*12 + 9].OpenVoltage,Cell_data[i*12 + 10].OpenVoltage,Cell_data[i*12 + 11].OpenVoltage);
      }
   */ 
    }
}



void app_main(void)
{
  init();
  // xTaskCreatePinnedToCore(LoRa_rx_check, "LoRa RX", 4096, NULL, LORA_RX_CHECK_PRIORITY, NULL, tskNO_AFFINITY);

  xTaskCreatePinnedToCore(twai_receive_task, "TWAI_rx", 4096, NULL, TWAI_RECEIVE_PRIORITY, NULL, 0);
  xTaskCreatePinnedToCore(slow_1_transmit_task, "slow 1 tx", 4096, NULL, LORA_SLOW_1_PRIORITY, NULL, 1);
  xTaskCreatePinnedToCore(slow_2_transmit_task, "slow 2 tx", 4096, NULL, LORA_SLOW_2_PRIORITY, NULL, 1);

  xTaskCreatePinnedToCore(fast_information_transmit_task, "fast info tx", 4096, NULL, LORA_FAST_INFORMATION_PRIORITY, NULL, 1);
  xTaskCreatePinnedToCore(fast_critical_transmit_task, "fast crit tx", 4096, NULL, LORA_FAST_CRITICAL_PRIORITY, NULL, 1);

  xTaskCreatePinnedToCore(cell_voltage_transmit_task, "cell voltage tx", 4096, NULL, LORA_CELL_VOLTAGE_PRIORITY, NULL, 1);
  xTaskCreatePinnedToCore(thermistor_transmit_task, "thermisotr tx", 4096, NULL, LORA_THERMISTOR_PRIORITY, NULL, 1);

  //xTaskCreatePinnedToCore(usb_debug_task, "usb debug",4096,NULL, 9,NULL,0);

  vTaskDelete(NULL);
}
