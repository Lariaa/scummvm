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

// Turn a decoded Portable Font Resource into a font the font library can read.
//
// Nothing here rasterises anything. The glyphs come out of pfrglyph.cpp as
// outlines in font units, and the shortest way from there to something on
// screen is to write them out as a Type 1 font and let FreeType do the rest --
// sizing, hinting and anti-aliasing all come for free, and the whole of
// ScummVM's font machinery goes on working unchanged, because as far as it is
// concerned the movie simply carried a font file with it.
//
// Type 1 rather than TrueType because PFR's curves are cubic, which is what
// Type 1 charstrings hold; a TrueType would mean converting every curve to
// quadratics and losing a little of each one.
//
// Glyphs are named uniXXXX after the character they are for. That is not
// decoration: FreeType builds a Unicode character map out of the glyph names,
// so a font whose characters run past 255 -- and these do, as far as U+2122 --
// is still addressable by code point, which an eight-bit Type 1 encoding
// vector could not manage on its own.

#include "common/debug.h"
#include "common/memstream.h"
#include "common/textconsole.h"

#include "director/director.h"
#include "director/pfr.h"

namespace Director {

namespace {

// Type 1 charstring operators, by their numbers in the specification
enum {
	kOpHSbW      = 13,
	kOpRLineTo   = 5,
	kOpRMoveTo   = 21,
	kOpRRCurveTo = 8,
	kOpClosePath = 9,
	kOpEndChar   = 14
};

// The same obfuscation guards the private part of a Type 1 font and each
// charstring in it; only the key differs, and each run is prefixed with
// lenIV bytes of padding that the reader throws away.
class Eexec {
public:
	Eexec(uint16 key) : _r(key) {}

	void encrypt(const Common::Array<byte> &plain, Common::Array<byte> &out, int lenIV) {
		// The padding has to be bytes the reader will discard, and anything
		// does; a fixed run keeps the output the same from one run to the next,
		// which is worth more here than unpredictability.
		for (int i = 0; i < lenIV; i++)
			step(0x55, out);

		for (uint i = 0; i < plain.size(); i++)
			step(plain[i], out);
	}

private:
	void step(byte p, Common::Array<byte> &out) {
		byte c = p ^ (_r >> 8);
		_r = (uint16)((c + _r) * 52845 + 22719);
		out.push_back(c);
	}

	uint16 _r;
};

void putNumber(Common::Array<byte> &out, int32 v) {
	// Charstring numbers are written in whichever of four widths fits, and a
	// glyph is mostly numbers, so this is most of the size of a font.
	if (v >= -107 && v <= 107) {
		out.push_back((byte)(v + 139));
	} else if (v >= 108 && v <= 1131) {
		int32 d = v - 108;
		out.push_back((byte)((d >> 8) + 247));
		out.push_back((byte)(d & 0xff));
	} else if (v <= -108 && v >= -1131) {
		int32 d = -v - 108;
		out.push_back((byte)((d >> 8) + 251));
		out.push_back((byte)(d & 0xff));
	} else {
		out.push_back(255);
		out.push_back((byte)(v >> 24));
		out.push_back((byte)(v >> 16));
		out.push_back((byte)(v >> 8));
		out.push_back((byte)v);
	}
}

// One glyph as a Type 1 charstring: the advance width, then the outline in
// steps from point to point, which is the only form these operators take.
void buildCharstring(const PFRGlyph &glyph, Common::Array<byte> &out) {
	// hsbw sets the advance and puts the pen at (sbx, 0). Leaving the side
	// bearing at zero makes the first step of each contour its plain position.
	putNumber(out, 0);
	putNumber(out, glyph.setWidth);
	out.push_back(kOpHSbW);

	int32 penX = 0;
	int32 penY = 0;

	for (uint i = 0; i < glyph.contours.size(); i++) {
		const Common::Array<PFRPathCmd> &cmds = glyph.contours[i].cmds;
		if (cmds.empty())
			continue;

		for (uint j = 0; j < cmds.size(); j++) {
			const PFRPathCmd &cmd = cmds[j];

			switch (cmd.op) {
			case kPFRMoveTo:
				putNumber(out, cmd.x - penX);
				putNumber(out, cmd.y - penY);
				out.push_back(kOpRMoveTo);
				break;

			case kPFRLineTo:
				putNumber(out, cmd.x - penX);
				putNumber(out, cmd.y - penY);
				out.push_back(kOpRLineTo);
				break;

			case kPFRCurveTo:
				// Three steps, each from where the one before it landed
				putNumber(out, cmd.x1 - penX);
				putNumber(out, cmd.y1 - penY);
				putNumber(out, cmd.x2 - cmd.x1);
				putNumber(out, cmd.y2 - cmd.y1);
				putNumber(out, cmd.x - cmd.x2);
				putNumber(out, cmd.y - cmd.y2);
				out.push_back(kOpRRCurveTo);
				break;

			default:
				break;
			}

			if (cmd.op != kPFRClose) {
				penX = cmd.x;
				penY = cmd.y;
			}
		}

		out.push_back(kOpClosePath);
	}

	out.push_back(kOpEndChar);
}

void putString(Common::Array<byte> &out, const char *s) {
	for (; *s; s++)
		out.push_back((byte)*s);
}

Common::String glyphName(uint32 code) {
	if (!code)
		return ".notdef";

	// FreeType reads this name back into a code point, which is what makes the
	// characters above 255 reachable.
	return Common::String::format("uni%04X", code);
}

} // End of anonymous namespace

Common::SeekableReadStream *buildType1FromPFR(const PFRFont &font, const byte *gps,
		const Common::String &fontName) {
	const PFRPhysicalFont &phys = font.physical;

	// A PostScript name may not carry spaces or the characters that mean
	// something to the language, and these names have both -- Director marks an
	// embedded font by putting " *" on the end of it.
	Common::String psName;
	for (uint i = 0; i < fontName.size(); i++) {
		char c = fontName[i];
		if ((c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z') || (c >= '0' && c <= '9')
				|| c == '-' || c == '_')
			psName += c;
	}
	if (psName.empty())
		psName = "EmbeddedFont";

	// Decode every glyph first: a font with nothing in it is not worth writing,
	// and the count has to be known before the dictionary is opened.
	Common::Array<Common::String> names;
	Common::Array<Common::Array<byte> > charstrings;
	Common::Array<uint32> codes;
	uint withOutline = 0;

	for (uint i = 0; i < phys.chars.size(); i++) {
		const PFRCharRecord &rec = phys.chars[i];

		PFRGlyph glyph;
		glyph.code = rec.code;
		glyph.setWidth = rec.setWidth;

		// A glyph program of one byte or none is a character with no outline: the
		// space is one -- TKKG 13's is a single byte -- and so are the control
		// codes. It is still a character and needs a charstring to carry its
		// width, or the font library finds no glyph for it, falls back on the
		// missing-character glyph, and draws a box where the space should be.
		// The one byte is only the format byte, with no commands behind it.
		bool hasOutline = rec.gpsSize > 1
				&& decodePFRGlyph(font, gps, font.gpsSectionSize, rec.gpsOffset, rec.gpsSize, glyph);

		if (rec.gpsSize > 1 && !hasOutline) {
			// A program that would not decode is kept as an empty glyph rather
			// than dropped, for the same reason: a letter that draws nothing
			// is a smaller fault than one that draws a box.
			debugC(3, kDebugText, "buildType1FromPFR(): character %d would not decode, kept as an empty glyph",
					(int)rec.code);
			glyph.contours.clear();
		}

		if (hasOutline)
			withOutline++;

		Common::Array<byte> cs;
		buildCharstring(glyph, cs);
		names.push_back(glyphName(rec.code));
		charstrings.push_back(cs);
		codes.push_back(rec.code);
	}

	if (!withOutline) {
		warning("buildType1FromPFR(): '%s' yielded no glyphs", fontName.c_str());
		return nullptr;
	}

	// ---- the part that is not encrypted ----
	Common::Array<byte> clear;
	putString(clear, "%!PS-AdobeFont-1.0: ");
	putString(clear, psName.c_str());
	putString(clear, " 001.001\n");
	putString(clear, "11 dict begin\n/FontInfo 2 dict dup begin\n/FullName (");
	putString(clear, psName.c_str());
	putString(clear, ") readonly def\nend readonly def\n/FontName /");
	putString(clear, psName.c_str());
	putString(clear, " def\n/PaintType 0 def\n/FontType 1 def\n");

	// The matrix is what turns font units into the em square FreeType expects,
	// so it carries the resolution the resource declared rather than a guess.
	putString(clear, Common::String::format("/FontMatrix [%.8f 0 0 %.8f 0 0] readonly def\n",
			1.0 / phys.outlineResolution, 1.0 / phys.outlineResolution).c_str());
	putString(clear, Common::String::format("/FontBBox {%d %d %d %d} readonly def\n",
			phys.xMin, phys.yMin, phys.xMax, phys.yMax).c_str());

	// An eight-bit vector cannot hold these characters, and it does not have
	// to: the Unicode map FreeType builds from the glyph names covers them.
	putString(clear, "/Encoding 256 array\n0 1 255 {1 index exch /.notdef put} for\n");
	for (uint i = 0; i < codes.size(); i++) {
		if (codes[i] > 0 && codes[i] < 256)
			putString(clear, Common::String::format("dup %d /%s put\n",
					codes[i], glyphName(codes[i]).c_str()).c_str());
	}
	putString(clear, "readonly def\ncurrentdict end\ncurrentfile eexec\n");

	// ---- the part that is ----
	Common::Array<byte> priv;
	putString(priv, "dup /Private 8 dict dup begin\n");
	putString(priv, "/RD {string currentfile exch readstring pop} executeonly def\n");
	putString(priv, "/ND {noaccess def} executeonly def\n");
	putString(priv, "/NP {noaccess put} executeonly def\n");
	putString(priv, "/lenIV 4 def\n/password 5839 def\n");
	putString(priv, "/BlueValues [] ND\n");
	putString(priv, Common::String::format("/StdHW [%d] ND\n/StdVW [%d] ND\n",
			phys.stdHW ? phys.stdHW : 1, phys.stdVW ? phys.stdVW : 1).c_str());
	putString(priv, Common::String::format("end\n2 index /CharStrings %d dict dup begin\n",
			(int)names.size() + 1).c_str());

	// Every charstring is obfuscated on its own, with the key the format fixes
	// for them, and announced with the length the reader is to take.
	for (uint i = 0; i < names.size(); i++) {
		Common::Array<byte> enc;
		Eexec cipher(4330);
		cipher.encrypt(charstrings[i], enc, 4);

		putString(priv, Common::String::format("/%s %d RD ",
				names[i].c_str(), (int)enc.size()).c_str());
		for (uint j = 0; j < enc.size(); j++)
			priv.push_back(enc[j]);
		putString(priv, " ND\n");
	}

	putString(priv, "end\nend\nreadonly put\nnoaccess put\n");
	putString(priv, "dup /FontName get exch definefont pop\nmark currentfile closefile\n");

	Common::Array<byte> encPriv;
	Eexec outer(55665);
	outer.encrypt(priv, encPriv, 4);

	// ---- the three segments of a PFB ----
	Common::Array<byte> trailer;
	for (int i = 0; i < 8; i++) {
		for (int j = 0; j < 64; j++)
			trailer.push_back('0');
		trailer.push_back('\n');
	}
	putString(trailer, "cleartomark\n");

	uint32 total = 6 + clear.size() + 6 + encPriv.size() + 6 + trailer.size() + 2;
	byte *out = (byte *)malloc(total);
	if (!out)
		return nullptr;

	uint32 at = 0;

	// Each segment opens with 0x80, its kind, and its length, least
	// significant byte first -- the one place in the format that is not big
	// endian.
	const Common::Array<byte> *segments[3] = { &clear, &encPriv, &trailer };
	const byte kinds[3] = { 1, 2, 1 };

	for (int i = 0; i < 3; i++) {
		uint32 len = segments[i]->size();
		out[at++] = 0x80;
		out[at++] = kinds[i];
		out[at++] = (byte)(len & 0xff);
		out[at++] = (byte)((len >> 8) & 0xff);
		out[at++] = (byte)((len >> 16) & 0xff);
		out[at++] = (byte)((len >> 24) & 0xff);
		for (uint32 j = 0; j < len; j++)
			out[at++] = (*segments[i])[j];
	}

	out[at++] = 0x80;
	out[at++] = 3;

	debugC(3, kDebugText, "buildType1FromPFR(): '%s' written as a %d byte Type 1 with %d glyphs over %d units per em",
			fontName.c_str(), (int)at, (int)names.size(), (int)phys.outlineResolution);

	return new Common::MemoryReadStream(out, at, DisposeAfterUse::YES);
}

} // End of namespace Director
