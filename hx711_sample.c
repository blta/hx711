/*
 * Copyright (c) 2006-2018, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author         Notes
 * 2019-08-01     LuoGong         the first version.
 * 2019-08-15     MurphyZhao      add lock and modify code style
 */

#include <rtthread.h>
#include <rtdevice.h>
#include "board.h"
#include "sensor.h"
#include "sensor_avia_hx711.h"

/* Modify this pin according to the actual wiring situation */
#define HX711_PD_SCK_PIN    GET_PIN(B, 0)
#define HX711_D_OUT_PIN     GET_PIN(B, 1)
struct hx711_device hx711_instance =
{
    .D_OUT  = (rt_base_t)HX711_D_OUT_PIN,
    .PD_SCK = (rt_base_t)HX711_PD_SCK_PIN,
};

static void read_temp_entry(void *parameter)
{
    rt_device_t dev = RT_NULL;
    struct rt_sensor_data sensor_data;
    rt_size_t res;
    rt_uint8_t get_data_freq = 1; /* 1Hz */

    dev = rt_device_find("temp_hx7");
    if (dev == RT_NULL)
    {
        return;
    }

    if (rt_device_open(dev, RT_DEVICE_FLAG_RDWR) != RT_EOK)
    {
        rt_kprintf("open device failed!\n");
        return;
    }

    rt_device_control(dev, RT_SENSOR_CTRL_SET_ODR, (void *)(&get_data_freq));

    while (1)
    {
        res = rt_device_read(dev, 0, &sensor_data, 1);

        if (res != 1)
        {
            rt_kprintf("read data failed! result is %d\n", res);
            rt_device_close(dev);
            return;
        }
        else
        {
            if (sensor_data.data.temp >= 0)
            {
//                uint8_t temp = (sensor_data.data.temp & 0xffff) >> 0;      // get temp
//                uint8_t humi = (sensor_data.data.temp & 0xffff0000) >> 16; // get humi
                rt_kprintf("weight:%d\n" ,sensor_data.data.temp);
            }
        }

        rt_thread_delay(1000);
    }
}

static int hx711_read_temp_sample(void)
{
    rt_thread_t hx711_thread;

    hx711_thread = rt_thread_create("dht_tem",
                                     read_temp_entry,
                                     RT_NULL,
                                     1024,
                                     RT_THREAD_PRIORITY_MAX / 2,
                                     20);
    if (hx711_thread != RT_NULL)
    {
        rt_thread_startup(hx711_thread);
    }

    return RT_EOK;
}
INIT_APP_EXPORT(hx711_read_temp_sample);

static int rt_hw_hx711_port(void)
{
    struct rt_sensor_config cfg;
    

    cfg.intf.user_data = (hx711_device_t)&hx711_instance;
    rt_hw_hx711_init("hx711", &cfg);

    return RT_EOK;
}
INIT_COMPONENT_EXPORT(rt_hw_hx711_port);
//MSH_CMD_EXPORT(rt_hw_hx711_port, register hx711 device);
