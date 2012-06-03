#include "turing.h"
#include "peano.h"
#include <stdlib.h>
#if defined(_MSC_VER)
#include "SDL.h"
#else
#include "SDL.h"
#endif

SDL_Surface *screen;

int block_size;
int corner;

int (*get_color)(int symbol); // pointer to a generic color getting function
int (*get_cpx_color)(struct complex z, double factor);
// filled in by get_color_Xbit

int get_color_8bit(int symbol) {
	int ret = 0;
	if (symbol & 0x01) ret += 0x00007f;
	if (symbol & 0x02) ret += 0x007f00;
	if (symbol & 0x04) ret += 0x7f0000;
	if (symbol & 0x08) ret += 0x404040;
	if (symbol & 0x10) ret += 0x202020;
	if (symbol & 0x20) ret += 0x000010;
	if (symbol & 0x40) ret += 0x001000;
	if (symbol & 0x80) ret += 0x100000;
	return ret;
}

int get_color_hot(struct complex z, double factor) {
	double d_mag = magnitude(z);
	int mag = d_mag * factor;
	int ret = 0;
	if (mag < 256) {
		ret = mag << 16;
	}
	else if (mag < 512) {
		ret = 0xff0000 + ((mag-256) << 8);
	}
	else if (mag < 768) {
		ret = 0xffff00 + mag - 512;
	}
	else {
		ret = 0xffffff;
	}
	return ret;
}

int get_color_red(struct complex z, double factor) {
	z.real *= factor;
	z.imag *= factor;
	z.real += 127;
	z.imag += 127;
	unsigned char r_byte, i_byte;
	// both essentially max(0,min(x, 255))
	r_byte = z.real < 0 ? 0 : (z.real > 255 ? 255 : z.real);
	i_byte = z.imag < 0 ? 0 : (z.imag > 255 ? 255 : z.imag);
	return 0x010000 * i_byte + 0x000101 * r_byte;
}

int get_color_4bit(int symbol) {
	int ret = 0;
	if (symbol & 0x01) ret += 0x000099;
	if (symbol & 0x02) ret += 0x009900;
	if (symbol & 0x04) ret += 0x990000;
	if (symbol & 0x08) ret += 0x666666;
	return ret;
}

int get_color_2bit(int symbol) {
	int ret = 0;
	if (symbol & 0x01) ret += 0x3399ff;
	if (symbol & 0x02) ret += 0x996600;
	return ret;
}

int get_color_1bit(int symbol) {
	int ret = 0;
	if (symbol & 0x01) ret += 0xffffff;
	return ret;
}

// stolen function
void putpixel(int x, int y, int color) {
	unsigned int *ptr = (unsigned int*)screen->pixels;
	int lineoffset = y * (screen->pitch / 4);
	ptr[lineoffset + x] = color;
}

void pretty_redraw(int mx, int my, int sym_bits, struct plate *p) {
	switch(sym_bits) {
	case 1:
		get_color = get_color_1bit;
		break;
	case 2:
		get_color = get_color_2bit;
		break;
	case 3: case 4:
		get_color = get_color_4bit;
		break;
	default:
		get_color = get_color_8bit;
		break;
	}
	int i, j;
	for (i = 0; i < mx; i++) {
		for (j = 0; j < my; j++) {
			pretty_update(i,j,mx,my,p);
		}
	}
	SDL_UpdateRect(screen, 0, 0, mx*block_size, my*block_size);
}
 
void pretty_update(int x, int y, int mx, int my, struct plate *p) {

	if (SDL_LockSurface(screen) < 0)
		return;

	int k, l;
	point pt;
	
	// pt = z1_to_z2(x*p->y + y, 16, 0);
	pt.x = x;
	pt.y = y;
	
	int temp_s,temp_c;

	for (k = 0; k < block_size; k++) {
		for (l = 0; l < block_size; l++) {
			temp_s = plate_get_symbol(p,x,y);
			temp_c = get_color(temp_s);
			//printf("color: %x symbol: %d\n",temp_c,temp_s);
			putpixel(pt.x*block_size+k,
				pt.y*block_size+l,
				temp_c
			);
		}
	}
	// if (corner) putpixel(x*block_size, y*block_size, 0xff0000);


	if (SDL_MUSTLOCK(screen))
		SDL_UnlockSurface(screen);
}

void pretty_fft_draw(int mx, int my, int bits, struct plate *p) {

	get_cpx_color = get_color_hot;
	
	struct complex * col_fft = malloc(sizeof(struct complex)*(p->y*p->x));
	int i, j, k, l, color;
	
	point peano_p;
	
	int2complex(plate_col_ptr(p, 0), col_fft, p->y*p->x);
	burrus_fft_ip(col_fft, bits); // WATCH OUT - NOT ALWAYS 6 BITS
	
	if (SDL_LockSurface(screen) < 0)
		return;

	
	for (i = 0; i < mx; i++) {
		for (j = 0; j < my; j++) {
			// printf("fft[%i] = %f + %fj\n", i*my+j, col_fft[i*my+j].real, col_fft[i*my+j].imag);
			color = get_cpx_color(col_fft[i*my+j], 1/6.0);
			peano_p = z1_to_z2(i*my+j,bits/2,0);
			for (k = 0; k < block_size; k++) {
				for (l = 0; l < block_size; l++) {
					putpixel(peano_p.x*block_size+k, peano_p.y*block_size+l, color);
				}
			}
		}
	}
	// if (corner) putpixel(x*block_size, y*block_size, 0xff0000);


	if (SDL_MUSTLOCK(screen))
		SDL_UnlockSurface(screen);
		
	free(col_fft);
	
	SDL_UpdateRect(screen, 0, 0, mx*block_size, my*block_size);
}

void pretty_init(int x, int y, int bs, struct plate *p) {
	block_size = bs;
	corner = (bs > 3);
	
	if ( SDL_Init(SDL_INIT_VIDEO) < 0 ) {
		fprintf(stderr, "Unable to init SDL: %s\n", SDL_GetError());
		exit(1);
	}

	atexit(SDL_Quit);
    
	screen = SDL_SetVideoMode(x*bs, y*bs, 32, SDL_SWSURFACE);
    // This title was hilarious when I first wrote it and shall be left in for posterity.
	SDL_WM_SetCaption("Stephen Wolfram\'s Penis", NULL);
    
	if ( screen == NULL ) {
		fprintf(stderr, "Unable to set video: %s\n", SDL_GetError());
		exit(1);
	}
}

struct signal pretty_check_signal() {
	SDL_Event event;
	struct signal ret;
	
	while (SDL_PollEvent(&event)) {
		switch (event.type) {
		case SDL_KEYDOWN:
			break;
		case SDL_KEYUP:
			// If escape is pressed, return (and thus, quit)
			if (event.key.keysym.sym == SDLK_ESCAPE) {
				ret.flag = QUIT;
				return ret;
			}
			else if (event.key.keysym.sym >= '5' && event.key.keysym.sym <= '8') {
				ret.flag = NEWSYMBOLCOUNT;
				ret.data = event.key.keysym.sym - '5';
				return ret;
			}
			else if (event.key.keysym.sym == '9') {
				ret.flag = NEWSYMBOLCOUNT;
				ret.data = 7;
				return ret;
			}
			else if (event.key.keysym.sym >= '1' && event.key.keysym.sym <= '4') {
				ret.flag = NEWSTATECOUNT;
				ret.data = event.key.keysym.sym - '1';
				return ret;
			}
			else if (event.key.keysym.sym == 'n') {
				ret.flag = NEWMACHINE;
				return ret;
			}
			else if (event.key.keysym.sym == 'c') {
				ret.flag = NEWPLATE;
				return ret;
			}
			else if (event.key.keysym.sym == 'j') {
				ret.flag = TOGGLEJUMP;
				return ret;
			}
			else if (event.key.keysym.sym == 'r') {
				ret.flag = RESTART;
				return ret;
			}
			else if (event.key.keysym.sym == 'e') {
				ret.flag = ERASE;
				return ret;
			}
			else if (event.key.keysym.sym == 'p') {
				ret.flag = PAUSE;
				return ret;
			}
			else if (event.key.keysym.sym == 'f') {
				ret.flag = TOGGLEFFT;
				return ret;
			}
			else if (event.key.keysym.sym == 'm') {
				ret.flag = MUTATE;
				return ret;
			}
			break;
		case SDL_QUIT:
			ret.flag = QUIT;
			return ret;
		}
	}
	ret.flag = NOSIGNAL;
	return ret;
}

void pretty_status(int s){
}

void pretty_shutdown() {
}
