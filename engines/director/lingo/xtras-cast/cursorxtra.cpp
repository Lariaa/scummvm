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

#include "director/director.h"
#include "director/cast.h"
#include "director/castmember/xtra.h"
#include "director/lingo/lingo-the.h"
#include "director/lingo/xtras-cast/cursorxtra.h"

namespace Director {

namespace CursorXtra {

// 56-byte (static) / 168-byte (animated) "cursor" payload observed in
// TKKG 7 (D7), all BE32; the member/mask pair mirrors the Lingo
// `cursor [member, mask]` command:
//   0: payload size        4: unk (1)
//   8: hotspot x          12: hotspot y
//  16: cursor size in px  20: frame interval in ms
//  24: unk (1)            28: cursor bitmap cast member
//  32: mask bitmap cast member
//  36: frame count (1, or 8 with per-frame extras following; unparsed)
//  40: 16-byte per-instance GUID
bool parseXtraData(const Common::Array<byte> &data, Info &info) {
	if (data.size() < 40)
		return false;

	uint32 payloadSize = READ_BE_UINT32(&data[0]);
	if (payloadSize != data.size())
		return false;

	info.hotspot.x = READ_BE_INT32(&data[8]);
	info.hotspot.y = READ_BE_INT32(&data[12]);
	info.sizePx = READ_BE_UINT32(&data[16]);
	info.intervalMs = READ_BE_UINT32(&data[20]);
	info.member = READ_BE_UINT32(&data[28]);
	info.mask = READ_BE_UINT32(&data[32]);
	info.frameCount = READ_BE_UINT32(&data[36]);

	if (!info.member || info.member > 0xFFFF || info.mask > 0xFFFF)
		return false;

	return true;
}

CastMember *createCastMember(Cast *cast, uint16 castId, XtraCastMember *xtra) {
	return new CursorXtraCastMember(cast, castId, *xtra);
}

} // End of namespace CursorXtra

CursorXtraCastMember::CursorXtraCastMember(Cast *cast, uint16 castId, XtraCastMember &source)
		: CastMember(cast, castId) {
	_type = kCastXtra;

	_valid = CursorXtra::parseXtraData(source.getXtraData(), _info);
	if (!_valid)
		warning("CursorXtraCastMember: failed to parse %d-byte cursor payload for cast member %d",
				source.getXtraData().size(), castId);
	else {
		debugC(3, kDebugLoading, "CursorXtraCastMember: hotspot: %d,%d, size: %dpx, interval: %dms, member: %d, mask: %d, frames: %d",
				_info.hotspot.x, _info.hotspot.y, _info.sizePx, _info.intervalMs, _info.member, _info.mask, _info.frameCount);
		if (_info.frameCount > 1)
			warning("STUB: CursorXtraCastMember: animated cursor with %d frames not yet supported for cast member %d, using the first frame",
					_info.frameCount, castId);
	}
}

CursorXtraCastMember::CursorXtraCastMember(Cast *cast, uint16 castId, CursorXtraCastMember &source)
		: CastMember(cast, castId) {
	_type = kCastXtra;
	_info = source._info;
	_valid = source._valid;
	if (cast == source._cast)
		_children = source._children;
}

bool CursorXtraCastMember::getCursorInfo(CastMemberID &image, CastMemberID &mask) {
	if (!_valid)
		return false;

	uint16 castLib = _cast->_castLibID;
	image = CastMemberID(_info.member, castLib);
	mask = _info.mask ? CastMemberID(_info.mask, castLib) : CastMemberID();
	return true;
}

// The cursor cast member properties, as documented in the Custom Cursor Xtra
// manual: autoMask, castMemberList, cursorSize, hotSpot and interval. Without
// them a plain `member("...").cursorSize = 16` is an uncaught Lingo error,
// which under lingostrict aborts the whole handler: TKKG 7's photofit program
// never got past the first line of its CursorObj constructor.
bool CursorXtraCastMember::hasField(int field) {
	switch (field) {
	case kTheAutoMask:
	case kTheCastMemberList:
	case kTheCursorSize:
	case kTheHotSpot:
	case kTheInterval:
		return true;
	default:
		break;
	}
	return CastMember::hasField(field);
}

Datum CursorXtraCastMember::getField(int field) {
	Datum d;

	switch (field) {
	case kTheAutoMask:
		d = (int)_info.autoMask;
		break;
	case kTheCastMemberList:
		// Only the first frame is parsed out of the payload, so the list has
		// at most one entry even for an animated cursor.
		d.type = ARRAY;
		d.u.farr = new FArray;
		if (_info.member)
			d.u.farr->arr.push_back(Datum(CastMemberID(_info.member, _cast->_castLibID)));
		break;
	case kTheCursorSize:
		d = (int)_info.sizePx;
		break;
	case kTheHotSpot:
		d = Datum(_info.hotspot);
		break;
	case kTheInterval:
		d = (int)_info.intervalMs;
		break;
	default:
		d = CastMember::getField(field);
		break;
	}

	return d;
}

void CursorXtraCastMember::setField(int field, const Datum &d) {
	switch (field) {
	case kTheAutoMask:
		_info.autoMask = d.asInt() != 0;
		return;
	case kTheCastMemberList:
		if (d.type == ARRAY && d.u.farr && !d.u.farr->arr.empty())
			_info.member = d.u.farr->arr[0].asMemberID().member;
		return;
	case kTheCursorSize:
		// "Only two values are permitted: 16 ... and 32".
		_info.sizePx = (d.asInt() >= 32) ? 32 : 16;
		return;
	case kTheHotSpot:
		_info.hotspot = d.asPoint();
		return;
	case kTheInterval:
		// -1 is documented as "do not animate, show the first frame". Stored
		// unsigned like the parsed payload; the cast back in getField()
		// returns it unchanged.
		_info.intervalMs = d.asInt();
		return;
	default:
		break;
	}

	CastMember::setField(field, d);
}

Common::String CursorXtraCastMember::formatInfo() {
	return Common::String::format(
		"cursor Xtra: hotspot: %d,%d, size: %dpx, interval: %dms, member: %d, mask: %d, frames: %d",
		_info.hotspot.x, _info.hotspot.y, _info.sizePx, _info.intervalMs,
		_info.member, _info.mask, _info.frameCount);
}

} // End of namespace Director
