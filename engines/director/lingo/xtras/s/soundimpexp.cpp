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
#include "director/lingo/xtras/s/soundimpexp.h"

/**************************************************
 *
 * USED IN:
 * TKKG 2 / 5 / 6
 * Loewenzahn 5 / 7 / 8 / Adventskalender / Spielebox
 * Oscar the Balloonist 3
 * Kommissar Kugelblitz, Tiger-Team 1 / 2
 *
 **************************************************/

/*
 * Macromedia's sound import/export Xtra ("Sound Import Export.x32"). Like
 * SOUNDSUP (see soundsup.cpp) it is a transcoder: it carries no message table,
 * only the MOA cast-format constants kMoaCfFormat_WAVE, kMoaCfFormat_AIFF and
 * kMoaCfFormat_snd, so Director drives it through the import/export interface
 * rather than from Lingo. There is no scriptable API to implement, and ScummVM
 * decodes those formats itself.
 *
 * This is therefore an empty stub, present only to register an opener so the
 * Xtra exists and the "Unimplemented xlib" warning goes away.
 */

namespace Director {

const char *SoundImpExpXtra::xlibName = "SoundImportExport";
const XlibFileDesc SoundImpExpXtra::fileNames[] = {
	{ "Sound Import Export",	nullptr },	// on-disk Xtra filename
	{ nullptr,					nullptr },
};

static MethodProto xlibMethods[] = {
	{ "new",	SoundImpExpXtra::m_new,		0, 0, 500 },
	{ nullptr, nullptr, 0, 0, 0 }
};

SoundImpExpXtraObject::SoundImpExpXtraObject(ObjectType ObjectType) :Object<SoundImpExpXtraObject>("SoundImportExport") {
	_objType = ObjectType;
}

bool SoundImpExpXtraObject::hasProp(const Common::String &propName) {
	return (propName == "name");
}

Datum SoundImpExpXtraObject::getProp(const Common::String &propName) {
	if (propName == "name")
		return Datum(SoundImpExpXtra::xlibName);
	warning("SoundImpExpXtra::getProp: unknown property '%s'", propName.c_str());
	return Datum();
}

void SoundImpExpXtra::open(ObjectType type, const Common::Path &path) {
	SoundImpExpXtraObject::initMethods(xlibMethods);
	SoundImpExpXtraObject *xobj = new SoundImpExpXtraObject(type);
	if (type == kXtraObj) {
		g_lingo->_openXtras.push_back(xlibName);
		g_lingo->_openXtraObjects.push_back(xobj);
	}
	g_lingo->exposeXObject(xlibName, xobj);
}

void SoundImpExpXtra::close(ObjectType type) {
	SoundImpExpXtraObject::cleanupMethods();
	g_lingo->_globalvars[xlibName] = Datum();
}

void SoundImpExpXtra::m_new(int nargs) {
	g_lingo->printSTUBWithArglist("SoundImpExpXtra::m_new", nargs);
	g_lingo->dropStack(nargs);
	g_lingo->push(g_lingo->_state->me);
}

} // End of namespace Director
