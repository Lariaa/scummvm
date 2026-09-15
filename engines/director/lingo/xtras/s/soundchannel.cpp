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
#include "director/score.h"
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
 * TKKG 6-14, Loewenzahn 3, Opera Fatal, Oscar 4, Kommissar Kugelblitz
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

	// The Sound Channel object's own methods, new in D8 (Director 8.5 Shockwave
	// Studio: The Complete Reference, ch. 8; Director MX manual, "Playing sounds
	// with Lingo"). TKKG 10 to 14 drive their sound through them.
	{ "breakLoop",			SoundChannelXtra::m_breakLoop,	 0, 0,	800 },
	{ "fadeIn",				SoundChannelXtra::m_fadeIn,		 0, 1,	800 },
	{ "fadeOut",			SoundChannelXtra::m_fadeOut,	 0, 1,	800 },
	{ "fadeTo",				SoundChannelXtra::m_fadeTo,		 1, 2,	800 },
	{ "getPlayList",		SoundChannelXtra::m_getPlayList, 0, 0,	800 },
	{ "isBusy",				SoundChannelXtra::m_isBusy,		 0, 0,	800 },
	{ "pause",				SoundChannelXtra::m_pause,		 0, 0,	800 },
	{ "play",				SoundChannelXtra::m_play,		 0, 1,	800 },
	{ "playNext",			SoundChannelXtra::m_playNext,	 0, 0,	800 },
	{ "queue",				SoundChannelXtra::m_queue,		 1, 1,	800 },
	{ "rewind",				SoundChannelXtra::m_rewind,		 0, 0,	800 },
	{ "setPlayList",		SoundChannelXtra::m_setPlayList, 1, 1,	800 },
	{ "stop",				SoundChannelXtra::m_stop,		 0, 0,	800 },

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

	// 0 idle, 1 loading, 2 queued, 3 playing, 4 paused. Nothing preloads here, so
	// a channel is never loading.
	if (propName.equalsIgnoreCase("status")) {
		if (sound->isQueuePaused(_channel))
			return Datum(4);
		if (sound->isChannelActive(_channel))
			return Datum(3);
		return Datum(sound->getPlayList(_channel).empty() ? 0 : 2);
	}

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

// The channel of the Sound Channel object a method was called on; 0 for the
// instance the Xtra registers itself under.
static int receiverChannel() {
	Datum me = g_lingo->_state->me;
	if (me.type != OBJECT || !me.u.obj)
		return 0;
	return static_cast<SoundChannelXtraObject *>(me.u.obj)->_channel;
}

static DirectorSound *channelSound(int channel) {
	if (channel <= 0)
		return nullptr;
	Window *window = g_director->getCurrentWindow();
	return window ? window->getSoundManager() : nullptr;
}

// Lingo passes milliseconds; the fader counts ticks.
static int msToTicks(int ms) {
	return MAX(1, ms * 60 / 1000);
}

// The score only runs the fader while it knows a fade is under way.
static void markFadeActive() {
	Movie *movie = g_director->getCurrentMovie();
	if (movie && movie->getScore())
		movie->getScore()->_activeFade = true;
}

// A play list entry from what Lingo passes: a member, or a property list with at
// least #member. #startTime, #endTime, #loopStartTime, #loopEndTime,
// #preLoadTime and #rateShift are accepted and ignored.
static bool readQueueEntry(const Datum &d, SoundQueueEntry &entry) {
	Datum member = d;
	entry.loopCount = 1;
	if (d.type == PARRAY) {
		member = Datum();
		for (uint i = 0; i < d.u.parr->arr.size(); i++) {
			const PCell &cell = d.u.parr->arr[i];
			Common::String key = cell.p.asString();
			if (key.equalsIgnoreCase("member"))
				member = cell.v;
			else if (key.equalsIgnoreCase("loopCount"))
				entry.loopCount = MAX(0, cell.v.asInt());
		}
	}

	if (!member.isVoid())
		entry.member = member.asMemberID(kCastSound);
	if (entry.member.member == 0) {
		warning("SoundChannelXtra: no sound member in %s", d.asString(true).c_str());
		return false;
	}
	return true;
}

void SoundChannelXtra::m_queue(int nargs) {
	if (nargs > 1)
		g_lingo->dropStack(nargs - 1);
	Datum arg = nargs > 0 ? g_lingo->pop() : Datum();

	int channel = receiverChannel();
	DirectorSound *sound = channelSound(channel);
	SoundQueueEntry entry;
	if (sound && readQueueEntry(arg, entry))
		sound->queueSound(channel, entry);
}

void SoundChannelXtra::m_setPlayList(int nargs) {
	if (nargs > 1)
		g_lingo->dropStack(nargs - 1);
	Datum list = nargs > 0 ? g_lingo->pop() : Datum();

	int channel = receiverChannel();
	DirectorSound *sound = channelSound(channel);
	if (!sound)
		return;

	// Replaces what is queued and leaves the sound playing now alone;
	// setPlayList([]) is how a script empties the queue (Director MX manual).
	Common::Array<SoundQueueEntry> entries;
	if (list.type == ARRAY) {
		for (uint i = 0; i < list.u.farr->arr.size(); i++) {
			SoundQueueEntry entry;
			if (readQueueEntry(list.u.farr->arr[i], entry))
				entries.push_back(entry);
		}
	} else if (!list.isVoid()) {
		warning("SoundChannelXtra::m_setPlayList(): expected a list, got %s", list.type2str());
	}
	sound->setPlayList(channel, entries);
}

void SoundChannelXtra::m_getPlayList(int nargs) {
	g_lingo->dropStack(nargs);

	int channel = receiverChannel();
	DirectorSound *sound = channelSound(channel);

	// The sounds still waiting, each as the property list queue() takes.
	Datum result;
	result.type = ARRAY;
	result.u.farr = new FArray;
	if (sound) {
		Common::Array<SoundQueueEntry> entries = sound->getPlayList(channel);
		for (uint i = 0; i < entries.size(); i++) {
			Datum memberKey("member");
			memberKey.type = SYMBOL;
			Datum loopKey("loopCount");
			loopKey.type = SYMBOL;

			Datum props;
			props.type = PARRAY;
			props.u.parr = new PArray;
			props.u.parr->arr.push_back(PCell(memberKey, Datum(entries[i].member)));
			props.u.parr->arr.push_back(PCell(loopKey, Datum(entries[i].loopCount)));
			result.u.farr->arr.push_back(props);
		}
	}
	g_lingo->push(result);
}

void SoundChannelXtra::m_play(int nargs) {
	if (nargs > 1)
		g_lingo->dropStack(nargs - 1);
	Datum arg = nargs > 0 ? g_lingo->pop() : Datum();

	int channel = receiverChannel();
	DirectorSound *sound = channelSound(channel);
	if (!sound)
		return;

	// play() starts the queue or resumes a pause; play(member) puts that sound
	// ahead of anything queued and starts it at once.
	SoundQueueEntry entry;
	if (nargs > 0 && readQueueEntry(arg, entry))
		sound->playNow(channel, entry);
	else
		sound->playQueue(channel);
}

void SoundChannelXtra::m_stop(int nargs) {
	g_lingo->dropStack(nargs);
	int channel = receiverChannel();
	if (DirectorSound *sound = channelSound(channel))
		sound->stopQueue(channel);
}

void SoundChannelXtra::m_pause(int nargs) {
	g_lingo->dropStack(nargs);
	int channel = receiverChannel();
	if (DirectorSound *sound = channelSound(channel))
		sound->pauseQueue(channel);
}

void SoundChannelXtra::m_rewind(int nargs) {
	g_lingo->dropStack(nargs);
	int channel = receiverChannel();
	if (DirectorSound *sound = channelSound(channel))
		sound->rewindQueue(channel);
}

void SoundChannelXtra::m_playNext(int nargs) {
	g_lingo->dropStack(nargs);
	int channel = receiverChannel();
	if (DirectorSound *sound = channelSound(channel))
		sound->playNext(channel);
}

void SoundChannelXtra::m_breakLoop(int nargs) {
	g_lingo->dropStack(nargs);
	int channel = receiverChannel();
	if (DirectorSound *sound = channelSound(channel))
		sound->breakLoop(channel);
}

void SoundChannelXtra::m_isBusy(int nargs) {
	g_lingo->dropStack(nargs);
	int channel = receiverChannel();
	DirectorSound *sound = channelSound(channel);
	g_lingo->push(Datum((sound && sound->isChannelActive(channel) && !sound->isQueuePaused(channel)) ? 1 : 0));
}

// fadeIn() drops the channel to silence at once and brings it back up to its
// volume, fadeOut() takes it down to 0, fadeTo() to the volume given. Times are in
// milliseconds, 1000 when left out, and the channel keeps the volume a fade
// arrives at.
void SoundChannelXtra::m_fadeIn(int nargs) {
	if (nargs > 1)
		g_lingo->dropStack(nargs - 1);
	int ms = nargs > 0 ? g_lingo->pop().asInt() : 1000;

	int channel = receiverChannel();
	if (DirectorSound *sound = channelSound(channel)) {
		sound->fadeChannelIn(channel, msToTicks(ms));
		markFadeActive();
	}
}

void SoundChannelXtra::m_fadeOut(int nargs) {
	if (nargs > 1)
		g_lingo->dropStack(nargs - 1);
	int ms = nargs > 0 ? g_lingo->pop().asInt() : 1000;

	int channel = receiverChannel();
	if (DirectorSound *sound = channelSound(channel)) {
		sound->fadeChannelTo(channel, 0, msToTicks(ms));
		markFadeActive();
	}
}

void SoundChannelXtra::m_fadeTo(int nargs) {
	if (nargs > 2)
		g_lingo->dropStack(nargs - 2);
	int ms = nargs >= 2 ? g_lingo->pop().asInt() : 1000;
	int volume = nargs >= 1 ? CLIP<int>(g_lingo->pop().asInt(), 0, 255) : 255;

	int channel = receiverChannel();
	if (DirectorSound *sound = channelSound(channel)) {
		sound->fadeChannelTo(channel, volume, msToTicks(ms));
		markFadeActive();
	}
}

}
