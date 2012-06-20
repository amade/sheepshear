/*
 *  audio_dummy.cpp - Audio support, dummy implementation
 *
 *  Basilisk II (C) 1997-2008 Christian Bauer
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
#include "prefs.h"
#include "audio.h"
#include "audio_defs.h"

#define DEBUG 0
#include "debug.h"


/*
 *  Initialization
 */
void
PlatformAudio::DeviceInit(void)
{
	// Init audio status and feature flags
	AudioStatus.sample_rate = 44100 << 16;
	AudioStatus.sample_size = 16;
	AudioStatus.channels = 2;
	AudioStatus.mixer = 0;
	AudioStatus.num_sources = 0;
	audio_component_flags = cmpWantsRegisterMessage | kStereoOut | k16BitOut;

	// Only one sample format is supported
	audio_sample_rates.push_back(44100 << 16);
	audio_sample_sizes.push_back(16);
	audio_channel_counts.push_back(2);

	// Sound disabled in prefs? Then do nothing
	if (PrefsFindBool("nosound"))
		return;

	// Audio not available
	fAudioOpen = false;
}


/*
 *  Deinitialization
 */
void
PlatformAudio::DeviceShutdown(void)
{
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
 *  MacOS audio interrupt, read next data block
 */
void
PlatformAudio::DeviceInterrupt(void)
{
	D(bug("AudioInterrupt\n"));
}


/*
 *  Get/set volume controls (volume values received/returned have the left channel
 *  volume in the upper 16 bits and the right channel volume in the lower 16 bits;
 *  both volumes are 8.8 fixed point values with 0x0100 meaning "maximum volume"))
 */
uint32
PlatformAudio::GetMainVolume(void)
{
	return 0x01000100;
}


void
PlatformAudio::SetMainVolume(uint32 vol)
{
}


bool
PlatformAudio::GetMainMute(void)
{
	return false;
}


void
PlatformAudio::SetMainMute(bool mute)
{
}


bool
PlatformAudio::GetSpeakerMute(void)
{
	return false;
}


void
PlatformAudio::SetSpeakerMute(bool mute)
{
}


uint32
PlatformAudio::GetSpeakerVolume(void)
{
	return 0x01000100;
}


void
PlatformAudio::SetSpeakerVolume(uint32 vol)
{
}
