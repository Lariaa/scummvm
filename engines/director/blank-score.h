/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

static const byte kBlankScoreD2[] = {
	0x00, 0x00, 0x00, 0x06, // _framesStreamSize
	0x00, 0x02              // frame with empty channel information
};

static const byte kBlankScoreD4[] = {
	0x00, 0x00, 0x00, 0x12, // _framesStreamSize
	0x00, 0x00, 0x00, 0x10, // frame1Offset
	0x00, 0x00, 0x00, 0x01, // numOfFrames
	0x00, 0x07,             // _framesVersion
	0x00, 0x00,				// _numChannels
	0x00, 0x00,             // skipped
	0x00, 0x02              // frame with empty channel information
};

// D6+ score: outer header (framesStreamSize, ver = -3, listStart) + an empty
// sprite-detail list (numEntries = listSize = maxDataLen = 0) + the inner frame
// header + a single empty frame. _framesVersion 13 yields the D6 default of 120
// displayed channels; _spriteRecordSize 48 matches the real D6/D7 score header.
// loadFrames() seeks to _firstFramePosition (just after the inner header) for
// frames, so the 24-byte outer/list prefix is harmless. Used for cast-only
// movies (no VWSC) such as TKKG 7's scene movies (Sz*.dir).
static const byte kBlankScoreD6[] = {
	// Outer header
	0x00, 0x00, 0x00, 0x2e, // outer _framesStreamSize (overwritten by inner)
	0xff, 0xff, 0xff, 0xfd, // ver = -3 (D6+ marker)
	0x00, 0x00, 0x00, 0x0c, // listStart = 12
	// Sprite-detail list (empty)
	0x00, 0x00, 0x00, 0x00, // numEntries = 0
	0x00, 0x00, 0x00, 0x00, // listSize = 0
	0x00, 0x00, 0x00, 0x00, // maxDataLen = 0
	// Inner frame header
	0x00, 0x00, 0x00, 0x16, // inner _framesStreamSize = 22 (header 20 + frame 2)
	0x00, 0x00, 0x00, 0x14, // frame1Offset
	0x00, 0x00, 0x00, 0x01, // numOfFrames
	0x00, 0x0d,             // _framesVersion = 13
	0x00, 0x30,             // _spriteRecordSize = 48
	0x00, 0x00,             // _numChannels
	0x00, 0x00,             // skipped
	0x00, 0x02              // frame with empty channel information
};
