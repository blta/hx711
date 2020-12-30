/*
#include <packages/hx711-latest/sensor_avia_hx711.h>
 * Copyright (c) 2006-2018, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author          Notes
 * 2019-08-01     LuoGong         the first version.
 * 2019-08-15     MurphyZhao      add lock and modify code style
 *
 */
#include <rtdevice.h>
#include <rthw.h>
#include "sensor.h"
#include "board.h"
#include <stdint.h>
#include "sensor_avia_hx711.h"

#define SENSOR_DEBUG
#define DBG_TAG               "sensor.hx711"

#ifdef SENSOR_DEBUG
#define DBG_LVL               DBG_LOG
#else
#define DBG_LVL               DBG_ERROR
#endif /* SENSOR_DEBUG */
#include <rtdbg.h>

#define SENSOR_TEMP_RANGE_MAX (100)
#define SENSOR_TEMP_RANGE_MIN (0)


#ifndef RT_USING_PIN
#error "Please enable RT_USING_PIN"
#endif

#ifndef RT_SENSOR_VENDOR_DALLAS
#define RT_SENSOR_VENDOR_DALLAS (7u)
#endif

#ifndef rt_hw_us_delay
RT_WEAK void rt_hw_us_delay(rt_uint32_t us)
{
    rt_uint32_t delta;

    us = us * (SysTick->LOAD / (1000000 / RT_TICK_PER_SECOND));
    delta = SysTick->VAL;

    while (delta - SysTick->VAL < us) continue;
}
#endif

static void hx711_reset(hx711_device_t pins)
{

    rt_pin_mode(pins->D_OUT, PIN_MODE_INPUT);
    rt_pin_mode(pins->PD_SCK, PIN_MODE_OUTPUT);
    /* disable the sensor */
    rt_pin_write(pins->PD_SCK, PIN_HIGH);
    rt_hw_us_delay(100);
    //    /* at enable status , D_OUT should be high */
    //    if(!rt_pin_read(pins->D_OUT))
    //        return CONNECT_SUCCESS;
    rt_pin_write(pins->PD_SCK, PIN_LOW);
    rt_hw_us_delay(2);
}

static uint8_t hx711_check(hx711_device_t pins)
{
    uint16_t retry = 0;
    while (rt_pin_read(pins->D_OUT) && retry < 500)
    {
        retry++;
        rt_hw_us_delay(1);
    }

    if(retry >= 500)
    {
        return CONNECT_FAILED;
    }

    return CONNECT_SUCCESS;
}


static uint32_t hx711_read_once(hx711_device_t pins)
{
    uint8_t i ;
    uint32_t dat = 0;
    for (i = 1; i <= 24; i++)
    {
        rt_pin_write(pins->PD_SCK, PIN_HIGH);
        rt_hw_us_delay(1);
        dat <<= 1;
        rt_pin_write(pins->PD_SCK, PIN_LOW);
        if(rt_pin_read(pins->D_OUT) == 1)
            dat++;

    }
    rt_pin_write(pins->PD_SCK, PIN_HIGH);
    rt_hw_us_delay(2);
    /* convert the data at 25th rising edge */
    dat=dat^0x800000;
    rt_pin_write(pins->PD_SCK, PIN_LOW);
    return dat;
}

static uint32_t hx711_read_Data(hx711_device_t pins)
{
	hx711_reset(pins);

	if(hx711_check(pins) == 0)
	{
	    return hx711_read_once(pins);
	}
    else
    {
        return 0;
    }


}

uint8_t hx711_init(hx711_device_t pins)
{
    uint8_t ret = 0;

    hx711_reset(pins);
    ret = hx711_check(pins);
    if (ret != 0)
    {
        hx711_reset(pins);
        ret = hx711_check(pins);
    }

    return ret;
}


static rt_size_t hx711_polling_get_data(rt_sensor_t sensor, struct rt_sensor_data *data)
{
    rt_int32_t weight;
    weight = hx711_read_Data((hx711_device_t)sensor->config.intf.user_data);
    data->data.temp = weight;
    data->timestamp = rt_sensor_get_ts();
    return 1;
}

static rt_size_t hx711_fetch_data(struct rt_sensor_device *sensor, void *buf, rt_size_t len)
{
    RT_ASSERT(buf);

    if (sensor->config.mode == RT_SENSOR_MODE_POLLING)
    {
        return hx711_polling_get_data(sensor, buf);
    }

    return 0;
}

static rt_err_t hx711_control(struct rt_sensor_device *sensor, int cmd, void *args)
{
    return RT_EOK;
}

static struct rt_sensor_ops sensor_ops =
{
    hx711_fetch_data,
    hx711_control
};

static struct rt_sensor_device hx711_dev;
int rt_hw_hx711_init(const char *name, struct rt_sensor_config *cfg)
{
    rt_err_t result = RT_EOK;
    rt_sensor_t sensor = &hx711_dev;

    rt_memset(sensor, 0x0, sizeof(struct rt_sensor_device));

    if (!hx711_init((hx711_device_t)cfg->intf.user_data))
    {
        sensor->module = rt_calloc(1, sizeof(struct rt_sensor_module));
        if (sensor->module == RT_NULL)
        {
            LOG_E("Memory error.");
            result = -RT_ENOMEM;
            goto __exit;
        }

        sensor->info.type       = RT_SENSOR_CLASS_NONE ;
        sensor->info.vendor     = RT_SENSOR_VENDOR_DALLAS;
        sensor->info.model      = "HX711";
        sensor->info.unit       = RT_SENSOR_UNIT_DCELSIUS;
        sensor->info.intf_type  = RT_SENSOR_INTF_ONEWIRE;
        sensor->info.range_max  = SENSOR_TEMP_RANGE_MAX;
        sensor->info.range_min  = SENSOR_TEMP_RANGE_MIN;
        sensor->info.period_min = 100; /* Read ten times in 1 second */

        sensor->config = *cfg;
        sensor->ops = &sensor_ops;

        /* hx711 sensor register */
        result = rt_hw_sensor_register(sensor, name, RT_DEVICE_FLAG_RDONLY, RT_NULL);
        if (result != RT_EOK)
        {
            LOG_E("device register err code: %d", result);
            goto __exit;
        }
    }
    else
    {
        LOG_E("hx711 init failed");
        result = -RT_ERROR;
        goto __exit;
    }
    return RT_EOK;

__exit:
    if (sensor->module)
        rt_free(sensor->module);
    return result;
}
