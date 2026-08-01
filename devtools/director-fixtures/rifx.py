"""Minimal RIFX/XFIR reader, mirroring Director::RIFXArchive::readMemoryMap
(engines/director/archive.cpp:792).

Container header + imap + mmap are stored in the *container* endianness that the
metaTag encodes: 'RIFX' -> big endian, 'XFIR' -> little endian (byte-swapped).
Resource *contents* are big endian in both cases for Director 4 movies, which is
why ScummVM wraps them in a SeekableReadStreamEndian.
"""
import struct


class Resource:
    __slots__ = ("index", "tag", "offset", "size", "flags", "unk1", "next_free")

    def __init__(self, index, tag, offset, size, flags, unk1, next_free):
        self.index = index
        self.tag = tag
        self.offset = offset
        self.size = size
        self.flags = flags
        self.unk1 = unk1
        self.next_free = next_free

    def __repr__(self):
        return f"<{self.tag} #{self.index} @{self.offset} {self.size}B>"


class RifxFile:
    def __init__(self, data):
        self.data = data
        meta = data[0:4]
        if meta == b"RIFX":
            self.big = True
        elif meta == b"XFIR":
            self.big = False
        else:
            raise ValueError(f"not a RIFX container: {meta!r}")
        self.meta_tag = meta
        self.E = ">" if self.big else "<"
        self.rifx_type = self._tag(4 + 4)
        self._read_map()

    # --- helpers -----------------------------------------------------------
    def _tag(self, off):
        """Read a 4-byte tag. In XFIR files every 4-byte group is byte-swapped,
        so the tag characters come out reversed on disk."""
        raw = self.data[off:off + 4]
        return (raw if self.big else raw[::-1]).decode("latin1")

    def _u32(self, off):
        return struct.unpack_from(self.E + "I", self.data, off)[0]

    def _u16(self, off):
        return struct.unpack_from(self.E + "H", self.data, off)[0]

    # --- map ---------------------------------------------------------------
    def _read_map(self):
        pos = 12
        if self._tag(pos) != "imap":
            raise ValueError("imap expected at offset 12")
        self.imap_length = self._u32(pos + 4)
        self.mapversion = self._u32(pos + 8)
        mmap_off = self._u32(pos + 12)
        self.version = self._u32(pos + 16)

        pos = mmap_off
        if self._tag(pos) != "mmap":
            raise ValueError("mmap expected at offset %d" % mmap_off)
        self.mmap_offset = mmap_off
        self.mmap_length = self._u32(pos + 4)
        self.mmap_header_size = self._u16(pos + 8)
        self.mmap_entry_size = self._u16(pos + 10)
        self.total_count = self._u32(pos + 12)
        self.res_count = self._u32(pos + 16)
        # then 8 bytes of 0xFF and the first-free-resource id

        base = pos + 8 + self.mmap_header_size
        self.resources = []
        for i in range(self.res_count):
            e = base + i * self.mmap_entry_size
            self.resources.append(Resource(
                i,
                self._tag(e),
                self._u32(e + 8),
                self._u32(e + 4),
                self._u16(e + 12),
                self._u16(e + 14),
                self._u32(e + 16),
            ))

    # --- access ------------------------------------------------------------
    def chunk(self, res):
        """Resource payload, without the 8-byte tag+size preamble."""
        return self.data[res.offset + 8: res.offset + 8 + res.size]

    def by_tag(self, tag):
        return [r for r in self.resources if r.tag == tag]

    def summary(self):
        counts = {}
        for r in self.resources:
            counts[r.tag] = counts.get(r.tag, 0) + 1
        return counts
