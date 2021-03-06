/*
 * Copyright (c) 2020-2022
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author          Notes
 * 2020-12-30     bltas         the first version.
 *
 */

#ifndef __HX711_H__
#define __HX711_H__

#include <rtthread.h>
#include "sensor.h"

#define CONNECT_SUCCESS  0
#define CONNECT_FAILED   1

struct hx711_device
{
    rt_base_t PD_SCK;
    rt_base_t D_OUT;
    rt_mutex_t lock;
};
typedef struct hx711_device *hx711_device_t;

//uint8_t hx711_init(rt_base_t pin);
//int32_t hx711_get_weight(rt_base_t pin);
int rt_hw_hx711_init(const char *name, struct rt_sensor_config *cfg);

#endif /* __DS18B20_H_ */


