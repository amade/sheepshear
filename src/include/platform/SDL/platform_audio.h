/*
 *  platform_audio.h - Audio platform defines
 *
 *  SheepShear, 2012 Alexander von Gluck IV
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
#ifndef _PLATFORM_AUDIO_H
#define _PLATFORM_AUDIO_H


#include "audio.h"
#include "audio_defs.h"


class PlatformAudio
{
public:
		void				DeviceInit();
		void				DeviceShutdown();
		void				DeviceInterrupt();
		bool				DeviceOpen();
		bool				DeviceClose();

		void				Stream(void *arg, uint8 *stream, int stream_len);

protected:
		bool				GetMainMute();
		void				SetMainMute(bool mute);
		uint32				GetMainVolume();
		void                SetMainVolume(uint32 vol);
		bool                GetSpeakerMute();
		void                SetSpeakerMute(bool mute);
		uint32              GetSpeakerVolume();
		void                SetSpeakerVolume(uint32 vol);

		struct audio_status fAudioStatus;
		bool				fAudioOpen;
		int					fSampleRateIndex;
		int					fSampleSizeIndex;
		int					fChannelCountIndex;

private:
		bool				DeviceOpenDSP(void);
		bool				DeviceOpenESD(void);
};


#endif /* _PLATFORM_AUDIO_H */
