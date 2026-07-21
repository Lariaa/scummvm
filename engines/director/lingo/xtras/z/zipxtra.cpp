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
#include "director/lingo/xtras/z/zipxtra.h"

/**************************************************
 *
 * USED IN:
 * TKKG 8, TKKG 9 (installed, no call observed yet)
 *
 **************************************************/

/*
 * ZipXtra writes zip/cab/jar archives, which ScummVM has no business doing, so
 * all three methods are stubs.
 *
 * The msgTable had to be assembled by hand: the Xtra keeps its header ("xtra
 * ZipXtra -- version %s.%s.%s.r%s", a printf template filled in at runtime) far
 * away from the method lines in .data, so the generator's carve gives up on the
 * binary. The lines below are the ones it stores at 0x7b10.
 */

/*
-- xtra ZipXtra -- version %s.%s.%s.r%s
new object me -- create a new child instance
ZipFiles object me, string zipDirectory, string zipFileName, any fileList -- create zip file.
CabFiles object me, string zipDirectory, string zipFileName, any fileList -- create zip file.
JarFiles object me, string zipDirectory, string zipFileName, any fileList -- create zip file.
 */

namespace Director {

const char *ZipXtraXtra::xlibName = "ZipXtra";
const XlibFileDesc ZipXtraXtra::fileNames[] = {
	{ "zipxtra",   nullptr },
	{ nullptr,        nullptr },
};

static MethodProto xlibMethods[] = {
	{ "new",				ZipXtraXtra::m_new,		 0, 0,	500 },
	{ "ZipFiles",				ZipXtraXtra::m_ZipFiles,		 3, 3,	500 },
	{ "CabFiles",				ZipXtraXtra::m_CabFiles,		 3, 3,	500 },
	{ "JarFiles",				ZipXtraXtra::m_JarFiles,		 3, 3,	500 },

	{ nullptr, nullptr, 0, 0, 0 }
};

static BuiltinProto xlibBuiltins[] = {

	{ nullptr, nullptr, 0, 0, 0, VOIDSYM }
};

ZipXtraXtraObject::ZipXtraXtraObject(ObjectType ObjectType) :Object<ZipXtraXtraObject>("ZipXtra") {
	_objType = ObjectType;
}

bool ZipXtraXtraObject::hasProp(const Common::String &propName) {
	return (propName == "name");
}

Datum ZipXtraXtraObject::getProp(const Common::String &propName) {
	if (propName == "name")
		return Datum(ZipXtraXtra::xlibName);
	warning("ZipXtraXtra::getProp: unknown property '%s'", propName.c_str());
	return Datum();
}

void ZipXtraXtra::open(ObjectType type, const Common::Path &path) {
	ZipXtraXtraObject::initMethods(xlibMethods);
	ZipXtraXtraObject *xobj = new ZipXtraXtraObject(type);
	if (type == kXtraObj) {
		g_lingo->_openXtras.push_back(xlibName);
		g_lingo->_openXtraObjects.push_back(xobj);
	}
	g_lingo->exposeXObject(xlibName, xobj);
	g_lingo->initBuiltIns(xlibBuiltins);
}

void ZipXtraXtra::close(ObjectType type) {
	ZipXtraXtraObject::cleanupMethods();
	g_lingo->_globalvars[xlibName] = Datum();

}

void ZipXtraXtra::m_new(int nargs) {
	g_lingo->printSTUBWithArglist("ZipXtraXtra::m_new", nargs);
	g_lingo->dropStack(nargs);
	g_lingo->push(g_lingo->_state->me);
}

XOBJSTUB(ZipXtraXtra::m_ZipFiles, 0)
XOBJSTUB(ZipXtraXtra::m_CabFiles, 0)
XOBJSTUB(ZipXtraXtra::m_JarFiles, 0)

}
