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


def build_cas(cast_indices):
    """CAS* is a positional u32 array: slot i holds the mmap index of the CASt
    for cast member i+1, or 0 (RIFXArchive::writeCast)."""
    return b"".join(struct.pack(BE + "I", i) for i in cast_indices)


def build_key(entries, max_entries=None):
    """KEY*: (childIndex, parentIndex, tag) triples."""
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
SPRITE_TYPE_TEXT = 7            # kTextSprite (types.h:162)

# Bit 0x80 of the thickness byte is set on 89% of the sprites in real D4 scores
# (24505 of 27500 sampled across Max and Marie). ScummVM keeps the byte in
# Sprite::_thickness but never looks at this bit before D7, so its absence costs
# nothing there -- Director draws nothing at all without it.
SPRITE_THICKNESS_DEFAULT = 0x80


def sprite_d4(*, cast_member, x, y, w, h, sprite_type=SPRITE_TYPE_TEXT, ink=0,
              editable=False, fore=255, back=0, script_id=0,
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


def sprite_d6(*, cast_member, x, y, w, h, sprite_type=SPRITE_TYPE_TEXT, ink=0, editable=False,
              fore=255, back=0, cast_lib=DEFAULT_CAST_LIB):
    """24-byte D6 sprite record (writeSpriteDataD6, frame.cpp).
    The editable bit is still 0x40, but colorcode moved to byte 20."""
    out = bytearray()
    out += bytes([sprite_type, ink, fore, back])
    out += struct.pack(BE + "hH", cast_lib, cast_member)
    out += struct.pack(BE + "I", 0)                  # spriteListIdx
    out += struct.pack(BE + "HH", y, x)
    out += struct.pack(BE + "HH", h, w)
    out += bytes([0x40 if editable else 0x00, 0, SPRITE_THICKNESS_DEFAULT, 0])
    assert len(out) == 24
    return bytes(out)


def sprite_d7(*, cast_member, x, y, w, h, sprite_type=SPRITE_TYPE_TEXT, ink=0, editable=False,
              fore=255, back=0, cast_lib=DEFAULT_CAST_LIB):
    """48-byte D7 sprite record; the first 23 bytes match D6."""
    out = bytearray(sprite_d6(cast_member=cast_member, x=x, y=y, w=w, h=h,
                              sprite_type=sprite_type, ink=ink,
                              editable=editable, fore=fore, back=back,
                              cast_lib=cast_lib)[:23])
    out += bytes([0])                                # flags
    out += bytes([0, 0, 0, 0])                       # fg/bg colour G and B
    out += struct.pack(BE + "II", 0, 0)              # angleRot, angleSkew
    out += b"\0" * 12
    assert len(out) == 48
    return bytes(out)


def main_channel_d6plus(size, *, action_id=0, cast_lib=DEFAULT_CAST_LIB):
    """D6/D7 main channel. Only the frame script is filled in; everything else
    stays zero, which reads back as no tempo, transition or palette change.

    Follows readMainChannelsD6/D7: castLib and member are two u16 at offsets 0
    and 2. Note that writeMainChannelsD6/D7 disagree with that -- they emit a
    u32 for castLib -- so the reader is the authority here.
    """
    out = bytearray(size)
    struct.pack_into(BE + "HH", out, 0, cast_lib, action_id)
    return bytes(out)


def build_vwsc_d6plus(frames, *, main_size, spr_size, frames_version,
                      num_channels=50):
    """D6 and later wrap the familiar score in an index: a small prologue points
    at a table of offsets, whose entry 0 is the score header and whose entry n is
    frame n (Score::loadFrames and loadFrame, score.cpp:1905 / :2156).

        0   u32 framesStreamSize (whole chunk)
        4   u32 ver              (-3)
        8   u32 listStart        (12)
        12  u32 numEntries       (frames + 1)
        16  u32 listSize         (numEntries + 1)
        20  u32 maxDataLen       (size of the data area)
        24  u32 offsets[]        relative to the data area
            data area: score header, then the frames
    """
    header, body, frame_offsets = _score_header_and_frames(
        frames, main_size=main_size, spr_size=spr_size,
        frames_version=frames_version, num_channels=num_channels,
        declared_frames=0)
    data = header + body

    num_entries = len(frames) + 1
    list_size = num_entries + 1
    index_start = 24
    frame_data_offset = index_start + list_size * 4

    out = bytearray()
    out += struct.pack(BE + "I", 0)                  # patched below
    out += struct.pack(BE + "i", -3)
    out += struct.pack(BE + "I", 12)
    out += struct.pack(BE + "III", num_entries, list_size, len(data))
    out += struct.pack(BE + "I", 0)                  # entry 0: the score header
    for off in frame_offsets:                        # already header relative
        out += struct.pack(BE + "I", off)
    out += b"\0" * 4 * (list_size - num_entries)     # spare index slots
    assert len(out) == frame_data_offset, (len(out), frame_data_offset)
    out += data
    struct.pack_into(BE + "I", out, 0, len(out))
    return bytes(out)


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
            frame += struct.pack(BE + "HH", len(payload), offset)
            frame += payload
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
            frame += struct.pack(BE + "HH", len(data), offset)
            frame += data
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
