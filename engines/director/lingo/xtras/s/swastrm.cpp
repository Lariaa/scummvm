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
#include "director/lingo/xtras/s/swastrm.h"

/**************************************************
 *
 * USED IN:
 * Loewenzahn 1 / 2 / 3 / 4 / 8 / Adventskalender
 * TKKG 6 / 7 / 8 / 9 / 10 / 11 / 13 / 14
 * Oscar the Balloonist (Lake / Countryside / Sea / Mountains)
 *
 **************************************************/

/*
 * Shockwave Audio streaming media Xtra (SWASTRM.X32 / SWASTRM.X16). It provides
 * streamed ".swa" playback consumed internally rather than via Lingo handlers,
 * and builds its msgTable at runtime, so the method table could not be
 * extracted statically with devtools/director-generate-xobj-stub.py. The
 * Loewenzahn titles auto-load it but call none of its handlers, so this is an
 * empty stub to register an opener and make the Xtra exist.
 */

namespace Director {

const char *SWAStrmXtra::xlibName = "SWAStrm";
const XlibFileDesc SWAStrmXtra::fileNames[] = {
	{ "SWASTRM",	nullptr },
	{ nullptr,		nullptr },
};

static MethodProto xlibMethods[] = {
	{ "new",	SWAStrmXtra::m_new,	0, 0, 500 },
	{ nullptr, nullptr, 0, 0, 0 }
};

SWAStrmXtraObject::SWAStrmXtraObject(ObjectType ObjectType) :Object<SWAStrmXtraObject>("SWAStrm") {
	_objType = ObjectType;
}

bool SWAStrmXtraObject::hasProp(const Common::String &propName) {
	return (propName == "name");
}

Datum SWAStrmXtraObject::getProp(const Common::String &propName) {
	if (propName == "name")
		return Datum(SWAStrmXtra::xlibName);
	warning("SWAStrmXtra::getProp: unknown property '%s'", propName.c_str());
	return Datum();
}

void SWAStrmXtra::open(ObjectType type, const Common::Path &path) {
	SWAStrmXtraObject::initMethods(xlibMethods);
	SWAStrmXtraObject *xobj = new SWAStrmXtraObject(type);
	if (type == kXtraObj) {
		g_lingo->_openXtras.push_back(xlibName);
		g_lingo->_openXtraObjects.push_back(xobj);
	}
	g_lingo->exposeXObject(xlibName, xobj);
}

void SWAStrmXtra::close(ObjectType type) {
	SWAStrmXtraObject::cleanupMethods();
	g_lingo->_globalvars[xlibName] = Datum();
}

void SWAStrmXtra::m_new(int nargs) {
	g_lingo->printSTUBWithArglist("SWAStrmXtra::m_new", nargs);
	g_lingo->dropStack(nargs);
	g_lingo->push(g_lingo->_state->me);
}

} // End of namespace Director
