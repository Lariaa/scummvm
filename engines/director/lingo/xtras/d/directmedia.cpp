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
#include "common/util.h"

#include "director/director.h"
#include "director/channel.h"
#include "director/movie.h"
#include "director/score.h"
#include "director/sprite.h"
#include "director/castmember/digitalvideo.h"
#include "director/lingo/lingo.h"
#include "director/lingo/lingo-object.h"
#include "director/lingo/lingo-utils.h"
#include "director/lingo/xtras/d/directmedia.h"

/**************************************************
 *
 * USED IN:
 * Löwenzahn 4 (D7), Löwenzahn 5 (D8), Löwenzahn 8 (D8.5),
 * Löwenzahn Adventskalender (D8.5), Löwenzahn 3/4/6 + Spielebox (D9)
 *
 **************************************************/

/*
-- xtra DirectMedia Xtra
new object me
videoplay (sprite me) -- Plays the video
videopause (sprite me) -- Pauses the video
videoseek (sprite me, whichtime) -- Seeks to the time specified in milliseconds
videoplaysegment (sprite me, starttime, endtime) -- Plays the specified segment
setvolume (sprite me, whichvolume) -- Sets the volume
getvolume (sprite me) -- Returns the current volume
setbalance (sprite me, whichbalance) -- Sets the balance
getbalance (sprite me) -- Returns the current balance
isPastCuePoint (sprite me, cuePointIndex) -- Returns TRUE if the cue point specified has already passed
mostRecentCuePoint (sprite me) -- Returns the index of the last cuepoint passed

 */

/*
 * The Xtra builds its message table at runtime, so the block above (carved from the
 * help text) is incomplete: it lists the sprite methods only. Löwenzahn 4 ships
 * DIRECTME.X32 dated 2000-05-24, whose name table also holds a member-level
 * isDirectShowInstalled, called as `isDirectShowInstalled(member(2))`.
 */

namespace Director {

const char *DirectMediaXtra::xlibName = "DirectMedia";
const XlibFileDesc DirectMediaXtra::fileNames[] = {
	{ "Directme",      nullptr },	// on-disk Xtra filename (DIRECTME.X32)
	{ "directmedia",   nullptr },
	{ "TBDirectMedia", nullptr },	// Tabuleiro's own name for it; Loewenzahn 5 ships this one
	{ nullptr,        nullptr },
};

static MethodProto xlibMethods[] = {
	{ "new",				DirectMediaXtra::m_new,		 0, 0,	500 },
	{ "videoplay",				DirectMediaXtra::m_videoplay,		 0, 0,	500 },
	{ "videopause",				DirectMediaXtra::m_videopause,		 0, 0,	500 },
	{ "videoseek",				DirectMediaXtra::m_videoseek,		 1, 1,	500 },
	{ "videoplaysegment",				DirectMediaXtra::m_videoplaysegment,		 2, 2,	500 },
	{ "setvolume",				DirectMediaXtra::m_setvolume,		 1, 1,	500 },
	{ "getvolume",				DirectMediaXtra::m_getvolume,		 0, 0,	500 },
	{ "setbalance",				DirectMediaXtra::m_setbalance,		 1, 1,	500 },
	{ "getbalance",				DirectMediaXtra::m_getbalance,		 0, 0,	500 },
	{ "isPastCuePoint",				DirectMediaXtra::m_isPastCuePoint,		 1, 1,	500 },
	{ "mostRecentCuePoint",				DirectMediaXtra::m_mostRecentCuePoint,		 0, 0,	500 },

	{ nullptr, nullptr, 0, 0, 0 }
};

static BuiltinProto xlibBuiltins[] = {
	{ "isDirectShowInstalled", DirectMediaXtra::m_isDirectShowInstalled, 1, 1, 500, HBLTIN },

	// The video calls are sprite methods -- the msgTable above spells every one of
	// them "(sprite me, ...)" -- and Director dispatches those as free functions
	// that take the sprite as their first argument. Registering them only as
	// object methods left `videoseek(sprite(n), 0)` reaching nothing, and an
	// undefined handler ends the movie under lingostrict. That is where
	// Loewenzahn 5 stopped. Argument counts include the sprite, as written in the
	// table; the methods stay registered as well, so an object-style call is
	// unaffected.
	{ "videoplay",			DirectMediaXtra::m_videoplay,			1, 1, 500, HBLTIN },
	{ "videopause",			DirectMediaXtra::m_videopause,			1, 1, 500, HBLTIN },
	{ "videoseek",			DirectMediaXtra::m_videoseek,			2, 2, 500, HBLTIN },
	{ "videoplaysegment",	DirectMediaXtra::m_videoplaysegment,	3, 3, 500, HBLTIN },
	{ "setvolume",			DirectMediaXtra::m_setvolume,			2, 2, 500, HBLTIN },
	{ "getvolume",			DirectMediaXtra::m_getvolume,			1, 1, 500, HBLTIN },
	{ "setbalance",			DirectMediaXtra::m_setbalance,			2, 2, 500, HBLTIN },
	{ "getbalance",			DirectMediaXtra::m_getbalance,			1, 1, 500, HBLTIN },
	{ "isPastCuePoint",		DirectMediaXtra::m_isPastCuePoint,		2, 2, 500, HBLTIN },
	{ "mostRecentCuePoint",	DirectMediaXtra::m_mostRecentCuePoint,	1, 1, 500, HBLTIN },
	{ nullptr, nullptr, 0, 0, 0, VOIDSYM }
};

DirectMediaXtraObject::DirectMediaXtraObject(ObjectType ObjectType) :Object<DirectMediaXtraObject>("DirectMedia") {
	_objType = ObjectType;
}

bool DirectMediaXtraObject::hasProp(const Common::String &propName) {
	return (propName == "name");
}

Datum DirectMediaXtraObject::getProp(const Common::String &propName) {
	if (propName == "name")
		return Datum(DirectMediaXtra::xlibName);
	warning("DirectMediaXtra::getProp: unknown property '%s'", propName.c_str());
	return Datum();
}

void DirectMediaXtra::open(ObjectType type, const Common::Path &path) {
    DirectMediaXtraObject::initMethods(xlibMethods);
    DirectMediaXtraObject *xobj = new DirectMediaXtraObject(type);
    if (type == kXtraObj) {
        g_lingo->_openXtras.push_back(xlibName);
		g_lingo->_openXtraObjects.push_back(xobj);
	}
    g_lingo->exposeXObject(xlibName, xobj);
    g_lingo->initBuiltIns(xlibBuiltins);
}

void DirectMediaXtra::close(ObjectType type) {
    DirectMediaXtraObject::cleanupMethods();
    g_lingo->_globalvars[xlibName] = Datum();

}

void DirectMediaXtra::m_new(int nargs) {
	g_lingo->printSTUBWithArglist("DirectMediaXtra::m_new", nargs);
	g_lingo->dropStack(nargs);
	g_lingo->push(g_lingo->_state->me);
}

// The msgTable spells every one of these "(sprite me, ...)", so the first
// argument is the sprite the media plays in. Resolve it to the channel and its
// cast member; everything the Xtra can do is something DigitalVideoCastMember
// already offers, since a DirectMedia member is promoted to one on load.
static DigitalVideoCastMember *resolveVideo(int nargs, Channel **outChannel = nullptr) {
	if (nargs < 1) {
		warning("DirectMediaXtra: called without a sprite");
		return nullptr;
	}

	Movie *movie = g_director->getCurrentMovie();
	Score *score = movie ? movie->getScore() : nullptr;
	if (!score)
		return nullptr;

	Channel *channel = score->getChannelById(g_lingo->peek(nargs - 1).asInt());
	if (!channel || !channel->_sprite || !channel->_sprite->_cast)
		return nullptr;

	if (channel->_sprite->_cast->_type != kCastDigitalVideo) {
		warning("DirectMediaXtra: sprite %d is not a media member", g_lingo->peek(nargs - 1).asInt());
		return nullptr;
	}

	DigitalVideoCastMember *video = (DigitalVideoCastMember *)channel->_sprite->_cast;
	video->setChannel(channel);
	if (outChannel)
		*outChannel = channel;

	return video;
}

// videoseek() and videoplaysegment() are documented in milliseconds, while
// seekMovie()/setStopTime() take the movie's own time scale (600 units a second
// by default, cf. getTimeScale()).
static int msToUnits(DigitalVideoCastMember *video, int ms) {
	return (int)((int64)ms * video->getTimeScale() / 1000);
}

void DirectMediaXtra::m_videoplay(int nargs) {
	DigitalVideoCastMember *video = resolveVideo(nargs);
	g_lingo->dropStack(nargs);

	if (video)
		video->setMovieRate(1.0);
}

void DirectMediaXtra::m_videopause(int nargs) {
	DigitalVideoCastMember *video = resolveVideo(nargs);
	g_lingo->dropStack(nargs);

	if (video)
		video->setMovieRate(0.0);
}

void DirectMediaXtra::m_videoseek(int nargs) {
	DigitalVideoCastMember *video = resolveVideo(nargs);
	int ms = (nargs >= 2) ? g_lingo->peek(0).asInt() : 0;
	g_lingo->dropStack(nargs);

	if (video)
		video->seekMovie(msToUnits(video, ms));
}

void DirectMediaXtra::m_videoplaysegment(int nargs) {
	DigitalVideoCastMember *video = resolveVideo(nargs);
	int startMs = (nargs >= 3) ? g_lingo->peek(1).asInt() : 0;
	int endMs = (nargs >= 3) ? g_lingo->peek(0).asInt() : 0;
	g_lingo->dropStack(nargs);

	if (!video)
		return;

	video->seekMovie(msToUnits(video, startMs));
	video->setStopTime(msToUnits(video, endMs));
	video->setMovieRate(1.0);
}

void DirectMediaXtra::m_setvolume(int nargs) {
	Channel *channel = nullptr;
	DigitalVideoCastMember *video = resolveVideo(nargs, &channel);
	int volume = (nargs >= 2) ? g_lingo->peek(0).asInt() : 0;
	g_lingo->dropStack(nargs);

	// Same store `the volume of sprite` uses, so the two agree -- Loewenzahn 3
	// sets the sprite property directly and reads it back through this Xtra.
	if (video && channel)
		channel->_sprite->_volume = CLIP(volume, 0, 255);
}

void DirectMediaXtra::m_getvolume(int nargs) {
	Channel *channel = nullptr;
	DigitalVideoCastMember *video = resolveVideo(nargs, &channel);
	g_lingo->dropStack(nargs);

	g_lingo->push(Datum((video && channel) ? channel->_sprite->_volume : 0));
}

XOBJSTUB(DirectMediaXtra::m_setbalance, 0)
XOBJSTUB(DirectMediaXtra::m_getbalance, 0)
XOBJSTUB(DirectMediaXtra::m_isPastCuePoint, 0)
XOBJSTUB(DirectMediaXtra::m_mostRecentCuePoint, 0)

void DirectMediaXtra::m_isDirectShowInstalled(int nargs) {
	// Whether DirectShow is available to play the member's video. Löwenzahn 4 gates
	// its MPEG intro on this (MPEG\INTROW, `if isDirectShowInstalled(member(2)) = 0
	// then go("noMP")`), so answer as a Windows install with DirectShow would.
	g_lingo->dropStack(nargs);
	g_lingo->push(Datum(1));
}

}
