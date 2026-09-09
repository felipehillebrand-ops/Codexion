*This project has been created as part of the 42 curriculum by fjose-hi.*

# Codexion

## Description

Codexion is a multithreaded simulation of a fixed-size, shared-resource
contention problem, inspired by the classic Dining Philosophers scenario.
`number_of_coders` coders sit in a circular co-working hub, each represented
by a POSIX thread. Between every pair of neighbouring coders sits one shared
USB dongle. Compiling requires **two** dongles held simultaneously (the
coder's left and right dongle); after compiling, the coder releases both
dongles and moves on to debugging and refactoring — two phases that require
no shared resource at all.

The goal is to keep every coder compiling regularly enough that none of
them "burns out" (misses its `time_to_burnout` deadline since the start of
its last compile), while enforcing fair, deadlock-free arbitration between
coders competing for the same dongle, under a configurable **FIFO** or
**EDF** scheduling policy.

The simulation stops either when every coder has completed
`number_of_compiles_required` compiles, or as soon as any coder burns out.

## Instructions

### Build
```sh
make          # builds the "codexion" binary
make clean    # removes object files
make fclean   # removes object files and the binary
make re       # fclean + all
```

### Run
```sh
./codexion number_of_coders time_to_burnout time_to_compile time_to_debug \
           time_to_refactor number_of_compiles_required dongle_cooldown scheduler
```

All 8 arguments are mandatory, non-negative integers except `scheduler`,
which must be exactly `fifo` or `edf`. All time values are in milliseconds.

Example:
```sh
./codexion 5 2000 200 200 200 10 0 fifo
```

### Compilation flags
The project builds in C89 (`-std=c89`) with `-Wall -Wextra -Werror -pthread`.
On Linux, `-D_DEFAULT_SOURCE` is also required: `usleep()` is a POSIX
function (explicitly listed as an allowed external function by the
subject), not part of the C89 standard itself, so glibc hides its
declaration under strict C89 mode unless a POSIX feature-test macro is
defined. This flag does not relax the language dialect the compiler
accepts — it only restores visibility of the POSIX declarations the
subject already requires us to use.

## Blocking cases handled

### Deadlock prevention (circular wait, Coffman's 4th condition)
Compiling requires holding two dongles simultaneously. If every coder
always requested its *left* dongle first and then its *right* one, a
circular-wait deadlock is possible (the classic Dining Philosophers
trap): every coder holds one dongle and waits forever for its neighbour's.

This project breaks the circular wait by always requesting dongles in a
**fixed, global order**: whichever of the coder's two dongles has the
**lower `id`** is always requested first (`coder_acquire_dongles` in
`dongle_acquire.c`). Because dongle IDs are fixed and shared, no cycle of
"coder A waits on the dongle coder B holds, who waits on the dongle
coder C holds, ... who waits on the dongle coder A holds" can ever form —
the acquisition order is a total order on dongle IDs, not a per-coder
local order.

### Starvation prevention / fairness
Each dongle keeps its own waiting queue implemented as a binary min-heap
(`heap_push`/`heap_pop`/`heapify_up`/`heapify_down` in `heap/`). Under
`fifo`, the heap key is a monotonically increasing per-dongle request
counter (first come, first served). Under `edf`, the key is the coder's
current burnout deadline (`last_compile_start + time_to_burnout`), so the
coder closest to burning out is served first, even if it asked later.

**EDF tie-breaker**: on equal deadlines, the coder with the **higher
`coder_id`** wins (verified with a dedicated unit test pushing
conflicting keys into the heap before wiring it into the dongle logic).

### Single-coder edge case (n = 1)
With only one coder, `left_dongle == right_dongle`: there is physically
only one dongle on the table. Compiling still requires *two* dongles held
simultaneously, so a single coder can **never** compile — the second
acquisition attempt blocks on a dongle the coder itself already holds,
which will never be released (release only happens after a successful
compile). The coder therefore waits until `time_to_burnout` elapses and
burns out, exactly matching the documented reference behaviour
(`./codexion 1 800 200 200 200 10 0 fifo` burns out around t=800ms). No
special-casing was needed in the acquisition code once this was
understood — the general two-dongle algorithm already produces the
correct behaviour by construction.

### Cooldown handling
After a coder releases a dongle, `dongle_release_single` stamps
`available_at_ms = now + dongle_cooldown`. `dongle_is_ready` refuses to
grant the dongle to the next waiter until that timestamp has passed, and
`pthread_cond_timedwait` is used (rather than a busy-poll loop) so a
waiting thread sleeps efficiently until either the cooldown expires or a
`broadcast` wakes it for re-evaluation.

### Precise burnout detection (≤10ms)
A dedicated monitor thread polls every coder's progress once per
millisecond (`MONITOR_POLL_MS` in `monitor.c`), comparing
`now - last_compile_start` against `time_to_burnout`. A coder that has
already completed its required number of compiles is excluded from this
check (fixed after an early false-positive bug), since a coder that has
legitimately finished should never be penalised for the remaining coders
still working. The 1ms poll interval keeps burnout detection safely
inside the 10ms tolerance window required by the subject.

### Log serialization (no interleaving)
A single `log_lock` mutex protects every `printf` call that writes a
simulation log line (`log.c`). No two log lines can ever interleave
character-by-character, and the timestamp used is captured while holding
the same lock that serializes the print, keeping timestamp order and
print order consistent.

### Clean shutdown without residual deadlocks
When the simulation stops (burnout or quota reached), the monitor thread
calls `wake_all_dongles`, which broadcasts on every dongle's condition
variable. Combined with periodic 50ms timeouts on every
`pthread_cond_timedwait` call as a safety net, this guarantees that no
coder thread can remain blocked forever waiting on a dongle after the
simulation has ended, so `pthread_join` in `main.c` always returns.

## Thread synchronization mechanisms

| Resource | Primitive | Purpose |
|---|---|---|
| Each dongle's availability/queue | `pthread_mutex_t` + `pthread_cond_t` (per dongle) | Mutual exclusion on `is_available`/`available_at_ms`/waiting-queue state; condition variable to sleep/wake waiters efficiently instead of busy-polling |
| Log output | `pthread_mutex_t log_lock` (in `t_data`) | Serializes every `printf` log line so no two lines interleave |
| "simulation stopped" flag | `pthread_mutex_t stop_lock` (in `t_data`) | Protects the only piece of state read by every coder thread *and* written by the monitor thread; accessed exclusively through `is_simulation_stopped`/`set_simulation_stopped` getters/setters, never directly |
| Each coder's own progress (`compiles_done`, `last_compile_start`) | `pthread_mutex_t progress_lock` (per coder) | Protects fields written by the coder's own thread but read by the monitor thread for burnout/quota checks, via `coder_get_*`/`coder_set_*` accessors |

No field that is shared between more than one thread is ever read or
written without going through one of these locks — this was enforced by
convention (every cross-thread field always has a matching lock and a
pair of accessor functions; direct field access from outside the
owning `.c` file is treated as a bug).

`pthread_cond_timedwait` (rather than a plain `pthread_cond_wait`) is
used everywhere a coder waits for a dongle, for two reasons: it allows
waking up automatically once a `dongle_cooldown` period ends without an
explicit signal, and it acts as a safety-net poll (bounded at 50ms) in
case a `broadcast` is ever missed, which is important given the
project's zero-tolerance policy on deadlocks.

## Design decisions

- **Fixed dongle-acquisition order by ID** to break circular wait
  (see "Blocking cases handled" above) rather than, e.g., a global lock
  around "acquire both dongles" — a global lock would serialize
  *all* compiling across the whole simulation, defeating the purpose of
  having multiple independent dongles.
- **One dongle = one mutex + one condition variable + one small
  (capacity-2) heap**, rather than one global priority queue for all
  dongles: at most two coders can ever contend for a specific dongle
  (its two physical neighbours), so a capacity-2 heap is sufficient and
  avoids unnecessary shared state between unrelated dongle pairs.
- **Threads created in coder-ID order** in `main.c`. This gives coder 1 a
  small statistical head start in early dongle contention on some runs,
  but never a hard guarantee — actual acquisition order still depends on
  real OS thread scheduling. This is documented here rather than
  "fixed" with an artificial start barrier, since the subject only
  requires fairness in dongle queueing (which the heap guarantees), not
  equal pacing between all coders from the first millisecond.
- **A `[debug] coder X compiled N times` summary is printed to `stderr`
  after the simulation ends**, unconditionally. `stderr` is used
  specifically so it never mixes with the official `stdout` log format
  the subject defines, and no extra external function (e.g. `getenv`)
  was needed to make it optional — redirecting `2>/dev/null` at the
  shell hides it if not wanted.

## Challenges faced

- **Windows vs Linux `long` overflow.** An early version of the
  ms→`timespec` conversion multiplied the whole absolute Unix epoch
  (`tv_sec`) by 1,000,000, which silently overflowed on Windows/MinGW
  (`long` is 32-bit there even in 64-bit builds) and caused
  `pthread_cond_timedwait` to wait on garbage timestamps, hanging the
  program. Rewriting the conversion to only ever add small offsets to
  `tv_sec` (never multiply it) fixed this on both platforms.
- **False burnout for already-finished coders.** The monitor initially
  checked every coder's deadline unconditionally, so a coder that
  finished its quota early could be falsely marked as burned out later
  in the run, while genuinely idle. Fixed by excluding coders that
  already reached `number_of_compiles_required` from the burnout check.
- **The single-coder edge case was initially designed incorrectly** (see
  "Blocking cases handled"): an early version let a single coder "cheat"
  by treating one dongle acquisition as sufficient. Comparing against the
  official evaluation sheet's documented reference behaviour revealed
  this was wrong; a single coder must never be able to compile at all.
- **A late-arriving log line after burnout.** A missing
  `is_simulation_stopped` check between the debug and refactor phases
  could let a coder print one more log line after "burned out" had
  already been printed. Added the missing checkpoint, plus an early
  stopped-check inside `dongle_acquire_single`, to keep "burned out" as
  the last printed line in the large majority of runs (a few-microsecond
  race window remains theoretically possible, consistent with the
  subject's own acknowledgment that "precise timing may vary slightly on
  some hardware").
- **Helgrind false positives.** `valgrind --tool=helgrind` intermittently
  reports `pthread_cond_{signal,broadcast}: ... associated lock is not
  held`, a documented false-positive pattern with modern glibc condition
  variable implementations. Cross-checked with `drd` (clean) and manual
  code audit (every broadcast call is always made while holding the
  matching mutex) to confirm this is not a real race.

## Performance analysis

- **Correctness under load**: tested up to 15 concurrent coders with
  `number_of_compiles_required = 10` (150 total compiles across heavy
  contention); every coder completed exactly its required number of
  compiles, with zero dongle duplication and zero false burnouts.
- **Memory**: `valgrind --leak-check=full --show-leak-kinds=all` reports
  no leaks across all tested scenarios, including the single-coder
  case where the dongle is created but potentially never fully
  acquired/released.
- **Race detection**: `valgrind --tool=drd` reports no errors on every
  tested scenario. `helgrind` occasionally reports a known false-positive
  (see "Challenges faced").
- **Timing accuracy**: burnout is detected and logged within roughly
  1-5ms of the theoretical deadline in practice (1ms monitor poll
  interval), comfortably inside the required 10ms tolerance.
- **Scalability note**: the topology is a cycle, so the maximum number
  of coders that can compile *simultaneously* is `floor(n/2)`, not `n`.
  With tight `time_to_burnout` budgets relative to
  `time_to_compile + dongle_cooldown`, this can make otherwise
  "reasonable-looking" parameters infeasible (some coders structurally
  cannot get a turn in time) — this was observed and is expected
  behaviour, not a bug, as confirmed against the reference
  `./codexion 5 500 200 200 200 10 0 fifo` case which is explicitly
  documented to be infeasible by design.

## Testing strategy

- **Unit-level**: the binary min-heap (push/pop/tie-breaker) was tested
  in isolation with hand-crafted key/coder_id combinations before being
  wired into the dongle acquisition logic, to separate scheduling-policy
  bugs from concurrency bugs.
- **Argument parsing**: manually tested with valid values, negative
  numbers, non-digit strings, mixed-digit-and-letter strings, and an
  invalid scheduler string, confirming each is rejected with a clear
  error message and none crash the program.
- **Reference scenarios** from the subject/evaluation sheet were run
  multiple times each (OS scheduling makes single runs non-conclusive):
  - `./codexion 1 800 200 200 200 10 0 fifo` → burns out around t≈800ms.
  - `./codexion 5 2000 200 200 200 10 0 fifo` → completes, no burnout.
  - `./codexion 5 2000 200 200 200 7 0 edf` → completes, no burnout.
  - `./codexion 5 500 200 200 200 10 0 fifo` → burns out around t≈500ms,
    "burned out" is the last printed line.
  - `./codexion 5 3000 200 200 200 10 400 fifo` → verified no
    "has taken a dongle" line for a given dongle occurs within 400ms of
    its previous release.
- **Stress testing**: 15 coders, `number_of_compiles_required = 10`,
  counted with `grep -c` per coder ID to get an exact, error-proof count
  rather than manually scanning hundreds of log lines.
- **Tooling**: `valgrind --leak-check=full --show-leak-kinds=all`,
  `valgrind --tool=drd`, and `valgrind --tool=helgrind` were all run
  against multiple scenarios (small/large coder counts, tight/loose
  timing budgets, both schedulers).

## Example usage

```sh
$ ./codexion 5 2000 200 200 200 10 0 fifo
0 1 has taken a dongle
0 1 has taken a dongle
0 1 is compiling
...
9010 5 is refactoring
[debug] coder 1 compiled 10 times
[debug] coder 2 compiled 10 times
[debug] coder 3 compiled 10 times
[debug] coder 4 compiled 10 times
[debug] coder 5 compiled 10 times

$ ./codexion 1 800 200 200 200 10 0 fifo
0 1 has taken a dongle
800 1 burned out
[debug] coder 1 compiled 0 times
```

## Resources

- POSIX Threads Programming (LLNL tutorial) — general reference on
  `pthread_create`/`pthread_mutex_*`/`pthread_cond_*` semantics.
- `man 3 pthread_cond_timedwait`, `man 3 pthread_mutex_init`,
  `man 2 gettimeofday`, `man 3 clock_gettime` — used to confirm exact
  argument types, return values and error conditions.
- The classic Dining Philosophers problem and E.W. Dijkstra's original
  resource-hierarchy solution — inspiration for the deadlock-avoidance
  strategy described below.
- Coffman's four necessary conditions for deadlock (mutual exclusion,
  hold-and-wait, no preemption, circular wait) — used as a checklist
  when designing dongle acquisition.

### How AI was used

An AI assistant (Claude) was used throughout the project as a design and
debugging partner, not as a code-generation shortcut copy-pasted without
review. Concretely, it was used for:

- **Initial architecture discussion**: splitting the project into small,
  single-responsibility files respecting the 42 five-functions-per-file
  limit, and deciding the struct layout in `codexion.h`.
- **Explaining unfamiliar APIs**: clarifying the exact semantics of
  `pthread_cond_timedwait` versus `pthread_cond_wait`, and why
  `struct timespec` needs an *absolute* time rather than a duration.
- **Bug hunting from real test output**: several real bugs were found by
  running the program myself, pasting the raw log/Valgrind output back to
  the assistant, and reasoning together about the root cause. Concrete
  examples: a `long` overflow bug in the ms→`timespec` conversion that
  only manifested on Windows (32-bit `long`), a data race on the shared
  "simulation stopped" flag, a logic error that let already-finished
  coders keep contending for dongles indefinitely, a false-burnout bug
  for coders that had already completed their quota, and — most
  importantly — a design bug in how the single-coder edge case was
  handled, corrected only after cross-checking against the official
  evaluation sheet's reference behaviour.
- **Interpreting Helgrind output**: understanding that the
  `pthread_cond_{signal,broadcast}: dubious...` warning is a known
  Helgrind/glibc false positive with modern condition-variable
  implementations, confirmed by cross-checking with `drd` and manual code
  audit (every `pthread_cond_broadcast` call in this project is always
  made while holding the associated mutex).

Every suggested change was manually reviewed, tested with multiple
parameter sets (including edge cases: 1 coder, 15+ coders, tight
cooldowns, infeasible burnout budgets), and understood before being kept
in the final code. No code was accepted without being able to explain
why it works.
