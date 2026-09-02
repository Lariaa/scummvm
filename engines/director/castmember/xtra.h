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

	const Common::String &getXtraSymbol() const { return _xtraSymbol; }
	const Common::Array<byte> &getXtraData() const { return _xtraData; }

	static CastMember *promote(Cast *cast, uint16 castId, XtraCastMember *xtra);

	bool hasField(int field) override;
	Datum getField(int field) override;
	void setField(int field, const Datum &value) override;

	bool hasProp(const Common::String &propName) override;
	Datum getProp(const Common::String &propName) override;
	void setProp(const Common::String &propName, const Datum &value, bool force = false) override;

	Common::String formatInfo() override;

	uint32 getCastDataSize() override;
	void writeCastData(Common::SeekableWriteStream *writeStream) override;
	bool canWriteCastData() override;

private:
	Common::String _xtraSymbol;
	Common::Array<byte> _xtraData;
	// Shockwave Audio (".swa") Xtra members are sounds; scripts set/read their
	// volume even though we don't play the underlying media. Stored so get/set
	// stay consistent. 0-255, matching Lingo's sound-volume range.
	int _volume = 255;

public:
	// Shockwave Audio streaming state, as the `state` property reports it:
	// 0 stopped, 1 reloading, 2 preloaded, 3 playing, 4 paused, 5 finished,
	// 9 error, 10 out of CPU. preLoadBuffer() is documented to leave it at 2.
	int _swaState = 0;
};

} // End of namespace Director

#endif
