#include "turing.h"

void init_instr(struct instr * i, int sm, int st, int x, int y) {
	i->symbol = sm;
	i->state = st;
	i->x = x;
	i->y = y;
	return;
}
