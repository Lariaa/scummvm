# Director 4 regression fixtures

Generates small Director 4 movies that make specific engine regressions visible.
The movies use nothing but stock Lingo, so they run in real Director as well as
in ScummVM, and each one prints its verdict into a field on stage (and via `put`,
which reaches the ScummVM log and Director's Message window).

## Running the movies

    scummvm -p devtools/director-fixtures/movies directortest-all

The directory carries the empty `lingotests-all` marker file that the
`directortest-all` target detects.

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
`listoverride-d7.dir`. **These derive from game data and must not be committed** --
put them in `movies-local/`, which is ignored. The verdicts on stage are the same
as for the D4 movies.

Any movie of the right version with a text cast member works as a donor. Pick a
big endian one (`RIFX`, not `XFIR`): resource contents follow the container's byte
order, and the generator writes big endian throughout. Check a candidate's version
at **offset 36** of its config chunk, not at offset 2 -- offset 2 is `fileVersion`,
which is garbage in protected movies.

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

### Traps worth remembering

* A script cast member's info block needs **seven** strings. String 0 is the raw
  source, string 1 the name as a *Pascal* string -- `Cast::loadCastInfo` reads
  them as `strings[0].readString(false)` and `strings[1].readString()`. Fewer
  entries make readers run off the end of the block.
* `castArrayEnd` in `VWCF` only gates the D2/D3 path; D4 loads members through
  `getResourceIDList('CASt')`. Leave `VWCF` alone and its checksum stays valid.
* mmap entries start at `mmapOffset + 8 + headerSize`; the header size does not
  count the 8 byte tag+size preamble.
* `Frame::readChannel()` dispatches on the *movie* version, not on the score
  header, so anything testing the editable bit has to be a D4+ movie.
* The bounds checks in `compileLingoV4` are `>=`, so handler code and the name
  arrays must end strictly before the end of the chunk.
* For a factory, `parentNumber` is an index, not a flag. ScummVM never reads it,
  but decompilers do.
