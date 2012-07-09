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

/*
 * This file is based on WME Lite.
 * http://dead-code.org/redir.php?target=wmelite
 * Copyright (c) 2011 Jan Nedoma
 */

#include "engines/wintermute/dcgf.h"
#include "engines/wintermute/Ad/AdGame.h"
#include "engines/wintermute/Ad/AdResponseBox.h"
#include "engines/wintermute/Base/BParser.h"
#include "engines/wintermute/Base/BSurfaceStorage.h"
#include "engines/wintermute/UI/UIButton.h"
#include "engines/wintermute/UI/UIWindow.h"
#include "engines/wintermute/Base/BDynBuffer.h"
#include "engines/wintermute/Base/BFontStorage.h"
#include "engines/wintermute/Base/BFont.h"
#include "engines/wintermute/Ad/AdResponse.h"
#include "engines/wintermute/Base/scriptables/ScScript.h"
#include "engines/wintermute/Base/scriptables/ScStack.h"
#include "engines/wintermute/Base/BSprite.h"
#include "engines/wintermute/Base/BFileManager.h"
#include "engines/wintermute/utils/utils.h"
#include "engines/wintermute/PlatformSDL.h"
#include "common/str.h"

namespace WinterMute {

IMPLEMENT_PERSISTENT(CAdResponseBox, false)

//////////////////////////////////////////////////////////////////////////
CAdResponseBox::CAdResponseBox(CBGame *inGame): CBObject(inGame) {
	_font = _fontHover = NULL;

	_window = NULL;
	_shieldWindow = new CUIWindow(Game);

	_horizontal = false;
	CBPlatform::setRectEmpty(&_responseArea);
	_scrollOffset = 0;
	_spacing = 0;

	_waitingScript = NULL;
	_lastResponseText = NULL;
	_lastResponseTextOrig = NULL;

	_verticalAlign = VAL_BOTTOM;
	_align = TAL_LEFT;
}


//////////////////////////////////////////////////////////////////////////
CAdResponseBox::~CAdResponseBox() {

	delete _window;
	_window = NULL;
	delete _shieldWindow;
	_shieldWindow = NULL;
	delete[] _lastResponseText;
	_lastResponseText = NULL;
	delete[] _lastResponseTextOrig;
	_lastResponseTextOrig = NULL;

	if (_font) Game->_fontStorage->removeFont(_font);
	if (_fontHover) Game->_fontStorage->removeFont(_fontHover);

	clearResponses();
	clearButtons();

	_waitingScript = NULL;
}


//////////////////////////////////////////////////////////////////////////
void CAdResponseBox::clearResponses() {
	for (int i = 0; i < _responses.getSize(); i++) {
		delete _responses[i];
	}
	_responses.removeAll();
}


//////////////////////////////////////////////////////////////////////////
void CAdResponseBox::clearButtons() {
	for (int i = 0; i < _respButtons.getSize(); i++) {
		delete _respButtons[i];
	}
	_respButtons.removeAll();
}


//////////////////////////////////////////////////////////////////////////
ERRORCODE CAdResponseBox::invalidateButtons() {
	for (int i = 0; i < _respButtons.getSize(); i++) {
		_respButtons[i]->_image = NULL;
		_respButtons[i]->_cursor = NULL;
		_respButtons[i]->_font = NULL;
		_respButtons[i]->_fontHover = NULL;
		_respButtons[i]->_fontPress = NULL;
		_respButtons[i]->setText("");
	}
	return STATUS_OK;
}


//////////////////////////////////////////////////////////////////////////
ERRORCODE CAdResponseBox::createButtons() {
	clearButtons();

	_scrollOffset = 0;
	for (int i = 0; i < _responses.getSize(); i++) {
		CUIButton *btn = new CUIButton(Game);
		if (btn) {
			btn->_parent = _window;
			btn->_sharedFonts = btn->_sharedImages = true;
			btn->_sharedCursors = true;
			// iconic
			if (_responses[i]->_icon) {
				btn->_image = _responses[i]->_icon;
				if (_responses[i]->_iconHover)   btn->_imageHover = _responses[i]->_iconHover;
				if (_responses[i]->_iconPressed) btn->_imagePress = _responses[i]->_iconPressed;

				btn->setCaption(_responses[i]->_text);
				if (_cursor) btn->_cursor = _cursor;
				else if (Game->_activeCursor) btn->_cursor = Game->_activeCursor;
			}
			// textual
			else {
				btn->setText(_responses[i]->_text);
				btn->_font = (_font == NULL) ? Game->_systemFont : _font;
				btn->_fontHover = (_fontHover == NULL) ? Game->_systemFont : _fontHover;
				btn->_fontPress = btn->_fontHover;
				btn->_align = _align;

				if (Game->_touchInterface)
					btn->_fontHover = btn->_font;


				if (_responses[i]->_font) btn->_font = _responses[i]->_font;

				btn->_width = _responseArea.right - _responseArea.left;
				if (btn->_width <= 0) btn->_width = Game->_renderer->_width;
			}
			btn->setName("response");
			btn->correctSize();

			// make the responses touchable
			if (Game->_touchInterface)
				btn->_height = MAX(btn->_height, 50);

			//btn->SetListener(this, btn, _responses[i]->_iD);
			btn->setListener(this, btn, i);
			btn->_visible = false;
			_respButtons.add(btn);

			if (_responseArea.bottom - _responseArea.top < btn->_height) {
				Game->LOG(0, "Warning: Response '%s' is too high to be displayed within response box. Correcting.", _responses[i]->_text);
				_responseArea.bottom += (btn->_height - (_responseArea.bottom - _responseArea.top));
			}
		}
	}
	_ready = false;

	return STATUS_OK;
}


//////////////////////////////////////////////////////////////////////////
ERRORCODE CAdResponseBox::loadFile(const char *filename) {
	byte *buffer = Game->_fileManager->readWholeFile(filename);
	if (buffer == NULL) {
		Game->LOG(0, "CAdResponseBox::LoadFile failed for file '%s'", filename);
		return STATUS_FAILED;
	}

	ERRORCODE ret;

	_filename = new char [strlen(filename) + 1];
	strcpy(_filename, filename);

	if (DID_FAIL(ret = loadBuffer(buffer, true))) Game->LOG(0, "Error parsing RESPONSE_BOX file '%s'", filename);


	delete [] buffer;

	return ret;
}


TOKEN_DEF_START
TOKEN_DEF(RESPONSE_BOX)
TOKEN_DEF(TEMPLATE)
TOKEN_DEF(FONT_HOVER)
TOKEN_DEF(FONT)
TOKEN_DEF(AREA)
TOKEN_DEF(HORIZONTAL)
TOKEN_DEF(SPACING)
TOKEN_DEF(WINDOW)
TOKEN_DEF(CURSOR)
TOKEN_DEF(TEXT_ALIGN)
TOKEN_DEF(VERTICAL_ALIGN)
TOKEN_DEF(EDITOR_PROPERTY)
TOKEN_DEF_END
//////////////////////////////////////////////////////////////////////////
ERRORCODE CAdResponseBox::loadBuffer(byte *buffer, bool complete) {
	TOKEN_TABLE_START(commands)
	TOKEN_TABLE(RESPONSE_BOX)
	TOKEN_TABLE(TEMPLATE)
	TOKEN_TABLE(FONT_HOVER)
	TOKEN_TABLE(FONT)
	TOKEN_TABLE(AREA)
	TOKEN_TABLE(HORIZONTAL)
	TOKEN_TABLE(SPACING)
	TOKEN_TABLE(WINDOW)
	TOKEN_TABLE(CURSOR)
	TOKEN_TABLE(TEXT_ALIGN)
	TOKEN_TABLE(VERTICAL_ALIGN)
	TOKEN_TABLE(EDITOR_PROPERTY)
	TOKEN_TABLE_END


	byte *params;
	int cmd;
	CBParser parser(Game);

	if (complete) {
		if (parser.getCommand((char **)&buffer, commands, (char **)&params) != TOKEN_RESPONSE_BOX) {
			Game->LOG(0, "'RESPONSE_BOX' keyword expected.");
			return STATUS_FAILED;
		}
		buffer = params;
	}

	while ((cmd = parser.getCommand((char **)&buffer, commands, (char **)&params)) > 0) {
		switch (cmd) {
		case TOKEN_TEMPLATE:
			if (DID_FAIL(loadFile((char *)params))) cmd = PARSERR_GENERIC;
			break;

		case TOKEN_WINDOW:
			delete _window;
			_window = new CUIWindow(Game);
			if (!_window || DID_FAIL(_window->loadBuffer(params, false))) {
				delete _window;
				_window = NULL;
				cmd = PARSERR_GENERIC;
			} else if (_shieldWindow) _shieldWindow->_parent = _window;
			break;

		case TOKEN_FONT:
			if (_font) Game->_fontStorage->removeFont(_font);
			_font = Game->_fontStorage->addFont((char *)params);
			if (!_font) cmd = PARSERR_GENERIC;
			break;

		case TOKEN_FONT_HOVER:
			if (_fontHover) Game->_fontStorage->removeFont(_fontHover);
			_fontHover = Game->_fontStorage->addFont((char *)params);
			if (!_fontHover) cmd = PARSERR_GENERIC;
			break;

		case TOKEN_AREA:
			parser.scanStr((char *)params, "%d,%d,%d,%d", &_responseArea.left, &_responseArea.top, &_responseArea.right, &_responseArea.bottom);
			break;

		case TOKEN_HORIZONTAL:
			parser.scanStr((char *)params, "%b", &_horizontal);
			break;

		case TOKEN_TEXT_ALIGN:
			if (scumm_stricmp((char *)params, "center") == 0) _align = TAL_CENTER;
			else if (scumm_stricmp((char *)params, "right") == 0) _align = TAL_RIGHT;
			else _align = TAL_LEFT;
			break;

		case TOKEN_VERTICAL_ALIGN:
			if (scumm_stricmp((char *)params, "top") == 0) _verticalAlign = VAL_TOP;
			else if (scumm_stricmp((char *)params, "center") == 0) _verticalAlign = VAL_CENTER;
			else _verticalAlign = VAL_BOTTOM;
			break;

		case TOKEN_SPACING:
			parser.scanStr((char *)params, "%d", &_spacing);
			break;

		case TOKEN_EDITOR_PROPERTY:
			parseEditorProperty(params, false);
			break;

		case TOKEN_CURSOR:
			delete _cursor;
			_cursor = new CBSprite(Game);
			if (!_cursor || DID_FAIL(_cursor->loadFile((char *)params))) {
				delete _cursor;
				_cursor = NULL;
				cmd = PARSERR_GENERIC;
			}
			break;
		}
	}
	if (cmd == PARSERR_TOKENNOTFOUND) {
		Game->LOG(0, "Syntax error in RESPONSE_BOX definition");
		return STATUS_FAILED;
	}

	if (_window) {
		for (int i = 0; i < _window->_widgets.getSize(); i++) {
			if (!_window->_widgets[i]->_listenerObject)
				_window->_widgets[i]->setListener(this, _window->_widgets[i], 0);
		}
	}

	return STATUS_OK;
}

//////////////////////////////////////////////////////////////////////////
ERRORCODE CAdResponseBox::saveAsText(CBDynBuffer *buffer, int indent) {
	buffer->putTextIndent(indent, "RESPONSE_BOX\n");
	buffer->putTextIndent(indent, "{\n");

	buffer->putTextIndent(indent + 2, "AREA { %d, %d, %d, %d }\n", _responseArea.left, _responseArea.top, _responseArea.right, _responseArea.bottom);

	if (_font && _font->_filename)
		buffer->putTextIndent(indent + 2, "FONT=\"%s\"\n", _font->_filename);
	if (_fontHover && _fontHover->_filename)
		buffer->putTextIndent(indent + 2, "FONT_HOVER=\"%s\"\n", _fontHover->_filename);

	if (_cursor && _cursor->_filename)
		buffer->putTextIndent(indent + 2, "CURSOR=\"%s\"\n", _cursor->_filename);

	buffer->putTextIndent(indent + 2, "HORIZONTAL=%s\n", _horizontal ? "TRUE" : "FALSE");

	switch (_align) {
	case TAL_LEFT:
		buffer->putTextIndent(indent + 2, "TEXT_ALIGN=\"%s\"\n", "left");
		break;
	case TAL_RIGHT:
		buffer->putTextIndent(indent + 2, "TEXT_ALIGN=\"%s\"\n", "right");
		break;
	case TAL_CENTER:
		buffer->putTextIndent(indent + 2, "TEXT_ALIGN=\"%s\"\n", "center");
		break;
	default:
		error("CAdResponseBox::SaveAsText - Unhandled enum");
		break;
	}

	switch (_verticalAlign) {
	case VAL_TOP:
		buffer->putTextIndent(indent + 2, "VERTICAL_ALIGN=\"%s\"\n", "top");
		break;
	case VAL_BOTTOM:
		buffer->putTextIndent(indent + 2, "VERTICAL_ALIGN=\"%s\"\n", "bottom");
		break;
	case VAL_CENTER:
		buffer->putTextIndent(indent + 2, "VERTICAL_ALIGN=\"%s\"\n", "center");
		break;
	}

	buffer->putTextIndent(indent + 2, "SPACING=%d\n", _spacing);

	buffer->putTextIndent(indent + 2, "\n");

	// window
	if (_window) _window->saveAsText(buffer, indent + 2);

	buffer->putTextIndent(indent + 2, "\n");

	// editor properties
	CBBase::saveAsText(buffer, indent + 2);

	buffer->putTextIndent(indent, "}\n");
	return STATUS_OK;
}


//////////////////////////////////////////////////////////////////////////
ERRORCODE CAdResponseBox::display() {
	Rect32 rect = _responseArea;
	if (_window) {
		CBPlatform::offsetRect(&rect, _window->_posX, _window->_posY);
		//_window->display();
	}

	int xxx, yyy, i;

	xxx = rect.left;
	yyy = rect.top;

	// shift down if needed
	if (!_horizontal) {
		int total_height = 0;
		for (i = 0; i < _respButtons.getSize(); i++) total_height += (_respButtons[i]->_height + _spacing);
		total_height -= _spacing;

		switch (_verticalAlign) {
		case VAL_BOTTOM:
			if (yyy + total_height < rect.bottom)
				yyy = rect.bottom - total_height;
			break;

		case VAL_CENTER:
			if (yyy + total_height < rect.bottom)
				yyy += ((rect.bottom - rect.top) - total_height) / 2;
			break;

		case VAL_TOP:
			// do nothing
			break;
		}
	}

	// prepare response buttons
	bool scrollNeeded = false;
	for (i = _scrollOffset; i < _respButtons.getSize(); i++) {
		if ((_horizontal && xxx + _respButtons[i]->_width > rect.right)
		        || (!_horizontal && yyy + _respButtons[i]->_height > rect.bottom)) {

			scrollNeeded = true;
			_respButtons[i]->_visible = false;
			break;
		}

		_respButtons[i]->_visible = true;
		_respButtons[i]->_posX = xxx;
		_respButtons[i]->_posY = yyy;

		if (_horizontal) {
			xxx += (_respButtons[i]->_width + _spacing);
		} else {
			yyy += (_respButtons[i]->_height + _spacing);
		}
	}

	// show appropriate scroll buttons
	if (_window) {
		_window->showWidget("prev", _scrollOffset > 0);
		_window->showWidget("next", scrollNeeded);
	}

	// go exclusive
	if (_shieldWindow) {
		_shieldWindow->_posX = _shieldWindow->_posY = 0;
		_shieldWindow->_width = Game->_renderer->_width;
		_shieldWindow->_height = Game->_renderer->_height;

		_shieldWindow->display();
	}

	// display window
	if (_window) _window->display();


	// display response buttons
	for (i = _scrollOffset; i < _respButtons.getSize(); i++) {
		_respButtons[i]->display();
	}

	return STATUS_OK;
}


//////////////////////////////////////////////////////////////////////////
ERRORCODE CAdResponseBox::listen(CBScriptHolder *param1, uint32 param2) {
	CUIObject *obj = (CUIObject *)param1;

	switch (obj->_type) {
	case UI_BUTTON:
		if (scumm_stricmp(obj->_name, "prev") == 0) {
			_scrollOffset--;
		} else if (scumm_stricmp(obj->_name, "next") == 0) {
			_scrollOffset++;
		} else if (scumm_stricmp(obj->_name, "response") == 0) {
			if (_waitingScript) _waitingScript->_stack->pushInt(_responses[param2]->_iD);
			handleResponse(_responses[param2]);
			_waitingScript = NULL;
			Game->_state = GAME_RUNNING;
			((CAdGame *)Game)->_stateEx = GAME_NORMAL;
			_ready = true;
			invalidateButtons();
			clearResponses();
		} else return CBObject::listen(param1, param2);
		break;
	default:
		error("AdResponseBox::Listen - Unhandled enum");
	}

	return STATUS_OK;
}


//////////////////////////////////////////////////////////////////////////
ERRORCODE CAdResponseBox::persist(CBPersistMgr *persistMgr) {
	CBObject::persist(persistMgr);

	persistMgr->transfer(TMEMBER(_font));
	persistMgr->transfer(TMEMBER(_fontHover));
	persistMgr->transfer(TMEMBER(_horizontal));
	persistMgr->transfer(TMEMBER(_lastResponseText));
	persistMgr->transfer(TMEMBER(_lastResponseTextOrig));
	_respButtons.persist(persistMgr);
	persistMgr->transfer(TMEMBER(_responseArea));
	_responses.persist(persistMgr);
	persistMgr->transfer(TMEMBER(_scrollOffset));
	persistMgr->transfer(TMEMBER(_shieldWindow));
	persistMgr->transfer(TMEMBER(_spacing));
	persistMgr->transfer(TMEMBER(_waitingScript));
	persistMgr->transfer(TMEMBER(_window));

	persistMgr->transfer(TMEMBER_INT(_verticalAlign));
	persistMgr->transfer(TMEMBER_INT(_align));

	return STATUS_OK;
}


//////////////////////////////////////////////////////////////////////////
ERRORCODE CAdResponseBox::weedResponses() {
	CAdGame *adGame = (CAdGame *)Game;

	for (int i = 0; i < _responses.getSize(); i++) {
		switch (_responses[i]->_responseType) {
		case RESPONSE_ONCE:
			if (adGame->branchResponseUsed(_responses[i]->_iD)) {
				delete _responses[i];
				_responses.removeAt(i);
				i--;
			}
			break;

		case RESPONSE_ONCE_GAME:
			if (adGame->gameResponseUsed(_responses[i]->_iD)) {
				delete _responses[i];
				_responses.removeAt(i);
				i--;
			}
			break;
		default:
			warning("CAdResponseBox::WeedResponses - Unhandled enum");
			break;
		}
	}
	return STATUS_OK;
}


//////////////////////////////////////////////////////////////////////////
void CAdResponseBox::setLastResponseText(const char *text, const char *textOrig) {
	CBUtils::setString(&_lastResponseText, text);
	CBUtils::setString(&_lastResponseTextOrig, textOrig);
}


//////////////////////////////////////////////////////////////////////////
ERRORCODE CAdResponseBox::handleResponse(CAdResponse *response) {
	setLastResponseText(response->_text, response->_textOrig);

	CAdGame *adGame = (CAdGame *)Game;

	switch (response->_responseType) {
	case RESPONSE_ONCE:
		adGame->addBranchResponse(response->_iD);
		break;

	case RESPONSE_ONCE_GAME:
		adGame->addGameResponse(response->_iD);
		break;
	default:
		warning("CAdResponseBox::HandleResponse - Unhandled enum");
	}

	return STATUS_OK;
}


//////////////////////////////////////////////////////////////////////////
CBObject *CAdResponseBox::getNextAccessObject(CBObject *currObject) {
	CBArray<CUIObject *, CUIObject *> objects;
	getObjects(objects, true);

	if (objects.getSize() == 0) return NULL;
	else {
		if (currObject != NULL) {
			for (int i = 0; i < objects.getSize(); i++) {
				if (objects[i] == currObject) {
					if (i < objects.getSize() - 1) return objects[i + 1];
					else break;
				}
			}
		}
		return objects[0];
	}
	return NULL;
}

//////////////////////////////////////////////////////////////////////////
CBObject *CAdResponseBox::getPrevAccessObject(CBObject *currObject) {
	CBArray<CUIObject *, CUIObject *> objects;
	getObjects(objects, true);

	if (objects.getSize() == 0) return NULL;
	else {
		if (currObject != NULL) {
			for (int i = objects.getSize() - 1; i >= 0; i--) {
				if (objects[i] == currObject) {
					if (i > 0) return objects[i - 1];
					else break;
				}
			}
		}
		return objects[objects.getSize() - 1];
	}
	return NULL;
}

//////////////////////////////////////////////////////////////////////////
ERRORCODE CAdResponseBox::getObjects(CBArray<CUIObject *, CUIObject *> &objects, bool interactiveOnly) {
	for (int i = 0; i < _respButtons.getSize(); i++) {
		objects.add(_respButtons[i]);
	}
	if (_window) _window->getWindowObjects(objects, interactiveOnly);

	return STATUS_OK;
}

} // end of namespace WinterMute
