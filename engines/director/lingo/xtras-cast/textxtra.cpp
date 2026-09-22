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

#include "graphics/macgui/macfontmanager.h"
#include "graphics/macgui/mactext.h"
#include "graphics/macgui/macwindowmanager.h"

#include "director/director.h"
#include "director/cast.h"
#include "director/util.h"
#include "director/castmember/xtra.h"
#include "director/lingo/lingo-the.h"
#include "director/lingo/xtras-cast/textxtra.h"
#include "director/window.h"

namespace Director {

namespace TextXtra {

static bool isHexDigit(byte c) {
	return (c >= '0' && c <= '9') || (c >= 'A' && c <= 'F') || (c >= 'a' && c <= 'f');
}

static uint hexValue(byte c) {
	if (c >= '0' && c <= '9')
		return c - '0';
	if (c >= 'A' && c <= 'F')
		return c - 'A' + 10;
	return c - 'a' + 10;
}

static bool isHexRun(const Common::Array<byte> &data, uint32 pos, uint32 digits) {
	if (pos + digits > data.size())
		return false;
	for (uint32 i = 0; i < digits; i++) {
		if (!isHexDigit(data[pos + i]))
			return false;
	}
	return true;
}

static uint32 hexField(const Common::Array<byte> &data, uint32 pos, uint32 digits) {
	uint32 value = 0;
	for (uint32 i = 0; i < digits; i++)
		value = value * 16 + hexValue(data[pos + i]);
	return value;
}

// XMED payload is an ASCII-hex serialized Hermes-Paige document:
// "FFFF" header, then sections, each 0x03, a 4-digit hex id, an 8-digit hex
// length and an 8-digit hex count, then a body; the next section starts
// 20 + length bytes after the 0x03. Section 0002 holds the text, as
// 0x00 <hex length> ',' <text>.
//
// A long text is split over several 0002 sections, and the count field then
// gives the character offset the piece starts at. TKKG 14's sound play lists
// need them: sc01 holds 4410 characters in two pieces, sc31 17819 in six.
// Reading only the first piece cut the list off mid-entry, so it no longer
// parsed and the scene held on its first frame waiting for a sequence that
// never started.
bool decodeXMED(const Common::Array<byte> &data, Common::String &text) {
	uint32 size = data.size();
	if (size < 4 || memcmp(data.data(), "FFFF", 4) != 0)
		return false;

	text.clear();

	// The header fields in front of the first section vary in length, so look
	// for the first thing that is shaped like a section header.
	uint32 pos = 20;
	while (pos + 21 <= size && !(data[pos] == 0x03 && isHexRun(data, pos + 1, 20)))
		pos++;

	while (pos + 21 <= size && data[pos] == 0x03 && isHexRun(data, pos + 1, 20)) {
		uint32 id = hexField(data, pos + 1, 4);
		uint32 sectionLen = hexField(data, pos + 5, 8);
		if (sectionLen < 1 || pos + 20 + sectionLen > size)
			break;

		if (id == 0x0002) {
			uint32 j = pos + 21;
			if (j < size && data[j] == 0x00) {
				j++;
				uint32 len = 0;
				int digits = 0;
				while (j < size && isHexDigit(data[j]) && digits < 6) {
					len = len * 16 + hexValue(data[j]);
					j++;
					digits++;
				}
				if (digits > 0 && j < size && data[j] == ',' && j + 1 + len <= size)
					text += Common::String((const char *)&data[j + 1], len);
			}
		}

		pos += 20 + sectionLen;
	}

	if (!text.empty())
		return true;

	// No usable section chain: fall back to the first blob that reads as text.
	for (uint32 i = 0; i + 2 < size; i++) {
		if (data[i] != 0x00)
			continue;

		uint32 j = i + 1;
		uint32 len = 0;
		int digits = 0;
		while (j < size && isHexDigit(data[j]) && digits < 6) {
			len = len * 16 + hexValue(data[j]);
			j++;
			digits++;
		}

		if (digits == 0 || len == 0 || j >= size || data[j] != ',')
			continue;
		j++;

		if (j + len > size)
			continue;

		uint32 printable = 0;
		for (uint32 k = 0; k < len; k++) {
			byte c = data[j + k];
			if ((c >= 0x20 && c <= 0x7e) || c >= 0x80 || c == '\r' || c == '\n' || c == '\t')
				printable++;
		}

		if (printable * 5 < len * 4)
			continue;

		text = Common::String((const char *)&data[j], len);
		return true;
	}
	return false;
}

// The document header in front of the text: "FFFF", a 16-digit hex field, then
// fields led by control bytes -- 0x01, 0x03, and a run of 0x02 ones. The second
// 0x02 field holds flags, in hex like everything else. Bit 0x100 is set when
// the text was typed on Windows: its high bytes are then Windows-1252, and
// without it they are Mac Roman, whatever the container is. Across the TKKG
// titles 104 Windows texts carry 101 and 792 Mac ones 1 or 10000001, with no
// exception either way; TKKG 7 has both kinds in one movie.
static bool readXMEDFlags(const Common::Array<byte> &data, uint32 &flags) {
	uint32 size = data.size();
	uint32 i = 20;
	if (size <= i || data[i] != 0x01)
		return false;

	while (i < size && data[i] != 0x03)
		i++;
	while (i < size && data[i] != 0x02)
		i++;
	if (i >= size)
		return false;

	// Skip the first 0x02 field, read the second one.
	i++;
	while (i < size && data[i] != 0x02)
		i++;
	if (i >= size)
		return false;
	i++;

	uint32 value = 0;
	int digits = 0;
	while (i < size && isHexDigit(data[i]) && digits < 8) {
		value = value * 16 + hexValue(data[i]);
		i++;
		digits++;
	}
	if (digits == 0 || i >= size || data[i] != 0x02)
		return false;

	flags = value;
	return true;
}

// 76-byte "text" payload observed in Physikus (D7): BE32 height at
// offset 36, width at offset 40, mirroring the authored xtraRect.
bool parseXtraData(const Common::Array<byte> &data, Common::Rect &rect) {
	if (data.size() < 44)
		return false;

	int32 h = READ_BE_INT32(&data[36]);
	int32 w = READ_BE_INT32(&data[40]);
	if (w <= 0 || h <= 0 || w > 0x4000 || h > 0x4000)
		return false;

	rect = Common::Rect((int16)w, (int16)h);
	return true;
}

CastMember *createCastMember(Cast *cast, uint16 castId, XtraCastMember *xtra) {
	return new TextXtraCastMember(cast, castId, *xtra);
}

} // End of namespace TextXtra

TextXtraCastMember::TextXtraCastMember(Cast *cast, uint16 castId, XtraCastMember &source)
		: CastMember(cast, castId) {
	// Not kCastText: the engine casts kCastText members to TextCastMember
	_type = kCastXtra;

	Common::Rect rect;
	if (TextXtra::parseXtraData(source.getXtraData(), rect))
		_initialRect = rect;
}

TextXtraCastMember::TextXtraCastMember(Cast *cast, uint16 castId, TextXtraCastMember &source)
		: CastMember(cast, castId) {
	_type = kCastXtra;
	_initialRect = source._initialRect;
	_boundingRect = source._boundingRect;
	_text = source._text;
	_loaded = source._loaded;
	if (cast == source._cast)
		_children = source._children;
}

void TextXtraCastMember::load() {
	if (_loaded)
		return;

	for (auto &it : _children) {
		if (it.tag != MKTAG('X', 'M', 'E', 'D'))
			continue;

		if (!_cast->getArchive()->hasResource(it.tag, it.index)) {
			warning("TextXtraCastMember::load(): XMED %d not found", it.index);
			break;
		}
		Common::SeekableReadStreamEndian *r = _cast->getArchive()->getResource(it.tag, it.index);
		Common::Array<byte> data(r->size());
		r->read(data.data(), r->size());
		delete r;

		Common::String text;
		if (TextXtra::decodeXMED(data, text)) {
			// The bytes belong to the machine the text was typed on, not to the
			// platform we present as, and not to the container either. TKKG 9
			// is a Windows game whose movies are Mac containers; decoded with
			// getPlatformEncoding() its credits turned "Übersetzung" into
			// "†bersetzung". The file's encoding fixed that, but TKKG 7 is all
			// Windows containers and still holds Mac texts: its Steckbrief read
			// "Schlo§" and "t?tlichen". The header says which it is; without a
			// readable header, fall back to the file.
			Common::CodePage encoding = _cast->getFileEncoding();
			uint32 flags;
			if (TextXtra::readXMEDFlags(data, flags))
				encoding = getEncoding((flags & 0x100) ? Common::kPlatformWindows : Common::kPlatformMacintosh,
						g_director->getLanguage());
			_text = Common::U32String(text, encoding);
			debugC(3, kDebugText, "TextXtraCastMember::load(): XMED %d: '%s'", it.index, text.c_str());
		} else {
			warning("TextXtraCastMember::load(): XMED %d not decoded", it.index);
		}
		break;
	}

	_loaded = true;
}

Graphics::MacWidget *TextXtraCastMember::createWidget(Common::Rect &bbox, Channel *channel, SpriteType spriteType) {
	if (!bbox.width() || !bbox.height())
		return nullptr;

	// TODO: Parse the authored Paige style runs (font, size, colours)
	Graphics::MacFont macFont(1, 12);
	Graphics::MacText *widget = new Graphics::MacText(g_director->getCurrentWindow()->getMacWindow(),
			bbox.left, bbox.top, bbox.width(), bbox.height(), g_director->_wm, _text, &macFont,
			g_director->_wm->_colorWhite, g_director->_wm->_colorBlack, _initialRect.width(),
			Graphics::kTextAlignLeft);
	widget->draw();
	return widget;
}

bool TextXtraCastMember::hasField(int field) {
	switch (field) {
	case kTheText:
		return true;
	default:
		break;
	}
	return CastMember::hasField(field);
}

Datum TextXtraCastMember::getField(int field) {
	Datum d;

	switch (field) {
	case kTheCastType:
	case kTheType:
		d = Common::String("text");
		d.type = SYMBOL;
		break;
	case kTheText:
		load();
		d = _text.encode(Common::kUtf8);
		break;
	default:
		d = CastMember::getField(field);
		break;
	}

	return d;
}

void TextXtraCastMember::setField(int field, const Datum &d) {
	switch (field) {
	case kTheText:
		_text = Common::U32String(d.asString(), Common::kUtf8);
		_loaded = true;
		setModified(true);
		return;
	default:
		break;
	}

	CastMember::setField(field, d);
}

Common::String TextXtraCastMember::formatInfo() {
	return Common::String::format("initialRect: %dx%d, text: \"%s\"",
		_initialRect.width(), _initialRect.height(), Common::toPrintable(_text.encode()).c_str());
}

} // End of namespace Director
