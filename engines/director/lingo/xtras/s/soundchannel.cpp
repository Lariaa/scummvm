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
#include "director/lingo/lingo-builtins.h"
#include "director/lingo/lingo-object.h"
#include "director/lingo/lingo-utils.h"
#include "director/lingo/xtras/s/soundchannel.h"

/**************************************************
 *
 * USED IN:
 * TKKG 6-9, Opera Fatal, Oscar 4, Kommissar Kugelblitz
 *
 **************************************************/

// The Xtra's msgTable, verbatim. It really does contain a stray "/*" -- the
// author evidently commented the private routine out in their own source and it
// leaked into the table -- so this is quoted with line comments to keep the text
// exact without nesting a block comment.
//
// -- xtra SoundChannel
// new object me
// -- Template handlers --
// * sound * * -- Gets a sound object for the given channel.
// /* _soundXtra * * -- Private routine to get a sound object.

namespace Director {

const char *SoundChannelXtra::xlibName = "SoundChannel";
const XlibFileDesc SoundChannelXtra::fileNames[] = {
	{ "Sound Control",   nullptr },	// on-disk Xtra filename (Sound Control.x32)
	{ "soundchannel",    nullptr },
	{ nullptr,        nullptr },
};

static MethodProto xlibMethods[] = {
	{ "new",				SoundChannelXtra::m_new,		 0, 0,	500 },

	{ nullptr, nullptr, 0, 0, 0 }
};

static BuiltinProto xlibBuiltins[] = {
	{ "sound", SoundChannelXtra::m_sound, 1, 3, 500, HBLTIN },
	{ nullptr, nullptr, 0, 0, 0, VOIDSYM }
};

SoundChannelXtraObject::SoundChannelXtraObject(ObjectType ObjectType) :Object<SoundChannelXtraObject>("SoundChannel") {
	_objType = ObjectType;
}

// The Sound Channel object's own properties, from the Director Scripting
// Dictionary 11.5 ("Property summary for the Sound Channel object"). Everything
// listed there is accepted, so that reading one never raises a Lingo error even
// where we can only answer a plausible default -- the games use these to steer
// playback, not to introspect the engine.
static const char *sndChannelProps[] = {
	"channelCount", "currentTime", "elapsedTime", "endTime", "loopCount",
	"loopEndTime", "loopsRemaining", "loopStartTime", "member", "pan",
	"sampleCount", "sampleRate", "startTime", "status", "volume", nullptr
};

bool SoundChannelXtraObject::hasProp(const Common::String &propName) {
	if (propName == "name")
		return true;

	if (_channel > 0) {
		for (const char **it = sndChannelProps; *it; it++) {
			if (propName.equalsIgnoreCase(*it))
				return true;
		}
	}

	return false;
}

Datum SoundChannelXtraObject::getProp(const Common::String &propName) {
	if (propName == "name")
		return Datum(SoundChannelXtra::xlibName);

	DirectorSound *sound = _channel > 0 ? g_director->getCurrentWindow()->getSoundManager() : nullptr;
	if (!sound)
		return Datum();

	if (propName.equalsIgnoreCase("volume"))
		return Datum((int)sound->getChannelVolume(_channel));

	if (propName.equalsIgnoreCase("pan"))
		return Datum((int)sound->getChannelBalance(_channel));

	// 0 idle, 1 loading, 2 queued, 3 playing, 4 paused. We do not model queueing
	// or pausing, so a channel is either playing or idle.
	if (propName.equalsIgnoreCase("status"))
		return Datum(sound->isChannelActive(_channel) ? 3 : 0);

	// elapsedTime counts from the start of the sound regardless of looping or of
	// currentTime being set; currentTime is the absolute position within it. We
	// have one clock, so both report it -- Loewenzahn 3's intro compares
	// sound(1).currentTime against a list of cue times in milliseconds.
	if (propName.equalsIgnoreCase("currentTime") || propName.equalsIgnoreCase("elapsedTime"))
		return Datum((int)sound->getChannelElapsedTime(_channel));

	if (propName.equalsIgnoreCase("member")) {
		SoundID last = sound->getChannelLastPlayed(_channel);
		if (last.type == kSoundCast)
			return Datum(CastMemberID(last.u.cast.member, last.u.cast.castLib));
		return Datum();
	}

	// Defaults for the rest of the documented set: no loop is running, and the
	// sound occupies the whole of whatever is playing.
	if (propName.equalsIgnoreCase("loopCount") || propName.equalsIgnoreCase("loopsRemaining")
			|| propName.equalsIgnoreCase("loopStartTime") || propName.equalsIgnoreCase("loopEndTime")
			|| propName.equalsIgnoreCase("startTime") || propName.equalsIgnoreCase("sampleCount"))
		return Datum(0);

	if (propName.equalsIgnoreCase("endTime"))
		return Datum((int)sound->getChannelElapsedTime(_channel));

	if (propName.equalsIgnoreCase("channelCount"))
		return Datum(2);

	if (propName.equalsIgnoreCase("sampleRate"))
		return Datum(44100);

	warning("SoundChannelXtra::getProp: unknown property '%s'", propName.c_str());
	return Datum();
}

void SoundChannelXtraObject::setProp(const Common::String &propName, const Datum &value, bool force) {
	if (_channel <= 0)
		return;

	DirectorSound *sound = g_director->getCurrentWindow()->getSoundManager();
	if (!sound)
		return;

	// volume runs 0 (mute) to 255 (the machine's full level); TKKG 10, 13 and 14
	// all open their intro with sound(1).volume = 255.
	if (propName.equalsIgnoreCase("volume")) {
		sound->setChannelVolume(_channel, (uint8)CLIP<int>(value.asInt(), 0, 255));
		return;
	}

	if (propName.equalsIgnoreCase("pan")) {
		sound->setChannelBalance(_channel, (int8)CLIP<int>(value.asInt(), -100, 100));
		return;
	}

	// status, elapsedTime and the loop counters are read-only per the dictionary;
	// the rest we accept and drop rather than raise an error mid-playback.
	debugC(3, kDebugXObj, "SoundChannelXtra::setProp: ignoring '%s' on channel %d", propName.c_str(), _channel);
}

void SoundChannelXtra::open(ObjectType type, const Common::Path &path) {
    SoundChannelXtraObject::initMethods(xlibMethods);
    SoundChannelXtraObject *xobj = new SoundChannelXtraObject(type);
    if (type == kXtraObj) {
        g_lingo->_openXtras.push_back(xlibName);
		g_lingo->_openXtraObjects.push_back(xobj);
	}
    g_lingo->exposeXObject(xlibName, xobj);
    g_lingo->initBuiltIns(xlibBuiltins);
}

void SoundChannelXtra::close(ObjectType type) {
    SoundChannelXtraObject::cleanupMethods();
    g_lingo->_globalvars[xlibName] = Datum();

}

void SoundChannelXtra::m_new(int nargs) {
	g_lingo->printSTUBWithArglist("SoundChannelXtra::m_new", nargs);
	g_lingo->dropStack(nargs);
	g_lingo->push(g_lingo->_state->me);
}

void SoundChannelXtra::m_sound(int nargs) {
	// The verbs are the ones Lingo's own `sound` command takes, and so is the
	// argument order: TKKG 6-9 call sound(#stop, channel) and
	// sound(#fadeIn, channel, ticks) / sound(#fadeOut, channel, ticks).
	if (nargs >= 2) {
		LB::b_sound(nargs);
		g_lingo->push(Datum(0));
		return;
	}

	// The one-argument form hands back the Sound Channel object for that channel.
	// It has to be an object: games reach straight through it, and pushing a bare
	// 0 made `sound(1).volume = 255` fail with "setObjectProp: Invalid object: 0",
	// which under lingostrict ends the movie. That one line was the wall TKKG 10,
	// 13 and 14 and Loewenzahn 3 all stopped at.
	int channel = g_lingo->pop().asInt();

	// Datum(AbstractObject *) shares the object's own refCount, so the instance is
	// freed with the last Datum holding it. That matters because this is called
	// per frame -- Loewenzahn 3's intro asks for sound(1) on every exitFrame.
	SoundChannelXtraObject *obj = new SoundChannelXtraObject(kXtraObj);
	obj->_channel = channel;

	g_lingo->push(Datum(obj));
}

}
