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
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#ifndef BADA_FILESYSTEM_FACTORY_H
#define BADA_FILESYSTEM_FACTORY_H

#include "config.h"
#include "backends/fs/fs-factory.h"

/**
 * Creates BadaFilesystemNode objects.
 *
 * Parts of this class are documented in the base interface class, FilesystemFactory.
 */
class BadaFilesystemFactory : public FilesystemFactory {
  AbstractFSNode *makeRootFileNode() const;
  AbstractFSNode *makeCurrentDirectoryFileNode() const;
  AbstractFSNode *makeFileNodePath(const Common::String &path) const;
};

#endif /*BADA_FILESYSTEM_FACTORY_H*/
