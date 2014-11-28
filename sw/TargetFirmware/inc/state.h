#ifndef STATE_H
#define STATE_H

#define MAX_EVENT_CNT 5

typedef void (*State)(uint8_t);

#define DEFAULT_EVENTS IDLE=0,ENTER=1,EXIT=2

typedef struct
{
    State   current_state;
    uint8_t event_code;
    State   next_state;
} Transition;

typedef struct
{
    uint8_t event_queue[MAX_EVENT_CNT];
    uint8_t start;
    uint8_t event_cnt;
    Transition* transitions;
    uint8_t transition_table_size;  /**< Size of transition table */
    State state;                    /**< Current state of the state machine */
} StateMachine;

extern StateMachine StateMachineCreate(Transition* rules, uint8_t t_size, State state);
extern int8_t StateMachinePublishEvent(StateMachine* s, uint8_t event);
extern void StateMachineRun(StateMachine* s);

#endif
