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

#ifndef DIRECTOR_PFR_H
#define DIRECTOR_PFR_H

#include "common/array.h"
#include "common/str.h"

namespace Common {
class SeekableReadStream;
}

namespace Director {

// Bitstream's Portable Font Resource, the format Director embeds a font in.
//
// A movie that wants its text to look the same on a machine which has never
// heard of the typeface carries the typeface with it, as a font cast member
// whose payload is one of these. TKKG 7, 9, 11, 13 and 14 and Loewenzahn 6 all
// ship one or more; TKKG 13's title card is set in an EurostileT-Medi that
// exists nowhere else.
//
// Two revisions are in the wild and they are not the same format. FreeType's
// PFR driver reads PFR0; everything these games carry says PFR1, and the
// difference is not the signature -- calling the resource PFR0 to get past the
// driver's magic check was tried and fails, because the character records are
// delta-coded where PFR0 writes fixed-size absolute ones, and the glyph
// programs are a nibble-coded instruction stream the driver cannot read at all.
//
// What this reader does is everything up to but not including the glyph
// programs: the header, the logical font, the physical font and the character
// records. That is enough for the metrics -- which character exists, how wide
// it is, and where its outline sits in the glyph section -- and metrics alone
// already keep a line's length, its breaks and its centring where the author
// put them while a substitute draws the letters.
//
// Reading the outlines themselves is a separate and much larger job. The
// format of this part was established against FreeType's own PFR sources
// (src/pfr/pfrload.c, pfrtypes.h) for the field order and flag values, and
// against the PFR1 reader in dirplayer-rs (GPL-3.0,
// vm-rust/src/director/chunks/pfr1) for the delta coding, then measured byte
// for byte on TKKG 13's font: its 225 records take up exactly the 866 bytes its
// physical font section has left for them, and their glyph pointers cover
// exactly the 6531 bytes of its glyph section.

struct PFRCharRecord {
	// The character this record is for. The font is usually a subset, so the
	// codes jump -- TKKG 13's run 0, 32..255, then 8226, 8230, 8240, 8249,
	// 8250, 8482.
	uint32 code = 0;
	// Advance width in font units, to be read against PFRPhysicalFont::metricsResolution
	int32 setWidth = 0;
	// Where this character's outline lives, relative to the glyph section
	uint32 gpsSize = 0;
	uint32 gpsOffset = 0;
};

struct PFRPhysicalFont {
	// Font units per em for the outlines and for the metrics. They are
	// usually the same number; TKKG 13 says 1000 for both.
	uint16 outlineResolution = 2048;
	uint16 metricsResolution = 2048;

	int16 xMin = 0;
	int16 yMin = 0;
	int16 xMax = 0;
	int16 yMax = 0;

	// The flags byte, with the same bits FreeType's PFR_PHY_* constants name
	byte flags = 0;

	// The width every character shares, when the font is not proportional
	int16 standardSetWidth = 0;

	Common::String fontId;
	Common::String familyName;
	Common::String styleName;

	// From the auxiliary records, and only if they carried them. The descent is
	// a positive magnitude, not a negative coordinate -- TKKG 13 says 208 where
	// its bounding box says -208. The third number is the one FreeType's reader
	// calls the leading, but measured it is 1078, which is exactly ascent plus
	// descent: it reads as the whole line's height rather than as anything
	// added on top of it, so do not add it to the other two.
	int16 ascent = 0;
	int16 descent = 0;
	int16 lineHeight = 0;

	// The alignment zones and the stem standards. A glyph reader needs these;
	// they are also the clearest sign that a resource has been understood --
	// TKKG 13's zones come out -6, 0, 466, 472, 666, 672, which is a baseline,
	// an x height and a cap height.
	Common::Array<int16> blueValues;
	byte blueFuzz = 0;
	byte blueScale = 0;
	int16 stdVW = 0;
	int16 stdHW = 0;

	// Private records out of the auxiliary block. Nothing here reads them yet;
	// they steer how the glyph programs are decoded, so they are picked up now
	// rather than parsed a second time later.
	byte privateMode = 4;
	byte privateByte28 = 0;
	byte privateByte29 = 0;
	int32 privateFlags = 0;
	bool hasLineSpacing = false;
	int16 lineSpacing = 0;

	Common::Array<PFRCharRecord> chars;

	bool isProportional() const { return (flags & 0x04) != 0; }
};

struct PFRLogicalFont {
	// Font units to output units, in 8.8 fixed point. TKKG 13's is the
	// identity, 256 0 0 256.
	int32 matrix[4] = { 256, 0, 0, 256 };
	uint32 size = 0;
	uint32 offset = 0;
};

struct PFRFont {
	// 1 for PFR1, 0 for PFR0
	uint16 version = 0;
	uint16 headerSize = 0;

	// The directory is a count and a (size, offset) pair per logical font. PFR1
	// is allowed to leave it too small to hold one, and then the record is the
	// start of the section instead -- see loadPFR().
	uint32 logFontDirSize = 0;
	uint32 logFontDirOffset = 0;

	uint32 logFontSectionSize = 0;
	uint32 logFontSectionOffset = 0;
	uint32 physFontSectionSize = 0;
	uint32 physFontSectionOffset = 0;
	uint32 gpsSectionSize = 0;
	uint32 gpsSectionOffset = 0;

	byte maxBlueValues = 0;
	byte maxXOrus = 0;
	byte maxYOrus = 0;
	byte physFontMaxSizeHigh = 0;
	bool invertBitmap = false;
	bool blackPixel = false;
	uint16 numPhysFonts = 0;
	uint16 maxChars = 0;

	Common::Array<PFRLogicalFont> logicalFonts;
	PFRPhysicalFont physical;
};

// ---------------------------------------------------------------------------
// Glyph outlines
// ---------------------------------------------------------------------------

enum PFRPathOp {
	kPFRMoveTo,
	kPFRLineTo,
	// A cubic curve: PFR's outlines are cubic, which is why they go to
	// FreeType as a Type 1 rather than as a TrueType.
	kPFRCurveTo,
	kPFRClose
};

struct PFRPathCmd {
	PFRPathOp op = kPFRMoveTo;
	int32 x = 0;
	int32 y = 0;
	int32 x1 = 0;
	int32 y1 = 0;
	int32 x2 = 0;
	int32 y2 = 0;
};

struct PFRContour {
	Common::Array<PFRPathCmd> cmds;
};

struct PFRGlyph {
	uint32 code = 0;
	int32 setWidth = 0;

	// The control grid the glyph's own coordinates are measured against: the
	// stem edges across and the alignment heights up. They read as the
	// typeface's own features -- a capital H comes out with two stems of equal
	// width placed symmetrically, a capital A with none at all because it has
	// no upright, and an o with its overshoot above and below the lines the
	// font's blue zones name.
	Common::Array<int16> controlX;
	Common::Array<int16> controlY;

	Common::Array<PFRContour> contours;
};

/**
 * Decode one glyph's program into outlines.
 *
 * @param font  the font it belongs to, for the control tables and the metrics
 * @param data  the glyph program, which is font data plus the character
 *              record's glyph offset, for the record's glyph size
 */
bool decodePFRGlyph(const PFRFont &font, const byte *gps, uint32 gpsSize,
		uint32 glyphOffset, uint32 glyphSize, PFRGlyph &glyph);

/**
 * Write a parsed resource out as a Type 1 font, which the font library can
 * read and this cannot.
 *
 * Type 1 rather than TrueType because these outlines are cubic, as Type 1
 * charstrings are; the glyphs are named after their code points so that
 * FreeType can build a Unicode character map from the names and reach the
 * characters an eight-bit encoding vector would leave out.
 *
 * @param gps       the glyph section, as decodePFRGlyph() takes it
 * @param fontName  the name the movie knows the font by, for the PostScript name
 * @return a stream the caller owns, or nullptr if no glyph would decode
 */
Common::SeekableReadStream *buildType1FromPFR(const PFRFont &font, const byte *gps,
		const Common::String &fontName);

/**
 * Read a Portable Font Resource as far as its metrics.
 *
 * Returns false and leaves the font half-filled if the resource is not one, or
 * if any field inside it points outside the data -- the bytes come out of a
 * game file, so nothing in here is taken on trust.
 */
bool loadPFR(const byte *data, uint32 size, PFRFont &font);

} // End of namespace Director

#endif
