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
#include "director/lingo/xtras/m/mix.h"

/**************************************************
 *
 * USED IN:
 * Loewenzahn 2 / 3
 * TKKG 3 / 4 / 5 / 6 / 7
 * Oscar the Balloonist (Lake / Countryside)
 *
 **************************************************/

/*
 * Macromedia Mix Services Xtra. Auto-loaded from the title's XTRAS folder under
 * two platform builds: MIX32.X32 (32-bit) and MIX16.X16 (16-bit) - both names
 * map to this single stub. It builds its msgTable at runtime, so the method
 * table could not be extracted statically with
 * devtools/director-generate-xobj-stub.py. The Loewenzahn titles call none of
 * its handlers, so this is an empty stub to register an opener and make the
 * Xtra exist.
 */

namespace Director {

const char *MixXtra::xlibName = "Mix";
const XlibFileDesc MixXtra::fileNames[] = {
	{ "MIX32",	nullptr },
	{ "MIX16",	nullptr },
	{ nullptr,	nullptr },
};

static MethodProto xlibMethods[] = {
	{ "new",	MixXtra::m_new,	0, 0, 500 },
	{ nullptr, nullptr, 0, 0, 0 }
};

MixXtraObject::MixXtraObject(ObjectType ObjectType) :Object<MixXtraObject>("Mix") {
	_objType = ObjectType;
}

bool MixXtraObject::hasProp(const Common::String &propName) {
	return (propName == "name");
}

Datum MixXtraObject::getProp(const Common::String &propName) {
	if (propName == "name")
		return Datum(MixXtra::xlibName);
	warning("MixXtra::getProp: unknown property '%s'", propName.c_str());
	return Datum();
}

void MixXtra::open(ObjectType type, const Common::Path &path) {
	MixXtraObject::initMethods(xlibMethods);
	MixXtraObject *xobj = new MixXtraObject(type);
	if (type == kXtraObj) {
		g_lingo->_openXtras.push_back(xlibName);
		g_lingo->_openXtraObjects.push_back(xobj);
	}
	g_lingo->exposeXObject(xlibName, xobj);
}

void MixXtra::close(ObjectType type) {
	MixXtraObject::cleanupMethods();
	g_lingo->_globalvars[xlibName] = Datum();
}

void MixXtra::m_new(int nargs) {
	g_lingo->printSTUBWithArglist("MixXtra::m_new", nargs);
	g_lingo->dropStack(nargs);
	g_lingo->push(g_lingo->_state->me);
}

} // End of namespace Director
