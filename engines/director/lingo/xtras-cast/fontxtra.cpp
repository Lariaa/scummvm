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

#include "common/endian.h"
#include "common/memstream.h"

#include "graphics/macgui/macfontmanager.h"
#include "graphics/macgui/macwindowmanager.h"

#include "director/director.h"
#include "director/cast.h"
#include "director/util.h"
#include "director/castmember/xtra.h"
#include "director/pfr.h"
#include "director/lingo/xtras-cast/fontxtra.h"

namespace Director {

namespace FontXtra {

// The payload names the font twice: the name the document's style runs use --
// with Director's trailing " *" for an embedded font -- and the plain family
// name after it. Both are NUL terminated and start at offset 16, after a
// count, the payload's own length and a flag word.
static bool readNames(const Common::Array<byte> &data, Common::String &display, Common::String &family) {
	if (data.size() < 20)
		return false;

	uint32 i = 16;
	while (i < data.size() && data[i])
		display += (char)data[i++];

	if (i >= data.size() || display.empty())
		return false;

	i++;
	while (i < data.size() && data[i])
		family += (char)data[i++];

	return true;
}

// Pick out the advance widths worth handing on.
//
// These fonts are subsets. TKKG 13's carries the letters of its title card and
// little else: of its 224 coded characters, 99 point at the same glyph as code
// 0, the one a character the font does not have falls back on -- every German
// umlaut and the sharp s among them. Their recorded width is that fallback
// glyph's, not the letter's, so handing it on would set a line of German text
// in widths that belong to nothing. Those characters are better left to the
// substitute entirely, which is why they are dropped here rather than scaled
// later.
void collectAdvanceWidths(const PFRFont &pfr, Common::HashMap<uint32, int32> &widths) {
	bool haveNotdef = false;
	uint32 notdefSize = 0;
	uint32 notdefOffset = 0;

	for (uint i = 0; i < pfr.physical.chars.size(); i++) {
		const PFRCharRecord &c = pfr.physical.chars[i];

		// Code 0 is the fallback glyph itself. It has no code point of its own,
		// and it is the yardstick for everything that shares it.
		if (c.code == 0) {
			haveNotdef = true;
			notdefSize = c.gpsSize;
			notdefOffset = c.gpsOffset;
			continue;
		}

		if (haveNotdef && c.gpsSize == notdefSize && c.gpsOffset == notdefOffset)
			continue;

		if (c.setWidth > 0)
			widths[c.code] = c.setWidth;
	}
}

CastMember *createCastMember(Cast *cast, uint16 castId, XtraCastMember *xtra) {
	return new FontXtraCastMember(cast, castId, *xtra);
}

} // End of namespace FontXtra

FontXtraCastMember::FontXtraCastMember(Cast *cast, uint16 castId, XtraCastMember &source)
		: CastMember(cast, castId) {
	_type = kCastXtra;

	Common::String family;
	FontXtra::readNames(source.getXtraData(), _fontName, family);
}

FontXtraCastMember::FontXtraCastMember(Cast *cast, uint16 castId, FontXtraCastMember &source)
		: CastMember(cast, castId) {
	_type = kCastXtra;
	_fontName = source._fontName;
	_fontId = source._fontId;
	_registered = source._registered;
	_widthsRegistered = source._widthsRegistered;
	if (cast == source._cast)
		_children = source._children;
}

void FontXtraCastMember::load() {
	if (_loaded)
		return;

	_loaded = true;

	if (_fontName.empty()) {
		warning("FontXtraCastMember::load(): cast %d carries a font with no name", _castId);
		return;
	}

	for (auto &it : _children) {
		if (it.tag != MKTAG('X', 'M', 'E', 'D'))
			continue;

		if (!_cast->getArchive()->hasResource(it.tag, it.index)) {
			warning("FontXtraCastMember::load(): XMED %d not found", it.index);
			break;
		}

		Common::SeekableReadStreamEndian *r = _cast->getArchive()->getResource(it.tag, it.index);
		uint32 size = r->size();
		byte *buf = (byte *)malloc(size);
		r->read(buf, size);
		delete r;

		// A Portable Font Resource opens with its four-character signature, a
		// version of at most 4, and CR/LF. Only hand the font library something
		// that answers all three; anything else is a format nobody has met yet,
		// and guessing would mean handing FreeType a blob and reading the
		// warning afterwards.
		bool pfr0 = size >= 12 && READ_BE_UINT32(buf) == MKTAG('P', 'F', 'R', '0');
		bool pfr1 = size >= 12 && READ_BE_UINT32(buf) == MKTAG('P', 'F', 'R', '1');
		if ((!pfr0 && !pfr1) || READ_BE_UINT16(buf + 4) > 4
				|| READ_BE_UINT16(buf + 6) != 0x0D0A || READ_BE_UINT16(buf + 8) < 58) {
			warning("FontXtraCastMember::load(): cast %d ('%s') holds %d bytes that are not a Portable Font Resource",
					_castId, _fontName.c_str(), size);
			free(buf);
			break;
		}

		_fontId = g_director->_wm->_fontMan->registerFontName(_fontName);

		// Whatever else happens, the metrics are readable, and they are worth
		// having on their own: with them a substitute lays the line out to the
		// authored length even though the letters have the wrong shape.
		PFRFont pfr;
		Common::HashMap<uint32, int32> widths;
		bool parsed = loadPFR(buf, size, pfr);
		if (parsed) {
			FontXtra::collectAdvanceWidths(pfr, widths);
			if (!widths.empty()) {
				// The ascent and the descent come from the auxiliary records
				// and may be absent; 0 then leaves the substitute's own.
				g_director->_wm->_fontMan->registerFontAdvanceWidths(_fontId, widths,
						pfr.physical.metricsResolution,
						pfr.physical.ascent, pfr.physical.descent);
				_widthsRegistered = true;
			}
		} else {
			warning("FontXtraCastMember::load(): cast %d ('%s') carries a PFR that would not read",
					_castId, _fontName.c_str());
		}

		// The resource itself is no use to the font library: FreeType's PFR
		// driver answers to "PFR0" and refuses "PFR1" outright, and the two
		// formats differ well past the signature -- the character records are
		// delta-coded where the driver expects fixed-size absolute ones, and
		// the glyph programs are a nibble-coded stream it cannot read at all.
		// What goes on the shelf instead is the same font written out as a
		// Type 1, which it reads perfectly well; from there on the movie's text
		// is drawn in the face the author chose, by the same path as any other
		// font file a game carries.
		//
		// A genuine PFR0 is handed over as it stands, since the driver was
		// written for it.
		if (pfr0) {
			g_director->_wm->_fontMan->registerFontData(_fontId,
					new Common::MemoryReadStream(buf, size, DisposeAfterUse::YES));
			_registered = true;
		} else if (parsed) {
			Common::SeekableReadStream *type1 = buildType1FromPFR(pfr,
					buf + pfr.gpsSectionOffset, _fontName);
			if (type1) {
				g_director->_wm->_fontMan->registerFontData(_fontId, type1);
				_registered = true;
			}
			free(buf);
		} else {
			free(buf);
		}

		debugC(3, kDebugText, "FontXtraCastMember::load(): cast %d: '%s' is font id %d, %d bytes of %s; glyphs %s, %d advance widths over %d units per em",
				_castId, _fontName.c_str(), _fontId, size, pfr0 ? "PFR0" : "PFR1",
				_registered ? "handed to the font library" : "not readable here",
				(int)widths.size(), (int)pfr.physical.metricsResolution);
		break;
	}
}

Common::String FontXtraCastMember::formatInfo() {
	return Common::String::format("font: \"%s\", id: %d, registered: %d, widths: %d",
			_fontName.c_str(), _fontId, _registered ? 1 : 0, _widthsRegistered ? 1 : 0);
}

} // End of namespace Director
