#ifndef STATEMACHINE_H
#define STATEMACHINE_H


/* FreeRTOS includes. */
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "timers.h"


#define MAX_NUMBER_OF_SM 4

typedef enum{
	event_init,
	event_enter,
	event_exit,
	// base events handled by the state machine
	event_timer_tick = 10,
	disp_timer_tick = 20,
}events;

typedef enum {
	state_handled, state_transition, state_error
} SM_return;

typedef struct {
	QueueHandle_t stateQueue;
	SM_return (*stateFun_ptr)(void*,void*);
	SM_return (*nextStateFun_ptr)(void*,void*);
	SM_return (*init_ptr)(void*,void*);
} SM_params;


void SM_broadcastSignal(void* sm_event);
void SM_nextState(SM_params *pvParameters, void* sm_state);
void SM_dispatcher(void* params);

#endif
