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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#include "config.h"
#include "backends/platform/bada/system.h"
#include "backends/platform/bada/fs.h"

#define BUFFER_SIZE 1024

// internal BADA paths
#define PATH_ROOT        "/"
#define PATH_HOME        "/data"
#define PATH_HOME_X      "/data/"

//
// BadaFileStream
//
class BadaFileStream :
	public Common::SeekableReadStream,
	public Common::WriteStream,
	public Common::NonCopyable {
public:
	static BadaFileStream *makeFromPath(const String &path, bool writeMode);

	BadaFileStream(File *file, bool writeMode);
	~BadaFileStream();

	bool err() const;
	void clearErr();
	bool eos() const;

	uint32 write(const void *dataPtr, uint32 dataSize);
	bool flush();

	int32 pos() const;
	int32 size() const;
	bool seek(int32 offs, int whence = SEEK_SET);
	uint32 read(void *dataPtr, uint32 dataSize);

private:
	byte _buffer[BUFFER_SIZE];
	uint32 _bufferIndex;
	uint32 _bufferLength;
	bool _writeMode;
	File *_file;
};

BadaFileStream::BadaFileStream(File *ioFile, bool writeMode) :
	_bufferIndex(0),
	_bufferLength(0),
	_writeMode(writeMode),
	_file(ioFile) {
	AppAssert(ioFile != 0);
}

BadaFileStream::~BadaFileStream() {
	if (_file) {
		if (_writeMode) {
			flush();
		}
		delete _file;
	}
}

bool BadaFileStream::err() const {
	result r = GetLastResult();
	return (r != E_SUCCESS && r != E_END_OF_FILE);
}

void BadaFileStream::clearErr() {
	SetLastResult(E_SUCCESS);
}

bool BadaFileStream::eos() const {
	return (_bufferLength - _bufferIndex == 0) && (GetLastResult() == E_END_OF_FILE);
}

int32 BadaFileStream::pos() const {
	return _file->Tell() - (_bufferLength - _bufferIndex);
}

int32 BadaFileStream::size() const {
	int32 oldPos = _file->Tell();
	_file->Seek(FILESEEKPOSITION_END, 0);

	int32 length = _file->Tell();
	SetLastResult(_file->Seek(FILESEEKPOSITION_BEGIN, oldPos));

	return length;
}

bool BadaFileStream::seek(int32 offs, int whence) {
	bool result = false;
	switch (whence) {
	case SEEK_SET:
		// set from start of file
		SetLastResult(_file->Seek(FILESEEKPOSITION_BEGIN, offs));
		result = (E_SUCCESS == GetLastResult());
		break;

	case SEEK_CUR:
		// set relative to offs
		if (_bufferIndex < _bufferLength && _bufferIndex > (uint32)-offs) {
			// re-position within the buffer
			SetLastResult(E_SUCCESS);
			_bufferIndex += offs;
			return true;
		} else {
			offs -= (_bufferLength - _bufferIndex);
			if (offs < 0 && _file->Tell() + offs < 0) {
				// avoid negative positioning
				offs = 0;
			}
			if (offs != 0) {
				SetLastResult(_file->Seek(FILESEEKPOSITION_CURRENT, offs));
				result = (E_SUCCESS == GetLastResult());
			} else {
				result = true;
			}
		}
		break;

	case SEEK_END:
		// set relative to end - positive will increase the file size
		SetLastResult(_file->Seek(FILESEEKPOSITION_END, offs));
		result = (E_SUCCESS == GetLastResult());
		break;

	default:
		AppLog("Invalid whence %d", whence);
		return false;
	}

	if (!result) {
		AppLog("seek failed");
	}

	_bufferIndex = _bufferLength = 0;
	return result;
}

uint32 BadaFileStream::read(void *ptr, uint32 len) {
	uint32 result = 0;
	if (!eos()) {
		if (_bufferIndex < _bufferLength) {
			// use existing buffer
			uint32 available = _bufferLength - _bufferIndex;
			if (len <= available) {
				// use allocation
				memcpy((byte *)ptr, &_buffer[_bufferIndex], len);
				_bufferIndex += len;
				result = len;
			} else {
				// use remaining allocation
				memcpy((byte *)ptr, &_buffer[_bufferIndex], available);
				uint32 remaining = len - available;
				result = available;

				if (remaining) {
					result += _file->Read(((byte *)ptr) + available, remaining);
				}
				_bufferIndex = _bufferLength = 0;
			}
		} else if (len < BUFFER_SIZE) {
			// allocate and use buffer
			_bufferIndex = 0;
			_bufferLength = _file->Read(_buffer, BUFFER_SIZE);
			if (_bufferLength) {
				if (_bufferLength < len) {
					len = _bufferLength;
				}
				memcpy((byte *)ptr, _buffer, len);
				result = _bufferIndex = len;
			}
		} else {
			result = _file->Read((byte *)ptr, len);
			_bufferIndex = _bufferLength = 0;
		}
	} else {
		AppLog("Attempted to read past EOS");
	}
	return result;
}

uint32 BadaFileStream::write(const void *ptr, uint32 len) {
	result r = _file->Write(ptr, len);
	SetLastResult(r);
	return (r == E_SUCCESS ? len : 0);
}

bool BadaFileStream::flush() {
  logEntered();
	SetLastResult(_file->Flush());
	return (E_SUCCESS == GetLastResult());
}

BadaFileStream *BadaFileStream::makeFromPath(const String &path, bool writeMode) {
	File *ioFile = new File();

	String filePath = path;
	if (writeMode && (path[0] != '.' && path[0] != '/')) {
		filePath.Insert(PATH_HOME_X, 0);
	}

	AppLog("Open file %S", filePath.GetPointer());

	result r = ioFile->Construct(filePath, writeMode ? L"w" : L"r", writeMode);
	if (r == E_SUCCESS) {
		return new BadaFileStream(ioFile, writeMode);
	}

	AppLog("Failed to open file");
	delete ioFile;
	return 0;
}

//
// converts a bada (wchar) String into a scummVM (char) string
//
Common::String fromString(const Tizen::Base::String &in) {
	ByteBuffer *buf = StringUtil::StringToUtf8N(in);
	Common::String result((const char*)buf->GetPointer());
	delete buf;

	return result;
}

//
// BadaFilesystemNode
//
BadaFilesystemNode::BadaFilesystemNode(const Common::String &nodePath) {
	AppAssert(nodePath.size() > 0);
	init(nodePath);
}

BadaFilesystemNode::BadaFilesystemNode(const Common::String &root, const Common::String &nodePath) {
	// Make sure the string contains no slashes
	AppAssert(!nodePath.contains('/'));

	// We assume here that path is already normalized (hence don't bother to
	// call Common::normalizePath on the final path).
	Common::String newPath(root);
	if (root.lastChar() != '/') {
		newPath += '/';
	}
	newPath += nodePath;

	init(newPath);
}

void BadaFilesystemNode::init(const Common::String &nodePath) {
	// Normalize the path (that is, remove unneeded slashes etc.)
	_path = Common::normalizePath(nodePath, '/');
	_displayName = Common::lastPathComponent(_path, '/');

	StringUtil::Utf8ToString(_path.c_str(), _unicodePath);
	_isVirtualDir = (_path == PATH_ROOT ||
                   _path == PATH_HOME);
	_isValid = _isVirtualDir || !IsFailed(File::GetAttributes(_unicodePath, _attr));
}

bool BadaFilesystemNode::exists() const {
	return _isValid;
}

bool BadaFilesystemNode::isReadable() const {
	return _isVirtualDir || _isValid;
}

bool BadaFilesystemNode::isDirectory() const {
	return _isVirtualDir || (_isValid && _attr.IsDirectory());
}

bool BadaFilesystemNode::isWritable() const {
	bool result = (_isValid && !_isVirtualDir && !_attr.IsDirectory() && !_attr.IsReadOnly());
	if (_path == PATH_HOME) {
		result = true;
	}
	return result;
}

AbstractFSNode *BadaFilesystemNode::getChild(const Common::String &n) const {
	AppAssert(!_path.empty());
	AppAssert(isDirectory());
	return new BadaFilesystemNode(_path, n);
}

bool BadaFilesystemNode::getChildren(AbstractFSList &myList, ListMode mode, bool hidden) const {
	AppAssert(isDirectory());

	bool result = false;

	if (_isVirtualDir && mode != Common::FSNode::kListFilesOnly) {
		// present well known BADA file system areas
		if (_path == PATH_ROOT) {
			myList.push_back(new BadaFilesystemNode(PATH_HOME));
			result = true; // no more entries
		}
	}

	if (!result) {
		DirEnumerator *pDirEnum = 0;
		Directory *pDir = new Directory();

		// open directory
		if (IsFailed(pDir->Construct(_unicodePath))) {
			AppLog("Failed to open directory: %S", _unicodePath.GetPointer());
		} else {
			// read all directory entries
			pDirEnum = pDir->ReadN();
			if (pDirEnum) {
				result = true;
			}

			// loop through all directory entries
			while (pDirEnum && pDirEnum->MoveNext() == E_SUCCESS) {
				DirEntry dirEntry = pDirEnum->GetCurrentDirEntry();

				// skip 'invisible' files if necessary
				Tizen::Base::String fileName = dirEntry.GetName();

				if (fileName[0] == '.' && !hidden) {
					continue;
				}

				// skip '.' and '..' to avoid cycles
				if (fileName == L"." || fileName == L"..") {
					continue;
				}

				// Honor the chosen mode
				if ((mode == Common::FSNode::kListFilesOnly && dirEntry.IsDirectory()) ||
						(mode == Common::FSNode::kListDirectoriesOnly && !dirEntry.IsDirectory())) {
					continue;
				}
				myList.push_back(new BadaFilesystemNode(_path, fromString(fileName)));
			}
		}

		// cleanup
		if (pDirEnum) {
			delete pDirEnum;
		}

		// close the opened directory
		if (pDir) {
			delete pDir;
		}
	}

	return result;
}

AbstractFSNode *BadaFilesystemNode::getParent() const {
  logEntered();
	if (_path == PATH_ROOT) {
		return 0; // The filesystem root has no parent
	}

	const char *start = _path.c_str();
	const char *end = start + _path.size();

	// Strip of the last component. We make use of the fact that at this
	// point, path is guaranteed to be normalized
	while (end > start && *(end-1) != '/') {
		end--;
	}

	if (end == start) {
		// This only happens if we were called with a relative path, for which
		// there simply is no parent.
		// TODO: We could also resolve this by assuming that the parent is the
		//			 current working directory, and returning a node referring to that.
		return 0;
	}

	return new BadaFilesystemNode(Common::String(start, end));
}

Common::SeekableReadStream *BadaFilesystemNode::createReadStream() {
	Common::SeekableReadStream *result = BadaFileStream::makeFromPath(_unicodePath, false);
	if (result != NULL) {
		_isValid = !IsFailed(File::GetAttributes(_unicodePath, _attr));
	}
	return result;
}

Common::WriteStream *BadaFilesystemNode::createWriteStream() {
	Common::WriteStream *result = BadaFileStream::makeFromPath(_unicodePath, true);
	if (result != NULL) {
		_isValid = !IsFailed(File::GetAttributes(_unicodePath, _attr));
	}
	return result;
}
