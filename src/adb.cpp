/*
 *  adb.cpp - ADB emulation (mouse/keyboard)
 *
 *  SheepShear, 2012 Alexander von Gluck IV
 *  Rewritten from Basilisk II (C) Christian Bauer
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
 *    Inside Macintosh: Devices, chapter 5 "ADB Manager"
 *    Technote HW 01: "ADB - The Untold Story: Space Aliens Ate My Mouse"
 */

#include <stdlib.h>

#include "sysdeps.h"
#include "adb.h"
#include "cpu_emulation.h"
#include "emul_op.h"
#include "main.h"
#include "prefs.h"
#include "video.h"

#ifdef POWERPC_ROM
#include "thunks.h"
#endif

#define DEBUG 0
#include "debug.h"


B2_mutex* gMouseLock;


/*
 *  Initialize ADB emulation
 */
ADBInput::ADBInput()
	:
	fMouseRelative(false),
	fMouseCoordX(0),
	fMouseCoordY(0),
	fMouseOldCoordX(0),
	fMouseOldCoordY(0),
	fKeyboardType(0x05),
	fKeyReadPtr(0),
	fKeyWritePtr(0)
{
	// Populate some initial data
	memset(&fMouseButton, 0, sizeof(fMouseButton));
	memset(&fMouseOldButton, 0, sizeof(fMouseButton));
	fMouseRegister3[0] = 0x63;
	fMouseRegister3[1] = 0x01;
	fKeyRegister2[0] = 0xff;
	fKeyRegister2[1] = 0xff;

	fKeyboardType = (uint8)PrefsFindInt32("keyboardtype");
	fKeyRegister3[0] = 0x62;
	fKeyRegister3[1] = fKeyboardType;

	gMouseLock = B2_create_mutex();
}


/*
 *  Exit ADB emulation
 */
ADBInput::~ADBInput()
{
	if (gMouseLock) {
		B2_delete_mutex(gMouseLock);
		gMouseLock = NULL;
	}
}


/*
 *  ADBOp() replacement
 */
void
ADBInput::Op(uint8 op, uint8 *data)
{
	D(bug("ADBOp op %02x, data %02x %02x %02x\n", op, data[0], data[1], data[2]));

	// ADB reset?
	if ((op & 0x0f) == 0) {
		fMouseRegister3[0] = 0x63;
		fMouseRegister3[1] = 0x01;
		fKeyRegister2[0] = 0xff;
		fKeyRegister2[1] = 0xff;
		fKeyRegister3[0] = 0x62;
		fKeyRegister3[1] = fKeyboardType;
		return;
	}

	// Cut op into fields
	uint8 adr = op >> 4;
	uint8 cmd = (op >> 2) & 3;
	uint8 reg = op & 3;

	// Check which device was addressed and act accordingly
	if (adr == (fMouseRegister3[0] & 0x0f)) {

		// Mouse
		if (cmd == 2) {

			// Listen
			switch (reg) {
				case 3:		// Address/HandlerID
					if (data[2] == 0xfe)			// Change address
						fMouseRegister3[0] = (fMouseRegister3[0] & 0xf0) | (data[1] & 0x0f);
					else if (data[2] == 1 || data[2] == 2 || data[2] == 4)	// Change device handler ID
						fMouseRegister3[1] = data[2];
					else if (data[2] == 0x00)		// Change address and enable bit
						fMouseRegister3[0] = (fMouseRegister3[0] & 0xd0) | (data[1] & 0x2f);
					break;
			}

		} else if (cmd == 3) {

			// Talk
			switch (reg) {
				case 1:		// Extended mouse protocol
					data[0] = 8;
					data[1] = 'a';				// Identifier
					data[2] = 'p';
					data[3] = 'p';
					data[4] = 'l';
					data[5] = 300 >> 8;			// Resolution (dpi)
					data[6] = 300 & 0xff;
					data[7] = 1;				// Class (mouse)
					data[8] = 3;				// Number of buttons
					break;
				case 3:		// Address/HandlerID
					data[0] = 2;
					data[1] = (fMouseRegister3[0] & 0xf0) | (rand() & 0x0f);
					data[2] = fMouseRegister3[1];
					break;
				default:
					data[0] = 0;
					break;
			}
		}
		D(bug(" mouse reg 3 %02x%02x\n", fMouseRegister3[0], fMouseRegister3[1]));

	} else if (adr == (fKeyRegister3[0] & 0x0f)) {

		// Keyboard
		if (cmd == 2) {

			// Listen
			switch (reg) {
				case 2:		// LEDs/Modifiers
					fKeyRegister2[0] = data[1];
					fKeyRegister2[1] = data[2];
					break;
				case 3:		// Address/HandlerID
					if (data[2] == 0xfe)			// Change address
						fKeyRegister3[0] = (fKeyRegister3[0] & 0xf0) | (data[1] & 0x0f);
					else if (data[2] == 0x00)		// Change address and enable bit
						fKeyRegister3[0] = (fKeyRegister3[0] & 0xd0) | (data[1] & 0x2f);
					break;
			}

		} else if (cmd == 3) {

			// Talk
			switch (reg) {
				case 2: {	// LEDs/Modifiers
					uint8 reg2hi = 0xff;
					uint8 reg2lo = fKeyRegister2[1] | 0xf8;
					if (MATRIX(0x6b))	// Scroll Lock
						reg2lo &= ~0x40;
					if (MATRIX(0x47))	// Num Lock
						reg2lo &= ~0x80;
					if (MATRIX(0x37))	// Command
						reg2hi &= ~0x01;
					if (MATRIX(0x3a))	// Option
						reg2hi &= ~0x02;
					if (MATRIX(0x38))	// Shift
						reg2hi &= ~0x04;
					if (MATRIX(0x36))	// Control
						reg2hi &= ~0x08;
					if (MATRIX(0x39))	// Caps Lock
						reg2hi &= ~0x20;
					if (MATRIX(0x75))	// Delete
						reg2hi &= ~0x40;
					data[0] = 2;
					data[1] = reg2hi;
					data[2] = reg2lo;
					break;
				}
				case 3:		// Address/HandlerID
					data[0] = 2;
					data[1] = (fKeyRegister3[0] & 0xf0) | (rand() & 0x0f);
					data[2] = fKeyRegister3[1];
					break;
				default:
					data[0] = 0;
					break;
			}
		}
		D(bug(" keyboard reg 3 %02x%02x\n", fKeyRegister3[0], fKeyRegister3[1]));

	} else												// Unknown address
		if (cmd == 3)
			data[0] = 0;								// Talk: 0 bytes of data
}


/*
 *  Mouse was moved (x/y are absolute or relative, depending on ADBSetRelMouseMode())
 */
void
ADBInput::MouseMoved(int x, int y)
{
	B2_lock_mutex(gMouseLock);
	if (fMouseRelative) {
		fMouseCoordX += x;
		fMouseCoordY += y;
	} else {
		fMouseCoordX = x;
		fMouseCoordY = y;
	}
	B2_unlock_mutex(gMouseLock);
	SetInterruptFlag(INTFLAG_ADB);
	TriggerInterrupt();
}


/* 
 *  Mouse button pressed
 */
void
ADBInput::MouseDown(int button)
{
	fMouseButton[button] = true;
	SetInterruptFlag(INTFLAG_ADB);
	TriggerInterrupt();
}


/*
 *  Mouse button released
 */
void
ADBInput::MouseUp(int button)
{
	fMouseButton[button] = false;
	SetInterruptFlag(INTFLAG_ADB);
	TriggerInterrupt();
}


/*
 *  Set mouse mode (absolute or relative)
 */
void
ADBInput::SetRelMouseMode(bool relative)
{
	if (fMouseRelative != relative) {
		fMouseRelative = relative;
		fMouseCoordX = 0;
		fMouseCoordY = 0;
	}
}


/*
 *  Key pressed ("code" is the Mac key code)
 */
void
ADBInput::KeyDown(int code)
{
	// Add keycode to buffer
	fKeyBuffer[fKeyWritePtr] = code;
	fKeyWritePtr = (fKeyWritePtr + 1) % KEY_BUFFER_SIZE;

	// Set key in matrix
	fKeyStates[code >> 3] |= (1 << (~code & 7));

	// Trigger interrupt
	SetInterruptFlag(INTFLAG_ADB);
	TriggerInterrupt();
}


/*
 *  Key released ("code" is the Mac key code)
 */
void
ADBInput::KeyUp(int code)
{
	// Add keycode to buffer
	fKeyBuffer[fKeyWritePtr] = code | 0x80;	// Key-up flag
	fKeyWritePtr = (fKeyWritePtr + 1) % KEY_BUFFER_SIZE;

	// Clear key in matrix
	fKeyStates[code >> 3] &= ~(1 << (~code & 7));

	// Trigger interrupt
	SetInterruptFlag(INTFLAG_ADB);
	TriggerInterrupt();
}


/*
 *  ADB interrupt function (executed as part of 60Hz interrupt)
 */

void
ADBInput::Interrupt(void)
{
	M68kRegisters r;

	// Return if ADB is not initialized
	uint32 adb_base = ReadMacInt32(0xcf8);
	if (!adb_base || adb_base == 0xffffffff)
		return;
	uint32 tmp_data = adb_base + 0x163;	// Temporary storage for faked ADB data

	// Get mouse state
	B2_lock_mutex(gMouseLock);
	int mx = fMouseCoordX;
	int my = fMouseCoordY;
	if (fMouseRelative) {
		fMouseCoordX = 0;
		fMouseCoordY = 0;
	}
	int mb[3] = {fMouseButton[0], fMouseButton[1], fMouseButton[2]};
	B2_unlock_mutex(gMouseLock);

	uint32 key_base = adb_base + 4;
	uint32 mouse_base = adb_base + 16;

	if (fMouseRelative) {

		// Mouse movement (relative) and buttons
		if (mx != 0 || my != 0
			|| mb[0] != fMouseOldButton[0]
			|| mb[1] != fMouseOldButton[1]
			|| mb[2] != fMouseOldButton[2]) {

			// Call mouse ADB handler
			if (fMouseRegister3[1] == 4) {
				// Extended mouse protocol
				WriteMacInt8(tmp_data, 3);
				WriteMacInt8(tmp_data + 1, (my & 0x7f) | (mb[0] ? 0 : 0x80));
				WriteMacInt8(tmp_data + 2, (mx & 0x7f) | (mb[1] ? 0 : 0x80));
				WriteMacInt8(tmp_data + 3,
					((my >> 3) & 0x70) | ((mx >> 7) & 0x07) | (mb[2] ? 0x08 : 0x88));
			} else {
				// 100/200 dpi mode
				WriteMacInt8(tmp_data, 2);
				WriteMacInt8(tmp_data + 1, (my & 0x7f) | (mb[0] ? 0 : 0x80));
				WriteMacInt8(tmp_data + 2, (mx & 0x7f) | (mb[1] ? 0 : 0x80));
			}	
			r.a[0] = tmp_data;
			r.a[1] = ReadMacInt32(mouse_base);
			r.a[2] = ReadMacInt32(mouse_base + 4);
			r.a[3] = adb_base;
			r.d[0] = (fMouseRegister3[0] << 4) | 0x0c;	// Talk 0
			Execute68k(r.a[1], &r);

			fMouseOldButton[0] = mb[0];
			fMouseOldButton[1] = mb[1];
			fMouseOldButton[2] = mb[2];
		}

	} else {

		// Update mouse position (absolute)
		if (mx != fMouseOldCoordX || my != fMouseOldCoordY) {
#ifdef POWERPC_ROM
			static const uint8 proc_template[] = {
				0x2f, 0x08,		// move.l a0,-(sp)
				0x2f, 0x00,		// move.l d0,-(sp)
				0x2f, 0x01,		// move.l d1,-(sp)
				0x70, 0x01,		// moveq #1,d0 (MoveTo)
				0xaa, 0xdb,		// CursorDeviceDispatch
				M68K_RTS >> 8, M68K_RTS & 0xff
			};
			BUILD_SHEEPSHAVER_PROCEDURE(proc);
			r.a[0] = ReadMacInt32(mouse_base + 4);
			r.d[0] = mx;
			r.d[1] = my;
			Execute68k(proc, &r);
#else
			WriteMacInt16(0x82a, mx);
			WriteMacInt16(0x828, my);
			WriteMacInt16(0x82e, mx);
			WriteMacInt16(0x82c, my);
			WriteMacInt8(0x8ce, ReadMacInt8(0x8cf));	// CrsrCouple -> CrsrNew
#endif
			fMouseOldCoordX = mx;
			fMouseOldCoordY = my;
		}

		// Send mouse button events
		if (mb[0] != fMouseOldButton[0]
			|| mb[1] != fMouseOldButton[1]
			|| mb[2] != fMouseOldButton[2]) {
			uint32 mouse_base = adb_base + 16;

			// Call mouse ADB handler
			if (fMouseRegister3[1] == 4) {
				// Extended mouse protocol
				WriteMacInt8(tmp_data, 3);
				WriteMacInt8(tmp_data + 1, mb[0] ? 0 : 0x80);
				WriteMacInt8(tmp_data + 2, mb[1] ? 0 : 0x80);
				WriteMacInt8(tmp_data + 3, mb[2] ? 0x08 : 0x88);
			} else {
				// 100/200 dpi mode
				WriteMacInt8(tmp_data, 2);
				WriteMacInt8(tmp_data + 1, mb[0] ? 0 : 0x80);
				WriteMacInt8(tmp_data + 2, mb[1] ? 0 : 0x80);
			}
			r.a[0] = tmp_data;
			r.a[1] = ReadMacInt32(mouse_base);
			r.a[2] = ReadMacInt32(mouse_base + 4);
			r.a[3] = adb_base;
			r.d[0] = (fMouseRegister3[0] << 4) | 0x0c;	// Talk 0
			Execute68k(r.a[1], &r);

			fMouseOldButton[0] = mb[0];
			fMouseOldButton[1] = mb[1];
			fMouseOldButton[2] = mb[2];
		}
	}

	// Process accumulated keyboard events
	while (fKeyReadPtr != fKeyWritePtr) {

		// Read keyboard event
		uint8 mac_code = fKeyBuffer[fKeyReadPtr];
		fKeyReadPtr = (fKeyReadPtr + 1) % KEY_BUFFER_SIZE;

		// Call keyboard ADB handler
		WriteMacInt8(tmp_data, 2);
		WriteMacInt8(tmp_data + 1, mac_code);
		WriteMacInt8(tmp_data + 2, mac_code == 0x7f ? 0x7f : 0xff);	// Power key is special
		r.a[0] = tmp_data;
		r.a[1] = ReadMacInt32(key_base);
		r.a[2] = ReadMacInt32(key_base + 4);
		r.a[3] = adb_base;
		r.d[0] = (fKeyRegister3[0] << 4) | 0x0c;	// Talk 0
		Execute68k(r.a[1], &r);
	}

	// Clear temporary data
	WriteMacInt32(tmp_data, 0);
	WriteMacInt32(tmp_data + 4, 0);
}
