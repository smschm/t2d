#if defined(_MSC_VER)
#include "SDL.h"
#else
#include "SDL.h"
#endif

// signal flags
#define NOSIGNAL		0
#define QUIT		1
#define NEWMACHINE	2
#define NEWPLATE		3
#define NEWSYMBOLCOUNT	4
#define NEWSTATECOUNT	5
#define RESTART		6
#define REDRAW		7
#define TOGGLEJUMP	8
#define ERASE		9
#define PAUSE		10
#define TOGGLEFFT		11
#define MUTATE	12

#define BYTES_PER_SAMPLE	2  // 2 channels x 16 bit
#define CHANNELS			1
#define BUFFER_SIZE		512

// UI signal
struct signal {
	int flag;
	int data;
};

// instructions
struct instr {
	int symbol;
	int state;
	int x, y;
};

void init_instr(struct instr * i, int sm, int st, int x, int y);

// machines
struct machine {
	int state;
	struct instr* ruleset;
	int rule_count;
	int states;
};

int machine_addr(struct machine *m, int symbol, int state);
void machine_init(struct machine *m, int mx, int my, int x_off, int y_off, int syms, int sts);
void machine_mutate_sym(struct machine *m, int syms, int sts);
void machine_eraser(struct machine *m, int syms);

struct instr machine_get_instr(struct machine *m, int symbol, int state);
void machine_destroy(struct machine *m);

// plates
struct plate {
	int* grid;
	int x;
	int y;
};

void plate_create(struct plate *p, int x, int y);
int plate_addr(struct plate *p, int x, int y);
int plate_get_symbol(struct plate *p, int x, int y);
void plate_set_symbol(struct plate *p, int x, int y, int n);
void plate_destroy(struct plate *p);
int* plate_col_ptr(struct plate *p, int col);

// from turing.c
void new_machine(int symbols, int states);
void get_instr();
int step();

// graphics phun
void pretty_init(int x, int y, int bs, struct plate *p);
void pretty_redraw(int mx, int my, int sym_bits, struct plate *p);
void pretty_update(int x, int y, int mx, int my, struct plate *p);
void pretty_status(int s);
void pretty_shutdown();
struct signal pretty_check_signal();

// buffer stuff
typedef signed short Sample;

void buffer_init();
int buffer_add_bits(int value, int n_bits);
int buffer_add_value(Sample value);
void fill_audio(void *udata, Uint8 *stream, int len);
int buffer_fill_plate(struct plate *p, int bpc);
int buffer_is_full();

// sound butts
void noisy_init();
void noisy_shutdown();

// FFT!
struct complex {
	double real;
	double imag;
};
void cplx_mult(struct complex *a, struct complex *b, struct complex *res);
void burrus_fft_ip(struct complex* data, int bits);
void burrus_unscramble(struct complex* data, int bits);
void steve_unscramble(struct complex* data, int bits);
void int2complex(int * from, struct complex * to, int n);
double magnitude(struct complex z);
void naive_dft(struct complex* data, int bits);
void fill_tables(int len);
void free_tables();

void pretty_fft_draw(int mx, int my, int bits, struct plate *p);
