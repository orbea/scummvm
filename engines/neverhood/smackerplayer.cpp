/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#include "graphics/palette.h"
#include "neverhood/smackerplayer.h"
#include "neverhood/palette.h"
#include "neverhood/resourceman.h"
#include "neverhood/scene.h"

namespace Neverhood {

// SmackerSurface

SmackerSurface::SmackerSurface(NeverhoodEngine *vm)
	: BaseSurface(vm, 0, 0, 0), _smackerFrame(NULL) {
}

void SmackerSurface::draw() {
	if (_smackerFrame && _visible && _drawRect.width > 0 && _drawRect.height > 0) {
		_vm->_screen->drawSurface2(_smackerFrame, _drawRect, _clipRect, false);
	}
}

void SmackerSurface::setSmackerFrame(const Graphics::Surface *smackerFrame) {
	_drawRect.x = 0;
	_drawRect.y = 0;
	_drawRect.width = smackerFrame->w;
	_drawRect.height = smackerFrame->h;
	// TODO: Check if _sysRect is needed at all in the reimplementation...
	_sysRect.x = 0;
	_sysRect.y = 0;
	_sysRect.width = (smackerFrame->w + 3) & 0xFFFC; // align by 4 bytes
	_sysRect.height = smackerFrame->h;
	_smackerFrame = smackerFrame;
}

// SmackerDoubleSurface

SmackerDoubleSurface::SmackerDoubleSurface(NeverhoodEngine *vm)
	: SmackerSurface(vm) {
}

void SmackerDoubleSurface::draw() {
	if (_smackerFrame && _visible && _drawRect.width > 0 && _drawRect.height > 0) {
		_vm->_screen->drawDoubleSurface2(_smackerFrame, _drawRect);
	}
}

// SmackerPlayer

SmackerPlayer::SmackerPlayer(NeverhoodEngine *vm, Scene *scene, uint32 fileHash, bool doubleSurface, bool flag)
	: Entity(vm, 0), _scene(scene), _doubleSurface(doubleSurface), _dirtyFlag(false), _flag2(false),
	_palette(NULL), _smackerDecoder(NULL), _smackerSurface(NULL), _stream(NULL), _smackerFirst(true),
	_drawX(-1), _drawY(-1) {

	SetUpdateHandler(&SmackerPlayer::update);
	open(fileHash, flag);
}

SmackerPlayer::~SmackerPlayer() {
	close();
}   

void SmackerPlayer::open(uint32 fileHash, bool keepLastFrame) {
	debug("SmackerPlayer::open(%08X)", fileHash);
	
	_fileHash = fileHash;
	_keepLastFrame = keepLastFrame;

	close();

	if (_doubleSurface) {
		_smackerSurface = new SmackerDoubleSurface(_vm);
	} else {
		_smackerSurface = new SmackerSurface(_vm);
	}

	_smackerFirst = true;

	_stream = _vm->_res->createStream(fileHash);

	// TODO: _keepLastFrame stuff

	_smackerDecoder = new Video::SmackerDecoder(_vm->_mixer);
	_smackerDecoder->loadStream(_stream);
	
	_palette = new Palette(_vm);
	_palette->usePalette();
	
}

void SmackerPlayer::close() {
	delete _smackerDecoder;
	delete _palette;
	// NOTE: The SmackerDecoder deletes the _stream
	delete _smackerSurface;
	_smackerDecoder = NULL;
	_palette = NULL;
	_stream = NULL;
	_smackerSurface = NULL;
}

void SmackerPlayer::gotoFrame(uint frameNumber) {
	// TODO?
}

uint32 SmackerPlayer::getFrameCount() {
	return _smackerDecoder ? _smackerDecoder->getFrameCount() : 0;
}

uint32 SmackerPlayer::getFrameNumber() {
	return _smackerDecoder ? _smackerDecoder->getCurFrame() : 0;
}

uint SmackerPlayer::getStatus() {
	return 0;
}

void SmackerPlayer::setDrawPos(int16 x, int16 y) {
	_drawX = x;
	_drawY = y;
	if (_smackerSurface) {
		_smackerSurface->getDrawRect().x = _drawX;
		_smackerSurface->getDrawRect().y = _drawY;
	}
}

void SmackerPlayer::rewind() {

	delete _smackerDecoder;
	_smackerDecoder = NULL;
	_stream = NULL;

	_smackerFirst = true;

	_stream = _vm->_res->createStream(_fileHash);

	_smackerDecoder = new Video::SmackerDecoder(_vm->_mixer);
	_smackerDecoder->loadStream(_stream);
	
}

void SmackerPlayer::update() {
	debug(8, "SmackerPlayer::update()");

	if (!_smackerDecoder)
		return;

	if (_dirtyFlag) {
		// TODO _vm->_screen->resetDirtyRects();
		_dirtyFlag = false;
	}

#if 0
	if (!_smackerDecoder->endOfVideo()) {
		updateFrame();
		if (_smackerDecoder->endOfVideo() && !_keepLastFrame) {
			// Inform the scene about the end of the video playback
			if (_scene) {
				_scene->sendMessage(0x3002, 0, this);
			}
			_flag2 = true;
		} else {
			if (_smackerDecoder->endOfVideo()) {
				rewind();
				updateFrame();
			}
			_flag2 = false;
		}
	}
#endif

	if (!_smackerDecoder->endOfVideo()) {
		updateFrame();
	} else if (!_keepLastFrame) {
		// Inform the scene about the end of the video playback
		if (_scene) {
			_scene->sendMessage(0x3002, 0, this);
		}
		_flag2 = true;
	} else {
		rewind();
		updateFrame();
		_flag2 = false;
	}
	
}

void SmackerPlayer::updateFrame() {
	const Graphics::Surface *smackerFrame = _smackerDecoder->decodeNextFrame();

	if (_smackerFirst) {
		_smackerSurface->setSmackerFrame(smackerFrame);
		if (_drawX < 0 || _drawY < 0) {
			if (_doubleSurface) {
				_drawX = 320 - _smackerDecoder->getWidth();
				_drawY = 240 - _smackerDecoder->getHeight();
			} else {
				_drawX = (640 - _smackerDecoder->getWidth()) / 2;
				_drawY = (480 - _smackerDecoder->getHeight()) / 2;
			}
		}
		_smackerSurface->getDrawRect().x = _drawX;
		_smackerSurface->getDrawRect().y = _drawY;
		_smackerFirst = false;
	}
	
	if (_doubleSurface) {
		// TODO
	}

	// TODO _vm->_screen->_skipUpdate = true;
	_dirtyFlag = true;

	if (_smackerDecoder->hasDirtyPalette()) {
		updatePalette();
	}
}

void SmackerPlayer::updatePalette() {
	byte tempPalette[1024];
	const byte *smackerPalette = _smackerDecoder->getPalette();
	for (int i = 0; i < 256; i++) {
		tempPalette[i * 4 + 0] = smackerPalette[i * 3 + 0];
		tempPalette[i * 4 + 1] = smackerPalette[i * 3 + 1];
		tempPalette[i * 4 + 2] = smackerPalette[i * 3 + 2];
	}
	_palette->copyPalette(tempPalette, 0, 256, 0);
}

} // End of namespace Neverhood
