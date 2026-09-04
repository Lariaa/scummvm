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

#ifndef DIRECTOR_LINGO_XTRAS_CAST_DIRECTMEDIAXTRA_H
#define DIRECTOR_LINGO_XTRAS_CAST_DIRECTMEDIAXTRA_H

#include "common/array.h"
#include "common/str.h"

namespace Director {

class Cast;
class CastMember;
class XtraCastMember;

namespace DirectMediaXtra {

struct Info {
	Common::String filename;	// bare name of the media file
	Common::String authorPath;	// absolute path from the authoring machine
	uint32 durationMs = 0;
	uint16 width = 0;
	uint16 height = 0;
	uint16 volume = 0;
};

CastMember *createCastMember(Cast *cast, uint16 castId, XtraCastMember *xtra);
bool parseXtraData(const Common::Array<byte> &data, Info &info);

} // End of namespace DirectMediaXtra

} // End of namespace Director

#endif
