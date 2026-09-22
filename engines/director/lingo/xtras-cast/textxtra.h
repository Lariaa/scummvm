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

#ifndef DIRECTOR_LINGO_XTRAS_CAST_TEXTXTRA_H
#define DIRECTOR_LINGO_XTRAS_CAST_TEXTXTRA_H

#include "common/array.h"
#include "common/rect.h"
#include "common/str.h"
#include "common/ustr.h"

#include "director/castmember/castmember.h"

namespace Director {

class XtraCastMember;

namespace TextXtra {

// How one stretch of the text is drawn, as the document was authored.
struct TextStyle {
	Common::String font;
	uint16 size = 12;
	byte slant = 0;
	byte r = 0, g = 0, b = 0;
};

struct StyleRun {
	uint32 offset = 0;	// first character the style applies to
	TextStyle style;
};

struct StyleRuns {
	Common::Array<StyleRun> runs;
	byte bgR = 0xff, bgG = 0xff, bgB = 0xff;	// the member's own background
	int align = 0;								// 0 left, 1 centre, 2 right
};

CastMember *createCastMember(Cast *cast, uint16 castId, XtraCastMember *xtra);
bool decodeXMED(const Common::Array<byte> &data, Common::String &text);
bool readXMEDStyles(const Common::Array<byte> &data, StyleRuns &out);
bool parseXtraData(const Common::Array<byte> &data, Common::Rect &rect);

} // End of namespace TextXtra

// "Text" Asset Xtra cast member (D7+), display-only in the Projector.
// Successor of the RTE rich text pipeline: both serialize Hermes-Paige
// documents (https://github.com/nmatavka/Hermes-Paige), RTE as binary
// chunks, this one as an ASCII-hex encoded 'XMED' child resource.
class TextXtraCastMember : public CastMember {
public:
	TextXtraCastMember(Cast *cast, uint16 castId, XtraCastMember &source);
	TextXtraCastMember(Cast *cast, uint16 castId, TextXtraCastMember &source);

	CastMember *duplicate(Cast *cast, uint16 castId) override { return (CastMember *)(new TextXtraCastMember(cast, castId, *this)); }

	void load() override;

	Graphics::MacWidget *createWidget(Common::Rect &bbox, Channel *channel, SpriteType spriteType) override;

	bool hasField(int field) override;
	Datum getField(int field) override;
	void setField(int field, const Datum &value) override;

	Common::String formatInfo() override;

	Common::U32String _text;
	// The same text with MacText's formatting codes in front of every style
	// run, built in load() from the document's own styles.
	Common::U32String _ftext;
	TextXtra::StyleRuns _styles;
};

} // End of namespace Director

#endif
