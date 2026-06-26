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
#include "director/lingo/xtras/b/border.h"

/**************************************************
 *
 * USED IN:
 * TKKG 6-9, Oscar 4, Kommissar Kugelblitz
 *
 **************************************************/

/*
-- xtra Border
+ new object me
+ register object me, string code
-- methods:
getAppWin object me                                       -- gets application window (result: int hwnd)
getStageWin object me, integer appwin                     -- gets stage window (result: int hwnd)
getMovieWin object me, integer appwin, string movietitle  -- gets movie in a window (result: int hwnd)
setBorder object me, integer hwnd, integer onoff          -- sets border
 */

namespace Director {

const char *BorderXtra::xlibName = "Border";
const XlibFileDesc BorderXtra::fileNames[] = {
	{ "border",   nullptr },
	{ nullptr,        nullptr },
};

static MethodProto xlibMethods[] = {
	{ "getAppWin",				BorderXtra::m_getAppWin,		 0, 0,	500 },
	{ "getStageWin",				BorderXtra::m_getStageWin,		 1, 1,	500 },
	{ "getMovieWin",				BorderXtra::m_getMovieWin,		 2, 2,	500 },
	{ "setBorder",				BorderXtra::m_setBorder,		 2, 2,	500 },
	{ "new",				BorderXtra::m_new,		 0, 0,	500 },
	{ "register",				BorderXtra::m_register,		 1, 1,	500 },
	{ nullptr, nullptr, 0, 0, 0 }
};

static BuiltinProto xlibBuiltins[] = {

	{ nullptr, nullptr, 0, 0, 0, VOIDSYM }
};

BorderXtraObject::BorderXtraObject(ObjectType ObjectType) :Object<BorderXtraObject>("Border") {
	_objType = ObjectType;
}

bool BorderXtraObject::hasProp(const Common::String &propName) {
	return (propName == "name");
}

Datum BorderXtraObject::getProp(const Common::String &propName) {
	if (propName == "name")
		return Datum(BorderXtra::xlibName);
	warning("BorderXtra::getProp: unknown property '%s'", propName.c_str());
	return Datum();
}

void BorderXtra::open(ObjectType type, const Common::Path &path) {
    BorderXtraObject::initMethods(xlibMethods);
    BorderXtraObject *xobj = new BorderXtraObject(type);
    if (type == kXtraObj) {
        g_lingo->_openXtras.push_back(xlibName);
		g_lingo->_openXtraObjects.push_back(xobj);
	}
    g_lingo->exposeXObject(xlibName, xobj);
    g_lingo->initBuiltIns(xlibBuiltins);
}

void BorderXtra::close(ObjectType type) {
    BorderXtraObject::cleanupMethods();
    g_lingo->_globalvars[xlibName] = Datum();

}

void BorderXtra::m_new(int nargs) {
	g_lingo->printSTUBWithArglist("BorderXtra::m_new", nargs);
	g_lingo->dropStack(nargs);
	g_lingo->push(g_lingo->_state->me);
}

XOBJSTUB(BorderXtra::m_register, 0)
XOBJSTUB(BorderXtra::m_getAppWin, 0)
XOBJSTUB(BorderXtra::m_getStageWin, 0)
XOBJSTUB(BorderXtra::m_getMovieWin, 0)
XOBJSTUB(BorderXtra::m_setBorder, 0)

}
