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
#include "director/lingo/xtras/i/ineturl.h"

/**************************************************
 *
 * USED IN:
 * Loewenzahn 1 / 2 / 3 / 4 / 8 / Adventskalender
 * TKKG 4 / 7 / 8 / 9 / 10 / 11 / 13 / 14
 * Oscar the Balloonist Discovers the Sea
 *
 **************************************************/

/*
 * Macromedia Internet URL Xtra (INETURL.X32 / INETURL.X16), part of the
 * NetLingo family. It builds its msgTable at runtime, so the method table
 * could not be extracted statically with devtools/director-generate-xobj-stub.py.
 * The Loewenzahn titles auto-load it but call none of its handlers, so this is
 * an empty stub to register an opener (silencing the "Unimplemented xlib"
 * warning) and make the Xtra exist. If a title is found that calls into it,
 * load the Xtra in real Director, run `put mMessageList(xtra("INetURL"))`, save
 * the output to a text file and re-run the generator to fill in the handlers.
 */

namespace Director {

const char *INetURLXtra::xlibName = "INetURL";
const XlibFileDesc INetURLXtra::fileNames[] = {
	{ "INETURL",	nullptr },
	{ nullptr,		nullptr },
};

static MethodProto xlibMethods[] = {
	{ "new",	INetURLXtra::m_new,	0, 0, 500 },
	{ nullptr, nullptr, 0, 0, 0 }
};

INetURLXtraObject::INetURLXtraObject(ObjectType ObjectType) :Object<INetURLXtraObject>("INetURL") {
	_objType = ObjectType;
}

bool INetURLXtraObject::hasProp(const Common::String &propName) {
	return (propName == "name");
}

Datum INetURLXtraObject::getProp(const Common::String &propName) {
	if (propName == "name")
		return Datum(INetURLXtra::xlibName);
	warning("INetURLXtra::getProp: unknown property '%s'", propName.c_str());
	return Datum();
}

void INetURLXtra::open(ObjectType type, const Common::Path &path) {
	INetURLXtraObject::initMethods(xlibMethods);
	INetURLXtraObject *xobj = new INetURLXtraObject(type);
	if (type == kXtraObj) {
		g_lingo->_openXtras.push_back(xlibName);
		g_lingo->_openXtraObjects.push_back(xobj);
	}
	g_lingo->exposeXObject(xlibName, xobj);
}

void INetURLXtra::close(ObjectType type) {
	INetURLXtraObject::cleanupMethods();
	g_lingo->_globalvars[xlibName] = Datum();
}

void INetURLXtra::m_new(int nargs) {
	g_lingo->printSTUBWithArglist("INetURLXtra::m_new", nargs);
	g_lingo->dropStack(nargs);
	g_lingo->push(g_lingo->_state->me);
}

} // End of namespace Director
