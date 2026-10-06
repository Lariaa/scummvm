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

#include "common/events.h"
#include "common/stream.h"

#include "graphics/macgui/mactext.h"

#include "director/director.h"
#include "director/cast.h"
#include "director/channel.h"
#include "director/movie.h"
#include "director/score.h"
#include "director/sprite.h"
#include "director/castmember/castmember.h"
#include "director/castmember/script.h"
#include "director/lingo/lingo-object.h"
#include "director/lingo/lingo-the.h"
#include "director/util.h"

namespace Director {

void EditInfo::read(Common::ReadStreamEndian *stream) {
	rect = Movie::readRect(*stream);
	selStart = stream->readUint32();
	selEnd = stream->readUint32();
	version = stream->readByte();
	rulerFlag = stream->readByte();
	// We're ignoring 2 bytes here
	valid = true;
	debugC(3, kDebugLoading, "  EditInfo: rect: [%s],  selStart: %d,  selEnd: %d,  version: %d,  rulerFlag: %d", rect.toString().c_str(), selStart, selEnd, version, rulerFlag);
}

void EditInfo::write(Common::WriteStream *stream) {
	Movie::writeRect(stream, rect);
	stream->writeUint32BE(selStart);
	stream->writeUint32BE(selEnd);
	stream->writeByte(version);
	stream->writeByte(rulerFlag);
}

CastMember::CastMember(Cast *cast, uint16 castId, Common::SeekableReadStreamEndian &stream) : Object<CastMember>("CastMember") {
	_type = kCastTypeNull;
	_cast = cast;
	_castId = castId;
	_hilite = false;
	_purgePriority = 3;
	_size = stream.size();
	_flags1 = 0;

	_loaded = false;
	_modified = true;
	_isChanged = false;
	_needsReload = false;

	_objType = kCastMemberObj;

	_widget = nullptr;
	_erase = false;
	_index = -1;
}

CastMember::CastMember(Cast *cast, uint16 castId) : Object<CastMember>("CastMember") {
	_type = kCastTypeNull;
	_cast = cast;
	_castId = castId;
	_hilite = false;
	_purgePriority = 3;
	_size = 0;
	_flags1 = 0;

	_loaded = false;
	_modified = true;
	_isChanged = false;

	_objType = kCastMemberObj;

	_widget = nullptr;
	_erase = false;
	_index = -1;
}

CastMember *CastMember::duplicate(Cast *cast, uint16 castId) {
	warning("CastMember::duplicate(): unsupported cast type %s", castType2str(_type));
	return nullptr;
}

void CastMember::setModified(bool modified) {
	_modified = modified;
	if (modified)
		_isChanged = true;
}

Common::Rect CastMember::getBbox() {
	Common::Rect result(_initialRect);
	Common::Point offset = getRegistrationOffset();
	result.moveTo(-offset.x, -offset.y);
	return result;
}

Common::Rect CastMember::getBbox(int16 currentWidth, int16 currentHeight) {
	Common::Rect result(currentWidth, currentHeight);
	Common::Point offset = getRegistrationOffset(currentWidth, currentHeight);
	result.moveTo(-offset.x, -offset.y);
	return result;
}

int CastMember::getPageHeight() {
	// The manual is explicit that this is a screen measurement, not a member
	// one: "a field Cast member could actually be 224 pixels high, but if
	// resized on the Stage to 140 pixels, the pageHeight of member would return
	// 140" (D8 Demystified, Lingo Lexicon). So find the sprite showing it.
	Movie *movie = g_director->getCurrentMovie();
	if (movie) {
		Common::Array<Channel *> &channels = movie->getScore()->_channels;
		for (uint i = 0; i < channels.size(); i++) {
			if (channels[i]->_sprite && channels[i]->_sprite->_cast == this)
				return channels[i]->getBbox().height();
		}
	}

	// Nothing shows it, so all we know is how tall it was authored.
	return _cast->getCastMemberInitialRect(_castId).height();
}

Graphics::MacText *CastMember::getShownText() {
	Movie *movie = g_director->getCurrentMovie();
	if (!movie)
		return nullptr;

	Common::Array<Channel *> &channels = movie->getScore()->_channels;
	for (uint i = 0; i < channels.size(); i++) {
		if (channels[i]->_sprite && channels[i]->_sprite->_cast == this && channels[i]->_widget)
			return (Graphics::MacText *)channels[i]->_widget;
	}

	return nullptr;
}

bool CastMember::hasProp(const Common::String &propName) {
	Common::String fieldName = Common::String::format("%d%s", kTheCast, propName.c_str());
	return g_lingo->_theEntityFields.contains(fieldName) && hasField(g_lingo->_theEntityFields[fieldName]->field);
}

Datum CastMember::getProp(const Common::String &propName) {
	Common::String fieldName = Common::String::format("%d%s", kTheCast, propName.c_str());
	if (g_lingo->_theEntityFields.contains(fieldName)) {
		return getField(g_lingo->_theEntityFields[fieldName]->field);
	}

	warning("CastMember::getProp: unknown property '%s'", propName.c_str());
	return Datum();
}

void CastMember::setProp(const Common::String &propName, const Datum &value, bool force) {
	Common::String fieldName = Common::String::format("%d%s", kTheCast, propName.c_str());
	if (g_lingo->_theEntityFields.contains(fieldName)) {
		setField(g_lingo->_theEntityFields[fieldName]->field, value);
		return;
	}

	warning("CastMember::setProp: unknown property '%s'", propName.c_str());
}

bool CastMember::hasField(int field) {
	switch (field) {
	case kTheBackColor:
	case kTheCastLibNum:
	case kTheCastType:
	case kTheColor:
	case kTheFileName:
	case kTheForeColor:
	case kTheHeight:
	case kTheHilite:
	case kTheLoaded:
	case kTheMediaReady:
	case kTheModified:
	case kTheMemberNum:
	case kTheName:
	case kTheNumber:
	case kTheRect:
	case kThePreLoad:
	case kThePurgePriority:
	case kTheScriptText:
	case kTheSize:
	case kTheType:
	case kTheWidth:
		return true;
	default:
		break;
	}
	return false;
}

Datum CastMember::getField(int field) {
	Datum d;

	CastMemberInfo *castInfo = _cast->getCastMemberInfo(_castId);
	if (!castInfo)
		warning("CastMember::getField(): CastMember info for %d not found", _castId);

	switch (field) {
	case kTheBackColor:
		d = (int)getBackColor();
		break;
	case kTheCastLibNum:
		d = _cast->_castLibID;
		break;
	case kTheCastType:
	case kTheType:
		d = Common::String(castType2str(_type));
		if (g_director->getVersion() >= 500 && _type == kCastText) {
			// D5 changes this from "text" to "field"
			d = Common::String("field");
		}
		d.type = SYMBOL;
		break;
	case kTheFileName:
		if (castInfo)
			d = Datum(castInfo->directory + g_director->_dirSeparator + castInfo->fileName);
		break;
	case kTheColor:
		// The same foreground colour foreColor names, but as a colour object
		// rather than a palette index -- "independent of the monitor color
		// depth setting", as the D8 lexicon puts it for the sprite property.
		d = Datum(new ColorObject((int)getForeColor()));
		break;
	case kTheForeColor:
		d = (int)getForeColor();
		break;
	case kTheHeight:
		d = _cast->getCastMemberInitialRect(_castId).height();
		break;
	case kTheHilite:
		d = (int)_hilite;
		break;
	case kTheLoaded:
		d = 1; // Not loaded handled in Lingo::getTheCast
		break;
	case kTheMediaReady:
		d = 1;	// Media is always downloaded from internet in ScummVM
		break;
	case kTheModified:
		d = (int)_isChanged;
		break;
	case kTheName:
		if (castInfo)
			d = Datum(castInfo->name);
		break;
	case kTheMemberNum:
		d = _castId;
		break;
	case kTheNumber:
		if (g_director->getVersion() >= 500) {
			d = CastMemberID(_castId, _cast->_castLibID).toMultiplex();
		} else {
			d = _castId;
		}
		break;
	case kTheRect:
		// not sure get the initial rect would be fine to castmember
		d = Datum(_cast->getCastMember(_castId)->_initialRect);
		break;
	case kThePageHeight:
		d = getPageHeight();
		break;
	case kTheSelectionField:
		{
			// "a linear list with two elements: the number of the starting
			// character and the number of the ending character. If the starting
			// and ending numbers in the return value are identical, then there
			// are no characters selected" (D8 Demystified, Lingo Lexicon). The
			// two numbers are therefore caret positions, counted exactly as the
			// selStart and selEnd properties count -- the Lexicon says so itself
			// by sending fields to those two and text members here.
			Graphics::MacText *text = getShownText();

			d.type = ARRAY;
			d.u.farr = new FArray();
			d.u.farr->arr.push_back((int)(text ? text->getSelectionIndex(true) : 0));
			d.u.farr->arr.push_back((int)(text ? text->getSelectionIndex(false) : 0));
		}
		break;
	/*
	ScummVM does not do preloading so we will always return false here.

	simpsonscartoonstudio checks this flag and if set to true does an updateStage
	in a loop which causes performance issues loading cartoons.
	*/
	case kThePreLoad:
		d = 0;
		break;
	case kThePurgePriority:
		d = _purgePriority;
		break;
	case kTheScriptText:
		if (castInfo)
			d = Datum(castInfo->script);
		break;
	case kTheSize:
		d = (int)_size;
		break;
	case kTheWidth:
		d = _cast->getCastMemberInitialRect(_castId).width();
		break;
	default:
		warning("CastMember::getField(): Unprocessed getting field \"%s\" of cast %d", g_lingo->field2str(field), _castId);
	//TODO find out about String fields
	}

	return d;
}

void CastMember::setField(int field, const Datum &d) {
	CastMemberInfo *castInfo = _cast->getCastMemberInfo(_castId);

	switch (field) {
	case kTheBackColor:
		_cast->getCastMember(_castId)->setBackColor(d.asInt());
		return;
	case kTheCastType:
	case kTheType:
		warning("BUILDBOT: CastMember::setField(): Attempt to set read-only field %s of cast %d", g_lingo->entity2str(field), _castId);
		return;
	case kTheSelectionField:
		{
			if (d.type != ARRAY || d.u.farr->arr.size() < 2) {
				warning("CastMember::setField(): the selection of cast %d wants a list of two, got %s", _castId, d.type2str());
				return;
			}

			// The selection belongs to the widget, so it can only be set while
			// one is up. A behaviour that selects in beginSprite -- the form the
			// Lexicon itself shows, and the one Loewenzahn 7's high score list
			// uses -- runs before the widget exists, and then there is nothing
			// to select in yet.
			Graphics::MacText *text = getShownText();
			if (!text) {
				debugC(3, kDebugLingoExec, "CastMember::setField(): nothing shows cast %d, dropping the selection", _castId);
				return;
			}

			// Start before end, which is the order the manual asks for: "For
			// reliable results, the selStart property should be set before the
			// selEnd property."
			text->setSelection(d.u.farr->arr[0].asInt(), true);
			text->setSelection(d.u.farr->arr[1].asInt(), false);
		}
		return;
	case kTheFileName:
		if (!castInfo) {
			warning("CastMember::setField(): CastMember info for %d not found", _castId);
			return;
		} else {
			Common::String rawPath = d.asString();
			Common::String filename = getFileName(rawPath);
			castInfo->fileName = filename;
			castInfo->directory = rawPath.substr(0, MAX((uint)0, rawPath.size() - filename.size() - 1));
			_needsReload = true;
			setModified(true);
		}
		return;
	case kTheColor:
		// Takes a colour object, but accept a plain number too: the numeric
		// spelling is what foreColor uses and games mix the two.
		_cast->getCastMember(_castId)->setForeColor(
				(d.type == OBJECT && d.u.obj->getObjType() == kColorObj)
					? ((ColorObject *)d.u.obj)->toPaletteIndex()
					: d.asInt());
		return;
	case kTheForeColor:
		_cast->getCastMember(_castId)->setForeColor(d.asInt());
		return;
	case kTheHeight:
		warning("BUILDBOT: CastMember::setField(): Attempt to set read-only field \"%s\" of cast %d", g_lingo->field2str(field), _castId);
		return;
	case kTheHilite:
		_hilite = (bool)d.asInt();
		setModified(true);
		return;
	case kTheName:
		if (!castInfo) {
			warning("CastMember::setField(): CastMember info for %d not found", _castId);
			return;
		}
		castInfo->name = d.asString();
		setModified(true);
		_cast->rebuildCastNameCache();
		return;
	case kTheRect:
		warning("CastMember::setField(): Attempt to set read-only field \"%s\" of cast %d", g_lingo->field2str(field), _castId);
		return;
	/*
	ScummVM does not do preloading so we will make this a no-op.
	*/
	case kThePreLoad:
		return;
	case kThePurgePriority:
		_purgePriority = CLIP<int>(d.asInt(), 0, 3);
		return;
	case kTheScriptText:
		if (!castInfo) {
			warning("CastMember::setField(): CastMember info for %d not found", _castId);
			return;
		}
		{
			ScriptType scriptType = kCastScript;
			if (_type == kCastLingoScript)
				scriptType = static_cast<ScriptCastMember *>(this)->_scriptType;
			_cast->_lingoArchive->replaceCode(*d.u.s, scriptType, _castId);
		}
		castInfo->script = d.asString();
		setModified(true);
		return;
	case kTheWidth:
		warning("BUILDBOT: CastMember::setField(): Attempt to set read-only field \"%s\" of cast %d", g_lingo->field2str(field), _castId);
		return;
	default:
		warning("CastMember::setField(): Unprocessed setting field \"%s\" of cast %d", g_lingo->field2str(field), _castId);
	}
}

CastMemberInfo *CastMember::getInfo() {
	return _cast->getCastMemberInfo(_castId);
}

void CastMember::load() {
	if (_loaded)
		return;

	_loaded = true;
}

void CastMember::unload() {
	if (!_loaded)
		return;

	_loaded = false;
}

// Cast members have two types of "information", one is _data_ and the other is _info_
// _data_ is the actual "data" (e.g. the pixel image data of a bitmap, sound data of a sound cast member)
// Whereas _info_ is metadata (size, name, flags, etc.)
// Some cast members have their _data_ as well as _info_ in this very 'CASt' resource, e.g. TextCastMember
// Whereas some other have their _info_ in a 'CASt' resource and _data_ in a dedicated resource (e.g. PaletteCastMember has 'CLUT' resource)
uint32 CastMember::writeCAStResource(Common::SeekableWriteStream *writeStream) {
	uint32 castResourceSize = getCastResourceSize();

	writeStream->writeUint32LE(MKTAG('C', 'A', 'S', 't'));
	writeStream->writeUint32LE(castResourceSize);		// this is excluding the 'CASt' header and the size itself (- 8 bytes)

	uint32 castDataToWrite = getCastDataSize();
	uint32 castInfoToWrite = getCastInfoSize();

	if (_cast->_version >= kFileVer400 && _cast->_version < kFileVer500) {
		writeStream->writeUint16BE(castDataToWrite);
		writeStream->writeUint32BE(castInfoToWrite);
		writeStream->writeByte((uint8)_type);
		castDataToWrite -= 1;

		if (_flags1 != 0xFF) {					// In case of TextCastMember, this should be true
			writeStream->writeByte(_flags1);
			castDataToWrite -= 1;
		}

		// For cast members with dedicated resources for data, the castDataToWrite is zero
		if (castDataToWrite) {
			writeCastData(writeStream);
		}

		if (castInfoToWrite) {
			_cast->writeCastInfo(writeStream, _castId);
		}
	} else if (_cast->_version >= kFileVer500 && _cast->_version < kFileVer1200) {
		writeStream->writeUint32BE((uint32)_type);
		writeStream->writeUint32BE(castInfoToWrite);
		writeStream->writeUint32BE(castDataToWrite);

		if (castInfoToWrite) {
			_cast->writeCastInfo(writeStream, _castId);
		}

		// For cast members with dedicated resources for data, the castDataToWrite is zero
		if (castDataToWrite) {
			writeCastData(writeStream);
		}
	}
	return 0;
}

uint32 CastMember::getCastDataSize() {
	return 0;
}

void CastMember::writeCastData(Common::SeekableWriteStream *writeStream) {
	warning("CastMember::writeCastData(): no writer for %s cast member %d", castType2str(_type), _castId);
}

// This is the info that is inside the 'CASt' resource
uint32 CastMember::getCastInfoSize() {
	return _cast->getCastInfoSize(_castId);
}

// getCastResourceSize only returns the size of the resource without the header
uint32 CastMember::getCastResourceSize() {
	uint32 headerSize = 0;

	if (_cast->_version >= kFileVer400 && _cast->_version < kFileVer500) {
		// Header size for director version 4
		headerSize = 7;			// see Cast::loadCastData() for director version 4
		if (_flags1 != 0xFF) {
			headerSize += 1;
		}
	} else if (_cast->_version >= kFileVer500 && _cast->_version < kFileVer1200) {
		headerSize = 12;		// See Cast::loadCastData() for director version 5
	}

	return headerSize + getCastInfoSize() + getCastDataSize();
}

} // End of namespace Director
