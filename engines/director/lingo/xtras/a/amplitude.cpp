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
#include "director/sound.h"
#include "director/window.h"
#include "director/lingo/lingo.h"
#include "director/lingo/lingo-object.h"
#include "director/lingo/lingo-utils.h"
#include "director/lingo/xtras/a/amplitude.h"

/**************************************************
 *
 * USED IN:
 * Löwenzahn 7 (D8), Löwenzahn 8 (D8.5), Löwenzahn Adventskalender (D8.5),
 * Löwenzahn Spielebox (D9)
 *
 **************************************************/

/*
-- xtra Amplitude
new object me
ampSetSound object me, integer soundChannel, any srcCMem -- define a sound to be scanned for amplitude data, the sound channel will be polled during payback
ampGetCurve object me -- returns a list of amplitude values for the current sound defined by SetSound()
ampSetCurve object me, any ampList, integer soundChannel, integer loopStart, integer loopEnd, integer stereo -- sends a list (ampList) of values to the xtra
ampSetLoopPoints object me, integer loopStart, integer loopEnd -- set loop points (in whole milliseconds) for Windows playback
ampGetError object me -- returns the current error status for the Amplitude xtra.
* ampRegister string userName, string regString -- enter user name and registration string.

Properties, read out of Amplitude.x32 alongside the method table above:
ampIndex, ampValue, ampValueLeft, ampValueRight, resolution, scale, smoothed, use_dB
 */

namespace Director {

const char *AmplitudeXtra::xlibName = "Amplitude";
const XlibFileDesc AmplitudeXtra::fileNames[] = {
	{ "amplitude",   nullptr },
	{ nullptr,        nullptr },
};

static MethodProto xlibMethods[] = {
	{ "new",				AmplitudeXtra::m_new,		 0, 0,	500 },
	{ "ampSetSound",				AmplitudeXtra::m_ampSetSound,		 2, 2,	500 },
	{ "ampGetCurve",				AmplitudeXtra::m_ampGetCurve,		 0, 0,	500 },
	{ "ampSetCurve",				AmplitudeXtra::m_ampSetCurve,		 5, 5,	500 },
	{ "ampSetLoopPoints",				AmplitudeXtra::m_ampSetLoopPoints,		 2, 2,	500 },
	{ "ampGetError",				AmplitudeXtra::m_ampGetError,		 0, 0,	500 },

	{ nullptr, nullptr, 0, 0, 0 }
};

static BuiltinProto xlibBuiltins[] = {
	{ "ampRegister", AmplitudeXtra::m_ampRegister, 2, 2, 500, HBLTIN },
	{ nullptr, nullptr, 0, 0, 0, VOIDSYM }
};

AmplitudeXtraObject::AmplitudeXtraObject(ObjectType ObjectType) :Object<AmplitudeXtraObject>("Amplitude") {
	_objType = ObjectType;
}

void AmplitudeXtraObject::reset() {
	_curve = Datum();
	_soundChannel = 0;
	_loopStart = 0;
	_loopEnd = 0;
	_stereo = false;
	_resolution = 15;
	_scale = 256;
	_smoothed = false;
	_useDB = false;
	_error.clear();
}

int AmplitudeXtraObject::curveSlice() const {
	if (_curve.type != ARRAY || _curve.u.farr->arr.empty() || _soundChannel <= 0 || _resolution <= 0)
		return -1;

	Window *window = g_director->getCurrentWindow();
	DirectorSound *sound = window ? window->getSoundManager() : nullptr;
	if (!sound || !sound->isChannelActive(_soundChannel))
		return -1;

	int32 elapsed = (int32)sound->getChannelElapsedTime(_soundChannel);

	// A loop region given in milliseconds folds the playing position back into
	// itself; loopEnd 0 means the sound is not looped (ampSetCurve's parameter
	// list in the user guide).
	if (_loopEnd > _loopStart && elapsed > _loopEnd) {
		int32 span = _loopEnd - _loopStart;
		elapsed = _loopStart + (elapsed - _loopStart) % span;
	}

	// resolution is the number of values the curve holds per second, so one
	// value covers 1000/resolution milliseconds.
	int slice = (int)((elapsed * (int64)_resolution) / 1000);
	int slices = (int)_curve.u.farr->arr.size() / (_stereo ? 2 : 1);
	if (slice >= slices)
		slice = slices - 1;
	return slice;
}

int AmplitudeXtraObject::sliceValue(int slice, int side) const {
	if (slice < 0 || _curve.type != ARRAY)
		return 0;

	const Common::Array<Datum> &arr = _curve.u.farr->arr;
	int stride = _stereo ? 2 : 1;
	int slices = (int)arr.size() / stride;
	if (slice >= slices)
		return 0;

	// A stereo curve interleaves the channels -- [L, R, L, R, ...] per
	// ampGetCurve() -- so one slice is two neighbouring values.
	int offset = _stereo ? side : 0;

	// smoothed averages the value with its neighbours over three slices so the
	// curve comes out less jagged; the guide says the property is ignored while
	// use_dB is on.
	int from = slice;
	int to = slice;
	if (_smoothed && !_useDB) {
		from = MAX(0, slice - 1);
		to = MIN(slices - 1, slice + 1);
	}

	int sum = 0;
	for (int i = from; i <= to; i++)
		sum += arr[i * stride + offset].asInt();
	return sum / (to - from + 1);
}

// The property names are the ones the xtra itself carries: Amplitude.x32 lists
// ampIndex, ampValue, ampValueLeft, ampValueRight, resolution, scale, smoothed
// and use_dB, and no others. The spectrum properties and `normalize` belong to
// its successor AmplitudePro, whose user guide describes the shared ones.
bool AmplitudeXtraObject::hasProp(const Common::String &propName) {
	return propName.equalsIgnoreCase("name")
		|| propName.equalsIgnoreCase("ampIndex")
		|| propName.equalsIgnoreCase("ampValue")
		|| propName.equalsIgnoreCase("ampValueLeft")
		|| propName.equalsIgnoreCase("ampValueRight")
		|| propName.equalsIgnoreCase("resolution")
		|| propName.equalsIgnoreCase("scale")
		|| propName.equalsIgnoreCase("smoothed")
		|| propName.equalsIgnoreCase("use_dB");
}

Datum AmplitudeXtraObject::getProp(const Common::String &propName) {
	if (propName.equalsIgnoreCase("name"))
		return Datum(AmplitudeXtra::xlibName);

	if (propName.equalsIgnoreCase("resolution"))
		return Datum(_resolution);
	if (propName.equalsIgnoreCase("scale"))
		return Datum(_scale);
	if (propName.equalsIgnoreCase("smoothed"))
		return Datum(_smoothed ? 1 : 0);
	if (propName.equalsIgnoreCase("use_dB"))
		return Datum(_useDB ? 1 : 0);

	if (propName.equalsIgnoreCase("ampIndex")) {
		// The index into the list ampGetCurve() hands out, counted the way
		// Lingo counts a list so that it can be used as a subscript. No title
		// in the corpus reads this, so the base is the Lingo convention rather
		// than a measurement.
		int slice = curveSlice();
		if (slice < 0)
			return Datum(0);
		return Datum(slice * (_stereo ? 2 : 1) + 1);
	}

	if (propName.equalsIgnoreCase("ampValue")) {
		int slice = curveSlice();
		if (slice < 0)
			return Datum(0);
		if (!_stereo)
			return Datum(sliceValue(slice, 0));
		// For a stereo sound this property holds the summed mono value, which
		// ampGetMonoCurve() describes as the average of both channels.
		return Datum((sliceValue(slice, 0) + sliceValue(slice, 1)) / 2);
	}

	if (propName.equalsIgnoreCase("ampValueLeft") || propName.equalsIgnoreCase("ampValueRight")) {
		// Undefined for a mono sound, says the guide; such a movie is meant to
		// read ampValue instead.
		if (!_stereo)
			return Datum();
		return Datum(sliceValue(curveSlice(), propName.equalsIgnoreCase("ampValueRight") ? 1 : 0));
	}

	warning("AmplitudeXtra::getProp: unknown property '%s'", propName.c_str());
	return Datum();
}

void AmplitudeXtraObject::setProp(const Common::String &propName, const Datum &value, bool force) {
	// resolution and scale both take a positive integer up to 32767; the
	// others are booleans. ampIndex and the ampValue* properties are read-only.
	if (propName.equalsIgnoreCase("resolution")) {
		_resolution = CLIP<int>(value.asInt(), 1, 32767);
	} else if (propName.equalsIgnoreCase("scale")) {
		_scale = CLIP<int>(value.asInt(), 1, 32767);
	} else if (propName.equalsIgnoreCase("smoothed")) {
		_smoothed = value.asInt() != 0;
	} else if (propName.equalsIgnoreCase("use_dB")) {
		_useDB = value.asInt() != 0;
	} else {
		warning("AmplitudeXtra::setProp: cannot set property '%s'", propName.c_str());
	}
}

void AmplitudeXtra::open(ObjectType type, const Common::Path &path) {
    AmplitudeXtraObject::initMethods(xlibMethods);
    AmplitudeXtraObject *xobj = new AmplitudeXtraObject(type);
    if (type == kXtraObj) {
        g_lingo->_openXtras.push_back(xlibName);
		g_lingo->_openXtraObjects.push_back(xobj);
	}
    g_lingo->exposeXObject(xlibName, xobj);
    g_lingo->initBuiltIns(xlibBuiltins);
}

void AmplitudeXtra::close(ObjectType type) {
    AmplitudeXtraObject::cleanupMethods();
    g_lingo->_globalvars[xlibName] = Datum();

}

static AmplitudeXtraObject *receiver() {
	Datum me = g_lingo->_state->me;
	if (me.type != OBJECT || !me.u.obj)
		return nullptr;
	return static_cast<AmplitudeXtraObject *>(me.u.obj);
}

void AmplitudeXtra::m_new(int nargs) {
	g_lingo->dropStack(nargs);

	// The movies throw the old instance away and ask for a fresh one
	// (`gAmpXtra = 0`, then `new(xtra "Amplitude")`), so a new one starts on
	// the documented defaults again.
	AmplitudeXtraObject *obj = receiver();
	if (obj)
		obj->reset();

	g_lingo->push(g_lingo->_state->me);
}

// ampSetCurve(list, soundChannel, loopStart, loopEnd, stereo) hands the xtra a
// curve the movie generated while authoring, so that nothing has to be scanned
// at runtime. Every title in the corpus takes this route: Loewenzahn 8's dog
// quiz, the Adventskalender's Christmas story and the Spielebox's colour game
// all keep their amplitude lists in a Lingo handler and play the sound in
// channel 4, then drive a mouth from `the ampValue`.
void AmplitudeXtra::m_ampSetCurve(int nargs) {
	if (nargs != 5) {
		g_lingo->printSTUBWithArglist("AmplitudeXtra::m_ampSetCurve", nargs);
		g_lingo->dropStack(nargs);
		g_lingo->push(Datum(1));
		return;
	}

	Datum stereo = g_lingo->pop();
	Datum loopEnd = g_lingo->pop();
	Datum loopStart = g_lingo->pop();
	Datum channel = g_lingo->pop();
	Datum list = g_lingo->pop();

	AmplitudeXtraObject *obj = receiver();
	if (!obj) {
		g_lingo->push(Datum(1));
		return;
	}

	if (list.type != ARRAY) {
		obj->_error = "Error reading amplitude list";
		warning("AmplitudeXtra::m_ampSetCurve: curve is %s, not a list", list.type2str());
		g_lingo->push(Datum(1));
		return;
	}

	obj->_curve = list;
	obj->_soundChannel = channel.asInt();
	obj->_loopStart = loopStart.asInt();
	obj->_loopEnd = loopEnd.asInt();
	obj->_stereo = stereo.asInt() != 0;
	obj->_error.clear();

	debugC(3, kDebugXObj, "AmplitudeXtra::m_ampSetCurve: %d values%s for channel %d, loop %d..%d, resolution %d",
		(int)obj->_curve.u.farr->arr.size(), obj->_stereo ? " (stereo)" : "", obj->_soundChannel,
		obj->_loopStart, obj->_loopEnd, obj->_resolution);

	g_lingo->push(Datum(0));
}

void AmplitudeXtra::m_ampSetLoopPoints(int nargs) {
	if (nargs != 2) {
		g_lingo->printSTUBWithArglist("AmplitudeXtra::m_ampSetLoopPoints", nargs);
		g_lingo->dropStack(nargs);
		g_lingo->push(Datum(1));
		return;
	}

	Datum loopEnd = g_lingo->pop();
	Datum loopStart = g_lingo->pop();

	AmplitudeXtraObject *obj = receiver();
	if (!obj) {
		g_lingo->push(Datum(1));
		return;
	}

	// Both points at 0 means "loop at the end of the sound", which is how a
	// curve with no loop region is set up as well.
	if (loopEnd.asInt() && loopEnd.asInt() <= loopStart.asInt()) {
		obj->_error = "Invalid loop point values";
		g_lingo->push(Datum(1));
		return;
	}

	obj->_loopStart = loopStart.asInt();
	obj->_loopEnd = loopEnd.asInt();
	obj->_error.clear();
	g_lingo->push(Datum(0));
}

void AmplitudeXtra::m_ampGetCurve(int nargs) {
	g_lingo->dropStack(nargs);

	AmplitudeXtraObject *obj = receiver();
	if (!obj || obj->_curve.type != ARRAY) {
		// Scanning a sound ourselves, the ampSetSound() route, is not
		// implemented, so there is only a curve here once a movie has handed
		// one over.
		if (obj)
			obj->_error = "There is no amplitude curve loaded";
		g_lingo->push(Datum(1));
		return;
	}

	g_lingo->push(obj->_curve);
}

// Returns the last error as a string fit to put in an alert, not a code.
void AmplitudeXtra::m_ampGetError(int nargs) {
	g_lingo->dropStack(nargs);

	AmplitudeXtraObject *obj = receiver();
	Common::String message = "Amplitude Xtra: no error";
	if (obj && !obj->_error.empty())
		message = Common::String::format("Amplitude Xtra error: %s", obj->_error.c_str());

	g_lingo->push(Datum(message));
}

XOBJSTUB(AmplitudeXtra::m_ampSetSound, 0)

// The games register the xtra before instantiating it and stop on a non-zero
// result, so answering 0 is what lets them carry on.
XOBJSTUB(AmplitudeXtra::m_ampRegister, 0)

}
