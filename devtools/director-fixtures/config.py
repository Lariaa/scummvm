"""Read, patch and re-checksum a Director config chunk (VWCF / DRCF).

Field offsets follow Cast::loadConfig (engines/director/cast.cpp:380ff) and the
checksum follows Cast::computeChecksum (:1198). Both are validated by
recomputing the checksum of an untouched chunk and comparing it with the value
stored in the file, so a wrong field mapping shows up immediately.
"""
import struct

M32 = 0xFFFFFFFF


def _u32(v):
    return v & M32


def parse(chunk):
    """Returns the fields computeChecksum() needs, by offset."""
    u16 = lambda o: struct.unpack_from(">H", chunk, o)[0]
    i16 = lambda o: struct.unpack_from(">h", chunk, o)[0]
    i32 = lambda o: struct.unpack_from(">i", chunk, o)[0]
    return dict(
        len=u16(0), fileVersion=u16(2),
        top=i16(4), left=i16(6), bottom=i16(8), right=i16(10),
        castArrayStart=u16(12), castArrayEnd=u16(14),
        readRate=chunk[16], lightswitch=chunk[17], unk1=i16(18),
        commentFont=u16(20), commentSize=u16(22), commentStyle=u16(24),
        stageColor=u16(26), bitdepth=u16(28),
        field17=chunk[30], field18=chunk[31], field19=i32(32),
        version=u16(36), movieDepth=i16(38),
        field22=i32(40), field23=i32(44), field24=i32(48),
        field25=struct.unpack_from(">b", chunk, 52)[0],
        field26=struct.unpack_from(">b", chunk, 53)[0],
        frameRate=i16(54), platformID=u16(56), protection=i16(58),
        field29=i32(60), checksum=struct.unpack_from(">I", chunk, 64)[0],
    )


def human_version(v):
    for lo, human in ((0x782, 1150), (0x781, 1100), (0x73B, 1000), (0x6A4, 850),
                      (0x582, 800), (0x4C8, 700), (0x4C2, 600), (0x4B1, 500),
                      (0x45D, 404), (0x45B, 400), (0x405, 310), (0x404, 300),
                      (0x400, 200)):
        if v >= lo:
            return human
    return 100


def compute_checksum(f):
    """Port of Cast::computeChecksum() for pre-D7 movies."""
    human = human_version(f["version"])

    # From D7 the two 16-bit slots at 18 and 26 hold packed RGB bytes instead.
    # In a big endian movie operand 11 works out to the same value either way
    # ((G << 8) | B reassembles unk1); only operand 15 changes, from the whole
    # 16-bit stage colour to just its low byte, which is the R component
    # (cast.cpp:464ff and :1213ff).
    operand11 = f["unk1"]
    operand15 = (f["stageColor"] & 0xFF) if human >= 700 else f["stageColor"]

    check = _u32(f["len"] + 1)
    check = _u32(check * (f["fileVersion"] + 2))
    check //= (f["top"] + 3)
    check = _u32(check * (f["left"] + 4))
    check //= (f["bottom"] + 5)
    check = _u32(check * (f["right"] + 6))
    check = _u32(check - (f["castArrayStart"] + 7))
    check = _u32(check * (f["castArrayEnd"] + 8))
    read_rate = f["readRate"] - 256 if f["readRate"] > 127 else f["readRate"]
    check = _u32(check - (read_rate + 9))
    check = _u32(check - (f["lightswitch"] + 10))
    check = _u32(check + operand11 + 11)
    check = _u32(check * (f["commentFont"] + 12))
    check = _u32(check + f["commentSize"] + 13)
    if human < 800:
        check = _u32(check * (((f["commentStyle"] >> 8) & 0xFF) + 14))
    else:
        check = _u32(check * (f["commentStyle"] + 14))
    check = _u32(check + operand15 + 15)
    check = _u32(check + f["bitdepth"] + 16)
    check = _u32(check + f["field17"] + 17)
    check = _u32(check * (f["field18"] + 18))
    check = _u32(check + f["field19"] + 19)
    check = _u32(check * (f["version"] + 20))
    check = _u32(check + f["movieDepth"] + 21)
    check = _u32(check + f["field22"] + 22)
    check = _u32(check + f["field23"] + 23)
    check = _u32(check + f["field24"] + 24)
    check = _u32(check * (f["field25"] + 25))
    check = _u32(check + f["frameRate"] + 26)
    check = _u32(check * (f["platformID"] + 27))
    check = _u32(check * _u32(f["protection"] * 0xE06 + 0xFF450000))
    check ^= 0x72616C66                      # 'ralf'
    return _u32(check)


def self_check(chunk):
    """True when our field mapping reproduces the stored checksum."""
    f = parse(chunk)
    return compute_checksum(f) == f["checksum"]


def set_cast_array_end(chunk, last_member):
    """Director uses castArrayEnd to decide how many cast members exist; a stale
    value hides everything past it. ScummVM only consults the field on the
    D2/D3 path, so the mismatch is invisible there.

    Returns a new chunk with the field and the checksum updated.
    """
    out = bytearray(chunk)
    struct.pack_into(">H", out, 14, last_member)
    f = parse(out)
    struct.pack_into(">I", out, 64, compute_checksum(f))
    return bytes(out)
