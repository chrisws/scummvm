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

#include "common/scummsys.h"
#include "common/config-manager.h"
#include "common/util.h"
#include "mads/mads.h"
#include "mads/screen.h"
#include "mads/msurface.h"
#include "mads/staticres.h"
#include "mads/nebular/dialogs_nebular.h"

namespace MADS {

namespace Nebular {

CopyProtectionDialog::CopyProtectionDialog(MADSEngine *vm, bool priorAnswerWrong):
		TextDialog(vm, FONT_INTERFACE, Common::Point(-1, -1), 32) {
	getHogAnusEntry(_hogEntry);

	if (priorAnswerWrong) {
		addLine("ANSWER INCORRECT!", true);
		wordWrap("\n");
		addLine("(But we'll give you another chance!)");
	} else {
		addLine("REX NEBULAR version 8.43", true);
		wordWrap("\n");
		addLine("(Copy Protection, for your convenience)");
	}
	wordWrap("\n");

	wordWrap("Now comes the part that everybody hates.  But if we don't");
	wordWrap("do this, nasty rodent-like people will pirate this game");
	wordWrap("and a whole generation of talented designers, programmers,");
	wordWrap("artists, and playtesters will go hungry, and will wander");
	wordWrap("aimlessly through the land at night searching for peace.");
	wordWrap("So let's grit our teeth and get it over with.  Just get");
	
	Common::String line = "out your copy of ";
	line += _hogEntry._bookId == 103 ? "the GAME MANUAL" : "REX'S LOGBOOK";
	line += ".  See!  That was easy.  ";
	wordWrap(line);

	line = Common::String::format("Next, just turn to page %d. On line %d, find word number %d, ",
		_hogEntry._pageNum, _hogEntry._lineNum, _hogEntry._wordNum);
	wordWrap(line);

	wordWrap("and type it on the line below (we',27h,'ve even given you");
	wordWrap("first letter as a hint).  As soon as you do that, we can get");
	wordWrap("right into this really COOL adventure game!\n");
	wordWrap("\n");
	wordWrap("                    ");
	addInput();
	wordWrap("\n");
}

bool CopyProtectionDialog::show() {
	draw();
	_vm->_events->showCursor();

	// TODO: Replace with text input
	while (!_vm->shouldQuit() && !_vm->_events->isKeyPressed() &&
			!_vm->_events->_mouseClicked) {
		_vm->_events->delay(1);
	}

	_vm->_events->_pendingKeys.clear();
	return true;
}

bool CopyProtectionDialog::getHogAnusEntry(HOGANUS &entry) {
	File f;
	f.open("*HOGANUS.DAT");

	// Read in the total number of entries, and randomly pick an entry to use
	int numEntries = f.readUint16LE();
	int entryIndex = _vm->getRandomNumber(1, numEntries);

	// Read in the encrypted entry
	f.seek(28 * entryIndex + 2);
	byte entryData[28];
	f.read(entryData, 28);

	// Decrypt it
	for (int i = 0; i < 28; ++i)
		entryData[i] = ~entryData[i];

	// Fill out the fields
	entry._bookId = entryData[0];
	entry._pageNum = READ_LE_UINT16(&entryData[2]);
	entry._lineNum = READ_LE_UINT16(&entryData[4]);
	entry._wordNum = READ_LE_UINT16(&entryData[6]);
	entry._word = Common::String((char *)&entryData[8]);

	f.close();
	return true;
}

/*------------------------------------------------------------------------*/

bool DialogsNebular::show(int msgId) {
	MADSAction &action = _vm->_game->_scene._action;
	Common::StringArray msg = _vm->_game->getMessage(msgId);
	Common::String title;
	Common::String commandText;
	Common::String valStr;
	Common::String dialogText;
	bool result = true;
	bool centerFlag = false;
	bool underlineFlag = false;
	bool commandFlag = false;
	bool crFlag = false;
	TextDialog *dialog = nullptr;
	_dialogWidth = 17; 
	_capitalizationMode = kUppercase;

	// Loop through the lines of the returned text
	for (uint idx = 0; idx < msg.size(); ++idx) {
		Common::String srcLine = msg[idx];
		const char *srcP = srcLine.c_str();

		// Loop through the text of the line
		while (srcP < srcLine.c_str() + srcLine.size()) {
			if (*srcP == '[') {
				// Starting a command
				commandText = "";
				commandFlag = true;
			} else if (*srcP == ']') {
				// Ending a command
				if (commandFlag) {
					if (commandCheck("CENTER", valStr, commandText)) {
						centerFlag = true;
					} else if (commandCheck("TITLE", valStr, commandText)) {
						centerFlag = true;
						underlineFlag = true;
						crFlag = true;
						int v = atoi(valStr.c_str());
						if (v != 0)
							_dialogWidth = v;
					} else if (commandCheck("CR", valStr, commandText)) {
						if (centerFlag) {
							crFlag = true;
						} else {
							dialog = new TextDialog(_vm, FONT_INTERFACE, _defaultPosition, _dialogWidth);
							dialog->wordWrap(dialogText);
							dialog->incNumLines();
						}
					} else if (commandCheck("ASK", valStr, commandText)) {
						dialog->addInput();
					} else if (commandCheck("VERB", valStr, commandText)) {
						dialogText += getVocab(action._activeAction._verbId);
					} else if (commandCheck("INDEX", valStr, commandText)) {
						int idx = atoi(valStr.c_str());
						if (_indexList[idx])
							dialogText += getVocab(_indexList[idx]);
					} else if (commandCheck("NUMBER", valStr, commandText)) {
						int idx = atoi(valStr.c_str());
						dialogText += Common::String::format("%.4d", _indexList[idx]);
					} else if (commandCheck("NOUN1", valStr, commandText)) {
						if (!textNoun(dialogText, 1, valStr))
							dialogText += getVocab(action._activeAction._objectNameId);
					} else if (commandCheck("NOUN2", valStr, commandText)) {
						if (!textNoun(dialogText, 2, valStr))
							dialogText += getVocab(action._activeAction._indirectObjectId);
					} else if (commandCheck("PREP", valStr, commandText)) {
						dialogText += kArticleList[action._savedFields._articleNumber];
					} else if (commandCheck("SENTENCE", valStr, commandText)) {
						dialogText += action._sentence;
					} else if (commandCheck("WIDTH", valStr, commandText)) {
						_dialogWidth = atoi(valStr.c_str());
					} else if (commandCheck("BAR", valStr, commandText)) {
						dialog->addBarLine();
					} else if (commandCheck("UNDER", valStr, commandText)) {
						underlineFlag = true;
					} else if (commandCheck("DOWN", valStr, commandText)) {
						dialog->downPixelLine();
					} else if (commandCheck("TAB", valStr, commandText)) {
						int xp = atoi(valStr.c_str());
						dialog->setLineXp(xp);
					}
				}

				commandFlag = false;
			} else if (commandFlag) {
				// Add the next character to the command
				commandText += *srcP;
			} else {
				// Add to the text to be displayed in the dialog
				dialogText += *srcP;
			}

			++srcP;
		}

		if (!dialog) {
			dialog = new TextDialog(_vm, FONT_INTERFACE, _defaultPosition, _dialogWidth);
		}

		if (centerFlag) {
			dialog->addLine(dialogText, underlineFlag);
			if (crFlag)
				dialog->incNumLines();
		} else {
			dialog->wordWrap(dialogText);
		}

		// Reset line processing flags in preparation for next line
		dialogText = "";
		commandFlag = false;
		underlineFlag = false;
		centerFlag = false;
		crFlag = false;
	}

	if (!centerFlag)
		dialog->incNumLines();
	
	// Show the dialog
	_vm->_events->setCursor(CURSOR_ARROW);
	dialog->show();

	delete dialog;
	return result;
}

Common::String DialogsNebular::getVocab(int vocabId) {
	assert(vocabId > 0);

	Common::String vocab = _vm->_game->_scene.getVocab(vocabId);

	switch (_capitalizationMode) {
	case kUppercase:
		vocab.toUppercase();
		break;
	case kLowercase:
		vocab.toLowercase();
		break;
	case kUpperAndLower:
		vocab.toLowercase();
		vocab.setChar(toupper(vocab[0]), 0);
	default:
		break;
	}

	return vocab;
}

bool DialogsNebular::textNoun(Common::String &dialogText, int nounNum,
		const Common::String &valStr) {
	warning("TODO: textNoun");
	return false;
}

bool DialogsNebular::commandCheck(const char *idStr, Common::String &valStr,
		const Common::String &command) {
	uint idLen = strlen(idStr);
	
	valStr = (command.size() <= idLen) ? "" : Common::String(command.c_str() + idLen);

	// Check whether the command starts with the given Id
	int result = scumm_strnicmp(idStr, command.c_str(), idLen) == 0;
	if (!result)
		return false;

	// It does, so set the command case mode
	if (Common::isUpper(command[0]) && Common::isUpper(command[1])) {
		_capitalizationMode = kUppercase;
	} else if (Common::isUpper(command[0])) {
		_capitalizationMode = kUpperAndLower;
	} else {
		_capitalizationMode = kLowercase;
	}

	return true;
}


} // End of namespace Nebular

} // End of namespace MADS
