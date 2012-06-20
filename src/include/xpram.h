/*
 *  xpram.h - XPRAM handling
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
#ifndef XPRAM_H
#define XPRAM_H


#include "sysdeps.h"


#if POWERPC_ROM
const int XPRAM_SIZE = 8192;
#else
const int XPRAM_SIZE = 256;
#endif


// Alias for PRAM data access
#define XPRAM gMacPRAM->fPRAM


class MacPRAM {
public:
							MacPRAM(const char*);
							~MacPRAM();

			void			Load();
			void			Save();
			void			Zap();

			uint8			fPRAM[XPRAM_SIZE];
private:
			char			fPRAMFile;
};


#endif
