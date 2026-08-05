"""Lscr assembler for Director 4 (RIFX/MV93).

Layout is taken from a real D4 script (STRTMAX2.dir, Lscr #52, an "on exitFrame"
with one handler and no constants) and from the reader in
LingoCompiler::compileLingoV4 (engines/director/lingo/lingo-bytecode.cpp:1012):

    0x00  header, 0x5c bytes
    0x5c  code store: handler bytecode, each padded to an even offset,
          followed by the per-handler argument and variable name arrays
          function table, 0x2a bytes per handler
          constants index (6 bytes per entry on D4) and constants store
          event map, 2 bytes per event
    end

Fields ScummVM neither reads nor validates are carried as opaque blobs so a real
script can be reproduced byte for byte; see OPAQUE_* below.
"""
import struct

HEADER_SIZE = 0x5c
FUNC_REC_SIZE = 0x2a
CONST_ENTRY_SIZE = 6            # D4; D5+ uses 8

# --- opcodes we need (engines/director/lingo/lingo-bytecode.cpp:41) ----------
OP_PROCRET = 0x01               # c_procret
OP_INTPUSH_B = 0x41             # c_intpush, int8 operand
OP_ARGCNORET_B = 0x42           # c_argcnoretpush, uint8 operand
OP_ARGC_B = 0x43                # c_argcpush, uint8 operand
OP_CALL_B = 0x57                # cb_call, uint8 name index
OP_THEENTITYPUSH_B = 0x5c       # cb_v4theentitypush, uint8 bank
OP_THEENTITYASSIGN_B = 0x5d     # cb_v4theentityassign, uint8 bank
OP_CONSTPUSH_B = 0x44           # push constant, operand = index * CONST_ENTRY_SIZE
OP_GLOBALPUSH_B = 0x49          # cb_globalpush, uint8 name index
OP_GLOBALASSIGN_B = 0x4f        # cb_globalassign, uint8 name index
OP_ADD = 0x05                   # c_add
OP_MUL = 0x04                   # c_mul
OP_AMPERSAND = 0x0a             # c_ampersand ('&')
OP_EQ = 0x0f                    # c_eq

# 'the <field> of sprite <n>' -> bank 0x06, selector per lingo-bytecode.cpp:182ff
BANK_SPRITE = 0x06
SPRITE_EDITABLETEXT = 0x1f

# 'the text of field <n>' -> bank 0x0b, selector 0x02 (lingo-bytecode.cpp:295)
BANK_FIELD = 0x0b
FIELD_TEXT = 0x02


OP_LIST = 0x1e                  # cb_list, count comes from a preceding argcpush
OP_NAMEPUSH_B = 0x45            # c_namepush, uint8 name index -> SYMBOL
OP_VARREFPUSH_B = 0x46          # cb_varrefpush, uint8 name index
OP_OBJECTCALL_B = 0x58          # cb_objectcall, uint8 varType

SCRIPT_FLAG_FACTORY_DEF = 0x10  # kScriptFlagFactoryDef (types.h:91)
FACTORY_ME_ARG = -1             # arg 0 of a factory method; the reader maps
                                # index -1 to "me" (lingo-bytecode.cpp:~1236)


def const_push(index, entry_size=CONST_ENTRY_SIZE):
    """The operand is a byte offset into the constants index, so it scales with
    the entry size: 6 bytes on D4, 8 from D5 on (lingo-bytecode.cpp:1454)."""
    return bytes([OP_CONSTPUSH_B, index * entry_size])


def name_push(name_index):
    return bytes([OP_NAMEPUSH_B, name_index])


def varref_push(name_index):
    return bytes([OP_VARREFPUSH_B, name_index])


def argc_push(n, want_result=True):
    return bytes([OP_ARGC_B if want_result else OP_ARGCNORET_B, n])


def make_list(n):
    """Builds a Lingo list from the n values already on the stack."""
    return argc_push(n) + bytes([OP_LIST])


def object_call(method_name_index, obj_name_index, nargs=1, want_result=False):
    """`<obj>(<method>, ...)`. Mirrors real D4 code: arguments first, then the
    arg count, then a varref to the object, then objectcall. cb_objectcall is
    what turns the leading SYMBOL into a VARREF, which is why factory and
    XObject calls go through it rather than plain cb_call."""
    return (name_push(method_name_index)
            + argc_push(nargs, want_result)
            + varref_push(obj_name_index)
            + bytes([OP_OBJECTCALL_B, 1]))


def call_function(name_index, nargs):
    """Call in expression position: ARGC, so the result is pushed."""
    return bytes([OP_ARGC_B, nargs, OP_CALL_B, name_index])


def global_push(name_index):
    if name_index <= 0xff:
        return bytes([OP_GLOBALPUSH_B, name_index])
    return bytes([OP_GLOBALPUSH_W]) + struct.pack(">H", name_index)


def global_assign(name_index):
    if name_index <= 0xff:
        return bytes([OP_GLOBALASSIGN_B, name_index])
    return bytes([OP_GLOBALASSIGN_W]) + struct.pack(">H", name_index)


def the_field_assign(field_num, selector=FIELD_TEXT):
    """Tail of an assignment to 'the <selector> of field <field_num>'; the value
    must already be on the stack. cb_v4theentityassign pops selector, then
    value, then the id -- so the push order is id, value, selector. Confirmed
    against real bytecode ('41 20 49 1e 41 06 5d 06' = 'set the cursor of
    sprite 32 to <global>')."""
    return intpush(selector) + bytes([OP_THEENTITYASSIGN_B, BANK_FIELD])


def field_assign_id(field_num, cast_lib=None):
    """Leading part of a field assignment, pushed before the value.

    From D5 on, kTheCast and kTheField take the member as two Datums, and
    cb_v4theentityassign pops the cast lib before the member -- so they go on
    the stack as member first, then cast lib (lingo-bytecode.cpp:920ff, and
    toCastMemberID(member, castLib) in lingo.cpp:2002). On D4 only the member
    is pushed.
    """
    if cast_lib is None:
        return intpush(field_num)
    return intpush(field_num) + intpush(cast_lib)


def build_consts(strings, entry_size=CONST_ENTRY_SIZE):
    """Returns (index_table, store). Strings are stored as a big-endian u32
    length that *includes* the terminating null, then the bytes and the null;
    entries start on even offsets. The index entry is a u16 type plus a u32
    value on D4, and a u32 type plus a u32 value from D5 on."""
    index = bytearray()
    store = bytearray()
    for s in strings:
        raw = s.encode("latin1")
        offset = len(store)
        if entry_size == 8:
            index += struct.pack(">II", 1, offset)  # type 1 = string
        else:
            index += struct.pack(">HI", 1, offset)
        store += struct.pack(">I", len(raw) + 1) + raw + b"\0"
        if len(store) % 2:
            store += b"\0"
    return bytes(index), bytes(store)

# Event map slot used by a lone exitFrame handler. ScummVM skips the map
# outright -- "we probably don't need to read this since we already did
# something similar with _eventHandlers" (lingo-bytecode.cpp:1084) -- but it is
# how Director finds a handler, and a script filed under the wrong slot simply
# never runs.
#
# The slots move between D4 and D5. Counted over 157115 scripts in 11057 movies,
# mouseDown, mouseUp, keyDown and keyUp hold 0 to 3 throughout, and everything
# from idle on shifts by five when timeout, prepareFrame, mouseEnter, mouseLeave
# and mouseWithin appear:
#
#              idle  startMovie  stopMovie  enterFrame  exitFrame
#   D4            5           6          7           9         10
#   D5 and later 10          11         12          14         15
#
# No version mixes the two: exitFrame has 22574 hits at slot 10 across D4 and
# over 50000 at slot 15 from D5 to D10.
EXITFRAME_EVENT_INDEX_D4 = 10
EXITFRAME_EVENT_INDEX_D5 = 15

# kept for callers that still assume D4
EXITFRAME_EVENT_INDEX = EXITFRAME_EVENT_INDEX_D4
EXITFRAME_EVENT_COUNT = EXITFRAME_EVENT_INDEX_D4 + 1
EXITFRAME_EVENT_FLAGS = 1 << EXITFRAME_EVENT_INDEX_D4


def exitframe_event_map(version=400):
    """(map, flags) for a script whose only handler is `on exitFrame`."""
    idx = EXITFRAME_EVENT_INDEX_D4 if version < 500 else EXITFRAME_EVENT_INDEX_D5
    return [-1] * idx + [0], 1 << idx


# Every opcode with a byte operand has a twin taking a uint16, and the reader
# tells them apart by the opcode alone (lingo-bytecode.cpp:111ff). A donor with
# a large cast or a long name table needs them.
OP_INTPUSH_W = 0x81             # c_intpush, int16
OP_ARGCNORET_W = 0x82           # c_argcnoretpush, uint16
OP_ARGC_W = 0x83                # c_argcpush, uint16
OP_CALL_W = 0x97                # cb_call, uint16 name index
OP_GLOBALPUSH_W = 0x89          # cb_globalpush, uint16 name index
OP_GLOBALASSIGN_W = 0x8f        # cb_globalassign, uint16 name index


def intpush(value):
    if -128 <= value <= 127:
        return bytes([OP_INTPUSH_B, value & 0xff])
    return bytes([OP_INTPUSH_W]) + struct.pack(">h", value)


def the_sprite_field(sprite_num, selector):
    """Pushes 'the <selector> of sprite <sprite_num>'.

    Push order is id first, then selector: cb_v4theentitypush pops the selector
    (firstArg) before the id. Confirmed against real bytecode, e.g. '41 03 41 12
    5c 06' = 'the puppet of sprite 3' in MaxandMarie DATEN/LADEN.DXR.
    """
    return intpush(sprite_num) + intpush(selector) + bytes([OP_THEENTITYPUSH_B, BANK_SPRITE])


def call_command(name_index, nargs):
    """Statement-position call: ARGCNORET, so LC::call gets allowRetVal=false.
    Works for HBLTIN builtins, which register in both the command and the
    function table (lingo-builtins.cpp:474)."""
    if name_index <= 0xff:
        return bytes([OP_ARGCNORET_B, nargs, OP_CALL_B, name_index])
    return (bytes([OP_ARGCNORET_B, nargs, OP_CALL_W])
            + struct.pack(">H", name_index))


def pad_even(blob, at):
    return b"\0" if (at + len(blob)) % 2 else b""


class Handler:
    def __init__(self, name_index, code, arg_names=(), var_names=(), tail=None):
        self.name_index = name_index
        self.code = code
        self.arg_names = list(arg_names)
        self.var_names = list(var_names)
        # 9 uint16 ScummVM skips; the reference script's values are reused
        self.tail = tail if tail is not None else struct.pack(
            ">9H", 0, 0, 0, 0, 12, 1, 1, 0, 0)


def build_lscr(handlers, *, script_id, assembly_id, unk1=b"\0" * 8, unk2=2,
               parent_number=-1, unk_block=None, unk3=0, script_flags=0,
               unk4=b"\0" * 4, factory_name_id=-1,
               event_map=None, event_map_flags=0, name_gap=b"",
               func_unk0=10, consts=(), properties=(), global_names=(),
               const_entry_size=CONST_ENTRY_SIZE):
    """Serialises one Lscr chunk. `handlers` is a list of Handler,
    `consts` a list of strings referenced by const_push()."""
    if unk_block is None:
        unk_block = b"\xff\xff" + b"\0" * 10

    body = bytearray()          # everything from HEADER_SIZE onwards

    def cur():
        return HEADER_SIZE + len(body)

    # 1. handler bytecode, each padded so the next section starts even
    starts = []
    for h in handlers:
        starts.append(cur())
        body += h.code
        body += pad_even(b"", cur())

    # 2. argument and variable name arrays
    arg_offsets, var_offsets = [], []
    for h in handlers:
        arg_offsets.append(cur())
        for n in h.arg_names:
            body += struct.pack(">h", n)
        var_offsets.append(cur())
        for n in h.var_names:
            body += struct.pack(">h", n)

    # opaque filler the reference script carries between the name arrays and
    # the function table; ScummVM addresses both by explicit offset
    body += name_gap
    if cur() % 2:
        body += b"\0"

    # 3. property list, global list, then the function table.
    #
    # The global list is how a script declares which names are globals, the
    # equivalent of writing `global gFoo` in the source. ScummVM creates a
    # global on first use and so never misses it (cb_globalpush), but Director
    # treats an undeclared name as a local and refuses it with "Variable used
    # before assigned a value".
    properties_offset = cur()
    for p in properties:
        body += struct.pack(">h", p)
    globals_offset = cur()
    for g in global_names:
        body += struct.pack(">h", g)
    functions_offset = cur()
    for h, start, ao, vo in zip(handlers, starts, arg_offsets, var_offsets):
        body += struct.pack(">HHII", h.name_index, func_unk0, len(h.code), start)
        body += struct.pack(">HI", len(h.arg_names), ao)
        body += struct.pack(">HI", len(h.var_names), vo)
        body += h.tail

    # 4. constants. With none, the index and store collapse onto the event map,
    #    exactly as the reference script does.
    const_index, const_store = build_consts(consts, const_entry_size)
    consts_offset = cur()
    body += const_index
    consts_store_offset = cur()
    body += const_store
    event_map_offset = cur()

    # 5. event map
    if event_map is None:
        event_map = []
    for e in event_map:
        body += struct.pack(">h", e)

    total = HEADER_SIZE + len(body)

    head = bytearray()
    head += unk1
    head += struct.pack(">II", total, total)
    head += struct.pack(">HH", HEADER_SIZE, script_id)
    head += struct.pack(">hh", unk2, parent_number)
    head += unk_block
    head += struct.pack(">HI", unk3, script_flags)
    head += unk4
    head += struct.pack(">Hh", assembly_id, factory_name_id)
    head += struct.pack(">HII", len(event_map), event_map_offset, event_map_flags)
    head += struct.pack(">HI", len(properties), properties_offset)
    head += struct.pack(">HI", len(global_names), globals_offset)
    head += struct.pack(">HI", len(handlers), functions_offset)
    head += struct.pack(">HI", len(consts), consts_offset)
    # The field after the count is the store's length in bytes, not a second
    # count: the donor's script with one constant says 12, which is exactly how
    # far its store runs. ScummVM reads it into a variable it never uses
    # (lingo-bytecode.cpp:1100), so a count passed unnoticed.
    head += struct.pack(">II", len(const_store), consts_store_offset)
    assert len(head) == HEADER_SIZE, len(head)

    return bytes(head) + bytes(body)
