#ifndef STATEMACHINE_H
#define STATEMACHINE_H


/* FreeRTOS includes. */
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "timers.h"


typedef enum{
	event_init,
	event_enter,
	event_timer_tick,
	event_exit
}events;

typedef enum {
	state_handled, state_transition, state_error
} SM_return;

typedef struct {
	QueueHandle_t stateQueue;
	SM_return (*stateFun_ptr)(void*,void*);
	SM_return (*init_ptr)(void*,void*);
} SM_params;

void nextState(SM_params *pvParameters, void* sm_state);
void SM_dispatcher(void* params);

#endif
