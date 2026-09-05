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

#ifndef DIRECTOR_CASTMEMBER_DIGITALVIDEO_H
#define DIRECTOR_CASTMEMBER_DIGITALVIDEO_H

#include "director/castmember/castmember.h"

namespace Video {
class VideoDecoder;
}

namespace Director {

class XtraCastMember;

enum DigitalVideoType {
	kDVQuickTime,
	kDVVideoForWindows,
	kDVMPEG,
	kDVUnknown = -1,
};

class DigitalVideoCastMember : public CastMember {
public:
	DigitalVideoCastMember(Cast *cast, uint16 castId);
	DigitalVideoCastMember(Cast *cast, uint16 castId, Common::SeekableReadStreamEndian &stream, uint16 version);
	DigitalVideoCastMember(Cast *cast, uint16 castId, DigitalVideoCastMember &source);
	~DigitalVideoCastMember();

	CastMember *duplicate(Cast *cast, uint16 castId) override { return (CastMember *)(new DigitalVideoCastMember(cast, castId, *this)); }

	static CastMember *createFromXtra(Cast *cast, uint16 castId, XtraCastMember *xtra);

	bool isModified() override;
	Graphics::MacWidget *createWidget(Common::Rect &bbox, Channel *channel, SpriteType spriteType) override;

	bool loadVideoFromCast();
	bool loadVideo(Common::String path);
	void setChannel(Channel *channel) { _channel = channel; }
	void startVideo();
	void stopVideo();
	void rewindVideo();
	bool endOfVideo();

	uint getTimeScale();
	uint getMovieCurrentTime();
	uint getMovieCurrentTimeMillis();
	uint getDuration();
	uint getMovieTotalTime();
	uint getMovieTotalTimeMillis();
	void seekMovie(int stamp);
	void setStartTime(int stamp);
	void setStopTime(int stamp);
	void setMovieTime(int units);
	void setMovieRate(double rate);
	void setFrameRate(int rate);

	bool hasField(int field) override;
	Datum getField(int field) override;
	void setField(int field, const Datum &value) override;

	Common::String formatInfo() override;

	Common::Rect getInitialRect() override;
	Common::Point getRegistrationOffset() override;
	Common::Point getRegistrationOffset(int16 width, int16 height) override;

	uint32 getCastDataSize() override;
	void writeCastData(Common::SeekableWriteStream *writeStream) override;
	bool canWriteCastData() override;

	Common::String _filename;

	// An Xtra-backed member names its media in its own payload rather than in a
	// MooV child, so Cast::getVideoPath() finds nothing for it. Tabuleiro's
	// DirectMedia Xtra also states the duration there, which is worth keeping:
	// a script can ask for it before the file is ever opened.
	Common::String _externalFilename;
	uint32 _externalDurationMs;

	// An Xtra-backed member answers `the type of member` with the Xtra's symbolString,
	// not with the Director type it behaves like. Set this to the symbol from the CASt
	// record to get that; leave it empty and the member reports #digitalVideo as before.
	// See DirectMediaXtra::createCastMember for who opts in and why.
	Common::String _xtraSymbol;

	// DirectMedia's setvolume()/getvolume() work in decibels of attenuation, not in
	// Director's linear 0-255 (see xtras/d/directmedia.cpp). Games read the value back,
	// change it and write it again, so keep the decibel figure verbatim rather than
	// round-tripping it through the sprite's byte-sized linear volume. 0.0 = full.
	double _externalVolumeDb;

	uint32 _vflags;
	bool _looping;
	bool _pausedAtStart;
	bool _enableVideo;
	bool _enableSound;
	bool _crop;
	bool _center;
	bool _preload;
	int _scaleX, _scaleY;	// D7+: playback size percentages [x, y]; [100, 100] = original size
	bool _showControls;
	bool _directToStage;
	bool _avimovie, _qtmovie;
	bool _dirty;
	bool _emptyFile;
	FrameRateType _frameRateType;
	DigitalVideoType _videoType;

	byte _ditheringPalette[256*3];

	uint16 _frameRate;
	bool _getFirstFrame;

	Video::VideoDecoder *_video;
	Graphics::Surface *_lastFrame;

	Channel *_channel;
};

} // End of namespace Director

#endif
