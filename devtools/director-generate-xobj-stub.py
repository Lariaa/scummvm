#!/usr/bin/env python

from __future__ import annotations

import argparse
import os
import re
import struct
from typing import Any, BinaryIO, Literal
from typing_extensions import TypedDict

XCodeType = Literal["XFCN", "XCMD", "XObject", "Xtra"]


class XCode(TypedDict):
    type: XCodeType
    name: str
    slug: str
    filename: str
    method_table: list[str]

class PESection(TypedDict):
    name: str
    virt_size: int
    virt_addr: int
    raw_size: int
    raw_ptr: int


DIRECTOR_SRC_PATH = os.path.abspath(
    os.path.join(__file__, "..", "..", "engines", "director")
)
MAKEFILE_PATH = os.path.join(DIRECTOR_SRC_PATH, "module.mk")
LINGO_XLIBS_PATH = os.path.join(DIRECTOR_SRC_PATH, "lingo", "xlibs")
LINGO_XTRAS_PATH = os.path.join(DIRECTOR_SRC_PATH, "lingo", "xtras")
LINGO_OBJECT_PATH = os.path.join(DIRECTOR_SRC_PATH, "lingo", "lingo-object.cpp")

LEGAL = """/* ScummVM - Graphic Adventure Engine
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
"""

TEMPLATE_H = (
    LEGAL
    + """
#ifndef DIRECTOR_LINGO_{base_upper}_{slug_upper_alpha}_{slug_upper}_H
#define DIRECTOR_LINGO_{base_upper}_{slug_upper_alpha}_{slug_upper}_H

namespace Director {{

class {xobject_class} : public Object<{xobject_class}> {{
public:
	{xobject_class}(ObjectType objType);{xtra_props_h}
}};

namespace {xobj_class} {{

extern const char *xlibName;
extern const XlibFileDesc fileNames[];

void open(ObjectType type, const Common::Path &path);
void close(ObjectType type);

{methlist}

}} // End of namespace {xobj_class}

}} // End of namespace Director

#endif
"""
)

TEMPLATE_HEADER_METH = """void m_{methname}(int nargs);"""

TEMPLATE = (
    LEGAL
    + """
#include "common/system.h"

#include "director/director.h"
#include "director/lingo/lingo.h"
#include "director/lingo/lingo-object.h"
#include "director/lingo/lingo-utils.h"
#include "director/lingo/{base}/{slug_alpha}/{slug}.h"

/**************************************************
 *
 * USED IN:
 * [insert game here]
 *
 **************************************************/

/*
{xmethtable}
 */

namespace Director {{

const char *{xobj_class}::xlibName = "{name}";
const XlibFileDesc {xobj_class}::fileNames[] = {{
	{{ "{filename}",   nullptr }},
	{{ nullptr,        nullptr }},
}};

static MethodProto xlibMethods[] = {{
{xlib_methods}
{xlib_toplevels}
	{{ nullptr, nullptr, 0, 0, 0 }}
}};

static BuiltinProto xlibBuiltins[] = {{
{xlib_builtins}
	{{ nullptr, nullptr, 0, 0, 0, VOIDSYM }}
}};

{xobject_class}::{xobject_class}(ObjectType ObjectType) :Object<{xobject_class}>("{name}") {{
	_objType = ObjectType;
}}{xtra_props}

void {xobj_class}::open(ObjectType type, const Common::Path &path) {{
    {xobject_class}::initMethods(xlibMethods);
    {xobject_class} *xobj = new {xobject_class}(type);
    if (type == kXtraObj) {{
        g_lingo->_openXtras.push_back(xlibName);
		g_lingo->_openXtraObjects.push_back(xobj);
	}}
    g_lingo->exposeXObject(xlibName, xobj);
    g_lingo->initBuiltIns(xlibBuiltins);
}}

void {xobj_class}::close(ObjectType type) {{
    {xobject_class}::cleanupMethods();
    g_lingo->_globalvars[xlibName] = Datum();

}}

{xobj_new}

{xobj_stubs}

}}
"""
)
XLIB_METHOD_TEMPLATE = """	{{ "{methname}",				{xobj_class}::m_{methname},		 {min_args}, {max_args},	{director_version} }},"""
XLIB_NEW_TEMPLATE = """void {xobj_class}::m_new(int nargs) {{
	g_lingo->printSTUBWithArglist("{xobj_class}::m_new", nargs);
	g_lingo->dropStack(nargs);
	g_lingo->push(g_lingo->_state->me);
}}"""


# XTRA PROPS TEMPLATE and Header contains extra newline at the beginning.
# This keeps the newlines correct when `TEMPLATE` is used for xlibs.
XTRA_PROPS_TEMPLATE = """

bool {xobject_class}::hasProp(const Common::String &propName) {{
	return (propName == "name");
}}

Datum {xobject_class}::getProp(const Common::String &propName) {{
	if (propName == "name")
		return Datum({xobj_class}::xlibName);
	warning("{xobj_class}::getProp: unknown property '%s'", propName.c_str());
	return Datum();
}}"""

XTRA_PROPS_H = """

	bool hasProp(const Common::String &propName) override;
	Datum getProp(const Common::String &propName) override;"""


XCMD_TEMPLATE_H = (
    LEGAL
    + """
#ifndef DIRECTOR_LINGO_XLIBS_{slug_upper_alpha}_{slug_upper}_H
#define DIRECTOR_LINGO_XLIBS_{slug_upper_alpha}_{slug_upper}_H

namespace Director {{

namespace {xobj_class} {{

extern const char *xlibName;
extern const XlibFileDesc fileNames[];

void open(ObjectType type, const Common::Path &path);
void close(ObjectType type);

{methlist}

}} // End of namespace {xobj_class}

}} // End of namespace Director

#endif
"""
)

XCMD_TEMPLATE = (
    LEGAL
    + """
#include "common/system.h"

#include "director/director.h"
#include "director/lingo/lingo.h"
#include "director/lingo/lingo-object.h"
#include "director/lingo/lingo-utils.h"
#include "director/lingo/xlibs/{slug_alpha}/{slug}.h"

/**************************************************
 *
 * USED IN:
 * [insert game here]
 *
 **************************************************/

namespace Director {{

const char *{xobj_class}::xlibName = "{name}";
const XlibFileDesc {xobj_class}::fileNames[] = {{
	{{ "{filename}", nullptr }},
	{{ nullptr, nullptr }}
}};

static BuiltinProto builtins[] = {{
{xlib_builtins}
	{{ nullptr, nullptr, 0, 0, 0, VOIDSYM }}
}};

void {xobj_class}::open(ObjectType type, const Common::Path &path) {{
	g_lingo->initBuiltIns(builtins);
}}

void {xobj_class}::close(ObjectType type) {{
	g_lingo->cleanupBuiltIns(builtins);
}}

{xobj_stubs}

}}
"""
)

BUILTIN_TEMPLATE = """	{{ "{name}", {xobj_class}::m_{name}, {min_args}, {max_args}, {director_version}, {methtype} }},"""

XOBJ_STUB_TEMPLATE = """XOBJSTUB({xobj_class}::m_{methname}, {default})"""

XOBJ_NR_STUB_TEMPLATE = """XOBJSTUBNR({xobj_class}::m_{methname})"""


def read_uint8(data: bytes) -> int:
    return struct.unpack("B", data)[0]


def read_uint16_le(data: bytes) -> int:
    return struct.unpack("<H", data)[0]


def read_uint16_be(data: bytes) -> int:
    return struct.unpack(">H", data)[0]


def read_uint32_le(data: bytes) -> int:
    return struct.unpack("<L", data)[0]


def read_uint32_be(data: bytes) -> int:
    return struct.unpack(">L", data)[0]


def inject_makefile(slug: str, xcode_type: XCodeType) -> None:
    make_contents = open(MAKEFILE_PATH, "r").readlines()
    slug_alpha = slug[:1]
    storage_path = f"lingo/xtras" if xcode_type == "Xtra" else f"lingo/xlibs"
    expr = re.compile(f"^\t{storage_path}/([a-zA-Z0-9\\-]+).o( \\\\|)")
    obj = f"{slug_alpha}/{slug}"
    for i in range(len(make_contents)):
        m = expr.match(make_contents[i])
        if m:
            if slug == m.group(1):
                # file already in makefile
                print(f"{obj}.o already in {MAKEFILE_PATH}, skipping")
                return
            elif slug < m.group(1):
                make_contents.insert(i, f"\t{storage_path}/{obj}.o \\\n")
                with open(MAKEFILE_PATH, "w") as f:
                    f.writelines(make_contents)
                return
            elif m.group(2) == "":
                # final entry in the list
                make_contents[i] += " \\"
                make_contents.insert(i + 1, f"\t{storage_path}/{obj}.o\n")
                with open(MAKEFILE_PATH, "w") as f:
                    f.writelines(make_contents)
                return


def inject_lingo_object(slug: str, xobj_class: str, director_version: int, xcode_type: XCodeType) -> None:
    # write include statement for the object header
    lo_contents = open(LINGO_OBJECT_PATH, "r").readlines()
    slug_alpha = slug[:1]
    storage_path = f"director/lingo/xtras" if xcode_type == "Xtra" else f"director/lingo/xlibs"
    obj_type = "kXtraObj" if xcode_type == "Xtra" else "kXObj"
    header = f"{slug_alpha}/{slug}"
    expr = re.compile(f'^#include "{storage_path}/([a-zA-Z0-9/\\-]+)\\.h"')
    in_xlibs = False
    for i in range(len(lo_contents)):
        m = expr.match(lo_contents[i])
        if m:
            in_xlibs = True
            if slug == m.group(1):
                print(
                    f"{storage_path}/{header}.h import already in {LINGO_OBJECT_PATH}, skipping"
                )
                break
            elif slug < m.group(1):
                lo_contents.insert(i, f'#include "{storage_path}/{header}.h"\n')
                with open(LINGO_OBJECT_PATH, "w") as f:
                    f.writelines(lo_contents)
                break
        elif in_xlibs:
            # final entry in the list
            lo_contents.insert(i, f'#include "{storage_path}/{header}.h"\n')
            with open(LINGO_OBJECT_PATH, "w") as f:
                f.writelines(lo_contents)
                break

    # write entry in the XLibProto table
    lo_contents = open(LINGO_OBJECT_PATH, "r").readlines()
    expr = re.compile("^\tXLIBDEF\\(([a-zA-Z0-9_]+),")
    in_xlibs = False
    for i in range(len(lo_contents)):
        m = expr.match(lo_contents[i])
        if m:
            in_xlibs = True
            if xobj_class == m.group(1):
                print(
                    f"{xobj_class} proto import already in {LINGO_OBJECT_PATH}, skipping"
                )
                break
            elif xobj_class < m.group(1):
                lo_contents.insert(
                    i,
                    f"	XLIBDEF({xobj_class},			{obj_type},					{director_version}),	// D{director_version // 100}\n",
                )
                with open(LINGO_OBJECT_PATH, "w") as f:
                    f.writelines(lo_contents)
                break
        elif in_xlibs:
            # final entry in the list
            lo_contents.insert(
                i,
                f"	XLIBDEF({xobj_class},			{obj_type},					{director_version}),	// D{director_version // 100}\n",
            )
            with open(LINGO_OBJECT_PATH, "w") as f:
                f.writelines(lo_contents)
                break


def extract_xcode_macbinary(
    file: BinaryIO, resource_offset: int, xobj_id: str | None = None
) -> XCode:
    file.seek(resource_offset)
    resource_data_offset = read_uint32_be(file.read(4))
    resource_map_offset = read_uint32_be(file.read(4))
    resource_data_size = read_uint32_be(file.read(4))
    resource_map_size = read_uint32_be(file.read(4))
    file.seek(resource_offset + resource_map_offset + 24)
    type_list_offset = read_uint16_be(file.read(2))
    name_list_offset = read_uint16_be(file.read(2))
    file.seek(resource_offset + resource_map_offset + type_list_offset)
    type_count = read_uint16_be(file.read(2))
    types = {}
    for _ in range(type_count + 1):
        key = file.read(4)
        types[key] = (read_uint16_be(file.read(2)) + 1, read_uint16_be(file.read(2)))
    xobj: dict[str, dict[str, Any]] = {}
    for chunk_type in [b"XCOD", b"XFCN", b"XCMD"]:
        if chunk_type in types:
            print(f"Found {chunk_type.decode('utf8')} resources!")
            file.seek(
                resource_offset
                + resource_map_offset
                + type_list_offset
                + types[chunk_type][1]
            )
            resources: list[tuple[str, int, int]] = []
            for _ in range(types[chunk_type][0]):
                id = f"{chunk_type.decode('utf8')}_{read_uint16_be(file.read(2))}"
                name_offset = read_uint16_be(file.read(2))
                file.read(1)
                data_offset = (read_uint8(file.read(1)) << 16) + read_uint16_be(
                    file.read(2)
                )
                file.read(4)
                resources.append((id, data_offset, name_offset))
            for id, data_offset, name_offset in resources:
                xobj[id] = {}
                if name_offset != 0xFFFF:
                    file.seek(
                        resource_offset
                        + resource_map_offset
                        + name_list_offset
                        + name_offset
                    )
                    name_size = read_uint8(file.read(1))
                    xobj[id]["name"] = file.read(name_size).decode("macroman")
                else:
                    xobj[id]["name"] = "<unknown>"
                file.seek(resource_offset + resource_data_offset + data_offset)
                xobj[id]["dump"] = file.read(read_uint32_be(file.read(4)) - 4)
                file.seek(resource_offset + resource_data_offset + data_offset)
                size = read_uint32_be(file.read(4)) - 12
                file.read(12)
                xobj[id]["xmethtable"] = []
                while size > 0:
                    count = read_uint8(file.read(1))
                    if count == 0:
                        break
                    xobj[id]["xmethtable"].append(file.read(count).decode("macroman"))
                    size -= 1 + count
    if not xobj:
        raise ValueError("No extension resources found!")

    if xobj_id is None or xobj_id not in xobj:
        print("Please re-run with one of the following resource IDs:")
        for id, data in xobj.items():
            print(f"{id} - {data['name']}")
        raise ValueError("Need to specify resource ID")
    type: XCodeType = (
        "XFCN"
        if xobj_id.startswith("XFCN_")
        else "XCMD"
        if xobj_id.startswith("XCMD_")
        else "XObject"
    )
    if type == "XObject":
        for entry in xobj[xobj_id]["xmethtable"]:
            print(entry)
    slug = xobj[xobj_id]["name"].lower()
    if type in ["XFCN", "XCMD"]:
        slug += type.lower()
    return {
        "type": type,
        "name": xobj[xobj_id]["name"],
        "slug": slug,
        "filename": xobj[xobj_id]["name"],
        "method_table": xobj[xobj_id]["xmethtable"],
    }


def extract_xcode_win16(file: BinaryIO, ne_offset: int) -> XCode:
    # get resource table
    file.seek(ne_offset + 0x24, os.SEEK_SET)
    restable_offset = read_uint16_le(file.read(0x2))
    resident_names_offset = read_uint16_le(file.read(0x2))
    file.seek(ne_offset + restable_offset)
    shift_count = read_uint16_le(file.read(0x2))
    # read each resource
    resources: list[dict[str, Any]] = []
    while file.tell() < ne_offset + resident_names_offset:
        type_id = read_uint16_le(file.read(0x2))  # should be 0x800a for XMETHTABLE
        if type_id == 0:
            break
        count = read_uint16_le(file.read(0x2))
        file.read(0x4)  # reserved
        entries = []
        for i in range(count):
            file_offset = read_uint16_le(file.read(0x2))
            file_length = read_uint16_le(file.read(0x2))
            entries.append(
                dict(
                    offset=file_offset << shift_count, length=file_length << shift_count
                )
            )
            file.read(0x2)  # flagword
            file.read(0x2)  # resource_id
            file.read(0x2)  # handle
            file.read(0x2)  # usage
        resources.append(dict(type_id=type_id, entries=entries))
    resource_names = []
    while file.tell() < ne_offset + resident_names_offset:
        length = read_uint8(file.read(0x1))
        if length == 0:
            break
        resource_names.append(file.read(length).decode("ASCII"))

    print("Resources found:")
    print(resources, resource_names)

    xmethtable_exists = "XMETHTABLE" in resource_names
    file.seek(ne_offset + resident_names_offset)
    name_length = read_uint8(file.read(0x1))
    file_name = file.read(name_length).decode("ASCII")

    # Borland C++ can put the XMETHTABLE token into a weird nonstandard resource
    for x in filter(lambda d: d["type_id"] == 0x800F, resources):
        for y in x["entries"]:
            file.seek(y["offset"], os.SEEK_SET)
            data = file.read(y["length"])
            xmethtable_exists |= b"XMETHTABLE" in data

    if not xmethtable_exists:
        raise ValueError("XMETHTABLE not found!")

    resources = list(filter(lambda x: x["type_id"] == 0x800A, resources))
    if len(resources) != 1:
        raise ValueError("Expected a single matching resource type entry!")

    xmethtable_offset = resources[0]["entries"][0]["offset"]
    xmethtable_length = resources[0]["entries"][0]["length"]
    print(f"Found XMETHTABLE for XObject library {file_name}!")
    file.seek(xmethtable_offset, os.SEEK_SET)
    xmethtable_raw = file.read(xmethtable_length)
    xmethtable = [
        entry.decode("iso-8859-1")
        for entry in xmethtable_raw.strip(b"\x00").split(b"\x00")
    ]
    for entry in xmethtable:
        print(entry)
    library_name = xmethtable[1]
    xmethtable[1] = "--" + library_name
    return {
        "type": "XObject",
        "name": library_name,
        "slug": file_name.lower(),
        "filename": file_name,
        "method_table": xmethtable,
    }


# Argument types a msgTable line can name. "*" is the varargs marker, which for
# handlers that take no fixed arguments is all a line has ("* netStatus * --").
_MSGTABLE_TYPE_KW = (
    r"(object|me|\*|integer|string|point|rect|list|symbol|"
    r"boolean|float|void|any|globalHeap)"
)


def _looks_like_msgtable_chunk(piece: str) -> bool:
    # Multi-line chunks are always table content.
    if "\n" in piece:
        return True
    stripped = piece.strip()
    if not stripped:
        return True
    # A lone terminal line: header, comment, global/xtra handler, or a
    # "<name> <type>..." method signature. Unrelated STRINGTABLE tokens
    # ("None", "High", "is", ...) match none of these.
    if (
        stripped.lower().startswith("xtra ")
        or stripped.startswith("--")
        or stripped[0] in "*+"
    ):
        return True
    return (
        re.match(r"[A-Za-z_]\w*\s+" + _MSGTABLE_TYPE_KW + r"(?!\w)", stripped)
        is not None
    )


def extract_msgtable_from_pe_resources(
    file: BinaryIO, sections: dict[str, PESection]
) -> list[str] | None:
    # Some Xtras (e.g. the PrintOMatic family) do not keep the msgTable as a
    # static C string. Instead they build it at runtime by concatenating a run
    # of Windows STRINGTABLE resources via LoadString(). The first line of that
    # run always starts with "xtra <Name>", so we can reconstruct the table
    # generically from the .rsrc section without disassembling any code.
    if ".rsrc" not in sections:
        return None
    rsrc = sections[".rsrc"]
    rsrc_base = rsrc["virt_addr"]

    def rva_to_offset(rva: int) -> int | None:
        for s in sections.values():
            span = max(s["virt_size"], s["raw_size"])
            if s["virt_addr"] <= rva < s["virt_addr"] + span:
                return s["raw_ptr"] + (rva - s["virt_addr"])
        return None

    def read_at(offset: int, size: int) -> bytes:
        file.seek(offset)
        return file.read(size)

    # Walk the three-level resource directory tree (type / name / language),
    # collecting the data entries for RT_STRING (type id 6).
    string_leaves: list[tuple[int, int, int]] = []  # (name_id, data_rva, size)

    def walk(dir_rva: int, level: int, type_id: int, name_id: int) -> None:
        off = rva_to_offset(rsrc_base + dir_rva)
        if off is None:
            return
        named = read_uint16_le(read_at(off + 12, 2))
        ided = read_uint16_le(read_at(off + 14, 2))
        for i in range(named + ided):
            entry = read_at(off + 16 + i * 8, 8)
            entry_id = read_uint32_le(entry[0:4])
            entry_off = read_uint32_le(entry[4:8])
            if entry_off & 0x80000000:
                sub = entry_off & 0x7FFFFFFF
                if level == 0:
                    walk(sub, 1, entry_id, name_id)
                elif level == 1:
                    walk(sub, 2, type_id, entry_id)
                else:
                    walk(sub, 3, type_id, name_id)
            else:
                leaf_off = rva_to_offset(rsrc_base + entry_off)
                if leaf_off is None:
                    continue
                data_rva = read_uint32_le(read_at(leaf_off, 4))
                size = read_uint32_le(read_at(leaf_off + 4, 4))
                if type_id == 6:  # RT_STRING
                    string_leaves.append((name_id, data_rva, size))

    walk(0, 0, 0, 0)
    if not string_leaves:
        return None

    # Each RT_STRING leaf is a bundle of 16 length-prefixed UTF-16LE strings.
    # String id = (bundle_id - 1) * 16 + index_within_bundle.
    strings: dict[int, str] = {}
    for name_id, data_rva, size in string_leaves:
        data_off = rva_to_offset(data_rva)
        if data_off is None:
            continue
        blob = read_at(data_off, size)
        pos = 0
        base_id = ((name_id & 0xFFFF) - 1) * 16
        for i in range(16):
            if pos + 2 > len(blob):
                break
            length = read_uint16_le(blob[pos : pos + 2])
            pos += 2
            if length:
                text = blob[pos : pos + length * 2].decode("utf-16-le", "replace")
                pos += length * 2
                strings[base_id + i] = text

    # Find the resource id whose string starts the msgTable ("xtra <Name>"),
    # then concatenate the contiguous run of ids that follow it. The runtime
    # LoadString() loop reads consecutive ids, so we stop at the first gap in
    # the id space, or at the first string that no longer looks like msgTable
    # content (some Xtras, e.g. ActiveX, pack unrelated short tokens into the
    # ids immediately after the table with no intervening gap).
    start_id = None
    for sid in sorted(strings):
        if strings[sid].lstrip().lower().startswith("xtra "):
            start_id = sid
            break
    if start_id is None:
        return None

    pieces = []
    sid = start_id
    while sid in strings and _looks_like_msgtable_chunk(strings[sid]):
        pieces.append(strings[sid])
        sid += 1
    methtable_text = "".join(pieces)

    print("Reconstructed msgTable from STRINGTABLE resources "
          f"(ids {start_id}..{sid - 1})")
    # Normalise " \n" separators the resources use into clean table lines.
    return [line.strip() for line in methtable_text.split("\n") if line.strip()]


# A msgTable line is "[+*] name arg1type, arg2type ...", or - for a handler that
# takes nothing at all - just "* name -- comment" with no argument list.
_MSGTABLE_LINE = re.compile(
    r"[+*]?\s*[A-Za-z_]\w*\s+" + _MSGTABLE_TYPE_KW + r"(?!\w)"
    r"|[+*]\s*[A-Za-z_]\w*\s*(--|$)"
)


def _score_msgtable_block(block: str) -> int:
    score = 0
    for line in re.split(r"[\r\n]+", block):
        stripped = line.strip()
        if (
            _MSGTABLE_LINE.match(stripped)
            or stripped.startswith("--")
            or stripped.lower().startswith("xtra ")
            or stripped.lower().startswith("new object")
        ):
            score += 1
    return score


def extract_msgtable_from_data_literal(
    file: BinaryIO, sections: dict[str, PESection]
) -> list[str] | None:
    # The most common reason the .text scan fails is not that the msgTable is
    # missing, but that the register call loads its address with instructions
    # other than the expected "push offset table" (MOV into a register, a
    # different argument order, a different selector...). The table body is
    # still present verbatim as a NUL-terminated ASCII string in a data section.
    #
    # Locate it directly: it always begins with "xtra <Name>", so anchor there
    # and read to the terminating NUL. Anchoring on the substring also copes
    # with a stray prefix byte from the preceding datum ("Za'$xtra MD5 ...").
    #
    # Some builds keep the header ("xtra fileio -- version %s.%s.%s.r%s", the
    # version filled in at runtime) in its own string and the method body in the
    # next one. So also try extending the anchored block over the NUL padding
    # with the strings that follow, while those still look like table text, and
    # keep whichever candidate scores highest.
    file.seek(0)
    data = file.read()
    text = data.decode("iso-8859-1")

    best_block = None
    best_score = 0
    for match in re.finditer(r"xtra[ \t]+[A-Za-z_]\w*", text):
        start = match.start()
        nul = text.find("\x00", start)
        end = nul if nul >= 0 else len(text)
        candidates = [text[start:end]]

        extended = candidates[0]
        pos = end
        for _ in range(64):
            while pos < len(text) and text[pos] == "\x00":
                pos += 1
            nxt_nul = text.find("\x00", pos)
            nxt_end = nxt_nul if nxt_nul >= 0 else len(text)
            nxt = text[pos:nxt_end]
            if not nxt.strip():
                # blank filler between the header and the body: skip, don't stop
                pos = nxt_end
                continue
            if _score_msgtable_block(nxt) < 1:
                break
            extended += "\n" + nxt
            candidates.append(extended)
            pos = nxt_end

        for candidate in candidates:
            score = _score_msgtable_block(candidate)
            if score > best_score:
                best_score = score
                best_block = candidate

    # Last resort: a few Xtras (e.g. ZipXtra) keep the body nowhere near the
    # header, so nothing was reachable from the anchor. Take the best-scoring
    # string in the binary as the body and pair it with the header. This is only
    # used when the anchored search came up empty, and the gate below still
    # requires a real header plus enough table lines.
    if best_score < 3:
        body = None
        body_score = 0
        body_off = -1
        for match in re.finditer(rb"[\x09\x0a\x0d\x20-\x7e]{8,}", data):
            block = match.group(0).decode("iso-8859-1")
            score = _score_msgtable_block(block)
            if score > body_score:
                body_score, body, body_off = score, block, match.start()
        if body and body_score >= 2:
            # Prefer the header closest in front of the body, but accept one
            # sitting after it too (ZipXtra stores it well past the table).
            header_line = None
            header_dist = None
            for match in re.finditer(r"xtra[ \t]+[A-Za-z_]\w*[^\x00\r\n]{0,80}", text):
                dist = abs(match.start() - body_off)
                if header_dist is None or dist < header_dist:
                    header_line = match.group(0).strip()
                    header_dist = dist
            if header_line and not body.lstrip().lower().startswith("xtra "):
                best_block = header_line + "\n" + body
                best_score = _score_msgtable_block(best_block)

    # Require a real "xtra <Name>" header plus a few table-like lines, so we
    # don't latch onto an unrelated string that merely contains the word "xtra".
    if not best_block or best_score < 3:
        return None

    lines = [line.strip() for line in re.split(r"[\r\n]+", best_block) if line.strip()]
    if not lines[0].lower().startswith("xtra "):
        return None
    print(f"Recovered msgTable literal from data section (score {best_score})")
    return lines


def extract_xcode_win32(file: BinaryIO, pe_offset: int) -> XCode:
    file.seek(pe_offset + 4)

    # read the COFF Header, perform basic sanity checks
    machine_type = read_uint16_le(file.read(0x2))
    if machine_type != 0x14c:
        raise ValueError(f"PE file is not 32-bit Intel x86")
    section_count = read_uint16_le(file.read(0x2))
    file.seek(12, os.SEEK_CUR)
    optional_size = read_uint16_le(file.read(0x2))
    characteristics = read_uint16_le(file.read(0x2))
    if not (characteristics & 0x2000):
        raise ValueError("DLL flag not set")
    if not (characteristics & 0x0100):
        raise ValueError("32-bit flag not set")

    # read the Optional Header to get the image base address
    optional = file.read(optional_size)
    image_base = 0
    if read_uint16_le(optional[0:2]) == 0x10b:
        image_base = read_uint32_le(optional[28:32])
        print(f"Found PE32, image base {image_base:08x}")
    elif read_uint16_le(optional[0:2]) == 0x20b:
        raise ValueError("PE32+ not supported")
    else:
        raise ValueError("Unknown optional header magic number")

    # read each Section Header from the Section Table
    sections: dict[str, PESection] = {}
    for i in range(section_count):
        segment: PESection = {
            "name": file.read(0x8).strip(b'\x00').decode('utf8'),
            "virt_size": read_uint32_le(file.read(0x4)),
            "virt_addr": read_uint32_le(file.read(0x4)),
            "raw_size": read_uint32_le(file.read(0x4)),
            "raw_ptr": read_uint32_le(file.read(0x4)),
        }
        file.seek(16, os.SEEK_CUR)
        sections[segment["name"]] = segment
        print(f"{segment['name']}: {segment['virt_addr']:08x} {segment['virt_size']:08x}")

    # grab the .text section; this contains the program instructions
    if ".text" not in sections:
        raise ValueError(".text section not found")
    file.seek(sections[".text"]["raw_ptr"])
    code = file.read(sections[".text"]["raw_size"])

    # Lingo Xtras are COM libraries with a generic calling API.
    # Director discovers what functions are available by requesting
    # a msgTable, which unfortunately for us is done with code.

    # Search for the basic case of passing xtra_methtable as a static C string.
    # We need to find the following x86 assembly:
    # 68 [ u32 addr 1 ]  ; push offset "msgTable"
    # 6a 00              ; push 0
    # 68 [ u32 addr 2 ]  ; push offset xtra_methtable
    # 6a 09              ; push 9

    instr = re.compile(rb"\x68(.{4})\x6a\x00\x68(.{4})\x6a\x09", flags=re.DOTALL)
    methtable_found = False
    methtable = []
    for msgtable_raw, methtable_raw in instr.findall(code):
        # should be the offset to the string "msgTable"
        msgtable_offset = read_uint32_le(msgtable_raw) - image_base
        # should be the offset to the full method table
        methtable_offset = read_uint32_le(methtable_raw) - image_base
        msgtable_found = False
        for s in sections.values():
            if msgtable_offset in range(s["virt_addr"], s["virt_addr"]+s["virt_size"]):
                file.seek(s["raw_ptr"])
                data = file.read(s["raw_size"])
                start = msgtable_offset - s["virt_addr"]
                end = data.find(b"\x00", start)
                if data[start:end] != b"msgTable":
                    continue
                print(f"Found msgTable!")
                msgtable_found = True
        if not msgtable_found:
            continue
        # If we found the text "msgTable" at the first address, we know we've found the right call.
        for s in sections.values():
            if methtable_offset in range(s["virt_addr"], s["virt_addr"]+s["virt_size"]):
                file.seek(s["raw_ptr"])
                data = file.read(s["raw_size"])
                start = methtable_offset - s["virt_addr"]
                end = data.find(b"\x00", start)
                methtable_found = True
                methtable = data[start:end].decode('iso-8859-1').split('\n')

    if not methtable_found:
        # The .text scan didn't match. Fall back to locating the table without
        # relying on the shape of the register call: first as a verbatim
        # NUL-terminated literal in a data section (the common case), then
        # rebuilt from STRINGTABLE resources (LoadString-based Xtras like
        # PrintOMatic).
        methtable = extract_msgtable_from_data_literal(file, sections)
        if not methtable:
            methtable = extract_msgtable_from_pe_resources(file, sections)
        if not methtable:
            raise ValueError("Could not find msgTable! You may have to copy the Xtra into real Director, run \"put mMessageList(xtra(\"xtraName\"))\" in the message window, then copy the output to a text file.")

    for entry in methtable:
        print(entry)

    library_name = methtable[0].split()[1].capitalize()
    methtable[0] = "-- " + methtable[0]

    return {
        "type": "Xtra",
        "name": library_name,
        "slug": library_name.lower(),
        "filename": library_name.lower(),
        "method_table": methtable
    }


def extract_xcode_textfile(file: BinaryIO) -> XCode:
    # For Xtras, it is entirely possible for the msgTable to be
    # generated at runtime. In these unlucky cases, your only option
    # is to load the Xtra into real Director and run
    #
    # put mMessageList(xtra("xtraName"))
    #
    # then manually copy and save the output to a text file encoded as UTF8.

    file.seek(0)

    # skip past the useless marker Microsoft Notepad appends to UTF8 files
    if file.read(3) != b"\xef\xbb\xbf":
        file.seek(0)

    data = file.read().decode("utf8")
    separator = "\r\n" if "\r\n" in data else "\n"

    methtable = data.split(separator)

    library_name = methtable[0].split()[1].capitalize()
    methtable[0] = "-- " + methtable[0]

    return {
        "type": "Xtra",
        "name": library_name,
        "slug": library_name.lower(),
        "filename": library_name.lower(),
        "method_table": methtable
    }


def extract_xcode(path: str, resid: str) -> XCode:
    with open(path, "rb") as file:
        magic = file.read(0x2)
        if magic == b"MZ":
            file.seek(0x3C, os.SEEK_SET)
            header_offset = read_uint16_le(file.read(0x2))
            file.seek(header_offset, os.SEEK_SET)
            magic = file.read(0x2)
            if magic == b"NE":
                print("Found Win16 NE DLL!")
                return extract_xcode_win16(file, header_offset)
            elif magic == b"PE":
                print("Found Win32 PE DLL!")
                return extract_xcode_win32(file, header_offset)
        file.seek(0)
        header = file.read(124)
        if (
            len(header) == 124
            and header[0] == 0
            and header[74] == 0
            and header[82] == 0
            and header[122] in [129, 130]
            and header[123] in [129, 130]
        ):
            print("Found MacBinary!")

            data_size = read_uint32_be(header[83:87])
            resource_size = read_uint32_be(header[87:91])
            resource_offset = (
                128
                + data_size
                + ((128 - (data_size % 128)) if (data_size % 128) else 0)
            )
            print(f"resource offset: {resource_offset}")
            return extract_xcode_macbinary(file, resource_offset, resid)
        if path.endswith(".txt"):
            # there's probably a more legit way of checking for text files
            print("Found text file!")
            return extract_xcode_textfile(file)
    raise ValueError("Unknown filetype")


def generate_xobject_stubs(
    xmethtable: list[str],
    slug: str,
    name: str,
    filename: str,
    director_version: int = 400,
    dry_run: bool = False,
) -> None:
    meths = []
    for e in xmethtable:
        if not e.strip():
            break
        elems = e.split()
        if not elems or elems[0].startswith("--"):
            continue

        first = elems[0]
        if first.startswith("/"):
            first = first[1:]
        returnval = first[0]
        args = first[1:]
        methname = elems[1].split(",")[0]
        if methname.startswith("+"):
            methname = methname[1:]
        if methname.startswith("m"):
            methname = methname[1].lower() + methname[2:]
        meths.append(
            dict(
                methname=methname,
                args=args,
                min_args=len(args),
                max_args=len(args),
                returnval=returnval,
                default='""' if returnval == "S" else "0",
            )
        )
    xobject_class = f"{name}XObject"
    xobj_class = f"{name}XObj"

    slug_alpha = slug[:1]

    cpp_text = TEMPLATE.format(
        base="xlibs",
        slug=slug,
        slug_alpha=slug_alpha,
        name=name,
        filename=filename,
        xmethtable="\n".join(xmethtable),
        xobject_class=xobject_class,
        xobj_class=xobj_class,
        xlib_builtins="",
        xlib_toplevels="",
        xlib_methods="\n".join(
            [
                XLIB_METHOD_TEMPLATE.format(
                    xobj_class=xobj_class, director_version=director_version, **x
                )
                for x in meths
            ]
        ),
        xtra_props="",
		xobj_new=XLIB_NEW_TEMPLATE.format(xobj_class=xobj_class),
        xobj_stubs="\n".join(
            [
                XOBJ_NR_STUB_TEMPLATE.format(xobj_class=xobj_class, **x)
                if x["returnval"] == "X"
                else XOBJ_STUB_TEMPLATE.format(xobj_class=xobj_class, **x)
                for x in meths
                if x["methname"] != "new"
            ]
        ),
    )
    if dry_run:
        print("C++ output:")
        print(cpp_text)
        print()
    else:
        os.makedirs(os.path.join(LINGO_XLIBS_PATH, f"{slug_alpha}"), exist_ok=True)
        with open(os.path.join(LINGO_XLIBS_PATH, f"{slug_alpha}/{slug}.cpp"), "w") as cpp:
            cpp.write(cpp_text)

    header_text = TEMPLATE_H.format(
        base_upper="XLIBS",
        slug_upper=slug.upper(),
        slug_upper_alpha=slug.upper()[:1],
        xobject_class=xobject_class,
        xobj_class=xobj_class,
        xtra_props_h="",
        methlist="\n".join([TEMPLATE_HEADER_METH.format(**x) for x in meths]),
    )
    if dry_run:
        print("Header output:")
        print(header_text)
        print()
    else:
        os.makedirs(os.path.join(LINGO_XLIBS_PATH, f"{slug_alpha}"), exist_ok=True)
        with open(os.path.join(LINGO_XLIBS_PATH, f"{slug_alpha}/{slug}.h"), "w") as header:
            header.write(header_text)

    if not dry_run:
        inject_makefile(slug, "XObject")
        inject_lingo_object(slug, xobj_class, director_version, "XObject")


def generate_xcmd_stubs(
    type: Literal["XCMD", "XFCN"],
    slug: str,
    name: str,
    filename: str,
    director_version: int = 400,
    dry_run: bool = False,
) -> None:
    xobj_class = f"{name}{type}"
    methtype = "CBLTIN" if type == "XCMD" else "HBLTIN"
    slug_alpha = slug[:1]
    cpp_text = XCMD_TEMPLATE.format(
        slug=slug,
        slug_alpha=slug_alpha,
        name=name,
        filename=filename,
        xobj_class=xobj_class,
        xlib_builtins=BUILTIN_TEMPLATE.format(
            name=name,
            xobj_class=xobj_class,
            min_args=-1,
            max_args=0,
            director_version=director_version,
            methtype=methtype,
        ),
        xobj_stubs=XOBJ_STUB_TEMPLATE.format(
            xobj_class=xobj_class, methname=name, default=0
        ),
    )
    if dry_run:
        print("C++ output:")
        print(cpp_text)
        print()
    else:
        os.makedirs(os.path.join(LINGO_XLIBS_PATH, f"{slug_alpha}"), exist_ok=True)
        with open(os.path.join(LINGO_XLIBS_PATH, f"{slug_alpha}/{slug}.cpp"), "w") as cpp:
            cpp.write(cpp_text)

    header_text = XCMD_TEMPLATE_H.format(
        slug_upper=slug.upper(),
        slug_upper_alpha=slug.upper()[:1],
        xobj_class=xobj_class,
        methlist=TEMPLATE_HEADER_METH.format(methname=name),
    )
    if dry_run:
        print("Header output:")
        print(header_text)
        print()
    else:
        os.makedirs(os.path.join(LINGO_XLIBS_PATH, f"{slug_alpha}"), exist_ok=True)
        with open(os.path.join(LINGO_XLIBS_PATH, f"{slug_alpha}/{slug}.h"), "w") as header:
            header.write(header_text)

    if not dry_run:
        inject_makefile(slug, type)
        inject_lingo_object(slug, xobj_class, director_version, "XObject")

def generate_xtra_stubs(
    msgtable: list[str],
    slug: str,
    name: str,
    filename: str,
    director_version: int = 500,
    dry_run: bool = False,
) -> None:
    meths = []
    slug_alpha=slug[:1]
    for e in msgtable:
        elem = e.split("--", 1)[0].strip()
        if not elem:
            continue
        if elem.startswith("/*"):
            # An entry the Xtra author commented out, so it isn't registered.
            # Keep it in the table listing, but don't generate a stub for it.
            continue
        functype = "method"
        if elem.startswith("+"):
            elem = elem[1:].strip()
            functype = "toplevel"
        elif elem.startswith("*"):
            elem = elem[1:].strip()
            functype = "global"
        if " " not in elem:
            methname, argv = elem, []
        else:
            methname, args = elem.split(" ", 1)
            argv = args.split(",")
        min_args = len(argv)
        max_args = len(argv)
        if argv and argv[-1].strip() == "*":
            min_args = -1
            max_args = 0
        elif functype == "method" or functype == "toplevel":
            min_args -= 1
            max_args -= 1

        meths.append(
            dict(
                functype=functype,
                methname=methname,
                args=argv,
                min_args=min_args,
                max_args=max_args,
                default="0",
            )
        )
    xobject_class = f"{name}XtraObject"
    xobj_class = f"{name}Xtra"

    cpp_text = TEMPLATE.format(
        base="xtras",
        slug=slug,
        slug_alpha=slug_alpha,
        name=name,
        filename=filename,
        xmethtable="\n".join(msgtable),
        xobject_class=xobject_class,
        xobj_class=xobj_class,
        xlib_methods="\n".join(
            [
                XLIB_METHOD_TEMPLATE.format(
                    xobj_class=xobj_class, director_version=director_version, **x
                )
                for x in meths if x["functype"] == "method"
            ]
        ),
        xlib_builtins="\n".join([BUILTIN_TEMPLATE.format(
            name=x["methname"],
            xobj_class=xobj_class,
            min_args=x["min_args"],
            max_args=x["max_args"],
            director_version=director_version,
            methtype="HBLTIN",
        ) for x in meths if x["functype"] == "global"]),
        xlib_toplevels="\n".join([
            XLIB_METHOD_TEMPLATE.format(
                    xobj_class=xobj_class, director_version=director_version, **x
        ) for x in meths if x["functype"] == "toplevel"]),
        xtra_props=XTRA_PROPS_TEMPLATE.format(xobj_class=xobj_class,
                                              xobject_class=xobject_class),
		xobj_new=XLIB_NEW_TEMPLATE.format(xobj_class=xobj_class),
        xobj_stubs="\n".join(
            [
                XOBJ_STUB_TEMPLATE.format(xobj_class=xobj_class, **x)
                for x in meths
                if x["methname"] != "new"
            ]
        ),
    )
    if dry_run:
        print("C++ output:")
        print(cpp_text)
        print()
    else:
        os.makedirs(os.path.join(LINGO_XTRAS_PATH, f"{slug_alpha}"), exist_ok=True)
        with open(os.path.join(LINGO_XTRAS_PATH, f"{slug_alpha}/{slug}.cpp"), "w") as cpp:
            cpp.write(cpp_text)

    header_text = TEMPLATE_H.format(
        base_upper="XTRAS",
        slug_upper=slug.upper(),
        slug_upper_alpha=slug.upper()[:1],
        xobject_class=xobject_class,
        xobj_class=xobj_class,
        xtra_props_h=XTRA_PROPS_H,
        methlist="\n".join([TEMPLATE_HEADER_METH.format(**x) for x in meths]),
    )
    if dry_run:
        print("Header output:")
        print(header_text)
        print()
    else:
        os.makedirs(os.path.join(LINGO_XTRAS_PATH, f"{slug_alpha}"), exist_ok=True)
        with open(os.path.join(LINGO_XTRAS_PATH, f"{slug_alpha}/{slug}.h"), "w") as header:
            header.write(header_text)

    if not dry_run:
        inject_makefile(slug, "Xtra")
        inject_lingo_object(slug, xobj_class, director_version, "Xtra")



def main() -> None:
    parser = argparse.ArgumentParser(
        description="Extract the method table from a Macromedia Director XObject/XLib and generate method stubs."
    )
    parser.add_argument("XOBJ_FILE", help="XObject/XLib file to test")
    parser.add_argument(
        "--resid", help="Resource ID (for MacBinary)", type=str, default=None
    )
    parser.add_argument(
        "--slug", help="Slug to use for files (e.g. {slug}.cpp, {slug}.h)"
    )
    parser.add_argument(
        "--name", help="Base name to use for classes (e.g. {name}XObj, {name}XObject)"
    )
    parser.add_argument(
        "--version",
        metavar="VER",
        help="Minimum Director version (default: 400)",
        type=int,
        default=400,
    )
    parser.add_argument(
        "--write",
        help="Write generated stubs to the source tree",
        dest="dry_run",
        action="store_false",
    )
    args = parser.parse_args()

    xcode = extract_xcode(args.XOBJ_FILE, args.resid)
    slug = args.slug or xcode["slug"]
    name = args.name or xcode["name"]
    if xcode["type"] == "XObject":
        generate_xobject_stubs(
            xcode["method_table"],
            slug,
            name,
            xcode["filename"],
            args.version,
            args.dry_run,
        )
    elif xcode["type"] == "Xtra":
        version = args.version if args.version > 500 else 500
        generate_xtra_stubs(
            xcode["method_table"], slug, name, xcode["filename"], version, args.dry_run
        )
    elif xcode["type"] == "XFCN" or xcode["type"] == "XCMD":
        generate_xcmd_stubs(
            xcode["type"], slug, name, xcode["filename"], args.version, args.dry_run
        )


if __name__ == "__main__":
    main()
