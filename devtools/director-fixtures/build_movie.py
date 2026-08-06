"""Build the Director 4 regression fixture movies.

Container is RIFX (big endian) throughout, like the donor, so no chunk has to
switch byte order. Layout mirrors what ScummVM reads:
  RIFXArchive::readMemoryMap        engines/director/archive.cpp:792
  Cast::loadCastData / loadCastInfo engines/director/cast.cpp:1547 / :1949
  Movie::loadInfoEntries            engines/director/movie.cpp:368
  Cast::loadLingoContext            engines/director/cast.cpp:1768
  Score::loadFrames / readOneFrame  engines/director/score.cpp:1873 / :2229
"""
import struct, gzip, re, pathlib
import rifx, lscr, lscr_asm

BE = ">"
KCAST_LINGO_SCRIPT = 11
SCRIPT_TYPE_SCORE = 1           # ScriptCastMember: 1 = kScoreScript
SCRIPT_TYPE_MOVIE = 3           #                   3 = kMovieScript
DEFAULT_CAST_LIB = 1            # movie.h:25
CASTLIB_KEY_PARENT = 1024       # KEY* parent id used for cast-wide resources

MAIN_CHANNEL_SIZE_D4 = 40
SPR_CHANNEL_SIZE_D4 = 20


# --------------------------------------------------------------------------
# chunk builders
# --------------------------------------------------------------------------
CAST_INFO_STRING_COUNT = 7      # what real D4 script members carry


def build_cast_info(*, script_id, name="", script_source=""):
    """castInfo block: header (offset, unk1, unk2, flags, scriptId) then a
    string table of cumulative offsets.

    String 0 is the raw script source, string 1 the member name as a Pascal
    string -- Cast::loadCastInfo reads them as strings[0].readString(false) and
    strings[1].readString() respectively. Real D4 script members always emit
    seven entries; emitting fewer makes readers that index the later slots run
    off the end of the block.
    """
    name_b = name.encode("latin1")
    strings = [script_source.encode("latin1"),
               (bytes([len(name_b)]) + name_b) if name_b else b""]
    strings += [b""] * (CAST_INFO_STRING_COUNT - len(strings))
    out = bytearray()
    out += struct.pack(BE + "I", 20)            # offset to the string table
    out += struct.pack(BE + "III", 0, 0, 0)     # unk1, unk2, flags
    out += struct.pack(BE + "I", script_id)
    out += struct.pack(BE + "H", len(strings))
    total = 0
    out += struct.pack(BE + "I", total)
    for s in strings:
        total += len(s)
        out += struct.pack(BE + "I", total)
    for s in strings:
        out += s
    return bytes(out)


def build_script_cast(script_id, name, source="", script_type=SCRIPT_TYPE_SCORE,
                      d5plus=False):
    """CASt for a script cast member. castData is exactly the 2-byte script
    type; ScriptCastMember asserts it consumed the whole stream.

    The two header shapes come from Cast::loadCastData (cast.cpp:1570ff): D4
    stores a u16 data size, a u32 info size and then the castType and flags1
    bytes inside the data block, with the data before the info. D5 and later
    store three u32 (type, info size, data size) and put the info block first,
    with no type/flags1 bytes in the data.
    """
    cast_data = struct.pack(BE + "H", script_type)
    info = build_cast_info(script_id=script_id, name=name, script_source=source)
    out = bytearray()
    if d5plus:
        out += struct.pack(BE + "III", KCAST_LINGO_SCRIPT, len(info), len(cast_data))
        out += info
        out += cast_data
    else:
        out += struct.pack(BE + "H", 2 + len(cast_data))   # incl. type+flags1
        out += struct.pack(BE + "I", len(info))
        out += bytes([KCAST_LINGO_SCRIPT, 0])              # castType, flags1
        out += cast_data
        out += info
    return bytes(out)


def build_lctx(lscr_indices, lnam_index):
    """Lctx: 42-byte header, then 12-byte entries holding the mmap index of
    each Lscr. Entry position (1-based) is the lctxIndex handed to addCodeV4,
    and must match the scriptId stored in the script member's castInfo."""
    items_offset = 0x2a
    entry_size = 12
    count = len(lscr_indices)
    out = bytearray()
    out += struct.pack(BE + "HHHH", 0, 0, 0, 0)
    out += struct.pack(BE + "ii", count, count)
    out += struct.pack(BE + "HH", items_offset, entry_size)
    out += struct.pack(BE + "I", 0)                 # unk1
    out += struct.pack(BE + "I", 0)                 # fileType
    out += struct.pack(BE + "I", 0)                 # unk2
    out += struct.pack(BE + "i", lnam_index)        # nameTableId
    out += struct.pack(BE + "hH", count, 0)         # validCount, flags
    out += struct.pack(BE + "h", -1)                # firstUnused: none
    assert len(out) == items_offset, len(out)
    for idx in lscr_indices:
        out += struct.pack(BE + "I", 0)             # unknown
        out += struct.pack(BE + "i", idx)           # mmap index of the Lscr
        out += struct.pack(BE + "Hh", 0, -1)        # entryFlags, nextUnused
    return bytes(out)


def build_mcsl(name="Internal", min_member=1, max_member=1,
               lib_resource_id=CASTLIB_KEY_PARENT):
    """MCsL, the cast library mapping D5 and later use.

    From D5 on this, not the config chunk, is where the member range and the
    library's resource id come from (Movie::loadCastLibMapping, movie.cpp:154),
    and it overrides whatever castArrayEnd says. Cloning a donor's MCsL
    therefore imports its library layout -- for a movie with three libraries and
    a resource id of 0x10400 that leaves nothing that matches the fixture.

    Layout: a 12 byte header, then at dataOffset an offset table of
    count * itemsPerCast + 1 entries, the items length, and the items. Per
    library: item 1 is the name as a Pascal string, item 2 the path (empty for
    an internal cast), item 3 the preload setting, item 4 the member range and
    resource id.
    """
    name_b = name.encode("latin1")
    items = [b"",                                           # 0, unused
             bytes([len(name_b)]) + name_b + b"\0",         # 1, name
             b"",                                           # 2, path
             struct.pack(BE + "H", 0),                      # 3, preload
             struct.pack(BE + "HHI", min_member, max_member, lib_resource_id)]

    offsets, pos = [], 0
    for it in items:
        offsets.append(pos)
        pos += len(it)

    out = bytearray()
    out += struct.pack(BE + "I", 12)            # dataOffset
    out += struct.pack(BE + "H", 0)             # unknown
    out += struct.pack(BE + "H", 1)             # count: one library
    out += struct.pack(BE + "H", 4)             # itemsPerCast
    out += struct.pack(BE + "H", 0)             # padding up to dataOffset
    out += struct.pack(BE + "H", len(items))
    for off in offsets:
        out += struct.pack(BE + "I", off)
    out += struct.pack(BE + "I", pos)           # itemsLen
    for it in items:
        out += it
    return bytes(out)


def internal_lib_resource_id(blob):
    """The resource id of library 1 in a donor's MCsL.

    KEY* uses it as the parent of the cast-wide chunks, and it is not always
    1024: the D7 donor's is 0x10400. Claiming 1024 instead points Director at a
    library with no CAS* under it, which it renders as a cast with no members at
    all -- not even the donor's.

    Items run as one unused entry followed by four per library (name, path,
    preload, range), so library 1's range is item 4, and the id is its last four
    bytes.
    """
    nitems = struct.unpack_from(BE + "H", blob, 12)[0]
    offs = [struct.unpack_from(BE + "I", blob, 14 + 4 * i)[0]
            for i in range(nitems)]
    items_at = 14 + nitems * 4 + 4
    RANGE_ITEM = 4
    start = items_at + offs[RANGE_ITEM]
    end = items_at + (offs[RANGE_ITEM + 1] if RANGE_ITEM + 1 < nitems
                      else len(blob) - items_at)
    if end - start != 8:
        raise ValueError("MCsL item %d is %d bytes, expected a range"
                         % (RANGE_ITEM, end - start))
    return struct.unpack_from(BE + "I", blob, start + 4)[0]


def dropped_lib_key_parents(blob):
    """The KEY* parents that stop meaning anything once we emit one library.

    A movie files its own chunks under one id and each cast library under
    another. The D7 donor uses 1024 for the movie, 0x10400 for the internal cast
    and 1025 to 1027 for the three libraries it declares; keeping only the
    internal one leaves the last three describing libraries that are gone.
    """
    count = struct.unpack_from(BE + "H", blob, 6)[0]
    return tuple(range(CASTLIB_KEY_PARENT + 1, CASTLIB_KEY_PARENT + 1 + count))


def single_internal_mcsl(blob, max_member, min_member=1):
    """One internal library, with the resource id the donor's already uses.

    Keeping the donor's mapping wholesale is no good either: the D7 donor
    declares three libraries, two of them external casts on a Macintosh volume,
    and Director stops on load to ask where highscor.cst went. So we emit a
    single library, which a fixture needs, but under the donor's own id, which
    is what its KEY* is filed under.
    """
    return build_mcsl("Internal", min_member, max_member,
                      internal_lib_resource_id(blob))


def drop_donor_scripts(donor, member_indices):
    """The donor's member list with every script member blanked out.

    The films test the scripts we write; the donor's are only in the way. Its D7
    one references an external cast we deliberately stopped declaring, and the
    handlers doing so sit in a behavior and a movie script both, so picking off
    one kind leaves the other. Declaring the library again to keep them quiet
    would ship a fixture that asks for a file it does not have.

    An emptied slot is what a slot with no member holds anyway. Pair this with
    unlink_donor_scripts: clearing the slot alone leaves the code reachable.
    """
    out = []
    for index in member_indices:
        res = next((r for r in donor.resources if r.index == index), None)
        keep = True
        if res is not None and res.tag == "CASt":
            body = donor.chunk(res)
            if len(body) >= 12:
                cast_type, info_len, data_len = struct.unpack(BE + "III", body[0:12])
                data = body[12 + info_len:12 + info_len + data_len]
                if cast_type == KCAST_LINGO_SCRIPT:
                    keep = False
        out.append(index if keep else 0)
    return out


def build_cas(cast_indices):
    """CAS* is a positional u32 array: slot i holds the mmap index of the CASt
    for cast member i+1, or 0 (RIFXArchive::writeCast)."""
    return b"".join(struct.pack(BE + "I", i) for i in cast_indices)


def build_key(entries, max_entries=None):
    """KEY*: (childIndex, parentIndex, tag) triples, sorted by parent then tag.

    Every one of 25 movies sampled from the archive is sorted that way, and
    appending an entry instead of inserting it is what makes Director call an
    added cast member corrupted -- it looks up a member's children in here and
    does not find them past the point where the order breaks. ScummVM builds a
    hashmap from the whole table (RIFXArchive::readKeyTable) and does not care.
    """
    entries = sorted(entries, key=lambda e: (e[1], e[2]))
    if max_entries is None:
        max_entries = len(entries)
    out = bytearray()
    out += struct.pack(BE + "HH", 12, 12)
    out += struct.pack(BE + "II", max_entries, len(entries))
    for child, parent, tag in entries:
        out += struct.pack(BE + "II", child, parent)
        out += tag
    out += b"\0" * 12 * (max_entries - len(entries))
    return bytes(out)


# --------------------------------------------------------------------------
# score
# --------------------------------------------------------------------------
# The donor's own text sprite, which probe-d4.dir proves Director renders. The
# palette index matters: on the Windows default palette 255 is white, so the
# obvious looking fore=255 draws white text, and on a white stage that is a movie
# that appears to show nothing at all.
SPRITE_FORE_D4 = 128
SPRITE_BACK_D4 = 128

SPRITE_TYPE_TEXT = 7            # kTextSprite (types.h:162), the D4 convention
# From D5 on a sprite that shows a cast member carries kCastMemberSprite and
# lets the member decide what it is; both sprites in the D7 donor's score do.
# Writing the D4 value here leaves the Score window empty on D6 and crashes
# Director 7 outright.
SPRITE_TYPE_CAST_MEMBER = 16    # kCastMemberSprite (types.h:171)

# Taken from the donor's own sprite records, which probe-d4.dir proves Director
# renders inside our container. An earlier attempt put 0x80 in the thickness
# byte, going by a histogram over accumulated frame state -- but that state is
# rebuilt from partial channel writes, so unwritten bytes carried over from
# other channels and the count meant nothing. The donor has 0 there and 0x80 in
# the ink byte, where it reads as Sprite::_stretch.
SPRITE_THICKNESS_DEFAULT = 0x00
SPRITE_INK_DEFAULT = 0x80


def sprite_d4(*, cast_member, x, y, w, h, sprite_type=SPRITE_TYPE_TEXT,
              ink=SPRITE_INK_DEFAULT,
              editable=False, fore=SPRITE_FORE_D4, back=SPRITE_BACK_D4,
              script_id=0,
              thickness=SPRITE_THICKNESS_DEFAULT):
    """20-byte D4 sprite record (writeSpriteDataD4, frame.cpp:742)."""
    colorcode = 0x40 if editable else 0x00
    out = bytearray()
    out += bytes([script_id & 0xff, sprite_type, fore, back, thickness, ink])
    out += struct.pack(BE + "H", cast_member)
    out += struct.pack(BE + "HH", y, x)          # startPoint: y then x
    out += struct.pack(BE + "HH", h, w)
    out += struct.pack(BE + "H", script_id)
    out += bytes([colorcode, 0])                 # colorcode, blendAmount
    assert len(out) == SPR_CHANNEL_SIZE_D4
    return bytes(out)


def main_channel_d4(*, action_id=0, tempo=0):
    """40-byte D4 main channel (writeMainChannelsD4, frame.cpp:584).
    actionId at offset 16 is the per-frame script cast member."""
    out = bytearray()
    out += bytes([0, 0, 0, 0, tempo, 0])
    out += struct.pack(BE + "HH", 0, 0)          # sound1, sound2
    out += bytes([0, 0, 0, 0, 0, 0])
    out += struct.pack(BE + "H", action_id)      # 16, 17
    out += bytes([0, 0])                         # colorScript, colorTrans
    out += b"\0" * (MAIN_CHANNEL_SIZE_D4 - len(out))
    assert len(out) == MAIN_CHANNEL_SIZE_D4
    return bytes(out)


def sprite_d6(*, cast_member, x, y, w, h,
              sprite_type=SPRITE_TYPE_CAST_MEMBER,
              ink=SPRITE_INK_DEFAULT, editable=False,
              fore=255, back=0, cast_lib=DEFAULT_CAST_LIB, list_idx=0):
    """24-byte D6 sprite record (writeSpriteDataD6, frame.cpp).
    The editable bit is still 0x40, but colorcode moved to byte 20.

    list_idx points at this sprite's detail entries and must not be 0: across
    577649 sprite records in the archive not one carries 0 there. ScummVM guards
    with `if (sprite->_spriteListIdx)` (score.cpp:2081) and so never notices,
    but Director follows the index unconditionally and 0 is entry 0 -- the score
    itself -- which it then reads as a SpriteInfo.
    """
    out = bytearray()
    out += bytes([sprite_type, ink, fore, back])
    out += struct.pack(BE + "hH", cast_lib, cast_member)
    out += struct.pack(BE + "I", list_idx)           # spriteListIdx
    out += struct.pack(BE + "HH", y, x)
    out += struct.pack(BE + "HH", h, w)
    out += bytes([0x40 if editable else 0x00, 0, SPRITE_THICKNESS_DEFAULT, 0])
    assert len(out) == 24
    return bytes(out)


def sprite_d7(*, cast_member, x, y, w, h,
              sprite_type=SPRITE_TYPE_CAST_MEMBER,
              ink=SPRITE_INK_DEFAULT, editable=False,
              fore=255, back=0, cast_lib=DEFAULT_CAST_LIB, list_idx=0):
    """48-byte D7 sprite record; the first 23 bytes match D6."""
    out = bytearray(sprite_d6(cast_member=cast_member, x=x, y=y, w=w, h=h,
                              sprite_type=sprite_type, ink=ink,
                              editable=editable, fore=fore, back=back,
                              cast_lib=cast_lib, list_idx=list_idx)[:23])
    out += bytes([0])                                # flags
    out += bytes([0, 0, 0, 0])                       # fg/bg colour G and B
    out += struct.pack(BE + "II", 0, 0)              # angleRot, angleSkew
    out += b"\0" * 12
    assert len(out) == 48
    return bytes(out)


def main_channel_d6plus(size, *, action_id=0, cast_lib=DEFAULT_CAST_LIB,
                        script_list_idx=0):
    """D6/D7 main channel. Only the frame script is filled in; everything else
    stays zero, which reads back as no tempo, transition or palette change.

    Follows readMainChannelsD6/D7: castLib and member are two u16 at offsets 0
    and 2. Note that writeMainChannelsD6/D7 disagree with that -- they emit a
    u32 for castLib -- so the reader is the authority here.
    """
    out = bytearray(size)
    struct.pack_into(BE + "HH", out, 0, cast_lib, action_id)
    # scriptSpriteListIdx: a u16 at offset 6, exactly where the donor puts it
    # (readMainChannelsD6 case 0+6). Like a sprite's, it must not be 0.
    struct.pack_into(BE + "H", out, 6, script_list_idx)
    return bytes(out)


SPRITE_INFO_FIXED = 40          # five int32 plus a five field TweenInfo


# TweenInfo values every real record carries. The curvature is 0x10000, which
# is 1.0 in the 16.16 fixed point Director uses; the flags differ between a
# sprite span and one of the main channel's own spans.
TWEEN_CURVATURE = 0x10000
TWEEN_FLAGS_SPRITE = 24589      # 0x600d, on every sprite sampled in the archive
TWEEN_FLAGS_MAIN = 1            # what the donor's script and transition spans use


def build_sprite_info(*, start_frame, end_frame, channel, key_frames=(0,),
                      tween_flags=None):
    """One detail entry, per SpriteInfo::read (spriteinfo.h:55).

    A detail index n claims three consecutive entries: n is this struct, n+1 the
    behaviour list and n+2 the name (score.cpp:2058ff). The last two may be
    empty, but they have to exist, or the entry after them is read as theirs.

    Real records are 44 bytes, not 40: the fixed part followed by a single
    keyframe of 0. Writing the bare fixed part leaves the keyframe loop with
    nothing to read, which ScummVM is happy with and Director is not.
    """
    if tween_flags is None:
        tween_flags = TWEEN_FLAGS_MAIN if channel == 0 else TWEEN_FLAGS_SPRITE
    out = struct.pack(BE + "iiiii", start_frame, end_frame, 0, 0, channel)
    out += struct.pack(BE + "iiiii", TWEEN_CURVATURE, tween_flags, 0, 0, 0)
    for f in key_frames:
        out += struct.pack(BE + "i", f)
    assert len(out) == SPRITE_INFO_FIXED + 4 * len(key_frames)
    return out


def build_behavior_list(members, cast_lib=DEFAULT_CAST_LIB):
    """Detail entry n+1: the behaviours attached to the span at index n.

    Eight bytes each -- cast lib, member, initializer index (BehaviorElement,
    spriteinfo.h:87). This is how D6 and later attach a frame script: the donor's
    script channel points at detail index 3 and entry 4 holds 00 01 00 02, its
    member 2. Setting the main channel's actionId and leaving this empty gets a
    script that the Score window draws in the script channel and that never runs.
    """
    out = b""
    for member in members:
        out += struct.pack(BE + "hhi", cast_lib, member, 0)
    return out


def build_detail_directory(indices):
    """Detail entry 1: how many indices the score uses, then each of them.

    It sits in what the index-modulo-three rule calls a behaviour slot, because
    it belongs to index 0 -- the score. Both donors carry it and so does every
    movie sampled from the archive, listing exactly the indices its sprites and
    main channel spans point at.
    """
    out = struct.pack(BE + "I", len(indices))
    for i in indices:
        out += struct.pack(BE + "I", i)
    return out


def build_vwsc_d6plus(frames, *, main_size, spr_size, frames_version,
                      num_channels=50, geometry=None, details=()):
    """D6 and later wrap the score in an index, but the score itself is unchanged.

        0   u32 framesStreamSize (whole chunk)
        4   u32 ver              (-3)
        8   u32 listStart        (12)
        12  u32 numEntries
        16  u32 listSize         (numEntries + 1)
        20  u32 maxDataLen
        24  u32 offsets[]        relative to the data area

    Entry 0 is **the whole score** -- the D4 style header followed by every frame
    back to back, exactly as before D6. The remaining entries are the per sprite
    detail blobs ScummVM tracks in `_spriteDetailOffsets` (score.cpp:1920); a
    movie without behaviours needs none, so we write a single entry.

    This was got wrong once, at the cost of several rounds of testing: putting
    each frame in an index entry of its own produces a movie ScummVM reads
    happily -- it walks the table -- and real Director reads as a score with no
    frames at all, which is a black stage and no error message. Both donors
    confirm the layout: the frame stream starts at `frame1Offset` = 20 inside
    entry 0 and runs to `framesStreamSize`.
    """
    # numOfFrames is version specific, and the archive is unanimous about it:
    # every score with framesVersion 11 writes 0 and lets the reader count,
    # every one with 13 writes the real number. Writing the count on a D6 movie
    # says something no D6 movie says.
    header, body, _ = _score_header_and_frames(
        frames, main_size=main_size, spr_size=spr_size,
        frames_version=frames_version, num_channels=num_channels,
        declared_frames=len(frames) if frames_version >= 13 else 0)
    if geometry is not None:
        # frames version, sprite record size and channel counts, taken from the
        # donor rather than guessed -- they describe the movie, not our frames
        header = header[:12] + geometry
    data = bytearray(header + body)
    if len(data) % 2:                                # donors pad the entry, and
        data += b"\0"                                # leave the size field odd

    entries = [bytes(data)] + [bytes(d) for d in details]
    num_entries = len(entries)
    list_size = num_entries + 1
    index_start = 24

    offsets, pos = [], 0
    for e in entries:
        offsets.append(pos)
        pos += len(e)

    out = bytearray()
    out += struct.pack(BE + "I", 0)                  # patched below
    out += struct.pack(BE + "i", -3)
    out += struct.pack(BE + "I", 12)
    out += struct.pack(BE + "III", num_entries, list_size, pos)
    for off in offsets:
        out += struct.pack(BE + "I", off)
    out += struct.pack(BE + "I", pos)                # end sentinel
    assert len(out) == index_start + list_size * 4, len(out)
    out += b"".join(entries)
    struct.pack_into(BE + "I", out, 0, len(out))
    return bytes(out)


def donor_score_geometry(donor):
    """The donor's frames version, sprite record size and channel counts."""
    b = donor.chunk(donor.by_tag("VWSC")[0])
    list_start = struct.unpack_from(BE + "I", b, 8)[0]
    list_size = struct.unpack_from(BE + "I", b, list_start + 4)[0]
    index_start = list_start + 12
    data = index_start + list_size * 4
    off = struct.unpack_from(BE + "I", b, index_start)[0]
    return b[data + off + 12:data + off + 20]


def _split_channel(channel, offset, payload, main_size, spr_size):
    """Turns one channel's state into the chunk shape real movies use.

    Director never writes a channel in one piece: sampled D4 scores emit the
    sprite record as 16 bytes at the channel start and the remaining four --
    script id, colour code, blend -- separately, and touch the main channel only
    where something changed, typically two bytes at offset 16 for the frame
    script. Writing the whole record instead is accepted by ScummVM, whose
    reader takes any (size, offset) pair, but leaves Director's score empty.
    """
    if channel == 0:
        # Only the frame script, which is all our main channel ever carries.
        # D4 keeps it in one u16 at offset 16; D6 and D7 use two, a cast lib and
        # a member, at offsets 0 and 2 (readMainChannelsD4 vs D6/D7).
        if main_size == MAIN_CHANNEL_SIZE_D4:
            return [(offset + 16, payload[16:18])]
        # The donor writes these as two pieces, 4 bytes of cast lib and member
        # then 2 bytes of scriptSpriteListIdx, and the reader's case labels only
        # line up that way -- a single 8 byte write would hit case 0+4, which is
        # a u32 read of the same field.
        return [(offset, payload[:4]), (offset + 6, payload[6:8])]
    if main_size != MAIN_CHANNEL_SIZE_D4:
        # D6 and D7 do not use the 16 + 4 split at all. Counting the channel
        # writes in the archive, the whole record in one piece is the second
        # most common shape either version uses -- 24 bytes at the channel start
        # 74530 times on D6, 48 bytes 343435 times on D7 -- while 16 at the
        # start followed by the rest does not appear.
        return [(offset, payload[:spr_size])]
    return [(offset, payload[:16]), (offset + 16, payload[16:spr_size])]


def _score_header_and_frames(frames, *, main_size, spr_size, frames_version,
                             num_channels, declared_frames=None):
    """The score header and frame blocks shared by every version from D4 on."""
    body = bytearray()
    offsets = []
    for chans in frames:
        offsets.append(len(body))
        frame = bytearray()
        for ch in sorted(chans):
            payload = chans[ch]
            offset = 0 if ch == 0 else main_size + (ch - 1) * spr_size
            for off, part in _split_channel(ch, offset, payload,
                                            main_size, spr_size):
                if not part:
                    continue
                frame += struct.pack(BE + "HH", len(part), off)
                frame += part
        body += struct.pack(BE + "H", len(frame) + 2)
        body += frame

    frame1_offset = 20
    header = bytearray()
    header += struct.pack(BE + "I", frame1_offset + len(body))
    header += struct.pack(BE + "I", frame1_offset)
    # Every real D6 movie surveyed carries 0 here and lets the reader count the
    # frames from the offset table instead; ScummVM does exactly that
    # ("numOfFrames in the header is often incorrect", score.cpp:1999).
    header += struct.pack(BE + "I",
                          len(frames) if declared_frames is None else declared_frames)
    header += struct.pack(BE + "HH", frames_version, spr_size)
    header += struct.pack(BE + "H", num_channels)
    header += struct.pack(BE + "H", 0x0100)          # skipped for framesVersion <= 13
    assert len(header) == frame1_offset
    return bytes(header), bytes(body), [frame1_offset + o for o in offsets]


def build_vwsc(frames, num_channels=50):
    """frames: list of {channel_index: payload}; channel 0 is the main channel.
    Frame encoding per Score::readOneFrame (score.cpp:2229): u16 frameSize
    (including itself), then (u16 size, u16 offset, data) per channel."""
    body = bytearray()
    for chans in frames:
        frame = bytearray()
        for ch in sorted(chans):
            data = chans[ch]
            offset = 0 if ch == 0 else MAIN_CHANNEL_SIZE_D4 + (ch - 1) * SPR_CHANNEL_SIZE_D4
            for off, part in _split_channel(ch, offset, data,
                                            MAIN_CHANNEL_SIZE_D4,
                                            SPR_CHANNEL_SIZE_D4):
                if not part:
                    continue
                frame += struct.pack(BE + "HH", len(part), off)
                frame += part
        body += struct.pack(BE + "H", len(frame) + 2)
        body += frame

    header = bytearray()
    frame1_offset = 20
    header += struct.pack(BE + "I", frame1_offset + len(body))   # framesStreamSize
    header += struct.pack(BE + "I", frame1_offset)
    header += struct.pack(BE + "I", len(frames))
    header += struct.pack(BE + "HH", 4, SPR_CHANNEL_SIZE_D4)     # framesVersion, spriteRecordSize
    header += struct.pack(BE + "H", num_channels)
    header += struct.pack(BE + "H", 0x0100)                      # skipped when framesVersion <= 13
    assert len(header) == frame1_offset
    return bytes(header) + bytes(body)


# --------------------------------------------------------------------------
# RIFX container
# --------------------------------------------------------------------------
def rebuild_preserving_indices(donor, *, imap_version=None, rewrite_key=False,
                               extra=(), extra_key=(), replace=None,
                               drop_key_parents=()):
    """Re-emits every resource of `donor` at its original mmap index.

    build_rifx() renumbers, which forces KEY* and Lctx to be rewritten -- the
    largest edit in what is meant to be a faithful copy. Here the numbering is
    kept, so every cross reference in the file stays valid untouched and only
    the offsets move. Free and junk entries are carried along, since the donor
    counts them in its used count.
    """
    entries = sorted(donor.resources, key=lambda r: r.index)
    count = entries[-1].index + 1
    replace = dict(replace or {})
    # appended chunks take the indices after the donor's, so nothing the donor
    # already references has to move
    appended = [(3 + 0, tag, payload) for tag, payload in ()]   # placeholder
    extra_start = count
    count += len(extra)
    header_size, entry_size = 24, 20

    imap_offset = 12
    mmap_offset = imap_offset + 8 + 24
    pos = mmap_offset + 8 + header_size + count * entry_size

    placed = {}
    for res in entries:
        if res.tag in ("RIFX", "XFIR", "imap", "mmap"):
            continue
        if res.size == 0 and res.offset == 0:
            continue                       # a free slot, no payload to move
        payload = replace.get(res.index, donor.chunk(res))
        placed[res.index] = (res.tag.encode("latin1"), payload, pos)
        pos += 8 + len(payload)
        if pos % 2:
            pos += 1
    for i, (tag, payload) in enumerate(extra):
        placed[extra_start + i] = (tag, payload, pos)
        pos += 8 + len(payload)
        if pos % 2:
            pos += 1
    total = pos

    out = bytearray(total)
    out[0:4] = b"RIFX"
    struct.pack_into(BE + "I", out, 4, total - 8)
    out[8:12] = donor.rifx_type.encode("latin1")

    out[imap_offset:imap_offset + 4] = b"imap"
    struct.pack_into(BE + "I", out, imap_offset + 4, 24)
    struct.pack_into(BE + "III", out, imap_offset + 8, 1, mmap_offset,
                     donor.version if imap_version is None else imap_version)

    out[mmap_offset:mmap_offset + 4] = b"mmap"
    struct.pack_into(BE + "I", out, mmap_offset + 4,
                     header_size + count * entry_size)
    struct.pack_into(BE + "HH", out, mmap_offset + 8, header_size, entry_size)
    struct.pack_into(BE + "II", out, mmap_offset + 12, count, count)
    out[mmap_offset + 20:mmap_offset + 28] = b"\xff" * 8
    struct.pack_into(BE + "i", out, mmap_offset + 28, -1)

    base = mmap_offset + 8 + header_size
    for i in range(len(extra)):
        idx = extra_start + i
        tag, payload, offset = placed[idx]
        e = base + idx * entry_size
        out[e:e + 4] = tag
        struct.pack_into(BE + "II", out, e + 4, len(payload), offset)
        out[offset:offset + 4] = tag
        struct.pack_into(BE + "I", out, offset + 4, len(payload))
        out[offset + 8:offset + 8 + len(payload)] = payload

    for res in entries:
        e = base + res.index * entry_size
        if res.tag == "RIFX" or res.tag == "XFIR":
            out[e:e + 4] = b"RIFX"
            struct.pack_into(BE + "II", out, e + 4, total - 8, 0)
        elif res.tag == "imap":
            out[e:e + 4] = b"imap"
            struct.pack_into(BE + "II", out, e + 4, 24, imap_offset)
        elif res.tag == "mmap":
            out[e:e + 4] = b"mmap"
            struct.pack_into(BE + "II", out, e + 4,
                             header_size + count * entry_size, mmap_offset)
        elif res.index in placed:
            tag, payload, offset = placed[res.index]
            if tag == b"KEY*" and (rewrite_key or extra_key or drop_key_parents):
                payload = _extend_key(payload, extra_key, drop_key_parents)
            out[e:e + 4] = tag
            struct.pack_into(BE + "II", out, e + 4, len(payload), offset)
            out[offset:offset + 4] = tag
            struct.pack_into(BE + "I", out, offset + 4, len(payload))
            out[offset + 8:offset + 8 + len(payload)] = payload
        else:
            out[e:e + 4] = res.tag.encode("latin1")
            struct.pack_into(BE + "II", out, e + 4, 0, 0)
        struct.pack_into(BE + "HH", out, e + 12, res.flags, res.unk1)
        struct.pack_into(BE + "I", out, e + 16, res.next_free & 0xFFFFFFFF)

    return bytes(out)


def _extend_key(payload, extra_key=(), drop_parents=()):
    """Re-emits a key table through our own encoder without touching a single
    index. Renumbering forces us to rewrite KEY*, so this separates the encoder
    from the renumbering when bisecting.

    `drop_parents` removes every entry filed under those parents. Dropping a
    cast library from MCsL leaves its KEY* entries behind pointing at a library
    nothing declares any more, and the donor and graft0 -- which keep all three
    libraries -- close cleanly where the fixtures that keep one do not.
    """
    used = struct.unpack_from(BE + "I", payload, 8)[0]
    max_entries = struct.unpack_from(BE + "I", payload, 4)[0]
    drop = set(drop_parents)
    entries = []
    for i in range(used):
        off = 12 + i * 12
        child, parent = struct.unpack_from(BE + "II", payload, off)
        if parent in drop:
            continue
        entries.append((child, parent, payload[off + 8:off + 12]))
    entries.extend(extra_key)
    return build_key(entries, max_entries=max(max_entries, len(entries)))


def build_rifx(chunks, *, rifx_type=b"MV93", max_map_entries=None,
               imap_version=0):
    """chunks: list of (tag, payload) in mmap index order, starting at index 3
    (0/1/2 are RIFX/imap/mmap). Returns the complete archive."""
    n = 3 + len(chunks)
    if max_map_entries is None:
        max_map_entries = n + 4
    imap_payload_size = 24
    mmap_payload_size = 24 + max_map_entries * 20

    imap_offset = 12
    mmap_offset = imap_offset + 8 + imap_payload_size
    pos = mmap_offset + 8 + mmap_payload_size

    placed = []
    for tag, payload in chunks:
        placed.append((tag, payload, pos))
        pos += 8 + len(payload)
        if pos % 2:
            pos += 1
    total = pos

    out = bytearray(total)
    out[0:4] = b"RIFX"
    struct.pack_into(BE + "I", out, 4, total - 8)
    out[8:12] = rifx_type

    # imap
    out[imap_offset:imap_offset + 4] = b"imap"
    struct.pack_into(BE + "I", out, imap_offset + 4, imap_payload_size)
    # The version here is 0 for D4, 0x4c1 for D5, 0x4c7 for D6 and so on.
    # ScummVM ignores it and takes the version from the config chunk, but
    # Director does not: leaving it at 0 makes Director treat a D6 or D7 movie
    # as D4 and hunt for a 'VWCF' config chunk that a D5+ movie does not have,
    # failing with "Could not find chunk: ChunkID='FCWV'".
    struct.pack_into(BE + "III", out, imap_offset + 8, 1, mmap_offset,
                     imap_version)

    # mmap
    out[mmap_offset:mmap_offset + 4] = b"mmap"
    struct.pack_into(BE + "I", out, mmap_offset + 4, mmap_payload_size)
    struct.pack_into(BE + "HH", out, mmap_offset + 8, 24, 20)
    struct.pack_into(BE + "II", out, mmap_offset + 12, max_map_entries, n)
    out[mmap_offset + 20:mmap_offset + 28] = b"\xff" * 8
    struct.pack_into(BE + "i", out, mmap_offset + 28, -1)

    entries_base = mmap_offset + 8 + 24

    def put_entry(i, tag, size, offset, flags=0, nxt=0):
        e = entries_base + i * 20
        out[e:e + 4] = tag
        struct.pack_into(BE + "II", out, e + 4, size, offset)
        struct.pack_into(BE + "HH", out, e + 12, flags, 0)
        struct.pack_into(BE + "i", out, e + 16, nxt)

    put_entry(0, b"RIFX", total - 8, 0, flags=1)
    put_entry(1, b"imap", imap_payload_size, imap_offset, flags=1)
    put_entry(2, b"mmap", mmap_payload_size, mmap_offset)

    for i, (tag, payload, offset) in enumerate(placed):
        put_entry(3 + i, tag, len(payload), offset)
        out[offset:offset + 4] = tag
        struct.pack_into(BE + "I", out, offset + 4, len(payload))
        out[offset + 8:offset + 8 + len(payload)] = payload

    # unused map slots, mirroring the donor's 'free' entries
    for i in range(n, max_map_entries):
        put_entry(i, b"free", 0, 0, flags=12, nxt=-1)

    return bytes(out)


def find_tests_cpp():
    """Locate engines/director/tests.cpp by walking up from this file."""
    for parent in pathlib.Path(__file__).resolve().parents:
        candidate = parent / "engines" / "director" / "tests.cpp"
        if candidate.is_file():
            return candidate
    raise FileNotFoundError(
        "engines/director/tests.cpp not found; run this from a ScummVM checkout")


def load_donor(path=None):
    """Without a path the donor is the test movie embedded in tests.cpp: a
    complete D4 RIFX with a text member, a shape member and a score, and free of
    licensing concerns since it already ships with ScummVM.

    For D5 and later there is no such donor in the tree, so `path` may point at
    any real movie of the wanted version. Its config, cast library mapping and
    text member get cloned; the result must stay on that machine, since it
    derives from game data.
    """
    if path is not None:
        return rifx.RifxFile(pathlib.Path(path).read_bytes())
    src = find_tests_cpp()
    m = re.search(r"const byte testMovie\[\]\s*=\s*\{(.*?)\};",
                  src.read_text(encoding="utf-8", errors="replace"), re.S)
    if not m:
        raise ValueError("testMovie[] blob not found in %s" % src)
    data = bytes(int(x, 16) for x in re.findall(r"0x([0-9A-Fa-f]{2})", m.group(1)))
    return rifx.RifxFile(gzip.decompress(data))


def extend_lnam(payload, names):
    """Adds names to a name table, keeping the existing ones at their index.

    The Lctx has a single nameTableId, so a movie has one name list; scripts we
    append have to share the donor's. Returns (chunk, indices) where indices
    gives, for each requested name, the index it can be referenced by.

    A name the donor already has is reused rather than appended. Its own scripts
    reference `exitFrame` as name 0, because that is where it already sits, and
    writing a second copy further down works but says something no real movie
    says.
    """
    head = payload[0:8]
    offset = struct.unpack_from(BE + "H", payload, 16)[0]
    count = struct.unpack_from(BE + "H", payload, 18)[0]
    existing, pos = [], offset
    for _ in range(count):
        n = payload[pos]
        existing.append(payload[pos + 1:pos + 1 + n].decode("latin1"))
        pos += 1 + n

    table = list(existing)
    indices = []
    for name in names:
        if name in table:
            indices.append(table.index(name))
        else:
            indices.append(len(table))
            table.append(name)

    # Keep whatever the donor subtracted from the chunk length before storing
    # it: nothing up to D6, the 20 byte header on D7.
    stored = struct.unpack_from(BE + "I", payload, 8)[0]
    return lscr.build_lnam(table, head, size_delta=len(payload) - stored), indices


def unlink_donor_scripts(payload):
    """Points every context entry the donor brought at nothing.

    Emptying a script's CAS* slot is not enough to stop it running: Director
    reaches its code through the Lingo context, not through the cast. The D7
    donor's member 2 carries `on stopMovie`, our films play four frames and then
    stop, and up it comes -- `Script error: Movie cast not found` for the
    external cast we no longer declare.

    The Lscr chunks stay in the file with nothing pointing at them, and our
    scripts keep their positions, so the scriptIds in their cast info still line
    up.

    An unused entry is not just a -1 where the Lscr index was: in both donors it
    also carries entryFlags 0 where a live one carries the movie's flag value,
    the header's validCount counts only the live ones, and firstUnused names the
    first free slot. Leaving those three saying what they said before is what
    crashes Director when the film closes -- graft-d7, which touches the cast
    but not this table, closes cleanly.
    """
    out = bytearray(payload)
    count = struct.unpack_from(BE + "i", out, 8)[0]
    items_offset = struct.unpack_from(BE + "H", out, 16)[0]
    entry_size = struct.unpack_from(BE + "H", out, 18)[0] or 12
    for i in range(count):
        off = items_offset + i * entry_size
        struct.pack_into(BE + "i", out, off + 4, -1)
        struct.pack_into(BE + "H", out, off + 8, 0)         # entryFlags
        struct.pack_into(BE + "h", out, off + 10, -1)       # nextUnused
    struct.pack_into(BE + "h", out, 36, 0)                  # validCount
    struct.pack_into(BE + "h", out, 40, 1 if count else -1)  # firstUnused
    return bytes(out)


def live_lctx_entry_flags(payload):
    """The entryFlags a donor puts on a context entry that points at an Lscr.

    0 on the D6 donor, 4 on the D7 one -- so it is not a constant, and our own
    entries were going in with 0 either way.
    """
    count = struct.unpack_from(BE + "i", payload, 8)[0]
    items_offset = struct.unpack_from(BE + "H", payload, 16)[0]
    entry_size = struct.unpack_from(BE + "H", payload, 18)[0] or 12
    for i in range(count):
        off = items_offset + i * entry_size
        if struct.unpack_from(BE + "i", payload, off + 4)[0] >= 0:
            return struct.unpack_from(BE + "H", payload, off + 8)[0]
    return 0


def extend_lctx(payload, lscr_indices, entry_flags=0):
    """Appends script context entries, keeping the donor's at their position.

    An entry's 1-based position is the lctxIndex handed to addCodeV4 and has to
    match the scriptId in the member's cast info, so appending rather than
    replacing leaves the donor's own scripts resolvable. Returns (chunk, base)
    where base is the 1-based position of our first entry.
   
    `entry_flags` should be what the donor puts on its own live entries; ours
    were going in with 0 while the D7 donor uses 4.
    """
    out = bytearray(payload)
    count = struct.unpack_from(BE + "i", out, 8)[0]
    items_offset = struct.unpack_from(BE + "H", out, 16)[0]
    entry_size = struct.unpack_from(BE + "H", out, 18)[0] or 12
    end = items_offset + count * entry_size

    tail = bytes(out[end:])
    out = bytearray(out[:end])
    for idx in lscr_indices:
        out += struct.pack(BE + "I", 0)
        out += struct.pack(BE + "i", idx)
        out += struct.pack(BE + "Hh", entry_flags, -1)
    out += tail

    new_count = count + len(lscr_indices)
    struct.pack_into(BE + "ii", out, 8, new_count, new_count)
    # validCount counts the entries that point at something, not all of them
    live = sum(1 for i in range(new_count)
               if struct.unpack_from(BE + "i", out,
                                     items_offset + i * entry_size + 4)[0] >= 0)
    struct.pack_into(BE + "h", out, 36, live)
    return bytes(out), count + 1
