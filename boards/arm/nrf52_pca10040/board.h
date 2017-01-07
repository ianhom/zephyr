/*
 * Copyright (c) 2016 Nordic Semiconductor ASA.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef __INC_BOARD_H
#define __INC_BOARD_H

#include <soc.h>

/* Push button switch 0 */
#define SW0_GPIO_PIN	13
#define SW0_GPIO_NAME	CONFIG_GPIO_NRF5_P0_DEV_NAME

/* Push button switch 1 */
#define SW1_GPIO_PIN	14
#define SW1_GPIO_NAME	CONFIG_GPIO_NRF5_P0_DEV_NAME

/* Push button switch 2 */
#define SW2_GPIO_PIN	15
#define SW2_GPIO_NAME	CONFIG_GPIO_NRF5_P0_DEV_NAME

/* Push button switch 3 */
#define SW3_GPIO_PIN	16
#define SW3_GPIO_NAME	CONFIG_GPIO_NRF5_P0_DEV_NAME

/* Onboard GREEN LED 0 */
#define LED0_GPIO_PIN   17
#define LED0_GPIO_PORT  CONFIG_GPIO_NRF5_P0_DEV_NAME

/* Onboard GREEN LED 1 */
#define LED1_GPIO_PIN   18
#define LED1_GPIO_PORT  CONFIG_GPIO_NRF5_P0_DEV_NAME

/* Onboard GREEN LED 2 */
#define LED2_GPIO_PIN   19
#define LED2_GPIO_PORT  CONFIG_GPIO_NRF5_P0_DEV_NAME

/* Onboard GREEN LED 3 */
#define LED3_GPIO_PIN   20
#define LED3_GPIO_PORT  CONFIG_GPIO_NRF5_P0_DEV_NAME

#endif /* __INC_BOARD_H */
