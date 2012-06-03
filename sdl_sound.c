#include "turing.h"
#include <stdlib.h>
#if defined(_MSC_VER)
#include "SDL.h"
#else
#include "SDL.h"
#endif

void noisy_init() {
	SDL_AudioSpec a_spec;
	
	a_spec.freq = 8000;
	a_spec.format = AUDIO_S16;
	a_spec.channels = CHANNELS;
	a_spec.samples = BUFFER_SIZE / BYTES_PER_SAMPLE / CHANNELS;
	a_spec.callback = fill_audio;
	a_spec.userdata = NULL;
	
	if ( SDL_OpenAudio(&a_spec, NULL) < 0 ) {
		fprintf(stderr, "Couldn't open audio: %s\n", SDL_GetError());
		exit(0);
	}
	
	SDL_PauseAudio(0);
}

void noisy_shutdown() {
	SDL_CloseAudio();
}
