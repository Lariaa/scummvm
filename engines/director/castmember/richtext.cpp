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

#include "common/stream.h"
#include "graphics/macgui/macfontmanager.h"
#include "graphics/macgui/mactext.h"
#include "graphics/macgui/macwidget.h"

#include "director/director.h"
#include "director/cast.h"
#include "director/channel.h"
#include "director/images.h"
#include "director/movie.h"
#include "director/picture.h"
#include "director/rte.h"
#include "director/score.h"
#include "director/window.h"
#include "director/castmember/richtext.h"
#include "director/lingo/lingo-the.h"

namespace Director {

RichTextCastMember::RichTextCastMember(Cast *cast, uint16 castId, Common::SeekableReadStreamEndian &stream, uint16 version)
		: CastMember(cast, castId, stream) {

	_pf32 = Graphics::PixelFormat(4, 8, 8, 8, 8, 24, 16, 8, 0);

	if (version >= kFileVer500 && version < kFileVer1100) {
		if (debugChannelSet(5, kDebugLoading)) {
			debugC(5, kDebugLoading, "RichTextCastMember():");
			stream.hexdump(stream.size());
		}

		_initialRect = Movie::readRect(stream);
		_boundingRect = Movie::readRect(stream);
		_antialiasFlag = stream.readByte();
		_cropFlags = stream.readByte();
		_scrollPos = stream.readUint16BE();
		_antialiasFontSize = stream.readUint16BE();
		_displayHeight = stream.readUint16BE();

		uint8 r = 0, g = 0, b = 0;
		stream.readByte(); // skip one byte
		r = stream.readByte();
		g = stream.readByte();
		b = stream.readByte();
		_foreColor = _pf32.RGBToColor(r, g, b);

		r = (stream.readUint16BE() >> 8);
		g = (stream.readUint16BE() >> 8);
		b = (stream.readUint16BE() >> 8);
		_bgColor = _pf32.RGBToColor(r, g, b);

		debugC(3, kDebugLoading, "  RichTextCastMember(): initialRect: [%s], boundingRect: [%s], antialiasFlag: 0x%02x, cropFlags: 0x%02x, scrollPos: %d, antialiasFontSize: %d, displayHeight: %d",
			_initialRect.toString().c_str(),
			_boundingRect.toString().c_str(),
			_antialiasFlag,
			_cropFlags,
			_scrollPos,
			_antialiasFontSize,
			_displayHeight);
		debugC(3, kDebugLoading, "  RichTextCastMember(): foreColor: 0x%08x, bgColor: 0x%08x", _foreColor, _bgColor);
	} else {
		warning("STUB: RichTextCastMember: RTE not yet supported for version v%d (%d)", humanVersion(_cast->_version), _cast->_version);
	}

	_type = kCastRichText;
	_picture = nullptr;
	_pictureWithBg = nullptr;
}

RichTextCastMember::RichTextCastMember(Cast *cast, uint16 castId, RichTextCastMember &source)
		: CastMember(cast, castId) {

	_type = kCastRichText;
	_initialRect = source._initialRect;
	_boundingRect = source._boundingRect;
	_bgColor = source._bgColor;
	_styles = source._styles;
	_ftext = source._ftext;
	if (cast == source._cast)
		_children = source._children;

}

RichTextCastMember::~RichTextCastMember() {
	if (_picture)
		delete _picture;

	if (_pictureWithBg)
		delete _pictureWithBg;
}

void RichTextCastMember::load() {
	if (_loaded)
		return;

	// RichText casts consist of 3 files:
	// RTE0: Editor data, used only by the Authoring Tool
	// RTE1: Plain text data
	// RTE2: Bitmap representation for rendering
	//
	// RTE0 is using Paige editor by Hermes, which was recently
	// open sourced. So, if anyone wants to look into internals,
	// https://github.com/nmatavka/Hermes-Paige/tree/main
	// the pgReadDoc() is the code entry:
	// https://github.com/nmatavka/Hermes-Paige/blob/main/PGSOURCE/PGREAD.C#L767
	uint rte0id = 0;
	uint rte1id = 0;
	uint rte2id = 0;
	for (auto &it : _children) {
		if (it.tag == MKTAG('R', 'T', 'E', '0')) {
			rte0id = it.index;
		} else if (it.tag == MKTAG('R', 'T', 'E', '1')) {
			rte1id = it.index;
		} else if (it.tag == MKTAG('R', 'T', 'E', '2')) {
			rte2id = it.index;
		}
	}

	const RTE0 *rte0 = nullptr;
	if (_cast->_loadedRTE0s.contains(rte0id)) {
		rte0 = _cast->_loadedRTE0s.getVal(rte0id);
	} else {
		warning("RichTextCastMember::load(): rte0tid %i isn't loaded", rte0id);
	}
	Common::U32String macText;
	if (_cast->_loadedRTE1s.contains(rte1id)) {
		const RTE1 *rte1 =  _cast->_loadedRTE1s.getVal(rte1id);
		if (!rte1->data.empty()) {
			_plainText = Common::U32String((const char *)&rte1->data[0], rte1->data.size(), g_director->getPlatformEncoding());
			macText = Common::U32String((const char *)&rte1->data[0], rte1->data.size(), Common::kMacRoman);
		}
	} else {
		warning("RichTextCastMember::load(): rte1tid %i isn't loaded, no plain text!", rte1id);
	}
	// The Windows player sets the text again with its own fonts, so a face the
	// author had on the Mac (Safecracker's menu is RussellSquare) comes out in
	// whatever Windows substituted. The RTE2 bitmap only keeps the Mac look.
	//
	// The text bytes are Mac Roman whatever the file is stamped with: 0xAA is the
	// trademark sign in Safecracker's menu, which Windows-1252 turns into a feminine ordinal.
	_styles = TextXtra::StyleRuns();
	_ftext.clear();
	if (g_director->getPlatform() == Common::kPlatformWindows && rte0 && !rte0->data.empty() && !macText.empty()) {
		if (TextXtra::readXMEDStyles(rte0->data, _styles) && !_styles.runs.empty())
			buildFormattedText(macText);
		else
			debugC(3, kDebugText, "RichTextCastMember::load(): RTE0 %d has no readable styles, keeping the bitmap", rte0id);
	}

	if (_cast->_loadedRTE2s.contains(rte2id)) {
		_picture = new Picture();
		const RTE2 *rte2 = _cast->_loadedRTE2s.getVal(rte2id);
		Graphics::ManagedSurface *surface = rte2->createSurface(_foreColor, _bgColor, _pf32, false);
		if (surface) {
			_picture->_surface.copyFrom(surface->rawSurface());
			surface->free();
			delete surface;
		}

		_pictureWithBg = new Picture();
		surface = rte2->createSurface(_foreColor, _bgColor, _pf32, true);
		if (surface) {
			_pictureWithBg->_surface.copyFrom(surface->rawSurface());
			surface->free();
			delete surface;
		}
	} else {
		warning("RichTextCastMember::load(): rte2tid %i isn't loaded, no bitmap text!", rte2id);
	}

	_loaded = true;
}


// Same formatting codes as TextXtraCastMember::buildFormattedText().
void RichTextCastMember::buildFormattedText(const Common::U32String &text) {
	_ftext.clear();

	for (uint i = 0; i < _styles.runs.size(); i++) {
		uint32 from = _styles.runs[i].offset;
		if (from >= text.size())
			break;

		uint32 to = (i + 1 < _styles.runs.size()) ? _styles.runs[i + 1].offset : text.size();
		to = MIN<uint32>(to, text.size());
		if (to <= from)
			continue;

		const TextXtra::TextStyle &s = _styles.runs[i].style;
		int fontId = s.font.empty() ? (int)Graphics::kMacFontSystem : g_director->_wm->_fontMan->registerFontName(s.font);

		// The parser keeps the high byte of each 16 bit colour component.
		_ftext += Common::String::format("\001\016%04x%02x%04x%04x%04x%04x",
				fontId, s.slant, s.size, s.r << 8, s.g << 8, s.b << 8);

		for (uint32 c = from; c < to; c++) {
			if (text[c] == '\001')
				_ftext += '\001';
			_ftext += text[c];
		}
	}
}

Graphics::MacWidget *RichTextCastMember::createWidget(Common::Rect &bbox, Channel *channel, SpriteType spriteType) {
	if (!_ftext.empty() && bbox.width() && bbox.height()) {
		const TextXtra::TextStyle &first = _styles.runs[0].style;
		Graphics::MacFont macFont(g_director->_wm->_fontMan->registerFontName(first.font), first.size, first.slant);

		Graphics::TextAlign align = Graphics::kTextAlignLeft;
		if (_styles.align == 1)
			align = Graphics::kTextAlignCenter;
		else if (_styles.align == 2)
			align = Graphics::kTextAlignRight;

		byte r, g, b;
		_pf32.colorToRGB(_bgColor, r, g, b);
		uint32 fg = g_director->_wm->findBestColor(first.r, first.g, first.b);
		uint32 bg = g_director->_wm->findBestColor(r, g, b);

		Graphics::MacText *text = new Graphics::MacText(g_director->getCurrentWindow()->getMacWindow(),
				bbox.left, bbox.top, bbox.width(), bbox.height(), g_director->_wm,
				_ftext, &macFont, fg, bg, _initialRect.width(), align);
		text->draw();
		return text;
	}

	if (!_pictureWithBg) {
		warning("RichTextCastMember::createWidget: No picture");
		return nullptr;
	}

	// skip creating widget when the bbox is not available, maybe we should create it using initialRect
	if (!bbox.width() || !bbox.height())
		return nullptr;

	// Check if we need to dither the image
	int dstBpp = g_director->_wm->_pixelformat.bytesPerPixel;

	Graphics::MacWidget *widget = new Graphics::MacWidget(g_director->getCurrentWindow()->getMacWindow(), bbox.left, bbox.top, bbox.width(), bbox.height(), g_director->_wm, false);

	Graphics::Surface *dithered = nullptr;
	// Always render the background: background-transparent ink masks out a real
	// colour rather than reading an alpha channel, and Channel::getPlotData() points
	// it at exactly the colour this picture paints its background in. _picture keeps
	// its transparency in alpha only, and every one of its pixels holds the
	// *foreground* colour, differing only in alpha -- so the ink can never key it.
	Picture *src = _pictureWithBg;

	if (dstBpp == 1) {
		dithered = src->_surface.convertTo(g_director->_wm->_pixelformat, nullptr, 0, g_director->_wm->getPalette(), g_director->_wm->getPaletteSize());
	}

	// scale for drawing a different size sprite
	copyStretchImg(
		dithered ? dithered : &src->_surface,
		widget->getSurface()->surfacePtr(),
		_initialRect,
		bbox,
		g_director->_wm->getPalette()
	);

	if (dithered) {
		dithered->free();
		delete dithered;
	}

	return widget;
}

uint32 RichTextCastMember::getBackgroundColor() {
	byte r, g, b;
	_pf32.colorToRGB(_bgColor, r, g, b);

	if (g_director->_wm->_pixelformat.isCLUT8())
		return g_director->_wm->findBestColor(r, g, b);

	return g_director->_wm->_pixelformat.RGBToColor(r, g, b);
}

bool RichTextCastMember::hasField(int field) {
	switch (field) {
	case kTheText:
	case kThePageHeight:
	case kTheScrollTop:
		return true;
	default:
		break;
	}
	return CastMember::hasField(field);
}

Datum RichTextCastMember::getField(int field) {
	Datum d;

	switch (field) {
	case kTheText:
		d = Datum(Common::String(_plainText));
		break;
	case kThePageHeight:
	case kTheScrollTop:
	default:
		d = CastMember::getField(field);
		break;
	}

	return d;
}

void RichTextCastMember::setField(int field, const Datum &d) {
	switch (field) {
	case kTheText:
		_plainText = Common::U32String(d.asString());
		warning("STUB: RichTextCastMember::setField: text set to \"%s\", but won't rerender!", d.asString().c_str());
		return;
	case kThePageHeight:
	case kTheScrollTop:
	default:
		break;
	}

	CastMember::setField(field, d);
}

Common::String RichTextCastMember::formatInfo() {
	// need to pull the data from the STXT resource before the
	// debug output will be visible
	load();
	Common::String format = formatStringForDump(_plainText.encode());

	return Common::String::format(
		"initialRect: %dx%d@%d,%d, boundingRect: %dx%d@%d,%d, text: \"%s\"",
		_initialRect.width(), _initialRect.height(),
		_initialRect.left, _initialRect.top,
		_boundingRect.width(), _boundingRect.height(),
		_boundingRect.left, _boundingRect.top,
		format.c_str()
	);
}

uint32 RichTextCastMember::getCastDataSize() {
	if (_cast->_version >= kFileVer500 && _cast->_version < kFileVer1100) {
		// 8 bytes (_initialRect) + 8 bytes (_boundingRect)
		// 1 antialiasFlag, 1 cropFlags, 2 scrollPos, 2 antialiasFontSize,
		// 2 displayHeight, 1 skip byte
		// 3 bytes r, g, b (foreground, each a byte)
		// 6 bytes r, g, b (background, each 2 bytes)
		return 34;
	} else {
		warning("RichTextCastMember()::getCastDataSize(): version v%d isn't handled", humanVersion(_cast->_version));
		return 0;
	}
}

void RichTextCastMember::writeCastData(Common::SeekableWriteStream *writeStream) {
	if (_cast->_version >= kFileVer500 && _cast->_version < kFileVer1100) {
		Movie::writeRect(writeStream, _initialRect);
		Movie::writeRect(writeStream, _boundingRect);

		writeStream->writeByte(_antialiasFlag);
		writeStream->writeByte(_cropFlags);
		writeStream->writeUint16BE(_scrollPos);
		writeStream->writeUint16BE(_antialiasFontSize);
		writeStream->writeUint16BE(_displayHeight);
		writeStream->writeByte(0); // skip byte

		uint8 r, g, b;
		_pf32.colorToRGB(_foreColor, r, g, b);
		writeStream->writeByte(r);
		writeStream->writeByte(g);
		writeStream->writeByte(b);

		_pf32.colorToRGB(_bgColor, r, g, b);
		writeStream->writeUint16BE(r << 8);
		writeStream->writeUint16BE(g << 8);
		writeStream->writeUint16BE(b << 8);
	} else {
		warning("RichTextCastMember()::writeCastData(): version v%d isn't handled", humanVersion(_cast->_version));
	}
}

bool RichTextCastMember::canWriteCastData() {
	return _cast->_version >= kFileVer500 && _cast->_version < kFileVer1100;
}

}	// End of namespace Director
