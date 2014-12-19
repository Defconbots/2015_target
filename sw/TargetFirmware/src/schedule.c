#include "global.h"
#include "schedule.h"

//::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
//                            __                        __
//                           / /   ____   _____ ____ _ / /
//                          / /   / __ \ / ___// __ `// /
//                         / /___/ /_/ // /__ / /_/ // /
//                        /_____/\____/ \___/ \__,_//_/
//
//::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
volatile uint32_t g_now = 0;

typedef struct
{
    Fn func;
    uint32_t run_time;
} Event;

typedef struct
{
    Fn func;
    uint8_t    enabled;
    uint32_t   run_time;
    uint32_t   next_run_time;
} PeriodicEvent;

static uint8_t _count;
static PeriodicEvent _pstore[MAX_PER_EVENT_CNT];
static uint32_t _map;

static Event _sstore[MAX_SING_EVENT_CNT];

void ScheduleService(void);
static void PeriodicService(uint32_t current_time);
void SingleEventService(uint32_t current_time);
static uint8_t get_map_size(void);

//::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
extern void ScheduleInit(void)
{
    SystemCoreClockUpdate();
    SysTick_Config(SystemCoreClock/1000);
}

// Hook the systick handler into the scheduler
void SysTick_Handler(void)
{
    ScheduleService();
}

//::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
inline void ScheduleService(void)
{
    g_now++;
#if MAX_PER_EVENT_CNT > 0
    PeriodicService(g_now);
#endif
#if MAX_SING_EVENT_CNT > 0
    SingleEventService(g_now);
#endif
}

//::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
int8_t SchedulePeriodicEvent(Fn func, uint32_t run_time)
{
    if (_count < sizeof(_pstore)/sizeof(Fn))
    {
        // default to disabled
        _pstore[_count].enabled       = false;
        _pstore[_count].func          = func;
        _pstore[_count].run_time      = run_time - 1;
        _pstore[_count].next_run_time = g_now + run_time;
        _count++;
        return (SUCCESS);
    }
    return (FAILURE);
}

//::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
void PeriodicService(uint32_t current_time)
{
    uint8_t i = 0;
    uint8_t remaining = _count;
    if (remaining == 0)
    {
        goto service_complete;
    }
    for (i = 0;i < _count;i++)
    {
        if (_pstore[i].enabled == true &&
            current_time == _pstore[i].next_run_time)
        {
            _pstore[i].next_run_time = current_time + _pstore[i].run_time;
            _pstore[i].func();
            if (--remaining == 0)
            {
                goto service_complete;
            }
        }
    }
service_complete:
    return;
}

//::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
void SchedulePeriodicEnable(Fn func, uint8_t mode)
{
    uint8_t i = 0;
    for (i = 0;i < _count;i++)
    {
        if (func == _pstore[i].func)
        {
            _pstore[i].enabled = mode;
            if (mode)
            {
                _pstore[i].next_run_time = g_now + _pstore[i].run_time;
            }
            break;
        }
    }
}

//::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
int8_t ScheduleSingleEvent(Fn func, uint32_t run_time)
{
    // event queue full?
    if (get_map_size() != MAX_SING_EVENT_CNT)
    {
        uint8_t i = 0;
        for (i = 0;i < MAX_SING_EVENT_CNT;i++)
        {
            // find the first open slot
            if (!(_map & _BV(i)))
            {
                // claim the slot
                _map |= _BV(i);
                // save our data
                _sstore[i].func = func;
                _sstore[i].run_time = g_now + run_time;
                // return success
                return (SUCCESS);
            }
        }
    }
    return (FAILURE);
}

//::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
void ScheduleSingleEventCancel(Fn func)
{
    uint8_t i = 0;
    for (i = 0;i < MAX_SING_EVENT_CNT;i++)
    {
        // find the slot
        if (_sstore[i].func == func)
        {
            // clear
            _map &= ~_BV(i);
            break;
        }
    }
}

//::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
uint8_t get_map_size(void)
{
    uint8_t sum = 0;
    uint16_t map = _map;
    for (sum = 0; map; sum++)
    {
        map &= map - 1;
    }
    return sum;
}

//::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
void SingleEventService(uint32_t current_time)
{
    uint8_t i = 0;
    // get the number of callouts
    uint8_t remaining = get_map_size();
    // if there are none to service
    if (!remaining)
    {
        goto service_complete;
    }
    for (i = 0;i < MAX_SING_EVENT_CNT;i++)
    {
        // find occupied slots and see if the function there is ready
        if ((_map & _BV(i)) && current_time == _sstore[i].run_time)
        {
            // run the function
            _sstore[i].func();
            // vacate the slot
            _map &= ~_BV(i);
            // decrement the number left and see if we can stop
            if (--remaining == 0)
            {
                goto service_complete;
            }
        }
    }
service_complete:
    return;
}
