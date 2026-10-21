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

#ifndef DIRECTOR_LINGO_XTRAS_A_AMPLITUDE_H
#define DIRECTOR_LINGO_XTRAS_A_AMPLITUDE_H

namespace Director {

class AmplitudeXtraObject : public Object<AmplitudeXtraObject> {
public:
	AmplitudeXtraObject(ObjectType objType);

	bool hasProp(const Common::String &propName) override;
	Datum getProp(const Common::String &propName) override;
	void setProp(const Common::String &propName, const Datum &value, bool force = false) override;

	void reset();

	// Which slice of the curve the watched channel is playing right now, as an
	// index into _curve, or -1 if there is nothing to answer.
	int curveSlice() const;
	// One channel's value out of that slice: 0 for the left (or mono) value,
	// 1 for the right one.
	int sliceValue(int slice, int side) const;

	// Set by ampSetCurve(): the curve the movie generated while authoring, the
	// sound channel it watches, the loop region in milliseconds, and whether
	// the values are interleaved left/right.
	Datum _curve;
	int _soundChannel = 0;
	int _loopStart = 0;
	int _loopEnd = 0;
	bool _stereo = false;

	// Defaults from the AmplitudePro user guide's property reference, which
	// documents the same properties this older xtra carries.
	int _resolution = 15;
	int _scale = 256;
	bool _smoothed = false;
	bool _useDB = false;

	// What ampGetError() reports; empty means no error.
	Common::String _error;
};

namespace AmplitudeXtra {

extern const char *xlibName;
extern const XlibFileDesc fileNames[];

void open(ObjectType type, const Common::Path &path);
void close(ObjectType type);

void m_new(int nargs);
void m_ampSetSound(int nargs);
void m_ampGetCurve(int nargs);
void m_ampSetCurve(int nargs);
void m_ampSetLoopPoints(int nargs);
void m_ampGetError(int nargs);
void m_ampRegister(int nargs);

} // End of namespace AmplitudeXtra

} // End of namespace Director

#endif
