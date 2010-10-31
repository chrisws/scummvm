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
 * $URL$
 * $Id$
 *
 */

#include "sci/sci.h"
#include "sci/resource.h"
#include "sci/engine/seg_manager.h"
#include "sci/engine/script.h"
#include "sci/engine/state.h"
#include "sci/engine/selector.h"
#include "sci/engine/kernel.h"

#include "common/file.h"

namespace Sci {

// Loads arbitrary resources of type 'restype' with resource numbers 'resnrs'
// This implementation ignores all resource numbers except the first one.
reg_t kLoad(EngineState *s, int argc, reg_t *argv) {
	ResourceType restype = g_sci->getResMan()->convertResType(argv[0].toUint16());
	int resnr = argv[1].toUint16();

	// Request to dynamically allocate hunk memory for later use
	if (restype == kResourceTypeMemory)
		return s->_segMan->allocateHunkEntry("kLoad()", resnr);

	return make_reg(0, ((restype << 11) | resnr)); // Return the resource identifier as handle
}

// Unloads an arbitrary resource of type 'restype' with resource numbber 'resnr'
//  behaviour of this call didn't change between sci0->sci1.1 parameter wise, which means getting called with
//  1 or 3+ parameters is not right according to sierra sci
reg_t kUnLoad(EngineState *s, int argc, reg_t *argv) {
	if (argc >= 2) {
		ResourceType restype = g_sci->getResMan()->convertResType(argv[0].toUint16());
		reg_t resnr = argv[1];

		// WORKAROUND for a broken script in room 320 in Castle of Dr. Brain.
		// Script 377 tries to free the hunk memory allocated for the saved area
		// (underbits) beneath the pop up window, which results in having the
		// window stay on screen even when it's closed. Ignore this request here.
		if (restype == kResourceTypeMemory && g_sci->getGameId() == GID_CASTLEBRAIN &&
			s->currentRoomNumber() == 320)
			return s->r_acc;

		if (restype == kResourceTypeMemory)
			s->_segMan->freeHunkEntry(resnr);
	}

	return s->r_acc;
}

reg_t kLock(EngineState *s, int argc, reg_t *argv) {
	int state = argc > 2 ? argv[2].toUint16() : 1;
	ResourceType type = g_sci->getResMan()->convertResType(argv[0].toUint16());
	ResourceId id = ResourceId(type, argv[1].toUint16());

	Resource *which;

	switch (state) {
	case 1 :
		g_sci->getResMan()->findResource(id, 1);
		break;
	case 0 :
		if (id.getNumber() == 0xFFFF) {
			// Unlock all resources of the requested type
			Common::List<ResourceId> *resources = g_sci->getResMan()->listResources(type);
			Common::List<ResourceId>::iterator itr = resources->begin();

			while (itr != resources->end()) {
				Resource *res = g_sci->getResMan()->testResource(*itr);
				if (res->isLocked())
					g_sci->getResMan()->unlockResource(res);
				++itr;
			}

			delete resources;
		} else {
			which = g_sci->getResMan()->findResource(id, 0);

			if (which)
				g_sci->getResMan()->unlockResource(which);
			else {
				if (id.getType() == kResourceTypeInvalid)
					warning("[resMan] Attempt to unlock resource %i of invalid type %i", id.getNumber(), type);
				else
					// Happens in CD games (e.g. LSL6CD) with the message
					// resource. It isn't fatal, and it's usually caused
					// by leftover scripts.
					debugC(2, kDebugLevelResMan, "[resMan] Attempt to unlock non-existant resource %s", id.toString().c_str());
			}
		}
		break;
	}
	return s->r_acc;
}

reg_t kResCheck(EngineState *s, int argc, reg_t *argv) {
	Resource *res = NULL;
	ResourceType restype = g_sci->getResMan()->convertResType(argv[0].toUint16());

	if (restype == kResourceTypeVMD) {
		char fileName[10];
		sprintf(fileName, "%d.vmd", argv[1].toUint16());
		return make_reg(0, Common::File::exists(fileName));
	}

	if ((restype == kResourceTypeAudio36) || (restype == kResourceTypeSync36)) {
		if (argc >= 6) {
			uint noun = argv[2].toUint16() & 0xff;
			uint verb = argv[3].toUint16() & 0xff;
			uint cond = argv[4].toUint16() & 0xff;
			uint seq = argv[5].toUint16() & 0xff;

			res = g_sci->getResMan()->testResource(ResourceId(restype, argv[1].toUint16(), noun, verb, cond, seq));
		}
	} else {
		res = g_sci->getResMan()->testResource(ResourceId(restype, argv[1].toUint16()));
	}

	return make_reg(0, res != NULL);
}

reg_t kClone(EngineState *s, int argc, reg_t *argv) {
	reg_t parentAddr = argv[0];
	const Object *parentObj = s->_segMan->getObject(parentAddr);
	reg_t cloneAddr;
	Clone *cloneObj; // same as Object*

	if (!parentObj) {
		error("Attempt to clone non-object/class at %04x:%04x failed", PRINT_REG(parentAddr));
		return NULL_REG;
	}

	debugC(2, kDebugLevelMemory, "Attempting to clone from %04x:%04x", PRINT_REG(parentAddr));

	uint16 infoSelector = readSelectorValue(s->_segMan, parentAddr, SELECTOR(_info_));
	cloneObj = s->_segMan->allocateClone(&cloneAddr);

	if (!cloneObj) {
		error("Cloning %04x:%04x failed-- internal error", PRINT_REG(parentAddr));
		return NULL_REG;
	}

	// In case the parent object is a clone itself we need to refresh our
	// pointer to it here. This is because calling allocateClone might
	// invalidate all pointers, references and iterators to data in the clones
	// segment.
	//
	// The reason why it might invalidate those is, that the segment code
	// (Table) uses Common::Array for internal storage. Common::Array now
	// might invalidate references to its contained data, when it has to
	// extend the internal storage size.
	if (infoSelector & kInfoFlagClone)
		parentObj = s->_segMan->getObject(parentAddr);

	*cloneObj = *parentObj;

	// Mark as clone
	infoSelector &= ~kInfoFlagClass; // remove class bit
	writeSelectorValue(s->_segMan, cloneAddr, SELECTOR(_info_), infoSelector | kInfoFlagClone);

	cloneObj->setSpeciesSelector(cloneObj->getPos());
	if (parentObj->isClass())
		cloneObj->setSuperClassSelector(parentObj->getPos());
	s->_segMan->getScript(parentObj->getPos().segment)->incrementLockers();
	s->_segMan->getScript(cloneObj->getPos().segment)->incrementLockers();

	return cloneAddr;
}

extern void _k_view_list_mark_free(EngineState *s, reg_t off);

reg_t kDisposeClone(EngineState *s, int argc, reg_t *argv) {
	reg_t obj = argv[0];
	Clone *object = s->_segMan->getObject(obj);

	if (!object) {
		error("Attempt to dispose non-class/object at %04x:%04x",
		         PRINT_REG(obj));
		return s->r_acc;
	}

	// SCI uses this technique to find out, if it's a clone and if it's supposed to get freed
	//  At least kq4early relies on this behaviour. The scripts clone "Sound", then set bit 1 manually
	//  and call kDisposeClone later. In that case we may not free it, otherwise we will run into issues
	//  later, because kIsObject would then return false and Sound object wouldn't get checked.
	uint16 infoSelector = readSelectorValue(s->_segMan, obj, SELECTOR(_info_));
	if ((infoSelector & 3) == kInfoFlagClone)
		object->markAsFreed();

	return s->r_acc;
}

// Returns script dispatch address index in the supplied script
reg_t kScriptID(EngineState *s, int argc, reg_t *argv) {
	int script = argv[0].toUint16();
	uint16 index = (argc > 1) ? argv[1].toUint16() : 0;

	if (argv[0].segment)
		return argv[0];

	SegmentId scriptSeg = s->_segMan->getScriptSegment(script, SCRIPT_GET_LOAD);

	if (!scriptSeg)
		return NULL_REG;

	Script *scr = s->_segMan->getScript(scriptSeg);

	if (!scr->getExportsNr()) {
		// This is normal. Some scripts don't have a dispatch (exports) table,
		// and this call is probably used to load them in memory, ignoring
		// the return value. If only one argument is passed, this call is done
		// only to load the script in memory. Thus, don't show any warning,
		// as no return value is expected. If an export is requested, then
		// it will most certainly fail with OOB access.
		if (argc == 2)
			error("Script 0x%x does not have a dispatch table and export %d "
					"was requested from it", script, index);
		return NULL_REG;
	}

	if (index > scr->getExportsNr()) {
		error("Dispatch index too big: %d > %d", index, scr->getExportsNr());
		return NULL_REG;
	}

	uint16 address = scr->validateExportFunc(index);

	// Point to the heap for SCI1.1+ games
	if (getSciVersion() >= SCI_VERSION_1_1)
		address += scr->getScriptSize();

	return make_reg(scriptSeg, address);
}

reg_t kDisposeScript(EngineState *s, int argc, reg_t *argv) {
	int script = argv[0].offset;

	SegmentId id = s->_segMan->getScriptSegment(script);
	Script *scr = s->_segMan->getScriptIfLoaded(id);
	if (scr) {
		if (s->_executionStack.back().addr.pc.segment != id)
			scr->setLockers(1);
	}

	s->_segMan->uninstantiateScript(script);

	if (argc != 2) {
		return s->r_acc;
	} else {
		// This exists in the KQ5CD and GK1 interpreter. We know it is used
		// when GK1 starts up, before the Sierra logo.
		warning("kDisposeScript called with 2 parameters, still untested");
		return argv[1];
	}
}

reg_t kIsObject(EngineState *s, int argc, reg_t *argv) {
	if (argv[0].offset == SIGNAL_OFFSET) // Treated specially
		return NULL_REG;
	else
		return make_reg(0, s->_segMan->isHeapObject(argv[0]));
}

reg_t kRespondsTo(EngineState *s, int argc, reg_t *argv) {
	reg_t obj = argv[0];
	int selector = argv[1].toUint16();

	return make_reg(0, s->_segMan->isHeapObject(obj) && lookupSelector(s->_segMan, obj, selector, NULL, NULL) != kSelectorNone);
}

} // End of namespace Sci
