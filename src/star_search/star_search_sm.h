#ifndef STAR_SEARCH_SM_H
#define STAR_SEARCH_SM_H

#include "../sm/statemachine.h"
#include "../display/dvi_vdma.h"
#include "../bsp/bsp.h"

#include <stdlib.h>
#include "../display/dvi_vdma.h"
#include "../display/GFX.h"
#include <math.h>
#include <string.h>
#include "../hcd/hcd.h"

#define GRID_WIDTH (16*4)
#define GRID_HEIGHT (12*4)
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
}cost_t;

typedef struct{
	int16_t x;
	int16_t y;
}node_t;

typedef struct{
	node_t node;
	int16_t h;
	int16_t w;
	int color;
}box_t;

typedef struct{
	box_t margin;
	box_t box;
	index_t index;
	int wall;
}item_t;

typedef struct gr{
	item_t spot;
	cost_t cost;
	struct gr* prev;
}grid_t;

typedef struct{
	int len;
	int max_size;
	grid_t **list;// [GRID_WIDTH*GRID_HEIGHT];
}list_t;

typedef grid_t gd_t[GRID_WIDTH][GRID_HEIGHT];

typedef struct{
	gd_t * gd;
	list_t * ol;
	list_t * cl;
	list_t * ph;
	list_t * nl;
	grid_t * start;
	grid_t * destination;
	grid_t * current;

}star_vars;

typedef struct {
	SM_params statemachine;
	TimerHandle_t timerhandle;
	TimerCallbackFunction_t timerCallbackFunc;
	star_vars svars;

} ss_params;

SM_return SS_init(ss_params *pvParameters,void * event);


grid_t* findLowestCost(list_t * const  l);
int findNeighbors(gd_t * gridptr, grid_t *current, list_t *neighbors);
void removeItemFromList(list_t * const l, grid_t *item);
void addItemToList(list_t * const l, grid_t *item);
grid_t* getItemByIndex(list_t * const l, int index);
int inList(list_t * l, grid_t * item);
void clearList(list_t * l);
float calculateEuclidian(grid_t * cur, grid_t * dest);


#endif
