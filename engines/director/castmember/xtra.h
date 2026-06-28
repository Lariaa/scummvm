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

#ifndef DIRECTOR_CASTMEMBER_XTRA_H
#define DIRECTOR_CASTMEMBER_XTRA_H

#include "director/castmember/castmember.h"

namespace Director {

class XtraCastMember : public CastMember {
public:
	XtraCastMember(Cast *cast, uint16 castId, Common::SeekableReadStreamEndian &stream, uint16 version);
	XtraCastMember(Cast *cast, uint16 castId, XtraCastMember &source);

	CastMember *duplicate(Cast *cast, uint16 castId) override { return (CastMember *)(new XtraCastMember(cast, castId, *this)); }

	bool hasField(int field) override;
	Datum getField(int field) override;
	void setField(int field, const Datum &value) override;

	bool hasProp(const Common::String &propName) override;
	Datum getProp(const Common::String &propName) override;
	void setProp(const Common::String &propName, const Datum &value, bool force = false) override;

	// Render D7 "Text Asset" Xtra members (those carrying text) as a plain text
	// widget so their content is shown on stage. Other Xtra members draw nothing.
	Graphics::MacWidget *createWidget(Common::Rect &bbox, Channel *channel, SpriteType spriteType) override;

	Common::String formatInfo() override;

	uint32 getCastDataSize() override;
	void writeCastData(Common::SeekableWriteStream *writeStream) override;

private:
	Common::String _xtraSymbol;
	Common::Array<byte> _xtraData;
	// Shockwave Audio (".swa") Xtra members are sounds; scripts set/read their
	// volume even though we don't play the underlying media. Stored so get/set
	// stay consistent. 0-255, matching Lingo's sound-volume range.
	int _volume = 255;

	// D7 "Text Asset" Xtra members behave like text fields: scripts read/write
	// `the text of member X`. We don't render the Xtra, but store the string so
	// get/set stay consistent and don't raise an "unknown property" Lingo error
	// (e.g. TKKG 7 clears its notebook text via `member("KB_NotebookText").text`).
	Common::String _text;
};

} // End of namespace Director

#endif
