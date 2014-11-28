#ifndef SCHEDULE_H
#define SCHEDULE_H

#define MAX_PER_EVENT_CNT 10
#define MAX_SING_EVENT_CNT 10

typedef void (*Fn)(void);

extern volatile uint32_t g_now;

extern void ScheduleService(void);
extern int8_t SchedulePeriodicEvent(Fn func, uint32_t run_time);
extern void SchedulePeriodicEnable(Fn func, uint8_t mode);
extern int8_t ScheduleSingleEvent(Fn func, uint32_t run_time);
extern void ScheduleSingleCancel(Fn func);

#endif // SCHEDULE_H
