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

#include "common/debug.h"
#include "common/endian.h"
#include "common/textconsole.h"
#include "common/util.h"

#include "director/director.h"
#include "director/pfr.h"

namespace Director {

// A reader that cannot be walked off the end of its buffer: any read past the
// end sets a flag, returns zero, and leaves the flag set, so a parse can be
// written straight through and checked once at the bottom.
//
// Bits run most significant first inside a byte, and a byte-sized read throws
// away whatever is left of the byte a bit-sized read was in the middle of.
// Multi-byte values are big-endian. All three of those are the format's, not a
// choice -- a summary of the reference reader that claimed little-endian would
// have transposed every field in the file.
class PFRReader {
public:
	PFRReader(const byte *data, uint32 size, uint32 pos = 0)
		: _data(data), _size(size), _pos(pos) {}

	bool bad() const { return _bad; }
	uint32 pos() const { return _pos; }
	uint32 remaining() const { return _pos >= _size ? 0 : _size - _pos; }

	void setPos(uint32 pos) {
		_pos = pos;
		alignToByte();
		if (_pos > _size)
			_bad = true;
	}

	void alignToByte() {
		_bitBuf = 0;
		_bitsLeft = 0;
	}

	void skip(uint32 n) {
		alignToByte();
		if (!room(n))
			return;
		_pos += n;
	}

	byte u8() {
		alignToByte();
		if (!room(1))
			return 0;
		return _data[_pos++];
	}

	int8 i8() { return (int8)u8(); }

	uint16 u16() {
		uint16 hi = u8();
		return (uint16)((hi << 8) | u8());
	}

	int16 i16() { return (int16)u16(); }

	uint32 u24() {
		uint32 b0 = u8();
		uint32 b1 = u8();
		uint32 b2 = u8();
		return (b0 << 16) | (b1 << 8) | b2;
	}

	int32 i24() {
		uint32 v = u24();
		return (v & 0x800000) ? (int32)(v | 0xff000000) : (int32)v;
	}

	// Read count bits, most significant first, crossing byte boundaries.
	uint32 bits(uint32 count) {
		uint32 result = 0;

		while (count) {
			if (!_bitsLeft) {
				if (!room(1))
					return result;
				_bitBuf = _data[_pos++];
				_bitsLeft = 8;
			}

			uint32 take = MIN(count, _bitsLeft);
			uint32 shift = _bitsLeft - take;
			uint32 mask = ((1u << take) - 1) << shift;

			result = (result << take) | ((_bitBuf & mask) >> shift);
			_bitsLeft -= take;
			count -= take;
		}

		return result;
	}

	bool bit() { return bits(1) != 0; }

private:
	bool room(uint32 n) {
		if (_bad || _pos + n > _size || _pos + n < _pos) {
			_bad = true;
			return false;
		}
		return true;
	}

	const byte *_data;
	uint32 _size;
	uint32 _pos;
	uint32 _bitBuf = 0;
	uint32 _bitsLeft = 0;
	bool _bad = false;
};

// The header is 58 bytes and says so in a field of its own, which is worth
// checking: it is the only place the file states its own shape before any of
// the offsets inside it are used.
static bool parseHeader(PFRReader &r, PFRFont &font) {
	byte magic[4];
	for (int i = 0; i < 4; i++)
		magic[i] = r.u8();

	bool pfr0 = !memcmp(magic, "PFR0", 4);
	bool pfr1 = !memcmp(magic, "PFR1", 4);
	if (!pfr0 && !pfr1)
		return false;

	uint16 version = r.u16();
	uint16 sig2 = r.u16();
	font.headerSize = r.u16();

	// A version above 4 is a revision nobody here has met, and CR/LF after the
	// version is the format's own guard against a text-mode transfer having
	// mangled the file.
	if (version > 4 || sig2 != 0x0d0a || font.headerSize < 58)
		return false;

	font.version = pfr1 ? 1 : 0;

	uint32 logFontDirSize = r.u16();
	uint32 logFontDirOffset = r.u16();
	r.u16();						// the largest logical font record
	font.logFontSectionSize = r.u24();
	font.logFontSectionOffset = r.u24();
	r.u16();						// the largest physical font record
	font.physFontSectionSize = r.u24();
	font.physFontSectionOffset = r.u24();
	r.u16();						// the largest glyph program
	font.gpsSectionSize = r.u24();
	font.gpsSectionOffset = r.u24();

	font.maxBlueValues = r.u8();
	font.maxXOrus = r.u8();
	font.maxYOrus = r.u8();
	font.physFontMaxSizeHigh = r.u8();

	byte flags = r.u8();
	font.invertBitmap = (flags & 0x02) != 0;
	font.blackPixel = (flags & 0x01) != 0;

	r.u24();						// bitmap character table sizes, three of them
	r.u24();
	r.u24();

	font.numPhysFonts = r.u16();

	r.u8();							// the largest stem snap tables
	r.u8();

	font.maxChars = r.u16();

	// Kept for the logical font step, which is the only reader of them
	font.logFontDirSize = logFontDirSize;
	font.logFontDirOffset = logFontDirOffset;

	return !r.bad();
}

// One logical font record: the matrix that takes font units to output units,
// and which physical font it draws from.
static void parseOneLogicalFont(PFRReader &r, PFRFont &font) {
	PFRLogicalFont lf;

	for (int i = 0; i < 4; i++)
		lf.matrix[i] = r.i24();

	// A single byte, but every bit of it is a switch, so it is read as bits.
	r.bit();						// must be zero
	bool extraItems = r.bit();
	bool twoByteBoldThickness = r.bit();
	bool bold = r.bit();
	bool twoByteStrokeThickness = r.bit();
	bool stroke = r.bit();
	uint32 lineJoin = r.bits(2);

	if (stroke) {
		r.bits(twoByteStrokeThickness ? 16 : 8);
		if (lineJoin == 0)			// a mitred join carries its limit with it
			r.bits(24);
	} else if (bold) {
		r.bits(twoByteBoldThickness ? 16 : 8);
	}

	if (extraItems) {
		uint32 count = r.bits(8);
		for (uint32 i = 0; i < count && !r.bad(); i++) {
			uint32 size = r.bits(8);
			r.bits(8);				// item type
			for (uint32 j = 0; j < size; j++)
				r.bits(8);
		}
	}

	lf.size = r.bits(16);
	lf.offset = r.bits(24);

	// The size is widened by a byte only when the header says any of them needs it
	if (font.physFontMaxSizeHigh)
		lf.size += r.bits(8) * 65536;

	if (!r.bad())
		font.logicalFonts.push_back(lf);
}

static void parseLogicalFonts(const byte *data, uint32 size, PFRFont &font) {
	if (!font.logFontDirOffset || !font.logFontDirSize)
		return;

	// PFR1 may leave the directory too small to hold a record, and then the one
	// record is simply the start of the logical font section. TKKG 13 is this
	// case: its directory is seven bytes, a count of one and a (size, offset)
	// pair pointing at the section, and the record there is the identity
	// matrix with neither stroke nor bold.
	if (font.version == 1 && font.logFontDirSize < 14) {
		if (font.logFontSectionSize < 18 || !font.logFontSectionOffset
				|| font.logFontSectionOffset >= size)
			return;

		PFRReader r(data, size, font.logFontSectionOffset);
		parseOneLogicalFont(r, font);
		return;
	}

	if (font.logFontDirOffset >= size)
		return;

	PFRReader r(data, size, font.logFontDirOffset);
	uint32 count = r.u16();

	// A resource claiming thousands of logical fonts is a resource we have
	// misread; the format allows many but no title here ships more than one.
	for (uint32 i = 0; i < MIN<uint32>(count, 16) && !r.bad(); i++)
		parseOneLogicalFont(r, font);
}

// The auxiliary block is a run of records, each a two byte length that counts
// its own four byte head, then a two byte type. The names live here, and so do
// private records that steer how the glyph programs are read.
static void parseAux(const byte *aux, uint32 size, PFRPhysicalFont &phys) {
	uint32 off = 0;

	while (off + 4 <= size) {
		uint32 len = READ_BE_UINT16(aux + off);
		if (len < 4 || off + len > size)
			break;

		uint16 type = READ_BE_UINT16(aux + off + 2);
		const byte *payload = aux + off + 4;
		uint32 payloadLen = len - 4;

		switch (type) {
		case 1:
			for (uint32 i = 0; i < payloadLen && payload[i]; i++)
				phys.familyName += (char)payload[i];
			break;

		case 2:
			// Two readings of one record, both of them true: the font's own
			// vertical metrics sit at the front, and a handful of bytes
			// further in decide how the glyph programs are decoded.
			if (payloadLen >= 16) {
				phys.ascent = (int16)READ_BE_UINT16(payload + 10);
				phys.descent = (int16)READ_BE_UINT16(payload + 12);
				phys.lineHeight = (int16)READ_BE_UINT16(payload + 14);
			}
			if (payloadLen >= 28) {
				byte mode = payload[27];
				// Two-byte character codes and this mode cannot both hold
				if (mode == 2 && (phys.flags & 0x02))
					mode = 0;
				phys.privateMode = mode;
			}
			if (payloadLen >= 30) {
				phys.privateByte28 = payload[28];
				phys.privateByte29 = payload[29];
			}
			if (payloadLen >= 27 && !phys.privateFlags) {
				if (READ_BE_UINT16(payload + 24) >= 500)
					phys.privateFlags |= 1;
				if (payload[26] != 0)
					phys.privateFlags |= 2;
			}
			break;

		case 3:
			for (uint32 i = 0; i < payloadLen && payload[i]; i++)
				phys.styleName += (char)payload[i];
			break;

		case 5:
			if (payloadLen >= 8) {
				phys.hasLineSpacing = true;
				phys.lineSpacing = (int16)READ_BE_UINT16(payload + 4);
			}
			break;

		case 7:
			// Where it exists this record decides the same two flags as the
			// fallback above, so it wins.
			phys.privateFlags = 0;
			if (payloadLen >= 17) {
				if (READ_BE_UINT16(payload + 4) > 550)
					phys.privateFlags |= 1;
				if (payload[16] != 0)
					phys.privateFlags |= 2;
			}
			break;

		default:
			break;
		}

		off += len;
	}
}

// The delta-coded character records, and the reason a PFR1 cannot be handed to
// a PFR0 reader. Each record opens with a flag byte whose four two-bit fields
// say how the code, the width, the glyph size and the glyph offset carry on
// from the record before, so a record is between one and ten bytes long where
// PFR0's are all the same length and hold absolute values. The form is the more
// compact of the two, which is how the mismatch shows: read the other way, the
// table runs past the end of its own section.
static void parseCharRecords(PFRReader &r, PFRPhysicalFont &phys, uint32 numChars) {
	int32 code = -1;
	int32 setWidth = phys.standardSetWidth;
	int32 gpsSize = 0;
	int32 gpsOffset = 0;

	phys.chars.reserve(numChars);

	for (uint32 i = 0; i < numChars; i++) {
		if (r.bad() || !r.remaining())
			break;

		byte flags = r.u8();

		// Taken before any delta is read: a record that says "the next one"
		// means the byte after the previous glyph, not after this one.
		int32 nextOffset = gpsOffset + gpsSize;

		code++;
		switch (flags & 0x03) {
		case 1: code += r.u8(); break;
		case 2: code += r.u16(); break;
		default: break;
		}

		switch ((flags >> 2) & 0x03) {
		case 1: setWidth += r.u8(); break;
		case 2: setWidth -= r.u8(); break;
		case 3: setWidth = r.i16(); break;
		default: break;
		}

		switch ((flags >> 4) & 0x03) {
		case 0: gpsSize = r.u8(); break;
		case 1: gpsSize = r.u8() + 256; break;
		case 2: gpsSize = r.u8() + 512; break;
		default: gpsSize = r.u16(); break;
		}

		switch ((flags >> 6) & 0x03) {
		case 0: gpsOffset = nextOffset; break;
		case 1: gpsOffset = nextOffset + r.u8(); break;
		case 2: gpsOffset = r.u16(); break;
		default: gpsOffset = r.u24(); break;
		}

		if (r.bad())
			break;

		PFRCharRecord rec;
		rec.code = (uint32)code;
		rec.setWidth = setWidth;
		rec.gpsSize = (uint32)gpsSize;
		rec.gpsOffset = (uint32)gpsOffset;
		phys.chars.push_back(rec);
	}
}

static bool parsePhysicalFont(const byte *data, uint32 size, PFRFont &font) {
	PFRPhysicalFont &phys = font.physical;

	uint32 start = font.physFontSectionOffset;
	if (!font.physFontSectionSize || start >= size
			|| font.physFontSectionSize > size - start)
		return false;

	// The section's own end, not the file's: a field that points past it is a
	// misreading, and stopping there is what tells us so.
	uint32 end = start + font.physFontSectionSize;
	PFRReader r(data, end, start);

	r.u16();						// font reference number

	phys.outlineResolution = r.u16();
	if (!phys.outlineResolution)
		phys.outlineResolution = 2048;

	phys.metricsResolution = r.u16();
	if (!phys.metricsResolution)
		phys.metricsResolution = phys.outlineResolution;

	phys.xMin = r.i16();
	phys.yMin = r.i16();
	phys.xMax = r.i16();
	phys.yMax = r.i16();

	phys.flags = r.u8();

	if (!phys.isProportional())
		phys.standardSetWidth = r.i16();

	if (phys.flags & 0x80) {
		uint32 count = r.u8();
		for (uint32 i = 0; i < count && !r.bad(); i++) {
			if (r.remaining() < 2)
				break;

			uint32 itemSize = r.u8();
			byte itemType = r.u8();
			uint32 itemStart = r.pos();

			// Only the name is wanted here. Every item is followed back to
			// where it said it ends, so the ones left alone -- the bitmap
			// strike specification and the stem snap tables -- cost nothing
			// to skip and cannot put the rest of the section out of step.
			if (itemType == 2) {
				for (uint32 j = 0; j < itemSize; j++) {
					byte ch = r.u8();
					if (!ch)
						break;
					phys.fontId += (char)ch;
				}
			}

			r.setPos(itemStart + itemSize);
		}
	}

	// The auxiliary block states its own length in three bytes, and that length
	// is not always to be trusted: the reference reader treats a count of 10000
	// or more as unusable and goes looking for the block's real end by probing
	// for the character count that follows it. No font here does that -- TKKG
	// 13's says 4091 and the records inside add up to it -- so rather than
	// carry a search nobody can test against a real file, stop.
	uint32 numAux = r.u24();
	if (numAux >= 10000)
		return false;

	if (numAux) {
		uint32 auxAt = r.pos();
		r.skip(numAux);
		if (!r.bad())
			parseAux(data + auxAt, numAux, phys);
	}

	uint32 numBlue = r.u8();
	phys.blueValues.reserve(numBlue);
	for (uint32 i = 0; i < numBlue && !r.bad(); i++)
		phys.blueValues.push_back(r.i16());

	phys.blueFuzz = r.u8();
	phys.blueScale = r.u8();

	phys.stdVW = r.i16();
	phys.stdHW = r.i16();

	uint32 numChars = r.u16();
	if (r.bad() || !numChars)
		return false;

	parseCharRecords(r, phys, numChars);

	// A short read is worth saying out loud rather than passing on quietly: it
	// means the records were read by the wrong rule, and every width after the
	// break would be wrong rather than missing.
	if (phys.chars.size() != numChars) {
		warning("loadPFR(): %d of %d character records read; the rest of the section does not fit them",
				(int)phys.chars.size(), (int)numChars);
		return false;
	}

	return true;
}

bool loadPFR(const byte *data, uint32 size, PFRFont &font) {
	if (!data || size < 58)
		return false;

	PFRReader r(data, size);
	if (!parseHeader(r, font))
		return false;

	parseLogicalFonts(data, size, font);

	if (!parsePhysicalFont(data, size, font))
		return false;

	// The glyph section is not read here, but a resource whose glyphs lie
	// outside it has been misread and its metrics should not be believed either.
	if (!font.gpsSectionSize || font.gpsSectionOffset >= size
			|| font.gpsSectionSize > size - font.gpsSectionOffset)
		return false;

	for (uint i = 0; i < font.physical.chars.size(); i++) {
		const PFRCharRecord &c = font.physical.chars[i];
		if (c.gpsOffset + c.gpsSize > font.gpsSectionSize) {
			warning("loadPFR(): character %d points at %d..%d of a %d byte glyph section",
					(int)c.code, (int)c.gpsOffset, (int)(c.gpsOffset + c.gpsSize),
					(int)font.gpsSectionSize);
			return false;
		}
	}

	debugC(3, kDebugText, "loadPFR(): PFR%d, '%s' / '%s', %d units per em, %d characters, %d blue values, glyphs %d bytes at %d",
			font.version, font.physical.familyName.c_str(), font.physical.fontId.c_str(),
			(int)font.physical.outlineResolution, (int)font.physical.chars.size(),
			(int)font.physical.blueValues.size(), (int)font.gpsSectionSize,
			(int)font.gpsSectionOffset);

	return true;
}

} // End of namespace Director
