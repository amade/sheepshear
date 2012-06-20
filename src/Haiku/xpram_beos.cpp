/*
 *  xpram_beos.cpp - XPRAM handling, BeOS specific stuff
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


#include <StorageKit.h>
#include <unistd.h>

#include "sysdeps.h"
#include "version.h"
#include "xpram.h"


#define DEBUG 0
#include "debug.h"


// XPRAM file name and path
#if POWERPC_ROM
const char XPRAM_FILE_NAME[] = "NVRAM_data";
#else
const char XPRAM_FILE_NAME[] = "XPRAM_data";
#endif


static status_t
generate_path(BPath* pramFile)
{
	find_directory(B_USER_SETTINGS_DIRECTORY, pramFile, true);
	pramFile->Append(PROGRAM_NAME);
	pramFile->Append(XPRAM_FILE_NAME);
}


/*
 *  Load XPRAM from settings file
 */
void
MacPRAM::Load()
{
	// Construct XPRAM path
	BPath pramFile;
	generate_path(&pramFile);

	D(bug("%s: %s\n", __func__, pramFile.Path()));

	// Load XPRAM from settings file
	int fd;
	if ((fd = open(pramFile.Path(), O_RDONLY)) >= 0) {
		read(fd, fPRAM, XPRAM_SIZE);
		close(fd);
	}
}


/*
 *  Save XPRAM to settings file
 */
void
MacPRAM::Save()
{
	// Construct XPRAM path
	BPath pramFile;
	generate_path(&pramFile);

	D(bug("%s: %s\n", __func__, pramFile.Path()));

	int fd;
	if ((fd = open(pramFile.Path(), O_WRONLY | O_CREAT, 0666)) >= 0) {
		write(fd, fPRAM, XPRAM_SIZE);
		close(fd);
	} else
		bug("%s: Failed to write PRAM to %s\n", __func__, pramFile.Path());
}


/*
 *  Delete PRAM file
 */

void
MacPRAM::Zap(void)
{
	// Construct XPRAM path
	BPath pramFile;
	generate_path(&pramFile);

	D(bug("%s: %s\n", __func__, pramFile.Path()));

	// Delete file
	unlink(pramFile.Path());
}
