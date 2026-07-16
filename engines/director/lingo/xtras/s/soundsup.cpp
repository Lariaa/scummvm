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

#include "common/system.h"

#include "director/director.h"
#include "director/lingo/lingo.h"
#include "director/lingo/lingo-object.h"
#include "director/lingo/lingo-utils.h"
#include "director/lingo/xtras/s/soundsup.h"

/**************************************************
 *
 * USED IN:
 * TKKG 3 / 4 / 5 / 6 / 7
 *
 **************************************************/

/*
 * Sound support Xtra (SOUNDSUP.X32). This is a transcoder: it carries no
 * message table at all, only the MOA cast-format constants kMoaCfFormat_WAVE,
 * kMoaCfFormat_AIFF and kMoaCfFormat_snd, so Director drives it through the
 * import/export interface rather than from Lingo. There is no scriptable API
 * to implement, and ScummVM decodes those formats itself.
 *
 * This is therefore an empty stub, present only to register an opener so the
 * Xtra exists and the "Unimplemented xlib" warning goes away.
 */

namespace Director {

const char *SoundSupXtra::xlibName = "SoundSup";
const XlibFileDesc SoundSupXtra::fileNames[] = {
	{ "SOUNDSUP",	nullptr },
	{ nullptr,		nullptr },
};

static MethodProto xlibMethods[] = {
	{ "new",	SoundSupXtra::m_new,	0, 0, 500 },
	{ nullptr, nullptr, 0, 0, 0 }
};

SoundSupXtraObject::SoundSupXtraObject(ObjectType ObjectType) :Object<SoundSupXtraObject>("SoundSup") {
	_objType = ObjectType;
}

bool SoundSupXtraObject::hasProp(const Common::String &propName) {
	return (propName == "name");
}

Datum SoundSupXtraObject::getProp(const Common::String &propName) {
	if (propName == "name")
		return Datum(SoundSupXtra::xlibName);
	warning("SoundSupXtra::getProp: unknown property '%s'", propName.c_str());
	return Datum();
}

void SoundSupXtra::open(ObjectType type, const Common::Path &path) {
	SoundSupXtraObject::initMethods(xlibMethods);
	SoundSupXtraObject *xobj = new SoundSupXtraObject(type);
	if (type == kXtraObj) {
		g_lingo->_openXtras.push_back(xlibName);
		g_lingo->_openXtraObjects.push_back(xobj);
	}
	g_lingo->exposeXObject(xlibName, xobj);
}

void SoundSupXtra::close(ObjectType type) {
	SoundSupXtraObject::cleanupMethods();
	g_lingo->_globalvars[xlibName] = Datum();
}

void SoundSupXtra::m_new(int nargs) {
	g_lingo->printSTUBWithArglist("SoundSupXtra::m_new", nargs);
	g_lingo->dropStack(nargs);
	g_lingo->push(g_lingo->_state->me);
}

} // End of namespace Director
