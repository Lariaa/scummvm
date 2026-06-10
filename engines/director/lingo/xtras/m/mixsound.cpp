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
#include "director/lingo/xtras/m/mixsound.h"

/**************************************************
 *
 * USED IN:
 * Loewenzahn 2 / 3
 * TKKG 3 / 4 / 5 / 6 / 7
 * Oscar the Balloonist (Lake / Countryside)
 *
 **************************************************/

/*
 * Macromedia audio-mixing media Xtra (MIXSOUND.X32 / MIXSOUND.X16). It builds
 * its msgTable at runtime, so the method table could not be extracted
 * statically with devtools/director-generate-xobj-stub.py. The Loewenzahn
 * titles auto-load it from their XTRAS folder but call none of its handlers, so
 * this is an empty stub to register an opener and make the Xtra exist.
 */

namespace Director {

const char *MixSoundXtra::xlibName = "MixSound";
const XlibFileDesc MixSoundXtra::fileNames[] = {
	{ "MIXSOUND",	nullptr },
	{ nullptr,		nullptr },
};

static MethodProto xlibMethods[] = {
	{ "new",	MixSoundXtra::m_new,	0, 0, 500 },
	{ nullptr, nullptr, 0, 0, 0 }
};

MixSoundXtraObject::MixSoundXtraObject(ObjectType ObjectType) :Object<MixSoundXtraObject>("MixSound") {
	_objType = ObjectType;
}

bool MixSoundXtraObject::hasProp(const Common::String &propName) {
	return (propName == "name");
}

Datum MixSoundXtraObject::getProp(const Common::String &propName) {
	if (propName == "name")
		return Datum(MixSoundXtra::xlibName);
	warning("MixSoundXtra::getProp: unknown property '%s'", propName.c_str());
	return Datum();
}

void MixSoundXtra::open(ObjectType type, const Common::Path &path) {
	MixSoundXtraObject::initMethods(xlibMethods);
	MixSoundXtraObject *xobj = new MixSoundXtraObject(type);
	if (type == kXtraObj) {
		g_lingo->_openXtras.push_back(xlibName);
		g_lingo->_openXtraObjects.push_back(xobj);
	}
	g_lingo->exposeXObject(xlibName, xobj);
}

void MixSoundXtra::close(ObjectType type) {
	MixSoundXtraObject::cleanupMethods();
	g_lingo->_globalvars[xlibName] = Datum();
}

void MixSoundXtra::m_new(int nargs) {
	g_lingo->printSTUBWithArglist("MixSoundXtra::m_new", nargs);
	g_lingo->dropStack(nargs);
	g_lingo->push(g_lingo->_state->me);
}

} // End of namespace Director
