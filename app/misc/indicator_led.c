#include "misc/indicator_led.h"

#include "platform.h"
#include "c_string.h"
#include "c_stdlib.h"
#include "c_stdio.h"

#include "gpio.h"
#include "osapi.h"

#define INDICATOR_ON 0
#define INDICATOR_OFF NORMAL_PWM_DEPTH

#define INDICATOR_MODE_OFF 0
#define INDICATOR_MODE_ON 1
#define INDICATOR_MODE_PULSATE 2
#define INDICATOR_MODE_FLASH 3
#define INDICATOR_MODE_TOGGLE 4

#define INDICATOR_SPEED_FAST 2
#define INDICATOR_SPEED_SLOW 1

#define INDICATOR_TIMER_FREQ 30

static unsigned _indicator_pin = 4;
static uint32_t _indicator_freq = 500;
static unsigned _mode = 0;
static unsigned _last_duty = INDICATOR_OFF;
static uint32_t _counter = 0;
static unsigned _speed = INDICATOR_SPEED_SLOW;

static bool _started = false;

os_timer_t _timer = {0, 0, 0, 0, 0};

void ICACHE_FLASH_ATTR
_indicator_timer_func(void)
{
    int new_duty;

    _counter += (1000 / INDICATOR_TIMER_FREQ * _speed) % 1000;

    switch (_mode)
    {
    case INDICATOR_MODE_PULSATE:
        new_duty = (_counter < 500 ? _counter : 1000 - _counter) * INDICATOR_OFF / 1000;
        break;
    case INDICATOR_MODE_TOGGLE:
        new_duty = _counter < 500 ? INDICATOR_OFF : INDICATOR_ON;
        break;
    case INDICATOR_MODE_FLASH:
        new_duty = _counter < 1 ? INDICATOR_OFF : INDICATOR_ON;
        break;
    case INDICATOR_MODE_ON:
        new_duty = INDICATOR_ON;
        break;
    case INDICATOR_MODE_OFF:
    default:
        new_duty = INDICATOR_OFF;
        break;
    }

    if (_last_duty != new_duty)
    {
        platform_pwm_set_duty(_indicator_pin, new_duty);
        _last_duty = new_duty;
    }
}

static void _inidicator_set(int mode, int speed)
{
    if (!_started)
    {
        //start
        platform_pwm_setup(_indicator_pin, _indicator_freq, INDICATOR_OFF);
        platform_pwm_start(_indicator_pin);

        os_timer_disarm(&_timer);
        os_timer_setfn(&_timer, (os_timer_func_t *)_indicator_timer_func, NULL);
        os_timer_arm(&_timer, 1000 / INDICATOR_TIMER_FREQ, true);
    }

    _mode = mode;
}

void indicator_on()
{
    _inidicator_set(INDICATOR_ON, 0);
}

void indicator_off()
{
    _inidicator_set(INDICATOR_OFF, 0);
}

void indicator_flash_slow()
{
    _inidicator_set(INDICATOR_MODE_FLASH, INDICATOR_SPEED_SLOW);
}

void indicator_flash_fast()
{
    _inidicator_set(INDICATOR_MODE_FLASH, INDICATOR_SPEED_FAST);
}

void indicator_toggle_slow()
{
    _inidicator_set(INDICATOR_MODE_TOGGLE, INDICATOR_SPEED_SLOW);
}

void indicator_toggle_fast()
{
    _inidicator_set(INDICATOR_MODE_TOGGLE, INDICATOR_SPEED_FAST);
}

void indicator_pulsate_slow()
{
    _inidicator_set(INDICATOR_MODE_PULSATE, INDICATOR_SPEED_SLOW);
}

void indicator_pulsate_fast()
{
    _inidicator_set(INDICATOR_MODE_PULSATE, INDICATOR_SPEED_FAST);
}