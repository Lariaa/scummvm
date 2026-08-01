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
import rifx, lscr, lscr_asm, build_movie as bm

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


def src_first(e):
    return ("on exitFrame\r  set gEditableResult to "
            "(the editableText of sprite %d = %d)\rend\r" % (SPRITE_CH, e))


def src_append(e):
    return ("on exitFrame\r  set gEditableResult to gEditableResult * 10 + "
            "(the editableText of sprite %d = %d)\rend\r" % (SPRITE_CH, e))


def src_report():
    return ('on exitFrame\r  set the text of field %d to "%s" & gEditableResult\r'
            '  put "%s" & gEditableResult\rend\r' % (RESULT_MEMBER, LABEL, LABEL))


def make_lscr(code, script_id, consts=()):
    h = lscr_asm.Handler(I_EXITFRAME, code)
    return lscr_asm.build_lscr([h], script_id=script_id, assembly_id=script_id,
                               event_map=[-1] * 10 + [0],
                               event_map_flags=lscr_asm.EXITFRAME_EVENT_FLAGS,
                               consts=consts)


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
        factory_name_id=B_FACTORY,
        # parentNumber is an index, not a flag: -1 (the value non-factory
        # scripts carry) makes readers walk off the parent list. 0 is the
        # valid "no parent" value for a factory.
        parent_number=0,
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
    report_src = ('on exitFrame\r  set the text of field %d to "%s" & gListResult\r'
                  '  put "%s" & gListResult\rend\r'
                  % (RESULT_MEMBER, B_LABEL, B_LABEL))
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
                               consts=consts)


if __name__ == "__main__":
    for builder in (build_editable, build_listoverride):
        p, d = builder()
        print(f"wrote {p} ({len(d)} bytes)")
        f = rifx.RifxFile(d)
        for r in f.by_tag("Lscr"):
            s = lscr.Lscr(f.chunk(r))
            flags = f" FACTORY({s.factory_name_id})" if s.script_flags & 0x10 else ""
            for fn in s.functions:
                print(f"  Lscr {r.index}{flags} args={fn.arg_names} "
                      f"code={fn.code.hex(' ')}")
