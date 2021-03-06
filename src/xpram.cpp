/*
 *  xpram.cpp - XPRAM handling
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

/*
 *  SEE ALSO
 *    Inside Macintosh: Operating System Utilities, chapter 7 "Parameter RAM Utilities"
 */


#include <string.h>

#include "sysdeps.h"
#include "xpram.h"


MacPRAM::MacPRAM(const char *vmdir)
{
	// Clear PRAM space
	memset(fPRAM, 0, XPRAM_SIZE);

	if (vmdir != NULL)
		strcpy(&fPRAMFile, vmdir);

	Load();
}


MacPRAM::~MacPRAM()
{
	Save();
}


// Other methods in platform dependant xpram_*.cpp


