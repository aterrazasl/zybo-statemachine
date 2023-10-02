#include <stdlib.h>
#include "star_search_sm.h"
#include "../display/dvi_vdma.h"
#include "../display/GFX.h"
#include <math.h>
#include <string.h>
#include "../hcd/hcd.h"

#define GRID_WIDTH (16*8)
#define GRID_HEIGHT (12*8)
#define GRID_BOX_WIDTH (DVI_HORIZONTAL/GRID_WIDTH)+0
#define GRID_BOX_HEIGHT (DVI_VERTICAL/GRID_HEIGHT)+0
#define SIDE_UP  	 [i  ][j-1]
#define SIDE_UPRIGHT [i+1][j-1]
#define SIDE_RIGHT   [i+1][j  ]
#define SIDE_DWRIGHT [i+1][j+1]
#define SIDE_DW  	 [i  ][j+1]
#define SIDE_DWLEFT  [i-1][j+1]
#define SIDE_LEFT    [i-1][j  ]
#define SIDE_UPLEFT  [i-1][j-1]




typedef enum{
	up,upright,right,downright,down,downleft,left,upleft
}sides;

typedef struct{
	uint16_t i;
	uint16_t j;
}index_t;

typedef struct{
	float f;
	float h;
	float g;
	float a;
	int done;
}cost_t;

typedef struct{
	int32_t x;
	int32_t y;
}node_t;

typedef struct{
	node_t node;
	int32_t h;
	int32_t w;
	int color;
}box_t;

typedef struct{
	box_t margin;
	box_t box;
	index_t index;
	int wall;
}item_t;

typedef struct{
	item_t box;
	cost_t cost;
}spot_t;

typedef struct{
	spot_t current;
	void* prev;
}grid_t;

typedef struct{
	grid_t *list [GRID_WIDTH*GRID_HEIGHT];
	int len;
}list_t;


grid_t grid[GRID_WIDTH][GRID_HEIGHT];
typedef grid_t gd_t[GRID_WIDTH][GRID_HEIGHT];
gd_t * const gd = &grid;

//grid_t grid_ol[GRID_WIDTH*GRID_HEIGHT];
list_t openList;
list_t * const ol = &openList;
//grid_t grid_cl[GRID_WIDTH*GRID_HEIGHT];
list_t closeList;
list_t * const cl = &closeList;
//grid_t grid_ph[GRID_WIDTH*GRID_HEIGHT];
list_t path;
list_t * const ph = &path;
//grid_t grid_nl[GRID_WIDTH*GRID_HEIGHT];
list_t neighbors;
list_t * const nl = &neighbors;

grid_t * start;
grid_t * destination;

grid_t *current = NULL;

static SM_return SS_stop(ss_params *pvParameters,void * event);
static SM_return SS_calculate(ss_params *pvParameters,void * event);
static SM_return SS_draw(ss_params *pvParameters,void * event);
static void clearList(list_t * l);

static void printFloat(float fval){
	int whole, thousandths;
	whole = fval;
	thousandths = (fval - whole) * 1000;
	xil_printf("%02d.%03d", whole, thousandths);
}

static void debug_printNeighbors(list_t *ng){

	xil_printf("size of List = %d\r\n", ng->len);
	for(int i = 0; i < ng->len; i++){
		xil_printf("n= %d,	i= %d, j= %d",i,ng->list[i]->current.box.index.i,ng->list[i]->current.box.index.j);
		print(", f= ");
		printFloat(ng->list[i]->current.cost.f);
		print(", g= ");
		printFloat(ng->list[i]->current.cost.g);
		print(", h= ");
		printFloat(ng->list[i]->current.cost.h);

		print("\r\n");
	}
}

static void vTimerCallback(TimerHandle_t xTimer) {
	events sm_event = disp_timer_tick;
	ss_params * pvParameters = (ss_params*)pvTimerGetTimerID(xTimer);

	xQueueSend(pvParameters->statemachine.stateQueue, &sm_event, (TickType_t ) 0);
}

static void initTimer(ss_params *pvParameters){
	pvParameters->timerCallbackFunc = vTimerCallback;
	pvParameters->timerhandle = xTimerCreate("SM_Timer_display", 1, pdFALSE, pvParameters, pvParameters->timerCallbackFunc);

}
static int inList(list_t * l, grid_t * item){
	int ret = 0;

	for(int i = 0; i < l->len; i++){
		if(l->list[i]  == item){
			ret = 1;
			break;
		}
	}

	return ret;
}

static void initGrid(gd_t * gridptr){

	int wall=0;
	int color;

	for(int i = 0; i < GRID_WIDTH; i++){
		for(int j = 0; j < GRID_HEIGHT; j++){

			(*gridptr)[i][j].current.cost = (cost_t){0,0,0,0};
			(*gridptr)[i][j].current.box.margin.node.x = i * GRID_BOX_WIDTH;
			(*gridptr)[i][j].current.box.margin.node.y = j * GRID_BOX_HEIGHT;
			(*gridptr)[i][j].current.box.margin.h = GRID_BOX_HEIGHT+0;
			(*gridptr)[i][j].current.box.margin.w = GRID_BOX_WIDTH+0;
			(*gridptr)[i][j].current.box.margin.color = CWhite;

			(*gridptr)[i][j].current.box.index = (index_t){i,j};
			(*gridptr)[i][j].current.box.box.node.x = (i * GRID_BOX_WIDTH ) + 1;
			(*gridptr)[i][j].current.box.box.node.y = (j * GRID_BOX_HEIGHT) + 1;
			(*gridptr)[i][j].current.box.box.h = GRID_BOX_HEIGHT - 1;
			(*gridptr)[i][j].current.box.box.w = GRID_BOX_WIDTH - 1;

			if(rand() > RAND_MAX *.8){
				wall=1;
				color = CBlack;
			}
			else{
				wall =0;
				color = CGray;
			}
//			wall =0;
//			color = CGray;
			(*gridptr)[i][j].current.box.wall = wall;
			(*gridptr)[i][j].current.box.box.color = color;
		}
	}
}

static void drawBox(gd_t * gridptr, int i, int j){
	int16_t x = (*gridptr)[i][j].current.box.margin.node.x;
	int16_t y = (*gridptr)[i][j].current.box.margin.node.y;
	int16_t h = (*gridptr)[i][j].current.box.margin.h;
	int16_t w = (*gridptr)[i][j].current.box.margin.w;
	int16_t c = (*gridptr)[i][j].current.box.margin.color;
	Display_drawRect(x, y, w, h, c);

	x = (*gridptr)[i][j].current.box.box.node.x;
	y = (*gridptr)[i][j].current.box.box.node.y;
	h = (*gridptr)[i][j].current.box.box.h;
	w = (*gridptr)[i][j].current.box.box.w;
	c = (*gridptr)[i][j].current.box.box.color;
	Display_fillRect(x, y, w, h, c);
}

static void drawGrid(gd_t * gridptr){
	for(int i = 0; i < GRID_WIDTH; i++){
		for(int j = 0; j < GRID_HEIGHT; j++){
			drawBox(gridptr,i,j);
		}
	}
}

static void drawList(list_t * lstPtr, int16_t color) {
	if (lstPtr->len > 0) {
		for (int i = 0; i < lstPtr->len; i++) {
			int16_t x = lstPtr->list[i]->current.box.margin.node.x;
			int16_t y = lstPtr->list[i]->current.box.margin.node.y;
			int16_t h = lstPtr->list[i]->current.box.margin.h;
			int16_t w = lstPtr->list[i]->current.box.margin.w;
//		int16_t c = lstPtr->list[i]->box.margin.color;
//		Display_drawRect(x, y, w, h, c);

			x = lstPtr->list[i]->current.box.box.node.x;
			y = lstPtr->list[i]->current.box.box.node.y;
			h = lstPtr->list[i]->current.box.box.h;
			w = lstPtr->list[i]->current.box.box.w;
//		c = lstPtr->list[i]->box.box.color;
			Display_fillRect(x, y, w, h, color);
		}
	}
}

static void addItemToList(list_t * const l, grid_t *item){
	if (!inList(l, item)) {
		l->list[l->len] = item;
		l->len += 1;
	}
}

static void drawPath(grid_t *cur){
	ph->len =0;
	addItemToList(ph,cur);
	grid_t * temp = cur;
	while(temp->prev){
		addItemToList(ph,temp);
		temp = (grid_t *)temp->prev;
	}
	drawList(ph,CGreen);
}


static void removeItemFromList(list_t * const l, grid_t *item) {

	if (l->len > 1) {
		for (int i = 0; i < l->len; i++) {
			if (l->list[i] == item) {
				memcpy(&l->list[i], &l->list[i + 1], sizeof(grid_t*) * (l->len - i));
			}
		}
	}
	l->len -= 1;
	l->list[l->len] = NULL;
}


static int findLowestCost(list_t * const  l){
	int i=0;
	int minIndex=0;
	float min = 1000.0;

	for (i=0; i< l->len; i++){
		if( l->list[i]->current.cost.f < min){
			min = l->list[i]->current.cost.f;
			minIndex=i;
		}

	}
//	while(l->list[i]){
//		if( l->list[i]->cost.f < min){
//			min = l->list[i]->cost.f;
//			minIndex=i;
//		}
//		i++;
//	}

	return minIndex;
}

static int boxEqual(item_t *a, item_t *b){
	int ret=0;
	if((a->box.node.x == b->box.node.x) && (a->box.node.y == b->box.node.y))
		ret=1;
	return ret;
}

static int isWall(grid_t * item){
	return item->current.box.wall;
}

static float square(float a){
	return a*a;
}

static float calculateEuclidian(grid_t * cur, grid_t * dest){
	float ret = 0.0;
	ret =  sqrtf( square(cur->current.box.index.i - dest->current.box.index.i) + square(cur->current.box.index.j - dest->current.box.index.j));
	return ret;
}

static void calculateCost(grid_t * cur, grid_t * dest) {

	if (dest->current.cost.done == 0) {
		dest->current.cost.g = calculateEuclidian(cur, dest)+ cur->current.cost.g;
		dest->current.cost.h = calculateEuclidian(dest, destination);
		dest->current.cost.f = dest->current.cost.g + dest->current.cost.h;
		dest->current.cost.done = 1;
	}
		dest->current.cost.a += calculateEuclidian(dest, destination) + calculateEuclidian(cur, dest);
}

static int findNeighbors(gd_t * gridptr, grid_t *current, list_t *neighbors){

	int i = current->current.box.index.i;
	int j = current->current.box.index.j;

	for(int side = up; side <= upleft; side ++){

		switch(side){
		case up:
			if(j != 0){
				if((!isWall(&(*gridptr)SIDE_UP))){//&&(!inList(cl,&(*gridptr)SIDE_UP))){
//					if((!inList(ol,current)))
							addItemToList(neighbors,&(*gridptr)SIDE_UP);
				}
			}
			break;
		case upright:
			if((j != 0) && (i != GRID_WIDTH-1)){
				if((!isWall(&(*gridptr)SIDE_UPRIGHT))){//&&(!inList(cl,&(*gridptr)SIDE_UPRIGHT))){
//					if((!inList(ol,current)))
					addItemToList(neighbors,&(*gridptr)SIDE_UPRIGHT);
				}
			}
			break;
		case right:
			if(i != GRID_WIDTH-1){
				if((!isWall(&(*gridptr)SIDE_RIGHT))){//&&(!inList(cl,&(*gridptr)SIDE_RIGHT))){
//					if((!inList(ol,current)))
					addItemToList(neighbors,&(*gridptr)SIDE_RIGHT);
				}
			}
			break;
		case downright:
			if((j != GRID_HEIGHT-1) && (i != GRID_WIDTH-1)){
				if((!isWall(&(*gridptr)SIDE_DWRIGHT))){//&&(!inList(cl,&(*gridptr)SIDE_DWRIGHT))){
//					if((!inList(ol,current)))
					addItemToList(neighbors,&(*gridptr)SIDE_DWRIGHT);
				}
			}
			break;
		case down:
			if(j != GRID_HEIGHT-1){
				if((!isWall(&(*gridptr)SIDE_DW))){//&&(!inList(cl,&(*gridptr)SIDE_DW))){
//					if((!inList(ol,current)))
					addItemToList(neighbors,&(*gridptr)SIDE_DW);
				}
			}
			break;
		case downleft:
			if((j != GRID_HEIGHT-1) && (i != 0)){
				if((!isWall(&(*gridptr)SIDE_DWLEFT))){//&&(!inList(cl,&(*gridptr)SIDE_DWLEFT))){
//					if((!inList(ol,current)))
					addItemToList(neighbors,&(*gridptr)SIDE_DWLEFT);
				}
			}
			break;
		case left:
			if(i != 0){
				if((!isWall(&(*gridptr)SIDE_LEFT))){//&&(!inList(cl,&(*gridptr)SIDE_LEFT))){
//					if((!inList(ol,current)))
					addItemToList(neighbors,&(*gridptr)SIDE_LEFT);
				}
			}
			break;
		case upleft:
			if((j != 0) && (i != 0)){
				if((!isWall(&(*gridptr)SIDE_UPLEFT))){//&&(!inList(cl,&(*gridptr)SIDE_UPLEFT))){
//					if((!inList(ol,current)))
					addItemToList(neighbors,&(*gridptr)SIDE_UPLEFT);
				}
			}
			break;
		default:
			return 1;// no neighbors
			break;
		}
	}

	return 0;

}

static int calculate(){
	int l;
	int done=0;


	clearList(nl);


	l = findLowestCost(ol);
	current = ol->list[l];

	if((boxEqual(&ol->list[l]->current.box, &destination->current.box))||(ol->len==0)){
		done=1;

	}
	else{
		addItemToList(cl, current);
		removeItemFromList(ol, current);
		done = findNeighbors(gd, current, nl);

		for (int n = 0; n < nl->len; n++) {
			grid_t * neighbor = nl->list[n];

//			calculateCost(current, nl->list[n]);
//			if ((!inList(ol, nl->list[n])) && (!inList(cl, nl->list[n]))) {
//				addItemToList(ol, nl->list[n]);
//			}

			if (!inList(cl, neighbor)) {
				float tempG = calculateEuclidian(current, neighbor);

				if(inList(ol,neighbor)){
					if(tempG < neighbor->current.cost.g){
						neighbor->current.cost.g = tempG;
					}
				}
				else{
					neighbor->current.cost.g = tempG;
					neighbor->current.cost.h = calculateEuclidian(neighbor, destination);
					neighbor->current.cost.f = neighbor->current.cost.g + neighbor->current.cost.h;
					neighbor->prev = (void*)current;
					addItemToList(ph,current);
					addItemToList(ol,neighbor);
				}

			}

		}
	}
//	drawPath(current);
//	drawList(ph,CGreen);

//	print("----- nl ------\r\n");
//	debug_printNeighbors(nl);  //remove
//	print("----- ol ------\r\n");
//	debug_printNeighbors(ol);  //remove
//	print("----- cl ------\r\n");
//	debug_printNeighbors(cl);  //remove
//	xil_printf("Low.i= %d, Low.j=%d, lowest index=%d, OL size=%d, CL size=%d, size Neighbor= %d ,done=%d\r\n",current->box.index.i,current->box.index.j,l,ol->len,cl->len,nl->len,done);

	return done;
}

static void clearList(list_t * l) {
	for (int i = 0; i < GRID_WIDTH * GRID_HEIGHT; i++) {
		l->list[i] = NULL;
	}
	l->len = 0;
}


static void printCost(gd_t * gridptr) {
	for (int j = 0; j < GRID_HEIGHT; j++) {
		for (int i = 0; i < GRID_WIDTH; i++) {

			if((*gridptr)[i][j].current.box.wall){
				print("------ , ");
			}
			else{
				float f=(*gridptr)[i][j].current.cost.a;
	//			xil_printf("n= %d,	i= %d, j= %d",n,ng->list[n]->current->current->box.index.i,ng->list[n]->current->current->box.index.j);
				printFloat(f);
				print(" , ");

			}
		}
			print("\r\n");
	}

}

//--------------State machine ----------------------------------------

static SM_return SS_stop(ss_params *pvParameters,void * event){

	events *e = (events*) event;
	SM_return ret = state_handled;
	switch (*e) {
	case event_enter:
		print("stopped\r\n");
		Display_flushMem();
		ret = state_handled;
		xTimerChangePeriod(pvParameters-> timerhandle, 400, 0);
		xTimerStart(pvParameters->timerhandle, 0);
		break;
	case hid_start:
		SM_nextState((void*)pvParameters, (void*)SS_calculate);
		ret = state_transition;
		break;
	case hid_select:
		SM_nextState((void*)pvParameters, (void*)SS_init);
		ret = state_transition;
		break;

	case disp_timer_tick:
		SM_nextState((void*)pvParameters, (void*)SS_init);
		ret = state_transition;
		break;
	case event_exit:
//		printCost(gd);
		ret = state_handled;
		break;
	default:
		ret = state_error;
		break;
	}
	return ret;
}

static SM_return SS_draw(ss_params *pvParameters,void * event){

	events *e = (events*) event;
	SM_return ret = state_handled;
	switch (*e) {
	case event_enter:
		BSP_clearLED(3);

		Display_flushMem();

		xTimerStart(pvParameters->timerhandle, 0);
		ret = state_handled;

//		SM_nextState((void*)pvParameters, (void*)SS_stop);
//		ret = state_transition;
		break;
	case disp_timer_tick:
		SM_nextState((void*)pvParameters, (void*)SS_calculate);
		ret = state_transition;
		break;

	case hid_select:
		SM_nextState((void*)pvParameters, (void*)SS_init);
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

static SM_return SS_calculate(ss_params *pvParameters,void * event){

	events *e = (events*) event;
	SM_return ret = state_handled;
	switch (*e) {
	case event_enter:
		BSP_setLED(3);


		if(calculate()){
			SM_nextState((void*)pvParameters, (void*)SS_stop);
		}
		else
		{
			drawList(ol,CRed);
			drawList(cl,CBlue);
			drawPath(current);
			SM_nextState((void*)pvParameters, (void*)SS_draw);
		}

		ret = state_transition;
		break;

	case hid_select:
		SM_nextState((void*)pvParameters, (void*)SS_init);
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

static void blockHalf(gd_t * gridptr) {
	for (int i = 0; i < GRID_WIDTH - 2; i++) {
			(*gridptr)[i][GRID_HEIGHT/2].current.box.wall= 1;
			(*gridptr)[i][GRID_HEIGHT/2].current.box.box.color = CBlack;
	}

	for (int j = 2; j < GRID_HEIGHT - 2; j++) {
			(*gridptr)[GRID_WIDTH/2][j].current.box.wall= 1;
			(*gridptr)[GRID_WIDTH/2][j].current.box.box.color = CBlack;
	}
}

static void initGame(ss_params *pvParameters){
	srand(xTaskGetTickCount());
	print("\r\n------ start -------- \r\n");
	clearList(ol);
	clearList(cl);
	clearList(ph);

	initTimer(pvParameters);
	initGrid(gd);
	blockHalf(gd);


	destination = &grid[0][0];
	destination->current.box.wall = 0;
	start = &grid[GRID_WIDTH-1][GRID_HEIGHT-1];
	start->current.cost = (cost_t){0.0,0,0,1};
	start->current.cost.f = calculateEuclidian(start, destination);
	start->current.box.wall=0;
	addItemToList(ol, start);

}

SM_return SS_init(ss_params *pvParameters,void * event){

	events *e = (events*) event;
	SM_return ret = state_handled;
	switch (*e) {
	case event_init:
		initGame(pvParameters);
		DVI_initDVI();
		SM_nextState((void*)pvParameters, (void*)SS_calculate);
		ret = state_transition;
		break;
	case event_enter:

		initGame(pvParameters);
		SM_nextState((void*)pvParameters, (void*)SS_calculate);
		ret = state_transition;
		break;
	case event_exit:
		drawGrid(gd);
		ret = state_handled;
		break;
	default:
		ret = state_error;
		break;
	}
	return ret;

}

