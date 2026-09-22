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

// One section of the document: its id, and where its body starts and ends.
struct Section {
	uint32 id = 0;
	uint32 count = 0;
	uint32 start = 0;
	uint32 end = 0;
};

// Walk the section chain. A section is 0x03, a 4-digit hex id, an 8-digit hex
// length and an 8-digit hex count; its body runs from +21 to +20+length, and
// the next section starts there.
static Common::Array<Section> readSections(const Common::Array<byte> &data) {
	Common::Array<Section> out;
	uint32 size = data.size();

	uint32 pos = 20;
	while (pos + 21 <= size && !(data[pos] == 0x03 && isHexRun(data, pos + 1, 20)))
		pos++;

	while (pos + 21 <= size && data[pos] == 0x03 && isHexRun(data, pos + 1, 20)) {
		Section s;
		s.id = hexField(data, pos + 1, 4);
		uint32 len = hexField(data, pos + 5, 8);
		s.count = hexField(data, pos + 13, 8);
		if (len < 1 || pos + 20 + len > size)
			break;

		s.start = pos + 21;
		s.end = pos + 20 + len;
		out.push_back(s);
		pos += 20 + len;
	}

	return out;
}

// The numbers in a section body. A value is 0x01 or 0x02 followed by hex
// digits; 0x81/0x82 repeat the one before them once or twice, and 0xc1/0xc2
// repeat it as often as the byte after them says. A blob (0x00 length ','
// bytes) counts as one value and is skipped here -- only the font table and
// the text hold those, and both are read on their own.
static Common::Array<int32> sectionValues(const Common::Array<byte> &data, const Section &sec) {
	Common::Array<int32> out;
	uint32 i = sec.start;

	while (i < sec.end) {
		byte c = data[i];

		if (c == 0x01 || c == 0x02) {
			uint32 j = i + 1;
			bool negative = (j < sec.end && data[j] == '-');
			if (negative)
				j++;

			int32 value = 0;
			uint32 digits = 0;
			while (j < sec.end && isHexDigit(data[j]) && digits < 8) {
				value = value * 16 + hexValue(data[j]);
				j++;
				digits++;
			}

			out.push_back(negative ? -value : value);
			i = j;
		} else if (c == 0x81 || c == 0x82) {
			int32 last = out.empty() ? 0 : out.back();
			for (int n = c & 0xf; n > 0; n--)
				out.push_back(last);
			i++;
		} else if ((c == 0xc1 || c == 0xc2) && i + 1 < sec.end) {
			int32 last = out.empty() ? 0 : out.back();
			for (int n = data[i + 1]; n > 0; n--)
				out.push_back(last);
			i += 2;
		} else if (c == 0x00) {
			uint32 j = i + 1;
			uint32 len = 0;
			uint32 digits = 0;
			while (j < sec.end && isHexDigit(data[j]) && digits < 6) {
				len = len * 16 + hexValue(data[j]);
				j++;
				digits++;
			}

			if (digits == 0 || j >= sec.end || data[j] != ',')
				break;

			out.push_back(0);
			i = j + 1 + len;
		} else {
			i++;
		}
	}

	return out;
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

// The Pascal strings in a section body, one per blob that holds one. The font
// table keeps two blobs per font, the name in the first and an empty one after
// it, so only the ones that read as a name are taken.
static Common::Array<Common::String> sectionNames(const Common::Array<byte> &data, const Section &sec) {
	Common::Array<Common::String> out;
	uint32 i = sec.start;

	while (i < sec.end) {
		if (data[i] != 0x00) {
			i++;
			continue;
		}

		uint32 j = i + 1;
		uint32 len = 0;
		uint32 digits = 0;
		while (j < sec.end && isHexDigit(data[j]) && digits < 6) {
			len = len * 16 + hexValue(data[j]);
			j++;
			digits++;
		}

		if (digits == 0 || j >= sec.end || data[j] != ',' || j + 1 + len > sec.end)
			break;

		const byte *blob = &data[j + 1];
		if (len > 1 && blob[0] > 0 && blob[0] < len)
			out.push_back(Common::String((const char *)blob + 1, blob[0]));

		i = j + 1 + len;
	}

	return out;
}

// The styles the document was authored with. Section 0006 holds one record per
// style behind a single leading value, so the record size follows from the
// count; it is 81 in a document Director 7 wrote and 83 or 84 in the ones the
// games ship, and from 83 on every field from the colour on sits two places
// higher. Section 0004 pairs a character offset with a style number, 0008
// names the fonts, 0000 carries the member's background colour and 0007 the
// paragraph alignment. Field map and evidence: memory director-paige-style-runs.
bool readXMEDStyles(const Common::Array<byte> &data, StyleRuns &out) {
	Common::Array<Section> sections = readSections(data);
	if (sections.empty())
		return false;

	Common::Array<Common::String> fonts;
	Common::Array<int32> styleValues, runValues, paraRunValues, paraValues;
	uint32 styleCount = 0, paraCount = 0;

	for (auto &sec : sections) {
		switch (sec.id) {
		case 0x0000: {
				Common::Array<int32> v = sectionValues(data, sec);
				if (v.size() > 33) {
					out.bgR = (v[31] >> 8) & 0xff;
					out.bgG = (v[32] >> 8) & 0xff;
					out.bgB = (v[33] >> 8) & 0xff;
				}
			}
			break;
		case 0x0004:
			runValues = sectionValues(data, sec);
			break;
		case 0x0005:
			paraRunValues = sectionValues(data, sec);
			break;
		case 0x0006:
			styleValues = sectionValues(data, sec);
			styleCount = sec.count;
			break;
		case 0x0007:
			paraValues = sectionValues(data, sec);
			paraCount = sec.count;
			break;
		case 0x0008:
			fonts = sectionNames(data, sec);
			break;
		default:
			break;
		}
	}

	// Alignment lives in the paragraph record that the first paragraph run
	// points at -- not in the first record, which the probe showed staying at
	// the default while the second one carried the change.
	if (paraCount && paraValues.size() >= paraCount && paraRunValues.size() >= 2) {
		uint32 paraSize = paraValues.size() / paraCount;
		uint32 index = (uint32)MAX<int32>(0, paraRunValues[1]);
		if (paraSize && index < paraCount)
			out.align = paraValues[index * paraSize];
	}

	if (!styleCount || styleValues.size() < styleCount + 1)
		return false;

	uint32 recordSize = (styleValues.size() - 1) / styleCount;
	if (recordSize * styleCount + 1 != styleValues.size() || recordSize < 45)
		return false;

	uint32 shift = recordSize >= 83 ? 2 : 0;

	Common::Array<TextStyle> styles;
	for (uint32 i = 0; i < styleCount; i++) {
		const int32 *rec = &styleValues[1 + i * recordSize];
		TextStyle style;

		style.font = (rec[0] >= 0 && (uint32)rec[0] < fonts.size()) ? fonts[rec[0]] : Common::String();
		style.size = (uint16)(rec[19 + shift] >> 16);
		style.r = (rec[9 + shift] >> 8) & 0xff;
		style.g = (rec[10 + shift] >> 8) & 0xff;
		style.b = (rec[11 + shift] >> 8) & 0xff;
		style.slant = (rec[40 + shift] ? Graphics::kMacFontBold : 0)
				| (rec[41 + shift] ? Graphics::kMacFontItalic : 0)
				| (rec[42 + shift] ? Graphics::kMacFontUnderline : 0);

		if (!style.size)
			style.size = 12;

		styles.push_back(style);
	}

	// Section 0004 is a flat list of (character offset, style number) pairs,
	// with a last entry one past the end of the text.
	for (uint32 i = 0; i + 1 < runValues.size(); i += 2) {
		uint32 index = (uint32)runValues[i + 1];
		if (index >= styles.size())
			continue;

		StyleRun run;
		run.offset = (uint32)MAX<int32>(0, runValues[i]);
		run.style = styles[index];
		out.runs.push_back(run);
	}

	return !out.runs.empty();
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

// The document names its fonts ("Arial", "AvantGarde", "Cooper Black"), so look
// them up the way a text member's Lingo font assignment does. registerFontName()
// would coin a fresh id instead, one the font manager has no _fontInfo for: it
// warned about every lookup and fell back to Geneva, whose glyph table has no
// umlauts, which turned TKKG 7's Steckbrief into "Schlo? Hohenblaubl?then".
// A name the manager does not know resolves to the system font, which draws
// them.
static int fontIdFor(const Common::String &name) {
	if (name.empty())
		return Graphics::kMacFontSystem;

	return g_director->_wm->_fontMan->getFontIdByName(name);
}

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
	_ftext = source._ftext;
	_styles = source._styles;
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

			// Hand MacText the authored styles the way Stxt does, one
			// formatting code per run (see stxt.cpp).
			TextXtra::StyleRuns styles;
			if (TextXtra::readXMEDStyles(data, styles)) {
				_styles = styles;
				_ftext.clear();

				for (uint i = 0; i < styles.runs.size(); i++) {
					uint32 from = styles.runs[i].offset;
					if (from >= _text.size())
						break;

					uint32 to = (i + 1 < styles.runs.size()) ? styles.runs[i + 1].offset : _text.size();
					to = MIN<uint32>(to, _text.size());
					if (to <= from)
						continue;

					const TextXtra::TextStyle &s = styles.runs[i].style;
					int fontId = fontIdFor(s.font);

					// The colour components travel as 16-bit fields whose low
					// byte MacText keeps (mactext-canvas.cpp: findBestColor(
					// palinfo1 & 0xff, ...)), so pass the 8-bit value itself.
					_ftext += Common::String::format("\001\016%04x%02x%04x%04x%04x%04x",
							fontId, s.slant, s.size, s.r, s.g, s.b);

					for (uint32 c = from; c < to; c++) {
						// A \001 in the text has to be doubled, or MacText
						// reads it as the start of a code of our own.
						if (_text[c] == '\001')
							_ftext += '\001';
						_ftext += _text[c];
					}

					debugC(4, kDebugText, "TextXtraCastMember::load(): run at %d: font '%s' %d pt, slant %d, RGB(%d, %d, %d)",
							from, s.font.c_str(), s.size, s.slant, s.r, s.g, s.b);
				}

				debugC(3, kDebugText, "TextXtraCastMember::load(): %d style runs, background RGB(%d, %d, %d), align %d",
						(int)styles.runs.size(), styles.bgR, styles.bgG, styles.bgB, styles.align);
			}
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

	// The document says which font, size, colour and face each run carries and
	// what the member's background is; load() turned that into _ftext. Without
	// it -- a document whose styles would not read -- fall back to the plain
	// text in the system font.
	Graphics::MacFont macFont(1, 12);
	if (!_styles.runs.empty()) {
		const TextXtra::TextStyle &first = _styles.runs[0].style;
		macFont = Graphics::MacFont(fontIdFor(first.font), first.size, first.slant);
	}

	Graphics::TextAlign align = Graphics::kTextAlignLeft;
	if (_styles.align == 1)
		align = Graphics::kTextAlignCenter;
	else if (_styles.align == 2)
		align = Graphics::kTextAlignRight;

	uint32 fg = g_director->_wm->_colorBlack;
	uint32 bg = g_director->_wm->_colorWhite;
	if (!_styles.runs.empty()) {
		const TextXtra::TextStyle &first = _styles.runs[0].style;
		fg = g_director->_wm->findBestColor(first.r, first.g, first.b);
		bg = g_director->_wm->findBestColor(_styles.bgR, _styles.bgG, _styles.bgB);
	}

	Graphics::MacText *widget = new Graphics::MacText(g_director->getCurrentWindow()->getMacWindow(),
			bbox.left, bbox.top, bbox.width(), bbox.height(), g_director->_wm,
			_ftext.empty() ? _text : _ftext, &macFont,
			fg, bg, _initialRect.width(), align);
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
