/*
 *  audio_sdl.cpp - Audio support, SDL implementation
 *
 *  SheepShear, 2012 Alexander von Gluck IV
 *  Rewritten from Basilisk II (C) 1997-2008 Christian Bauer
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#include "sysdeps.h"
#include "cpu_emulation.h"
#include "main.h"
#include "prefs.h"
#include "user_strings.h"
#include "audio.h"
#include "audio_defs.h"

#include <SDL_mutex.h>
#include <SDL_audio.h>

#define DEBUG 0
#include "debug.h"

#if defined(BINCUE)
#include "bincue_unix.h"
#endif


// Global variables
static SDL_sem *audio_irq_done_sem = NULL;			// Signal from interrupt to streaming thread: data block read
static uint8 silence_byte;							// Byte value to use to fill sound buffers with silence


static void
stream_hook(void *arg, uint8 *stream, int stream_len)
{
	gMacAudio->Stream(arg, stream, stream_len);
}


/*
 *  Initialization
 */
bool
PlatformAudio::DeviceOpen(void)
{
	// SDL supports a variety of twisted little audio formats, all different
	if (audio_sample_sizes.empty()) {
		audio_sample_rates.push_back(11025 << 16);
		audio_sample_rates.push_back(22050 << 16);
		audio_sample_rates.push_back(44100 << 16);
		audio_sample_sizes.push_back(8);
		audio_sample_sizes.push_back(16);
		audio_channel_counts.push_back(1);
		audio_channel_counts.push_back(2);

		// Default to highest supported values
		fSampleRateIndex = audio_sample_rates.size() - 1;
		fSampleSizeIndex = audio_sample_sizes.size() - 1;
		fChannelCountIndex = audio_channel_counts.size() - 1;
	}

	SDL_AudioSpec audio_spec;
	audio_spec.freq = audio_sample_rates[fSampleRateIndex] >> 16;
	audio_spec.format = (audio_sample_sizes[fSampleSizeIndex] == 8)
		? AUDIO_U8 : AUDIO_S16MSB;
	audio_spec.channels = audio_channel_counts[fSampleSizeIndex];
	audio_spec.samples = 4096;
	audio_spec.callback = stream_hook;
	audio_spec.userdata = NULL;

	// Open the audio device, forcing the desired format
	if (SDL_OpenAudio(&audio_spec, NULL) < 0) {
		fprintf(stderr, "WARNING: Cannot open audio: %s\n", SDL_GetError());
		WarningAlert(GetString(STR_NO_AUDIO_WARN));
		return false;
	}

#if defined(BINCUE)
	OpenAudio_bincue(audio_spec.freq, audio_spec.format, audio_spec.channels,
	audio_spec.silence);
#endif

	char driver_name[32];
	printf("Using SDL/%s audio output\n", SDL_AudioDriverName(driver_name, sizeof(driver_name) - 1));
	silence_byte = audio_spec.silence;
	SDL_PauseAudio(0);

	// Sound buffer size = 4096 frames
	audio_frames_per_block = audio_spec.samples;

	// Everything went fine
	fAudioOpen = true;
	return true;
}


void
PlatformAudio::DeviceInit(void)
{
	// Init audio status and feature flags
	fAudioStatus.sample_rate = 44100 << 16;
	fAudioStatus.sample_size = 16;
	fAudioStatus.channels = 2;
	fAudioStatus.mixer = 0;
	fAudioStatus.num_sources = 0;
	audio_component_flags = cmpWantsRegisterMessage | kStereoOut | k16BitOut;

	// Sound disabled in prefs? Then do nothing
	if (PrefsFindBool("nosound"))
		return;

	// Init semaphore
	audio_irq_done_sem = SDL_CreateSemaphore(0);
}


/*
 *  Deinitialization
 */
bool
PlatformAudio::DeviceClose()
{
	// Close audio device
	SDL_CloseAudio();
	fAudioOpen = false;
}


void
PlatformAudio::DeviceShutdown(void)
{
	// Close audio device
	DeviceClose();

	// Delete semaphore
	if (audio_irq_done_sem)
		SDL_DestroySemaphore(audio_irq_done_sem);
}


/*
 *  First source added, start audio stream
 */
void audio_enter_stream()
{
}


/*
 *  Last source removed, stop audio stream
 */
void audio_exit_stream()
{
}


/*
 *  Streaming function
 */
void
PlatformAudio::Stream(void *arg, uint8 *stream, int stream_len)
{
	if (fAudioStatus.num_sources) {
		// Trigger audio interrupt to get new buffer
		D(bug("stream: triggering irq\n"));
		SetInterruptFlag(INTFLAG_AUDIO);
		TriggerInterrupt();
		D(bug("stream: waiting for ack\n"));
		SDL_SemWait(audio_irq_done_sem);
		D(bug("stream: ack received\n"));

		// Get size of audio data
		uint32 apple_stream_info = ReadMacInt32(audio_data + adatStreamInfo);
		if (apple_stream_info) {
			int work_size = ReadMacInt32(apple_stream_info + scd_sampleCount)
				* (fAudioStatus.sample_size >> 3) * fAudioStatus.channels;
			D(bug("stream: work_size %d\n", work_size));
			if (work_size > stream_len)
				work_size = stream_len;
			if (work_size == 0)
				goto silence;

			// Send data to audio device
			Mac2Host_memcpy(stream,
				ReadMacInt32(apple_stream_info + scd_buffer), work_size);
			if (work_size != stream_len) {
				memset((uint8 *)stream + work_size, silence_byte,
					stream_len - work_size);
			}
			D(bug("stream: data written\n"));
		} else
			goto silence;

	} else {

		// Audio not active, play silence
silence: memset(stream, silence_byte, stream_len);
	}
#if defined(BINCUE)
	MixAudio_bincue(stream, stream_len);
#endif
}


/*
 *  MacOS audio interrupt, read next data block
 */
void
PlatformAudio::DeviceInterrupt(void)
{
	D(bug("AudioInterrupt\n"));

	// Get data from apple mixer
	if (fAudioStatus.mixer) {
		M68kRegisters r;
		r.a[0] = audio_data + adatStreamInfo;
		r.a[1] = fAudioStatus.mixer;
		Execute68k(audio_data + adatGetSourceData, &r);
		D(bug(" GetSourceData() returns %08lx\n", r.d[0]));
	} else
		WriteMacInt32(audio_data + adatStreamInfo, 0);

	// Signal stream function
	SDL_SemPost(audio_irq_done_sem);
	D(bug("AudioInterrupt done\n"));
}


/*
 *  Get/set volume controls (volume values received/returned have the left channel
 *  volume in the upper 16 bits and the right channel volume in the lower 16 bits;
 *  both volumes are 8.8 fixed point values with 0x0100 meaning "maximum volume"))
 */
bool audio_get_main_mute(void)
{
	return false;
}

uint32 audio_get_main_volume(void)
{
	return 0x01000100;
}

bool audio_get_speaker_mute(void)
{
	return false;
}

uint32 audio_get_speaker_volume(void)
{
	return 0x01000100;
}

void audio_set_main_mute(bool mute)
{
}

void audio_set_main_volume(uint32 vol)
{
}

void audio_set_speaker_mute(bool mute)
{
}

void audio_set_speaker_volume(uint32 vol)
{
}
