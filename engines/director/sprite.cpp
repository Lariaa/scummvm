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

#include "graphics/macgui/macwidget.h"

#include "director/director.h"
#include "director/frame.h"
#include "director/movie.h"
#include "director/score.h"
#include "director/sprite.h"
#include "director/castmember/castmember.h"
#include "director/castmember/digitalvideo.h"
#include "director/castmember/movie.h"
#include "director/castmember/shape.h"
#include "director/castmember/text.h"

namespace Director {

Sprite::Sprite(Frame *frame) {
	_frame = frame;
	_score = _frame ? _frame->getScore() : nullptr;
	_movie = _score ? _score->getMovie() : nullptr;

	_matte = nullptr;
	_puppet = false;
	_autoPuppet = kAPNone; // Based on Director in a Nutshell, page 15
	_cast = nullptr;
	reset();
}

void Sprite::reset() {
	_copyBackMask = static_cast<uint32>(kSCBNoMask);
	_scriptId = CastMemberID(0, 0);
	_colorcode = 0;
	_blendAmount = 0;
	_unk3 = 0;

	_enabled = false;
	_castId = CastMemberID(0, 0);
	_pattern = 0;

	_spriteType = kInactiveSprite;
	_inkData = 0;
	_ink = kInkTypeCopy;
	_trails = false;

	if (_matte)
		delete _matte;
	_matte = nullptr;
	if (_cast) {
		_cast->decRefCount();
		_cast = nullptr;
	}

	_thickness = 0;
	_width = 0;
	_height = 0;
	_moveable = false;
	_editable = false;
	_immediate = false;
	_backColor = g_director->_wm->_colorWhite;
	_foreColor = g_director->_wm->_colorWhite;

	_volume = 0;
	_stretch = false;

	_spriteListIdx = 0; // D6+

	// D7+
	_flags = 0;
	_fgColorG = _fgColorB = 0;
	_bgColorG = _bgColorB = 0;
	_angleRot = 0;
	_angleSkew = 0;
}

Sprite& Sprite::operator=(const Sprite &sprite) {
	if (this == &sprite) {
		return *this;
	}

	_copyBackMask = sprite._copyBackMask;
	_frame = sprite._frame;
	_score = sprite._score;
	_movie = sprite._movie;

	_scriptId = sprite._scriptId;
	_colorcode = sprite._colorcode;
	_blendAmount = sprite._blendAmount;
	_unk3 = sprite._unk3;

	_enabled = sprite._enabled;
	_castId = sprite._castId;
	_pattern = sprite._pattern;

	_spriteType = sprite._spriteType;
	_inkData = sprite._inkData;
	_ink = sprite._ink;
	_trails = sprite._trails;

	_cast = sprite._cast;
	if (_cast)
		_cast->incRefCount();

	if (_matte)
		delete _matte;
	_matte = nullptr;

	_thickness = sprite._thickness;
	_startPoint = sprite._startPoint;
	_width = sprite._width;
	_height = sprite._height;
	_moveable = sprite._moveable;
	_editable = sprite._editable;
	_puppet = sprite._puppet;
	_autoPuppet = sprite._autoPuppet;
	_immediate = sprite._immediate;
	_backColor = sprite._backColor;
	_foreColor = sprite._foreColor;

	_volume = sprite._volume;
	_stretch = sprite._stretch;

	_spriteInfo = sprite._spriteInfo;
	_spriteListIdx = sprite._spriteListIdx;

	_flags = sprite._flags;
	_fgColorG = sprite._fgColorG;
	_fgColorB = sprite._fgColorB;
	_bgColorG = sprite._bgColorG;
	_bgColorB = sprite._bgColorB;
	_angleRot = sprite._angleRot;
	_angleSkew = sprite._angleSkew;

	_behaviors = sprite._behaviors;

	return *this;
}

bool Sprite::operator==(const Sprite &sprite) {
	return _spriteType == sprite._spriteType &&
		_castId == sprite._castId &&
		_startPoint == sprite._startPoint &&
		_width == sprite._width &&
		_height == sprite._height &&
		_ink == sprite._ink &&
		_foreColor == sprite._foreColor &&
		_backColor == sprite._backColor &&
		_blendAmount == sprite._blendAmount &&
		_ink == sprite._ink &&
		(_thickness | kTTweened) == (sprite._thickness | kTTweened);
}

Sprite::Sprite(const Sprite &sprite) {
	_matte = nullptr;
	*this = sprite;
}

Sprite::~Sprite() {
	if (_cast) {
		_cast->decRefCount();
		_cast = nullptr;
	}

	if (_matte) {
		delete _matte;
		_matte = nullptr;
	}
}

bool Sprite::isQDShape() {
	return _spriteType == kRectangleSprite ||
		_spriteType == kRoundedRectangleSprite ||
		_spriteType == kOvalSprite ||
		_spriteType == kLineTopBottomSprite ||
		_spriteType == kLineBottomTopSprite ||
		_spriteType == kOutlinedRectangleSprite ||
		_spriteType == kOutlinedRoundedRectangleSprite ||
		_spriteType == kOutlinedOvalSprite ||
		_spriteType == kThickLineSprite;
}

void Sprite::createQDMatte() {
	Graphics::ManagedSurface tmp;
	tmp.create(_width, _height, g_director->_pixelformat);
	tmp.clear(g_director->_wm->_colorWhite);

	Common::Rect srcRect(_width, _height);

	Common::Rect fillAreaRect((int)srcRect.width(), (int)srcRect.height());
	Graphics::MacPlotData plotFill(&tmp, nullptr, &g_director->getPatterns(), getPattern(), 0, 0, {1, 1}, g_director->_wm->_colorBlack);

	Graphics::Primitives &primitives = g_director->_wm->getDrawPrimitives();

	// it's the same for filled and outlined qd shape when we are using floodfill, so we use filled rect directly since it won't be affected by line size.
	switch (_spriteType) {
	case kOutlinedRectangleSprite:
	case kRectangleSprite:
		primitives.drawFilledRect1(fillAreaRect, g_director->_wm->_colorBlack, &plotFill);
		break;
	case kOutlinedRoundedRectangleSprite:
	case kRoundedRectangleSprite:
		primitives.drawRoundRect1(fillAreaRect, 12, g_director->_wm->_colorBlack, true, &plotFill);
		break;
	case kOutlinedOvalSprite:
	case kOvalSprite:
		primitives.drawEllipse(fillAreaRect.left, fillAreaRect.top, fillAreaRect.right, fillAreaRect.bottom, g_director->_wm->_colorBlack, true, &plotFill);
		break;
	case kLineBottomTopSprite:
	case kLineTopBottomSprite:
		warning("Sprite::createQDMatte doesn't support creating matte for type %d (%s)", _spriteType, spriteType2str(_spriteType));
		break;
	default:
		warning("Sprite::createQDMatte Expected shape type but got type %d (%s)", _spriteType, spriteType2str(_spriteType));
	}

	Graphics::Surface surface;
	surface.create(_width, _height, g_director->_pixelformat);
	surface.copyFrom(tmp);

	_matte = new Graphics::FloodFill(&surface, g_director->_wm->_colorWhite, 0, true);

	for (int yy = 0; yy < surface.h; yy++) {
		_matte->addSeed(0, yy);
		_matte->addSeed(surface.w - 1, yy);
	}

	for (int xx = 0; xx < surface.w; xx++) {
		_matte->addSeed(xx, 0);
		_matte->addSeed(xx, surface.h - 1);
	}

	_matte->fillMask();
	tmp.free();
	surface.free();
}

Graphics::Surface *Sprite::getQDMatte() {
	if (!isQDShape() || _ink != kInkTypeMatte)
		return nullptr;
	if (!_matte)
		createQDMatte();
	return _matte ? _matte->getMask() : nullptr;
}

MacShape *Sprite::getShape() {
	if (!isQDShape() && (_cast && _cast->_type != kCastShape))
		return nullptr;

	MacShape *shape = new MacShape();

	shape->ink = _ink;
	shape->spriteType = _spriteType;
	shape->foreColor = _foreColor;
	shape->backColor = _backColor;
	shape->lineSize = _thickness & kTThickness;
	shape->pattern = getPattern();
	shape->tile = nullptr;
	shape->tileRect = nullptr;

	if (shape->pattern > 56 && shape->pattern <= 64) {
		shape->tile = g_director->getTile(shape->pattern - 57);
		shape->tileRect = &g_director->getTileRect(shape->pattern - 57);
	}

	if (g_director->getVersion() >= 300 && shape->spriteType == kCastMemberSprite) {
		if (!_cast) {
			// Clearing a sprite from Lingo ("set the member of sprite n to 0") leaves
			// the channel typed as kCastMemberSprite with no member, and the channel
			// still counts as active (isEmpty() only tests kInactiveSprite), so it
			// reaches us every frame with nothing to draw. That is normal -- only a
			// sprite that names a member we then failed to resolve is worth reporting.
			if (!_castId.isNull())
				warning("Sprite::getShape(): sprite has no cast defined for %s", _castId.asString().c_str());
			delete shape;
			return nullptr;
		}

		ShapeCastMember *sc = (ShapeCastMember *)_cast;
		switch (sc->_shapeType) {
		case kShapeRectangle:
			shape->spriteType = sc->_fillType ? kRectangleSprite : kOutlinedRectangleSprite;
			break;
		case kShapeRoundRect:
			shape->spriteType = sc->_fillType ? kRoundedRectangleSprite : kOutlinedRoundedRectangleSprite;
			break;
		case kShapeOval:
			shape->spriteType = sc->_fillType ? kOvalSprite : kOutlinedOvalSprite;
			break;
		case kShapeLine:
			shape->spriteType = sc->_lineDirection == 6 ? kLineBottomTopSprite : kLineTopBottomSprite;
			break;
		default:
			break;
		}
	}

	// for outlined shapes, line thickness of 1 means invisible.
	shape->lineSize -= 1;

	return shape;
}

uint32 Sprite::getBackColor() {
	if (!_cast)
		return _backColor;

	switch (_cast->_type) {
	case kCastText:
	case kCastButton: {
		return _cast->getBackColor();
	}
	default:
		return _backColor;
	}
}

uint32 Sprite::getForeColor() {
	if (!_cast)
		return _foreColor;

	switch (_cast->_type) {
	case kCastText:
	case kCastButton: {
		return _cast->getForeColor();
	}
	default:
		return _foreColor;
	}
}

bool Sprite::getEditable() {
	if (!_cast)
		return _editable;

	return _editable || _cast->isEditable();
}

static bool scriptHandlesMouse(ScriptContext *script) {
	return script && (script->_eventHandlers.contains(kEventMouseDown)
				   || script->_eventHandlers.contains(kEventMouseUp)
				   || script->_eventHandlers.contains(kEventMouseWithin)
				   || script->_eventHandlers.contains(kEventMouseEnter)
				   || script->_eventHandlers.contains(kEventMouseLeave)
				   || script->_eventHandlers.contains(kEventMouseUpOutSide));
}

bool Sprite::respondsToMouse() {
	if (_moveable)
		return true;

	if (_cast && _cast->_type == kCastButton)
		return true;

	// A movie cast member is interactive only when its scripts are enabled:
	// its embedded movie may then have mouse handlers to route clicks into.
	if (_cast && _cast->_type == kCastMovie)
		return ((MovieCastMember *)_cast)->_enableScripts;

	// D6+ stores sprite scripts as behaviors. A sprite only responds to the mouse
	// if at least one of its behaviors actually defines a mouse handler. A behavior
	// with e.g. only an on beginSprite handler must NOT capture clicks: the hit test
	// would pick it, the event dispatcher (resolveScriptEvent) would then drop it for
	// lack of a handler, and the click would never fall through to a lower click-catcher
	// sprite (e.g. the playfield catcher in TKKG5's Sz140 box-pushing puzzle, whose
	// behavior holds the on mouseDown that drives movement). This mirrors the per-handler
	// gate the dispatcher already applies.
	if (g_director->getVersion() >= 600) {
		for (uint i = 0; i < _behaviors.size(); i++) {
			if (scriptHandlesMouse(_movie->getScriptContext(kScoreScript, _behaviors[i].memberID)))
				return true;
		}
	}

	ScriptContext *spriteScript = _movie->getScriptContext(kScoreScript, _scriptId);
	if (spriteScript && (spriteScript->_eventHandlers.contains(kEventGeneric)
					  || scriptHandlesMouse(spriteScript)))
		return true;

	ScriptContext *castScript = _movie->getScriptContext(kCastScript, _castId);
	if (scriptHandlesMouse(castScript))
		return true;

	return false;
}

bool Sprite::isActive() {
	if (_moveable)
		return true;

	if (_cast && _cast->_type == kCastButton)
		return true;

	// D6+ sprites carry their scripts as behaviors rather than via _scriptId.
	// respondsToMouse() already accounts for this; isActive() must too, otherwise
	// `the clickOn` (which uses getActiveSpriteIDFromPos -> isActive) returns 0 for
	// behavior-driven sprites even though they were clicked. TKKG2 relies on
	// `the clickOn` to identify the clicked character sprite for character changes.
	if (g_director->getVersion() >= 600 && _behaviors.size() > 0)
		return true;

	return (_movie->getScriptContext(kScoreScript, _scriptId) != nullptr)
		|| (_movie->getScriptContext(kCastScript, _castId) != nullptr);
}

bool Sprite::shouldHilite() {
	if (!isActive())
		return false;

	if (_moveable)
		return false;

	if (_puppet)
		return false;

	if (_cast) {
		// Restrict to bitmap cast members.
		// Buttons also hilite on click, but they have their own check.
		if (_cast->_type != kCastBitmap)
			return false;

		if (g_director->getVersion() >= 300) {
			// The Auto Hilite flag was introduced in D3.

			CastMemberInfo *castInfo = _cast->getInfo();
			if (castInfo)
				return castInfo->autoHilite;

			// If there's no cast info, fall back to the old matte check.
			// In D4 or above, there should always be a cast info,
			// but in D3, it is not present unless you set a cast member's
			// name, script, etc.
		}
	} else {
		// QuickDraw shapes may also hilite on click.
		if (!isQDShape())
			return false;
	}

	return _ink == kInkTypeMatte;
}

uint16 Sprite::getPattern() {
	if (!_cast) {
		if (isQDShape())
			return _pattern;
	} else if (_cast->_type == kCastShape) {
		return ((ShapeCastMember *)_cast)->_pattern;
	}

	return 0;
}

void Sprite::setPattern(uint16 pattern) {
	switch (_spriteType) {
	case kRectangleSprite:
	case kRoundedRectangleSprite:
	case kOvalSprite:
	case kLineTopBottomSprite:
	case kLineBottomTopSprite:
	case kOutlinedRectangleSprite:
	case kOutlinedRoundedRectangleSprite:
	case kOutlinedOvalSprite:
		_pattern = pattern;
		break;

	case kCastMemberSprite:
		// TODO
		warning("Sprite::setPattern(): kCastMemberSprite");
		return;

	default:
		return;
	}
}

void Sprite::setAutoPuppet(AutoPuppetProperty property, bool value) {
	// Skip this if we're not in D6 or above (auto-puppet is introduced in D6)
	if (_puppet || g_director->getVersion() < 600)
		return;

	if (value)
		_autoPuppet |= (1 << property);
	else
		_autoPuppet &= ~(1 << property);
}

bool Sprite::getAutoPuppet(AutoPuppetProperty property) {
	return (_autoPuppet & (1 << property)) != 0;
}

// Predicates mirror the auto-puppet checks in replaceFrom().
void Sprite::releaseAutoPuppet(uint32 copyBackMask) {
	static const struct { AutoPuppetProperty property; uint32 mask; } releases[] = {
		{ kAPInk,       kSCBInk },
		{ kAPForeColor, kSCBForeColor },
		{ kAPBackColor, kSCBBackColor },
		{ kAPCast,      kSCBCastId },
		{ kAPLoc,       kSCBStartPoint },
		{ kAPHeight,    kSCBCastId | kSCBHeight },
		{ kAPWidth,     kSCBCastId | kSCBWidth },
		{ kAPMoveable,  kSCBMoveable }
	};

	for (auto &release : releases)
		if (copyBackMask & release.mask)
			setAutoPuppet(release.property, false);
}

void Sprite::setWidth(int w) {
	_width = MAX<int>(w, 0);

	// Based on Director in a Nutshell, page 15
	setAutoPuppet(kAPWidth, true);
}

void Sprite::setHeight(int h) {
	_height = MAX<int>(h, 0);

	// Based on Director in a Nutshell, page 15
	setAutoPuppet(kAPHeight, true);
}

// D7 and later store a rotation and a skew per sprite (frame.cpp, sprite record
// fields 28 and 32) in hundredths of a degree. We cannot draw an arbitrary
// angle, but the half turns are exactly the axis flips we can already do:
// rotating by 180 degrees negates both axes, and a skew of 180 degrees turns the
// sprite's vertical axis around, negating only that one. Combining the two
// therefore mirrors horizontally.
//
// TKKG 8 builds the harbour captain that way: head, mouth and eyes all carry
// rotation and skew of 180 while the body carries neither, and every part hangs
// off the same registration point. Ignoring the fields left the face about 80
// pixels to the right of the body -- the mirrored parts landed on the wrong side
// of that shared anchor.
//
// Any other angle is left alone, as before. Reporting them would be noise: the
// same corpus has plenty of genuine 45, 60 and 330 degree rotations we still
// cannot draw.
static bool isHalfTurn(int32 hundredthsOfADegree) {
	if (!hundredthsOfADegree)
		return false;

	int32 angle = hundredthsOfADegree % 36000;
	if (angle < 0)
		angle += 36000;
	return angle == 18000;
}

// Flipping twice cancels out, so the score's own flip bits and the angles
// combine with exclusive or rather than stacking.
bool Sprite::isFlippedH() {
	// Only a rotation moves the horizontal axis.
	bool flip = (_thickness & kTFlipH) != 0;
	return flip != isHalfTurn(_angleRot);
}

bool Sprite::isFlippedV() {
	// A half turn flips the vertical axis as well, so a skewed half turn on top
	// of it cancels out and leaves a plain horizontal mirror.
	bool flip = (_thickness & kTFlipV) != 0;
	return flip != (isHalfTurn(_angleRot) != isHalfTurn(_angleSkew));
}

Common::Rect Sprite::getBbox(bool unstretched) {
	Common::Rect result(_width, _height);
	// If this is a cast member, use the cast member's getBbox function
	// so we start with a rect containing the correct registration offset.
	if (_cast)
		result = _cast->getBbox(_width, _height);

	if (_cast && _cast->_type == kCastBitmap) {
		if (isFlippedH())
			result.moveTo(-result.right, result.top);
		if (isFlippedV())
			result.moveTo(result.left, -result.bottom);
	}

	// The origin of the rect should be at the registration offset,
	// e.g. for bitmap sprites this defaults to the centre.
	// Now we move the rect to the correct spot.
	Common::Point startPos = _startPoint;
	result.translate(startPos.x, startPos.y);
	return result;

}

void Sprite::setBbox(int l, int t, int r, int b) {
	// A rect that is degenerate in only one axis is legitimate: Lingo uses it to
	// build a one-dimensional rail, e.g. for `the constraint of sprite`. Clamp each
	// axis on its own -- zeroing both would collapse such a rail down to its
	// top-left corner, pinning the constrained sprite there.
	_width = MAX<int>(0, r - l);
	_height = MAX<int>(0, b - t);

	Common::Rect source(_width, _height);
	if (_cast) {
		source = _cast->getBbox(_width, _height);
	}
	_startPoint.x = (int16)(l - source.left);
	_startPoint.y = (int16)(t - source.top);

	// Based on Director in a Nutshell, page 15
	setAutoPuppet(kAPBbox, true);
}

Common::Point Sprite::getPosition() {
	return _startPoint;
}

void Sprite::setPosition(int x, int y) {
	_startPoint = Common::Point(x, y);

	// Based on Director in a Nutshell, page 15
	setAutoPuppet(kAPLoc, true);
}

bool Sprite::checkSpriteType() {
	// check whether the sprite type match the cast type
	// if it doesn't match, then we treat it as transparent
	// this happens in warlock-mac data/stambul/c up
	if (_spriteType == kBitmapSprite && !(_cast->_type == kCastBitmap || _cast->_type == kCastFilmLoop)) {
		if (debugChannelSet(4, kDebugImages))
			warning("Sprite::checkSpriteType: Didn't render sprite due to the sprite type mismatch with cast type");
		return false;
	}
	return true;
}

void Sprite::setCast(CastMemberID memberID, bool replaceDims) {
	/**
	 * There are two things we need to take into account here:
	 *   1. The cast member's type
	 *   2. The sprite's type
	 * If the two types do not align, the sprite should not render.
	 *
	 * Before D4, you needed to manually set a sprite's type along
	 * with its castNum.
	 *
	 * Starting in D4, setting a sprite's castNum also set its type
	 * to an appropriate default.
	 */

	_castId = memberID;
	if (_cast) {
		_cast->decRefCount();
	}
	_cast = _movie->getCastMember(_castId);
	//As QDShapes don't have an associated cast, we must not change their _SpriteType.
	if (g_director->getVersion() >= 400 && !isQDShape() && _castId != CastMemberID(0, 0))
		_spriteType = kCastMemberSprite;

	if (_cast) {
		_cast->incRefCount();
		if (g_director->getVersion() >= 400) {
			// Set the sprite type to be more specific ONLY for bitmap or text.
			// Others just use the generic kCastMemberSprite in D4.
			switch (_cast->_type) {
			case kCastBitmap:
				_spriteType = kBitmapSprite;
				break;
			case kCastText:
				_spriteType = kTextSprite;
				break;
			case kCastButton:
				{
					TextCastMember *text = (TextCastMember *)_cast;
					switch (text->_buttonType) {
					case kTypeButton:
						_spriteType = kButtonSprite;
						break;
					case kTypeCheckBox:
						_spriteType = kCheckboxSprite;
						break;
					case kTypeRadio:
						_spriteType = kRadioButtonSprite;
						break;
					default:
						break;
					}
					break;
				}
			default:
				break;
			}
		}

		if (replaceDims) {
			Common::Rect dims = _cast->getInitialRect();
			switch (_cast->_type) {
			case kCastText:
				// A fixed or scrolling field is laid out at its member's rect:
				// TextCastMember::createWidget() raises the widget to it with
				// exactly this MAX. The sprite has to follow, or the widget is
				// drawn at one size and clipped to another.
				if (((TextCastMember *)_cast)->_textType == kTextTypeFixed ||
						((TextCastMember *)_cast)->_textType == kTextTypeScrolling) {
					_width = MAX<int>(_width, dims.width());
					_height = MAX<int>(_height, dims.height());
					break;
				}
				// fall through
			case kCastShape:
				// These normally keep the size the score gave them, but a
				// sprite with no extent can never draw, so there is nothing to
				// preserve: take the member's own rect. Same shape as the
				// digital-video case below, which keeps 0x0 because there it is
				// deliberate; a score that parks a channel on a 0x0 dummy until
				// Lingo fills it in means the opposite.
				if (_width <= 0 || _height <= 0) {
					_width = dims.width();
					_height = dims.height();
				}
				break;
			case kCastDigitalVideo:
				// A video the score sizes to 0x0 is intentionally invisible (a
				// hidden timing/audio clock); keep it, don't force native size.
				if (_width > 0 && _height > 0) {
					_width = dims.width();
					_height = dims.height();
				}
				break;
			case kCastBitmap:
			default:
				_width = dims.width();
				_height = dims.height();
				break;
			}

			// setWidth(), setHeight() and setBbox() all clamp to zero; this is the
			// one path that copies a member's rect through unchecked. TKKG 7's
			// photofit menu reads back `the rect of sprite 55` as
			// rect(73, 36, 73, -8180) right after Lingo wrote
			// rect(-56, -100, 56, 101) to it, and a height of -8216 sends the
			// dropped face element some 6800 pixels above the canvas. Name the
			// member that carries it rather than guessing.
			if (_width < 0 || _height < 0)
				warning("Sprite::setCast(): %s has a negative extent %dx%d", memberID.asString().c_str(), _width, _height);
		}

	} else {
		if (_castId.member != 0 && debugChannelSet(4, kDebugImages))
			warning("Sprite::setCast(): %s is null", memberID.asString().c_str());
	}
}


Common::String Sprite::formatInfo() {
	return Common::String::format("castId: %s, [inkData: 0x%02x [ink: %s, trails: %d, stretch: %d, line: %d], %dx%d@%d,%d type: %d (%s) fg: %08x bg: %08x], script: %s, colorcode: 0x%x, blendAmount: 0x%x, unk3: 0x%x, puppet: %d, moveable: %d",
		_castId.asString().c_str(), _inkData,
		inkType2str(_ink), _trails, _stretch, _thickness,
		_width, _height, _startPoint.x, _startPoint.y,
		_spriteType, spriteType2str(_spriteType), _foreColor, _backColor,
		_scriptId.asString().c_str(), _colorcode, _blendAmount, _unk3,
		_puppet, _moveable);
}

void Sprite::replaceFrom(Sprite *nextSprite) {
	if (!nextSprite)
		return;

	if (nextSprite->_copyBackMask & kSCBScriptId)
		_scriptId = nextSprite->_scriptId;
	// Copy all the behavior scripts
	_behaviors = nextSprite->_behaviors;
	_spriteInfo = nextSprite->_spriteInfo;
	if (nextSprite->_copyBackMask & kSCBSpriteListIdx)
		_spriteListIdx = nextSprite->_spriteListIdx;

	if (_puppet) {
		// Whole sprite is in puppet mode.
		// The only thing we want to copy over is the script ID.
		return;
	}

	// The editable flag is read from the score every frame (colorcode bit
	// 0x40), so it must follow the score even when the cast member does not
	// change: a field that the score turns editable on the same member has to
	// take effect (e.g. the login name fields in "Ein Fall fuer Muetze & Co",
	// which become editable on the same member when "new member" is clicked).
	// Lingo-controlled editability is handled separately via sprite puppeting,
	// which returns above before reaching this point.
	bool editable = nextSprite->_editable;

	bool immediate = _immediate;

	// Copy over all the sprite fields from one to another.
	// For D6+, exclude individual fields with autopuppet switched on
	if (nextSprite->_copyBackMask & kSCBSpriteType)
		_spriteType = nextSprite->_spriteType;

	if (nextSprite->_copyBackMask & kSCBEnabled)
		_enabled = nextSprite->_enabled;

	if (!getAutoPuppet(kAPInk) && (nextSprite->_copyBackMask & kSCBInk)) {
		_inkData = nextSprite->_inkData;
		_ink = nextSprite->_ink;
		_trails = nextSprite->_trails;
		_stretch = nextSprite->_stretch;
	}
	if (!getAutoPuppet(kAPForeColor) && (nextSprite->_copyBackMask & kSCBForeColor)) {
		_foreColor = nextSprite->_foreColor;
		_fgColorB = nextSprite->_fgColorB;
		_fgColorG = nextSprite->_fgColorG;
	}
	if (!getAutoPuppet(kAPBackColor) && (nextSprite->_copyBackMask & kSCBBackColor)) {
		_backColor = nextSprite->_backColor;
		_bgColorB = nextSprite->_bgColorB;
		_bgColorG = nextSprite->_bgColorG;
	}
	if (!getAutoPuppet(kAPCast)) {
		if (nextSprite->_copyBackMask & kSCBCastId) {
			_castId = nextSprite->_castId;
			if (_cast)
				_cast->decRefCount();
			_cast = nextSprite->_cast;
			if (_cast)
				_cast->incRefCount();
		}
		if (nextSprite->_copyBackMask & kSCBSpriteListIdx)
			_spriteListIdx = nextSprite->_spriteListIdx;
	}
	if (!getAutoPuppet(kAPLoc) && (nextSprite->_copyBackMask & kSCBStartPoint)) {
		_startPoint = nextSprite->_startPoint;
	}
	// height and width seem to be copied back if the cast ID changes (e.g. intro of sabotenman)
	if (!getAutoPuppet(kAPHeight) && ((nextSprite->_copyBackMask & kSCBCastId) || (nextSprite->_copyBackMask & kSCBHeight))) {
		_height = nextSprite->_height;
	}
	if (!getAutoPuppet(kAPWidth) && ((nextSprite->_copyBackMask & kSCBCastId) || (nextSprite->_copyBackMask & kSCBWidth))) {
		_width = nextSprite->_width;
	}
	if (!getAutoPuppet(kAPMoveable) && (nextSprite->_copyBackMask & kSCBMoveable)) {
		_colorcode = nextSprite->_colorcode;
		_editable = nextSprite->_editable;
		_moveable = nextSprite->_moveable;
	}
	if (nextSprite->_copyBackMask & kSCBBlendAmount)
		_blendAmount = nextSprite->_blendAmount;
	if (nextSprite->_copyBackMask & kSCBThickness)
		_thickness = nextSprite->_thickness;
	// Rotation and skew have no auto-puppet property of their own, so like the
	// thickness they simply follow the score. Without this the flips they encode
	// never reach the channel and the sprite draws unmirrored.
	if (nextSprite->_copyBackMask & kSCBAngle) {
		_angleRot = nextSprite->_angleRot;
		_angleSkew = nextSprite->_angleSkew;
	}
	if (nextSprite->_copyBackMask & kSCBPattern)
		_pattern = nextSprite->_pattern;

	// Persist the immediate flag
	_immediate = immediate;

	_editable = editable;

}


} // End of namespace Director
