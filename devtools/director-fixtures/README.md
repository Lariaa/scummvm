# Director regression fixtures

Generates small Director movies that make specific engine regressions visible.
The movies use nothing but stock Lingo, so they run in real Director as well as
in ScummVM, and each one prints its verdict into a field on stage (and via `put`,
which reaches the ScummVM log and Director's Message window).

Status: the D4 pair is proven -- `editable.dir` reports `editable 101` on an
upstream build and `editable 111` on one with the fixes, which is exactly what it
exists to distinguish. The D6 and D7 variants load and render in ScummVM but
Director still refuses them, and their frame scripts do not run; see the trap
lists at the end before touching them.

## Running the movies

    scummvm -p devtools/director-fixtures/movies --debugflags=noloop directortest-all

The directory carries the empty `lingotests-all` marker file that the
`directortest-all` target detects. Without `noloop` a movie restarts at frame 1
forever instead of handing over to the next one (`score.cpp:477`), so a plain run
never terminates.

In Director, open the `.dir` directly and play it. Do not save: Director 6 and
later convert the file format on save. Each script cast member also carries its
Lingo source in the cast info, so the scripts are readable in the script window.

## movies/editable.dir

Covers:

* `616b3d29cd9` DIRECTOR: Mark a channel dirty when the score toggles the editable flag
* `5a896cf1c70` DIRECTOR: Follow the score editable flag on the same cast member

Sprite 1 shows the same text member at the same position in every frame; only the
score's editable bit changes -- the situation `LOG_IN.DXR` in "Ein Fall für Mütze
& Co" runs into. Frame 2 additionally changes the back colour, so that channel is
dirty regardless of the editable comparison, which isolates `Sprite::replaceFrom()`.
Frame 3 changes *only* the editable bit, which isolates `Channel::isDirty()`.

Each frame appends one digit to a global, and the last frame displays it:

| On stage       | Meaning                                                     |
| -------------- | ----------------------------------------------------------- |
| `editable 111` | both fixes present                                           |
| `editable 101` | `5a896cf1c70` missing -- editable never follows the score    |
| `editable 110` | `616b3d29cd9` missing -- the editable-only transition is skipped |

The verdict is written into cast member 3, not into the member under test:
`Channel::isDirty()` also returns true when `Cast::isModified()` is set, so
writing into the tested member would mask the very bug this checks for.

## movies/listoverride.dir

Covers:

* `83c04ad6fe0` DIRECTOR: Let list builtins override same-named handlers from me methods

A factory defines its own `getLast` method and still calls
`getLast(["IN_OUT", "LINTRANS"], 2)` from inside `mNew` -- the shape the
`SpecialAddresses` parent script in "Ein Fall für Mütze & Co" uses. The list
builtin has to win over the object's own method.

| On stage        | Meaning                                          |
| --------------- | ------------------------------------------------ |
| `list LINTRANS` | fix present -- the builtin ran                    |
| `list METHOD`   | fix missing -- the factory's own method ran       |

The same case is also covered from source by
`engines/director/lingo/tests/listoverride.lingo`, which needs no movie.

## Director 6 and 7 variants

The same two checks exist for D6 and D7, because the editable bit lives in a
different place in every score format (`colorcode & 0x40` sits at byte 18 on D4
but at byte 20 on D6 and D7) and only these variants exercise
`readSpriteDataD6`/`D7`. The three commits themselves are version independent, so
the D4 movies already cover them; these add the version specific score parsing.

There is no license free D6 or D7 donor in the tree, so the generator clones the
config, cast library mapping and text member out of a real movie you point it at:

    python devtools/director-fixtures/make_fixtures.py \
        --d6 /path/to/some_d6_movie.dxr --d7 /path/to/some_d7_movie.dxr

That produces `editable-d6.dir`, `listoverride-d6.dir`, `editable-d7.dir` and
`listoverride-d7.dir`, which live in `movies-d6d7/`, plus
`editable-d6-append.dir` and `editable-d7-append.dir`, which build the same movie
without renumbering the donor's resources -- see "Append, never renumber" below
for why that distinction exists. The verdicts on stage are the
same as for the D4 movies:

    scummvm -p devtools/director-fixtures/movies-d6d7 directortest-all

They are kept apart from `movies/` on purpose. Unlike the D4 pair, they clone
chunks out of a commercial movie, so they carry third party data and are not
suitable for upstream.

### movies-d6d7/probe

Director rejects the D6 and D7 fixtures without saying what it dislikes. These
two carry the donor's own score, untouched, plus the two text members and
nothing else -- no scripts, no generated `VWSC`. Loading one of them in Director
splits the question in a single try:

* it loads -- the cast side is sound and the fault is in the score we generate
* it does not -- the fault lies earlier, in the cast, the `MCsL` or the container

They sit in a subdirectory so `directortest-all`, which does not recurse, leaves
them out of the ScummVM sweep.

Picking a donor:

* It must be **unprotected**, which in practice means a `.dir` and not a `.dxr`.
  Protection is a flag in the config chunk, so cloning that chunk carries it over
  and the fixture stays protected whatever you name it -- Director then refuses to
  open it. The generator checks this and stops. To use a protected movie anyway,
  run `projectorrays decompile <movie> -o <dir>` first, which writes an open copy.
* Prefer **big endian** (`RIFX`, not `XFIR`). Structural chunks follow the
  container's byte order and the generator writes big endian throughout. (`Lscr`
  and `Lnam` are big endian even inside an `XFIR` movie, so a little endian
  fixture would need mixed byte order.)
* Read the version at **offset 36** of the config chunk, not at offset 2 --
  offset 2 is `fileVersion`, which is garbage in protected movies.
* It needs a text cast member; the generator clones the first one it finds.

## Regenerating

    python devtools/director-fixtures/make_fixtures.py

Writes into `out/` next to the script. The generator reads the donor movie out of
`engines/director/tests.cpp`, so it needs to run from a checkout.

## How this is put together

ScummVM can write movies (`RIFXArchive::writeToFile`), but `rebuildResources()`
does not know `Lscr`/`Lnam`/`Lctx`, and there is no Lingo-to-bytecode compiler --
so the movies are assembled here instead, using ScummVM's readers as the
byte-level specification:

| Module          | Mirrors                                                    |
| --------------- | ---------------------------------------------------------- |
| `rifx.py`       | `RIFXArchive::readMemoryMap` (archive.cpp)                  |
| `lscr.py`       | `LingoCompiler::compileLingoV4`, `addNamesV4`               |
| `lscr_asm.py`   | the `lingoV4[]` opcode table (lingo-bytecode.cpp)           |
| `build_movie.py`| cast, score and container chunks                            |

`lscr_asm.build_lscr()` reproduces a real Director 4 script byte for byte
(`STRTMAX2.dir` from "Max and Marie Go Shopping", a 164 byte
`on exitFrame / updateStage / end`), which is what the assembler is checked
against. ProjectorRays decompiles `editable.dir` back to the intended Lingo.

### Traps ScummVM will tell you about

* A script cast member's info block needs **seven** strings. String 0 is the raw
  source, string 1 the name as a *Pascal* string -- `Cast::loadCastInfo` reads
  them as `strings[0].readString(false)` and `strings[1].readString()`. Fewer
  entries make readers run off the end of the block.
* mmap entries start at `mmapOffset + 8 + headerSize`; the header size does not
  count the 8 byte tag+size preamble.
* `Frame::readChannel()` dispatches on the *movie* version, not on the score
  header, so anything testing the editable bit has to be a D4+ movie.
* The bounds checks in `compileLingoV4` are `>=`, so handler code and the name
  arrays must end strictly before the end of the chunk.
* The D6+ score wraps the familiar score in an offset table, but does not change
  it: entry 0 is the *whole* score, header and every frame back to back exactly
  as before D6. The remaining entries are the per sprite detail blobs
  (`_spriteDetailOffsets`, `score.cpp:1920`) and a movie without behaviours has
  none. This one is worth checking against a donor before believing it -- see
  "A frame per index entry" below.
* Constant index entries are 6 bytes on D4 and 8 from D5 on, which also scales
  the operand of the constant push opcode.
* From D5 on, `the text of field` takes a cast lib, pushed after the member.

### Traps only Director will tell you about

Everything in this list was invisible to ScummVM -- it is tolerant, or derives
the value itself -- and every one of them was found by loading the movie in
Director and reading the error. If you change the generator, assume this list is
incomplete and test in Director before believing it works.

| Field | Must be | Why ScummVM does not care |
| ----- | ------- | ------------------------- |
| `imap` version | the real version (0 for D4, 0x4c7 for D6, 0x57e for D7) | it takes the version from the config chunk |
| config `protection` (offset 58) | not a multiple of 23, i.e. an unprotected donor | protection does not gate loading |
| config `castArrayEnd` (offset 14) | the actual member count, **with the checksum recomputed** | it loads members from `CAS*`, and only reads the field on the D2/D3 path |
| `MCsL` min/maxMember and libResourceId (D5+) | built to match the movie, never cloned | ditto -- and it overrides `castArrayEnd` |
| sprite type | on D4 the type of the member, 7 for text; **from D5 on 16, `kCastMemberSprite`**, and the member decides | it overwrites the score's value from the cast member (`sprite.cpp:550`) |
| sprite thickness bit 0x80 | set, as on 89% of real D4 sprites | the byte is only ever compared before D7 |
| factory `parentNumber` | an index at another script, never its own | it stores the field and never reads it |
| `Lscr` global list | every global the script touches | it creates a global on first use |
| **the Lingo source in the cast info** | the same `global` declarations as the bytecode | it runs the bytecode and ignores the source |
| **every donor resource's mmap index** | unchanged -- append, never repack | it resolves resources through `KEY*` and the maps it just read |
| sprite fore/back colour | a palette index that is actually visible -- the donor's, not a round number | it renders through `transformColor()` and its own palette handling |
| **`spriteListIdx`** (D6+, sprites and the frame script) | a real detail index, never 0, and **always a multiple of 3** | `if (sprite->_spriteListIdx)` skips the lookup (`score.cpp:2081`); Director follows it unconditionally, and 0 is entry 0, the score. An index off the multiple of 3 makes it read a behaviour list as a SpriteInfo and die |
| D6+ `numOfFrames` | 0 on framesVersion 11, the real count on 13 | it recounts the frames itself (`score.cpp:1999`) |

The last one is the one to remember: the source kept next to the bytecode is not
documentation. Director compiles from it, and its error messages quote those
lines. ProjectorRays reads the bytecode, so it will happily confirm a script the
source contradicts -- the two have to be kept in step by hand.

### If Director refuses a movie

Its messages are terse but specific, and each one so far pointed straight at a
field:

| Message | It meant |
| ------- | -------- |
| `Could not find chunk: ChunkID='FCWV'` | the config chunk was not usable; 'FCWV' is how a Windows build prints 'VWCF' |
| `Problem reading file ...: -50` | `paramErr` -- a resource was described but absent, in our case by a cloned `MCsL` |
| `Script error: Variable used before assigned a value` | an undeclared global, in the *source* rather than the bytecode |
| `There is not enough memory ...` | not memory at all -- a resource reference landed on the wrong chunk, in our case because the mmap was renumbered |
| Director crashes outright | a sprite pointing at a member of the wrong type; ours put a text sprite on a script member |
| no message, black stage, empty Score window | the score parsed to zero frames -- check `frame1Offset` and `framesStreamSize` in entry 0 |

`movies-d6d7/probe` exists to split "the score is wrong" from "something earlier
is wrong" in a single load attempt; build the same kind of cut-down movie when a
new failure appears rather than guessing at fields.

### Read the ladder before drawing the conclusion

The first ladder was built on a conclusion that turned out to be wrong. Because
`probe-d4` rendered and `editable.dir` did not, and the two differ in their
score, the score looked like the only candidate and the rungs walked from the
donor's score to ours. Running them in Director 5 showed that **rung a is empty
too** -- and rung a carries the donor's score, byte for byte identical to the one
`probe-d4` renders. The score was never the cause. What the ladder actually
proved is that everything it varies is innocent.

That is what a ladder is for, so this counts as it working, but it cost a round
of testing that reading rung a first would have saved. `castladder-a` … `-d`
walk the other gap -- two members, three, seven, and seven with a Lingo context
-- with the donor's score throughout.

### Append, never renumber

The D6 and D7 fixtures were first built by reading the donor apart and writing a
fresh container. That produced a movie ScummVM was perfectly happy with and
Director 7 answered with "out of memory". The bisection ladder pinned it down:
`ladder-d6-0`, which keeps every resource at its original index, loads;
`ladder-d6-1`, identical except that the index space is packed, does not. So the
container layout is not the problem -- **renumbering is**. Something outside the
chunks we regenerate refers to mmap indices, and once a resource moves, that
reference points at whatever now sits there.

`rebuild_preserving_indices()` is the rule made mechanical. Every donor resource
keeps its index, free and junk entries included; new chunks take indices after
the donor's last; payloads we do change are swapped in place. `KEY*` is extended
rather than rebuilt, and `extend_lnam()`/`extend_lctx()` do the same for the name
table and the script context, whose *positions* are themselves referenced --
`Lctx.nameTableId` picks the one name table, and an entry's 1-based position is
the `scriptId` a script cast member stores in its info block.

Practical consequence: a fixture can only ever add to a donor. If you need a
member the donor does not have, clone one it does have and append the copy.

### A frame per index entry

The D6+ score writer put each frame in an index entry of its own. ScummVM read
that back perfectly -- it walks the table and takes whatever it finds -- and
Director read it as a score with **no frames at all**: a black stage, no error,
nothing in the Score window. It cost several rounds of testing, because a black
stage looks exactly like a donor whose own stage is black.

What settled it was decoding both donors' scores with the same parser the writer
uses, which is the check that had already caught every earlier mistake and which
the score writer had never been put through. Entry 0 is the whole score: a 20
byte header whose `frame1Offset` is 20 and whose `framesStreamSize` is where the
frame stream ends, followed by the frames. Both donors parse to exactly that,
down to the byte.

The lesson generalises past this one field. **Round-trip every writer against a
real chunk before trusting it** -- reproduce a donor byte for byte, or decode a
donor with the same code and check the totals close. `lscr_asm`, `build_mcsl` and
the config checksum were all built that way and none of them ever produced a bug
Director found first; the score writer was not, and it did.

### Measure it against the archive

Two of the traps above were settled not by reading Director's source, which we
do not have, but by counting what real movies do. Sweeping every Director file
under a games archive takes a few minutes and answers questions no amount of
staring at one donor can:

* entry 0 is the whole score -- **5020 of 5020** D6+ scores parse that way
* a cast member sprite is type 16 -- 549273 records say so and none says
  anything else
* `spriteListIdx` is never 0 -- 577649 records, not one exception
* `numOfFrames` is 0 on framesVersion 11 and the real count on 13 -- without
  exception in either direction
* a channel is written whole or in pieces the D4 shape never uses: counting
  4.7 million channel writes, the whole record at the channel start is common
  (24 bytes 74530 times on D6, 48 bytes 343435 times on D7) and the D4 habit of
  16 bytes then the rest never appears

That last pair mattered because a single donor cannot show you a rule; it can
only show you one instance of it, and the D6 donor turned out to have no sprites
in its score at all. When a field's correct value is not obvious, count.

While you are there, take the geometry from the donor rather than inventing it:
`donor_score_geometry()` lifts the frames version, sprite record size and channel
counts out of its header. Those describe the movie, not our frames.

Decoding the donors' frames the same way is worth the few minutes on its own. It
showed that the sprite records carried the wrong type -- D4 puts the member's own
type in the record and the generator kept doing that on D6 and D7, where a sprite
showing a cast member is `kCastMemberSprite`, 16. It also showed that the **D6
donor's score contains no sprites at all**, only main channel writes. Its black
stage is therefore genuine and says nothing about the fixture; the two sprite
records in frame 10 of the D7 donor are the only real reference either version
has, which is enough, because D6 and D7 share the first 24 bytes of the record.
