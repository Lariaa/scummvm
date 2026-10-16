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

		// Only hand on what the font library will recognise. FreeType's PFR
		// driver takes the file if it starts with "PFR1", its version is at
		// most 4 and the two bytes after that are CR/LF; TKKG 13's
		// EurostileT-Medi is version 4 and answers all three. Anything else
		// here is a format nobody has met yet, and guessing would mean handing
		// FreeType a blob and reading the warning afterwards.
		if (size < 8 || READ_BE_UINT32(buf) != MKTAG('P', 'F', 'R', '1')) {
			warning("FontXtraCastMember::load(): cast %d ('%s') holds %d bytes that are not a Portable Font Resource",
					_castId, _fontName.c_str(), size);
			free(buf);
			break;
		}

		_fontId = g_director->_wm->_fontMan->registerFontName(_fontName);
		g_director->_wm->_fontMan->registerFontData(_fontId,
				new Common::MemoryReadStream(buf, size, DisposeAfterUse::YES));
		_registered = true;

		debugC(3, kDebugText, "FontXtraCastMember::load(): cast %d: '%s' registered as font id %d, %d bytes of PFR",
				_castId, _fontName.c_str(), _fontId, size);
		break;
	}
}

Common::String FontXtraCastMember::formatInfo() {
	return Common::String::format("font: \"%s\", id: %d, registered: %d",
			_fontName.c_str(), _fontId, _registered ? 1 : 0);
}

} // End of namespace Director
