#include "statemachine.h"


SM_params *signal_pool[MAX_NUMBER_OF_SM]; //Keep track of all the SM references in order to send signals to all of them
int pool_count = 0;	// number of signal pools

void SM_nextState(SM_params *pvParameters, void* sm_state) {
	pvParameters->nextStateFun_ptr = sm_state;
//	pvParameters->stateFun_ptr = sm_state;
}

void SM_broadcastSignal(void* sm_event) {
	if (pool_count == MAX_NUMBER_OF_SM) {
		for (int i = 0; i < pool_count; i++) {
			xQueueSend(signal_pool[i]->stateQueue, sm_event, (TickType_t ) 0);
		}
	}
}

void SM_dispatcher(void* params) {
	SM_params* pvParameters = (SM_params*)params;
	SM_return sm_ret;
	events sm_event;

	pvParameters->stateQueue = xQueueCreate(100, sizeof(events));
	signal_pool[pool_count]= pvParameters;
	pool_count++;

	pvParameters->stateFun_ptr = pvParameters->init_ptr; // Init the state
	SM_nextState(pvParameters, pvParameters->init_ptr);
	sm_event = event_init;
	xQueueSend(pvParameters->stateQueue,  &sm_event, (TickType_t ) 0);

	while (1) {

		xQueueReceive(pvParameters->stateQueue, &sm_event,(TickType_t) portMAX_DELAY);

		sm_ret = (*pvParameters->stateFun_ptr)(pvParameters, &sm_event);

		switch (sm_ret) {
		case state_handled:
			pvParameters->stateFun_ptr = pvParameters->nextStateFun_ptr;
			break;
		case state_transition:
			sm_event =event_exit;
			xQueueSend(pvParameters->stateQueue,  &sm_event, (TickType_t ) 0);
			sm_event =event_enter;
			xQueueSend(pvParameters->stateQueue,  &sm_event, (TickType_t ) 0);

			break;
		case state_error:
			break;
		default:
			break;

		}
	}
}
