#define SLOW_1_ID 0x12
#define SLOW_2_ID 0x34

#define FAST_CRITICAL_ID 0xCC
#define FAST_INFORMATION_ID 0xAA

#define CELL_VOLTAGE_SEG_1_ID(ID) (0xF0 | ID)  


#define LORA_RX_CHECK_PERIOD_MS 2000
#define SLOW_1_TRANSMIT_PERIOD_MS 1000
#define SLOW_2_TRANSMIT_PERIOD_MS 1000
#define SLOW_2_TRANSMIT_PERIOD_MS 1000
#define FAST_CRITICAL_TRANSMIT_PERIOD_MS 100
#define FAST_INFORMATION_TRANSMIT_PERIOD_MS 500
#define CELL_VOLTAGE_TRANSMIT_PERIOD_MS 500
#define THERMISTOR_TRANSMIT_PERIOD_MS 500


#define LORA_RX_BUFFER_SIZE 20



void LoRa_rx_check(void * arg);
void slow_1_transmit_task(void *arg);
void slow_2_transmit_task(void *arg);
void fast_critical_transmit_task(void *arg);
void fast_information_transmit_task(void *arg);
void cell_voltage_transmit_task(void *arg);
