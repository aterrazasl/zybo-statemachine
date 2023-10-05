#include "quadtree_sm.h"
#include "../bsp/bsp.h"
#include "../display/dvi_vdma.h"
#include <stdlib.h>
#include "../hcd/hcd.h"

#define NUM_POINTS 500
static point_t newPoint[640*480];

list_t * foundPoints= NULL;
boundary_t test={{0,0},100,100};

static SM_return quadtree_draw(quadtree_params *pvParameters,void * event);
static SM_return quadtree_addpoint(quadtree_params *pvParameters,void * event);

static void vTimerCallback(TimerHandle_t xTimer) {
	events sm_event = disp_timer_tick;
	quadtree_params * pvParameters = (quadtree_params*)pvTimerGetTimerID(xTimer);

	xQueueSend(pvParameters->statemachine.stateQueue, &sm_event, (TickType_t ) 0);
}

static point_t generateRandomPoint(void){
	point_t ret;
	ret.x = ((double)rand() / RAND_MAX) * (320 - (-320)) + (-320);
	ret.y = ((double)rand() / RAND_MAX) * (240 - (-240)) + (-240);

	return ret;
}

static point_t generateRandomNum(void){

	point_t ret;
	ret.x = ((double)rand() / RAND_MAX) * (2 - (-2)) + (-2);
	ret.y = ((double)rand() / RAND_MAX) * (2 - (-2)) + (-2);

	return ret;
}

static void drawPoints(quadtree_t *qt,int color){
	if(qt == NULL)
		return;

	Display_changePenColor(color);
	for(int i = 0; i < qt->points->len; i++){
		Display_setPixel(qt->points->list[i].x,qt->points->list[i].y);
	}
	drawPoints(qt->nw,color);
	drawPoints(qt->ne,color);
	drawPoints(qt->sw,color);
	drawPoints(qt->se,color);

	return;
}

static void drawBoundary(quadtree_t *qt,int color){
	if(qt == NULL)
		return;
	Display_drawRectCenter(qt->boundary.center.x, qt->boundary.center.y,qt->boundary.w,qt->boundary.h,color);

	drawBoundary(qt->nw,color);
	drawBoundary(qt->ne,color);
	drawBoundary(qt->sw,color);
	drawBoundary(qt->se,color);

	return;
}

static void updatePoints(){
	for (int i = 0; i< NUM_POINTS; i++){
		point_t temp = generateRandomNum();
		newPoint[i].x += temp.x;
		newPoint[i].y += temp.y;
	}
}

static SM_return quadtree_draw(quadtree_params *pvParameters,void * event){

	events *e = (events*) event;
	SM_return ret = state_handled;
	switch (*e) {
	case event_enter:
		srand(xTaskGetTickCount());
		updatePoints();
//		drawBoundary(pvParameters->qt,CGray);
		drawPoints(pvParameters->qt,CRed);

		Display_drawRectCenter(test.center.x, test.center.y,test.w,test.h,CGreen);
		Display_changePenColor(CGreen);
		for(int i = 0; i < foundPoints->len; i++){
			Display_setPixel(foundPoints->list[i].x,foundPoints->list[i].y);
		}

		Display_flushMem();
		xTimerStart(pvParameters->timerhandle, 0);
		ret = state_handled;
		break;
	case disp_timer_tick:
		SM_nextState((void*)pvParameters, (void*)quadtree_init);
		ret = state_transition;
		break;
	case hid_select:
		SM_nextState((void*)pvParameters, (void*)quadtree_init);
		ret = state_transition;
		break;
	case hid_up:
		Display_drawRectCenter(test.center.x, test.center.y,test.w,test.h,CBlack);
		test.center.y += 5;
		ret = state_handled;
		break;
	case hid_down:
		Display_drawRectCenter(test.center.x, test.center.y,test.w,test.h,CBlack);
		test.center.y -= 5;
		ret = state_handled;
		break;
	case hid_left:
		Display_drawRectCenter(test.center.x, test.center.y,test.w,test.h,CBlack);
		test.center.x -= 5;
		ret = state_handled;
		break;
	case hid_right:
		Display_drawRectCenter(test.center.x, test.center.y,test.w,test.h,CBlack);
		test.center.x += 5;
		ret = state_handled;
		break;
	case event_exit:
//		drawBoundary(pvParameters->qt,CBlack);
		drawPoints(pvParameters->qt,CBlack);
		quadtree_destroy();
		ret = state_handled;
		break;
	default:
		ret = state_error;
		break;
	}
	return ret;
}

static SM_return quadtree_addpoint(quadtree_params *pvParameters,void * event){

	events *e = (events*) event;
	SM_return ret = state_handled;
	switch (*e) {
	case event_enter:

		for (int i = 0; i< NUM_POINTS; i++){
			quadtree_insert(pvParameters->qt,newPoint[i]);
		}

		foundPoints->len =0;
		foundPoints = quadtree_queryRange(pvParameters->qt,foundPoints, test);

		SM_nextState((void*)pvParameters, (void*)quadtree_draw);
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

SM_return quadtree_init(quadtree_params *pvParameters,void * event) {
	events *e = (events*) event;
	SM_return ret = state_handled;
	switch (*e) {
	case event_init:
		DVI_initDVI();
		Display_updateXYaxis(0,0);
		srand(xTaskGetTickCount());

		for (int i = 0; i< NUM_POINTS; i++){
			newPoint[i] = generateRandomPoint();
		}
		pvParameters->timerCallbackFunc = vTimerCallback;
		pvParameters->timerhandle = xTimerCreate("QT_Timer", 3, pdFALSE, pvParameters,
				pvParameters->timerCallbackFunc);

	case event_enter:
		srand(xTaskGetTickCount());
		boundary_t b = {{0,0},DVI_HORIZONTAL_CENTER,DVI_VERTICAL_CENTER};

		pvParameters->qt = quadtree_create(b);


		SM_nextState((void*)pvParameters, (void*)quadtree_addpoint);
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

