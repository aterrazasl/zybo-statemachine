#ifndef QUADTREE_H
#define QUADTREE_H

#include "../sm/statemachine.h"

#define QT_NODE_CAPACITY 4
#define NUM_POINTS 500

typedef struct{
	int16_t x;
	int16_t y;

}point_t;

typedef struct{
	point_t center;
	int16_t w;
	int16_t h;
}boundary_t;

typedef struct{
	int len;
	int max_size;
	point_t *list;
}list_t;

typedef struct qt_t{
	boundary_t boundary;
	list_t * points;
	struct qt_t* nw;
	struct qt_t* ne;
	struct qt_t* sw;
	struct qt_t* se;

}quadtree_t;

typedef struct {
	SM_params statemachine;
	TimerHandle_t timerhandle;
	TimerCallbackFunction_t timerCallbackFunc;
	quadtree_t *qt;

} quadtree_params;

SM_return quadtree_init(quadtree_params *pvParameters,void * event);
quadtree_t* quadtree_create(boundary_t boundary);
int quadtree_insert(quadtree_t* me, point_t p);
void quadtree_destroy(void);
list_t * quadtree_queryRange(quadtree_t* me, list_t * pointsInRange, boundary_t range);


#endif
