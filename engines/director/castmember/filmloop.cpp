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

#include "common/memstream.h"
#include "graphics/surface.h"
#include "graphics/macgui/macwidget.h"

#include "video/avi_decoder.h"
#include "video/qt_decoder.h"

#include "director/director.h"
#include "director/cast.h"
#include "director/channel.h"
#include "director/frame.h"
#include "director/movie.h"
#include "director/score.h"
#include "director/sprite.h"
#include "director/window.h"
#include "director/castmember/bitmap.h"
#include "director/castmember/filmloop.h"
#include "director/picture.h"


namespace Director {

FilmLoopCastMember::FilmLoopCastMember(Cast *cast, uint16 castId, Common::SeekableReadStreamEndian &stream, uint16 version)
		: CastMember(cast, castId, stream) {
	_type = kCastFilmLoop;
	_looping = true;
	_enableSound = true;
	_crop = false;
	_center = false;
	_index = -1;
	_score = nullptr;
	_flags = 0;
	_subchannelsValid = false;

	if (cast->_version >= kFileVer400) {
		_initialRect = Movie::readRect(stream);
		_flags = stream.readUint32BE();
		uint16 unk1 = stream.readUint16BE();
		_looping = _flags & 32 ? 0 : 1;
		_enableSound = _flags & 8 ? 1 : 0;
		_crop = _flags & 2 ? 0 : 1;
		_center = _flags & 1 ? 1 : 0;

		debugC(5, kDebugLoading, "FilmLoopCastMember::FilmLoopCastMember(): flags: %d, unk1: %d, looping: %d, enableSound: %d, crop: %d, center: %d", _flags, unk1, _looping, _enableSound, _crop, _center);
	}
}

FilmLoopCastMember::FilmLoopCastMember(Cast *cast, uint16 castId, FilmLoopCastMember &source)
		: CastMember(cast, castId) {
	_type = kCastFilmLoop;
	// force a load so we can copy the cast resource information
	source.load();
	_loaded = true;

	_initialRect = source._initialRect;
	_boundingRect = source._boundingRect;
	if (cast == source._cast)
		_children = source._children;

	_flags = source._flags;
	_enableSound = source._enableSound;
	_crop = source._crop;
	_center = source._center;
	_score = nullptr;
	if (source._score)
		_score = new Score(*source._score);
	_subchannels = source._subchannels;
	_subchannelsValid = false;
	_looping = source._looping;
}

FilmLoopCastMember::~FilmLoopCastMember() {
	if (_score) {
		delete _score;
		_score = nullptr;
	}
}

bool FilmLoopCastMember::isModified() {
	if (_score && _score->_scoreCache.size())
		return true;

	if (_initialRect.width() && _initialRect.height())
		return true;

	return false;
}

Common::Array<Channel> *FilmLoopCastMember::getSubChannels(Common::Rect &bbox, uint frame, int parentInk, uint32 parentForeColor, uint32 parentBackColor) {
	// A placement rect with no dimensions (a sprite that has not been sized yet) means
	// "draw the loop at its own size", which is what the cells are already stored at.
	int16 placeWidth = bbox.width() ? bbox.width() : _initialRect.width();
	int16 placeHeight = bbox.height() ? bbox.height() : _initialRect.height();

	// Drawing a frame walks a film loop twice -- once to erase the sprite's previous
	// bounding box, once to draw the new one -- and both passes ask for the same
	// cells with the same arguments. Rebuilding them is not cheap: every cell costs
	// a Sprite copy and a replaceWidget(), and a Trails loop's list grows with the
	// loop's frame number. Hand back what we already built when nothing that feeds
	// it has changed.
	//
	// The movie's frame number is part of the key, so the cells can only ever be
	// reused within the frame that built them: anything the score does between two
	// frames -- a palette switch, a cast member replaced under us -- drops the cache
	// rather than being rendered from stale widgets.
	uint16 movieFrame = 0;
	if (g_director->getCurrentMovie() && g_director->getCurrentMovie()->getScore())
		movieFrame = g_director->getCurrentMovie()->getScore()->getCurrentFrameNum();

	if (_subchannelsValid && frame == _cachedFrame && bbox == _cachedBbox
			&& parentInk == _cachedInk && parentForeColor == _cachedForeColor
			&& parentBackColor == _cachedBackColor && movieFrame == _cachedMovieFrame)
		return &_subchannels;

	_subchannelsValid = false;
	_subchannels.clear();

	if (!_score || frame >= _score->_scoreCache.size()) {
		warning("FilmLoopCastMember::getSubChannels(): Film loop frame %d requested, only %d available", frame, _score ? _score->_scoreCache.size() : 0);
		return &_subchannels;
	}

	// Build the list of cells to draw: which frame of the loop's own score each one
	// comes from, and which of its channels.
	Common::Array<FilmLoopCell> cells;

	// A cell drawn with Trails leaves its pixels behind. Inside Director the film
	// loop plays its own score onto its own canvas, so such a cell stays put and the
	// picture builds up over the loop's frames -- TKKG4's and TKKG6's intro
	// handwriting is authored exactly that way: one opaque letter per frame on a
	// single Trails channel, accumulating into a word. We rebuild this list from
	// scratch on every frame, so nothing survives from the previous one; replay what
	// the earlier frames left behind.
	//
	// What a Trails channel already drew does not depend on what it holds now: these
	// intros park the channel on an empty cell for a few frames between two words,
	// and the finished word has to stay on screen through that pause rather than
	// blink out and come back. So this is driven by the earlier frames alone, not by
	// the current one. Frame numbering is relative to the loop, so a looping film
	// loop wrapping back to frame 0 clears the trail again, as it should.
	for (uint f = 0; f < frame; f++) {
		Common::Array<Sprite *> &prevSprites = _score->_scoreCache[f]->_sprites;
		for (uint i = 0; i < prevSprites.size(); i++) {
			Sprite *prev = prevSprites[i];
			if (prev && !prev->_castId.isNull() && prev->_trails)
				cells.push_back(FilmLoopCell(f, i));
		}
	}

	// Then this frame's own cells, on top of the trail. Walking the frames in order
	// and their channels within each is the order Director paints them in, so
	// overlapping cells composite the way they did when the loop was authored.
	for (uint i = 0; i < _score->_scoreCache[frame]->_sprites.size(); ++i) {
		Sprite *cur = _score->_scoreCache[frame]->_sprites[i];
		if (cur && !cur->_castId.isNull())
			cells.push_back(FilmLoopCell(frame, i));
	}

	debugC(5, kDebugImages, "FilmLoopCastMember::getSubChannels(): castId: %d, frame: %d, count: %d, initRect: %d,%d %dx%d, bbox: %d,%d %dx%d",
			_castId, frame, cells.size(),
			_initialRect.left + _initialRect.width()/2,
			_initialRect.top + _initialRect.height()/2,
			_initialRect.width(), _initialRect.height(),
			bbox.left + bbox.width()/2,
			bbox.top + bbox.height()/2,
			bbox.width(), bbox.height());

	bool needToScale = (placeWidth != _initialRect.width() || placeHeight != _initialRect.height());
	float scaleX = 1.0f;
	float scaleY = 1.0f;

	if (needToScale) {
		scaleX = (float)placeWidth / _initialRect.width();
		scaleY = (float)placeHeight / _initialRect.height();
	}

	// Film loop placement/scaling diagnostics. Enable with: --debugflags=images --debuglevel=3
	// If the placement bbox differs from the film loop's _initialRect, the sub-sprites
	// get stretched (needToScale=1), which shows up as a distorted/oversized figure.
	debugC(3, kDebugImages, "FilmLoopCastMember::getSubChannels(): cast %d, frame %d, %d sprites: filmloop _initialRect %dx%d@%d,%d, placement bbox %dx%d@%d,%d, needToScale %d (scaleX %.3f scaleY %.3f)",
		_castId, frame, (int)cells.size(),
		_initialRect.width(), _initialRect.height(), _initialRect.left, _initialRect.top,
		bbox.width(), bbox.height(), bbox.left, bbox.top,
		needToScale, scaleX, scaleY);

	// copy the sprites in order to the list
	for (auto &iter : cells) {
		Sprite src = *_score->_scoreCache[iter.frame]->_sprites[iter.channel];
		if (src._castId.isNull())
			continue;

		// In Director, an ink effect applied to a film-loop sprite is applied to
		// every cell inside the loop. The film loop's own cells are usually
		// authored with Copy ink (they composite against each other), so when the
		// score sprite uses a transparency ink (e.g. Background Transparent) we
		// must propagate it down, otherwise each cell renders as an opaque box.
		// Only override when the parent actually set a non-Copy ink, so loops
		// placed with the default Copy ink keep their cells' own inks.
		//
		// The transparency key for Background Transparent ink is the sprite's
		// background colour, so the parent's fore/back colours must travel with
		// the ink; otherwise each cell would key its own (wrong) colour and the
		// background pixels would stay opaque.
		if (parentInk != kInkTypeCopy) {
			// Only transparency-class inks make sense to apply per cell: they decide
			// which pixels of each cell are drawn, so propagating them keys every
			// cell's background and the figure assembles cleanly.
			//
			// Blend- and colour-effect inks (Blend/Add/AddPin/Sub/SubPin/Light/Dark/
			// Reverse/...) are meant for the film loop's *composite* against the stage,
			// not for each cell. Forcing them onto every cell makes each cell blend
			// with the background individually, so the whole figure turns translucent
			// (the TKKG5 shop-scene figures whose loop sprite uses Light/Dark/AddPin).
			// We cannot composite-then-blend here, so cut the cells out cleanly with
			// Background Transparent instead -- an opaque figure beats a see-through one.
			switch (parentInk) {
			case kInkTypeBackgndTrans:
			case kInkTypeMatte:
			case kInkTypeMask:
			case kInkTypeTransparent:
				src._ink = (InkType)parentInk;
				break;
			default:
				src._ink = kInkTypeBackgndTrans;
				break;
			}
			src._foreColor = parentForeColor;
			src._backColor = parentBackColor;
		}

		if (src._cast == nullptr) {
			// castLib -1 means "the enclosing movie's cast", i.e. the film loop's
			// own cast -- resolve the cell there (this is what makes e.g. the
			// Mission to Planet X walk cycle work).
			if (src._castId.castLib == -1 && _cast != nullptr) {
				src._cast = _cast->getCastMember(src._castId.member, true);
				// Matches the bookkeeping Sprite::setCast() does; the old pointer
				// is null here, so there is nothing to release first
				if (src._cast)
					src._cast->incRefCount();
			} else {
				// castLib 0 = "no cast": an empty/disabled film-loop channel that
				// Director leaves blank. Forcing it onto the loop's own cast draws a
				// spurious bitmap -- e.g. the TKKG5 Sz40 "FB blinzelt" loop carries a
				// stale man's-arm/cane cell (HB_ar06, castLib 0) that must not render
				// while the man himself is absent. Skip it.
				continue;
			}
		}

		debugCN(5, kDebugImages, "FilmLoopCastMember::getSubChannels(): sprite: %d (from frame %d) - cast: %s, orig: %d,%d %dx%d",
				iter.channel, iter.frame, src._castId.asString().c_str(),
				src._startPoint.x, src._startPoint.y, src._width, src._height);

		// translate sprite relative to the global bounding box
		if (needToScale) {
			src._startPoint.x = (src._startPoint.x - _initialRect.left) * scaleX + bbox.left;
			src._startPoint.y = (src._startPoint.y - _initialRect.top) * scaleY + bbox.top;

			// Scale each cell by itself, don't blow it up to the whole placement rect.
			// A film loop is a miniature score: one of its frames can hold a dozen
			// cells that only add up to a figure because each keeps its own size and
			// sits at its own registration offset (TKKG7's Sz70g1 assembles a figure
			// from nine such channels). Resizing every cell to the loop's full rect
			// stacked them all on top of each other, each stretched to the size of the
			// whole figure. Start from the bitmap's native size for the same reason the
			// unscaled branch does -- cells are stored at the loop's canvas size --
			// then apply the placement scale. The registration offset follows along:
			// BitmapCastMember::getRegistrationOffset() scales it by the sprite's size
			// over the member's own, which is exactly scaleX/scaleY once the size is.
			if (src._cast && src._cast->_type == kCastBitmap) {
				src._width = src._cast->_initialRect.width();
				src._height = src._cast->_initialRect.height();
			}
			src._width = (int16)(src._width * scaleX);
			src._height = (int16)(src._height * scaleY);
			src._stretch = true;

			debugCN(5, kDebugImages, ", scaled: %d,%d %dx%d", src._startPoint.x, src._startPoint.y, src._width, src._height);
		} else {
			src._startPoint.x = (src._startPoint.x - _initialRect.left) + bbox.left;
			src._startPoint.y = (src._startPoint.y - _initialRect.top) + bbox.top;

			// Film-loop cells store each sprite at the loop's full canvas size,
			// while the per-frame bitmaps are content-cropped to different widths
			// and positioned inside that canvas via their registration point (the
			// varying regX - initialRect.left is what makes the figure walk).
			// Drawing them at the stored canvas width would stretch every frame by
			// a different factor (and scale the registration offset), producing a
			// distorted, oversized figure inside a fixed rectangle. Restore the
			// bitmap's native size so it renders 1:1, anchored by its unscaled
			// registration offset. Bitmaps already stored at native size (e.g.
			// full-frame film loops) are unaffected.
			if (src._cast && src._cast->_type == kCastBitmap) {
				src._width = src._cast->_initialRect.width();
				src._height = src._cast->_initialRect.height();
			}

			debugCN(5, kDebugImages, ", no scaling");
		}

		// Sub-sprite ink/transparency diagnostics (--debugflags=images --debuglevel=3).
		// Each sub-sprite is drawn with its OWN ink; if the figure sub-sprite uses an
		// opaque ink (Copy) or its background colour is not keyed, an opaque rectangle
		// shows around the figure. Sample the raw bitmap's corners + centre: a uniform
		// keyable margin means the background should be transparent (Fall 1); a textured
		// margin means a baked scene background (Fall 2).
		Common::Rect natRect = src._cast ? src._cast->_initialRect : Common::Rect();
		int pTL = -1, pTR = -1, pBL = -1, pBR = -1, pC = -1;
		if (src._cast && src._cast->_type == kCastBitmap) {
			Picture *pic = ((BitmapCastMember *)src._cast)->_picture;
			if (pic && pic->_surface.getPixels() && pic->_surface.format.bytesPerPixel == 1) {
				const Graphics::Surface &s = pic->_surface;
				int xr = s.w - 1, yb = s.h - 1, xc = s.w / 2, yc = s.h / 2;
				pTL = *(const byte *)s.getBasePtr(0, 0);
				pTR = *(const byte *)s.getBasePtr(xr, 0);
				pBL = *(const byte *)s.getBasePtr(0, yb);
				pBR = *(const byte *)s.getBasePtr(xr, yb);
				pC  = *(const byte *)s.getBasePtr(xc, yc);
			}
		}
		debugC(3, kDebugImages, "  film-loop sub-sprite %d (from frame %d): cast %s, ink %d, blend %d, NAT %dx%d, sprW/H %dx%d, final pos %d,%d, stretch %d, corners[TL=%d TR=%d BL=%d BR=%d C=%d]",
			iter.channel, iter.frame, src._castId.asString().c_str(), src._ink, src._blendAmount,
			natRect.width(), natRect.height(), src._width, src._height,
			src._startPoint.x, src._startPoint.y, src._stretch,
			pTL, pTR, pBL, pBR, pC);

		// Film loop frames are constructed as a series of Channels, much like how a normal frame
		// is rendered by the Score. We don't include a pointer to the current Score here,
		// that's only for querying the constraint channel which is not used.
		Channel chan(nullptr, &src);
		_subchannels.push_back(chan);
	}

	// Initialise the widgets on all of the subchannels.
	// This has to be done once the list has been constructed, otherwise
	// the list grow operation will erase the widgets as they aren't
	// part of the Channel assignment constructor.
	for (auto &iter : _subchannels) {
		iter.replaceWidget();
	}

	_cachedBbox = bbox;
	_cachedFrame = frame;
	_cachedInk = parentInk;
	_cachedForeColor = parentForeColor;
	_cachedBackColor = parentBackColor;
	_cachedMovieFrame = movieFrame;
	_subchannelsValid = true;

	return &_subchannels;
}

CastMemberID FilmLoopCastMember::getSubChannelSound1(uint frame) {
	if (!_score || frame >= _score->_scoreCache.size()) {
		warning("FilmLoopCastMember::getSubChannelSound1(): Film loop frame %d requested, only %d available", frame, _score ? _score->_scoreCache.size() : 0);
		return CastMemberID();
	}

	return _score->_scoreCache[frame]->_mainChannels.sound1;
}

CastMemberID FilmLoopCastMember::getSubChannelSound2(uint frame) {
	if (!_score || frame >= _score->_scoreCache.size()) {
		warning("FilmLoopCastMember::getSubChannelSound2(): Film loop frame %d requested, only %d available", frame, _score ? _score->_scoreCache.size() : 0);
		return CastMemberID();
	}

	return _score->_scoreCache[frame]->_mainChannels.sound2;
}

Common::String FilmLoopCastMember::formatInfo() {
	return Common::String::format(
		"initialRect: %dx%d@%d,%d, boundingRect: %dx%d@%d,%d, frameCount: %d, subchannelCount: %d, enableSound: %d, looping: %d, crop: %d, center: %d",
		_initialRect.width(), _initialRect.height(),
		_initialRect.left, _initialRect.top,
		_boundingRect.width(), _boundingRect.height(),
		_boundingRect.left, _boundingRect.top,
		_score ? _score->_scoreCache.size() : -1, _score ? _subchannels.size() : -1, _enableSound, _looping,
		_crop, _center
	);
}

void FilmLoopCastMember::load() {
	if (_loaded)
		return;

	Common::SeekableReadStreamEndian *loop = nullptr;
	uint16 filmLoopId = 0;
		uint32 tag = 0;

	if (_cast->_version < kFileVer400) {
		// Director 3 and below should have a SCVW resource
		filmLoopId = _castId + _cast->_castIDoffset;
		tag = MKTAG('S', 'C', 'V', 'W');
		loop = _cast->getResource(tag, filmLoopId);
	} else {
		// D4 and later (including D7+) store the film loop's frames in an SCVW
		// child chunk; the frame/sprite delta format is the same one the main
		// score uses, so loadFrames() handles it across versions.
		for (auto &it : _children) {
			if (it.tag == MKTAG('S', 'C', 'V', 'W')) {
				filmLoopId = it.index;
				tag = it.tag;
				loop = _cast->getResource(tag, filmLoopId);
				break;
			} else {
				debugC(5, kDebugLoading, "FilmLoopCastMember::load(): Ignoring child with tag '%s' id: %d", tag2str(it.tag), it.index);
			}
		}
	}

	if (_score) {
		delete _score;
		_score = nullptr;
	}

	if (loop) {
		debugC(2, kDebugLoading, "****** FilmLoopCastMember::load(): Loading '%s' id: %d, %d bytes", tag2str(tag), filmLoopId, (int)loop->size());
		_score = new Score(g_director->getCurrentMovie(), false);
		_score->loadFrames(*loop, _cast->_version, true);
		delete loop;
	} else {
		warning("FilmLoopCastMember::load(): Film loop not found");
	}

	_loaded = true;
}

void FilmLoopCastMember::unload() {
	// No unload necessary.
}

Common::Point FilmLoopCastMember::getRegistrationOffset() {
	return Common::Point(_initialRect.width() / 2, _initialRect.height() / 2);
}

Common::Point FilmLoopCastMember::getRegistrationOffset(int16 currentWidth, int16 currentHeight) {
	return Common::Point(currentWidth / 2, currentHeight / 2);
}

bool FilmLoopCastMember::canWriteCastData() {
	return _cast->_version >= kFileVer400 && _cast->_version < kFileVer700;
}

uint32 FilmLoopCastMember::getCastDataSize() {
	// We're only reading the _initialRect and _vflags from the Cast Data
	// _initialRect : 8 bytes + flags : 4 bytes + 2 bytes unk1 + 2 bytes (castType and _flags1 (see Cast::loadCastData() for Director 4 only)
	if (_cast->_version >= kFileVer400 && _cast->_version < kFileVer500) {
		// It has been observed that the FilmCastMember has _flags as 0x00
		return 8 + 4 + 2 + 2;
	} else if (_cast->_version >= kFileVer500 && _cast->_version < kFileVer700) {
		return 8 + 4 + 2;
	}

	warning("FilmLoopCastMember::getCastDataSize(): unhandled or invalid cast version: %d", _cast->_version);
	return 0;
}

void FilmLoopCastMember::writeCastData(Common::SeekableWriteStream *writeStream) {
	Movie::writeRect(writeStream, _initialRect);

	uint32 flags = 0;
	if (_cast->_version >= kFileVer400) {
		flags |= (_looping) ? 0 : 32;
		flags |= (_enableSound) ? 8 : 0;
		flags |= (_crop) ? 0 : 2;
		flags |= (_center) ? 1 : 0;
	}

	writeStream->writeUint32LE(flags);
	writeStream->writeUint16LE(0);		// May need to save proper value in the future, currently ignored
}

void FilmLoopCastMember::writeSCVWResource(Common::SeekableWriteStream *writeStream, uint32 offset) {
	// Load it before writing
	if (!_loaded) {
		load();
	}
	if (!_score) {
		warning("FilmLoopCastMember::writeSCVWResource: Film loop has no score, skipping");
		return;
	}

	uint32 channelSize = 0;
	if (_cast->_version >= kFileVer400 && _cast->_version < kFileVer500) {
		channelSize = kSprChannelSizeD4;
	} else if (_cast->_version >= kFileVer500 && _cast->_version < kFileVer600) {
		channelSize = kSprChannelSizeD5;
	} else {
		warning("FilmLoopCastMember::writeSCVWResource: Writing Director Version 6+ not supported yet");
		return;
	}

	// Go to the desired offset put in the memory map
	writeStream->seek(offset);

	uint32 filmloopSize = getSCVWResourceSize();
	debugC(5, kDebugSaving, "FilmLoopCastmember::writeSCVWResource: Saving FilmLoop 'SCVW' data of size: %d", filmloopSize);

	writeStream->writeUint32LE(MKTAG('S', 'C', 'V', 'W'));
	writeStream->writeUint32LE(filmloopSize);	// Size of the resource

	writeStream->writeUint32BE(filmloopSize);

	uint32 frameOffset = 20;							// Should be greater than 20
	writeStream->writeUint32BE(frameOffset);			// framesOffset
	writeStream->seek(6, SEEK_CUR);						// Ignored data
	writeStream->writeUint16BE(channelSize);
	writeStream->seek(frameOffset - 16, SEEK_CUR);				// Ignored data

	// The structure of the filmloop 'SCVW' data is as follows
	// The 'SCVW' tag -> the size of the resource ->
	// frameoffset (This offset is where the frame date actually starts) ->
	// Some headers which we ignore except the Sprite Channel Size (which we also ignore during loading) ->

	// until there are no more frames
		// size of the frame ->
		// until there are no more channels in the frame
			// width of message (One chunk of data) (This is the size of data for the sprite that needs to be read) ->
			// order of message (this order tells us the channel we're reading) ->
			// 1-20 bytes of Sprite data

	for (Frame *frame : _score->_scoreCache) {
		writeStream->writeUint16BE(frame->_sprites.size() * (channelSize + 4) + 2);					// Frame Size

		for (uint channel = 0; channel < frame->_sprites.size(); ++channel) {
			// TODO: For now writing the order considering that each sprite will have 20 bytes of data
			// In the future, for optimization, we can actually calculate the data of each sprite
			// And write the order accordingly
			// But for this we'll need a way to find how many data values (out of 20) of a sprite are valid, i.e. determine message width
			// this means while loading, the channelOffset will always be 0, order will always be multiple of 20
			// And message width will always be 20
			// Channel indexes start with 0
			writeStream->writeUint16BE(channelSize);						// message width
			writeStream->writeUint16BE(channel * channelSize);

			Sprite *sprite = frame->_sprites[channel];

			if (_cast->_version >= kFileVer400 && _cast->_version < kFileVer500) {
				writeSpriteDataD4(writeStream, *sprite);
			} else if (_cast->_version >= kFileVer500 && _cast->_version < kFileVer600) {
				writeSpriteDataD5(writeStream, *sprite);
			}
		}

	}

	// FIXME: can't dereference SeekableWriteStream
	/*if (debugChannelSet(7, kDebugSaving)) {
		// Adding +8 because the stream doesn't include the header and the entry for the size itself
		byte *dumpData = (byte *)calloc(filmloopSize + 8, sizeof(byte));

		Common::SeekableMemoryWriteStream *dumpStream = new Common::SeekableMemoryWriteStream(dumpData, filmloopSize + 8);

		uint32 currentPos = writeStream->pos();
		writeStream->seek(offset);
		dumpStream->write(writeStream, filmloopSize + 8);
		writeStream->seek(currentPos);

		dumpFile("FilmLoopData", 0, MKTAG('V', 'W', 'C', 'F'), dumpData, filmloopSize);
		free(dumpData);
		delete dumpStream;
	}*/
}

uint32 FilmLoopCastMember::getSCVWResourceSize() {
	uint32 channelSize = 0;
	if (_cast->_version >= kFileVer400 && _cast->_version < kFileVer500) {
		channelSize = kSprChannelSizeD4;
	} else if (_cast->_version >= kFileVer500) {
		channelSize = kSprChannelSizeD5;
	} else {
		warning("FilmLoopCastMember::getSCVWResourceSize: Director version unsupported");
	}

	uint32 framesSize = 0;
	if (!_score)
		return 0;
	for (Frame *frame : _score->_scoreCache) {
		// Frame size
		framesSize += 2;

		// message width: 2 bytes
		// order: 2 bytes
		// Sprite data: 20 bytes
		framesSize += (2 + 2 + channelSize) * frame->_sprites.size();
	}

	// Size: 4 bytes
	// frameoffset: 4 bytes
	// Header (Ignored data): 16 bytes
	return 4 + 4 + 16 + framesSize;
}

} // End of namespace Director
