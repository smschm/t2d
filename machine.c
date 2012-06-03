#include "turing.h"
#include <stdlib.h>

const int primes[] = {13, 17, 19, 23, 29, 31, 37, 41};
const int n_primes_half = 4;

int machine_addr(struct machine *m, int symbol, int state) {
	return symbol * m->states + state;
}

void machine_init(struct machine *m, int mx, int my, int x_off, int y_off, int syms, int sts) {
	m->state = 0;
	m->rule_count = syms*sts; // symbols * states
	m->ruleset = malloc(sizeof(struct instr)*(m->rule_count));
	m->states = sts;
	
	int i,a,b,c,d;
	struct instr tmp;
	for (i = 0; i < m->rule_count; i++) {
		a = random() % syms;
		b = random() % sts;
		c = random() % mx + x_off;
		d = random() % my + y_off;
		//printf("rule %d: %d %d %d %d\n",i,a,b,c,d);
		init_instr( &tmp,
			a,
			b,
			c,
			d);
		(m->ruleset)[i] = tmp;
	}
}

void machine_mutate_sym(struct machine *m, int syms, int sts) {
	int rc = m->rule_count;
	int rn = random() % rc;
	((m->ruleset)[rn]).symbol = random() % syms;
}

void machine_eraser(struct machine *m, int syms) {
	m->state = 0;
	m->rule_count = syms;
	m->ruleset = malloc(sizeof(struct instr)*(m->rule_count));
	m->states = 1;
	
	int i;
	struct instr tmp;
	for (i = 0; i < m->rule_count; i++) {
		init_instr( &tmp,
			0,
			0,
			13,
			1 );
		(m->ruleset)[i] = tmp;
	}
}
struct instr machine_get_instr(struct machine *m, int symbol, int state) {
	return (m->ruleset)[machine_addr(m,symbol,state)];
}

void machine_destroy(struct machine *m) {
	free(m->ruleset);
	m->ruleset = NULL;
}
