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

#include "graphics/macgui/macfontmanager.h"
#include "graphics/macgui/mactext.h"
#include "graphics/macgui/macwindow.h"
#include "graphics/macgui/macwindowmanager.h"

#include "director/director.h"
#include "director/cast.h"
#include "director/movie.h"
#include "director/window.h"
#include "director/castmember/xtra.h"
#include "director/lingo/lingo-the.h"

namespace Director {

XtraCastMember::XtraCastMember(Cast *cast, uint16 castId, Common::SeekableReadStreamEndian &stream, uint16 version)
		: CastMember(cast, castId, stream) {
	_type = kCastXtra;

	if (debugChannelSet(5, kDebugLoading)) {
		stream.hexdump(stream.size());
	}

	warning("STUB: XtraCastMember::XtraCastMember(): Xtra cast members not yet supported for version v%d (%d)", humanVersion(_cast->_version), _cast->_version);
}

XtraCastMember::XtraCastMember(Cast *cast, uint16 castId, XtraCastMember &source)
		: CastMember(cast, castId) {
	_volume = source._volume;
}

bool XtraCastMember::hasField(int field) {
	switch (field) {
	case kTheCuePointNames:		// D6
	case kTheCuePointTimes:		// D6
	case kTheCurrentTime:		// D6
	case kTheMediaBusy:			// D6, undocumented
		return true;
	default:
		break;
	}
	return CastMember::hasField(field);
}

Datum XtraCastMember::getField(int field) {
	Datum d;

	switch (field) {
	default:
		d = CastMember::getField(field);
		break;
	}

	return d;
}

void XtraCastMember::setField(int field, const Datum &d) {
	switch (field) {
	default:
		break;
	}

	CastMember::setField(field, d);
}

bool XtraCastMember::hasProp(const Common::String &propName) {
	// Xtra media members (e.g. Shockwave Audio ".swa") expose playback-progress
	// properties. We don't stream/play the underlying Xtra media yet, but scripts
	// commonly poll these in an `on exitFrame` wait-loop before advancing, so we
	// answer them to avoid an "unknown property" Lingo error.
	if (propName.equalsIgnoreCase("percentPlayed") || propName.equalsIgnoreCase("percentStreamed")
			|| propName.equalsIgnoreCase("volume") || propName.equalsIgnoreCase("text"))
		return true;
	return CastMember::hasProp(propName);
}

Datum XtraCastMember::getProp(const Common::String &propName) {
	// The Xtra media isn't actually played, so report it as "fully played" (100).
	// Intro/logo frames typically loop until `the percentPlayed of member X`
	// reaches 100; returning 100 lets them proceed instead of hanging forever
	// (returning 0) or erroring (property unknown).
	if (propName.equalsIgnoreCase("percentPlayed") || propName.equalsIgnoreCase("percentStreamed"))
		return Datum(100);
	if (propName.equalsIgnoreCase("volume"))
		return Datum(_volume);
	// D7 "Text Asset" Xtra members are used as text fields by scripts.
	if (propName.equalsIgnoreCase("text"))
		return Datum(_text);
	return CastMember::getProp(propName);
}

void XtraCastMember::setProp(const Common::String &propName, const Datum &value, bool force) {
	// Accept `set the volume of member X` on Shockwave Audio members. We don't
	// play the media, but the assignment must not raise a Lingo error; store it
	// so a subsequent read is consistent.
	if (propName.equalsIgnoreCase("volume")) {
		_volume = value.asInt();
		return;
	}
	// Store text written to D7 "Text Asset" Xtra members and flag the member as
	// changed so the channel rebuilds its widget and the new text is drawn.
	if (propName.equalsIgnoreCase("text")) {
		_text = value.asString();
		_modified = true;
		return;
	}
	CastMember::setProp(propName, value, force);
}

Graphics::MacWidget *XtraCastMember::createWidget(Common::Rect &bbox, Channel *channel, SpriteType spriteType) {
	// Only Xtra members that carry text (D7 "Text Asset", e.g. TKKG 7's notebook
	// fields) should draw anything; everything else (Shockwave Audio, etc.) has
	// no visual and returns nullptr so no stray box is rendered.
	if (_text.empty())
		return nullptr;

	// Minimal rendering: the Xtra's authored font/size/colour/box are not parsed
	// yet, so use the system font and Director's default text colours (black on
	// white, matching TextCastMember's defaults) sized to the sprite's bbox.
	Graphics::MacFont *macFont = new Graphics::MacFont();
	Common::Rect dims(bbox);

	Graphics::MacText *widget = new Graphics::MacText(
		g_director->getCurrentWindow()->getMacWindow(),
		bbox.left, bbox.top, dims.width(), dims.height(),
		g_director->_wm, Common::U32String(_text), macFont,
		/* fgcolor */ 0xff, /* bgcolor */ 0, dims.width(),
		Graphics::kTextAlignLeft);
	widget->draw();

	delete macFont;
	return widget;
}

Common::String XtraCastMember::formatInfo() {
	return Common::String::format("Xtra");
}

uint32 XtraCastMember::getCastDataSize() {
	warning("XtraCastMember()::getCastDataSize(): CastMember version invalid or not handled");
	return 0;
}

void XtraCastMember::writeCastData(Common::SeekableWriteStream *writeStream) {
	warning("XtraCastMember()::writeCastData(): CastMember version invalid or not handled");
}

} // End of namespace Director
