"""Lscr / Lnam parser, mirroring LingoCompiler::compileLingoV4
(engines/director/lingo/lingo-bytecode.cpp:1012) and LingoArchive::addNamesV4 (:1734).

Everything inside a resource is big endian.
"""
import struct

HEADER_SIZE = 0x5c
FUNC_REC_SIZE = 0x2a


def u8(b, o):   return b[o]
def u16(b, o):  return struct.unpack_from(">H", b, o)[0]
def s16(b, o):  return struct.unpack_from(">h", b, o)[0]
def u32(b, o):  return struct.unpack_from(">I", b, o)[0]


class Function:
    __slots__ = ("name_index", "unk0", "length", "start_offset", "arg_count",
                 "arg_offset", "var_count", "var_offset", "tail", "code",
                 "arg_names", "var_names")

    def __repr__(self):
        return (f"<fn name={self.name_index} code@{self.start_offset}+{self.length} "
                f"args={self.arg_count}@{self.arg_offset} vars={self.var_count}@{self.var_offset}>")


class Lscr:
    def __init__(self, data, const_entry_size=None):
        """const_entry_size is 6 on D4 and 8 from D5 on (lingo-bytecode.cpp:1454).
        Left unset it is inferred from the store: the reference table runs from
        consts_offset up to consts_store_offset, so the stride follows from how
        many entries fit."""
        if len(data) < HEADER_SIZE:
            raise ValueError("Lscr header too small")
        self.raw = data
        self.unk1 = data[0:8]
        self.length = u32(data, 0x08)
        self.length2 = u32(data, 0x0c)
        self.code_store_offset = u16(data, 0x10)
        self.script_id = u16(data, 0x12)
        self.unk2 = s16(data, 0x14)
        self.parent_number = s16(data, 0x16)
        self.unk_block = data[0x18:0x24]
        self.unk3 = u16(data, 0x24)
        self.script_flags = u32(data, 0x26)
        self.unk4 = data[0x2a:0x2e]
        self.assembly_id = u16(data, 0x2e)
        self.factory_name_id = s16(data, 0x30)

        self.event_map_count = u16(data, 0x32)
        self.event_map_offset = u32(data, 0x34)
        self.event_map_flags = u32(data, 0x38)
        self.properties_count = u16(data, 0x3c)
        self.properties_offset = u32(data, 0x3e)
        self.globals_count = u16(data, 0x42)
        self.globals_offset = u32(data, 0x44)
        self.functions_count = u16(data, 0x48)
        self.functions_offset = u32(data, 0x4a)
        self.consts_count = u16(data, 0x4e)
        self.consts_offset = u32(data, 0x50)
        self.consts_store_count = u32(data, 0x54)
        self.consts_store_offset = u32(data, 0x58)

        self.properties = [s16(data, self.properties_offset + 2 * i)
                           for i in range(self.properties_count)]
        self.globals = [s16(data, self.globals_offset + 2 * i)
                        for i in range(self.globals_count)]
        self.event_map = [s16(data, self.event_map_offset + 2 * i)
                          for i in range(self.event_map_count)]

        # constants: reference table of (type, value); 6 bytes for D4, 8 for D5+
        if const_entry_size is None:
            span = self.consts_store_offset - self.consts_offset
            const_entry_size = 6
            if self.consts_count and span // self.consts_count in (6, 8):
                const_entry_size = span // self.consts_count
        self.const_entry_size = const_entry_size
        self.consts = []
        for i in range(self.consts_count):
            o = self.consts_offset + const_entry_size * i
            self.consts.append((u16(data, o), u32(data, o + 2)))
        self.consts_store = data[self.consts_store_offset:]

        # functions
        self.functions = []
        for i in range(self.functions_count):
            o = self.functions_offset + FUNC_REC_SIZE * i
            f = Function()
            f.name_index = u16(data, o)
            f.unk0 = u16(data, o + 2)
            f.length = u32(data, o + 4)
            f.start_offset = u32(data, o + 8)
            f.arg_count = u16(data, o + 12)
            f.arg_offset = u32(data, o + 14)
            f.var_count = u16(data, o + 18)
            f.var_offset = u32(data, o + 20)
            f.tail = data[o + 24:o + FUNC_REC_SIZE]
            f.code = data[f.start_offset:f.start_offset + f.length]
            f.arg_names = [s16(data, f.arg_offset + 2 * j) for j in range(f.arg_count)]
            f.var_names = [s16(data, f.var_offset + 2 * j) for j in range(f.var_count)]
            self.functions.append(f)

    def layout(self):
        """Section start offsets in file order, for figuring out the canonical layout."""
        secs = [("header", 0, HEADER_SIZE)]
        if self.properties_count:
            secs.append(("properties", self.properties_offset, self.properties_count * 2))
        if self.globals_count:
            secs.append(("globals", self.globals_offset, self.globals_count * 2))
        if self.event_map_count:
            secs.append(("eventmap", self.event_map_offset, self.event_map_count * 2))
        if self.consts_count:
            secs.append(("constsIdx", self.consts_offset, self.consts_count * 6))
        secs.append(("constsStore", self.consts_store_offset, len(self.raw) - self.consts_store_offset))
        secs.append(("codeStore", self.code_store_offset, 0))
        if self.functions_count:
            secs.append(("funcTable", self.functions_offset, self.functions_count * FUNC_REC_SIZE))
        for f in self.functions:
            secs.append(("  code", f.start_offset, f.length))
            if f.arg_count:
                secs.append(("  args", f.arg_offset, f.arg_count * 2))
            if f.var_count:
                secs.append(("  vars", f.var_offset, f.var_count * 2))
        return sorted(secs, key=lambda s: (s[1], s[0]))


def parse_lnam(data):
    """Returns the list of names. Header is 0x14 bytes."""
    if len(data) < 0x14:
        raise ValueError("Lnam header too small")
    size = u32(data, 8)
    size2 = u32(data, 12)
    offset = u16(data, 16)
    count = u16(data, 18)
    names = []
    p = offset
    for _ in range(count):
        n = data[p]
        names.append(data[p + 1:p + 1 + n].decode("latin1"))
        p += 1 + n
    return names, dict(head=data[0:8], size=size, size2=size2, offset=offset, count=count, end=p)


def build_lnam(names, head=b"\0" * 8, size_delta=0):
    """Inverse of parse_lnam.

    `size_delta` is what the source subtracted from the chunk length before
    storing it: 0 up to D6, where the field is the whole chunk, and 20 -- the
    header -- on D7, where it counts only the names. ScummVM shrugs the
    difference off ("D7+ size may not match the stream length",
    lingo-bytecode.cpp:1758), so it is easy to get wrong and never notice.
    """
    offset = 0x14
    body = b"".join(bytes([len(n)]) + n.encode("latin1") for n in names)
    size = offset + len(body) - size_delta
    return (head
            + struct.pack(">II", size, size)
            + struct.pack(">HH", offset, len(names))
            + body)
