#include "statemachine.h"


void nextState(SM_params *pvParameters, void* sm_state) {

//	pvParameters->nextStateFun_ptr = sm_state;
	pvParameters->stateFun_ptr = sm_state;


}


void SM_dispatcher(void* params) {
	SM_params* pvParameters = (SM_params*)params;
	SM_return sm_ret;
	events sm_event;

	pvParameters->stateQueue = xQueueCreate(10, sizeof(events));

	pvParameters->stateFun_ptr = pvParameters->init_ptr; // Init the state
	nextState(pvParameters, pvParameters->init_ptr);
	sm_event = event_init;
	xQueueSend(pvParameters->stateQueue,  &sm_event, (TickType_t ) 0);

	while (1) {

		xQueueReceive(pvParameters->stateQueue, &sm_event,(TickType_t) portMAX_DELAY);

		sm_ret = (*pvParameters->stateFun_ptr)(pvParameters, &sm_event);

		switch (sm_ret) {
		case state_handled:
//			pvParameters->stateFun_ptr = pvParameters->nextStateFun_ptr;
			break;
		case state_transition:
//			sm_event =event_exit;
//			xQueueSend(pvParameters->stateQueue,  &sm_event, (TickType_t ) 0);
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
