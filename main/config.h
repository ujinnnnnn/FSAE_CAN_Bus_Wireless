#ifndef CONFIG_H
#define CONFIG_H


#define LORA_TRANSMIT_PERIOD_MS 2000
#define LORA_PACKET_SIZE_BYTES 58
#define AUX_PIN (GPIO_NUM_14)
#define M1_PIN (GPIO_NUM_13)
#define M0_PIN (GPIO_NUM_12)
#define TXD_PIN (GPIO_NUM_4)
#define RXD_PIN (GPIO_NUM_5)

#define CAN_TX_PIN (GPIO_NUM_1)
#define CAN_RX_PIN (GPIO_NUM_2)


#define DATA_OFFSET_BITS 3
#define CRC_LENGTH_BYTES 2
#endif // config.h