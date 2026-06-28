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
#include "director/movie.h"
#include "director/util.h"
#include "director/lingo/lingo.h"
#include "director/lingo/lingo-object.h"
#include "director/lingo/lingo-utils.h"
#include "director/lingo/xtras/k/keypoll.h"

/**************************************************
 *
 * USED IN:
 * Safecracker
 * Teazle
 * Loewenzahn 2 / 4 / 5
 *
 **************************************************/

/*
-- -- KeyPoll Xtra
-- by Brian Gray
-- (c) 1996 Macromedia, Inc.  All Rights Reserved.

xtra KeyPoll
new object me

-- KeyPoll handlers --
* bgOneKey integer keyCode -- returns TRUE if key (argument) is down, else FALSE
* bgAllKeys -- returns a linear list of the keycodes of every key currently down

 */

namespace Director {

const char *const KeypollXtra::xlibName = "Keypoll";
const XlibFileDesc KeypollXtra::fileNames[] = {
	{ "keypoll",   nullptr },
	{ "KeyPollFat",   nullptr },
	{ nullptr,        nullptr },
};

static const MethodProto xlibMethods[] = {
	{ "xtra",				KeypollXtra::m_xtra,		 0, 0,	500 },
	{ "new",				KeypollXtra::m_new,		 0, 0,	500 },
	{ nullptr, nullptr, 0, 0, 0 }
};

static const BuiltinProto xlibBuiltins[] = {
	{ "bgOneKey", KeypollXtra::m_bgOneKey, 1, 1, 500, HBLTIN },
	{ "bgAllKeys", KeypollXtra::m_bgAllKeys, 0, 0, 500, HBLTIN },
	{ nullptr, nullptr, 0, 0, 0, VOIDSYM }
};

KeypollXtraObject::KeypollXtraObject(ObjectType ObjectType) :Object<KeypollXtraObject>("Keypoll") {
	_objType = ObjectType;
}

bool KeypollXtraObject::hasProp(const Common::String &propName) {
	return (propName == "name");
}

Datum KeypollXtraObject::getProp(const Common::String &propName) {
	if (propName == "name")
		return Datum(KeypollXtra::xlibName);
	warning("KeypollXtra::getProp: unknown property '%s'", propName.c_str());
	return Datum();
}

void KeypollXtra::open(ObjectType type, const Common::Path &path) {
    KeypollXtraObject::initMethods(xlibMethods);
    KeypollXtraObject *xobj = new KeypollXtraObject(type);
	if (type == kXtraObj) {
		g_lingo->_openXtras.push_back(xlibName);
		g_lingo->_openXtraObjects.push_back(xobj);
	}
    g_lingo->exposeXObject(xlibName, xobj);
    g_lingo->initBuiltIns(xlibBuiltins);
}

void KeypollXtra::close(ObjectType type) {
    KeypollXtraObject::cleanupMethods();
    g_lingo->_globalvars[xlibName] = Datum();

}

void KeypollXtra::m_new(int nargs) {
	g_lingo->printSTUBWithArglist("KeypollXtra::m_new", nargs);
	g_lingo->dropStack(nargs);
	g_lingo->push(g_lingo->_state->me);
}

XOBJSTUB(KeypollXtra::m_xtra, 0)

// The KeyPoll Xtra reports key state using platform virtual key codes: Windows
// VK codes when running as a Windows projector (machineType 256), Mac key codes
// otherwise. This matches what the game passes in, since it branches on
// "the machineType" when choosing the key codes to query.
static bool isWindowsKeyPoll() {
	return g_director->_machineType == 256;
}

static int keyToPlatformCode(Common::KeyCode code) {
	return isWindowsKeyPoll() ? scummVMKeyCodeToWin(code) : scummVMKeyCodeToMac(code);
}

// Modifier keys are reported via key flags rather than the held-key set.
static bool keyPollModifierDown(int requestedKey, byte flags) {
	if (isWindowsKeyPoll()) {
		switch (requestedKey) {
		case 16: return flags & Common::KBD_SHIFT;	// VK_SHIFT
		case 17: return flags & Common::KBD_CTRL;	// VK_CONTROL
		case 18: return flags & Common::KBD_ALT;	// VK_MENU (Alt)
		default: return false;
		}
	}
	switch (requestedKey) {
	case 56: return flags & Common::KBD_SHIFT;	// shift
	case 54: // fallthrough
	case 59: return flags & Common::KBD_CTRL;	// control
	case 58: return flags & Common::KBD_ALT;	// option
	case 55: return flags & Common::KBD_META;	// command
	default: return false;
	}
}

void KeypollXtra::m_bgOneKey(int nargs) {
	ARGNUMCHECK(1);

	int requestedKey = g_lingo->pop().asInt();
	Movie *movie = g_director->getCurrentMovie();
	bool down = false;

	if (movie) {
		if (keyPollModifierDown(requestedKey, g_director->_keyFlags)) {
			down = true;
		} else {
			for (auto &it : g_director->_keysDown) {
				if (keyToPlatformCode((Common::KeyCode)it._key) == requestedKey) {
					down = true;
					break;
				}
			}
		}
	}

	g_lingo->push(down ? 1 : 0);
}

void KeypollXtra::m_bgAllKeys(int nargs) {
	ARGNUMCHECK(0);

	Datum result;
	result.type = ARRAY;
	result.u.farr = new FArray;

	Movie *movie = g_director->getCurrentMovie();
	if (movie) {
		for (auto &it : g_director->_keysDown) {
			int code = keyToPlatformCode((Common::KeyCode)it._key);
			if (code >= 0)
				result.u.farr->arr.push_back(Datum(code));
		}

		byte flags = g_director->_keyFlags;
		if (isWindowsKeyPoll()) {
			if (flags & Common::KBD_SHIFT) result.u.farr->arr.push_back(Datum(16));
			if (flags & Common::KBD_CTRL)  result.u.farr->arr.push_back(Datum(17));
			if (flags & Common::KBD_ALT)   result.u.farr->arr.push_back(Datum(18));
		} else {
			if (flags & Common::KBD_SHIFT) result.u.farr->arr.push_back(Datum(56));
			if (flags & Common::KBD_CTRL)  result.u.farr->arr.push_back(Datum(54));
			if (flags & Common::KBD_ALT)   result.u.farr->arr.push_back(Datum(58));
			if (flags & Common::KBD_META)  result.u.farr->arr.push_back(Datum(55));
		}
	}

	g_lingo->push(result);
}

}
