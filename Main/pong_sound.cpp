#include "load_entire_file.h"
#include <Windows.h>
#include <dsound.h>
#include <mmreg.h>
#include "log.h"
#include <stdio.h>

#include "pong_sound.h"

struct playing_sound {
	Sound_Effect* sound_effect;
	int64_t current_sample;
};


#define concurrently_playable_sounds 16
playing_sound currently_playing_sounds[concurrently_playable_sounds];

void play_sound(Sound_Effect* sound_effect) {
	for (int i = 0; i < concurrently_playable_sounds; i++) {
		if (!currently_playing_sounds[i].sound_effect) {
			currently_playing_sounds[i].sound_effect = sound_effect;
			currently_playing_sounds[i].current_sample = 0;
			return;
		}
	}

	log_error("Keine weiteren Soundeffekte abspielbar");
	return;
}

#if 1

#define print_value(v) _Generic((v), \
	int8_t: printf("%hhd", v), \
	int16_t: printf("%hd", v), \
	int32_t: printf("%d", v),  \
	int64_t: printf("%lld", v) \
)

bool _expect(string& file, string expected_value, const char* path, const char* error_msg) {
	if (file.length < expected_value.length)
		return false;
	if (!(string{ expected_value.length, file.data } == expected_value)) {
		log_error("'%s' %s expected '%.*s' got '%.*s'\n", path, error_msg, expected_value.length, expected_value.data, expected_value.length, file.data);
		return false;
	}
	advance(file, expected_value.length);
	return true;
}
 
template<typename T>
bool _expect(string &file, T expected_value, const char *path, const char *error_msg) {
	if (file.length < sizeof(T))
		return false;
	if (*(T*)file.data != expected_value) {
		printf("'%s' %s expected '", path, error_msg);
		print_value(expected_value);
		printf("' got '");
		print_value(*(T*)file.data);
		printf("'\n");
		return false;
	}

	advance(file, sizeof(T));
	return true;
}

#define expect(expected_value, error_msg) if(!_expect(file, expected_value, path, error_msg)) return{0, 0}; 

Sound_Effect load_sound_effect(const char* path) {
	auto file = load_entire_file(path);
	auto file_start = file;


	if (!file.length) {
		log_error("'%s' Datei konnte nicht geladen werden", path);
		return { 0, 0 };
	}

	if (file.length < 36) {
		log_error("'%s' Datei ist zu klein", path);
		return { 0, 0 };
	}

	expect("RIFF"_s, "RIFF");

	advance(file, 4);

	expect("WAVE"_s, "WAVE header corrupted");

	expect("fmt "_s, "fmt chunk corrupted");

	expect((int32_t)16, "fmt Chunklength corrupted");

	auto format = *(WAVEFORMATEX*)file.data;
	advance(file, 16);

	expect("data"_s, "data chunk corrupted");

	auto data_length = *(int32_t*)file.data;
	advance(file, 4);

	if (!(format.nChannels == 1 || format.nChannels == 2)) {
		log_error("'%s' Nicht unterstuetzte Channelanzahl, '1' oder '2' erwartet, '%hd' erhalten", path, format.nChannels);
		return { 0, 0 };
	}

	if (format.wFormatTag != 1) {
		log_error("'%s' Falscher Formattag, '1' erwartet, '%hd' erhalten (siehe Google)", path, format.wFormatTag);
		return { 0, 0 };
	}

	if (format.nSamplesPerSec != 48000) {
		log_error("'%s' Falsche SamplesProSekunde, '48000' erwartet, '%d' erhalten", path, format.nSamplesPerSec);
		return { 0, 0 };
	}

	if (format.wBitsPerSample != 16) {
		log_error("'%s' Falsche BitsProSample '16' erwartet, '%hd' erhalten", path, format.wBitsPerSample);
		return { 0, 0 };
	}

	auto expected_block_align = format.wBitsPerSample / 8 * format.nChannels;

	if (format.nBlockAlign != expected_block_align) {
		log_error("'%s' Falscher BlockAlign '%d' erwartet, '%hd' erhalten", path, expected_block_align, format.nBlockAlign);
		return { 0, 0 };
	}

	int64_t num_samples = data_length / (format.wBitsPerSample / 8);
	if (format.nChannels == 2)
		num_samples /= 2;

	float* samples = (float*)malloc(num_samples * sizeof(float) * 2);

	if (!samples) {
		log_error("'%s' malloc failed", path);
		return { 0, 0 };
	}

	if (format.nChannels == 1) {
		for (int i = 0; i < num_samples; i++) {
			samples[i * 2 + 0] = (*(int16_t*)file.data) / 32767.0f;
			samples[i * 2 + 1] = (*(int16_t*)file.data) / 32767.0f;
			advance(file, 2);
		}
	} else {
		for (int i = 0; i < num_samples; i++) {
			samples[i * 2 + 0] = (*(int16_t*)file.data) / 32767.0f;
			advance(file, 2);
			samples[i * 2 + 1] = (*(int16_t*)file.data) / 32767.0f;
			advance(file, 2);
		}
	}

	free(file_start.data);
	return { num_samples, samples };
}


#else
Sound_Effect load_sound_effect(const char* path) {
	auto file = load_entire_file(path);
	auto file_start = file;

	if (!file.length) {
		log_error("'%s' Datei konnte nicht geladen werden", path);
		return { 0, 0 };
	}

	if (file.length < 36) {
		log_error("'%s' Datei ist zu klein", path);
		return { 0, 0 };
	}

	if (!starts_with(file, "RIFF")) {
		log_error("'%s' Keine Wave Datei - 'RIFF' erwartet, '%.*s' erhalten", path, min(file.length, 4), file.data);
		return { 0, 0 };
	}
	advance(file, 4);

	advance(file, 4);

	if (!starts_with(file, "WAVE")) {
		log_error("'%s' Keine Wave Datei - 'WAVE' erwartet, '%.*s' erhalten", path, min(file.length, 4), file.data);
		return { 0, 0 };
	}
	advance(file, 4);

	if (!starts_with(file, "fmt ")) {
		log_error("'%s' Keine Wave datei - 'fmt ' erwartet, '%.*s' erhalten", path, min(file.length, 4), file.data);
		return { 0, 0 };
	}
	advance(file, 4);

	if ((*(int32_t*)file.data) != 16) {
		log_error("'%s' fmt Chunklength korrumpiert '16' erwartet, '%d' erhalten", path, *(int32_t *)file.data);
		return { 0, 0 };
	}
	advance(file, 4);

	auto format = *(WAVEFORMATEX*)file.data;
	advance(file, 16);
	
	if (!starts_with(file, "data")) {
		log_error("'%s' Keine Wave Datei - 'data' erwartet, '%.*s' erhalten", path, min(file.length, 4), file.data);
		return { 0, 0 };
	}
	advance(file, 4);

	auto data_length = *(int32_t*)file.data;
	advance(file, 4);

	if (!(format.nChannels == 1 || format.nChannels == 2)) {
		log_error("'%s' Nicht unterstuetzte Channelanzahl, '1' oder '2' erwartet, '%hd' erhalten", path, format.nChannels);
		return { 0, 0 };
	}

	if (format.wFormatTag != 1) {
		log_error("'%s' Falscher Formattag, '1' erwartet, '%hd' erhalten (siehe Google)", path, format.wFormatTag);
		return { 0, 0 };
	}

	if (format.nSamplesPerSec != 48000) {
		log_error("'%s' Falsche SamplesProSekunde, '48000' erwartet, '%d' erhalten", path, format.nSamplesPerSec);
		return { 0, 0 };
	}

	if (format.wBitsPerSample != 16) {
		log_error("'%s' Falsche BitsProSample '16' erwartet, '%hd' erhalten", path, format.wBitsPerSample);
		return { 0, 0 };
	}

	auto expected_block_align = format.wBitsPerSample / 8 * format.nChannels;

	if (format.nBlockAlign != expected_block_align) {
		log_error("'%s' Falscher BlockAlign '%d' erwartet, '%hd' erhalten", path, expected_block_align, format.nBlockAlign);
		return { 0, 0 };
	}

	int64_t num_samples = data_length / (format.wBitsPerSample / 8);
	if (format.nChannels == 2)
		num_samples /= 2;

	float* samples = (float*)malloc(num_samples * sizeof(float) * 2);

	if (!samples) {
		log_error("'%s' malloc failed", path);
		return { 0, 0 };
	}

	if (format.nChannels == 1) {
		for (int i = 0; i < num_samples; i++) {
			samples[i * 2 + 0] = (*(int16_t*)file.data) / 32767.0f;
			samples[i * 2 + 1] = (*(int16_t*)file.data) / 32767.0f;
			advance(file, 2);
		}
	} else {
		for (int i = 0; i < num_samples; i++) {
			samples[i * 2 + 0] = (*(int16_t*)file.data) / 32767.0f;
			advance(file, 2);
			samples[i * 2 + 1] = (*(int16_t*)file.data) / 32767.0f;
			advance(file, 2);
		}
	}

	free(file_start.data);
	return { num_samples, samples };
}
#endif

int SoundThread(void* arg) {
	typedef HRESULT direct_sound_create(LPGUID lpGuid, LPDIRECTSOUND* ppDS, LPUNKNOWN  pUnkOuter);

	HMODULE DSound = LoadLibraryA("dsound.dll");

	if (!DSound) {
		log_error("LoadLibrary dsound.dll failed");
		return 0;
	}

	direct_sound_create* DSoundCreate = (direct_sound_create*)GetProcAddress(DSound, "DirectSoundCreate");
	if (!DSoundCreate) {
		log_error("GetProcAddress of DirectSoundCreate failed");
		return 0;
	}

	//WICHTIG
	LPDIRECTSOUND direct_sound;

	if (DSoundCreate(0, &direct_sound, 0) != DS_OK) {
		log_error("DSoundCreate failed");
		return 0;
	}

	if (direct_sound->SetCooperativeLevel(GetDesktopWindow(), DSSCL_PRIORITY) != DS_OK) {
		log_error("SetCooperationLevel failed");
		return 0;
	}

	DSBUFFERDESC primary_buffer_description = {};
	primary_buffer_description.dwSize = sizeof(DSBUFFERDESC);
	primary_buffer_description.dwFlags = DSBCAPS_PRIMARYBUFFER;

	LPDIRECTSOUNDBUFFER primary_buffer;

	if (direct_sound->CreateSoundBuffer(&primary_buffer_description, &primary_buffer, 0) != DS_OK) {
		log_error("CreatePrimaryBuffer failed");
		return 0;
	}

	WAVEFORMATEX format = {};
	format.wFormatTag = WAVE_FORMAT_IEEE_FLOAT;
	format.nChannels = 2;
	format.nSamplesPerSec = 48000;
	format.wBitsPerSample = 32;
	format.nBlockAlign = (format.nChannels * format.wBitsPerSample) / 8;
	format.nAvgBytesPerSec = format.nBlockAlign * format.nSamplesPerSec;

	auto bytes_per_sample = (format.wBitsPerSample / 8) * format.nChannels;

	if (primary_buffer->SetFormat(&format) != DS_OK) {
		log_error("SetFormat failed");
		return 0;
	}

	DSBUFFERDESC sound_buffer_description = {};
	sound_buffer_description.dwSize = sizeof(DSBUFFERDESC);
	sound_buffer_description.dwBufferBytes = (format.nSamplesPerSec / 20) * bytes_per_sample;
	sound_buffer_description.dwFlags = DSBCAPS_GLOBALFOCUS;
	sound_buffer_description.lpwfxFormat = &format;

	LPDIRECTSOUNDBUFFER sound_buffer;

	if (direct_sound->CreateSoundBuffer(&sound_buffer_description, &sound_buffer, 0) != DS_OK) {
		log_error("CreateSoundBuffer failed");
		return 0;
	}

	if (sound_buffer->Play(0, 0, DSBPLAY_LOOPING) != DS_OK) {
		log_error("play sound_buffer failed");
		return 0;
	}

	float* region_1 = 0;
	DWORD region_1_size = 0;
	float* region_2 = 0;
	DWORD region_2_size = 0;

	DWORD current_play_position;
	DWORD current_write_position;

	int64_t last_written_sample = 0;
	int64_t samples_to_write = 0;

	int64_t buffer_samples = sound_buffer_description.dwBufferBytes / bytes_per_sample;

	while (true) {
		sound_buffer->GetCurrentPosition(&current_play_position, &current_write_position);

		/*if (last_written_sample == current_play_position) {
			Sleep(1);
			continue;
		}

		if (last_written_sample < current_play_position)
			samples_to_write = min((current_play_position - last_written_sample) / (bytes_per_sample * 1), (long long)(samples_to_play_num - current_sample));
		else
			samples_to_write = min((current_play_position + sound_buffer_description.dwBufferBytes - last_written_sample) / (bytes_per_sample * 1), (long long)(samples_to_play_num - current_sample));

		if(!SUCCEEDED(sound_buffer->Lock()*/


		auto current_play_samples = current_play_position / bytes_per_sample;

		if (current_play_samples >= last_written_sample)
			samples_to_write = current_play_samples - last_written_sample;
		else
			samples_to_write = buffer_samples - last_written_sample + current_play_samples;

		if (samples_to_write == 0)
			continue;

		last_written_sample = (last_written_sample + samples_to_write) % buffer_samples;

		if (sound_buffer->Lock(last_written_sample * bytes_per_sample, samples_to_write * bytes_per_sample, (void**)&region_1, &region_1_size, (void**)&region_2, &region_2_size, 0) != DS_OK) {
			log_error("lock sound_buffer failed");
			return 0;
		}

		auto region_1_samples = region_1_size / sizeof(float) / 2;
		auto region_2_samples = region_2_size / sizeof(float) / 2;

		for (int i = 0; i < region_1_samples; i++) {
			float playing_sound_effects_left = 0;
			float playing_sound_effects_right = 0;

			for (int j = 0; j < concurrently_playable_sounds; j++) {
				if (!currently_playing_sounds[j].sound_effect)
					continue;

				if (!currently_playing_sounds[j].sound_effect->num_samples)
					continue;

				playing_sound_effects_left += currently_playing_sounds[j].sound_effect->samples[currently_playing_sounds[j].current_sample * 2 + 0];
				playing_sound_effects_right += currently_playing_sounds[j].sound_effect->samples[currently_playing_sounds[j].current_sample * 2 + 1];
				currently_playing_sounds[j].current_sample++;
				if (currently_playing_sounds[j].current_sample == currently_playing_sounds[j].sound_effect->num_samples) {
					currently_playing_sounds[j].sound_effect = 0;
				}
			}
			region_1[i * 2 + 0] = playing_sound_effects_left;
			region_1[i * 2 + 1] = playing_sound_effects_right;
		}

		for (int i = 0; i < region_2_samples; i++) {
			float playing_sound_effects_left = 0;
			float playing_sound_effects_right = 0;

			for (int j = 0; j < concurrently_playable_sounds; j++) {
				if (!currently_playing_sounds[j].sound_effect)
					continue;

				if (!currently_playing_sounds[j].sound_effect->num_samples)
					continue;

				playing_sound_effects_left += currently_playing_sounds[j].sound_effect->samples[currently_playing_sounds[j].current_sample * 2 + 0];
				playing_sound_effects_right += currently_playing_sounds[j].sound_effect->samples[currently_playing_sounds[j].current_sample * 2 + 1];
				currently_playing_sounds[j].current_sample++;
				if (currently_playing_sounds[j].current_sample == currently_playing_sounds[j].sound_effect->num_samples) {
					currently_playing_sounds[j].sound_effect = 0;
				}
			}
			region_2[i * 2 + 0] = playing_sound_effects_left;
			region_2[i * 2 + 1] = playing_sound_effects_right;
		}



		if (sound_buffer->Unlock(region_1, region_1_size, region_2, region_2_size) != DS_OK) {
			log_error("unlock sound_buffer failed");
			return 0;
		}
	}

	return 0;
}

void init_sound() {
	CreateThread(0, 0, (LPTHREAD_START_ROUTINE)&SoundThread, 0, 0, 0);
}