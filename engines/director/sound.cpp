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

// SNDDecoder based on snd2wav by Abraham Macias Paredes
// https://github.com/System25/drxtract/blob/master/snd2wav
// License: GNU GPL v2 (see COPYING file for details)

#include "audio/audiostream.h"
#include "common/file.h"
#include "common/macresman.h"
#include "common/substream.h"
#include "common/xpfloat.h"

#include "audio/decoders/wave.h"
#include "audio/decoders/raw.h"
#include "audio/softsynth/pcspk.h"
#include "audio/decoders/aiff.h"
#include "audio/decoders/mp3.h"

#include "director/director.h"
#include "director/movie.h"
#include "director/sound.h"
#include "director/window.h"
#include "director/castmember/sound.h"

namespace Director {

DirectorSound::DirectorSound(Window *window) : _window(window) {
	uint numChannels = 2;
	if (g_director->getVersion() >= 300) {
		numChannels = 4;
	}

	for (uint i = 1; i <= numChannels; i++) {
		_channels[i] = new SoundChannel();
	}

	_mixer = g_system->getMixer();

	_speaker = new Audio::PCSpeaker();
	_speaker->init();

	_enable = true;
}

DirectorSound::~DirectorSound() {
	this->stopSound();
	unloadSampleSounds();
	delete _speaker;
	for (auto &it : _channels)
		delete it._value;
}

SoundChannel *DirectorSound::getChannel(int soundChannel) {
	if (!assertChannel(soundChannel))
		return nullptr;
	return _channels[soundChannel];
}

void DirectorSound::playFile(Common::String filename, int soundChannel, CastMemberID cuePointSource) {
	if (!assertChannel(soundChannel))
		return;

	if (debugChannelSet(-1, kDebugFast))
		return;

	AudioFileDecoder af(filename);
	Audio::AudioStream *sound = af.getAudioStream(false, false, DisposeAfterUse::YES);

	if (!sound) {
		debugC(1, kDebugSound, "DirectorSound::playFile(): channel %d, '%s' -> no audio stream decoded", soundChannel, filename.c_str());
		return;
	}

	cancelFade(soundChannel);
	stopSound(soundChannel);

	setChannelDefaultVolume(soundChannel);
	debugC(1, kDebugSound, "DirectorSound::playFile(): channel %d, '%s', enabled: %d, volume: %d (default %d)",
		soundChannel, filename.c_str(), _enable, getChannelVolume(soundChannel), g_director->_defaultVolume);
	_mixer->playStream(Audio::Mixer::kSFXSoundType, &_channels[soundChannel]->handle, sound, -1, getChannelVolume(soundChannel));
	_channels[soundChannel]->originalRate = (int)_mixer->getChannelRate(_channels[soundChannel]->handle);
	if (_channels[soundChannel]->pitchShiftPercent != 100) {
		_mixer->setChannelRate(_channels[soundChannel]->handle, _channels[soundChannel]->originalRate*_channels[soundChannel]->pitchShiftPercent/100);
	}

	// Set the last played sound so that cast member 0 in the sound channel doesn't stop this file.
	setLastPlayedSound(soundChannel, SoundID(), false);
	// A file is played by name, so the empty id above leaves nothing to look the
	// cue points up on. Remember the member that asked for it. The index starts
	// over too: the elapsed time this is measured against restarts here.
	_channels[soundChannel]->cuePointSource = cuePointSource;
	_channels[soundChannel]->lastCuePointIndex = -1;
	_channels[soundChannel]->fromLastMovie = false;
}

void DirectorSound::playMCI(Audio::AudioStream &stream, uint32 from, uint32 to) {
	Audio::SeekableAudioStream *seekStream = dynamic_cast<Audio::SeekableAudioStream *>(&stream);
	Audio::SubSeekableAudioStream *subSeekStream = new Audio::SubSeekableAudioStream(seekStream, Audio::Timestamp(from, seekStream->getRate()), Audio::Timestamp(to, seekStream->getRate()));

	// TODO: make sound enable settings work on this one
	_mixer->stopHandle(_scriptSound);
	_mixer->playStream(Audio::Mixer::kSFXSoundType, &_scriptSound, subSeekStream);
}

uint8 DirectorSound::getChannelVolume(int soundChannel) {
	if (!assertChannel(soundChannel))
		return 0;

	return _enable ? _channels[soundChannel]->volume : 0;
}

void DirectorSound::setChannelDefaultVolume(int soundChannel) {
	int vol = _volumes.getValOrDefault(soundChannel, g_director->_defaultVolume);

	_channels[soundChannel]->volume = vol;
}

void DirectorSound::setChannelPitchShift(int soundChannel, int pitchShiftPercent) {
	_channels[soundChannel]->pitchShiftPercent = pitchShiftPercent;
	if (isChannelActive(soundChannel)) {
		_mixer->setChannelRate(_channels[soundChannel]->handle, _channels[soundChannel]->originalRate*_channels[soundChannel]->pitchShiftPercent/100);
	}
}


void DirectorSound::playStream(Audio::AudioStream &stream, int soundChannel) {
	if (!assertChannel(soundChannel))
		return;

	cancelFade(soundChannel);

	_mixer->stopHandle(_channels[soundChannel]->handle);

	// Whatever played before has had its cue points, and the elapsed time the
	// next sound is measured against starts at zero again. Without this, a
	// channel kept every cue it had already passed and the second sound on it
	// skipped all of its own -- which is how TKKG 13 and 14 speak, line after
	// line through the same channel.
	_channels[soundChannel]->lastCuePointIndex = -1;
	// An embedded sound is looked up through lastPlayedSound, so drop whatever a
	// linked file left behind.
	_channels[soundChannel]->cuePointSource = CastMemberID();

	setChannelDefaultVolume(soundChannel);


	_mixer->playStream(Audio::Mixer::kSFXSoundType, &_channels[soundChannel]->handle, &stream, -1, getChannelVolume(soundChannel));
	_channels[soundChannel]->originalRate = (int)_mixer->getChannelRate(_channels[soundChannel]->handle);
	if (_channels[soundChannel]->pitchShiftPercent != 100) {
		_mixer->setChannelRate(_channels[soundChannel]->handle, _channels[soundChannel]->originalRate*_channels[soundChannel]->pitchShiftPercent/100);
	}
	_channels[soundChannel]->fromLastMovie = false;
}

void DirectorSound::playSound(SoundID soundID, int soundChannel, bool forPuppet) {
	switch (soundID.type) {
	case kSoundCast:
		playCastMember(CastMemberID(soundID.u.cast.member, soundID.u.cast.castLib), soundChannel, forPuppet);
		break;
	case kSoundExternal:
		playExternalSound(soundID.u.external.menu, soundID.u.external.submenu, soundChannel);
		break;
	}
}

void DirectorSound::playCastMember(CastMemberID memberID, int soundChannel, bool forPuppet) {
	if (!assertChannel(soundChannel))
		return;

	if (memberID.member == 0) {
		// Normally cast member 0 stops the sound.
		// But there are some sounds where it doesn't. Those are:
		//   1. playFile
		//   2. FPlay
		//   3. non-puppet looping sounds
		//   4. maybe more?
		if (shouldStopOnZero(soundChannel)) {
			stopSound(soundChannel);
			// Director 4 will stop after the current loop iteration, but
			// Director 3 will continue looping until the sound is replaced.
		} else if (g_director->getVersion() >= 400 && !_channels[soundChannel]->fromLastMovie) {
			// If there is a loopable stream specified, set the loop to expire by itself
			if (_channels[soundChannel]->loopPtr) {
				debugC(5, kDebugSound, "DirectorSound::playCastMember(): telling loop in channel %d to stop", soundChannel);
				_channels[soundChannel]->loopPtr->setRemainingIterations(1);
				_channels[soundChannel]->loopPtr = nullptr;
			}

			// Don't stop the currently playing sound, just set the last played sound to 0.
			setLastPlayedSound(soundChannel, SoundID(), false);
		}
	} else {
		CastMember *soundCast = _window->getCurrentMovie()->getCastMember(memberID);
		if (soundCast) {
			if (soundCast->_type != kCastSound) {
				warning("DirectorSound::playCastMember: attempted to play a non-SoundCastMember %s", memberID.asString().c_str());
			} else {
				bool looping = ((SoundCastMember *)soundCast)->_looping;
				bool stopOnZero = true;

				// For a non-puppet sound, if the sound is the same ID as the last
				// played, do nothing.
				if (!forPuppet && isLastPlayedSound(soundChannel, memberID))
					return;

				if (!forPuppet && looping) {
					// We know that this is a non-puppet, looping sound.
					// We don't want to stop it if this channel's cast member changes to 0.
					stopOnZero = false;
				}

				AudioDecoder *ad = ((SoundCastMember *)soundCast)->_audio;
				if (!ad) {
					warning("DirectorSound::playCastMember: no audio data attached to %s", memberID.asString().c_str());
					return;
				}

				Audio::AudioStream *as;
				as = ad->getAudioStream(looping, forPuppet);

				if (!as) {
					warning("DirectorSound::playCastMember: audio data failed to load from cast");
					return;
				}
				// For looping sounds, keep a copy of the AudioStream so it is
				// possible to gracefully stop the playback
				if (looping)
					_channels[soundChannel]->loopPtr = dynamic_cast<Audio::LoopableAudioStream *>(as);
				else
					_channels[soundChannel]->loopPtr = nullptr;
				playStream(*as, soundChannel);
				debugC(5, kDebugSound, "DirectorSound::playCastMember(): playing cast ID %s, channel %d, looping %d, stopOnZero %d, forPuppet %d, volume %d", memberID.asString().c_str(), soundChannel, looping, stopOnZero, forPuppet, _channels[soundChannel]->volume);
				setLastPlayedSound(soundChannel, memberID, stopOnZero);
			}
		} else {
			warning("DirectorSound::playCastMember: couldn't find %s", memberID.asString().c_str());
		}
	}
}

void DirectorSound::setSoundEnabled(bool enabled) {
	if (_enable == enabled)
		return;
	if (!enabled)
		stopSound();
	_enable = enabled;
}

void SNDDecoder::loadExternalSoundStream(Common::SeekableReadStreamEndian &stream) {
	_size = stream.readUint32BE();

	uint16 sampleRateFlag = stream.readUint16();
	/*uint16 unk2 = */ stream.readUint16();

	_data = (byte *)malloc(_size);
	stream.read(_data, _size);

	switch (sampleRateFlag) {
	case 1:
		_rate = 22254;
		break;
	case 2:
		_rate = 11127;
		break;
	case 3:
		_rate = 7300;
		break;
	case 4:
		_rate = 5500;
		break;
	default:
		warning("DirectorSound::loadExternalSoundStream: Can't handle sampleRateFlag %d, using default one", sampleRateFlag);
		_rate = 5500;
		break;
	}

	// this may related to the unk2 flag
	// TODO: figure out how to read audio flags
	_flags = Audio::FLAG_UNSIGNED;
	_channels = 1;
}

void DirectorSound::registerFade(int soundChannel, bool fadeIn, int ticks, bool autoStop) {
	if (!assertChannel(soundChannel))
		return;

	int startVol = fadeIn ? 0 :  _channels[soundChannel]->volume;
	int targetVol = fadeIn ? _channels[soundChannel]->volume : 0;
	registerFade(soundChannel, startVol, targetVol, ticks, autoStop);
}

void DirectorSound::registerFade(int soundChannel, int startVol, int targetVol, int ticks, bool autoStop) {
	if (!assertChannel(soundChannel))
		return;

	debugC(5, kDebugSound, "DirectorSound::registerFade(): registered fading channel %d over %d ticks, startVol: %d, targetVol: %d, autostop: %d", soundChannel, ticks, startVol, targetVol, autoStop);

	// sound enable is not working on fade sounds, so we just return directly when sounds are not enabling
	if (!_enable)
		return;

	cancelFade(soundChannel);

	_channels[soundChannel]->fade = new FadeParams(startVol, targetVol, ticks, _window->getVM()->getMacTicks(), targetVol > startVol, autoStop);
	_mixer->setChannelVolume(_channels[soundChannel]->handle, startVol);

	_channels[soundChannel]->volume = startVol;
}

bool DirectorSound::fadeChannels() {
	bool ongoing = false;

	for (auto &it : _channels) {
		FadeParams *fade = it._value->fade;
		if (!fade)
			continue;

		fade->lapsedTicks = _window->getVM()->getMacTicks() - fade->startTicks;
		if (fade->lapsedTicks > fade->totalTicks) {
			if (fade->persist) {
				// Arrived: the target is the channel's volume from now on.
				_mixer->setChannelVolume(it._value->handle, fade->targetVol);
				it._value->volume = fade->targetVol;
				_volumes[it._key] = fade->targetVol;
				delete fade;
				it._value->fade = nullptr;
				continue;
			}
			if (fade->autoStop)
				stopSound(it._key);
			continue;
		}

		int fadeVol = fadeVolume(fade);

		debugC(5, kDebugSound, "DirectorSound::fadeChannel(): fading channel %d volume to %d", it._key, fadeVol);
		_mixer->setChannelVolume(it._value->handle, fadeVol);

		it._value->volume = fadeVol;
		ongoing = true;
	}
	return ongoing;
}

// Where a fade stands after its lapsed ticks: a straight line from the start
// volume to the target. This used to take one end for silence, which is all the
// `sound fadeIn` and `sound fadeOut` commands ask for, but would have taken
// sound(1).fadeTo(128) from full volume all the way down to nothing.
int DirectorSound::fadeVolume(const FadeParams *fade) {
	if (fade->totalTicks <= 0 || fade->lapsedTicks >= fade->totalTicks)
		return fade->targetVol;

	int vol = fade->startVol + (fade->targetVol - fade->startVol) * fade->lapsedTicks / fade->totalTicks;
	return CLIP<int>(vol, 0, Audio::Mixer::kMaxChannelVolume);
}

void DirectorSound::cancelFade(int soundChannel) {
	if (!assertChannel(soundChannel))
		return;
	// NOTE: It is assumed that soundChannel has already been validated, which is
	// why this method is private.

	if (_channels[soundChannel]->fade) {
		FadeParams *fade = _channels[soundChannel]->fade;
		int restoreVol = fade->fadeIn ? fade->targetVol : fade->startVol;
		if (fade->persist) {
			// A sound channel object's fade stands for the channel's new volume.
			restoreVol = fade->targetVol;
			_channels[soundChannel]->volume = restoreVol;
			_volumes[soundChannel] = restoreVol;
		}
		debugC(5, kDebugSound, "DirectorSound::cancelFade(): resetting channel %d volume to %d", soundChannel, restoreVol);
		_mixer->setChannelVolume(_channels[soundChannel]->handle, restoreVol);

		delete fade;
		_channels[soundChannel]->fade = nullptr;
	}
}

uint32 DirectorSound::getChannelElapsedTime(int soundChannel) {
	if (!assertChannel(soundChannel))
		return 0;

	return _mixer->getSoundElapsedTime(_channels[soundChannel]->handle);
}

int DirectorSound::getChannelLastCuePoint(int soundChannel) {
	if (!assertChannel(soundChannel))
		return -1;

	return _channels[soundChannel]->lastCuePointIndex;
}

SoundID DirectorSound::getChannelLastPlayed(int soundChannel) {
	if (!assertChannel(soundChannel))
		return SoundID();

	return _channels[soundChannel]->lastPlayedSound;
}

bool DirectorSound::isChannelActive(int soundChannel) {
	if (!assertChannel(soundChannel))
		return false;

	if (!_mixer->isSoundHandleActive(_channels[soundChannel]->handle))
		return false;

	// Looped sounds are considered to be inactive after the first play
	// WORKAROUND HACK
	if (_channels[soundChannel]->loopPtr != nullptr)
		return _channels[soundChannel]->loopPtr->getCompleteIterations() < 1;

	return true;
}

bool DirectorSound::assertChannel(int soundChannel) {
	if (soundChannel <= 0) {
		warning("DirectorSound::assertChannel(): Invalid sound channel %d", soundChannel);
		return false;
	}
	if (!_channels.contains(soundChannel)) {
		debugC(5, kDebugSound, "DirectorSound::assertChannel(): allocating sound channel %d", soundChannel);
		_channels[soundChannel] = new SoundChannel();
	}
	return true;
}

void DirectorSound::loadSampleSounds(uint type) {
	if (type < kMinSampledMenu || type > kMaxSampledMenu) {
		warning("DirectorSound::loadSampleSounds: Invalid menu number %d", type);
		return;
	}

	if (!_sampleSounds[type - kMinSampledMenu].empty())
		return;

	// trying to load external sample sounds
	// lazy loading
	uint32 tag = MKTAG('C', 'S', 'N', 'D');
	uint id = 0xFF;
	Archive *archive = nullptr;

	for (auto &it : g_director->_allOpenResFiles) {
		if (!g_director->_allSeenResFiles.contains(it)) {
			warning("DirectorSound::loadSampleSounds(): file %s not found in allSeenResFiles, skipping", it.toString().c_str());
			break;
		}
		Common::Array<uint16> idList = g_director->_allSeenResFiles[it]->getResourceIDList(tag);
		for (uint j = 0; j < idList.size(); j++) {
			if (static_cast<uint>(idList[j] & 0xFF) == type) {
				id = idList[j];
				archive = g_director->_allSeenResFiles[it].get();
				break;
			}
		}
		if (id != 0xFF)
			break;
	}

	if (!archive) {
		warning("DirectorSound::loadSampleSounds(): could not find a valid archive");
		return;
	}

	if (id == 0xFF) {
		warning("Score::loadSampleSounds: can not find CSND resource with id %d", type);
		return;
	}

	Common::SeekableReadStreamEndian *csndData = archive->getResource(tag, id);

	/*uint32 flag = */ csndData->readUint32();

	// the flag should be 0x604E
	// i'm not sure what's that mean, but it occurs in those csnd files

	// contains how many csnd data
	uint16 num = csndData->readUint16();

	// read the offset first;
	Common::Array<uint32> offset(num);
	for (uint i = 0; i < num; i++)
		offset[i] = csndData->readUint32();

	for (uint i = 0; i < num; i++) {
		csndData->seek(offset[i]);

		SNDDecoder *ad = new SNDDecoder();
		ad->loadExternalSoundStream(*csndData);
		_sampleSounds[type - kMinSampledMenu].push_back(ad);
	}

	delete csndData;
}

void DirectorSound::unloadSampleSounds() {
	for (uint i = 0; i < kNumSampledMenus; i++) {
		for (uint j = 0; j < _sampleSounds[i].size(); j++) {
			delete _sampleSounds[i][j];
		}
		_sampleSounds[i].clear();
	}
}

void DirectorSound::playExternalSound(uint16 menu, uint16 submenu, int soundChannel) {
	if (!assertChannel(soundChannel))
		return;

	SoundID soundId(kSoundExternal, menu, submenu);
	// If the sound is the same ID ast the last played, do nothing.
	if (isLastPlayedSound(soundChannel, soundId))
		return;

	if (menu < kMinSampledMenu || menu > kMaxSampledMenu) {
		warning("DirectorSound::playExternalSound: Invalid menu number %d", menu);
		return;
	}

	Common::Array<AudioDecoder *> &menuSounds = _sampleSounds[menu - kMinSampledMenu];
	if (menuSounds.empty())
		loadSampleSounds(menu);

	if (1 <= submenu && submenu <= menuSounds.size()) {
		debugC(5, kDebugSound, "DirectorSound::playExternalSound(): playing menu ID %d, submenu ID %d, channel %d, volume %d", menu, submenu, soundChannel, _channels[soundChannel]->volume);
		playStream(*(menuSounds[submenu - 1]->getAudioStream()), soundChannel);
		setLastPlayedSound(soundChannel, soundId);
	} else {
		warning("DirectorSound::playExternalSound: Could not find sound %d %d", menu, submenu);
	}
}

void DirectorSound::changingMovie() {
	for (auto &it : _channels) {
		it._value->movieChanged = true;

		// When switching movies, the puppetSound flag is disabled. This means any sound (channel or script)
		// can take over. Any existing sound that is playing (sample or loop) will continue to play until that happens.
		if (isChannelPuppet(it._key)) {
			disablePuppetSound(it._key); // disable puppet sound
		}

		if (isChannelActive(it._key)) {
			// Don't stop this sound until there's a new, non-zero sound in this channel.
			it._value->stopOnZero = false;

			// If this is a looping sound, make it loop automatically until that happens.
			const SoundID &lastPlayedSound = it._value->lastPlayedSound;
			if (lastPlayedSound.type == kSoundCast) {
				CastMemberID memberID(lastPlayedSound.u.cast.member, lastPlayedSound.u.cast.castLib);
				CastMember *soundCast = _window->getCurrentMovie()->getCastMember(memberID);
				if (soundCast && soundCast->_type == kCastSound && static_cast<SoundCastMember *>(soundCast)->_looping) {
					_mixer->loopChannel(it._value->handle);
				}
			}

			// Flag that this is from a previous movie, therefore looping should continue.
			it._value->fromLastMovie = true;
		}
	}
	unloadSampleSounds(); // TODO: we can possibly keep this between movies
}

void DirectorSound::setLastPlayedSound(int soundChannel, SoundID soundId, bool stopOnZero) {
	_channels[soundChannel]->lastPlayedSound = soundId;
	_channels[soundChannel]->stopOnZero = stopOnZero;
	_channels[soundChannel]->movieChanged = false;
}

bool DirectorSound::isLastPlayedSound(int soundChannel, const SoundID &soundId) {
	return !_channels[soundChannel]->movieChanged && _channels[soundChannel]->lastPlayedSound == soundId;
}

bool DirectorSound::shouldStopOnZero(int soundChannel) {
	return _channels[soundChannel]->stopOnZero;
}

void DirectorSound::stopSound(int soundChannel) {
	if (!assertChannel(soundChannel))
		return;

	debugC(5, kDebugSound, "DirectorSound::stopSound(): stopping channel %d", soundChannel);
	if (_channels[soundChannel]->loopPtr)
		_channels[soundChannel]->loopPtr = nullptr;
	cancelFade(soundChannel);
	_mixer->stopHandle(_channels[soundChannel]->handle);
	setLastPlayedSound(soundChannel, SoundID());
	_channels[soundChannel]->fromLastMovie = false;
	return;
}

void DirectorSound::stopSound() {
	debugC(5, kDebugSound, "DirectorSound::stopSound(): stopping all channels");
	for (auto &it : _channels) {
		if (!it._value)
			continue;

		if (it._value->loopPtr)
			it._value->loopPtr = nullptr;
		cancelFade(it._key);

		_mixer->stopHandle(it._value->handle);
		setLastPlayedSound(it._key, SoundID());
		it._value->fromLastMovie = false;
	}

	_mixer->stopHandle(_scriptSound);
	_speaker->quit();
}

void DirectorSound::systemBeep() {
	debugC(5, kDebugSound, "DirectorSound::systemBeep(): beep!");
	_speaker->play(Audio::PCSpeaker::kWaveFormSquare, 500, 150);
}

bool DirectorSound::isChannelPuppet(int soundChannel) {
	if (!assertChannel(soundChannel))
		return false;

	// cast member ID 0 means "not a puppet"
	if (_channels[soundChannel]->puppet.type == kSoundCast && _channels[soundChannel]->puppet.u.cast.member == 0)
		return false;

	return true;
}

void DirectorSound::setPuppetSound(SoundID soundId, int soundChannel) {
	if (!assertChannel(soundChannel))
		return;

	if (soundId.isZero()) {
		// If soundId is zero, kill the current sound and clear the puppet flag.
		stopSound(soundChannel);
		disablePuppetSound(soundChannel);
	} else {
		// soundId is non-zero, set the puppet sound value to that.
		_channels[soundChannel]->newPuppet = true;
		_channels[soundChannel]->puppet = soundId;
		_channels[soundChannel]->stopOnZero = true;
	}
}

void DirectorSound::disablePuppetSound(int soundChannel) {
	if (!assertChannel(soundChannel))
		return;

	_channels[soundChannel]->puppet = SoundID();
}

void DirectorSound::playPuppetSound(int soundChannel) {
	if (!assertChannel(soundChannel))
		return;

	// only play if the puppet was just set
	if (!_channels[soundChannel]->newPuppet)
		return;

	debugC(5, kDebugSound, "DirectorSound::playPuppetSound(): playing on channel %d", soundChannel);

	_channels[soundChannel]->newPuppet = false;
	playSound(_channels[soundChannel]->puppet, soundChannel, true);
}

void DirectorSound::queueSound(int soundChannel, const SoundQueueEntry &entry) {
	if (!assertChannel(soundChannel))
		return;

	_channels[soundChannel]->playList.push_back(entry);
}

void DirectorSound::setPlayList(int soundChannel, const Common::Array<SoundQueueEntry> &playList) {
	if (!assertChannel(soundChannel))
		return;

	// Replaces what is queued; the sound playing now carries on.
	_channels[soundChannel]->playList = playList;
}

Common::Array<SoundQueueEntry> DirectorSound::getPlayList(int soundChannel) {
	if (!assertChannel(soundChannel))
		return Common::Array<SoundQueueEntry>();

	return _channels[soundChannel]->playList;
}

bool DirectorSound::isQueuePaused(int soundChannel) {
	if (!assertChannel(soundChannel))
		return false;

	return _channels[soundChannel]->paused;
}

void DirectorSound::startQueueEntry(int soundChannel, const SoundQueueEntry &entry) {
	SoundChannel *channel = _channels[soundChannel];
	channel->current = entry;
	channel->loopsRemaining = entry.loopCount;
	channel->paused = false;

	// A fade started before play() -- queue(), fadeIn(), play() is the order the
	// Director 8.5 reference gives -- belongs to the channel, not to the sound, so
	// it has to outlive playStream() cancelling fades and resetting the volume.
	FadeParams *fade = channel->fade;
	channel->fade = nullptr;

	// Lingo owns the channel while its list plays, so the score's sound channels
	// leave it alone, as they do for puppetSound.
	setPuppetSound(SoundID(entry.member), soundChannel);

	if (!entry.linkedPath.empty()) {
		// The entry knows which file the member pointed at when it was queued.
		// Reading the member again here would give whatever Lingo has aimed it at
		// since, and TKKG 13 and 14 re-aim the same two placeholders for every
		// line they speak: one take was heard twice and the next not at all.
		_channels[soundChannel]->newPuppet = false;
		playFile(entry.linkedPath, soundChannel, entry.member);
	} else {
		playPuppetSound(soundChannel);
	}

	if (fade) {
		channel->fade = fade;
		fade->lapsedTicks = _window->getVM()->getMacTicks() - fade->startTicks;
		int vol = fadeVolume(fade);
		_mixer->setChannelVolume(channel->handle, vol);
		channel->volume = vol;
	}

	if (!isChannelActive(soundChannel)) {
		// Not a playable sound: move on rather than retry it every frame.
		warning("DirectorSound::startQueueEntry(): %s did not start in channel %d", entry.member.asString().c_str(), soundChannel);
		channel->current = SoundQueueEntry();
		channel->loopsRemaining = 0;
	}
}

void DirectorSound::playQueue(int soundChannel) {
	if (!assertChannel(soundChannel))
		return;

	SoundChannel *channel = _channels[soundChannel];
	if (channel->paused) {
		_mixer->pauseHandle(channel->handle, false);
		channel->paused = false;
		return;
	}

	channel->playListActive = true;
	if (!isChannelActive(soundChannel) && !channel->playList.empty())
		startQueueEntry(soundChannel, channel->playList.remove_at(0));
}

void DirectorSound::playNow(int soundChannel, const SoundQueueEntry &entry) {
	if (!assertChannel(soundChannel))
		return;

	// play(member) puts the sound ahead of anything queued and starts it at once.
	_channels[soundChannel]->playListActive = true;
	startQueueEntry(soundChannel, entry);
}

void DirectorSound::playNext(int soundChannel) {
	if (!assertChannel(soundChannel))
		return;

	SoundChannel *channel = _channels[soundChannel];
	if (channel->playList.empty()) {
		stopQueue(soundChannel);
		return;
	}

	channel->playListActive = true;
	startQueueEntry(soundChannel, channel->playList.remove_at(0));
}

void DirectorSound::stopQueue(int soundChannel) {
	if (!assertChannel(soundChannel))
		return;

	// Stops the sound playing now. What is queued stays queued for the next
	// play(); setPlayList([]) is what empties the queue (Director MX manual).
	SoundChannel *channel = _channels[soundChannel];
	channel->playListActive = false;
	channel->paused = false;
	channel->current = SoundQueueEntry();
	channel->loopsRemaining = 0;
	stopSound(soundChannel);
	disablePuppetSound(soundChannel);
}

void DirectorSound::pauseQueue(int soundChannel) {
	if (!assertChannel(soundChannel) || !isChannelActive(soundChannel))
		return;

	_mixer->pauseHandle(_channels[soundChannel]->handle, true);
	_channels[soundChannel]->paused = true;
}

void DirectorSound::rewindQueue(int soundChannel) {
	if (!assertChannel(soundChannel))
		return;

	SoundChannel *channel = _channels[soundChannel];
	if (channel->current.member.member == 0)
		return;

	// Back to the start of the sound playing now, keeping its remaining loops.
	SoundQueueEntry entry = channel->current;
	int loops = channel->loopsRemaining;
	startQueueEntry(soundChannel, entry);
	if (channel->current.member.member != 0)
		channel->loopsRemaining = loops;
}

void DirectorSound::breakLoop(int soundChannel) {
	if (!assertChannel(soundChannel))
		return;

	// The pass playing now is the last one; then the list moves on.
	SoundChannel *channel = _channels[soundChannel];
	channel->loopsRemaining = 1;
	if (channel->loopPtr)
		channel->loopPtr->setRemainingIterations(1);
}

void DirectorSound::registerPersistentFade(int soundChannel, int startVol, int targetVol, int ticks) {
	registerFade(soundChannel, startVol, targetVol, MAX(ticks, 1));
	if (_channels[soundChannel]->fade)
		_channels[soundChannel]->fade->persist = true;
}

void DirectorSound::fadeChannelTo(int soundChannel, int targetVol, int ticks) {
	if (!assertChannel(soundChannel))
		return;

	registerPersistentFade(soundChannel, _channels[soundChannel]->volume, targetVol, ticks);
}

void DirectorSound::fadeChannelIn(int soundChannel, int ticks) {
	if (!assertChannel(soundChannel))
		return;

	// Silent at once, then back up to the channel's volume.
	int target = _volumes.getValOrDefault(soundChannel, g_director->_defaultVolume);
	registerPersistentFade(soundChannel, 0, target, ticks);
}

void DirectorSound::updatePlayLists() {
	for (auto &it : _channels) {
		SoundChannel *channel = it._value;
		if (!channel || !channel->playListActive || channel->paused || isChannelActive(it._key))
			continue;

		// A pass has just ended. Play it again while loops remain -- a loopCount of
		// 0 repeats until breakLoop() -- then carry on down the list.
		if (channel->current.member.member != 0 && channel->loopsRemaining != 1) {
			int loops = channel->loopsRemaining > 1 ? channel->loopsRemaining - 1 : 0;
			SoundQueueEntry entry = channel->current;
			startQueueEntry(it._key, entry);
			if (channel->current.member.member != 0)
				channel->loopsRemaining = loops;
			continue;
		}

		if (!channel->playList.empty()) {
			startQueueEntry(it._key, channel->playList.remove_at(0));
			continue;
		}

		channel->playListActive = false;
		channel->current = SoundQueueEntry();
		disablePuppetSound(it._key);
	}
}

void DirectorSound::playFPlaySound() {
	if (_fplayQueue.empty())
		return;
	// only when the previous sound is finished, shall we play next one
	if (isChannelActive(1))
		return;

	Common::String sndName = _fplayQueue.pop();
	if (sndName.equalsIgnoreCase("stop")) {
		stopSound(1);
		_currentSoundName = "";

		if (_fplayQueue.empty())
			return;
		else
			sndName = _fplayQueue.pop();
	}

	uint32 tag = MKTAG('s', 'n', 'd', ' ');
	uint id = 0xFFFF;
	Archive *archive = nullptr;

	// iterate opened ResFiles
	for (auto &it : g_director->_allOpenResFiles) {
		id = g_director->_allSeenResFiles[it]->findResourceID(tag, sndName, true);
		if (id != 0xFFFF) {
			archive = g_director->_allSeenResFiles[it].get();
			break;
		}
	}

	if (id == 0xFFFF) {
		warning("DirectorSound:playFPlaySound: can not find sound %s", sndName.c_str());
		return;
	}

	Common::SeekableReadStreamEndian *sndData = archive->getResource(tag, id);
	if (sndData != nullptr) {
		SNDDecoder ad;
		ad.loadStream(*sndData);
		delete sndData;

		Audio::AudioStream *as;
		bool looping = false;

		if (!_fplayQueue.empty() && _fplayQueue.front().equalsIgnoreCase("continuous")) {
			_fplayQueue.pop();
			looping = true;
		}

		// FPlay is controlled by Lingo, not the score, like a puppet,
		// so we'll get the puppet version of the stream.
		as = ad.getAudioStream(looping, true);

		if (!as) {
			warning("DirectorSound:playFPlaySound: failed to get audio stream");
			return;
		}

		// update current playing sound
		_currentSoundName = sndName;

		playStream(*as, 1);
	}

	// Set the last played sound so that cast member 0 in the sound channel doesn't stop this file.
	setLastPlayedSound(1, SoundID(), false);
}

void DirectorSound::playFPlaySound(const Common::Array<Common::String> &fplayList) {
	for (uint i = 0; i < fplayList.size(); i++)
		_fplayQueue.push(fplayList[i]);

	// stop the previous sound, because new one is coming
	if (isChannelActive(1))
		stopSound(1);

	playFPlaySound();
}

void DirectorSound::setChannelVolumeInternal(int soundChannel, uint8 volume) {
	if (!(_channels[soundChannel]) || volume == _channels[soundChannel]->volume)
		return;

	cancelFade(soundChannel);

	_channels[soundChannel]->volume = volume;
	_volumes[soundChannel] = volume;

	if (_enable)
		_mixer->setChannelVolume(_channels[soundChannel]->handle, _channels[soundChannel]->volume);
}

// -1 represent all the sound channel
void DirectorSound::setChannelVolume(int channel, uint8 volume) {
	if (channel != -1) {
		if (!assertChannel(channel))
			return;
		debugC(5, kDebugSound, "DirectorSound::setChannelVolume: setting channel %d to volume %d", channel, volume);
		setChannelVolumeInternal(channel, volume);
	} else {
		debugC(5, kDebugSound, "DirectorSound::setChannelVolume: setting all channels to volume %d", volume);
			for (auto &it : _channels)
				setChannelVolumeInternal(it._key, volume);
	}
}

void DirectorSound::setChannelBalance(int channel, int8 balance) {
	if (!assertChannel(channel))
		return;
	_mixer->setChannelBalance(_channels[channel]->handle, balance);
}

int8 DirectorSound::getChannelBalance(int channel) {
	if (!assertChannel(channel))
		return 0;
	return _mixer->getChannelBalance(_channels[channel]->handle);
}

void DirectorSound::setChannelFaderL(int channel, uint8 faderL) {
	if (!assertChannel(channel))
		return;
	_mixer->setChannelFaderL(_channels[channel]->handle, faderL);
}

uint8 DirectorSound::getChannelFaderL(int channel) {
	if (!assertChannel(channel))
		return 0;
	return _mixer->getChannelFaderL(_channels[channel]->handle);
}

void DirectorSound::setChannelFaderR(int channel, uint8 faderR) {
	if (!assertChannel(channel))
		return;
	_mixer->setChannelFaderR(_channels[channel]->handle, faderR);
}

uint8 DirectorSound::getChannelFaderR(int channel) {
	if (!assertChannel(channel))
		return 0;
	return _mixer->getChannelFaderR(_channels[channel]->handle);
}

void DirectorSound::processCuePoints() {
	for (auto &it : _channels) {
		SoundChannel *channel = it._value;

		// A linked file names the member it was played for; everything else is
		// found through the sound that last started on the channel.
		CastMemberID memberID = channel->cuePointSource;
		if (memberID.member == 0) {
			const SoundID &lastPlayedSound = channel->lastPlayedSound;
			if (lastPlayedSound.type != kSoundCast)
				continue;
			memberID = CastMemberID(lastPlayedSound.u.cast.member, lastPlayedSound.u.cast.castLib);
		}

		CastMember *member = _window->getCurrentMovie()->getCastMember(memberID);

		// The sound channel can reference a cast member that is not (or no
		// longer) a sound -- e.g. when the member ID resolves to a bitmap.
		// Guard the downcast; otherwise we read _cuePoints off an unrelated
		// object (here a BitmapCastMember) and crash in a release build.
		if (!member || member->_type != kCastSound)
			continue;

		SoundCastMember *soundCast = (SoundCastMember *)member;

		if (soundCast->_cuePoints.empty())
			continue;

		uint32 elapsedTime = _mixer->getSoundElapsedTime(channel->handle);

		if (!elapsedTime)
			continue;

		for (uint i = channel->lastCuePointIndex + 1; i < soundCast->_cuePoints.size(); i++) {
			int32 cuePoint = soundCast->_cuePoints[i];

			if (cuePoint > (int32)elapsedTime)
				break;

			Common::String cueName = i < soundCast->_cuePointNames.size() ? soundCast->_cuePointNames[i] : Common::String();

			debugC(3, kDebugSound, "DirectorSound::processCuePoints(): cue point %d ('%s') reached on channel %d",
					cuePoint, cueName.c_str(), it._key);

			// "Notice that the channel is reported as sound2, which
			// differentiates it from the sprite channels" (Director 8
			// Demystified). The cue point number is the one Lingo counts, so
			// one-based, as cuePointNames and cuePointTimes are.
			Datum channelName(Common::String::format("sound%d", it._key));
			channelName.type = SYMBOL;
			g_lingo->_cuePointChannel = channelName;
			g_lingo->_cuePointNumber = i + 1;
			g_lingo->_cuePointName = cueName;

			_window->getCurrentMovie()->processEvent(kEventCuePassed, i);

			channel->lastCuePointIndex = i;
		}
	}
}


SNDDecoder::SNDDecoder()
		: AudioDecoder() {
	_data = nullptr;
	_channels = 0;
	_size = 0;
	_rate = 0;
	_bits = 0;
	_flags = 0;
	_loopStart = _loopEnd = 0;
}

SNDDecoder::~SNDDecoder() {
	if (_data) {
		free(_data);
	}
}

bool SNDDecoder::loadStream(Common::SeekableReadStreamEndian &stream) {
	if (_data) {
		free(_data);
		_data = nullptr;
	}

	if (debugChannelSet(5, kDebugLoading)) {
		debugC(5, kDebugLoading, "snd header:");
		stream.hexdump(0x4e);
	}

	uint16 format = stream.readUint16();
	if (format == 1) {
		uint16 dataTypeCount = stream.readUint16();
		for (uint16 i = 0; i < dataTypeCount; i++) {
			uint16 dataType = stream.readUint16();
			if (dataType == 5) {
				// Sampled sound data
				uint32 options = stream.readUint32();
				_channels = (options & 0x80) ? 1 : 2;
				if (!processCommands(stream))
					return false;
			} else {
				warning("SNDDecoder: Unsupported data type: %d", dataType);
				return false;
			}
		}
	} else if (format == 2) {
		_channels = 1;
		/*uint16 refCount =*/stream.readUint16();
		if (!processCommands(stream))
			return false;
	} else {
		warning("SNDDecoder: Bad format: %d", format);
		return false;
	}

	return true;
}

bool SNDDecoder::processCommands(Common::SeekableReadStreamEndian &stream) {
	uint16 cmdCount = stream.readUint16();
	for (uint16 i = 0; i < cmdCount; i++) {
		uint16 cmd = stream.readUint16();
		if (cmd == 0x8050 || cmd == 0x8051) {
			if (!processBufferCommand(stream))
				return false;
		} else {
			warning("SNDDecoder: Unsupported command: %d", cmd);
			return false;
		}
	}

	return true;
}

bool SNDDecoder::processBufferCommand(Common::SeekableReadStreamEndian &stream) {
	if (_data) {
		warning("SNDDecoder: Already read data");
		return false;
	}

	/*uint16 unk1 =*/stream.readUint16();
	int32 offset = stream.readUint32();
	if (offset != stream.pos()) {
		warning("SNDDecoder: Bad sound header offset. Expected: %d, read: %d", (int)stream.pos(), offset);
		return false;
	}
	/*uint32 dataPtr =*/stream.readUint32();
	uint32 param = stream.readUint32();
	_rate = stream.readUint16();
	/*uint16 rateExt =*/stream.readUint16();
	_loopStart = stream.readUint32();
	_loopEnd = stream.readUint32();
	byte encoding = stream.readByte();
	byte baseFrequency = stream.readByte();
	if (baseFrequency != 0x3c) {
		warning("SNDDecoder: Unsupported base frequency: %d", baseFrequency);
		return false;
	}
	uint32 frameCount = 0;
	_bits = 8;
	if (encoding == 0x00) {
		// Standard sound header
		frameCount = param / _channels;
	} else if (encoding == 0xff) {
		// Extended sound header
		_channels = param;
		frameCount = stream.readUint32();
		for (uint32 i = 0; i < 0x0a; i++) {
			// aiff sample rate
			stream.readByte();
		}
		/*uint32 markerChunk =*/stream.readUint32();
		/*uint32 instrumentsChunk =*/stream.readUint32();
		/*uint32 aesRecording =*/stream.readUint32();
		_bits = stream.readUint16();

		// future use
		stream.readUint16();
		stream.readUint32();
		stream.readUint32();
		stream.readUint32();
	} else if (encoding == 0xfe) {
		// Compressed sound header
		warning("SNDDecoder: Compressed sound header not supported");
		return false;
	} else {
		warning("SNDDecoder: Bad encoding: %d", encoding);
		return false;
	}

	_flags = 0;
	_flags |= (_channels == 2) ? Audio::FLAG_STEREO : 0;
	_flags |= (_bits == 16) ? Audio::FLAG_16BITS : 0;
	_flags |= (_bits == 8) ? Audio::FLAG_UNSIGNED : 0;
	_size = frameCount * _channels * (_bits == 16 ? 2 : 1);

	_data = (byte *)malloc(_size);
	assert(_data);
	stream.read(_data, _size);

	return true;
}

Audio::AudioStream *SNDDecoder::getAudioStream(bool looping, bool forPuppet, DisposeAfterUse::Flag disposeAfterUse) {
	if (!_data)
		return nullptr;
	byte *buffer = (byte *)malloc(_size);
	memcpy(buffer, _data, _size);

	Audio::SeekableAudioStream *stream = Audio::makeRawStream(buffer, _size, _rate, _flags, disposeAfterUse);

	if (looping) {
		if (hasLoopBounds()) {
			// FIXME: determine the correct behaviour for non-consecutive loop bounds
			if (_loopEnd <= _loopStart) {
				warning("SNDDecoder::getAudioStream: Looping sound has non-consecutive bounds, using entire sample");
				return new Audio::LoopingAudioStream(stream, 0);
			} else {
				// Return an automatically looping stream.
				debugC(5, kDebugSound, "DirectorSound::getAudioStream(): returning a loop at positions start: %i, end: %i", _loopStart, _loopEnd);
				return new Audio::SubLoopingAudioStream(stream, 0, Audio::Timestamp(0, _loopStart, _rate), Audio::Timestamp(0, _loopEnd, _rate));
			}
		} else {
			// Not sure if looping sounds can appear without loop bounds.
			// Let's just log a warning and loop the entire sound...
			warning("SNDDecoder::getAudioStream: Looping sound has no loop bounds");
			return new Audio::LoopingAudioStream(stream, 0);
		}
	}

	return stream;
}

uint32 SNDDecoder::getDuration() {
	if (!_data || !_rate || !_channels || !_bits)
		return 0;

	uint32 frameSize = _channels * (_bits / 8);
	if (!frameSize)
		return 0;

	return (uint32)((uint64)(_size / frameSize) * 1000 / _rate);
}

bool SNDDecoder::hasLoopBounds() {
	return _loopStart != 0 || _loopEnd != 0;
}

bool SNDDecoder::hasValidLoopBounds() {
	return hasLoopBounds() && _loopStart < _loopEnd && _loopEnd <= _size;
}

void SNDDecoder::resetLoopBounds() {
	_loopStart = _loopEnd = 0;
}

AudioFileDecoder::AudioFileDecoder(Common::String &path)
		: AudioDecoder() {
	_path = path;
}

AudioFileDecoder::~AudioFileDecoder() {
}

/**
 * Locate the first MPEG frame in a Shockwave Audio (".swa") file.
 *
 * A .swa is a Macromedia header followed by plain MPEG Layer III: a big-endian
 * uint32 holding the header length, then the header itself, which carries the
 * sample rate, the bit rate, the Xtra's CLSID and a copyright string. Its
 * "MACR" signature sits at offset 36, and the audio begins at 4 + headerLength.
 *
 * Measured across the 4314 .swa files of the Loewenzahn/TKKG corpus: 4312 match
 * this layout exactly. The other two are bare MP3s carrying an iTunes ID3v2 tag
 * that were simply named .swa (both Loewenzahn 3 and 4 ship one as LOGO.swa),
 * so a raw MP3 is accepted as well and MAD skips the tag on its own.
 *
 * Returns the offset the audio starts at, or -1 if this is not SWA or MP3.
 */
static int32 findMP3Start(Common::SeekableReadStream *stream) {
	if (stream->size() < 40)
		return -1;

	stream->seek(36);
	if (stream->readUint32BE() == MKTAG('M', 'A', 'C', 'R')) {
		stream->seek(0);
		uint32 headerSize = stream->readUint32BE();
		// "MACR" living at offset 36 means the header always reaches at least
		// that far; the length field excludes its own four bytes.
		if (headerSize >= 36 && (int64)headerSize + 4 < stream->size())
			return 4 + headerSize;
	}

	stream->seek(0);
	byte magic[3];
	if (stream->read(magic, sizeof(magic)) == sizeof(magic)) {
		if (magic[0] == 'I' && magic[1] == 'D' && magic[2] == '3')
			return 0;
		// MPEG frame sync: eleven set bits.
		if (magic[0] == 0xff && (magic[1] & 0xe0) == 0xe0)
			return 0;
	}

	return -1;
}

Audio::AudioStream *AudioFileDecoder::getAudioStream(bool looping, bool forPuppet, DisposeAfterUse::Flag disposeAfterUse) {
	if (_path.empty())
		return nullptr;

	Common::Path newPath = findAudioPath(_path);
	Common::SeekableReadStream *copiedStream = Common::MacResManager::openFileOrDataFork(newPath);
	if (!copiedStream) {
		warning("Failed to open %s", _path.c_str());
		return nullptr;
	}

	uint32 magic1 = copiedStream->readUint32BE();
	copiedStream->readUint32BE();
	uint32 magic2 = copiedStream->readUint32BE();
	int32 mp3Start = findMP3Start(copiedStream);
	copiedStream->seek(0);

	Audio::RewindableAudioStream *stream = nullptr;
	if (magic1 == MKTAG('R', 'I', 'F', 'F') &&
		magic2 == MKTAG('W', 'A', 'V', 'E')) {
		stream = Audio::makeWAVStream(copiedStream, disposeAfterUse);
	} else if (magic1 == MKTAG('F', 'O', 'R', 'M') &&
				(magic2 == MKTAG('A', 'I', 'F', 'F') || magic2 == MKTAG('A', 'I', 'F', 'C'))) {
		stream = Audio::makeAIFFStream(copiedStream, disposeAfterUse);
	} else if (mp3Start >= 0) {
#ifdef USE_MAD
		// makeMP3Stream() must not be handed a stream that is merely seeked to
		// the audio: its skipID3() wraps the stream in a SeekableSubReadStream
		// anchored at absolute offset 0, which would hand the Macromedia header
		// straight back to the decoder. Wrap the audio portion here instead, so
		// that offset 0 of what MAD sees is the first MPEG frame.
		Common::SeekableReadStream *audio = new Common::SeekableSubReadStream(
			copiedStream, mp3Start, copiedStream->size(), disposeAfterUse);
		stream = Audio::makeMP3Stream(audio, DisposeAfterUse::YES);
		if (!stream)
			warning("AudioFileDecoder::getAudioStream(): failed to decode MPEG audio in %s", _path.c_str());
#else
		warning("AudioFileDecoder::getAudioStream(): %s needs MP3 support, which this build lacks", _path.c_str());
		delete copiedStream;
#endif
	} else {
		warning("Unknown file type for %s", _path.c_str());
		delete copiedStream;
	}

	if (stream) {
		if (looping) {
			return new Audio::LoopingAudioStream(stream, 0);
		}
		return stream;
	}

	return nullptr;
}

// A Shockwave Audio header is 320 bytes of fixed fields, and whatever follows
// is a cue point table: a count, then one 36-byte record per cue point holding
// its time in milliseconds and a 32-byte name. Measured over TKKG 14's 2249
// speech files, where `324 + count * 36` lands exactly on the end of the header
// every single time, and over Loewenzahn's .swa, where the files that say
// nothing simply stop at 320.
static const uint32 kSWACuePointCount = 320;
static const uint32 kSWACuePointTable = 324;
static const uint32 kSWACuePointSize = 36;

static bool readSWACuePoints(Common::SeekableReadStream *stream, Common::Array<int32> &times, Common::StringArray &names) {
	stream->seek(0);
	uint32 headerEnd = 4 + stream->readUint32BE();
	if (headerEnd <= kSWACuePointTable || headerEnd > (uint32)stream->size())
		return false;

	stream->seek(kSWACuePointCount);
	uint32 count = stream->readUint32BE();
	if (count > 0xffff || kSWACuePointTable + count * kSWACuePointSize != headerEnd)
		return false;

	uint32 skipped = 0;

	for (uint32 i = 0; i < count; i++) {
		stream->seek(kSWACuePointTable + i * kSWACuePointSize);
		uint32 time = stream->readUint32BE();

		byte raw[kSWACuePointSize - 4];
		if (stream->read(raw, sizeof(raw)) != sizeof(raw))
			break;

		Common::String name;
		for (uint j = 0; j < sizeof(raw) && raw[j]; j++)
			name += (char)raw[j];

		// A few records carry neither a readable name nor a sane time -- 594 of
		// 19950 in TKKG 14. Director skips what it cannot use, and so do we,
		// rather than hand the game a cue it would act on.
		if (name.empty() || time > 0x7fffffff) {
			skipped++;
			continue;
		}
		bool printable = true;
		for (uint j = 0; j < name.size(); j++)
			if (name[j] < 32 || (byte)name[j] > 126)
				printable = false;
		if (!printable) {
			skipped++;
			continue;
		}

		times.push_back((int32)time);
		names.push_back(name);
	}

	// How many records were thrown away matters: the count a game sees is the
	// one after this filter, and a table read at the wrong offset shows up as
	// everything being skipped rather than as an error.
	debugC(5, kDebugSound, "readSWACuePoints(): %d of %d records usable, %d skipped as unnamed or out of range",
			(int)times.size(), (int)count, (int)skipped);

	return !times.empty();
}

// AIFF keeps its markers in a 'MARK' chunk, positioned in sample frames, so the
// rate from 'COMM' turns them into milliseconds. TKKG 13 speaks this way where
// TKKG 14 speaks Shockwave Audio -- same names, different container.
static bool readAIFFCuePoints(Common::SeekableReadStream *stream, Common::Array<int32> &times, Common::StringArray &names) {
	double rate = 0.0;
	Common::Array<uint32> positions;
	Common::StringArray markNames;

	int64 pos = 12;
	while (pos + 8 <= stream->size()) {
		stream->seek(pos);
		uint32 tag = stream->readUint32BE();
		uint32 size = stream->readUint32BE();
		int64 body = pos + 8;

		// An inventory of what the file actually holds. The shared AIFF decoder
		// already names the chunks it steps over -- 4047 "Skipping AIFF 'INST'
		// chunk", 3468 'MARK' and 671 'APPL' in the logs so far -- but only
		// their names, only on the global debug level, and only for files that
		// get decoded for playback. This walker sees every chunk of every file
		// whose cue points are asked for, so it is the cheaper place to say
		// where each one sits and how long it is.
		debugC(5, kDebugSound, "readAIFFCuePoints(): chunk '%s' at %d, %d bytes",
				tag2str(tag), (int)pos, (int)size);

		if (tag == MKTAG('C', 'O', 'M', 'M') && size >= 18) {
			stream->seek(body);
			uint16 channels = stream->readUint16BE();
			uint32 frames = stream->readUint32BE();
			uint16 bits = stream->readUint16BE();
			// Both halves in a defined order: as arguments to the constructor the
			// compiler is free to read them the other way round, which takes the
			// exponent from the middle of the mantissa and yields a rate of zero.
			uint16 signAndExponent = stream->readUint16BE();
			uint64 mantissa = stream->readUint64BE();
			rate = Common::XPFloat(signAndExponent, mantissa).toDouble();

			// The rate is what turns a marker's sample frame into a time, so a
			// marker table that lands in the wrong place is just as likely to be
			// this number's fault as the table's.
			debugC(5, kDebugSound, "readAIFFCuePoints():   COMM %d channels, %d frames, %d bits, rate %f -> %d ms",
					channels, frames, bits, rate, rate > 0.0 ? (int)(frames / rate * 1000.0 + 0.5) : 0);
		} else if (tag == MKTAG('M', 'A', 'R', 'K') && size >= 2) {
			stream->seek(body);
			uint16 count = stream->readUint16BE();
			debugC(5, kDebugSound, "readAIFFCuePoints():   MARK %d markers", count);
			for (uint16 i = 0; i < count && stream->pos() + 7 <= body + (int64)size; i++) {
				// The id is what an INST loop points at, so print it rather than
				// dropping it unseen.
				uint16 markerId = stream->readUint16BE();
				uint32 frame = stream->readUint32BE();
				positions.push_back(frame);
				byte len = stream->readByte();
				Common::String name;
				for (byte j = 0; j < len; j++)
					name += (char)stream->readByte();
				markNames.push_back(name);
				if (!(len & 1))
					stream->readByte();	// pad to an even length

				debugC(5, kDebugSound, "readAIFFCuePoints():     marker %d id %d: '%s' at frame %d",
						i + 1, markerId, name.c_str(), frame);
			}
		} else if (tag == MKTAG('I', 'N', 'S', 'T') && size >= 20 && debugChannelSet(5, kDebugSound)) {
			// Twenty fixed bytes, and the two loops name marker ids rather than
			// frames. Nothing reads them yet; a looping sound that starts over at
			// the wrong place would show up here first. Behind an explicit channel
			// check because nothing but the log wants it, and debugC would do the
			// reading either way.
			stream->seek(body);
			int8 baseNote = stream->readSByte();
			int8 detune = stream->readSByte();
			int8 lowNote = stream->readSByte();
			int8 highNote = stream->readSByte();
			int8 lowVel = stream->readSByte();
			int8 highVel = stream->readSByte();
			int16 gain = stream->readSint16BE();
			int16 susMode = stream->readSint16BE();
			int16 susBegin = stream->readSint16BE();
			int16 susEnd = stream->readSint16BE();
			int16 relMode = stream->readSint16BE();
			int16 relBegin = stream->readSint16BE();
			int16 relEnd = stream->readSint16BE();

			debugC(5, kDebugSound, "readAIFFCuePoints():   INST base %d detune %d, notes %d-%d, velocity %d-%d, gain %d dB",
					baseNote, detune, lowNote, highNote, lowVel, highVel, gain);
			debugC(5, kDebugSound, "readAIFFCuePoints():   INST sustain loop mode %d marker %d..%d, release loop mode %d marker %d..%d",
					susMode, susBegin, susEnd, relMode, relBegin, relEnd);
		} else if (tag == MKTAG('A', 'P', 'P', 'L') && size >= 4 && debugChannelSet(5, kDebugSound)) {
			// Application specific: a four-character signature and then whatever
			// that application wanted. Say which application and show the start
			// of it, so an unknown block stops being invisible. Behind the same
			// channel check: building the preview string for 2249 speech files
			// would otherwise cost something for nothing.
			stream->seek(body);
			uint32 signature = stream->readUint32BE();
			Common::String preview;
			for (uint32 i = 0; i < MIN<uint32>(size - 4, 32); i++) {
				byte b = stream->readByte();
				preview += Common::String::format("%02x", b);
				if (b >= 32 && b <= 126)
					preview += Common::String::format("(%c)", (char)b);
				preview += ' ';
			}
			debugC(5, kDebugSound, "readAIFFCuePoints():   APPL '%s', %d bytes: %s",
					tag2str(signature), (int)size - 4, preview.c_str());
		}

		pos = body + size + (size & 1);
	}

	if (rate <= 0.0 || positions.empty() || positions.size() != markNames.size()) {
		debugC(3, kDebugSound, "readAIFFCuePoints(): nothing usable -- rate %f, %d positions, %d names",
				rate, (int)positions.size(), (int)markNames.size());
		return false;
	}

	for (uint i = 0; i < positions.size(); i++) {
		times.push_back((int32)(positions[i] / rate * 1000.0 + 0.5));
		names.push_back(markNames[i]);
	}

	return true;
}

bool AudioFileDecoder::getCuePoints(Common::Array<int32> &times, Common::StringArray &names) {
	if (_path.empty())
		return false;

	Common::Path newPath = findAudioPath(_path);
	Common::SeekableReadStream *stream = Common::MacResManager::openFileOrDataFork(newPath);
	if (!stream)
		return false;

	bool found = false;
	if (stream->size() >= 40) {
		stream->seek(0);
		uint32 magic1 = stream->readUint32BE();
		stream->readUint32BE();
		uint32 magic2 = stream->readUint32BE();
		stream->seek(36);
		uint32 macr = stream->readUint32BE();

		if (magic1 == MKTAG('F', 'O', 'R', 'M') &&
				(magic2 == MKTAG('A', 'I', 'F', 'F') || magic2 == MKTAG('A', 'I', 'F', 'C')))
			found = readAIFFCuePoints(stream, times, names);
		else if (macr == MKTAG('M', 'A', 'C', 'R'))
			found = readSWACuePoints(stream, times, names);
	}

	delete stream;

	if (found) {
		debugC(3, kDebugSound, "AudioFileDecoder::getCuePoints(): %d cue points in '%s', first '%s' at %d ms",
				(int)times.size(), _path.c_str(), names[0].c_str(), times[0]);

		// Naming only the first one says nothing about a table that goes wrong
		// in the middle, which is the way an offset or a rate is wrong: the
		// count still looks right and the first entry still reads "MB" at 0 ms.
		// The numbering is the one Lingo uses -- cuePointNames and
		// cuePointTimes are one-based, while _lastCuePointIndex counts from 0.
		for (uint i = 0; i < times.size(); i++)
			debugC(5, kDebugSound, "AudioFileDecoder::getCuePoints():   %d: '%s' at %d ms",
					(int)i + 1, names[i].c_str(), times[i]);
	}

	return found;
}

uint32 AudioFileDecoder::getDuration() {
	// A linked sound only knows how long it is once its file has been read, so
	// decode it, ask, and throw the stream away again. Scripts read this once
	// per sound to pace themselves against it, not per frame.
	Audio::AudioStream *stream = getAudioStream(false, false, DisposeAfterUse::YES);
	if (!stream)
		return 0;

	uint32 duration = 0;
	Audio::SeekableAudioStream *seekable = dynamic_cast<Audio::SeekableAudioStream *>(stream);
	if (seekable)
		duration = seekable->getLength().msecs();

	delete stream;
	return duration;
}

MoaStreamDecoder::MoaStreamDecoder(Common::String &format, Common::SeekableReadStreamEndian *stream)
		: AudioDecoder() {
	_format = format;
	_stream = stream;
}

MoaStreamDecoder::~MoaStreamDecoder() {
	if (_stream) {
		delete _stream;
		_stream = nullptr;
	}
}

Audio::AudioStream *MoaStreamDecoder::getAudioStream(bool looping, bool forPuppet, DisposeAfterUse::Flag disposeAfterUse) {
	if (!_stream)
		return nullptr;

	// Make sure we're at the start of the stream
	_stream->seek(0, SEEK_SET);

	Audio::RewindableAudioStream *stream = nullptr;
	if (_format.equalsIgnoreCase("kMoaCfFormat_AIFF")) {
		stream = Audio::makeAIFFStream(_stream, DisposeAfterUse::NO);
	} else {
		warning("Unsupported Moa stream type '%s'", _format.c_str());
		delete _stream;
	}

	if (stream) {
		if (looping) {
			return new Audio::LoopingAudioStream(stream, 0);
		}
		return stream;
	}

	return nullptr;
}

MoaSoundFormatDecoder::MoaSoundFormatDecoder() {
}

MoaSoundFormatDecoder::~MoaSoundFormatDecoder() {
	if (_data) {
		free(_data);
		_data = nullptr;
	}
}

bool MoaSoundFormatDecoder::loadHeaderStream(Common::SeekableReadStreamEndian &stream) {
	_format.offset = stream.readSint32BE();
	_format.size = stream.readSint32BE();
	_format.playbackStart = stream.readSint32BE();
	_format.playbackStartFrame = stream.readSint32BE();
	_format.loopStart = stream.readSint32BE();
	_format.loopStartFrame = stream.readSint32BE();
	_format.loopEnd = stream.readSint32BE();
	_format.loopEndFrame = stream.readSint32BE();
	_format.playbackEnd = stream.readSint32BE();
	_format.playbackEndFrame = stream.readSint32BE();
	_format.numFrames = stream.readSint32BE();
	_format.frameRate = stream.readSint32BE();
	_format.byteRate = stream.readSint32BE();
	stream.read(_format.compressionType, 16);
	_format.bitsPerSample = stream.readSint32BE();
	_format.bytesPerSample = stream.readSint32BE();
	_format.numChannels = stream.readSint32BE();
	_format.bytesPerFrame = stream.readSint32BE();
	stream.read(_format.soundHeaderType, 16);
	for (int i = 0; i < 63; i++) {
		_format.platformData[i] = stream.readUint32BE();
	}
	_format.bytesPerBlock = stream.readSint32BE();

	if (debugChannelSet(5, kDebugLoading)) {
		debugC(5, kDebugLoading, "MoaSoundFormatDecoder: Loading header");
		debugC(5, kDebugLoading, "offset: %d, size: %d, playbackStart: %d, playbackStartFrame: %d",
		_format.offset, _format.size, _format.playbackStart, _format.playbackStartFrame);
		debugC(5, kDebugLoading, "loopStart: %d, loopStartEndFrame: %d, loopEnd: %d, loopEndFrame: %d",
		_format.loopStart, _format.loopStartFrame, _format.loopEnd, _format.loopEndFrame);
		debugC(5, kDebugLoading, "playbackEnd: %d, playbackEndFrame: %d, numFrames: %d, frameRate: %d, byteRate: %d",
		_format.playbackEnd, _format.playbackEndFrame, _format.numFrames, _format.frameRate, _format.byteRate);
		debugC(5, kDebugLoading, "bitsPerSample: %d, bytesPerSample: %d, numChannels: %d, bytesPerFrame: %d, bytesPerBlock: %d",
		_format.bitsPerSample, _format.bytesPerSample, _format.numChannels, _format.bytesPerFrame, _format.bytesPerBlock);

	}
	return false;
}

bool MoaSoundFormatDecoder::loadSampleStream(Common::SeekableReadStreamEndian &stream) {
	_size = stream.size();
	if (_data) {
		free(_data);
		_data = nullptr;
	}
	_data = (byte *)malloc(_size);
	stream.read(_data, _size);
	return false;
}

Audio::AudioStream *MoaSoundFormatDecoder::getAudioStream(bool looping, bool forPuppet, DisposeAfterUse::Flag disposeAfterUse) {
	if (!_data)
		return nullptr;
	byte *buffer = (byte *)malloc(_size);
	memcpy(buffer, _data, _size);

	Audio::SeekableAudioStream *stream = Audio::makeRawStream(buffer,
			_size, _format.frameRate,
			((_format.bitsPerSample == 16) ? Audio::RawFlags::FLAG_16BITS : 0) |
			((_format.numChannels == 2) ? Audio::RawFlags::FLAG_STEREO : 0) |
			((_format.bitsPerSample == 8) ? Audio::RawFlags::FLAG_UNSIGNED : 0),
			disposeAfterUse);

	if (looping) {
		if (_format.loopEndFrame <= _format.loopStartFrame) {
			return new Audio::LoopingAudioStream(stream, 0);
		} else {
			return new Audio::SubLoopingAudioStream(stream, 0, Audio::Timestamp(0, _format.loopStartFrame, _format.frameRate), Audio::Timestamp(0, _format.loopEndFrame, _format.frameRate));
		}
	}
	return stream;
}

} // End of namespace Director
