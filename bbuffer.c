#include "turing.h"
#include <stdlib.h>
#include <fcntl.h>
#if defined(_MSC_VER)
#include "SDL.h"
#else
#include "SDL.h"
#endif


// sample buffer
Uint8 buffer[BUFFER_SIZE];
int buffer_top;

// bit buffer
int bit_top;
int bitbuffer;

int fd;


void buffer_init() {
	buffer_top = 0;
	bit_top = 0;
	bitbuffer = 0;
	#ifdef WRITEOUT
	fd = open("./samples.raw",O_WRONLY | O_CREAT);
	#endif
	printf("fd = %d\n",fd);
	//assert(fd > 0);
}

int buffer_add_bits(int value, int n_bits) {
	//printf("adding %d bit %d\n",n_bits,value);
	int success;
	bitbuffer <<= n_bits;
	bitbuffer += value;
	bit_top += n_bits;
	if (bit_top >= 8) {
		success = buffer_add_value(bitbuffer);
		bit_top = 0;
		bitbuffer = 0;
	}
	return success;
}

int buffer_add_value(Sample value) {
	if (buffer_top < BUFFER_SIZE) {
		buffer[buffer_top++] = value;
		return 0;
	}
	else {
		return -1;
	}
}

int buffer_is_full() {
	return (buffer_top >= BUFFER_SIZE);
}

void fill_audio(void *udata, Uint8 *stream, int len) {
	// if the available buffer length can't handle the current byte buffer,
	// we're screwed, and I don't want to deal with this case
	int fill_len;
	printf("fill audio requests %d, have %d\n",len,buffer_top);
	if (len < buffer_top) {
		fprintf(stderr, "SDL Audio buffer can't handle byte buffer =[\n");
		fprintf(stderr, "len = %i, buffer_len = %i\n", len, buffer_top);
		fill_len = len;	
	}
	else {
		fill_len = buffer_top;
	}
	SDL_MixAudio(stream, (Uint8*)buffer, fill_len, SDL_MIX_MAXVOLUME);
	#ifdef WRITEOUT
	write(fd,buffer,fill_len);
	#endif
	buffer_top = 0;
}

int buffer_fill_plate(struct plate *p, int bpc) {
	int i, j, s;
	Sample current;
	s = 0;
	for (i = 0; i < p->x; i++) {
		for (j = 0; j < p->y; j++) {
			 s |= buffer_add_bits((p->grid)[plate_addr(p,i,j)], bpc);
			// current = (p->grid)[plate_addr(p,i,j)];
			//s |= buffer_add_value(current << (12 - bpc));
		}
	}
	return s;
}
