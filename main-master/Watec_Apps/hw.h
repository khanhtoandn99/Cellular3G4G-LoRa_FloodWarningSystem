/*
 * hw.h
 *
 *  Created on: Jan 23, 2021
 *      Author: ASUS
 */

#ifndef _HW_H_
#define _HW_H_

/*
 * Note : Toàn bộ thư viện lora và sim sẽ được #include tại đây
 */

#ifdef __cplusplus
extern "C" {
#endif

#include "i2c.h"
#include "usart.h"
#include "gpio.h"
#include "tim.h"
#include "iwdg.h"
#include "string.h"
#include "stdio.h"
#include "stm32l4xx_hal.h"

#include "sim7x00.h"
#include "si7021.h"
#include "lrwan.h"
#include "power.h"
#include "debug.h"

#define LED1_Pin GPIO_PIN_2
#define LED1_GPIO_Port GPIOA
#define LED2_Pin GPIO_PIN_4
#define LED2_GPIO_Port GPIOA
#define LED3_Pin GPIO_PIN_5
#define LED3_GPIO_Port GPIOA
#define LED4_Pin GPIO_PIN_6
#define LED4_GPIO_Port GPIOA
#define LED5_Pin GPIO_PIN_7
#define LED5_GPIO_Port GPIOA
#define SIM7600_PWRKEY_Pin GPIO_PIN_0
#define SIM7600_PWRKEY_GPIO_Port GPIOB
#define SIM7600_FLIGHTMODE_Pin GPIO_PIN_1
#define SIM7600_FLIGHTMODE_GPIO_Port GPIOB
#define SIM7600_RST_Pin GPIO_PIN_2
#define SIM7600_RST_GPIO_Port GPIOB
#define PWR_TX_Pin GPIO_PIN_10
#define PWR_TX_GPIO_Port GPIOB
#define PWR_RX_Pin GPIO_PIN_11
#define PWR_RX_GPIO_Port GPIOB
#define OUT1_Pin GPIO_PIN_8
#define OUT1_GPIO_Port GPIOA
#define OUT2_Pin GPIO_PIN_9
#define OUT2_GPIO_Port GPIOA
#define OUT3_Pin GPIO_PIN_10
#define OUT3_GPIO_Port GPIOA
#define OUT4_Pin GPIO_PIN_11
#define OUT4_GPIO_Port GPIOA
#define LRWAN_TX_Pin GPIO_PIN_10
#define LRWAN_TX_GPIO_Port GPIOC
#define LRWAN_RX_Pin GPIO_PIN_11
#define LRWAN_RX_GPIO_Port GPIOC
#define DEBUG_TX_Pin GPIO_PIN_12
#define DEBUG_TX_GPIO_Port GPIOC
#define DEBUG_RX_Pin GPIO_PIN_2
#define DEBUG_RX_GPIO_Port GPIOD
#define SIM7600_TX_Pin GPIO_PIN_6
#define SIM7600_TX_GPIO_Port GPIOB
#define SIM7600_RX_Pin GPIO_PIN_7
#define SIM7600_RX_GPIO_Port GPIOB


void Error_Handler(void);

#ifdef __cplusplus
}
#endif

#endif /* INC_HW_H_ */
