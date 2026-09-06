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
 */

#ifndef DIRECTOR_LINGO_LINGODEC_UTIL_H
#define DIRECTOR_LINGO_LINGODEC_UTIL_H

#include "common/stream.h"
#include "common/textconsole.h"

namespace LingoDec {

/**
 * Seek to an offset that came out of the script chunk.
 *
 * Every offset in a Lingo script is a value read from the file, and the chunks arrive as a
 * SeekableSubReadStream, whose seek() asserts rather than returning false. One bad offset
 * therefore aborted the whole engine over what is only a decompiler hiccup -- and the
 * offsets do go bad, either from a damaged file or from reading the records at the wrong
 * Lingo version. Reads past the end are harmless by comparison: they return zero.
 *
 * Refuse the seek instead, and let the caller skip that part of the script.
 */
inline bool safeSeek(Common::SeekableReadStream &stream, int64 offset, const char *what) {
	if (offset < 0 || offset > stream.size()) {
		warning("LingoDec: %s at offset %d lies outside the %d-byte chunk, skipping",
				what, (int)offset, (int)stream.size());
		return false;
	}
	stream.seek(offset);
	return true;
}

} // End of namespace LingoDec

#endif
