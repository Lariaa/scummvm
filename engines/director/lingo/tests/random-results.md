# Director `random` — one argument or two

What `random(a)` and `random(a, b)` answer, measured in **real Director 7.0.2**
and **real Director 8.5.1** (the Lingo engine Director MX carries).

The lexicon documents the one-argument form alone and writes a range as
`random(16) + 4` (*Director 8 Demystified*, p. 1001). The two-argument form is
undocumented, and the games use it anyway: **162 call sites** in the corpus pass
two arguments, and one Löwenzahn 8 run reaches them **2615 times**.

## Behaviour

- `random(a)` with `a >= 1` — uniform over **1..a**, both ends included.
- `random(a, b)` with `b > a` — uniform over **a..b**, **both ends included**,
  and the lower end may be zero or negative.
- `random(a, b)` with `b <= a` — **exactly like `random(a)`**. Director neither
  pins the result to the bound nor swaps the ends; it drops back to the
  one-argument form.
- `random(a)` with `a <= 0` — **undefined**: Director hands back uninitialised
  memory.

## Results

400 draws per case, per Director. **Director 7.0.2 and 8.5.1 answered alike in
every case**, so the rule carries no version gate.

| Case | min | max | distinct | |
|------|----:|----:|---------:|---|
| `random(3)` | 1 | 3 | 3 | control — a case whose answer nobody disputes |
| `random(0, 1)` | 0 | 1 | 2 | lower end **zero** |
| `random(12, 14)` | 12 | 14 | 3 | **both ends included** — not 12..13 |
| `random(0, 5)` | 0 | 5 | 6 | |
| `random(-1, 1)` | -1 | 1 | 3 | |
| `random(-3, 3)` | -3 | 3 | 7 | six call sites in the corpus |
| `random(-10, 0)` | -10 | 0 | 11 | span straddles zero |
| `random(-10, -5)` | -10 | -5 | 6 | both ends below zero |
| `random(5, 5)` | 1 | 5 | 5 | `b = a` → falls back on `random(5)` |
| `random(14, 12)` | 1 | 14 | 14 | `b < a` → falls back on `random(14)`, **no swap** |
| `random(0)` | — | — | **400** | uninitialised memory |
| `random(0, 0)` | — | — | **400** | same, through the fallback |
| `random(-3)` | — | — | **400** | same |

The last three produce a different value on every single draw, spread over the
whole signed 32-bit range. Observed extremes across the runs: `-2145681433` to
`2146289312`. The exact values are uninitialised memory and differ per run, so
only "400 distinct values in 400 draws" is reproducible.

## What ScummVM does with the undefined cases

It answers something **bounded** (1..65535) instead of a wild value. A movie
asking for a random number up to zero has already lost its footing, and handing
it `2146289312` would take the rest of the scene with it. `random.lingo`
therefore asserts the bound for those three cases and the exact span for the
other ten.

## Relevance

`random`'s arity in ScummVM was `1, 1`, so every two-argument call lost its
second argument to `LC::call`'s surplus-argument rule and silently answered
`random(a)` — a Löwenzahn 8 run logged 2615 of them. Fixed on branch
`xtras&more`, commit *"Let random take a second argument"*.

Measuring traps, all three hit while taking these numbers (see also the probe
handler at the top of `random.lingo`):

- A probe for Director must be written **comment-free**; with `--` comments
  Director reports the handler as undefined, and "Recompile All Scripts" stays
  silent about it.
- Line endings must be **CR alone**. LF runs the statements together, CRLF
  leaves the LF inside the line as a visible `¶`; both give "Comma expected".
- The script type must be `#movie`, or the handler is never looked up globally:
  `set the scriptType of member 1 to #movie`.
