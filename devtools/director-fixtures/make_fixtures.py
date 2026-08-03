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


def src_report():
    return ("on exitFrame\r"
            "  global gEditableResult\r"
            '  set the text of field %d to "%s" & gEditableResult\r'
            '  put "%s" & gEditableResult\r'
            "end\r" % (RESULT_MEMBER, LABEL, LABEL))


def make_lscr(code, script_id, consts=()):
    h = lscr_asm.Handler(I_EXITFRAME, code)
    return lscr_asm.build_lscr([h], script_id=script_id, assembly_id=script_id,
                               event_map=[-1] * 10 + [0],
                               event_map_flags=lscr_asm.EXITFRAME_EVENT_FLAGS,
                               consts=consts, global_names=[I_GLOBAL])


def build_editable():
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

    chunks[i_vwsc - 3] = (b"VWSC", bm.build_vwsc([
        frame(True, 0, script_member[0]),
        frame(False, 255, script_member[1]),
        frame(True, 255, script_member[2]),
        frame(True, 255, script_member[3]),
    ]))

    data = bm.build_rifx(chunks)
    path = OUT / "editable.dir"
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
                   ("reportResult", field_report(), src_report(), (label,))]
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


def report(path, data):
    print(f"wrote {path.name} ({len(data)} bytes)")
    f = rifx.RifxFile(data)
    for r in f.by_tag("Lscr"):
        s = lscr.Lscr(f.chunk(r))
        flags = f" FACTORY({s.factory_name_id})" if s.script_flags & 0x10 else ""
        for fn in s.functions:
            print(f"   Lscr {r.index}{flags} args={fn.arg_names} "
                  f"code={fn.code.hex(' ')}")


if __name__ == "__main__":
    import argparse
    ap = argparse.ArgumentParser(description=__doc__)
    ap.add_argument("--d6", metavar="MOVIE",
                    help="a real D6 movie to clone config and text member from")
    ap.add_argument("--d7", metavar="MOVIE", help="likewise for D7")
    args = ap.parse_args()

    for builder in (build_editable, build_listoverride):
        report(*builder())

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
