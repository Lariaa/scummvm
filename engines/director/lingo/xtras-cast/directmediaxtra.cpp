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

#include "common/util.h"

#include "director/director.h"
#include "director/cast.h"
#include "director/castmember/digitalvideo.h"
#include "director/castmember/xtra.h"
#include "director/lingo/xtras-cast/directmediaxtra.h"

namespace Director {
namespace DirectMediaXtra {

// Tabuleiro's DirectMedia Xtra plays MPEG (and other DirectShow media) from a
// cast member of its own. One Xtra, three names, and they sit next to each
// other in DIRECTME.X32's registration block:
//
//   displayCategoryString -> "Tabuleiro Xtras"
//   displayNameString     -> "DirectMedia Xtra"
//   symbolString          -> "TBDIRECTMEDIA"
//
// The symbolString is what ends up in the CASt record, so that is the name to
// match here; "DirectMedia" is the xlib side, handled in xtras/d/directmedia.cpp.
//
// The Xtra's own msgTable documents what a member answers:
//   the file / title / loop / playsegment / segmentstart / segmentend /
//   hastoolbar / hidecursor / duration / videoheight / videowidth /
//   cuepointtimes / cuepointnames of member
//
// The payload is the Xtra's private blob and is LITTLE-endian, even though the
// CASt record around it is big-endian. Offsets below are verified against 68
// members in Loewenzahn 3 and 5 -- every one of them 3692 bytes long -- and the
// dimensions cross-checked against the MPEG sequence headers of the files they
// name (LZ-intro.mpg 352x288, lz5_intro.mpg 288x320, both matching exactly).
enum {
	kOffDuration  = 8,		// uint32, milliseconds
	kOffVolume    = 28,		// uint32, 0-100
	kOffWidth     = 64,		// uint32
	kOffHeight    = 68,		// uint32
	kOffAuthorPath = 112,	// char[255], absolute path on the authoring machine
	kOffFilename  = 367,	// char[255], bare file name -- kOffAuthorPath + 255
	kFieldLen     = 255,
	kMinPayload   = kOffFilename + 1
};

static Common::String readFixedString(const Common::Array<byte> &data, uint offset) {
	Common::String res;

	for (uint i = offset; i < MIN<uint>(offset + kFieldLen, data.size()) && data[i]; i++)
		res += (char)data[i];

	return res;
}

bool parseXtraData(const Common::Array<byte> &data, Info &info) {
	if (data.size() < kMinPayload)
		return false;

	info.durationMs = READ_LE_UINT32(&data[kOffDuration]);
	info.volume = (uint16)READ_LE_UINT32(&data[kOffVolume]);
	info.width = (uint16)READ_LE_UINT32(&data[kOffWidth]);
	info.height = (uint16)READ_LE_UINT32(&data[kOffHeight]);
	info.authorPath = readFixedString(data, kOffAuthorPath);
	info.filename = readFixedString(data, kOffFilename);

	return !info.filename.empty();
}

CastMember *createCastMember(Cast *cast, uint16 castId, XtraCastMember *xtra) {
	Info info;

	if (!parseXtraData(xtra->getXtraData(), info)) {
		warning("DirectMediaXtra::createCastMember(): cast %d has no usable payload (%d bytes), leaving it as a plain Xtra member",
				castId, xtra->getXtraData().size());
		return nullptr;
	}

	// A DirectMedia member is a digital video in all but the wrapper, so hand
	// it to the class that already knows how to load, place and play one --
	// the same route "quickTimeMedia" takes.
	DigitalVideoCastMember *dv = new DigitalVideoCastMember(cast, castId);
	dv->_qtmovie = false;
	dv->_avimovie = false;
	dv->_externalFilename = info.filename;
	dv->_externalDurationMs = info.durationMs;

	// ...but it still has to say so when asked. `the type of member` on an Xtra-owned
	// member is the Xtra's symbolString, and Peter entdeckt die Steinzeit tests exactly
	// that before it touches the MPEG:
	//     if sprite(20).member.type = #TBDIRECTMEDIA then
	//         Lautstaerke = getVolume(sprite(20))
	// in S_MpegLauterWin / S_MpegLeiserWin. Without the symbol the member answers
	// #digitalVideo, the gate never opens and the volume buttons do nothing.
	//
	// Take it from the CASt record rather than hardcoding it, so the spelling is
	// whatever the movie actually stored. Deliberately not done for the other entries
	// in xtraCastMemberProtos: Loewenzahn 1 and 2 gate video handling on
	// `.type = #digitalVideo`, and only DirectMedia has a corpus case proving the
	// symbolString is what the scripts expect.
	dv->_xtraSymbol = xtra->getXtraSymbol();

	if (info.width && info.height)
		dv->_initialRect = Common::Rect(info.width, info.height);

	debugC(2, kDebugLoading, "DirectMediaXtra::createCastMember(): cast %d: '%s', %dx%d, %d ms (authored as '%s')",
			castId, info.filename.c_str(), info.width, info.height, info.durationMs, info.authorPath.c_str());

	return dv;
}

} // End of namespace DirectMediaXtra
} // End of namespace Director
