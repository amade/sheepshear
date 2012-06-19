/*
 *  audio.h - Audio support
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
#ifndef AUDIO_H
#define AUDIO_H


#include <vector>
#include "platform_audio.h"


#ifndef NO_STD_NAMESPACE
using std::vector;
#endif


class MacAudio : public PlatformAudio
{
public:
							MacAudio();
							~MacAudio();

			bool			Open();
			bool			Close();
			void			Reset();
			void			Interrupt();
			int32			Dispatch(uint32 params, uint32 ti);

			int16			InOpen(uint32 pb, uint32 dce);
			int16			InPrime(uint32 pb, uint32 dce);
			int16			InControl(uint32 pb, uint32 dce);
			int16			InStatus(uint32 pb, uint32 dce);
			int16			InClose(uint32 pb, uint32 dce);

			bool			SetSampleRate(int index);
			bool			SetSampleSize(int index);
			bool			SetChannels(int index);
private:
			int32			GetInfo(uint32 infoPtr, uint32 selector, uint32 sourceID);
			int32			SetInfo(uint32 infoPtr, uint32 selector, uint32 sourceID);
};


extern void audio_enter_stream(void);
extern void audio_exit_stream(void);

extern bool audio_set_sample_rate(int index);
extern bool audio_set_sample_size(int index);
extern bool audio_set_channels(int index);

extern bool audio_get_main_mute(void);
extern uint32 audio_get_main_volume(void);
extern bool audio_get_speaker_mute(void);
extern uint32 audio_get_speaker_volume(void);
extern void audio_set_main_mute(bool mute);
extern void audio_set_main_volume(uint32 vol);
extern void audio_set_speaker_mute(bool mute);
extern void audio_set_speaker_volume(uint32 vol);

extern struct audio_status AudioStatus;

extern bool audio_open;					// Flag: audio is open and ready
extern int audio_frames_per_block;		// Number of audio frames per block
extern uint32 audio_component_flags;	// Component feature flags

extern vector<uint32> audio_sample_rates;	// Vector of supported sample rates (16.16 fixed point)
extern vector<uint16> audio_sample_sizes;	// Vector of supported sample sizes
extern vector<uint16> audio_channel_counts;	// Array of supported channels counts

// Audio component global data and 68k routines
enum {
	adatDelegateCall = 0,		// 68k code to call DelegateCall()
	adatOpenMixer = 14,			// 68k code to call OpenMixerSoundComponent()
	adatCloseMixer = 36,		// 68k code to call CloseMixerSoundComponent()
	adatGetInfo = 54,			// 68k code to call GetInfo()
	adatSetInfo = 78,			// 68k code to call SetInfo()
	adatPlaySourceBuffer = 102,	// 68k code to call PlaySourceBuffer()
	adatGetSourceData = 126,	// 68k code to call GetSourceData()
	adatStartSource = 146,		// 68k code to call StartSource()
	adatData = 168,				// SoundComponentData struct
	adatMixer = 196,			// Mac address of mixer, returned by adatOpenMixer
	adatStreamInfo = 200,		// Mac address of stream info, returned by adatGetSourceData
	SIZEOF_adat = 204
};

extern uint32 audio_data;		// Mac address of global data area

#endif
