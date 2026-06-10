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

#include "common/system.h"
#include "common/fs.h"

#include "director/director.h"
#include "director/util.h"
#include "director/lingo/lingo.h"
#include "director/lingo/lingo-object.h"
#include "director/lingo/lingo-utils.h"
#include "director/lingo/xtras/f/filextra4.h"

/**************************************************
 *
 * USED IN:
 * Loewenzahn 1 (D10, v2.0)
 * TKKG 11 / 13 / 14
 * Oscar the Balloonist Flies into the Mountains
 *
 **************************************************/

/*
-- xtra FileXtra4
new object me
fx_GetVersion object me
fx_FileOpenDialog object me, string initialFolder, string filtStr, string dlogTitle, Boolean createPrompt, Boolean fileMustExist
fx_FileSaveAsDialog object me, string initialFolder, string filename, string dlogTitle, Boolean overwritePrompt
fx_FileExists object me, string fileName
fx_FileIsLink object me, string fileName
fx_FileRename object me, string oldName, string newName
fx_FileDelete object me, string fileName
fx_FileRecycle object me, string fileName
fx_FileCopy object me, string fromFName, string toFName
fx_FileMove object me, string fromFName, string toFName
fx_FileGetWriteState object me, string fileName
fx_FileSetWriteState object me, string fileName, Boolean writeable
fx_FileGetModDate object me, string fileName
fx_FileGetModNumber object me, string fileName
fx_FileGetSize object me, string fileName
fx_FileGetType object me, string fileName
fx_FileSetType object me, string fileName, string fileType
fx_FileCompare object me, string filename1, string filename2
fx_FileOpenDocument object me, string filename
fx_FilePrintDocument object me, string filename
fx_FileGetAppPath object me, string fileType
fx_FileRunApp object me, string commandLine
fx_LinkCreate object me, string filename, string destPath
fx_LinkResolve object me, string filename
fx_FolderSelectDialog object me, string infoString
fx_FolderGetSpecialPath object me, string folderType
fx_FolderExists object me, string folderName
fx_FolderCreate object me, string folderName
fx_FolderRename object me, string oldFolderName, string newFolderName
fx_FolderDelete object me, string folderName, Boolean recursive
fx_FolderRecycle object me, string folderName
fx_FolderCopy object me, string fromFolderName, string toFolderName, Boolean recursive
fx_FolderMove object me, string fromFolderName, string toFolderName
fx_FolderGetWriteState object me, string folderName
fx_FolderSetWriteState object me, string folderName, Boolean writeable, Boolean recursive
fx_FolderSyncOneWay object me, string fromFolderName, string toFolderName, Boolean recursive, Boolean deleteStrays
fx_FolderSyncBothWays object me, string fromFolderName, string toFolderName, Boolean recursive
fx_FolderToList object me, string folderName
fx_VolumeSelectDialog object me, string infoString
fx_VolumeExists object me, string volumeName
fx_VolumeGetFreeBytes object me, string volumeName
fx_VolumeGetTotalBytes object me, string volumeName
fx_VolumeIsCDROM object me, string volumeName
fx_VolumeIsRemovable object me, string volumeName
fx_VolumeEject object me, string volumeName
fx_VolumesToList object me
fx_ErrorNumber object me
fx_ErrorString object me

 */

namespace Director {

const char *FileXtra4Xtra::xlibName = "FileXtra4";
const XlibFileDesc FileXtra4Xtra::fileNames[] = {
	{ "filextra4",   nullptr },
	{ nullptr,        nullptr },
};

static MethodProto xlibMethods[] = {
	{ "new",				FileXtra4Xtra::m_new,		 0, 0,	500 },
	{ "fx_GetVersion",				FileXtra4Xtra::m_fx_GetVersion,		 0, 0,	500 },
	{ "fx_FileOpenDialog",				FileXtra4Xtra::m_fx_FileOpenDialog,		 5, 5,	500 },
	{ "fx_FileSaveAsDialog",				FileXtra4Xtra::m_fx_FileSaveAsDialog,		 4, 4,	500 },
	{ "fx_FileExists",				FileXtra4Xtra::m_fx_FileExists,		 1, 1,	500 },
	{ "fx_FileIsLink",				FileXtra4Xtra::m_fx_FileIsLink,		 1, 1,	500 },
	{ "fx_FileRename",				FileXtra4Xtra::m_fx_FileRename,		 2, 2,	500 },
	{ "fx_FileDelete",				FileXtra4Xtra::m_fx_FileDelete,		 1, 1,	500 },
	{ "fx_FileRecycle",				FileXtra4Xtra::m_fx_FileRecycle,		 1, 1,	500 },
	{ "fx_FileCopy",				FileXtra4Xtra::m_fx_FileCopy,		 2, 2,	500 },
	{ "fx_FileMove",				FileXtra4Xtra::m_fx_FileMove,		 2, 2,	500 },
	{ "fx_FileGetWriteState",				FileXtra4Xtra::m_fx_FileGetWriteState,		 1, 1,	500 },
	{ "fx_FileSetWriteState",				FileXtra4Xtra::m_fx_FileSetWriteState,		 2, 2,	500 },
	{ "fx_FileGetModDate",				FileXtra4Xtra::m_fx_FileGetModDate,		 1, 1,	500 },
	{ "fx_FileGetModNumber",				FileXtra4Xtra::m_fx_FileGetModNumber,		 1, 1,	500 },
	{ "fx_FileGetSize",				FileXtra4Xtra::m_fx_FileGetSize,		 1, 1,	500 },
	{ "fx_FileGetType",				FileXtra4Xtra::m_fx_FileGetType,		 1, 1,	500 },
	{ "fx_FileSetType",				FileXtra4Xtra::m_fx_FileSetType,		 2, 2,	500 },
	{ "fx_FileCompare",				FileXtra4Xtra::m_fx_FileCompare,		 2, 2,	500 },
	{ "fx_FileOpenDocument",				FileXtra4Xtra::m_fx_FileOpenDocument,		 1, 1,	500 },
	{ "fx_FilePrintDocument",				FileXtra4Xtra::m_fx_FilePrintDocument,		 1, 1,	500 },
	{ "fx_FileGetAppPath",				FileXtra4Xtra::m_fx_FileGetAppPath,		 1, 1,	500 },
	{ "fx_FileRunApp",				FileXtra4Xtra::m_fx_FileRunApp,		 1, 1,	500 },
	{ "fx_LinkCreate",				FileXtra4Xtra::m_fx_LinkCreate,		 2, 2,	500 },
	{ "fx_LinkResolve",				FileXtra4Xtra::m_fx_LinkResolve,		 1, 1,	500 },
	{ "fx_FolderSelectDialog",				FileXtra4Xtra::m_fx_FolderSelectDialog,		 1, 1,	500 },
	{ "fx_FolderGetSpecialPath",				FileXtra4Xtra::m_fx_FolderGetSpecialPath,		 1, 1,	500 },
	{ "fx_FolderExists",				FileXtra4Xtra::m_fx_FolderExists,		 1, 1,	500 },
	{ "fx_FolderCreate",				FileXtra4Xtra::m_fx_FolderCreate,		 1, 1,	500 },
	{ "fx_FolderRename",				FileXtra4Xtra::m_fx_FolderRename,		 2, 2,	500 },
	{ "fx_FolderDelete",				FileXtra4Xtra::m_fx_FolderDelete,		 2, 2,	500 },
	{ "fx_FolderRecycle",				FileXtra4Xtra::m_fx_FolderRecycle,		 1, 1,	500 },
	{ "fx_FolderCopy",				FileXtra4Xtra::m_fx_FolderCopy,		 3, 3,	500 },
	{ "fx_FolderMove",				FileXtra4Xtra::m_fx_FolderMove,		 2, 2,	500 },
	{ "fx_FolderGetWriteState",				FileXtra4Xtra::m_fx_FolderGetWriteState,		 1, 1,	500 },
	{ "fx_FolderSetWriteState",				FileXtra4Xtra::m_fx_FolderSetWriteState,		 3, 3,	500 },
	{ "fx_FolderSyncOneWay",				FileXtra4Xtra::m_fx_FolderSyncOneWay,		 4, 4,	500 },
	{ "fx_FolderSyncBothWays",				FileXtra4Xtra::m_fx_FolderSyncBothWays,		 3, 3,	500 },
	{ "fx_FolderToList",				FileXtra4Xtra::m_fx_FolderToList,		 1, 1,	500 },
	{ "fx_VolumeSelectDialog",				FileXtra4Xtra::m_fx_VolumeSelectDialog,		 1, 1,	500 },
	{ "fx_VolumeExists",				FileXtra4Xtra::m_fx_VolumeExists,		 1, 1,	500 },
	{ "fx_VolumeGetFreeBytes",				FileXtra4Xtra::m_fx_VolumeGetFreeBytes,		 1, 1,	500 },
	{ "fx_VolumeGetTotalBytes",				FileXtra4Xtra::m_fx_VolumeGetTotalBytes,		 1, 1,	500 },
	{ "fx_VolumeIsCDROM",				FileXtra4Xtra::m_fx_VolumeIsCDROM,		 1, 1,	500 },
	{ "fx_VolumeIsRemovable",				FileXtra4Xtra::m_fx_VolumeIsRemovable,		 1, 1,	500 },
	{ "fx_VolumeEject",				FileXtra4Xtra::m_fx_VolumeEject,		 1, 1,	500 },
	{ "fx_VolumesToList",				FileXtra4Xtra::m_fx_VolumesToList,		 0, 0,	500 },
	{ "fx_ErrorNumber",				FileXtra4Xtra::m_fx_ErrorNumber,		 0, 0,	500 },
	{ "fx_ErrorString",				FileXtra4Xtra::m_fx_ErrorString,		 0, 0,	500 },

	{ nullptr, nullptr, 0, 0, 0 }
};

static BuiltinProto xlibBuiltins[] = {

	{ nullptr, nullptr, 0, 0, 0, VOIDSYM }
};

FileXtra4XtraObject::FileXtra4XtraObject(ObjectType ObjectType) :Object<FileXtra4XtraObject>("FileXtra4") {
	_objType = ObjectType;
}

bool FileXtra4XtraObject::hasProp(const Common::String &propName) {
	return (propName == "name");
}

Datum FileXtra4XtraObject::getProp(const Common::String &propName) {
	if (propName == "name")
		return Datum(FileXtra4Xtra::xlibName);
	warning("FileXtra4Xtra::getProp: unknown property '%s'", propName.c_str());
	return Datum();
}

void FileXtra4Xtra::open(ObjectType type, const Common::Path &path) {
    FileXtra4XtraObject::initMethods(xlibMethods);
    FileXtra4XtraObject *xobj = new FileXtra4XtraObject(type);
    if (type == kXtraObj) {
        g_lingo->_openXtras.push_back(xlibName);
		g_lingo->_openXtraObjects.push_back(xobj);
	}
    g_lingo->exposeXObject(xlibName, xobj);
    g_lingo->initBuiltIns(xlibBuiltins);
}

void FileXtra4Xtra::close(ObjectType type) {
    FileXtra4XtraObject::cleanupMethods();
    g_lingo->_globalvars[xlibName] = Datum();

}

void FileXtra4Xtra::m_new(int nargs) {
	g_lingo->printSTUBWithArglist("FileXtra4Xtra::m_new", nargs);
	g_lingo->dropStack(nargs);
	g_lingo->push(g_lingo->_state->me);
}

// Pop all nargs arguments and return the first one (deepest on the stack) as a
// string. FileXtra4 methods take "object me" implicitly (in _state->me), so the
// declared args are the ones on the stack.
static Common::String fx4FirstArg(int nargs) {
	Common::String first;
	for (int i = 0; i < nargs; i++) {
		Datum d = g_lingo->pop();
		if (i == nargs - 1)
			first = d.asString();
	}
	return first;
}

// Resolve a (possibly absolute "x:\...") FileXtra4 path to an absolute
// filesystem path under the game directory. Empty if it does not resolve.
static Common::Path fx4ResolveAbs(const Common::String &raw, bool directory) {
	Common::Path rel = findPath(raw, true, true, directory);
	if (rel.empty())
		return Common::Path();
	Common::Path abs = Common::Path(g_director->getGameDataDir()->getPath());
	abs.appendInPlace(Common::String(g_director->_dirSeparator), g_director->_dirSeparator);
	abs.appendInPlace(rel);
	return abs;
}

void FileXtra4Xtra::m_fx_GetVersion(int nargs) {
	g_lingo->dropStack(nargs);
	g_lingo->push(Datum(Common::String("FileXtra 4.0.2 (ScummVM)")));
}
XOBJSTUB(FileXtra4Xtra::m_fx_FileOpenDialog, 0)
XOBJSTUB(FileXtra4Xtra::m_fx_FileSaveAsDialog, 0)
void FileXtra4Xtra::m_fx_FileExists(int nargs) {
	Common::String name = fx4FirstArg(nargs);
	// findPath() resolves absolute "x:\..." paths against the game directory
	// (the drive letter is stripped by convertPath), so a CD path like
	// "c:\TKKG11.exe" maps to the game root. Non-empty result => the file exists.
	g_lingo->push(Datum(findPath(name, true, true, false).empty() ? 0 : 1));
}
XOBJSTUB(FileXtra4Xtra::m_fx_FileIsLink, 0)
// PDF: returns True (1) on success. ScummVM sandboxes the filesystem (real
// renames happen through the save system), so report success without mutating.
XOBJSTUB(FileXtra4Xtra::m_fx_FileRename, 1)
XOBJSTUB(FileXtra4Xtra::m_fx_FileDelete, 0)
XOBJSTUB(FileXtra4Xtra::m_fx_FileRecycle, 0)
XOBJSTUB(FileXtra4Xtra::m_fx_FileCopy, 0)
XOBJSTUB(FileXtra4Xtra::m_fx_FileMove, 0)
XOBJSTUB(FileXtra4Xtra::m_fx_FileGetWriteState, 0)
XOBJSTUB(FileXtra4Xtra::m_fx_FileSetWriteState, 0)
XOBJSTUB(FileXtra4Xtra::m_fx_FileGetModDate, 0)
XOBJSTUB(FileXtra4Xtra::m_fx_FileGetModNumber, 0)
XOBJSTUB(FileXtra4Xtra::m_fx_FileGetSize, 0)
void FileXtra4Xtra::m_fx_FileGetType(int nargs) {
	Common::String name = fx4FirstArg(nargs);
	// PDF (Windows): return the file's extension with the leading period intact,
	// e.g. ".png" or ".txt"; the empty string on error / no extension.
	Common::String ext;
	for (int i = (int)name.size() - 1; i >= 0; i--) {
		char c = name[i];
		if (c == '\\' || c == '/' || c == ':')
			break;
		if (c == '.') {
			ext = Common::String(name.c_str() + i);
			break;
		}
	}
	g_lingo->push(Datum(ext));
}
XOBJSTUB(FileXtra4Xtra::m_fx_FileSetType, 0)
XOBJSTUB(FileXtra4Xtra::m_fx_FileCompare, 0)
XOBJSTUB(FileXtra4Xtra::m_fx_FileOpenDocument, 0)
XOBJSTUB(FileXtra4Xtra::m_fx_FilePrintDocument, 0)
XOBJSTUB(FileXtra4Xtra::m_fx_FileGetAppPath, 0)
XOBJSTUB(FileXtra4Xtra::m_fx_FileRunApp, 0)
XOBJSTUB(FileXtra4Xtra::m_fx_LinkCreate, 0)
XOBJSTUB(FileXtra4Xtra::m_fx_LinkResolve, 0)
XOBJSTUB(FileXtra4Xtra::m_fx_FolderSelectDialog, 0)
void FileXtra4Xtra::m_fx_FolderGetSpecialPath(int nargs) {
	g_lingo->dropStack(nargs);
	// PDF: returns the path to a "special" folder (preferences, documents, temp,
	// ...). We map every type to the game data directory: it is guaranteed to
	// exist (so fx_FolderExists() agrees and the games' folder-wait loops
	// terminate), and FileIO routes writes there into the ScummVM save system
	// regardless of the path prefix.
	Common::Path base = g_director->getGameDataDir()->getPath();
	Common::String path = base.toString(g_director->_dirSeparator);
	if (!path.empty() && path.lastChar() != g_director->_dirSeparator)
		path += g_director->_dirSeparator;
	g_lingo->push(Datum(path));
}
void FileXtra4Xtra::m_fx_FolderExists(int nargs) {
	Common::String name = fx4FirstArg(nargs);
	g_lingo->push(Datum(findPath(name, true, true, true /* directory */).empty() ? 0 : 1));
}
// PDF: returns True (1) on success. ScummVM does not create real directories
// (the save system handles persistence); the games ignore the result anyway.
XOBJSTUB(FileXtra4Xtra::m_fx_FolderCreate, 1)
XOBJSTUB(FileXtra4Xtra::m_fx_FolderRename, 0)
XOBJSTUB(FileXtra4Xtra::m_fx_FolderDelete, 0)
XOBJSTUB(FileXtra4Xtra::m_fx_FolderRecycle, 0)
XOBJSTUB(FileXtra4Xtra::m_fx_FolderCopy, 0)
XOBJSTUB(FileXtra4Xtra::m_fx_FolderMove, 0)
XOBJSTUB(FileXtra4Xtra::m_fx_FolderGetWriteState, 0)
XOBJSTUB(FileXtra4Xtra::m_fx_FolderSetWriteState, 0)
XOBJSTUB(FileXtra4Xtra::m_fx_FolderSyncOneWay, 0)
XOBJSTUB(FileXtra4Xtra::m_fx_FolderSyncBothWays, 0)
void FileXtra4Xtra::m_fx_FolderToList(int nargs) {
	Common::String name = fx4FirstArg(nargs);

	Datum result;
	result.type = ARRAY;
	result.u.farr = new FArray();

	Common::Path dirPath = fx4ResolveAbs(name, true);
	if (!dirPath.empty()) {
		Common::FSNode dir(dirPath);
		Common::FSList fslist;
		if (dir.isDirectory() && dir.getChildren(fslist, Common::FSNode::kListAll)) {
			for (auto &node : fslist) {
				Common::String childName = node.getName();
				if (node.isDirectory())
					childName += g_director->_dirSeparator;
				result.u.farr->arr.push_back(Datum(childName));
			}
		}
	}
	g_lingo->push(result);
}
XOBJSTUB(FileXtra4Xtra::m_fx_VolumeSelectDialog, 0)
void FileXtra4Xtra::m_fx_VolumeExists(int nargs) {
	g_lingo->dropStack(nargs);
	// The game data acts as the single available volume.
	g_lingo->push(Datum(1));
}
XOBJSTUB(FileXtra4Xtra::m_fx_VolumeGetFreeBytes, 0)
XOBJSTUB(FileXtra4Xtra::m_fx_VolumeGetTotalBytes, 0)
void FileXtra4Xtra::m_fx_VolumeIsCDROM(int nargs) {
	g_lingo->dropStack(nargs);
	// ScummVM exposes no real CD-ROM drive. Returning 0 makes the games'
	// CD-detection loops fall through to their local/"c:" fallback, which then
	// succeeds via fx_FileExists against the (drive-stripped) game directory.
	g_lingo->push(Datum(0));
}
void FileXtra4Xtra::m_fx_VolumeIsRemovable(int nargs) {
	g_lingo->dropStack(nargs);
	// PDF: True (1) for removable media (incl. CD-ROM). Unlike fx_VolumeIsCDROM()
	// (which TKKG uses to decide save-path redirection, where 0 is wanted), the
	// only observed use of this method is OscarMountain's clsCDCheck, where it is
	// a *lenient acceptance* path: a non-CD volume that is "removable" and of type
	// Hard/Removable is still accepted as the game disk. Returning 1 can only help
	// the CD check pass and never causes rejection.
	g_lingo->push(Datum(1));
}
XOBJSTUB(FileXtra4Xtra::m_fx_VolumeEject, 0)
void FileXtra4Xtra::m_fx_VolumesToList(int nargs) {
	g_lingo->dropStack(nargs);
	Datum result;
	result.type = ARRAY;
	result.u.farr = new FArray();
	result.u.farr->arr.push_back(Datum(Common::String("c:\\")));
	g_lingo->push(result);
}
void FileXtra4Xtra::m_fx_ErrorNumber(int nargs) {
	g_lingo->dropStack(nargs);
	g_lingo->push(Datum(0));
}

void FileXtra4Xtra::m_fx_ErrorString(int nargs) {
	g_lingo->dropStack(nargs);
	g_lingo->push(Datum(Common::String("")));
}

}
