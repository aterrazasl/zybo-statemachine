#include "display_sm.h"
#include "dvi_vdma.h"
#include "display_matrix.h"
#include "GFX.h"
#include "../hcd/hcd.h"

#include "display_matrix.h"

#define NUMBER_OF_CUBES 5
#define NUMBER_OF_VERTICES 8


vector points[NUMBER_OF_VERTICES]= {	{-.5,-.5,.5}, //X,Y,Z
										{ .5,-.5,.5}, //X,Y,Z
										{ .5, .5,.5}, //X,Y,Z
										{-.5, .5,.5}, //X,Y,Z
										{-.5,-.5,-.5}, //X,Y,Z
										{ .5,-.5,-.5}, //X,Y,Z
										{ .5, .5,-.5}, //X,Y,Z
										{-.5, .5,-.5}}; //X,Y,Z

vector cubes[NUMBER_OF_CUBES][NUMBER_OF_VERTICES];
vector old_points[NUMBER_OF_CUBES][NUMBER_OF_VERTICES];
vector axis_arr[NUMBER_OF_CUBES] = {{-100,100,0},{-100,-100,0},{100,100,0},{100,-100,0},{0,0,0}};
int16_t scales_arr[NUMBER_OF_CUBES]={100,200,150,75,100};
float start_angle[NUMBER_OF_CUBES]={0,0.5,0.3,0.8,0};
float inc_angle[NUMBER_OF_CUBES]={0.01,0.02,-0.03,0.04,-0.01};




static SM_return Display_rotate(display_params *pvParameters,void * event);
static SM_return Display_draw(display_params *pvParameters,void * event);


static void incrementAngle(float incAngle){
	for(int i =0; i<NUMBER_OF_CUBES;i++){
		inc_angle[i]+=incAngle;
	}
}

static void changeDirection(float dir){
	for(int i =0; i<NUMBER_OF_CUBES;i++){
		inc_angle[i]*=dir;
	}
}

static void vTimerCallback(TimerHandle_t xTimer) {
	events sm_event = disp_timer_tick;
	display_params * pvParameters = (display_params*)pvTimerGetTimerID(xTimer);

	xQueueSend(pvParameters->statemachine.stateQueue, &sm_event, (TickType_t ) 0);
}

static void initTimer(display_params *pvParameters){
	pvParameters->timerCallbackFunc = vTimerCallback;
	pvParameters->timerhandle = xTimerCreate("SM_Timer_display", 1, pdFALSE, pvParameters, pvParameters->timerCallbackFunc);

}

static void Display_drawlineVector(vector* start, vector* end){

	Display_drawLine(start->x,start->y,end->x,end->y);
}

static void initCubes(void){
	for(int j= 0; j<NUMBER_OF_CUBES;j++){
		for (int p = 0; p < NUMBER_OF_VERTICES; p++) {
			multiplicateVectorsScalar(&points[p], 1, &cubes[j][p]);
		}
	}
}

static void rotateCube(vector *axis, vector* point_old, vector* cube, float angle,float scale){
	int p;

	Display_updateXYaxis(axis->x,axis->y);
	GFX_changePenColor(0x00);	// change color to Black
	for (p = 0; p < NUMBER_OF_VERTICES; p++) {
		Display_fillCircle(point_old[p].x, point_old[p].y, 2);
	}

	for (p = 0; p < 4; p++) {
		Display_drawlineVector(&point_old[p], &point_old[(p + 1) % 4]);
		Display_drawlineVector(&point_old[p+4],&point_old[(((p+4) + 1) % 4) + 4]);
		Display_drawlineVector(&point_old[p], &point_old[(p + 4)]);
	}

	GFX_changePenColor(0xff);

	for (p = 0; p < NUMBER_OF_VERTICES; p++) {
		vector res;
		vector rotate;

		rotateZ_vector(&cube[p], angle, &res);
		copyVector(&res, &rotate);
		rotateX_vector(&rotate, angle, &res);
		copyVector(&res, &rotate);
		rotateY_vector(&rotate, angle, &res);
		copyVector(&res, &rotate);

		float distance = 2;
		float z = 1/(distance - rotate.z);

		matrix projection ={{z,0,0},
							{0,z,0},
							{0,0,0}};

		multiplateMatrixVector(&projection, &rotate, &res);
		multiplicateVectorsScalar(&res, scale, &point_old[p]);
	}

	for (p = 0; p < NUMBER_OF_VERTICES; p++) {
		Display_fillCircle(point_old[p].x, point_old[p].y, 2);
	}
	for (p = 0; p < 4; p++) {
		Display_drawlineVector(&point_old[p], &point_old[(p + 1) % 4]);
		Display_drawlineVector(&point_old[p+4],&point_old[(((p+4) + 1) % 4) + 4]);
		Display_drawlineVector(&point_old[p], &point_old[(p + 4)]);
	}
}


//--------------State machine ----------------------------------------


static SM_return Display_stop(display_params *pvParameters,void * event){

	events *e = (events*) event;
	SM_return ret = state_handled;
	switch (*e) {
	case event_enter:
		ret = state_handled;
		break;
	case hid_a:
		SM_nextState((void*)pvParameters, (void*)Display_rotate);
		ret = state_transition;
		break;

	case event_exit:
		ret = state_handled;
		break;
	default:
		ret = state_error;
		break;
	}
	return ret;
}

static SM_return Display_draw(display_params *pvParameters,void * event){

	events *e = (events*) event;
	SM_return ret = state_handled;
	switch (*e) {
	case event_enter:

		xTimerStart(pvParameters->timerhandle, 0);
		Display_flushMem();
		ret = state_handled;
		break;
	case disp_timer_tick:

		SM_nextState((void*)pvParameters, (void*)Display_rotate);
		ret = state_transition;
		break;
	case hid_start:
		SM_nextState((void*)pvParameters, (void*)Display_stop);
		ret = state_transition;
		break;
	case hid_up:
		incrementAngle(0.01);
		ret = state_handled;
		break;
	case hid_down:
		incrementAngle(-0.01);
		ret = state_handled;
		break;
	case hid_left:
		changeDirection(-1);
		ret = state_handled;
		break;
	case hid_right:
		changeDirection(-1);
		ret = state_handled;
		break;

	case event_exit:
		ret = state_handled;
		break;
	default:
		ret = state_error;
		break;
	}
	return ret;
}

static SM_return Display_rotate(display_params *pvParameters,void * event){

	events *e = (events*) event;
	SM_return ret = state_handled;
	switch (*e) {
	case event_enter:
		for(int c=0; c<NUMBER_OF_CUBES;c++){
			rotateCube(&axis_arr[c],old_points[c],cubes[c],start_angle[c],scales_arr[c]);
			start_angle[c]+= inc_angle[c];
		}
		SM_nextState((void*)pvParameters, (void*)Display_draw);
		ret = state_transition;
		break;
	case event_exit:
		ret = state_handled;
		break;
	default:
		ret = state_error;
		break;
	}
	return ret;
}

SM_return Display_init(display_params *pvParameters,void * event){

	events *e = (events*) event;
	SM_return ret = state_handled;
	switch (*e) {
	case event_init:

		initTimer(pvParameters);

		initCubes();

		DVI_initDVI();

		SM_nextState((void*)pvParameters, (void*)Display_rotate);
		ret = state_transition;
		break;
	case event_exit:
		ret = state_handled;
		break;
	default:
		ret = state_error;
		break;
	}
	return ret;

}

