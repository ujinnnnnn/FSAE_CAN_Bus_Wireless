#ifndef CONFIG_H
#define CONFIG_H

#define TRANSMIT_MODE LORA
// #define TRANSMIT_MODE USB


/* 
FreeRTOS Priorities
*/
#define TWAI_RECEIVE_PRIORITY 9

#define LORA_RX_CHECK_PRIORITY 4
#define SENDER_PRIORITY 10
#define LORA_SLOW_1_PRIORITY 8
#define LORA_SLOW_2_PRIORITY 8
#define LORA_FAST_CRITICAL_PRIORITY 9
#define LORA_FAST_INFORMATION_PRIORITY 6
#define LORA_CELL_VOLTAGE_PRIORITY 8
#define LORA_THERMISTOR_PRIORITY 8

#define USB_DEBUG_PRIORITY 5

/*
UART 
*/
#define UART_NUM UART_NUM_1

/*
GPIO
*/

#define AUX_PIN (GPIO_NUM_14)
#define M1_PIN (GPIO_NUM_13)
#define M0_PIN (GPIO_NUM_12)
#define TXD_PIN (GPIO_NUM_4)
#define RXD_PIN (GPIO_NUM_5)

#define CAN_TX_PIN (GPIO_NUM_1)
#define CAN_RX_PIN (GPIO_NUM_2)

#endif // config.h