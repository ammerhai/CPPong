#pragma once

#include <stdint.h>

struct Sound_Effect {
	int64_t num_samples;
	float* samples;
};

void init_sound();

Sound_Effect load_sound_effect(const char* path);
void play_sound(Sound_Effect* sound_effect);