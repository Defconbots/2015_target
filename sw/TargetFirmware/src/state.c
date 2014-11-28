#include "global.h"
#include "state.h"

//::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
enum LocalDefaults {DEFAULT_EVENTS};
static uint8_t DequeueEvent(StateMachine* s);
static State LookupTransition(StateMachine* s, uint8_t event);

//::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
StateMachine StateMachineCreate(Transition* rules, uint8_t t_size, State state)
{
    StateMachine s;
    s.start = 0;
    s.event_cnt = 0;
    s.transitions = rules;
    s.transition_table_size = (t_size / sizeof(Transition));
    s.state = state;
    StateMachinePublishEvent(&s, ENTER);
    return (s);
}

//::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
int8_t StateMachinePublishEvent(StateMachine* s, uint8_t event)
{
    int8_t ret = FAILURE;

    if (s->event_cnt < MAX_EVENT_CNT)
    {
        if (s->start + s->event_cnt == MAX_EVENT_CNT)
        {
            s->event_queue[0] = event;
        }
        else
        {
            s->event_queue[s->start + s->event_cnt] = event;
        }
        s->event_cnt++;
    }
    return ret;
}

//::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
uint8_t DequeueEvent(StateMachine* s)
{
    uint8_t ret = IDLE;
    if (s->event_cnt)
    {
        ret = s->event_queue[s->start];
        if (s->start == MAX_EVENT_CNT - 1)
        {
            s->start = 0;
        }
        else
        {
            s->start += 1;
        }
        s->event_cnt -= 1;
    }
    return ret;
}

//::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
State LookupTransition(StateMachine* s, uint8_t event)
{
    uint8_t i = 0;
    State ret_state = s->state;

    // if the event is idle or enter/exit just return state)
    if (event != IDLE && event != ENTER && event != EXIT)
    {
        // if event is a new event find the transition
        for (i = 0; i < s->transition_table_size; i++)
        {
            if (s->transitions[i].current_state == s->state &&
                s->transitions[i].event_code    == event)
            {
                ret_state = s->transitions[i].next_state;
                break;
            }
        }
    }
    return ret_state;
}

//::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
void StateMachineRun(StateMachine* s)
{
    uint8_t next_event = DequeueEvent(s);
    State next_state = LookupTransition(s, next_event);
    // Will this cause a transition?
    if (s->state != next_state)
    {
        (s->state)(EXIT);
        s->state = next_state;
        (s->state)(ENTER);
    }
    else
    {
        (s->state)(next_event);
    }
}
