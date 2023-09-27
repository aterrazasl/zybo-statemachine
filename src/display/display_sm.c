#include "display_sm.h"
#include "dvi_vdma.h"
#include "display_matrix.h"
#include "GFX.h"
#include <math.h>
#include "../hcd/hcd.h"


typedef struct{
	float x;
	float y;
	float z;
}vector;

typedef struct{
	vector a;
	vector b;
	vector c;
}matrix;

vector points[8]= {	{-.5,-.5,.5}, //X,Y,Z
					{ .5,-.5,.5}, //X,Y,Z
					{ .5, .5,.5}, //X,Y,Z
					{-.5, .5,.5}, //X,Y,Z
					{-.5,-.5,-.5}, //X,Y,Z
					{ .5,-.5,-.5}, //X,Y,Z
					{ .5, .5,-.5}, //X,Y,Z
					{-.5, .5,-.5}}; //X,Y,Z

vector cubes[4][8];
vector old_points[4][8];
vector axis_arr[4] = {{-100,100,0},{-100,-100,0},{100,100,0},{100,-100,0}};
int16_t scales_arr[4]={100,200,150,75};
float angles[4]={0,0.5,.3,.8};



float angle=0;

matrix orthogonal_projection ={ {1,0,0},
								{0,1,0},
								{0,0,1}};


static SM_return Display_rotate(display_params *pvParameters,void * event);
static SM_return Display_draw(display_params *pvParameters,void * event);
static SM_return Display_idle(display_params *pvParameters,void * event);

static void vTimerCallback(TimerHandle_t xTimer) {
	events sm_event = disp_timer_tick;
	display_params * pvParameters = (display_params*)pvTimerGetTimerID(xTimer);

	xQueueSend(pvParameters->statemachine.stateQueue, &sm_event, (TickType_t ) 0);
}

static SM_return Display_stop(display_params *pvParameters,void * event){

	events *e = (events*) event;
	SM_return ret = state_handled;
	switch (*e) {
	case event_enter:
		ret = state_handled;
		break;
	case hid_a:
		nextState((void*)pvParameters, (void*)Display_rotate);
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

		Display_flushMem();
		xTimerStart(pvParameters->timerhandle, 0);
		ret = state_handled;
		break;
	case disp_timer_tick:

		nextState((void*)pvParameters, (void*)Display_rotate);
		ret = state_transition;
		break;
	case hid_start:
		nextState((void*)pvParameters, (void*)Display_stop);
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

static void multiplicateVectorsScalar(vector * a, float b, vector* res){

	res->x = a->x * b;
	res->y = a->y * b;
	res->z = a->z * b;
}

static float multiplicateVectors(vector * a, vector* b){

	float result=0;

	result = (a->x * b->x) + (a->y * b->y) + (a->z * b->z);
	return result;
}

static void multiplateMatrixVector(matrix* m, vector* v, vector * res){
	res->x = multiplicateVectors(&m->a,v);
	res->y = multiplicateVectors(&m->b,v);
	res->z = multiplicateVectors(&m->c,v);
}

static void rotateZ_vector(vector * v, float angle, vector * res){

	 matrix rotate = {	{cosf(angle), -sinf(angle),0},
						{sinf(angle),  cosf(angle),0},
						{         0,           0,1}};


	multiplateMatrixVector(&rotate,v,res);

}

static void rotateX_vector(vector * v, float angle, vector * res){

	 matrix rotate = {	{          1,            0,           0},
						{          0,  cosf(angle),-sinf(angle)},
						{          0,  sinf(angle), cosf(angle)}};

	multiplateMatrixVector(&rotate,v,res);

}

static void rotateY_vector(vector * v, float angle, vector * res){

	 matrix rotate = {	{cosf(angle),            0, sinf(angle)},
						{          0,            1,            0},
						{-sin(angle),            0,  cos(angle)}};


	multiplateMatrixVector(&rotate,v,res);

}

static void copyVector(vector* src,vector* dest ){
	memcpy(dest,src,sizeof(vector));

}

static void Display_clear(void){
	GFX_changePenColor(0x00);	// change color to Black
	GFX_fillRect(1, 1, DVI_HORIZONTAL-2, DVI_VERTICAL-2);

	GFX_changePenColor(0xff);
}

static void Display_drawlineVector(vector* start, vector* end){

	Display_drawLine(start->x,start->y,end->x,end->y);
}

static void rotateCube(vector *axis, vector* point_old, vector* cube, float angle,float scale){
	int p;

	Display_updateXYaxis(axis->x,axis->y);
	GFX_changePenColor(0x00);	// change color to Black
	for (p = 0; p < 8; p++) {
		Display_fillCircle(point_old[p].x, point_old[p].y, 2);
	}

	for (p = 0; p < 4; p++) {
		Display_drawlineVector(&point_old[p], &point_old[(p + 1) % 4]);
		Display_drawlineVector(&point_old[p+4],&point_old[(((p+4) + 1) % 4) + 4]);
		Display_drawlineVector(&point_old[p], &point_old[(p + 4)]);
	}

	GFX_changePenColor(0xff);

	for (p = 0; p < 8; p++) {
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
//		copyVector(&res, &point_old[p]);
		multiplicateVectorsScalar(&res, scale, &point_old[p]);
	}

	for (p = 0; p < 8; p++) {
		Display_fillCircle(point_old[p].x, point_old[p].y, 2);
	}
	for (p = 0; p < 4; p++) {
		Display_drawlineVector(&point_old[p], &point_old[(p + 1) % 4]);
		Display_drawlineVector(&point_old[p+4],&point_old[(((p+4) + 1) % 4) + 4]);
		Display_drawlineVector(&point_old[p], &point_old[(p + 4)]);
	}



}


static SM_return Display_rotate(display_params *pvParameters,void * event){

	events *e = (events*) event;
	SM_return ret = state_handled;
	switch (*e) {
	case event_enter:
		for(int c=0; c<4;c++){
			rotateCube(&axis_arr[c],old_points[c],cubes[c],angles[c],scales_arr[c]);
		}
			angles[0] += .01;
			angles[1] -= .01;
			angles[2] += .02;
			angles[3] -= .04;
		nextState((void*)pvParameters, (void*)Display_draw);
		ret = state_transition;
		break;
	case hid_start:
		nextState((void*)pvParameters, (void*)Display_stop);
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

static SM_return Display_idle(display_params *pvParameters,void * event){

	static int stat = 0;

	events *e = (events*) event;
	SM_return ret = state_handled;
	switch (*e) {
	case event_enter:
		ret = state_handled;
		break;
	case disp_timer_tick:
		if(stat == 0){
			BSP_clearLED(3);
			stat =1;
		}
		else{
			BSP_setLED(3);
			stat = 0;
		}
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

SM_return Display_init(display_params *pvParameters,void * event){

	events *e = (events*) event;
	SM_return ret = state_handled;
	switch (*e) {
	case event_init:

		pvParameters->timerCallbackFunc = vTimerCallback;
		pvParameters->timerhandle = xTimerCreate("SM_Timer_display", 1, pdFALSE, pvParameters,
				pvParameters->timerCallbackFunc);
//		xTimerStart(pvParameters->timerhandle, 0);

		for(int j= 0; j<4;j++){
			for (int p = 0; p < 8; p++) {
				multiplicateVectorsScalar(&points[p], 1, &cubes[j][p]);
			}
		}


		DVI_initDVI();

		nextState((void*)pvParameters, (void*)Display_rotate);
		ret = state_transition;
		break;
	case hid_start:
		nextState((void*)pvParameters, (void*)Display_stop);
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

