#define SLOW_1_ID 0x80
#define SLOW_2_ID 0x81

#define FAST_CRITICAL_ID 0x90
#define FAST_INFORMATION_ID 0x91

#define CELL_VOLTAGE_SEG_1_ID(ID) (0xF0 | ID)  


#define LORA_RX_CHECK_PERIOD_MS 2000
#define SLOW_1_TRANSMIT_PERIOD_MS 2000
#define SLOW_2_TRANSMIT_PERIOD_MS 2000
#define FAST_CRITICAL_TRANSMIT_PERIOD_MS 2000
#define FAST_INFORMATION_TRANSMIT_PERIOD_MS 2000
#define CELL_VOLTAGE_TRANSMIT_PERIOD_MS 2000
#define THERMISTOR_TRANSMIT_PERIOD_MS 2000


#define LORA_RX_BUFFER_SIZE 20

#define START_BYTES 1
#define START_BYTE '$'
#define END_BYTES 1
#define END_BYTE '#'
#define CRC_BYTES 2
#define ID_BYTES 1


typedef struct {
  uint8_t id;
  uint8_t *data;
  size_t length;
} LoRa_message_t;
/*
void _transmit_task(void *arg)
{
#define DATA_BYTES 
  LoRa_message_t message = {.id = ,.length = DATA_BYTES + CRC_BYTES};

  while (uxSemaphoreGetCount(transmit_sem) == 1)
  {
    message.data = malloc(DATA_BYTES + CRC_BYTES);
    memcpy((void *)&message.data[0], (void *)&Inv_data.inverter_temp, 2);

    uint16_t crc = crc_16((unsigned char *) message.data,DATA_BYTES);
    memcpy((void*) &message.data[DATA_BYTES],(void *) &crc,CRC_BYTES);

    xQueueSend(LoRa_TX_Queue,&message,portMAX_DELAY);

    vTaskDelay(pdMS_TO_TICKS());
#undef DATA_BYTES
  }
}
*/
void LoRa_rx_check(void * arg);

void LoRa_sender_task(void* arg);

void slow_1_transmit_task(void *arg);
void slow_2_transmit_task(void *arg);
void fast_critical_transmit_task(void *arg);
void fast_information_transmit_task(void *arg);
void cell_voltage_transmit_task(void *arg);
void thermistor_transmit_task(void *arg);