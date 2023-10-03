#include "star_search_sm.h"


static grid_t grid[GRID_WIDTH][GRID_HEIGHT];
//static list_t openList;
//static list_t closeList;
//static list_t path;
//static list_t neighbors;

static SM_return SS_stop(ss_params *pvParameters,void * event);
static SM_return SS_calculate(ss_params *pvParameters,void * event);
static SM_return SS_draw(ss_params *pvParameters,void * event);

static void vTimerCallback(TimerHandle_t xTimer) {
	events sm_event = disp_timer_tick;
	ss_params * pvParameters = (ss_params*)pvTimerGetTimerID(xTimer);

	xQueueSend(pvParameters->statemachine.stateQueue, &sm_event, (TickType_t ) 0);
}

static void initTimer(ss_params *pvParameters){
	pvParameters->timerCallbackFunc = vTimerCallback;
	pvParameters->timerhandle = xTimerCreate("SM_Timer_display", 1, pdFALSE, pvParameters, pvParameters->timerCallbackFunc);

}


//--------------State machine ----------------------------------------
static void drawList(list_t * lstPtr, int16_t color) {
	if (lstPtr->len > 0) {
		for (int i = 0; i < lstPtr->len; i++) {
			int16_t x = lstPtr->list[i]->spot.box.node.x;
			int16_t y = lstPtr->list[i]->spot.box.node.y;
			int16_t h = lstPtr->list[i]->spot.box.h;
			int16_t w = lstPtr->list[i]->spot.box.w;

			Display_fillRect(x, y, w, h, color);
		}
	}
}

static void drawPath(list_t *ph, grid_t *cur){
	clearList(ph);
	addItemToList(ph,cur);
	grid_t * temp = cur;
	while(temp->prev){
		addItemToList(ph,temp);
		temp = (grid_t *)temp->prev;
	}
	drawList(ph,CGreen);
}

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
		list_t *ol = pvParameters->svars.ol;
		list_t *cl = pvParameters->svars.cl;
		list_t *ph = pvParameters->svars.ph;
		grid_t * current = pvParameters->svars.current;
		grid_t * start = pvParameters->svars.start;
		grid_t * end = pvParameters->svars.destination;
		BSP_clearLED(3);

		drawList(ol,CRed);
		drawList(cl,CBlue);

		Display_fillRect(start->spot.box.node.x, start->spot.box.node.y, start->spot.box.w, start->spot.box.h, CYellow);
		Display_fillRect(end->spot.box.node.x, end->spot.box.node.y, end->spot.box.w, end->spot.box.h, CYellow);

		drawPath(ph, current);


		Display_flushMem();

		xTimerStart(pvParameters->timerhandle, 0);
		ret = state_handled;
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

static int calc_Astar(star_vars *sv) {
	int done = 0;
	list_t *nl = sv->nl;
	list_t *ol = sv->ol;
	list_t *cl = sv->cl;
	grid_t *current;
	grid_t *destination = sv->destination;
	gd_t * gd = sv->gd;

	clearList(nl);

	sv->current= findLowestCost(ol);
	current = sv->current;

	if ((current == destination) || (ol->len == 0)) {
		done = 1;
	} else {
		addItemToList(cl, current);
		removeItemFromList(ol, current);
		done = findNeighbors(gd, current, nl);

		for (int n = 0; n < nl->len; n++) {
			grid_t * neighbor = getItemByIndex(nl, n);

			if (!inList(cl, neighbor)) {
				float tempG = calculateHeuristic(current, neighbor);

				if (inList(ol, neighbor)) {
					if (tempG < neighbor->cost.g) {
						neighbor->cost.g = tempG;
					}
				} else {
					neighbor->cost.g = tempG;
					neighbor->cost.h = calculateHeuristic(neighbor,	destination);
					neighbor->cost.f = neighbor->cost.g	+ neighbor->cost.h;
					neighbor->prev = (grid_t*) current;
					addItemToList(ol, neighbor);
				}

			}

		}
	}
	return done;
}

static SM_return SS_calculate(ss_params *pvParameters,void * event){

	events *e = (events*) event;
	SM_return ret = state_handled;
	switch (*e) {
	case event_enter:
		BSP_setLED(3);

		if(calc_Astar(&pvParameters->svars)){
			SM_nextState((void*)pvParameters, (void*)SS_stop);
		}
		else
		{
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

static void drawBox(gd_t * gridptr, int i, int j){
	int16_t x = (*gridptr)[i][j].spot.margin.node.x;
	int16_t y = (*gridptr)[i][j].spot.margin.node.y;
	int16_t h = (*gridptr)[i][j].spot.margin.h;
	int16_t w = (*gridptr)[i][j].spot.margin.w;
	int16_t c = (*gridptr)[i][j].spot.margin.color;
	Display_drawRect(x, y, w, h, c);

	x = (*gridptr)[i][j].spot.box.node.x;
	y = (*gridptr)[i][j].spot.box.node.y;
	h = (*gridptr)[i][j].spot.box.h;
	w = (*gridptr)[i][j].spot.box.w;
	c = (*gridptr)[i][j].spot.box.color;
	Display_fillRect(x, y, w, h, c);
}

static void drawGrid(gd_t * gridptr){
	for(int i = 0; i < GRID_WIDTH; i++){
		for(int j = 0; j < GRID_HEIGHT; j++){
			drawBox(gridptr,i,j);
		}
	}
}

static void blockHalf(gd_t * gridptr) {
	for (int i = 0; i < GRID_WIDTH - 2; i++) {
			(*gridptr)[i][GRID_HEIGHT/2].spot.wall= 1;
			(*gridptr)[i][GRID_HEIGHT/2].spot.box.color = CBlack;
	}

	for (int j = 2; j < GRID_HEIGHT - 2; j++) {
			(*gridptr)[GRID_WIDTH/2][j].spot.wall= 1;
			(*gridptr)[GRID_WIDTH/2][j].spot.box.color = CBlack;
	}
}


static void initGrid(gd_t * gridptr){

	int wall=0;
	int color;

	for(int i = 0; i < GRID_WIDTH; i++){
		for(int j = 0; j < GRID_HEIGHT; j++){

			(*gridptr)[i][j].prev = NULL;
			(*gridptr)[i][j].cost = (cost_t){0.0,0.0,0.0};
			(*gridptr)[i][j].spot.margin.node.x = i * GRID_BOX_WIDTH;
			(*gridptr)[i][j].spot.margin.node.y = j * GRID_BOX_HEIGHT;
			(*gridptr)[i][j].spot.margin.h = GRID_BOX_HEIGHT+0;
			(*gridptr)[i][j].spot.margin.w = GRID_BOX_WIDTH+0;
			(*gridptr)[i][j].spot.margin.color = CWhite;

			(*gridptr)[i][j].spot.index = (index_t){i,j};
			(*gridptr)[i][j].spot.box.node.x = (i * GRID_BOX_WIDTH ) + 1;
			(*gridptr)[i][j].spot.box.node.y = (j * GRID_BOX_HEIGHT) + 1;
			(*gridptr)[i][j].spot.box.h = GRID_BOX_HEIGHT - 1;
			(*gridptr)[i][j].spot.box.w = GRID_BOX_WIDTH - 1;

			if(rand() > RAND_MAX *.8){
				wall=1;
				color = CBlack;
			}
			else{
				wall =0;
				color = CGray;
			}
			(*gridptr)[i][j].spot.wall = wall;
			(*gridptr)[i][j].spot.box.color = color;
		}
	}
}

static void initGame(star_vars *sv){
	grid_t *start = NULL;
	grid_t *destination = NULL;

	srand(xTaskGetTickCount());
	print("\r\n------ start -------- \r\n");
	sv->current = NULL;
	clearList(sv->ol);
	clearList(sv->cl);
	clearList(sv->ph);

	initGrid(sv->gd);
	blockHalf(sv->gd);


	unsigned int i = rand()>>25;
	unsigned int j = rand()>>26;
	if(i>GRID_WIDTH-1)
		i=GRID_WIDTH-1;

	if(j>GRID_HEIGHT-1)
		j=GRID_HEIGHT-1;

	destination = &(*sv->gd)[i][j];
	destination->spot.wall = 0;

//	xil_printf("end i= %d, j=%d\r\n",i,j);

	i = rand()>>25;
	j = rand()>>26;
	if(i>GRID_WIDTH-1)
		i=0;

	if(j>GRID_HEIGHT-1)
		j=0;

//	xil_printf("start i= %d, j=%d\r\n",i,j);

	start = &(*sv->gd)[i][j];//[GRID_WIDTH-1][GRID_HEIGHT-1];
	start->cost = (cost_t){0.0,0.0,0.0};
	start->cost.f = 0;calculateHeuristic(start, destination);
	start->spot.wall=0;
	addItemToList(sv->ol, start);

	sv->start = start;
	sv->destination = destination;


}

static list_t* createList(int size){
	list_t * lst = malloc(sizeof(list_t));
	lst->len= 0;
	lst->max_size = size;
	lst->list = (grid_t**)malloc(sizeof(grid_t)*size);
	return lst;
}

SM_return SS_init(ss_params *pvParameters,void * event){

	events *e = (events*) event;
	SM_return ret = state_handled;
	switch (*e) {
	case event_init:
		pvParameters->svars.gd = &grid;
		pvParameters->svars.cl = createList(GRID_WIDTH*GRID_HEIGHT);//&closeList;
		pvParameters->svars.ol = createList((GRID_WIDTH*GRID_HEIGHT)/4);//&openList;
		pvParameters->svars.ph = createList((GRID_WIDTH*GRID_HEIGHT)/2);//&path;
		pvParameters->svars.nl = createList(8);//&neighbors;

//		pvParameters->svars.cl = &closeList;
//		pvParameters->svars.ol = &openList;
//		pvParameters->svars.ph = &path;
//		pvParameters->svars.nl = &neighbors;
		pvParameters->svars.current = NULL;

		initTimer(pvParameters);
		DVI_initDVI();
	case event_enter:
		SM_nextState((void*)pvParameters, (void*)SS_calculate);
		ret = state_transition;
		break;
	case event_exit:
		xTimerChangePeriod(pvParameters-> timerhandle, 1, 0);
		initGame(&pvParameters->svars);
		drawGrid(pvParameters->svars.gd);
		ret = state_handled;
		break;
	default:
		ret = state_error;
		break;
	}
	return ret;
}

