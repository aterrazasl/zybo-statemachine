/* FreeRTOS includes. */
#include "FreeRTOS.h"
#include "sm/blinky1_sm.h"
#include "sm/blinky2_sm.h"
#include "hcd/hcd.h"
//#include "display/display_sm.h"
//#include "star_search/star_search_sm.h"
#include "quadTree/quadtree_sm.h"


static blinky1_params blinky1;
static blinky1_params blinky2;
static hcd_params hcd;
//static display_params disp;
//static ss_params ss;
static quadtree_params qt;


// task handles
static TaskHandle_t blinky1_handle;
static TaskHandle_t blinky2_handle;
static TaskHandle_t hcd_handle;
//static TaskHandle_t display_handle;
//static TaskHandle_t ss_handle;
static TaskHandle_t qt_handle;


int main(void) {

	blinky1.statemachine.init_ptr = Blinky1_init;
	xTaskCreate(SM_dispatcher, (const char *) "Blinky1",
	configMINIMAL_STACK_SIZE, &blinky1, tskIDLE_PRIORITY,
			&blinky1_handle);

	blinky2.statemachine.init_ptr = Blinky2_init;
	xTaskCreate(SM_dispatcher, (const char *) "Blinky2",
	configMINIMAL_STACK_SIZE, &blinky2, tskIDLE_PRIORITY,
			&blinky2_handle);

	hcd.statemachine.init_ptr = hcd_init;
	xTaskCreate(SM_dispatcher, (const char *) "HCD",
	configMINIMAL_STACK_SIZE, &hcd, tskIDLE_PRIORITY+2,
			&hcd_handle);

//	disp.statemachine.init_ptr = Display_init;
//	xTaskCreate(SM_dispatcher, (const char *) "DVI",
//	configMINIMAL_STACK_SIZE, &disp, tskIDLE_PRIORITY+1,
//			&display_handle);

//	ss.statemachine.init_ptr = SS_init;
//	xTaskCreate(SM_dispatcher, (const char *) "SS",
//	configMINIMAL_STACK_SIZE, &ss, tskIDLE_PRIORITY+1,
//			&ss_handle);

	qt.statemachine.init_ptr = quadtree_init;
	xTaskCreate(SM_dispatcher, (const char *) "QT",
	configMINIMAL_STACK_SIZE, &qt, tskIDLE_PRIORITY+1,
			&qt_handle);


	vTaskStartScheduler();

	for (;;);
}
