#include "star_search_sm.h"
#include <math.h>



void clearList(list_t * l) {
	for (int i = 0; i < l->max_size; i++) {
		l->list[i] = NULL;
	}
	l->len = 0;
}

int inList(list_t * l, grid_t * item){
	int ret = 0;

	for(int i = 0; i < l->len; i++){
		if(l->list[i]  == item){
			ret = 1;
			break;
		}
	}

	return ret;
}

grid_t* getItemByIndex(list_t * const l, int index){
	return l->list[index];
}

void addItemToList(list_t * const l, grid_t *item){
	if (!inList(l, item)) {
		l->list[l->len] = item;
		l->len += 1;
	}
}

void removeItemFromList(list_t * const l, grid_t *item) {

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

static float square(float a){
	return a*a;
}

float calculateEuclidian(grid_t * cur, grid_t * dest){
	float ret = 0.0;
	ret =  sqrtf( square(cur->spot.index.i - dest->spot.index.i) + square(cur->spot.index.j - dest->spot.index.j));
	return ret;
}


static int isWall(grid_t * item){
	return item->spot.wall;
}

int findNeighbors(gd_t * gridptr, grid_t *current, list_t *neighbors){

	int i = current->spot.index.i;
	int j = current->spot.index.j;

	for(int side = up; side <= upleft; side ++){

		switch(side){
		case up:
			if(j != 0){
				if((!isWall(&(*gridptr)SIDE_UP))){
							addItemToList(neighbors,&(*gridptr)SIDE_UP);
				}
			}
			break;
		case upright:
			if((j != 0) && (i != GRID_WIDTH-1)){
				if((!isWall(&(*gridptr)SIDE_UPRIGHT))){
					addItemToList(neighbors,&(*gridptr)SIDE_UPRIGHT);
				}
			}
			break;
		case right:
			if(i != GRID_WIDTH-1){
				if((!isWall(&(*gridptr)SIDE_RIGHT))){
					addItemToList(neighbors,&(*gridptr)SIDE_RIGHT);
				}
			}
			break;
		case downright:
			if((j != GRID_HEIGHT-1) && (i != GRID_WIDTH-1)){
				if((!isWall(&(*gridptr)SIDE_DWRIGHT))){
					addItemToList(neighbors,&(*gridptr)SIDE_DWRIGHT);
				}
			}
			break;
		case down:
			if(j != GRID_HEIGHT-1){
				if((!isWall(&(*gridptr)SIDE_DW))){
					addItemToList(neighbors,&(*gridptr)SIDE_DW);
				}
			}
			break;
		case downleft:
			if((j != GRID_HEIGHT-1) && (i != 0)){
				if((!isWall(&(*gridptr)SIDE_DWLEFT))){
					addItemToList(neighbors,&(*gridptr)SIDE_DWLEFT);
				}
			}
			break;
		case left:
			if(i != 0){
				if((!isWall(&(*gridptr)SIDE_LEFT))){
					addItemToList(neighbors,&(*gridptr)SIDE_LEFT);
				}
			}
			break;
		case upleft:
			if((j != 0) && (i != 0)){
				if((!isWall(&(*gridptr)SIDE_UPLEFT))){
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

grid_t* findLowestCost(list_t * const  l){
	int i=0;
	int minIndex=0;

	float min = l->list[0]->cost.f;

	for (i=0; i< l->len; i++){
		if( l->list[i]->cost.f < min){
			min = l->list[i]->cost.f;
			minIndex=i;
		}
	}
	return l->list[minIndex];
}



//static void printFloat(float fval){
//	int whole, thousandths;
//	whole = fval;
//	thousandths = (fval - whole) * 1000;
//	xil_printf("%02d.%03d", whole, thousandths);
//}


//static void printCost(gd_t * gridptr) {
//	for (int j = 0; j < GRID_HEIGHT; j++) {
//		for (int i = 0; i < GRID_WIDTH; i++) {
//
//			if((*gridptr)[i][j].current.box.wall){
//				print("------ , ");
//			}
//			else{
//				float f=(*gridptr)[i][j].current.cost.a;
//	//			xil_printf("n= %d,	i= %d, j= %d",n,ng->list[n]->current->current->box.index.i,ng->list[n]->current->current->box.index.j);
//				printFloat(f);
//				print(" , ");
//
//			}
//		}
//			print("\r\n");
//	}
//
//}


//static void debug_printNeighbors(list_t *ng){
//
//	xil_printf("size of List = %d\r\n", ng->len);
//	for(int i = 0; i < ng->len; i++){
//		xil_printf("n= %d,	i= %d, j= %d",i,ng->list[i]->current.box.index.i,ng->list[i]->current.box.index.j);
//		print(", f= ");
//		printFloat(ng->list[i]->current.cost.f);
//		print(", g= ");
//		printFloat(ng->list[i]->current.cost.g);
//		print(", h= ");
//		printFloat(ng->list[i]->current.cost.h);
//
//		print("\r\n");
//	}
//}

