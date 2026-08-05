"""Build the Director 4 regression fixtures.

Both movies use nothing but stock Lingo so they run in real Director as well as
in ScummVM, and both print their verdict on stage.

editable.dir -- covers
    616b3d29cd9  DIRECTOR: Mark a channel dirty when the score toggles the editable flag
    5a896cf1c70  DIRECTOR: Follow the score editable flag on the same cast member

  Sprite 1 shows the same text member at the same position in every frame; only
  the score's editable bit (and, in one frame, the back colour) changes. Each
  frame appends one digit to a global:

      frame 1  editable 1, backColor 0    expect 1   -- initial load
      frame 2  editable 0, backColor 255  expect 0   -- channel is dirty anyway
                                                        because backColor moved,
                                                        so this isolates
                                                        Sprite::replaceFrom()
      frame 3  editable 1, backColor 255  expect 1   -- ONLY the editable bit
                                                        differs, so this isolates
                                                        Channel::isDirty()
      frame 4  displays the result

  Verdict shown on stage:
      111  both fixes present
      101  replaceFrom fix missing (editable never follows the score at all)
      110  isDirty fix missing (the editable-only transition is skipped)
"""
import struct, pathlib
import rifx, lscr, lscr_asm, config, build_movie as bm

OUT = pathlib.Path(__file__).parent / "out"
OUT.mkdir(exist_ok=True)

SPRITE_CH = 1           # channel with the tested text member
RESULT_CH = 2           # channel with the result field
TEXT_MEMBER = 1         # donor cast member 1 (text)
RESULT_MEMBER = 3       # duplicate of the donor text member, used for output

NAMES = ["exitFrame", "put", "gEditableResult"]
I_EXITFRAME, I_PUT, I_GLOBAL = 0, 1, 2

LABEL = "editable "
TRIVIAL_FROM_BYTECODE = "FROM BYTECODE"
TRIVIAL_FROM_SOURCE = "FROM SOURCE"


def code_first(expected):
    """set gEditableResult to (the editableText of sprite 1 = <expected>)"""
    return (lscr_asm.the_sprite_field(SPRITE_CH, lscr_asm.SPRITE_EDITABLETEXT)
            + lscr_asm.intpush(expected)
            + bytes([lscr_asm.OP_EQ])
            + lscr_asm.global_assign(I_GLOBAL)
            + bytes([lscr_asm.OP_PROCRET]))


def code_append(expected):
    """set gEditableResult to gEditableResult * 10 + (the editableText of sprite 1 = <expected>)"""
    return (lscr_asm.global_push(I_GLOBAL)
            + lscr_asm.intpush(10)
            + bytes([lscr_asm.OP_MUL])
            + lscr_asm.the_sprite_field(SPRITE_CH, lscr_asm.SPRITE_EDITABLETEXT)
            + lscr_asm.intpush(expected)
            + bytes([lscr_asm.OP_EQ, lscr_asm.OP_ADD])
            + lscr_asm.global_assign(I_GLOBAL)
            + bytes([lscr_asm.OP_PROCRET]))


def code_report():
    """set the text of field 3 to "editable " & gEditableResult
       put "editable " & gEditableResult"""
    message = (lscr_asm.const_push(0)
               + lscr_asm.global_push(I_GLOBAL)
               + bytes([lscr_asm.OP_AMPERSAND]))
    return (lscr_asm.intpush(RESULT_MEMBER)      # id
            + message                            # value
            + lscr_asm.the_field_assign(RESULT_MEMBER)
            + message
            + lscr_asm.call_command(I_PUT, 1)
            + bytes([lscr_asm.OP_PROCRET]))


# The source kept in the cast info is not decoration: Director compiles from it,
# and its error messages quote these very lines. It therefore needs the `global`
# declaration just as much as the Lscr's global list does.
def src_first(e):
    return ("on exitFrame\r"
            "  global gEditableResult\r"
            "  set gEditableResult to (the editableText of sprite %d = %d)\r"
            "end\r" % (SPRITE_CH, e))


def src_append(e):
    return ("on exitFrame\r"
            "  global gEditableResult\r"
            "  set gEditableResult to gEditableResult * 10 + "
            "(the editableText of sprite %d = %d)\r"
            "end\r" % (SPRITE_CH, e))


def src_report(result_member=RESULT_MEMBER):
    """The source has to name the same field the bytecode writes to.

    Director keeps this text in the cast info and compiles from it -- the cast
    window labels a member it has not compiled yet as exactly that -- so a source
    saying field 3 while the Lscr writes field 7 gives a movie that runs and puts
    its answer somewhere invisible. In the append fixtures member 3 is one of the
    donor's scripts, so the assignment lands nowhere at all.
    """
    return ("on exitFrame\r"
            "  global gEditableResult\r"
            '  set the text of field %d to "%s" & gEditableResult\r'
            '  put "%s" & gEditableResult\r'
            "end\r" % (result_member, LABEL, LABEL))


def make_lscr(code, script_id, consts=()):
    h = lscr_asm.Handler(I_EXITFRAME, code)
    return lscr_asm.build_lscr([h], script_id=script_id, assembly_id=script_id,
                               event_map=[-1] * 10 + [0],
                               event_map_flags=lscr_asm.EXITFRAME_EVENT_FLAGS,
                               consts=consts, global_names=[I_GLOBAL])


def build_editable(vwsc_override=None, out_name="editable.dir"):
    donor = bm.load_donor()
    keep = {}
    for r in donor.resources:
        if r.tag in ("VWCF", "Sord", "VWFI", "VWFM", "CASt", "STXT"):
            keep.setdefault(r.tag, []).append(r)
    cast_text, cast_shape = keep["CASt"][0], keep["CASt"][1]
    stxt = keep["STXT"][0]

    chunks = []

    def add(tag, payload):
        chunks.append((tag, payload))
        return 3 + len(chunks) - 1

    i_key = add(b"KEY*", b"")
    i_vwcf = add(b"VWCF", donor.chunk(keep["VWCF"][0]))
    i_cas = add(b"CAS*", b"")
    i_sord = add(b"Sord", donor.chunk(keep["Sord"][0]))
    i_vwfi = add(b"VWFI", donor.chunk(keep["VWFI"][0]))
    i_vwfm = add(b"VWFM", donor.chunk(keep["VWFM"][0]))
    i_vwsc = add(b"VWSC", b"")

    i_cast_text = add(b"CASt", donor.chunk(cast_text))
    i_stxt = add(b"STXT", donor.chunk(stxt))
    i_cast_shape = add(b"CASt", donor.chunk(cast_shape))
    # cast member 3: a second text member for the verdict. Writing into the
    # *tested* member would set Cast::isModified() and make the channel dirty,
    # which would mask exactly the bug we want to catch (channel.cpp:296).
    i_cast_res = add(b"CASt", donor.chunk(cast_text))
    i_stxt_res = add(b"STXT", donor.chunk(stxt))

    scripts = [
        ("checkFrame1", code_first(1), src_first(1), ()),
        ("checkFrame2", code_append(0), src_append(0), ()),
        ("checkFrame3", code_append(1), src_append(1), ()),
        ("reportResult", code_report(), src_report(), (LABEL,)),
    ]
    cast_script_idx, lscr_idx = [], []
    for i, (name, code, src, consts) in enumerate(scripts, start=1):
        cast_script_idx.append(add(b"CASt", bm.build_script_cast(i, name, src)))
        lscr_idx.append(add(b"Lscr", make_lscr(code, i, consts)))

    i_lnam = add(b"Lnam", lscr.build_lnam(NAMES))
    i_lctx = add(b"Lctx", bm.build_lctx(lscr_idx, i_lnam))

    # CAS* slot n -> cast member n+1
    members = [i_cast_text, i_cast_shape, i_cast_res] + cast_script_idx
    script_member = {i: 4 + i for i in range(len(scripts))}   # members 4..7

    chunks[i_cas - 3] = (b"CAS*", bm.build_cas(members))
    chunks[i_vwcf - 3] = (b"VWCF", config.set_cast_array_end(
        chunks[i_vwcf - 3][1], len(members)))

    key_entries = [
        (i_stxt, i_cast_text, b"STXT"),
        (i_stxt_res, i_cast_res, b"STXT"),
        (i_cas, bm.CASTLIB_KEY_PARENT, b"CAS*"),
        (i_sord, bm.CASTLIB_KEY_PARENT, b"Sord"),
        (i_vwcf, bm.CASTLIB_KEY_PARENT, b"VWCF"),
        (i_vwfi, bm.CASTLIB_KEY_PARENT, b"VWFI"),
        (i_vwfm, bm.CASTLIB_KEY_PARENT, b"VWFM"),
        (i_vwsc, bm.CASTLIB_KEY_PARENT, b"VWSC"),
        (i_lctx, bm.CASTLIB_KEY_PARENT, b"Lctx"),
        (i_lnam, bm.CASTLIB_KEY_PARENT, b"Lnam"),
    ]
    for c, s in zip(cast_script_idx, lscr_idx):
        key_entries.append((s, c, b"Lscr"))
    chunks[i_key - 3] = (b"KEY*", bm.build_key(key_entries, max_entries=24))

    def frame(editable, back, action_member):
        return {
            0: bm.main_channel_d4(action_id=action_member),
            SPRITE_CH: bm.sprite_d4(cast_member=TEXT_MEMBER, x=20, y=20,
                                    w=200, h=40, editable=editable, back=back),
            RESULT_CH: bm.sprite_d4(cast_member=RESULT_MEMBER, x=20, y=120,
                                    w=200, h=40),
        }

    chunks[i_vwsc - 3] = (b"VWSC", vwsc_override if vwsc_override is not None
                          else bm.build_vwsc([
        frame(True, bm.SPRITE_BACK_D4, script_member[0]),
        frame(False, 255, script_member[1]),
        frame(True, 255, script_member[2]),
        frame(True, 255, script_member[3]),
    ]))

    data = bm.build_rifx(chunks)
    path = OUT / out_name
    path.write_bytes(data)
    return path, data


# --------------------------------------------------------------------------
# listoverride.dir -- covers
#   83c04ad6fe0  DIRECTOR: Let list builtins override same-named handlers from
#                          me methods
#
# A factory defines its own getLast method and still calls getLast(<list>, 2)
# from inside mNew -- the exact shape the SpecialAddresses parent script in
# 'Ein Fall fuer Muetze & Co' uses. The list builtin has to win.
#
#   list LINTRANS   fix present  (the builtin ran, last element of the list)
#   list METHOD     fix missing  (the factory's own getLast method ran)
# --------------------------------------------------------------------------
B_NAMES = ["exitFrame", "put", "gListResult", "mNew", "getLast", "return",
           "MyListFactory"]
B_EXITFRAME, B_PUT, B_GLOBAL, B_MNEW, B_GETLAST, B_RETURN, B_FACTORY = range(7)
B_LABEL = "list "


def build_listoverride():
    donor = bm.load_donor()
    keep = {}
    for r in donor.resources:
        if r.tag in ("VWCF", "Sord", "VWFI", "VWFM", "CASt", "STXT"):
            keep.setdefault(r.tag, []).append(r)
    cast_text, cast_shape = keep["CASt"][0], keep["CASt"][1]
    stxt = keep["STXT"][0]

    chunks = []

    def add(tag, payload):
        chunks.append((tag, payload))
        return 3 + len(chunks) - 1

    i_key = add(b"KEY*", b"")
    i_vwcf = add(b"VWCF", donor.chunk(keep["VWCF"][0]))
    i_cas = add(b"CAS*", b"")
    i_sord = add(b"Sord", donor.chunk(keep["Sord"][0]))
    i_vwfi = add(b"VWFI", donor.chunk(keep["VWFI"][0]))
    i_vwfm = add(b"VWFM", donor.chunk(keep["VWFM"][0]))
    i_vwsc = add(b"VWSC", b"")
    i_cast_text = add(b"CASt", donor.chunk(cast_text))
    i_stxt = add(b"STXT", donor.chunk(stxt))
    i_cast_shape = add(b"CASt", donor.chunk(cast_shape))
    i_cast_res = add(b"CASt", donor.chunk(cast_text))
    i_stxt_res = add(b"STXT", donor.chunk(stxt))

    # --- the factory ------------------------------------------------------
    m_new = lscr_asm.Handler(
        B_MNEW,
        (lscr_asm.const_push(0) + lscr_asm.const_push(1)
         + lscr_asm.make_list(2)
         + lscr_asm.intpush(2)
         + lscr_asm.call_function(B_GETLAST, 2)
         + lscr_asm.global_assign(B_GLOBAL)
         + bytes([lscr_asm.OP_PROCRET])),
        arg_names=[lscr_asm.FACTORY_ME_ARG])
    m_getlast = lscr_asm.Handler(
        B_GETLAST,
        (lscr_asm.const_push(2)
         + lscr_asm.call_command(B_RETURN, 1)
         + bytes([lscr_asm.OP_PROCRET])),
        arg_names=[lscr_asm.FACTORY_ME_ARG])
    factory_lscr = lscr_asm.build_lscr(
        [m_new, m_getlast], script_id=1, assembly_id=1,
        script_flags=lscr_asm.SCRIPT_FLAG_FACTORY_DEF,
        factory_name_id=B_FACTORY, global_names=[B_GLOBAL],
        # parentNumber is an index into the Lctx entry list, not a flag, and
        # LingoDec resolves it as scripts[parentNumber + 1] to decide which
        # script owns the factory (lingodec/context.cpp:55). The list is
        # 1-based, so this factory (Lctx entry 1) must NOT use 0 -- that would
        # make it its own parent and send writeScriptText into endless
        # recursion. Point it at entry 2, the script that instantiates it.
        parent_number=1,
        properties=[-1],
        consts=["IN_OUT", "LINTRANS", "METHOD"])
    factory_src = (
        "factory MyListFactory\r"
        "method mNew\r"
        "  global gListResult\r"
        '  set gListResult to getLast(["IN_OUT", "LINTRANS"], 2)\r'
        "method getLast\r"
        '  return "METHOD"\r')

    run_src = "on exitFrame\r  MyListFactory(mNew)\rend\r"
    run_lscr = make_lscr_b(
        (lscr_asm.object_call(B_MNEW, B_FACTORY, nargs=1, want_result=False)
         + bytes([lscr_asm.OP_PROCRET])),
        script_id=2)

    report_msg = (lscr_asm.const_push(0)
                  + lscr_asm.global_push(B_GLOBAL)
                  + bytes([lscr_asm.OP_AMPERSAND]))
    report_src = ("on exitFrame\r"
                  "  global gListResult\r"
                  '  set the text of field %d to "%s" & gListResult\r'
                  '  put "%s" & gListResult\r'
                  "end\r" % (RESULT_MEMBER, B_LABEL, B_LABEL))
    report_lscr = make_lscr_b(
        (lscr_asm.intpush(RESULT_MEMBER) + report_msg
         + lscr_asm.the_field_assign(RESULT_MEMBER)
         + report_msg + lscr_asm.call_command(B_PUT, 1)
         + bytes([lscr_asm.OP_PROCRET])),
        script_id=3, consts=(B_LABEL,))

    i_cast_fac = add(b"CASt", bm.build_script_cast(
        1, "MyListFactory", factory_src, bm.SCRIPT_TYPE_MOVIE))
    i_lscr_fac = add(b"Lscr", factory_lscr)
    i_cast_run = add(b"CASt", bm.build_script_cast(2, "runFactory", run_src))
    i_lscr_run = add(b"Lscr", run_lscr)
    i_cast_rep = add(b"CASt", bm.build_script_cast(3, "reportList", report_src))
    i_lscr_rep = add(b"Lscr", report_lscr)

    i_lnam = add(b"Lnam", lscr.build_lnam(B_NAMES))
    i_lctx = add(b"Lctx", bm.build_lctx(
        [i_lscr_fac, i_lscr_run, i_lscr_rep], i_lnam))

    members = [i_cast_text, i_cast_shape, i_cast_res,
               i_cast_fac, i_cast_run, i_cast_rep]
    MEMBER_RUN, MEMBER_REP = 5, 6

    chunks[i_cas - 3] = (b"CAS*", bm.build_cas(members))
    chunks[i_vwcf - 3] = (b"VWCF", config.set_cast_array_end(
        chunks[i_vwcf - 3][1], len(members)))
    chunks[i_key - 3] = (b"KEY*", bm.build_key([
        (i_stxt, i_cast_text, b"STXT"),
        (i_stxt_res, i_cast_res, b"STXT"),
        (i_lscr_fac, i_cast_fac, b"Lscr"),
        (i_lscr_run, i_cast_run, b"Lscr"),
        (i_lscr_rep, i_cast_rep, b"Lscr"),
        (i_cas, bm.CASTLIB_KEY_PARENT, b"CAS*"),
        (i_sord, bm.CASTLIB_KEY_PARENT, b"Sord"),
        (i_vwcf, bm.CASTLIB_KEY_PARENT, b"VWCF"),
        (i_vwfi, bm.CASTLIB_KEY_PARENT, b"VWFI"),
        (i_vwfm, bm.CASTLIB_KEY_PARENT, b"VWFM"),
        (i_vwsc, bm.CASTLIB_KEY_PARENT, b"VWSC"),
        (i_lctx, bm.CASTLIB_KEY_PARENT, b"Lctx"),
        (i_lnam, bm.CASTLIB_KEY_PARENT, b"Lnam"),
    ], max_entries=24))

    def frame(action_member):
        return {0: bm.main_channel_d4(action_id=action_member),
                RESULT_CH: bm.sprite_d4(cast_member=RESULT_MEMBER,
                                        x=20, y=120, w=200, h=40)}

    chunks[i_vwsc - 3] = (b"VWSC", bm.build_vwsc(
        [frame(MEMBER_RUN), frame(MEMBER_REP)]))

    data = bm.build_rifx(chunks)
    path = OUT / "listoverride.dir"
    path.write_bytes(data)
    return path, data


def make_lscr_b(code, script_id, consts=()):
    h = lscr_asm.Handler(B_EXITFRAME, code)
    return lscr_asm.build_lscr([h], script_id=script_id, assembly_id=script_id,
                               event_map=[-1] * 10 + [0],
                               event_map_flags=lscr_asm.EXITFRAME_EVENT_FLAGS,
                               consts=consts, global_names=[B_GLOBAL])


# --------------------------------------------------------------------------
# D6 / D7 variants
#
# There is no license free D6 or D7 donor in the tree, so these clone their
# config, cast library mapping and text member out of a real movie given on the
# command line. The result therefore stays local and must not be committed.
# --------------------------------------------------------------------------
CLONE_TAGS = ("DRCF", "VWCF", "MCsL", "Sord", "VWFI", "FXmp", "Fmap", "Cinf")


class Profile:
    """Everything that differs between Director versions."""

    def __init__(self, name, *, donor, main_size, spr_size, frames_version,
                 sprite_writer, const_entry=8, d5plus=True):
        self.name = name
        self.donor = donor
        self.main_size = main_size
        self.spr_size = spr_size
        self.frames_version = frames_version
        self.sprite = sprite_writer
        self.const_entry = const_entry
        self.d5plus = d5plus


def check_donor_unprotected(donor, path):
    """A protected movie (what a .dxr normally is) carries that state in its
    config chunk, and cloning the chunk carries it along -- the fixture then
    looks protected no matter what it is called, and Director refuses to open
    it. Cast::loadConfig reads the flag as an int16 at offset 58 and treats a
    multiple of 23 as protected (cast.cpp:546).

    ProjectorRays can turn a protected movie into an open one, so a .dxr is
    usable as a donor after running it through `projectorrays decompile`.
    """
    cfg = donor.by_tag("DRCF") or donor.by_tag("VWCF")
    if not cfg:
        raise ValueError(f"{path}: no config chunk")
    body = donor.chunk(cfg[0])
    protection = struct.unpack(">h", body[58:60])[0]
    if protection % 23 == 0:
        raise SystemExit(
            f"{path} is a protected movie (protection={protection}), and the "
            f"fixture would inherit that.\nUse an unprotected .dir, or unprotect "
            f"this one first:\n    projectorrays decompile {path} -o <dir>")


def donor_text_member(donor):
    """Returns (CASt resource, STXT resource) of the donor's first text member."""
    cas = donor.by_tag("CAS*")[0]
    body = donor.chunk(cas)
    slots = [struct.unpack(">I", body[i:i + 4])[0] for i in range(0, len(body), 4)]
    by_index = {r.index: r for r in donor.resources}

    key = donor.chunk(donor.by_tag("KEY*")[0])
    used = struct.unpack(">I", key[8:12])[0]
    children = {}
    for i in range(used):
        off = 12 + i * 12
        child, parent = struct.unpack(">II", key[off:off + 8])
        children.setdefault((parent, key[off + 8:off + 12]), []).append(child)

    for mi in slots:
        res = by_index.get(mi)
        if not res or res.tag != "CASt":
            continue
        cast_type = struct.unpack(">I", donor.chunk(res)[0:4])[0]
        if cast_type != 3:                      # kCastText
            continue
        stxt = children.get((mi, b"STXT"))
        if stxt:
            return res, by_index[stxt[0]]
    raise ValueError("donor has no text member with an STXT")


def make_lscr_for(profile, name_index, code, script_id, consts=(),
                  global_names=()):
    h = lscr_asm.Handler(name_index, code)
    return lscr_asm.build_lscr([h], script_id=script_id, assembly_id=script_id,
                               event_map=[-1] * 10 + [0],
                               event_map_flags=lscr_asm.EXITFRAME_EVENT_FLAGS,
                               consts=consts, global_names=global_names,
                               const_entry_size=profile.const_entry)


def build_variant(profile, kind):
    """kind is 'editable' or 'listoverride'. Cast layout for the donor based
    versions is: 1 = the text member under test, 2 = the result field (a second
    clone of it), 3.. = the scripts."""
    donor = bm.load_donor(profile.donor)
    check_donor_unprotected(donor, profile.donor)
    cast_text, stxt = donor_text_member(donor)

    tested_member, result_member = 1, 2
    chunks = []

    def add(tag, payload):
        chunks.append((tag, payload))
        return 3 + len(chunks) - 1

    i_key = add(b"KEY*", b"")
    i_cas = add(b"CAS*", b"")
    i_vwsc = add(b"VWSC", b"")
    cloned = []
    for tag in CLONE_TAGS:
        for res in donor.by_tag(tag):
            cloned.append((tag, add(tag.encode("latin1"), donor.chunk(res))))
            break

    i_cast_text = add(b"CASt", donor.chunk(cast_text))
    i_stxt = add(b"STXT", donor.chunk(stxt))
    i_cast_res = add(b"CASt", donor.chunk(cast_text))
    i_stxt_res = add(b"STXT", donor.chunk(stxt))

    def const_push(i):
        return lscr_asm.const_push(i, profile.const_entry)

    if kind == "editable":
        names = ["exitFrame", "put", "gEditableResult"]
        gvar, label = 2, LABEL

        def field_report():
            msg = (const_push(0) + lscr_asm.global_push(gvar)
                   + bytes([lscr_asm.OP_AMPERSAND]))
            return (lscr_asm.field_assign_id(result_member, bm.DEFAULT_CAST_LIB)
                    + msg
                    + lscr_asm.the_field_assign(result_member)
                    + msg + lscr_asm.call_command(1, 1)
                    + bytes([lscr_asm.OP_PROCRET]))

        def check(expected, first):
            probe = (lscr_asm.the_sprite_field(SPRITE_CH,
                                               lscr_asm.SPRITE_EDITABLETEXT)
                     + lscr_asm.intpush(expected) + bytes([lscr_asm.OP_EQ]))
            if first:
                return probe + lscr_asm.global_assign(gvar) + bytes([lscr_asm.OP_PROCRET])
            return (lscr_asm.global_push(gvar) + lscr_asm.intpush(10)
                    + bytes([lscr_asm.OP_MUL]) + probe
                    + bytes([lscr_asm.OP_ADD]) + lscr_asm.global_assign(gvar)
                    + bytes([lscr_asm.OP_PROCRET]))

        scripts = [("checkFrame1", check(1, True), src_first(1), ()),
                   ("checkFrame2", check(0, False), src_append(0), ()),
                   ("checkFrame3", check(1, False), src_append(1), ()),
                   ("reportResult", field_report(), src_report(result_member),
                    (label,))]
    else:
        names = B_NAMES
        scripts = None                       # filled in after the factory below

    cast_script_idx, lscr_idx = [], []
    if kind == "editable":
        for i, (nm, code, src, consts) in enumerate(scripts, start=1):
            cast_script_idx.append(add(b"CASt", bm.build_script_cast(
                i, nm, src, bm.SCRIPT_TYPE_SCORE, d5plus=profile.d5plus)))
            lscr_idx.append(add(b"Lscr", make_lscr_for(
                profile, 0, code, i, consts, global_names=[2])))
    else:
        m_new = lscr_asm.Handler(
            B_MNEW,
            (const_push(0) + const_push(1) + lscr_asm.make_list(2)
             + lscr_asm.intpush(2) + lscr_asm.call_function(B_GETLAST, 2)
             + lscr_asm.global_assign(B_GLOBAL) + bytes([lscr_asm.OP_PROCRET])),
            arg_names=[lscr_asm.FACTORY_ME_ARG])
        m_get = lscr_asm.Handler(
            B_GETLAST,
            (const_push(2) + lscr_asm.call_command(B_RETURN, 1)
             + bytes([lscr_asm.OP_PROCRET])),
            arg_names=[lscr_asm.FACTORY_ME_ARG])
        factory = lscr_asm.build_lscr(
            [m_new, m_get], script_id=1, assembly_id=1,
            script_flags=lscr_asm.SCRIPT_FLAG_FACTORY_DEF,
            # see the note in build_listoverride: entry 1 must not own itself
            factory_name_id=B_FACTORY, parent_number=1, properties=[-1],
            global_names=[B_GLOBAL], consts=["IN_OUT", "LINTRANS", "METHOD"],
            const_entry_size=profile.const_entry)
        msg = (const_push(0) + lscr_asm.global_push(B_GLOBAL)
               + bytes([lscr_asm.OP_AMPERSAND]))
        report = (lscr_asm.field_assign_id(result_member, bm.DEFAULT_CAST_LIB)
                  + msg
                  + lscr_asm.the_field_assign(result_member)
                  + msg + lscr_asm.call_command(B_PUT, 1)
                  + bytes([lscr_asm.OP_PROCRET]))
        run = (lscr_asm.object_call(B_MNEW, B_FACTORY, nargs=1, want_result=False)
               + bytes([lscr_asm.OP_PROCRET]))

        cast_script_idx.append(add(b"CASt", bm.build_script_cast(
            1, "MyListFactory", "", bm.SCRIPT_TYPE_MOVIE, d5plus=profile.d5plus)))
        lscr_idx.append(add(b"Lscr", factory))
        for sid, (nm, code, consts) in enumerate(
                [("runFactory", run, ()), ("reportList", report, (B_LABEL,))], start=2):
            cast_script_idx.append(add(b"CASt", bm.build_script_cast(
                sid, nm, "", bm.SCRIPT_TYPE_SCORE, d5plus=profile.d5plus)))
            lscr_idx.append(add(b"Lscr", make_lscr_for(
                profile, B_EXITFRAME, code, sid, consts,
                global_names=[B_GLOBAL])))

    i_lnam = add(b"Lnam", lscr.build_lnam(names))
    i_lctx = add(b"Lctx", bm.build_lctx(lscr_idx, i_lnam))

    members = [i_cast_text, i_cast_res] + cast_script_idx
    chunks[i_cas - 3] = (b"CAS*", bm.build_cas(members))
    for tag, idx in cloned:
        if tag in ("DRCF", "VWCF"):
            chunks[idx - 3] = (tag.encode("latin1"), config.set_cast_array_end(
                chunks[idx - 3][1], len(members)))
        elif tag == "MCsL":
            # Built rather than cloned: from D5 on the member range and the
            # library resource id come from here and override castArrayEnd, so
            # a donor's mapping would describe the donor's libraries, not ours.
            chunks[idx - 3] = (b"MCsL", bm.build_mcsl(
                "Internal", 1, len(members), bm.CASTLIB_KEY_PARENT))

    key_entries = [(i_stxt, i_cast_text, b"STXT"), (i_stxt_res, i_cast_res, b"STXT"),
                   (i_cas, bm.CASTLIB_KEY_PARENT, b"CAS*"),
                   (i_vwsc, bm.CASTLIB_KEY_PARENT, b"VWSC"),
                   (i_lctx, bm.CASTLIB_KEY_PARENT, b"Lctx"),
                   (i_lnam, bm.CASTLIB_KEY_PARENT, b"Lnam")]
    for tag, idx in cloned:
        key_entries.append((idx, bm.CASTLIB_KEY_PARENT, tag.encode("latin1")))
    for c, s in zip(cast_script_idx, lscr_idx):
        key_entries.append((s, c, b"Lscr"))
    chunks[i_key - 3] = (b"KEY*", bm.build_key(key_entries, max_entries=32))

    first_script_member = 3

    def frame(action_member, *, editable=False, back=0):
        chans = {0: bm.main_channel_d6plus(profile.main_size,
                                           action_id=action_member),
                 RESULT_CH: profile.sprite(cast_member=result_member,
                                           x=20, y=120, w=200, h=40)}
        if kind == "editable":
            chans[SPRITE_CH] = profile.sprite(cast_member=tested_member, x=20,
                                              y=20, w=200, h=40,
                                              editable=editable, back=back)
        return chans

    if kind == "editable":
        frames = [frame(first_script_member + 0, editable=True, back=0),
                  frame(first_script_member + 1, editable=False, back=255),
                  frame(first_script_member + 2, editable=True, back=255),
                  frame(first_script_member + 3, editable=True, back=255)]
    else:
        frames = [frame(first_script_member + 1), frame(first_script_member + 2)]

    chunks[i_vwsc - 3] = (b"VWSC", bm.build_vwsc_d6plus(
        frames, main_size=profile.main_size, spr_size=profile.spr_size,
        frames_version=profile.frames_version))

    data = bm.build_rifx(chunks, imap_version=donor.version)
    path = OUT / f"{kind}-{profile.name}.dir"
    path.write_bytes(data)
    return path, data


def build_probe(profile):
    """A bisecting movie: the donor's own score, untouched, plus our two text
    members and nothing else -- no scripts, no generated VWSC.

    Director rejects the D6 and D7 fixtures without saying why. If this one
    loads, the cast side is sound and the fault is in the score we generate; if
    it does not, the fault lies earlier, in the cast, the MCsL or the container.
    """
    donor = bm.load_donor(profile.donor)
    check_donor_unprotected(donor, profile.donor)
    cast_text, stxt = donor_text_member(donor)

    chunks = []

    def add(tag, payload):
        chunks.append((tag, payload))
        return 3 + len(chunks) - 1

    i_key = add(b"KEY*", b"")
    i_cas = add(b"CAS*", b"")
    i_vwsc = add(b"VWSC", donor.chunk(donor.by_tag("VWSC")[0]))
    cloned = []
    for tag in CLONE_TAGS:
        for res in donor.by_tag(tag):
            cloned.append((tag, add(tag.encode("latin1"), donor.chunk(res))))
            break

    i_cast_text = add(b"CASt", donor.chunk(cast_text))
    i_stxt = add(b"STXT", donor.chunk(stxt))
    i_cast_res = add(b"CASt", donor.chunk(cast_text))
    i_stxt_res = add(b"STXT", donor.chunk(stxt))

    members = [i_cast_text, i_cast_res]
    chunks[i_cas - 3] = (b"CAS*", bm.build_cas(members))
    for tag, idx in cloned:
        if tag in ("DRCF", "VWCF"):
            chunks[idx - 3] = (tag.encode("latin1"), config.set_cast_array_end(
                chunks[idx - 3][1], len(members)))
        elif tag == "MCsL":
            chunks[idx - 3] = (b"MCsL", bm.build_mcsl(
                "Internal", 1, len(members), bm.CASTLIB_KEY_PARENT))

    entries = [(i_stxt, i_cast_text, b"STXT"), (i_stxt_res, i_cast_res, b"STXT"),
               (i_cas, bm.CASTLIB_KEY_PARENT, b"CAS*"),
               (i_vwsc, bm.CASTLIB_KEY_PARENT, b"VWSC")]
    for tag, idx in cloned:
        entries.append((idx, bm.CASTLIB_KEY_PARENT, tag.encode("latin1")))
    chunks[i_key - 3] = (b"KEY*", bm.build_key(entries, max_entries=24))

    data = bm.build_rifx(chunks, imap_version=donor.version)
    path = OUT / f"probe-{profile.name}.dir"
    path.write_bytes(data)
    return path, data


def build_probe_d4():
    """The D4 counterpart of build_probe: our container and cast, but the
    donor's own score, which holds one frame with one sprite.

    Director 5 loads editable.dir and shows its cast correctly, yet its score
    window stays empty. If this movie shows the donor's sprite, the container
    and the cast are fine and the fault is in the VWSC we generate; if it does
    not, the score is being rejected for a reason that has nothing to do with
    how we lay out frames.
    """
    donor = bm.load_donor()
    keep = {}
    for r in donor.resources:
        if r.tag in ("VWCF", "Sord", "VWFI", "VWFM", "CASt", "STXT", "VWSC"):
            keep.setdefault(r.tag, []).append(r)

    chunks = []

    def add(tag, payload):
        chunks.append((tag, payload))
        return 3 + len(chunks) - 1

    i_key = add(b"KEY*", b"")
    i_vwcf = add(b"VWCF", donor.chunk(keep["VWCF"][0]))
    i_cas = add(b"CAS*", b"")
    i_sord = add(b"Sord", donor.chunk(keep["Sord"][0]))
    i_vwfi = add(b"VWFI", donor.chunk(keep["VWFI"][0]))
    i_vwfm = add(b"VWFM", donor.chunk(keep["VWFM"][0]))
    i_vwsc = add(b"VWSC", donor.chunk(keep["VWSC"][0]))     # untouched
    i_cast_text = add(b"CASt", donor.chunk(keep["CASt"][0]))
    i_stxt = add(b"STXT", donor.chunk(keep["STXT"][0]))
    i_cast_shape = add(b"CASt", donor.chunk(keep["CASt"][1]))

    members = [i_cast_text, i_cast_shape]
    chunks[i_cas - 3] = (b"CAS*", bm.build_cas(members))
    chunks[i_vwcf - 3] = (b"VWCF", config.set_cast_array_end(
        chunks[i_vwcf - 3][1], len(members)))
    chunks[i_key - 3] = (b"KEY*", bm.build_key([
        (i_stxt, i_cast_text, b"STXT"),
        (i_cas, bm.CASTLIB_KEY_PARENT, b"CAS*"),
        (i_sord, bm.CASTLIB_KEY_PARENT, b"Sord"),
        (i_vwcf, bm.CASTLIB_KEY_PARENT, b"VWCF"),
        (i_vwfi, bm.CASTLIB_KEY_PARENT, b"VWFI"),
        (i_vwfm, bm.CASTLIB_KEY_PARENT, b"VWFM"),
        (i_vwsc, bm.CASTLIB_KEY_PARENT, b"VWSC"),
    ], max_entries=16))

    data = bm.build_rifx(chunks)
    path = OUT / "probe-d4.dir"
    path.write_bytes(data)
    return path, data


def report(path, data):
    print(f"wrote {path.name} ({len(data)} bytes)")
    f = rifx.RifxFile(data)
    for r in f.by_tag("Lscr"):
        s = lscr.Lscr(f.chunk(r))
        flags = f" FACTORY({s.factory_name_id})" if s.script_flags & 0x10 else ""
        for fn in s.functions:
            print(f"   Lscr {r.index}{flags} args={fn.arg_names} "
                  f"code={fn.code.hex(' ')}")


# --------------------------------------------------------------------------
# Bisection ladder for the empty score in Director
#
# probe-d4.dir (our container and cast, the donor's score) renders in Director;
# editable.dir (the same, our score) leaves the score window empty. Everything
# outside the VWSC is therefore ruled out. These five movies close that gap one
# change at a time, keeping cast and container identical throughout, so the
# first one that comes up empty names the cause.
# --------------------------------------------------------------------------
def donor_sprite_record():
    """The donor's own 16 byte sprite record for channel 1, padded to 20."""
    donor = bm.load_donor()
    s = donor.chunk(donor.by_tag("VWSC")[0])
    frame1 = struct.unpack(">I", s[4:8])[0]
    size, off = struct.unpack(">HH", s[frame1 + 2:frame1 + 6])
    return s[frame1 + 6:frame1 + 6 + size] + b"\0" * (20 - size)


def build_ladder():
    donor = bm.load_donor()
    donor_vwsc = donor.chunk(donor.by_tag("VWSC")[0])
    rec = donor_sprite_record()
    mine = bm.sprite_d4(cast_member=TEXT_MEMBER, x=20, y=20, w=200, h=40,
                        editable=True)
    made = []

    def emit(letter, what, vwsc):
        path, data = build_editable(vwsc, "ladder-%s.dir" % letter)
        made.append((letter, what, path.name, len(data)))

    # a: the donor's score verbatim -- the known good end, now next to our
    #    full cast rather than the two members probe-d4 carries
    emit("a", "donor score, unchanged", donor_vwsc)

    # b: our frame writer, but emitting the donor's own sprite bytes
    emit("b", "our writer, donor's sprite bytes",
         bm.build_vwsc([{SPRITE_CH: rec}]))

    # c: as b, with our geometry and colours
    emit("c", "our writer, our sprite bytes", bm.build_vwsc([{SPRITE_CH: mine}]))

    # d: as c, plus the frame script in the main channel
    emit("d", "plus the frame script",
         bm.build_vwsc([{0: bm.main_channel_d4(action_id=4), SPRITE_CH: mine}]))

    # e: as d, but four frames instead of one
    emit("e", "plus four frames", bm.build_vwsc(
        [{0: bm.main_channel_d4(action_id=4 + i), SPRITE_CH: mine}
         for i in range(4)]))

    return made


# --------------------------------------------------------------------------
# Bisection ladder for the D6/D7 load failure
#
# Unlike D4, even probe-d6/d7 -- which carries the donor's own score -- is
# refused, so the fault lies before the score. These walk from a faithful copy
# of the donor towards our fixture, one transformation at a time.
#
# Note what the first step already exercises: rebuilding the container renumbers
# every mmap index, and KEY* refers to chunks by index, so its entries have to
# be remapped. Cloning a KEY* verbatim into a rebuilt container would point
# every relationship at the wrong chunk.
# --------------------------------------------------------------------------
DROPPED_TAGS = ("VWLB", "VERS", "FCOL", "XTRl", "SCRF", "XMED", "Fmap", "Cinf")


def _remap_lctx(chunk, old_to_new):
    """Lctx points at chunks by mmap index too -- its nameTableId names the Lnam
    and every entry names an Lscr -- so renumbering the map breaks it just as it
    breaks KEY*. Missing this is why the faithful clones crashed ProjectorRays
    while the more heavily rewritten ones did not."""
    out = bytearray(chunk)
    name_table = struct.unpack_from(">i", out, 32)[0]
    if name_table in old_to_new:
        struct.pack_into(">i", out, 32, old_to_new[name_table])

    count = struct.unpack_from(">i", out, 8)[0]
    items_offset = struct.unpack_from(">H", out, 16)[0]
    entry_size = struct.unpack_from(">H", out, 18)[0] or 12
    for i in range(count):
        pos = items_offset + i * entry_size + 4
        if pos + 4 > len(out):
            break
        idx = struct.unpack_from(">i", out, pos)[0]
        if idx in old_to_new:
            struct.pack_into(">i", out, pos, old_to_new[idx])
    return bytes(out)


def build_donor_clone(donor_path, out_name, drop=(), rebuild_mcsl=False,
                      patch_config=False):
    """Re-emits a donor through our own container writer.

    `drop` names tags to leave out, `rebuild_mcsl` swaps in a generated cast
    library mapping, `patch_config` rewrites castArrayEnd and the checksum. With
    all three off the result should be the donor in everything but chunk order
    and mmap numbering.
    """
    donor = bm.load_donor(donor_path)
    keep = [r for r in donor.resources
            if r.tag not in ("RIFX", "XFIR", "imap", "mmap", "free", "junk")
            and r.tag not in drop]

    chunks, old_to_new = [], {}
    for res in keep:
        payload = donor.chunk(res)
        chunks.append([res.tag.encode("latin1"), payload])
        old_to_new[res.index] = 3 + len(chunks) - 1

    # KEY* refers to chunks by mmap index, which we have just renumbered
    key_src = donor.chunk(donor.by_tag("KEY*")[0])
    used = struct.unpack(">I", key_src[8:12])[0]
    entries = []
    for i in range(used):
        off = 12 + i * 12
        child, parent = struct.unpack(">II", key_src[off:off + 8])
        tag = key_src[off + 8:off + 12]
        if child not in old_to_new:
            continue                     # child was dropped
        # a parent is either another chunk or a cast library id such as 1024
        entries.append((old_to_new[child], old_to_new.get(parent, parent), tag))

    for chunk in chunks:
        if chunk[0] == b"KEY*":
            chunk[1] = bm.build_key(entries, max_entries=max(24, len(entries)))
        elif chunk[0] == b"Lctx":
            chunk[1] = _remap_lctx(chunk[1], old_to_new)
        elif rebuild_mcsl and chunk[0] == b"MCsL":
            cas = donor.chunk(donor.by_tag("CAS*")[0])
            chunk[1] = bm.build_mcsl("Internal", 1, len(cas) // 4,
                                     bm.CASTLIB_KEY_PARENT)
        elif patch_config and chunk[0] in (b"DRCF", b"VWCF"):
            cas = donor.chunk(donor.by_tag("CAS*")[0])
            chunk[1] = config.set_cast_array_end(chunk[1], len(cas) // 4)

    data = bm.build_rifx([(t, p) for t, p in chunks],
                         imap_version=donor.version)
    path = OUT / out_name
    path.write_bytes(data)
    return path, data


def build_ladder_d6d7(profile):
    """v1 the donor through our container, v2 without the chunks we normally
    drop, v3 with a generated MCsL, v4 with the config patched as well."""
    n = profile.name
    made = []
    for letter, what, kw in (
            ("1", "donor through our container", {}),
            ("2", "without the chunks we drop", {"drop": DROPPED_TAGS}),
            ("3", "plus a generated MCsL",
             {"drop": DROPPED_TAGS, "rebuild_mcsl": True}),
            ("4", "plus the patched config",
             {"drop": DROPPED_TAGS, "rebuild_mcsl": True,
              "patch_config": True})):
        path, data = build_donor_clone(profile.donor, f"ladder-{n}-{letter}.dir",
                                       **kw)
        made.append((f"{n}-{letter}", what, path.name, len(data)))
    return made



def sprite_details(spans):
    """Allocates the detail entries a D6+ score needs and returns their indices.

    Every sprite and every frame script points at a detail index, and it may not
    be 0 -- across 577649 records in the archive not one is. Each index claims
    three entries: the SpriteInfo, the behaviour list and the name. Ours carry no
    names, so that slot stays empty, but it still has to exist or the entry after
    it is read as its own.

    `spans` is a list of (startFrame, endFrame, channel) with an optional fourth
    element, the cast member of a behaviour to attach. A frame script needs it:
    the main channel's actionId alone gets a script the Score window shows and
    Director never runs. Index 0 belongs to the score itself, so ours start at 3.
    """
    # Slots 1 and 2 belong to the entry before ours: index 0's SpriteInfo slot is
    # entry 0, which is the score, so its behaviour and name slots sit unused
    # right after it. Ours therefore start at 3, and every index is a multiple of
    # 3 -- which is how ScummVM labels them when it audits unread entries
    # (`int type = i % 3`, score.cpp:2016) and what both donors do: 3 and 6 on
    # D6, 84 and 87 on D7.
    entries, indices, n = [None, b""], [], 3
    for span in spans:
        start, end, channel = span[:3]
        behaviors = bm.build_behavior_list(span[3:4]) if len(span) > 3 else b""
        indices.append(n)
        entries += [bm.build_sprite_info(start_frame=start, end_frame=end,
                                         channel=channel), behaviors, b""]
        n += 3
    entries[0] = bm.build_detail_directory(indices)
    return indices, entries


def build_editable_append(profile, trivial=False, tag=None):
    """The D6/D7 editable fixture, built by appending to the donor.

    ladder-d6-0x showed that rebuilding the index space is what Director chokes
    on, so nothing here moves: our chunks take indices after the donor's last,
    the name table and script context are extended rather than replaced, and
    only CAS*, MCsL and VWSC are swapped in place.
    """
    donor = bm.load_donor(profile.donor)
    check_donor_unprotected(donor, profile.donor)
    cast_text, cast_stxt = donor_text_member(donor)

    next_index = max(r.index for r in donor.resources) + 1
    cas = donor.chunk(donor.by_tag("CAS*")[0])
    donor_members = [struct.unpack(">I", cas[i:i + 4])[0]
                     for i in range(0, len(cas), 4)]

    lnam_res = donor.by_tag("Lnam")[0]
    lnam, name_base = bm.extend_lnam(donor.chunk(lnam_res), NAMES)
    if name_base + len(NAMES) > 0xffff:
        raise SystemExit("donor's name table is too long")
    n_exit, n_put, n_global = (name_base + i for i in range(3))

    def const_push(i):
        return lscr_asm.const_push(i, profile.const_entry)

    probe = (lscr_asm.the_sprite_field(SPRITE_CH, lscr_asm.SPRITE_EDITABLETEXT))

    def check(expected, first):
        tail = (lscr_asm.intpush(expected) + bytes([lscr_asm.OP_EQ]))
        if first:
            return probe + tail + lscr_asm.global_assign(n_global) + \
                bytes([lscr_asm.OP_PROCRET])
        return (lscr_asm.global_push(n_global) + lscr_asm.intpush(10)
                + bytes([lscr_asm.OP_MUL]) + probe + tail
                + bytes([lscr_asm.OP_ADD]) + lscr_asm.global_assign(n_global)
                + bytes([lscr_asm.OP_PROCRET]))

    # Both text members are clones we append. Reaching into the donor's own
    # numbering does not survive a change of donor: member 1 is a text member in
    # the D6 donor but a script in the D7 one, and a text sprite pointing at a
    # script member takes Director 7 down.
    tested_member = len(donor_members) + 1
    result_member = len(donor_members) + 2
    msg = (const_push(0) + lscr_asm.global_push(n_global)
           + bytes([lscr_asm.OP_AMPERSAND]))
    report = (lscr_asm.field_assign_id(result_member, bm.DEFAULT_CAST_LIB) + msg
              + lscr_asm.the_field_assign(result_member)
              + msg + lscr_asm.call_command(n_put, 1)
              + bytes([lscr_asm.OP_PROCRET]))

    scripts = [("checkFrame1", check(1, True), src_first(1), ()),
               ("checkFrame2", check(0, False), src_append(0), ()),
               ("checkFrame3", check(1, False), src_append(1), ()),
               ("reportResult", report, src_report(result_member), (LABEL,))]

    if trivial:
        # The source and the bytecode deliberately disagree, and each writes its
        # own name into the result field. Whatever the field ends up saying names
        # what Director actually executed:
        #
        #   FROM BYTECODE   it runs the Lscr we assembled
        #   FROM SOURCE     it ignores the Lscr and compiles the cast info text
        #   nothing at all  the frame scripts never run
        #
        # Nothing here touches a sprite property, a global or arithmetic, so a
        # blank field cannot be blamed on the thing under test.
        # Two ways out, because one that stays silent cannot say why: the field
        # on stage and the message window. If only the message appears, the
        # script runs and writing a field is what fails.
        write_const = (lscr_asm.field_assign_id(result_member,
                                                bm.DEFAULT_CAST_LIB)
                       + const_push(0)
                       + lscr_asm.the_field_assign(result_member)
                       + const_push(0)
                       + lscr_asm.call_command(n_put, 1)
                       + bytes([lscr_asm.OP_PROCRET]))
        nothing = bytes([lscr_asm.OP_PROCRET])
        src_write = ('on exitFrame\r'
                     '  set the text of field %d to "%s"\r'
                     '  put "%s"\r'
                     'end\r' % (result_member, TRIVIAL_FROM_SOURCE,
                                TRIVIAL_FROM_SOURCE))
        src_nothing = "on exitFrame\rend\r"
        scripts = [("writeConstant", write_const, src_write,
                    (TRIVIAL_FROM_BYTECODE,)),
                   ("doNothing2", nothing, src_nothing, ()),
                   ("doNothing3", nothing, src_nothing, ()),
                   ("doNothing4", nothing, src_nothing, ())]

    # indices: the two text members with their STXT, then a CASt and an Lscr
    # per script
    i_test, i_test_stxt = next_index, next_index + 1
    i_res, i_res_stxt = next_index + 2, next_index + 3
    script_cast = [next_index + 4 + 2 * i for i in range(len(scripts))]
    script_lscr = [next_index + 5 + 2 * i for i in range(len(scripts))]

    lctx_res = donor.by_tag("Lctx")[0]
    lctx, script_base = bm.extend_lctx(donor.chunk(lctx_res), script_lscr)

    extra = [(b"CASt", donor.chunk(cast_text)),
             (b"STXT", donor.chunk(cast_stxt)),
             (b"CASt", donor.chunk(cast_text)),
             (b"STXT", donor.chunk(cast_stxt))]
    for i, (nm, code, src, consts) in enumerate(scripts):
        extra.append((b"CASt", bm.build_script_cast(
            script_base + i, nm, src, bm.SCRIPT_TYPE_SCORE, d5plus=True)))
        extra.append((b"Lscr", lscr_asm.build_lscr(
            [lscr_asm.Handler(n_exit, code)],
            script_id=script_base + i, assembly_id=script_base + i,
            event_map=[-1] * 10 + [0],
            event_map_flags=lscr_asm.EXITFRAME_EVENT_FLAGS,
            consts=consts, global_names=[n_global],
            const_entry_size=profile.const_entry)))

    members = donor_members + [i_test, i_res] + script_cast
    first_script_member = result_member + 1

    # both sprites span all four frames; each frame script is its own one frame
    # span in the script channel
    idx, details = sprite_details(
        [(1, 4, SPRITE_CH), (1, 4, RESULT_CH)]
        + [(i + 1, i + 1, 0, first_script_member + i) for i in range(4)])

    def frame(i, editable, back):
        return {0: bm.main_channel_d6plus(profile.main_size,
                                          action_id=first_script_member + i,
                                          script_list_idx=idx[2 + i]),
                SPRITE_CH: profile.sprite(cast_member=tested_member,
                                          x=20, y=20, w=200,
                                          h=40, editable=editable, back=back,
                                          list_idx=idx[0]),
                RESULT_CH: profile.sprite(cast_member=result_member,
                                          x=20, y=120, w=200, h=40,
                                          list_idx=idx[1])}

    vwsc = bm.build_vwsc_d6plus(
        [frame(0, True, 0), frame(1, False, 255),
         frame(2, True, 255), frame(3, True, 255)],
        main_size=profile.main_size, spr_size=profile.spr_size,
        frames_version=profile.frames_version,
        geometry=bm.donor_score_geometry(donor), details=details)

    # Raising castArrayEnd is what makes an appended member exist for Director;
    # ScummVM takes the range from MCsL on D5+ and never reads it.
    cfg = (donor.by_tag("DRCF") or donor.by_tag("VWCF"))[0]
    replace = {cfg.index: config.set_cast_array_end(donor.chunk(cfg),
                                                    len(members)),
               donor.by_tag("CAS*")[0].index: bm.build_cas(members),
               donor.by_tag("MCsL")[0].index: bm.build_mcsl(
                   "Internal", 1, len(members), bm.CASTLIB_KEY_PARENT),
               donor.by_tag("VWSC")[0].index: vwsc,
               lnam_res.index: lnam,
               lctx_res.index: lctx}

    key_extra = [(i_test_stxt, i_test, b"STXT"), (i_res_stxt, i_res, b"STXT")]
    key_extra += [(l, c, b"Lscr") for c, l in zip(script_cast, script_lscr)]

    data = bm.rebuild_preserving_indices(donor, extra=extra,
                                         extra_key=key_extra, replace=replace)
    path = OUT / f"{tag or ('editable-' + profile.name + '-append')}.dir"
    path.write_bytes(data)
    return path, data


def build_score_probe(profile):
    """Donor plus two text members plus our own score -- and nothing else.

    A black stage is not an answer on its own: ladder-d6-0x shows one too, and
    that one carries the donor's untouched score. This puts our generated VWSC
    on a cast that is already known to load, with no scripts anywhere near it,
    so what the stage shows answers exactly one question -- can Director read a
    score we wrote?
    """
    donor = bm.load_donor(profile.donor)
    check_donor_unprotected(donor, profile.donor)
    cast_text, cast_stxt = donor_text_member(donor)

    next_index = max(r.index for r in donor.resources) + 1
    cas = donor.chunk(donor.by_tag("CAS*")[0])
    donor_members = [struct.unpack(">I", cas[i:i + 4])[0]
                     for i in range(0, len(cas), 4)]

    i_a, i_a_stxt, i_b, i_b_stxt = (next_index + n for n in range(4))
    member_a, member_b = len(donor_members) + 1, len(donor_members) + 2
    extra = [(b"CASt", donor.chunk(cast_text)), (b"STXT", donor.chunk(cast_stxt)),
             (b"CASt", donor.chunk(cast_text)), (b"STXT", donor.chunk(cast_stxt))]
    members = donor_members + [i_a, i_b]

    idx, details = sprite_details([(1, 4, SPRITE_CH), (1, 4, RESULT_CH)])

    def frame(editable, back):
        return {SPRITE_CH: profile.sprite(cast_member=member_a, x=20, y=20,
                                          w=200, h=40, editable=editable,
                                          back=back, list_idx=idx[0]),
                RESULT_CH: profile.sprite(cast_member=member_b, x=20, y=120,
                                          w=200, h=40, list_idx=idx[1])}

    vwsc = bm.build_vwsc_d6plus(
        [frame(True, 0), frame(False, 255), frame(True, 255), frame(True, 255)],
        main_size=profile.main_size, spr_size=profile.spr_size,
        frames_version=profile.frames_version,
        geometry=bm.donor_score_geometry(donor), details=details)

    # Raising castArrayEnd is what makes an appended member exist for Director;
    # ScummVM takes the range from MCsL on D5+ and never reads it.
    cfg = (donor.by_tag("DRCF") or donor.by_tag("VWCF"))[0]
    replace = {cfg.index: config.set_cast_array_end(donor.chunk(cfg),
                                                    len(members)),
               donor.by_tag("CAS*")[0].index: bm.build_cas(members),
               donor.by_tag("MCsL")[0].index: bm.build_mcsl(
                   "Internal", 1, len(members), bm.CASTLIB_KEY_PARENT),
               donor.by_tag("VWSC")[0].index: vwsc}

    key_extra = [(i_a_stxt, i_a, b"STXT"), (i_b_stxt, i_b, b"STXT")]
    data = bm.rebuild_preserving_indices(donor, extra=extra,
                                         extra_key=key_extra, replace=replace)
    path = OUT / f"score-{profile.name}.dir"
    path.write_bytes(data)
    return path, data


def build_graft_probe(profile):
    """The donor's own score, with exactly one channel write grafted into it.

    score-d6 rebuilds the score from scratch and Director shows nothing. This
    changes as little as possible instead: the donor's header stays byte for
    byte, its frames keep the main channel state they set up, and frame 1 gains
    a single sprite write for a text member we appended. Only the lengths that
    have to follow are touched.

        it shows the text -- the container and header are right and the fault is
                             in how we build frames
        it stays empty    -- the fault is in the header or the geometry, because
                             nothing else is left
    """
    donor = bm.load_donor(profile.donor)
    check_donor_unprotected(donor, profile.donor)
    cast_text, cast_stxt = donor_text_member(donor)

    next_index = max(r.index for r in donor.resources) + 1
    cas = donor.chunk(donor.by_tag("CAS*")[0])
    donor_members = [struct.unpack(">I", cas[i:i + 4])[0]
                     for i in range(0, len(cas), 4)]
    member = len(donor_members) + 1

    b = donor.chunk(donor.by_tag("VWSC")[0])
    list_start = struct.unpack(">I", b[8:12])[0]
    list_size = struct.unpack(">I", b[list_start + 4:list_start + 8])[0]
    index_start = list_start + 12
    data = index_start + list_size * 4
    donor_offsets = [struct.unpack(">I", b[index_start + 4 * i:index_start + 4 * i + 4])[0]
                     for i in range(list_size)]
    entry0 = bytearray(b[data + donor_offsets[0]:data + donor_offsets[1]])
    size, frame1 = struct.unpack(">II", entry0[0:8])

    # frame 1 keeps everything it had and gains one write at the start of
    # sprite channel 1
    # The donor's whole index survives here, so its detail entries stay valid;
    # entry 3 is the triple its own frame script uses and serves ours as well.
    rec = profile.sprite(cast_member=member, x=20, y=20, w=200, h=40,
                         editable=True, list_idx=3)
    write = struct.pack(">HH", len(rec), profile.main_size) + rec
    fsz = struct.unpack(">H", entry0[frame1:frame1 + 2])[0]
    grafted = (entry0[:frame1]
               + struct.pack(">H", fsz + len(write))
               + entry0[frame1 + 2:frame1 + fsz]
               + write
               + entry0[frame1 + fsz:size])
    struct.pack_into(">I", grafted, 0, size + len(write))
    if len(grafted) % 2:
        grafted += b"\0"

    # Keep every detail entry the donor has. Dropping them while its frame
    # scripts -- and our sprite -- still point into them leaves the score full of
    # dangling indices, which is what an earlier version of this probe did.
    tail = [b[data + o:data + n]
            for o, n in zip(donor_offsets, donor_offsets[1:])][1:]
    entries = [bytes(grafted)] + tail
    offsets, pos = [], 0
    for e in entries:
        offsets.append(pos)
        pos += len(e)

    out = bytearray()
    out += struct.pack(">I", 0)
    out += struct.pack(">i", -3)
    out += struct.pack(">I", 12)
    out += struct.pack(">III", len(entries), len(entries) + 1, pos)
    for off in offsets:
        out += struct.pack(">I", off)
    out += struct.pack(">I", pos)
    out += b"".join(entries)
    struct.pack_into(">I", out, 0, len(out))

    extra = [(b"CASt", donor.chunk(cast_text)), (b"STXT", donor.chunk(cast_stxt))]
    cfg = (donor.by_tag("DRCF") or donor.by_tag("VWCF"))[0]
    replace = {cfg.index: config.set_cast_array_end(donor.chunk(cfg),
                                                    len(donor_members) + 1),
               donor.by_tag("CAS*")[0].index: bm.build_cas(donor_members + [next_index]),
               donor.by_tag("MCsL")[0].index: bm.build_mcsl(
                   "Internal", 1, len(donor_members) + 1, bm.CASTLIB_KEY_PARENT),
               donor.by_tag("VWSC")[0].index: bytes(out)}
    data_out = bm.rebuild_preserving_indices(
        donor, extra=extra, extra_key=[(next_index + 1, next_index, b"STXT")],
        replace=replace)
    path = OUT / f"graft-{profile.name}.dir"
    path.write_bytes(data_out)
    return path, data_out


# --------------------------------------------------------------------------
# Bisection ladder for the empty stage, cast side
#
# The Director 5 run of ladder-a settled something the earlier reasoning had
# backwards: ladder-a carries the donor's score *unchanged* and its stage is
# still empty, while probe-d4 -- same score, same container -- renders. So the
# score was never the cause, and probe-d4 is the only fixture whose config
# chunk we do not rewrite and whose cast we do not extend. These four rungs walk
# that gap one change at a time.
# --------------------------------------------------------------------------
def build_cast_ladder():
    """probe-d4 renders, ladder-a does not, and they share a byte identical
    VWSC. Everything between them is cast: how many members, whether any of
    them are scripts, and whether the movie carries a Lingo context."""
    donor = bm.load_donor()
    keep = {}
    for r in donor.resources:
        if r.tag in ("VWCF", "Sord", "VWFI", "VWFM", "CASt", "STXT", "VWSC"):
            keep.setdefault(r.tag, []).append(r)
    made = []

    for letter, n_extra_text, n_scripts, lingo, what in (
            ("a", 0, 0, False, "2 members -- probe-d4, the control"),
            ("b", 1, 0, False, "3 members, one more text member"),
            ("c", 1, 4, False, "7 members, the scripts without their code"),
            ("d", 1, 4, True, "7 members with Lscr, Lnam and Lctx")):
        chunks = []

        def add(tag, payload):
            chunks.append((tag, payload))
            return 3 + len(chunks) - 1

        i_key = add(b"KEY*", b"")
        i_vwcf = add(b"VWCF", donor.chunk(keep["VWCF"][0]))
        i_cas = add(b"CAS*", b"")
        i_sord = add(b"Sord", donor.chunk(keep["Sord"][0]))
        i_vwfi = add(b"VWFI", donor.chunk(keep["VWFI"][0]))
        i_vwfm = add(b"VWFM", donor.chunk(keep["VWFM"][0]))
        i_vwsc = add(b"VWSC", donor.chunk(keep["VWSC"][0]))
        i_text = add(b"CASt", donor.chunk(keep["CASt"][0]))
        i_stxt = add(b"STXT", donor.chunk(keep["STXT"][0]))
        i_shape = add(b"CASt", donor.chunk(keep["CASt"][1]))

        key = [(i_stxt, i_text, b"STXT")]
        members = [i_text, i_shape]
        if n_extra_text:
            i_res = add(b"CASt", donor.chunk(keep["CASt"][0]))
            i_res_stxt = add(b"STXT", donor.chunk(keep["STXT"][0]))
            key.append((i_res_stxt, i_res, b"STXT"))
            members.append(i_res)

        scripts = [("checkFrame1", code_first(1), src_first(1), ()),
                   ("checkFrame2", code_append(0), src_append(0), ()),
                   ("checkFrame3", code_append(1), src_append(1), ()),
                   ("reportResult", code_report(), src_report(), (LABEL,))]
        lscr_idx = []
        for i, (name, code, src, consts) in enumerate(scripts[:n_scripts], 1):
            members.append(add(b"CASt", bm.build_script_cast(i, name, src)))
            if lingo:
                lscr_idx.append(add(b"Lscr", make_lscr(code, i, consts)))
                key.append((lscr_idx[-1], members[-1], b"Lscr"))
        if lingo:
            i_lnam = add(b"Lnam", lscr.build_lnam(NAMES))
            i_lctx = add(b"Lctx", bm.build_lctx(lscr_idx, i_lnam))
            key += [(i_lctx, bm.CASTLIB_KEY_PARENT, b"Lctx"),
                    (i_lnam, bm.CASTLIB_KEY_PARENT, b"Lnam")]

        chunks[i_cas - 3] = (b"CAS*", bm.build_cas(members))
        chunks[i_vwcf - 3] = (b"VWCF", config.set_cast_array_end(
            chunks[i_vwcf - 3][1], len(members)))
        key += [(i_cas, bm.CASTLIB_KEY_PARENT, b"CAS*"),
                (i_sord, bm.CASTLIB_KEY_PARENT, b"Sord"),
                (i_vwcf, bm.CASTLIB_KEY_PARENT, b"VWCF"),
                (i_vwfi, bm.CASTLIB_KEY_PARENT, b"VWFI"),
                (i_vwfm, bm.CASTLIB_KEY_PARENT, b"VWFM"),
                (i_vwsc, bm.CASTLIB_KEY_PARENT, b"VWSC")]
        chunks[i_key - 3] = (b"KEY*", bm.build_key(key, max_entries=24))

        data = bm.build_rifx(chunks)
        path = OUT / ("castladder-%s.dir" % letter)
        path.write_bytes(data)
        made.append((letter, what, path.name, len(data)))
    return made


def build_graft_minimal(profile):
    """The donor, untouched, plus one channel write. Nothing else at all.

    Every other probe still changes the cast: it appends a text member, rewrites
    CAS* and MCsL, and moves castArrayEnd. This one changes exactly one thing --
    frame 1 of the score gains a sprite showing a member the donor already has,
    with the detail index one of its own sprites already uses. If Director shows
    nothing even here, then writing a channel is what we get wrong, and no part
    of the cast is involved.
    """
    donor = bm.load_donor(profile.donor)
    check_donor_unprotected(donor, profile.donor)

    cast_text, _ = donor_text_member(donor)
    cas = donor.chunk(donor.by_tag("CAS*")[0])
    members = [struct.unpack(">I", cas[i:i + 4])[0]
               for i in range(0, len(cas), 4)]
    member = members.index(cast_text.index) + 1

    b = donor.chunk(donor.by_tag("VWSC")[0])
    list_start = struct.unpack(">I", b[8:12])[0]
    list_size = struct.unpack(">I", b[list_start + 4:list_start + 8])[0]
    index_start = list_start + 12
    data = index_start + list_size * 4
    offs = [struct.unpack(">I", b[index_start + 4 * i:index_start + 4 * i + 4])[0]
            for i in range(list_size)]
    entry0 = bytearray(b[data + offs[0]:data + offs[1]])
    size, frame1 = struct.unpack(">II", entry0[0:8])

    rec = profile.sprite(cast_member=member, x=20, y=20, w=200, h=40,
                         editable=True, list_idx=3)
    write = struct.pack(">HH", len(rec), profile.main_size) + rec
    fsz = struct.unpack(">H", entry0[frame1:frame1 + 2])[0]
    grafted = (entry0[:frame1] + struct.pack(">H", fsz + len(write))
               + entry0[frame1 + 2:frame1 + fsz] + write
               + entry0[frame1 + fsz:size])
    struct.pack_into(">I", grafted, 0, size + len(write))
    if len(grafted) % 2:
        grafted += b"\0"

    entries = [bytes(grafted)] + [b[data + o:data + n]
                                  for o, n in zip(offs, offs[1:])][1:]
    positions, pos = [], 0
    for e in entries:
        positions.append(pos)
        pos += len(e)
    out = bytearray()
    out += struct.pack(">I", 0) + struct.pack(">i", -3) + struct.pack(">I", 12)
    out += struct.pack(">III", len(entries), len(entries) + 1, pos)
    for o in positions:
        out += struct.pack(">I", o)
    out += struct.pack(">I", pos)
    out += b"".join(entries)
    struct.pack_into(">I", out, 0, len(out))

    payload = bm.rebuild_preserving_indices(
        donor, replace={donor.by_tag("VWSC")[0].index: bytes(out)})
    path = OUT / f"graft0-{profile.name}.dir"
    path.write_bytes(payload)
    return path, payload


def build_score_donor_index(profile, main_channel=False, tag="scoreidx",
                            own_details=False, key_frames=(), sprites=1):
    """Our frame stream, the donor's detail index. The step after graft0.

    graft0 renders, so the container, the cast, the sprite record and the detail
    entries are all sound, and what is left is the score we generate. That is
    still two things at once: the frames and the index around them. This keeps
    the donor's index entries exactly where they are and swaps only entry 0 for
    our own four frame score, whose sprites borrow the donor's detail triple.

        it renders -- our frames are fine and the detail entries we generate are
                      what Director rejects
        it does not -- our frame stream or its header is at fault
    """
    donor = bm.load_donor(profile.donor)
    check_donor_unprotected(donor, profile.donor)

    cast_text, _ = donor_text_member(donor)
    cas = donor.chunk(donor.by_tag("CAS*")[0])
    members = [struct.unpack(">I", cas[i:i + 4])[0]
               for i in range(0, len(cas), 4)]
    member = members.index(cast_text.index) + 1

    idx = 3

    def frame(editable, back):
        chans = {SPRITE_CH: profile.sprite(cast_member=member, x=20, y=20,
                                           w=200, h=40, editable=editable,
                                           back=back, list_idx=idx)}
        if sprites > 1:
            # A second channel, still showing a member the donor already has, so
            # this differs from the one sprite probe in nothing but the sprite.
            chans[RESULT_CH] = profile.sprite(cast_member=member, x=20, y=120,
                                              w=200, h=40, list_idx=idx + 3)
        if main_channel:
            # graft0's frames all touch the main channel and it renders; ours
            # touch nothing but the sprite. This variant closes that gap so the
            # pair of them tells the two apart.
            chans[0] = bm.main_channel_d6plus(profile.main_size,
                                              script_list_idx=3)
        return chans

    ours = bm.build_vwsc_d6plus(
        [frame(True, 0), frame(False, 255), frame(True, 255), frame(True, 255)],
        main_size=profile.main_size, spr_size=profile.spr_size,
        frames_version=profile.frames_version,
        geometry=bm.donor_score_geometry(donor))

    # unwrap our entry 0 and drop it into the donor's index
    lsx = struct.unpack(">I", ours[8:12])[0]
    ne, ls, _ = struct.unpack(">III", ours[lsx:lsx + 12])
    idx = lsx + 12
    mine = ours[idx + ls * 4:]

    b = donor.chunk(donor.by_tag("VWSC")[0])
    d_ls = struct.unpack(">I", b[struct.unpack(">I", b[8:12])[0] + 4:
                                 struct.unpack(">I", b[8:12])[0] + 8])[0]
    d_idx = struct.unpack(">I", b[8:12])[0] + 12
    d_data = d_idx + d_ls * 4
    d_offs = [struct.unpack(">I", b[d_idx + 4 * i:d_idx + 4 * i + 4])[0]
              for i in range(d_ls)]
    if own_details:
        spans = [(SPRITE_CH, 3)] + ([(RESULT_CH, 6)] if sprites > 1 else [])
        entries = [mine, bm.build_detail_directory([i for _, i in spans]), b""]
        for channel, _ in spans:
            entries += [bm.build_sprite_info(
                start_frame=1, end_frame=4, channel=channel,
                **({"key_frames": key_frames} if key_frames else {})), b"", b""]
    else:
        entries = [mine] + [b[d_data + o:d_data + n]
                            for o, n in zip(d_offs, d_offs[1:])][1:]

    positions, pos = [], 0
    for e in entries:
        positions.append(pos)
        pos += len(e)
    out = bytearray()
    out += struct.pack(">I", 0) + struct.pack(">i", -3) + struct.pack(">I", 12)
    out += struct.pack(">III", len(entries), len(entries) + 1, pos)
    for o in positions:
        out += struct.pack(">I", o)
    out += struct.pack(">I", pos)
    out += b"".join(entries)
    struct.pack_into(">I", out, 0, len(out))

    payload = bm.rebuild_preserving_indices(
        donor, replace={donor.by_tag("VWSC")[0].index: bytes(out)})
    path = OUT / f"{tag}-{profile.name}.dir"
    path.write_bytes(payload)
    return path, payload


if __name__ == "__main__":
    import argparse
    ap = argparse.ArgumentParser(description=__doc__)
    ap.add_argument("--d6", metavar="MOVIE",
                    help="a real D6 movie to clone config and text member from")
    ap.add_argument("--d7", metavar="MOVIE", help="likewise for D7")
    args = ap.parse_args()

    for builder in (build_editable, build_listoverride, build_probe_d4):
        report(*builder())
    for letter, what, name, size in build_ladder() + build_cast_ladder():
        print("wrote %s (%d bytes) -- %s" % (name, size, what))

    profiles = []
    if args.d6:
        profiles.append(Profile("d6", donor=args.d6, main_size=144, spr_size=24,
                                frames_version=11, sprite_writer=bm.sprite_d6))
    if args.d7:
        profiles.append(Profile("d7", donor=args.d7, main_size=288, spr_size=48,
                                frames_version=13, sprite_writer=bm.sprite_d7))
    for profile in profiles:
        for kind in ("editable", "listoverride"):
            report(*build_variant(profile, kind))
        report(*build_probe(profile))
        report(*build_editable_append(profile))
        report(*build_editable_append(
            profile, trivial=True, tag=f'scriptran-{profile.name}'))
        report(*build_score_probe(profile))
        report(*build_graft_probe(profile))
        report(*build_graft_minimal(profile))
        report(*build_score_donor_index(profile))
        report(*build_score_donor_index(profile, tag="scoredet",
                                        own_details=True))
        report(*build_score_donor_index(profile, tag="scoredet2",
                                        own_details=True, sprites=2))
        for name, what, out_name, size in build_ladder_d6d7(profile):
            print(f"wrote {out_name} ({size} bytes)  ladder {name}: {what}")
