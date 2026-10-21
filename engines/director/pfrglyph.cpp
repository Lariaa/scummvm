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
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

// Glyph programs of a Bitstream Portable Font Resource, revision 1.
//
// This part of the format is not documented anywhere and is not the one
// FreeType's PFR driver reads. It was recovered from the Bitstream rasteriser's
// machine code by the dirplayer-rs project (GPL-3.0, Igor Lira and
// contributors, vm-rust/src/director/chunks/pfr1), whose reader is the
// reference this follows; the function names there still carry the addresses
// they were found at. Everything below was checked against TKKG 13's
// EurostileT-Medi before it was written, and the checks are quoted where they
// say something.
//
// A glyph program opens with a format byte, then a grid of control values, then
// a stream of four-bit commands, and a second stream of hinting nibbles running
// backwards from the glyph's last byte. Nothing about it resembles PFR0, where
// the commands are whole bytes and the stem values come with a bit mask.

#include "common/debug.h"
#include "common/textconsole.h"
#include "common/util.h"

#include "director/director.h"
#include "director/pfr.h"

namespace Director {

// The format byte, which says two different things depending on its top field.
enum {
	// bits 6-7 of the first byte. 2 and 3 mark a glyph assembled out of others.
	kGlyphOutlineFormatShift = 6,
	// For a compound glyph, how many components, in the low six bits
	kGlyphComponentMask = 0x3f,
	// bits 0-1: how the control counts are written
	kGlyphCountEncMask = 0x03,
	// bit 2: repeat the first vertical control value at the front
	kGlyphRepeatFirstY = 0x04,
	// bit 3: a block of extra items sits between the controls and the commands
	kGlyphExtraItems = 0x08,
	// bit 4, bit 5: how the first horizontal and the first vertical control
	// value are written
	kGlyphXEncShift = 4,
	kGlyphYEncShift = 5,
	// bit 6: every control value after the first carries its own flag nibble
	kGlyphFlagPerCoord = 0x40
};

// One glyph program, read in place.
//
// The reader walks the program in nibbles as often as in bytes, and which half
// of a byte it is in has to be carried along: a control value can begin in the
// low half of one byte and end in the high half of the next. That is what
// `_aligned` tracks -- false means the next read starts on a byte boundary.
class PFRGlyphDecoder {
public:
	// The whole glyph section is needed, not just this glyph: a compound glyph
	// is built out of others and has to be able to reach them.
	PFRGlyphDecoder(const PFRFont &font, const byte *gps, uint32 gpsSize,
			uint32 glyphOffset, uint32 glyphSize, int depth = 0)
		: _font(font), _gps(gps), _gpsSize(gpsSize), _glyphOffset(glyphOffset),
		  _data(gps + glyphOffset), _size(glyphSize), _depth(depth) {}

	bool parse(PFRGlyph &glyph);

private:
	bool parseHeader();
	void parseControlValues(byte flags);

	// A flag nibble, which holds four uses: the first comes from the nibble
	// itself and the next three from shifting what is left of it along.
	int flagNibble();

	// One control value, as a delta on the one before it
	int16 coordValue(int mode);

	// The command stream. Commands are four bits wide, so the reader carries
	// which half of the byte it is in; this is a separate state from the one
	// the control values used, and it starts in the high half.
	bool parseCommands();
	bool processCommand(int cmd);

	int readNibble();
	int readByteAligned();
	int readInlineExtraByte();

	// A coordinate given as a step through the control grid rather than as a
	// number: "the next stem edge along", which is how a stem keeps its width
	// whatever the size.
	int16 orusLookup(int axis, int16 direction);

	void readEncodedCoordPair(int enc, int16 &outX, int16 &outY);
	int16 readEncodedCoordValue(int enc, int axis, int16 current);

	bool processCurve(int cmd);
	void addCurve(int16 startX, int16 startY, int16 c1x, int16 c1y,
			int16 c2x, int16 c2y, int16 endX, int16 endY, bool fromStart);

	void addPoint(int16 x, int16 y);
	void moveTo(int16 x, int16 y);
	void updateAndAddPoint(int16 x, int16 y);
	void finishContour();
	void closeContours();
	void dropRepeatedPoints();

	static int16 toInt16(int32 v) { return (int16)(v & 0xffff); }

	byte at(uint32 pos) const { return pos < _size ? _data[pos] : 0; }
	bool atEnd() const { return _pos >= _size; }

	void parseCompound();
	void readComponentTransform(int format, int32 &scale, int32 &offset);
	void readComponentOffset(int format, int32 &accumulator, int32 &offset, int32 &subSize);
	uint32 nextGlyphStart(uint32 offset) const;

	// Plain reads for the component records, which are bytes throughout
	byte readRawByte();
	uint16 readRawU16();
	uint32 readRawU24();

	const PFRFont &_font;
	const byte *_gps;
	uint32 _gpsSize;
	uint32 _glyphOffset;
	const byte *_data;
	uint32 _size;
	int _depth;
	uint32 _pos = 0;

	// Half-byte state of the control value reader
	bool _aligned = false;
	int _flagCache = 0;
	int _flagCacheCount = 0;

	bool _threeByteMode = false;

	byte _flags = 0;
	int _outlineFormat = 0;
	bool _compound = false;
	uint32 _xCount = 0;
	uint32 _yCount = 0;

	Common::Array<int16> _controlX;
	Common::Array<int16> _controlY;

	// Where the command stream begins, and the last byte of the program, which
	// is where the hinting stream begins and runs backwards from
	uint32 _outlineStart = 0;
	uint32 _outlineEnd = 0;

	// Half-byte state of the command stream, and the pen
	bool _nibbleHigh = false;
	int16 _curX = 0;
	int16 _curY = 0;
	int16 _prevX = 0;
	int16 _prevY = 0;

	Common::Array<PFRContour> _contours;
	PFRContour _contour;
};

// ---------------------------------------------------------------------------
// Compound glyphs
// ---------------------------------------------------------------------------

// One axis of a component's placement. The format number is one sixth of a
// byte, so a component that only needs an offset costs a byte and one that
// needs nothing at all costs none.
void PFRGlyphDecoder::readComponentTransform(int format, int32 &scale, int32 &offset) {
	// 4096 is one to one
	scale = 4096;
	offset = 0;

	if (format == 5)
		scale = 0;
	else if (format == 3 || format == 4)
		scale = (int32)readRawU16();

	if (format == 1 || format == 3)
		offset = (int8)readRawByte();
	else if (format == 2 || format == 4)
		offset = (int16)readRawU16();
}

// Where the component's own glyph program is, and how long it is. Most forms
// are a step back from where the last one started, which is why the offsets
// come out of an accumulator rather than being written down.
void PFRGlyphDecoder::readComponentOffset(int format, int32 &accumulator, int32 &offset, int32 &subSize) {
	offset = 0;
	subSize = 0;

	switch (format) {
	case 0:
	case 1:
	case 2: {
		int32 delta;
		if (format == 0)
			delta = readRawByte();
		else if (format == 1)
			delta = (int32)readRawByte() + 256;
		else
			delta = (int32)readRawU16();

		subSize = delta;
		accumulator -= delta;
		offset = accumulator;
		break;
	}

	case 3: {
		// Three bytes holding a size and a step back, and the accumulator is
		// left where it was
		int32 combined = (int32)readRawU24();
		subSize = combined >> 15;
		offset = accumulator - (combined & 0x7fff);
		break;
	}

	case 4: {
		int32 combined = (int32)readRawU24();
		subSize = combined >> 15;
		offset = combined & 0x7fff;
		break;
	}

	case 5: {
		int32 combined = (int32)readRawU24();
		combined = (combined << 8) | readRawByte();
		subSize = (combined >> 23) & 0x1ff;
		offset = combined & 0x7fffff;
		break;
	}

	case 6:
		subSize = (int32)readRawU16();
		offset = (int32)readRawU24();
		break;

	default:
		offset = accumulator;
		break;
	}
}

byte PFRGlyphDecoder::readRawByte() {
	return atEnd() ? 0 : _data[_pos++];
}

uint16 PFRGlyphDecoder::readRawU16() {
	uint16 hi = readRawByte();
	return (uint16)((hi << 8) | readRawByte());
}

uint32 PFRGlyphDecoder::readRawU24() {
	uint32 v = readRawByte();
	v = (v << 8) | readRawByte();
	return (v << 8) | readRawByte();
}

void PFRGlyphDecoder::parseCompound() {
	// A component that is itself compound is allowed, and a resource that
	// pointed a glyph at itself would otherwise never stop.
	if (_depth >= 8) {
		warning("decodePFRGlyph(): compound glyphs nested more than 8 deep, giving up");
		return;
	}

	uint32 count = _data[0] & kGlyphComponentMask;
	_pos = 1;

	// Bit 6 means a block of extra items comes first
	if ((_data[0] & 0x40) && !atEnd()) {
		uint32 extras = _data[_pos++];
		for (uint32 i = 0; i < extras && !atEnd(); i++) {
			uint32 len = _data[_pos++];
			_pos += len + 1;
		}
	}

	struct Component {
		int32 xScale;
		int32 yScale;
		int32 xOffset;
		int32 yOffset;
		int32 glyphOffset;
		int32 size;
	};

	Common::Array<Component> components;
	int32 accumulator = (int32)_glyphOffset;

	for (uint32 i = 0; i < count && !atEnd(); i++) {
		int format = _data[_pos++];

		// The byte is three numbers in base six, one per axis and one for the
		// component's whereabouts.
		int xFormat = format % 6;
		int yFormat = (format / 6) % 6;
		int offsetFormat = format / 36;

		Component c;
		readComponentTransform(xFormat, c.xScale, c.xOffset);
		readComponentTransform(yFormat, c.yScale, c.yOffset);
		readComponentOffset(offsetFormat, accumulator, c.glyphOffset, c.size);

		components.push_back(c);
	}

	for (uint i = 0; i < components.size(); i++) {
		const Component &c = components[i];
		if (c.glyphOffset < 0 || (uint32)c.glyphOffset >= _gpsSize)
			continue;

		uint32 room = _gpsSize - (uint32)c.glyphOffset;

		// How far the component's program reaches. It says so itself where it
		// can; otherwise it runs to wherever the next glyph in the font
		// begins, which the character records already tell us.
		uint32 size;
		if (c.size > 0)
			size = MIN<uint32>((uint32)c.size, room);
		else
			size = MIN<uint32>(nextGlyphStart((uint32)c.glyphOffset), room);

		if (!size)
			continue;

		const byte *sub = _gps + c.glyphOffset;

		// A program opening on either of these is not one, and following it
		// would turn a letter into noise.
		if (sub[0] == 0xff || sub[0] == 0xfe)
			continue;

		PFRGlyphDecoder decoder(_font, _gps, _gpsSize, (uint32)c.glyphOffset, size, _depth + 1);
		PFRGlyph part;
		if (!decoder.parse(part))
			continue;

		// Scale is in 4096ths, and a scale of nothing means one to one.
		int32 xScale = c.xScale ? c.xScale : 4096;
		int32 yScale = c.yScale ? c.yScale : 4096;

		for (uint j = 0; j < part.contours.size(); j++) {
			PFRContour contour = part.contours[j];
			for (uint k = 0; k < contour.cmds.size(); k++) {
				PFRPathCmd &cmd = contour.cmds[k];
				cmd.x = (cmd.x * xScale >> 12) + c.xOffset;
				cmd.y = (cmd.y * yScale >> 12) + c.yOffset;
				cmd.x1 = (cmd.x1 * xScale >> 12) + c.xOffset;
				cmd.y1 = (cmd.y1 * yScale >> 12) + c.yOffset;
				cmd.x2 = (cmd.x2 * xScale >> 12) + c.xOffset;
				cmd.y2 = (cmd.y2 * yScale >> 12) + c.yOffset;
			}
			_contours.push_back(contour);
		}
	}
}

// Where the next glyph of the font starts, counted from the one at this
// offset. The character records are a complete map of the glyph section, which
// is a better bound than anything the component record carries.
uint32 PFRGlyphDecoder::nextGlyphStart(uint32 offset) const {
	uint32 best = _gpsSize;

	for (uint i = 0; i < _font.physical.chars.size(); i++) {
		uint32 at = _font.physical.chars[i].gpsOffset;
		if (at > offset && at < best)
			best = at;
	}

	return best - offset;
}

bool PFRGlyphDecoder::parseHeader() {
	if (!_size)
		return false;

	_flags = _data[0];
	_outlineFormat = (_flags >> kGlyphOutlineFormatShift) & 3;

	// The same byte is read two ways. For a glyph assembled out of others the
	// top bit says so and the low six bits count the components; for a glyph
	// drawn in its own right those low bits are encoding switches. Which
	// reading applies is decided by both halves at once -- a top field of 2 or
	// 3 with no components below it is not a compound glyph.
	_compound = _outlineFormat >= 2 && (_flags & kGlyphComponentMask) > 0;
	if (_compound)
		return true;

	_pos = 1;

	switch (_flags & kGlyphCountEncMask) {
	case 0:
		// No control grid at all
		break;

	case 1:
		// Both counts in one byte, across in the low half and up in the high
		if (!atEnd()) {
			byte counts = _data[_pos++];
			_xCount = counts & 0x0f;
			_yCount = (counts >> 4) & 0x0f;
		}
		break;

	default:
		// A byte each
		if (_pos + 1 < _size) {
			_xCount = _data[_pos++];
			_yCount = _data[_pos++];
		}
		break;
	}

	if (_xCount || _yCount)
		parseControlValues(_flags);

	// The extra items sit after the control grid and before the commands. None
	// of them is wanted here, and each states its own length, so they are
	// stepped over.
	if ((_flags & kGlyphExtraItems) && !atEnd()) {
		uint32 count = _data[_pos++];
		for (uint32 i = 0; i < count; i++) {
			if (_pos + 1 >= _size)
				break;
			_pos += (uint32)_data[_pos] + 2;
		}
	}

	_outlineStart = _pos;
	_outlineEnd = _size - 1;

	return true;
}

int PFRGlyphDecoder::flagNibble() {
	if (_flagCacheCount > 0) {
		int v = (_flagCache >> 1) & 0x7f;
		_flagCache = v;
		_flagCacheCount--;
		return v & 1;
	}

	if (atEnd())
		return 0;

	byte b = _data[_pos];
	int v;
	if (_aligned) {
		v = b & 0x0f;
		_pos++;
		_aligned = false;
	} else {
		v = b >> 4;
		_aligned = true;
	}

	_flagCache = v;
	_flagCacheCount = 3;
	return v & 1;
}

int16 PFRGlyphDecoder::coordValue(int mode) {
	if (atEnd())
		return 0;

	// An even mode is the plain form: one byte, or the two halves that straddle
	// a byte boundary when the reader is mid-byte.
	if (!(mode & 1)) {
		if (_aligned) {
			int low = _data[_pos++] & 0x0f;
			if (atEnd())
				return toInt16(low << 4);
			int high = _data[_pos] >> 4;
			return toInt16((low << 4) | high);
		}

		return toInt16(_data[_pos++]);
	}

	// An odd mode is the wide form. Where the format byte asked for three-byte
	// coordinates it is a plain sixteen bit number; otherwise it is a signed
	// byte scaled by sixteen with a nibble added on, which is how a value
	// outside a byte's range is written without spending two whole bytes on it.
	if (_threeByteMode) {
		if (_aligned) {
			// Mid-byte, the high half of the byte under the cursor is already
			// spent, so the sixteen bits are the next four nibbles: the low
			// half of this byte, the whole of the next, and the high half of
			// the one after. The cursor then sits on that last byte, with its
			// high half spent -- still mid-byte.
			//
			// The reference reader takes the first nibble from the byte
			// *before* the cursor, which is a nibble that has already been
			// used. It never shows on TKKG 13, whose coordinates all fit the
			// shorter forms; on Courier's vertical bar it turns a bottom edge
			// of -504 into 31230. The correct reading was settled by the font
			// itself: -504 is exactly the lowest point its bounding box allows.
			int n0 = _data[_pos] & 0x0f;
			int b1 = (_pos + 1 < _size) ? _data[_pos + 1] : 0;
			int n3 = (_pos + 2 < _size) ? (_data[_pos + 2] >> 4) : 0;
			_pos += 2;
			return toInt16((n0 << 12) | (b1 << 4) | n3);
		}

		int b0 = _data[_pos++];
		int b1 = atEnd() ? 0 : _data[_pos];
		if (!atEnd())
			_pos++;
		return toInt16((b0 << 8) | b1);
	}

	if (_aligned) {
		int low = _data[_pos++] & 0x0f;
		int next = 0;
		if (!atEnd())
			next = _data[_pos++];

		// The nibble is the high half of a signed byte, so it carries the sign
		int shifted = (int8)(low << 4);
		_aligned = false;
		return toInt16(next + 16 * shifted);
	}

	int signedByte = (int8)_data[_pos++];
	int high = atEnd() ? 0 : (_data[_pos] >> 4);
	_aligned = true;
	return toInt16(high + 16 * signedByte);
}

void PFRGlyphDecoder::parseControlValues(byte flags) {
	_controlX.clear();
	_controlY.clear();

	_threeByteMode = (flags & kGlyphCountEncMask) == 3;
	bool perCoord = (flags & kGlyphFlagPerCoord) != 0;
	int xEnc = (flags >> kGlyphXEncShift) & 1;
	int yEnc = (flags >> kGlyphYEncShift) & 1;

	_aligned = false;
	_flagCache = 0;
	_flagCacheCount = 0;

	// Each value is a step from the one before it, so the grid is read as a
	// running total. The first value of each axis takes its form from the
	// format byte; the rest take theirs from a flag nibble, when there is one.
	int16 accum = 0;
	for (uint32 i = 0; i < _xCount && !atEnd(); i++) {
		int mode = (i == 0) ? xEnc : (perCoord ? flagNibble() : 0);
		accum = toInt16(accum + coordValue(mode));
		_controlX.push_back(accum);
	}

	accum = 0;
	for (uint32 i = 0; i < _yCount && !atEnd(); i++) {
		int mode = (i == 0) ? yEnc : (perCoord ? flagNibble() : 0);
		accum = toInt16(accum + coordValue(mode));
		_controlY.push_back(accum);
	}

	// Two fixups on the vertical grid, both of which make it an even-sized
	// list of pairs: a repeat of the first value when the format byte asks for
	// one, and a repeat of the last when the count came out odd. The grid is
	// read in pairs further on -- a zone is two values, a bottom and a top.
	if ((flags & kGlyphRepeatFirstY) && !_controlY.empty())
		_controlY.insert_at(0, _controlY[0]);

	if ((_controlY.size() & 1) && !_controlY.empty())
		_controlY.push_back(_controlY[_controlY.size() - 1]);

	// A value that ended mid-byte leaves half a byte unread, and the commands
	// start on a boundary.
	if (_aligned)
		_pos++;

	_aligned = false;
}

// ---------------------------------------------------------------------------
// The command stream
// ---------------------------------------------------------------------------

int PFRGlyphDecoder::readNibble() {
	if (atEnd())
		return -1;

	// The half is switched before the read, not after: a stream that starts
	// byte-aligned therefore begins with the high half.
	_nibbleHigh = !_nibbleHigh;

	if (_nibbleHigh)
		return _data[_pos] >> 4;

	return _data[_pos++] & 0x0f;
}

int PFRGlyphDecoder::readByteAligned() {
	if (atEnd())
		return 0;

	// Mid-byte, a byte is the low half of this one and the high half of the
	// next; the half the reader is in does not change.
	if (_nibbleHigh) {
		int low = _data[_pos++] & 0x0f;
		if (atEnd())
			return low << 4;
		return (low << 4) | (_data[_pos] >> 4);
	}

	return _data[_pos++];
}

int PFRGlyphDecoder::readInlineExtraByte() {
	if (atEnd())
		return 0;

	if (_nibbleHigh) {
		int low = _data[_pos++] & 0x0f;
		int high = atEnd() ? 0 : (_data[_pos] >> 4);
		return (low << 4) | high;
	}

	return _data[_pos++];
}

int16 PFRGlyphDecoder::orusLookup(int axis, int16 direction) {
	const Common::Array<int16> &ctrl = (axis == 1) ? _controlY : _controlX;
	int16 current = (axis == 1) ? _curY : _curX;

	// A glyph with no grid on this axis has nothing to step through, and the
	// command then means "stay put" rather than "go to zero". A capital A is
	// the case: it has no upright, so its horizontal grid is empty.
	if (ctrl.empty())
		return current;

	int16 previous = (axis == 1) ? _prevY : _prevX;

	// Direction zero means carry on the way the pen was already going.
	if (direction == 0) {
		if (current < previous)
			direction = -1;
		else if (current == previous)
			return current;
		else
			direction = 1;
	}

	int count = (int)ctrl.size();

	if (direction > 0) {
		int i = 0;
		while (i < count && ctrl[i] <= current)
			i++;

		if (i >= count)
			return current;

		int at = CLIP<int>(i + direction - 1, 0, count - 1);
		return ctrl[at];
	}

	for (int i = count - 1; i >= 0; i--) {
		if (ctrl[i] < current) {
			int at = MAX<int>(i + direction + 1, 0);
			return ctrl[at];
		}
	}

	return current;
}

int16 PFRGlyphDecoder::readEncodedCoordValue(int enc, int axis, int16 current) {
	switch (enc) {
	case 1: {
		// A nibble holding -8 to +7
		int n = readNibble();
		if (n < 0)
			return current;
		return toInt16(current + n - 8);
	}

	case 2: {
		// A byte that is either a step through the control grid, when it is
		// small enough to be one, or a plain delta.
		int8 b = (int8)readByteAligned();
		if (b >= -8 && b < 8) {
			int16 direction = (b >= 0) ? (int16)(b + 1) : (int16)b;
			return orusLookup(axis, direction);
		}
		return toInt16(current + b);
	}

	case 3: {
		// Twelve bits, and if they fit in a byte then sixteen: the short form
		// is kept for the values a byte cannot hold, so a twelve bit value
		// inside a byte range means another byte follows.
		int hi = readByteAligned();
		_nibbleHigh = !_nibbleHigh;

		int low;
		if (atEnd())
			low = -1;
		else if (_nibbleHigh)
			low = _data[_pos] >> 4;
		else
			low = _data[_pos++] & 0x0f;

		if (low < 0)
			return current;

		int delta = ((int)(int8)hi << 4) | low;
		if (delta >= -128 && delta < 128) {
			int extra = readInlineExtraByte();
			return toInt16(current + ((delta << 8) | (extra & 0xff)));
		}

		return toInt16(current + delta);
	}

	default:
		// Encoding zero means this axis does not move
		return current;
	}
}

void PFRGlyphDecoder::readEncodedCoordPair(int enc, int16 &outX, int16 &outY) {
	int xEnc = enc & 3;
	int yEnc = (enc >> 2) & 3;

	// The axes are read in order and the pen follows along as they are, because
	// a step through the grid is measured against where the pen stands now.
	if (xEnc)
		outX = readEncodedCoordValue(xEnc, 0, _curX);

	_prevX = _curX;
	_curX = outX;

	if (yEnc)
		outY = readEncodedCoordValue(yEnc, 1, _curY);

	_prevY = _curY;
	_curY = outY;
}

void PFRGlyphDecoder::addPoint(int16 x, int16 y) {
	PFRPathCmd cmd;
	cmd.op = _contour.cmds.empty() ? kPFRMoveTo : kPFRLineTo;
	cmd.x = x;
	cmd.y = y;
	_contour.cmds.push_back(cmd);
}

void PFRGlyphDecoder::moveTo(int16 x, int16 y) {
	PFRPathCmd cmd;
	cmd.op = kPFRMoveTo;
	cmd.x = x;
	cmd.y = y;
	_contour.cmds.push_back(cmd);
}

void PFRGlyphDecoder::updateAndAddPoint(int16 x, int16 y) {
	_prevX = _curX;
	_prevY = _curY;
	addPoint(x, y);
	_curX = x;
	_curY = y;
}

void PFRGlyphDecoder::finishContour() {
	// One point is not a contour; that happens where a command stream ends
	// just after a move.
	if (_contour.cmds.size() > 1)
		_contours.push_back(_contour);

	_contour.cmds.clear();
}

void PFRGlyphDecoder::dropRepeatedPoints() {
	for (uint i = 0; i < _contours.size(); i++) {
		Common::Array<PFRPathCmd> kept;
		bool first = true;
		int32 lastX = 0;
		int32 lastY = 0;

		for (uint j = 0; j < _contours[i].cmds.size(); j++) {
			const PFRPathCmd &cmd = _contours[i].cmds[j];
			if (!first && cmd.x == lastX && cmd.y == lastY)
				continue;
			kept.push_back(cmd);
			lastX = cmd.x;
			lastY = cmd.y;
			first = false;
		}

		_contours[i].cmds = kept;
	}
}

void PFRGlyphDecoder::closeContours() {
	for (uint i = 0; i < _contours.size(); i++) {
		Common::Array<PFRPathCmd> &cmds = _contours[i].cmds;
		if (cmds.empty() || cmds[0].op != kPFRMoveTo)
			continue;

		const PFRPathCmd &last = cmds[cmds.size() - 1];
		if (last.x == cmds[0].x && last.y == cmds[0].y)
			continue;

		PFRPathCmd cmd;
		cmd.op = kPFRLineTo;
		cmd.x = cmds[0].x;
		cmd.y = cmds[0].y;
		cmds.push_back(cmd);
	}
}

// ---------------------------------------------------------------------------
// Curves
// ---------------------------------------------------------------------------

// Three of the nine curve commands name their coordinate encodings through a
// table rather than spelling them out, which is how a curve of a shape the
// typeface uses over and over costs one nibble instead of three.
static const uint16 kCurveTable9[16] = {
	0x0451, 0x0452, 0x0461, 0x0462, 0x0491, 0x0492, 0x04a1, 0x04a2,
	0x0851, 0x0852, 0x0861, 0x0862, 0x0891, 0x0892, 0x08a1, 0x08a2
};

static const uint16 kCurveTable10[16] = {
	0x0154, 0x0158, 0x0164, 0x0168, 0x0194, 0x0198, 0x01a4, 0x01a8,
	0x0254, 0x0258, 0x0264, 0x0268, 0x0294, 0x0298, 0x02a4, 0x02a8
};

static const uint16 kCurveTable13[16] = {
	0x0fff, 0x03aa, 0x0caa, 0x0aa3, 0x0aac, 0x0aaa, 0x02aa, 0x08aa,
	0x0aa2, 0x0aa8, 0x00aa, 0x0555, 0x0155, 0x0455, 0x0551, 0x0554
};

static const byte kCurveTable14A[8] = { 1, 2, 4, 5, 6, 8, 9, 10 };
static const byte kCurveTable14B[4] = { 5, 6, 9, 10 };

static uint32 curveEncoding14(uint32 b) {
	uint32 v = kCurveTable14B[(b >> 3) & 3] + 16 * kCurveTable14A[(b >> 5) & 7];
	return kCurveTable14A[b & 7] + v * 16;
}

void PFRGlyphDecoder::addCurve(int16 startX, int16 startY, int16 c1x, int16 c1y,
		int16 c2x, int16 c2y, int16 endX, int16 endY, bool fromStart) {
	// A curve that leaps an absurd distance is a curve read wrongly, and
	// carrying it would drag the whole outline out of shape. Breaking the
	// contour there keeps what came before it.
	const int32 maxJump = 50000;
	if (fromStart) {
		int32 toC1 = MAX(ABS((int32)c1x - startX), ABS((int32)c1y - startY));
		int32 toC2 = MAX(ABS((int32)c2x - startX), ABS((int32)c2y - startY));
		int32 toEnd = MAX(ABS((int32)endX - startX), ABS((int32)endY - startY));

		if (toC1 > maxJump || toC2 > maxJump || toEnd > maxJump) {
			if (!_contour.cmds.empty())
				finishContour();
			_curX = endX;
			_curY = endY;
			_prevX = c2x;
			_prevY = c2y;
			return;
		}
	}

	if (_contour.cmds.empty()) {
		// Which point opens the contour differs between the two callers: the
		// straight case starts where the curve starts, the turning case one
		// point further back.
		PFRPathCmd move;
		move.op = kPFRMoveTo;
		move.x = fromStart ? startX : _prevX;
		move.y = fromStart ? startY : _prevY;
		_contour.cmds.push_back(move);
	}

	PFRPathCmd cmd;
	cmd.op = kPFRCurveTo;
	cmd.x1 = c1x;
	cmd.y1 = c1y;
	cmd.x2 = c2x;
	cmd.y2 = c2y;
	cmd.x = endX;
	cmd.y = endY;
	_contour.cmds.push_back(cmd);

	if (fromStart) {
		_prevX = c2x;
		_prevY = c2y;
	}
}

bool PFRGlyphDecoder::processCurve(int cmd) {
	uint32 enc = 0;
	int path = 0;

	// Each command says how its three coordinate pairs are written, either
	// outright, through a table, or in a byte of its own. The path number
	// decides which point each pair is measured from.
	switch (cmd) {
	case 7:
		enc = 2210;
		path = 49;
		break;

	case 8:
		enc = 680;
		path = 54;
		break;

	case 9: {
		int n = readNibble();
		if (n < 0)
			return true;
		enc = kCurveTable9[n & 0x0f];
		path = 49;
		break;
	}

	case 10: {
		int n = readNibble();
		if (n < 0)
			return true;
		enc = kCurveTable10[n & 0x0f];
		path = 54;
		break;
	}

	case 11: {
		uint32 b = (uint32)readByteAligned();
		enc = (b & 3) + 4 * ((b & 0x3c) + 4 * (b & 0xc0));
		path = 49;
		break;
	}

	case 12:
		enc = (uint32)readByteAligned() * 4;
		path = 54;
		break;

	case 13: {
		int n = readNibble();
		if (n < 0)
			return true;
		enc = kCurveTable13[n & 0x0f];
		path = 70;
		break;
	}

	case 14:
		enc = curveEncoding14((uint32)readByteAligned());
		path = 70;
		break;

	case 15: {
		int high = readNibble();
		if (high < 0)
			return true;
		enc = (uint32)readByteAligned() + ((uint32)high << 8);
		path = 70;
		break;
	}

	default:
		return true;
	}

	int16 startX = _curX;
	int16 startY = _curY;

	int16 c1x = _curX;
	int16 c1y = _curY;
	int16 c2x, c2y, endX, endY;

	if (path == 49) {
		// The first control point is read freely, the second starts on the
		// nearest stem edge across, and the end on the nearest alignment
		// height up -- so a quarter turn costs almost nothing to write down.
		readEncodedCoordPair(enc & 0x0f, c1x, c1y);

		c2x = orusLookup(0, 0);
		c2y = c1y;
		readEncodedCoordPair((enc >> 4) & 0x0f, c2x, c2y);

		endX = c2x;
		endY = orusLookup(1, 0);
		readEncodedCoordPair((enc >> 8) & 0x0f, endX, endY);
	} else if (path == 54) {
		// The same, with the axes the other way round
		readEncodedCoordPair(enc & 0x0f, c1x, c1y);

		c2x = c1x;
		c2y = orusLookup(1, 0);
		readEncodedCoordPair((enc >> 4) & 0x0f, c2x, c2y);

		endX = orusLookup(0, 0);
		endY = c2y;
		readEncodedCoordPair((enc >> 8) & 0x0f, endX, endY);
	} else {
		// Every point relative to the one before, starting with the direction
		// the pen was already travelling in -- a smooth join needs no numbers.
		c1x = toInt16(c1x + (_curX - _prevX));
		c1y = toInt16(c1y + (_curY - _prevY));
		readEncodedCoordPair(enc & 0x0f, c1x, c1y);

		c2x = c1x;
		c2y = c1y;
		readEncodedCoordPair((enc >> 4) & 0x0f, c2x, c2y);

		endX = c2x;
		endY = c2y;
		readEncodedCoordPair((enc >> 8) & 0x0f, endX, endY);
	}

	// Whether the three points turn at all. They may not: a curve command is
	// also how a straight run gets written when it falls out of the encoding
	// cheaply, and the two cases open their contour from different points.
	int32 cross = ((int32)c1x - startX) * ((int32)endY - c1y)
				- ((int32)c1y - startY) * ((int32)endX - c1x);

	addCurve(startX, startY, c1x, c1y, c2x, c2y, endX, endY, cross == 0);

	_curX = endX;
	_curY = endY;

	return true;
}

bool PFRGlyphDecoder::processCommand(int cmd) {
	switch (cmd) {
	case 0: {
		// A step through the control grid, with the direction and the axis
		// packed into one nibble. This is how a stem edge is named rather than
		// measured, and it is most of what a straight letter is made of: a
		// capital I is one move and three of these.
		int nibble = readNibble();
		if (nibble < 0)
			return true;

		int16 x = _curX;
		int16 y = _curY;

		int16 direction = (nibble & 4) ? (int16)((nibble & 7) - 8) : (int16)((nibble & 7) + 1);

		if (nibble & 8)
			y = orusLookup(1, direction);
		else
			x = orusLookup(0, direction);

		updateAndAddPoint(x, y);
		return true;
	}

	case 1:
	case 2: {
		// A signed byte along one axis
		int16 delta = (int16)(int8)readByteAligned();
		if (cmd == 1)
			updateAndAddPoint(toInt16(_curX + delta), _curY);
		else
			updateAndAddPoint(_curX, toInt16(_curY + delta));
		return true;
	}

	case 3:
	case 4: {
		// Twelve bits along one axis, widened to sixteen when they would fit
		// in a byte, by the same rule as coordinate encoding 3.
		int hi = readByteAligned();
		_nibbleHigh = !_nibbleHigh;

		int low;
		if (atEnd())
			low = 0;
		else if (_nibbleHigh)
			low = _data[_pos] >> 4;
		else
			low = _data[_pos++] & 0x0f;

		int delta = ((int)(int8)hi << 4) | low;
		if (delta >= -128 && delta < 128)
			delta = (delta << 8) | readInlineExtraByte();

		if (cmd == 3)
			updateAndAddPoint(toInt16(_curX + delta), _curY);
		else
			updateAndAddPoint(_curX, toInt16(_curY + delta));
		return true;
	}

	case 5:
	case 6: {
		int enc = readNibble();
		if (enc < 0)
			return true;

		int16 x = _curX;
		int16 y = _curY;
		readEncodedCoordPair(enc, x, y);

		if (cmd == 5) {
			// A line to where the pair put the pen
			addPoint(_curX, _curY);
		} else {
			// A move, which ends whatever contour was open
			if (!_contour.cmds.empty())
				finishContour();
			moveTo(_curX, _curY);
		}
		return true;
	}

	default:
		return processCurve(cmd);
	}
}

bool PFRGlyphDecoder::parseCommands() {
	_curX = 0;
	_curY = 0;
	_prevX = 0;
	_prevY = 0;
	_nibbleHigh = false;

	// Two commands to a byte at the most, so the loop cannot outlast the
	// stream however badly it is being misread.
	uint32 maxCommands = _size * 2;
	bool first = true;

	for (uint32 i = 0; i < maxCommands; i++) {
		// The stream stops at the last byte of the glyph, which belongs to the
		// hinting stream running the other way. A half-read byte there is the
		// end as well.
		if (_pos >= _outlineEnd && (_pos != _outlineEnd || _nibbleHigh))
			break;

		if (atEnd())
			break;

		int cmd;
		if (first) {
			// A glyph opens on a move whether its stream says so or not
			cmd = 6;
			first = false;
		} else {
			cmd = readNibble();
			if (cmd < 0)
				break;
		}

		if (!processCommand(cmd))
			return false;
	}

	finishContour();
	dropRepeatedPoints();
	closeContours();

	return true;
}

bool PFRGlyphDecoder::parse(PFRGlyph &glyph) {
	if (!parseHeader())
		return false;

	glyph.controlX = _controlX;
	glyph.controlY = _controlY;

	if (_compound) {
		parseCompound();
		if (_contours.empty())
			return false;

		glyph.contours = _contours;
		return true;
	}

	debugC(6, kDebugText, "decodePFRGlyph(): flags 0x%02x, %d by %d controls over %d units per em, commands %d..%d of %d bytes",
			_flags, (int)_controlX.size(), (int)_controlY.size(),
			(int)_font.physical.outlineResolution,
			(int)_outlineStart, (int)_outlineEnd, (int)_size);

	if (!parseCommands()) {
		debugC(6, kDebugText, "decodePFRGlyph(): a command this reader does not know yet, at %d", (int)_pos);
		return false;
	}

	if (_contours.empty())
		return false;

	glyph.contours = _contours;

	return true;
}

bool decodePFRGlyph(const PFRFont &font, const byte *gps, uint32 gpsSize,
		uint32 glyphOffset, uint32 glyphSize, PFRGlyph &glyph) {
	if (!gps || !glyphSize || glyphOffset >= gpsSize || glyphSize > gpsSize - glyphOffset)
		return false;

	PFRGlyphDecoder decoder(font, gps, gpsSize, glyphOffset, glyphSize);
	return decoder.parse(glyph);
}

} // End of namespace Director
