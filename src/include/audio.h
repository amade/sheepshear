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


#include "audio_defs.h"
#include "platform_audio.h"

#include <vector>


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

extern int audio_frames_per_block;		// Number of audio frames per block
extern uint32 audio_component_flags;	// Component feature flags

extern vector<uint32> audio_sample_rates;	// Vector of supported sample rates (16.16 fixed point)
extern vector<uint16> audio_sample_sizes;	// Vector of supported sample sizes
extern vector<uint16> audio_channel_counts;	// Array of supported channels counts

extern uint32 audio_data;		// Mac address of global data area


#endif
