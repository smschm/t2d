#include "turing.h"
#include <stdio.h>
#include <time.h>

struct plate p;
struct machine m;
int x, y, steps, m_x, m_y; // width, height, steps, machine x, y
int x_bits, y_bits;
int samples;

//flags
int jump;
int pause;
int fft;

int cur_symbol;
int cur_state;
struct instr cur_instr;

int eraser_countdown;  // steps left to erase plate for erase machine
int incoherency;		// maximum jump, "non-straight-lined-ness"
struct signal ui_signal;

int main(int argc, char** argv) {

	int symbol_bits = 4;
	int symbol_count = 16;
	int state_count = 1;
	
	int step_count = 0;
	srand(time(0));
	steps = 0;
	ui_signal.flag = 0;
	samples = 0;
	jump = 0;
	fft = 0;
	eraser_countdown = 0;
	pause = 0;
	incoherency = 1;
	
	int sample = 0;
	
	
	x_bits = 4; y_bits = 4;
	x = 1 << x_bits;
	y = 1 << x_bits;
	m_x = x >> 1;
	m_y = y >> 1;
	
	fill_tables(x*y);
	plate_create(&p, x, y);
	new_machine(symbol_count, state_count);
	pretty_init(x, y, 32, &p);
	pretty_redraw(x, y, symbol_bits, &p);
	noisy_init();
	
	while (ui_signal.flag != QUIT) {
	
		/*while (buffer_is_full()) {
			usleep(10);
		}*/
		SDL_LockAudio();
		
		if ( buffer_fill_plate(&p, symbol_bits) < 0 ) {
			//printf("full\n");
		}
		SDL_UnlockAudio();
		
		if (!pause) {
			sample = step();
			/*SDL_LockAudio();
			buffer_add_bits(sample,symbol_bits);
			SDL_UnlockAudio();*/
			step_count++;
			if (!(step_count % 1000)) {
				#ifdef MMUTATE
				machine_mutate_sym(&m,symbol_count,state_count);
				#endif
				printf("%d steps\n",step_count);
			}
			usleep(1);
		}
		
		if (eraser_countdown > 0) {
			eraser_countdown--;
			if (!eraser_countdown) {
				machine_destroy(&m);
				new_machine(symbol_count, state_count);
			}
		}
		
		if (!(steps%16)) {
			if (fft)
				pretty_fft_draw(x, y, x_bits + y_bits, &p);
			else
				pretty_redraw(x,y, symbol_bits, &p);
		}
		
		ui_signal = pretty_check_signal();
		/*
		if (!(steps%16384)) {
			ui_signal.flag = NEWMACHINE;
		}
		if (!(steps%65536)) {
			ui_signal.flag = NEWPLATE;
			//incoherency++;
			//printf("incoherency: %i\n", incoherency);
		}
		*/
		switch (ui_signal.flag) {
		case NEWSYMBOLCOUNT:
			symbol_bits = ui_signal.data + 1;
			symbol_count = 1 << symbol_bits;
			plate_destroy(&p);
			plate_create(&p, x, y);
			machine_destroy(&m);
			new_machine(symbol_count, state_count);
			pause = 0;
			break;
			
		case NEWSTATECOUNT:
			state_count = 1 << ui_signal.data;
			machine_destroy(&m);
			new_machine(symbol_count, state_count);
			pause = 0;
			break;
			
		case NEWPLATE:
			plate_destroy(&p);
			plate_create(&p, x, y);
			pause = 0;
			break;
			
		case NEWMACHINE:
			machine_destroy(&m);
			new_machine(symbol_count, state_count);
			pause = 0;
			break;
			
		case TOGGLEJUMP:
			jump = !jump;
			machine_destroy(&m);
			new_machine(symbol_count, state_count);
			pause = 0;
			break;
		
		case RESTART:
			plate_destroy(&p);
			plate_create(&p, x, y);
			machine_destroy(&m);
			new_machine(symbol_count, state_count);
			pause = 0;
			break;
			
		case ERASE:
			machine_destroy(&m);
			machine_eraser(&m, symbol_count);
			eraser_countdown = x*y+1;
			break;
		
		case PAUSE:
			pause = !pause;
			break;
			
		case TOGGLEFFT:
			fft = !fft;
			if (fft)
				pretty_fft_draw(x, y, x_bits + y_bits, &p);
			else
				pretty_redraw(x,y, symbol_bits, &p);
			break;
			
		case MUTATE:
			machine_mutate_sym(&m,symbol_count,state_count);
			break;

		default:
			break;
		}
	}
	
	plate_destroy(&p);
	machine_destroy(&m);
	
	noisy_shutdown();
	pretty_shutdown();
	free_tables();
}

void new_machine(int symbols, int states) {
	//m_x = rand() % 5;
	//m_y = rand() % 5;
	machine_init(&m, jump?x:(2*incoherency+1), jump?y:(2*incoherency+1),
		-incoherency, -incoherency, symbols, states);
	printf("%i %i\n",symbols, states);
}

void get_instr() {
	cur_symbol = plate_get_symbol(&p, m_x, m_y);
	cur_state = m.state;
	cur_instr = machine_get_instr(&m, cur_symbol, cur_state);
}


int step() {
	//SDL_Delay(100);
	get_instr();
	plate_set_symbol(&p, m_x, m_y, cur_instr.symbol);
	m_x = (m_x + cur_instr.x + x) % x;
	m_y = (m_y + cur_instr.y + y) % y;
	m.state = cur_instr.state;
	steps++;
	return cur_instr.symbol;
}
/***/
/* test step() for fft testing
void step() {
	int i, j;
	for (i = 0; i < x; i++) {
		for (j = 0; j < y; j++) {
			plate_set_symbol(&p, i, j, 15*((i)%2));
		}
	}
	steps += 256;
}
/***/
