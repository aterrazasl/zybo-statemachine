#include "quadtree_sm.h"
#include <stdlib.h>

//Garbage collector, keeps track of the QT created in order to dealloc when needed
//TODO: change later to recursively dealloc the subdivisions
quadtree_t *gc[640*480];
int gc_count = 0;

static void addToGC(quadtree_t *qt){
	gc[gc_count] = qt;
	gc_count++;
}


static quadtree_t * allocateQuadtree(){
	quadtree_t* ret = (quadtree_t*)malloc(sizeof(quadtree_t));
	addToGC(ret);
	return ret;
}

static void clearList(list_t * l) {
//	for (int i = 0; i < l->max_size; i++) {
//		l->list[i] = {0,0};
//	}
	l->len = 0;
}

static int inList(list_t * l, point_t  item){
	int ret = 0;

	for(int i = 0; i < l->len; i++){
		if((l->list[i].x  == item.x)&&(l->list[i].y  == item.y)){
			ret = 1;
			break;
		}
	}

	return ret;
}

static list_t* createList(int size){
	list_t * lst = malloc(sizeof(list_t));
	lst->len= 0;
	lst->max_size = size;
	lst->list = (point_t*)malloc(sizeof(point_t)*size);
	return lst;
}

static void addItemToList(list_t *  l, point_t item){
	if (!inList(l, item)) {
		l->list[l->len] = item;
		l->len += 1;
	}
}

static int inBoundary(boundary_t boundary, point_t p){
	int ret=0;
	if((p.x >= (boundary.center.x - boundary.w))&&(p.x < (boundary.center.x + boundary.w))){
		if((p.y > (boundary.center.y - boundary.h))&&(p.y <= (boundary.center.y + boundary.h))){
			ret = 1;
		}
	}
	return ret;
}

quadtree_t* quadtree_create(boundary_t boundary){
	quadtree_t* qt = allocateQuadtree();

	qt->boundary = boundary;
	qt->points = createList(QT_NODE_CAPACITY);
	qt->nw = NULL;
	qt->ne = NULL;
	qt->sw = NULL;
	qt->se = NULL;
	return qt;
}



static void subdivide(quadtree_t* me){
	boundary_t b = {{0,0},0,0};
	b.center.x = me->boundary.center.x - (me->boundary.w / 2);
	b.center.y = me->boundary.center.y + (me->boundary.h / 2);
	b.h = (me->boundary.h / 2);
	b.w = (me->boundary.w / 2);
	me->nw = quadtree_create(b);

	b.center.x = me->boundary.center.x + (me->boundary.w / 2);
	b.center.y = me->boundary.center.y + (me->boundary.h / 2);
	b.h = (me->boundary.h / 2);
	b.w = (me->boundary.w / 2);
	me->ne = quadtree_create(b);

	b.center.x = me->boundary.center.x - (me->boundary.w / 2);
	b.center.y = me->boundary.center.y - (me->boundary.h / 2);
	b.h = (me->boundary.h / 2);
	b.w = (me->boundary.w / 2);
	me->sw = quadtree_create(b);

	b.center.x = me->boundary.center.x + (me->boundary.w / 2);
	b.center.y = me->boundary.center.y - (me->boundary.h / 2);
	b.h = (me->boundary.h / 2);
	b.w = (me->boundary.w / 2);
	me->se = quadtree_create(b);
}

int quadtree_insert(quadtree_t* me, point_t p){
	if(!inBoundary(me->boundary, p)){
		return 0;
	}

	if((me->points->len < QT_NODE_CAPACITY )&&(me->nw == NULL)){
		addItemToList(me->points, p);
		return 1;
	}

	if(me->nw == NULL){
		subdivide(me);
	}

	if(quadtree_insert(me->nw, p))
		return 1;
	if(quadtree_insert(me->ne, p))
		return 1;
	if(quadtree_insert(me->sw, p))
		return 1;
	if(quadtree_insert(me->se, p))
		return 1;

	return 0;
}

void quadtree_destroy(void){
	quadtree_t * temp = NULL;
	int temp_gc_count=gc_count;
	for(int i = 0; i < temp_gc_count; i++){

		temp = gc[i];
		if(temp->nw){
			free(temp->nw->points->list);
			free(temp->nw->points);
//			free(temp->nw);

			free(temp->ne->points->list);
			free(temp->ne->points);
//			free(temp->ne);

			free(temp->sw->points->list);
			free(temp->sw->points);
//			free(temp->sw);

			free(temp->se->points->list);
			free(temp->se->points);
//			free(temp->se);

		}
			free(temp);
			gc_count--;

	}
//	xil_printf("mem Address qt= 0x%08X, gc_count=%d\r",(uint32_t)gc[0],gc_count);
	gc_count = 0;

}

static int quadtree_intersects(quadtree_t* me, boundary_t range){

	boundary_t b = me->boundary;
	point_t p;

	p.x = b.center.x - b.w;
	p.y = b.center.y + b.h;
	if(inBoundary(range, p)) return 1;

	p.x = b.center.x + b.w;
	p.y = b.center.y + b.h;
	if(inBoundary(range, p)) return 1;

	p.x = b.center.x + b.w;
	p.y = b.center.y - b.h;
	if(inBoundary(range, p)) return 1;

	p.x = b.center.x - b.w;
	p.y = b.center.y - b.h;
	if(inBoundary(range, p)) return 1;



	p.x = range.center.x - range.w;
	p.y = range.center.y + range.h;
	if(inBoundary(b, p)) return 1;

	p.x = range.center.x + range.w;
	p.y = range.center.y + range.h;
	if(inBoundary(b, p)) return 1;

	p.x = range.center.x + range.w;
	p.y = range.center.y - range.h;
	if(inBoundary(b, p)) return 1;

	p.x = range.center.x - range.w;
	p.y = range.center.y - range.h;
	if(inBoundary(b, p)) return 1;

	return 0;

}



list_t * quadtree_queryRange(quadtree_t* me, list_t * pointsInRange, boundary_t range){

	list_t * retpointsInRange = NULL;
	if(pointsInRange == NULL){
		retpointsInRange = createList(640*480);
	}
	else{
		retpointsInRange = pointsInRange;
	}

	if(!quadtree_intersects(me,range)){
		return retpointsInRange;
	}

	for(int i = 0; i < me->points->len; i++){
		if(inBoundary(range,me->points->list[i])){
			addItemToList(retpointsInRange, me->points->list[i]);
		}
	}

	if(!me->nw)
		return retpointsInRange;

	list_t *temp;
	temp = quadtree_queryRange(me->nw,retpointsInRange,range);
	for(int i = 0; i < temp->len; i++){
		addItemToList(retpointsInRange, temp->list[i]);
	}

	temp = quadtree_queryRange(me->ne,retpointsInRange,range);
	for(int i = 0; i < temp->len; i++){
		addItemToList(retpointsInRange, temp->list[i]);
	}

	temp = quadtree_queryRange(me->sw,retpointsInRange,range);
	for(int i = 0; i < temp->len; i++){
		addItemToList(retpointsInRange, temp->list[i]);
	}

	temp= quadtree_queryRange(me->se,retpointsInRange,range);
	for(int i = 0; i < temp->len; i++){
		addItemToList(retpointsInRange, temp->list[i]);
	}


	return retpointsInRange;
}



//// Find all points that appear within a range
//function queryRange(AABB range)
//{
//    // Prepare an array of results
//    Array of XY pointsInRange;
//
//    // Automatically abort if the range does not intersect this quad
//    if (!boundary.intersectsAABB(range))
//        return pointsInRange; // empty list
//
//    // Check objects at this quad level
//    for (int p = 0; p < points.size; p++)
//    {
//        if (range.containsPoint(points[p]))
//            pointsInRange.append(points[p]);
//    }
//
//    // Terminate here, if there are no children
//    if (northWest == null)
//        return pointsInRange;
//
//    // Otherwise, add the points from the children
//    pointsInRange.appendArray(northWest->queryRange(range));
//    pointsInRange.appendArray(northEast->queryRange(range));
//    pointsInRange.appendArray(southWest->queryRange(range));
//    pointsInRange.appendArray(southEast->queryRange(range));
//
//    return pointsInRange;
//}

