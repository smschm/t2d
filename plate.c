#include "turing.h"
#include <stdlib.h>

void plate_create(struct plate *p, int x, int y) {
	p->grid = malloc(x*y*sizeof(int));
	p->x = x;
	p->y = y;
	int i;
	for (i = 0; i < x*y; i++) {
		(p->grid)[i] = 0;
	}
	return;
}

int plate_addr(struct plate *p, int x, int y) {
	return (x * p->y) + y;
}

int plate_get_symbol(struct plate *p, int x, int y) {
	return p->grid[plate_addr(p,x,y)];
}

void plate_set_symbol(struct plate *p, int x, int y, int n) {
	p->grid[plate_addr(p,x,y)] = n;
}

void plate_destroy(struct plate *p) {
	free(p->grid);
	p->grid = NULL;
}

int* plate_col_ptr(struct plate *p, int col) {
	return &(p->grid[col * p->y]);
}
