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

#ifndef DIRECTOR_LINGO_XTRAS_S_SOUNDCHANNEL_H
#define DIRECTOR_LINGO_XTRAS_S_SOUNDCHANNEL_H

namespace Director {

class SoundChannelXtraObject : public Object<SoundChannelXtraObject> {
public:
	SoundChannelXtraObject(ObjectType objType);

	bool hasProp(const Common::String &propName) override;
	Datum getProp(const Common::String &propName) override;
	void setProp(const Common::String &propName, const Datum &value, bool force = false) override;

	// Which sound channel this object stands for. 0 for the instance the Xtra
	// registers itself under, which is not bound to any channel; sound(n) hands
	// out objects carrying n.
	int _channel = 0;
};

namespace SoundChannelXtra {

extern const char *xlibName;
extern const XlibFileDesc fileNames[];

void open(ObjectType type, const Common::Path &path);
void close(ObjectType type);

void m_new(int nargs);
void m_sound(int nargs);

} // End of namespace SoundChannelXtra

} // End of namespace Director

#endif
