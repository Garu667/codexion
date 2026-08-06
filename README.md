*This project has been created as part of the 42 curriculum by ramaroud.*

# codexion

## Description

**codexion** is a multithreaded simulation of the classic Dining Philosophers
problem, reframed as coders sharing scarce USB dongles in a co-working space.

`number_of_coders` coders sit in a circular arrangement around a shared
Quantum Compiler. There are exactly as many USB dongles as coders, arranged
the same way: each coder has one dongle on their left and one on their right.
To compile, a coder must hold **both** their dongles at the same time.

Each coder repeats an endless cycle:

```
acquire left + right dongle → compile → release both dongles → debug → refactor → (repeat)
```

If a coder fails to start compiling within `time_to_burnout` milliseconds of
their last compile (or of the start of the simulation), they **burn out**,
and the whole simulation stops. It also stops once every coder has compiled
at least `number_of_compiles_required` times.

The project's real goal is building a correct, deadlock-free,
starvation-free, and precisely-timed concurrent program using only POSIX
threads and mutexes, with a hand-rolled priority queue for fair dongle
arbitration.

## Instructions

### Compilation

```sh
git clone <this-repo-url>
cd codexion
make
```

This produces the `codexion` executable at the root of the project. The
Makefile also provides `clean`, `fclean`, and `re`.

### Usage

```sh
./codexion number_of_coders time_to_burnout time_to_compile time_to_debug \
           time_to_refactor number_of_compiles_required dongle_cooldown scheduler
```

| Argument                    | Meaning                                                                 |
|------------------------------|--------------------------------------------------------------------------|
| `number_of_coders`           | Number of coders (and number of dongles)                               |
| `time_to_burnout`             | ms since last compile start before a coder burns out                   |
| `time_to_compile`             | ms a compile takes (both dongles held)                                 |
| `time_to_debug`               | ms spent debugging                                                     |
| `time_to_refactor`            | ms spent refactoring                                                    |
| `number_of_compiles_required` | Simulation stops once every coder has compiled at least this many times |
| `dongle_cooldown`             | ms a dongle stays unusable after being released                        |
| `scheduler`                   | `fifo` or `edf` — arbitration policy when several coders want a dongle  |

All arguments are mandatory and must be strictly positive integers (except
`scheduler`, which must be exactly `fifo` or `edf`). Invalid input is
rejected with an error message and a non-zero exit code.

Example:

```sh
./codexion 5 800 100 100 100 5 100 edf
```

### Reading the output

Every state change is logged as `timestamp_in_ms coder_id action`:

```
0 1 has taken a dongle
2 1 has taken a dongle
2 1 is compiling
202 1 is debugging
402 1 is refactoring
```

## Project structure

| File          | Responsibility                                                        |
|---------------|-------------------------------------------------------------------------|
| `main.c`      | Argument count check, top-level orchestration, final cleanup           |
| `parsing.c`   | Argument validation and parsing                                        |
| `init.c`      | Simulation setup, thread creation, monitor thread                      |
| `dongle.c`    | Dongle acquisition/release, cooldown, waking waiters on stop            |
| `coders.c`    | Coder thread lifecycle (compile / debug / refactor loop), logging      |
| `heap.c`      | Hand-rolled binary min-heap used as the FIFO/EDF priority queue         |
| `utils.c`     | Timing helpers, interruptible sleep, shared-state accessors             |
| `codexion.h`  | Shared structs (`t_sim`, `t_coder`, `t_dongle`, `t_waiter`) and protos  |

## Blocking cases handled

- **Deadlock prevention (Coffman's circular wait).** Instead of every coder
  always acquiring `left` then `right`, each coder acquires whichever of its
  two dongles has the **lower id first**. This imposes a single global
  acquisition order across all coders, which makes a circular wait — the
  classic dining-philosophers deadlock — structurally impossible, regardless
  of scheduling luck.

- **Starvation prevention.** Each dongle keeps its own waiting queue,
  implemented as a binary min-heap ordered by arrival time (`fifo`) or by
  deadline `last_compile_start + time_to_burnout` (`edf`, with arrival time
  as a deterministic tie-breaker). On release, the dongle is handed off
  directly to the highest-priority waiter — it is never reopened to
  first-come-first-served contention — so the scheduler's ordering guarantee
  actually holds under load.

- **Cooldown handling.** After release, a dongle is unusable until
  `dongle_cooldown` ms have passed. This is enforced once ownership of the
  dongle is already secured (whether acquired directly or handed off), and
  the wait happens without holding the dongle's mutex, so it never blocks
  other threads from being scheduled.

- **Precise burnout detection.** A dedicated monitor thread polls every
  coder's `last_compile` timestamp on a short interval and compares it
  against `time_to_burnout`, logging the burnout and stopping the simulation
  well within the 10 ms tolerance required by the subject.

- **Log serialization.** All logging goes through a single function that
  locks a dedicated mutex around the `printf` call, so two messages can
  never interleave on one line.

- **Graceful, non-hanging shutdown.** The shared `stop` flag is itself
  protected by a mutex (read and written the same way everywhere — there is
  no "read-only, no lock needed" shortcut). When the simulation stops, every
  waiter currently blocked on a dongle is explicitly woken up, and every
  sleep (compile/debug/refactor, and the cooldown wait) is interruptible: it
  checks the stop flag on a short interval instead of sleeping blindly for
  the full duration. Without this, a coder mid-`debug` with a large
  `time_to_debug` would keep the whole program alive long after the
  simulation should have ended.

- **Lock-order deadlock between mutexes.** Beyond the resource-level deadlock
  above, a second, more subtle deadlock existed purely between two mutexes:
  a coder could lock a dongle's mutex and then lock the stop mutex (via the
  stop-check), while the monitor could lock the stop mutex and then lock a
  dongle's mutex (to wake waiters) — two threads locking the same two mutexes
  in opposite order, a classic AB-BA deadlock risk. It was found with
  `helgrind` before it ever triggered in practice, and fixed by never holding
  the stop mutex while touching a dongle mutex.

## Thread synchronization mechanisms

- **Per-dongle `pthread_mutex_t`** protects that dongle's `in_use` flag,
  `released_at` timestamp, and its waiting queue. Every read or write of
  these fields goes through this lock.

- **Per-waiter `pthread_cond_t`** (one condition variable per pending
  request, not one shared condition variable per dongle). When a dongle is
  released, the releasing thread pops the correct waiter from the heap and
  signals *that waiter's own* condition variable directly. This gives a
  precise hand-off instead of waking every waiter and letting them race for
  the dongle, which would break the FIFO/EDF ordering guarantee.

- **`coders_mutex`** protects `last_compile` and `compile_count` on every
  `t_coder`. These fields are written by the owning coder thread and read by
  the monitor thread; without this lock, the monitor could read a torn or
  stale value while a coder is mid-write — a genuine data race, not just a
  theoretical one, since nothing in the C standard guarantees a plain memory
  write is visible to another thread without synchronization.

- **`stop_mutex`** protects the single `stop` flag shared by every thread.
  It is locked for every read, not only every write — reading a flag another
  thread can modify concurrently is exactly as much a data race as writing
  it.

- **`log_mutex`** wraps the one `printf` call used for all logging, so
  concurrent log lines from different threads never interleave.

- **Manual binary heap (`heap.c`)** acts as the priority queue backing both
  `fifo` and `edf` scheduling — no standard library priority queue is used.
  Push/pop use the usual sift-up/sift-down operations; the only thing that
  changes between `fifo` and `edf` is the comparator (`has_priority`), which
  compares arrival time or deadline depending on the configured scheduler.

## Resources

- Dijkstra, E. W. — *Hierarchical ordering of sequential processes* (origin
  of the Dining Philosophers problem)
- Liu, C. L. & Layland, J. W. (1973) — *Scheduling Algorithms for
  Multiprogramming in a Hard-Real-Time Environment* (Earliest Deadline First)
- `man 3 pthread_mutex_lock`, `man 3 pthread_cond_wait`,
  `man 3 pthread_cond_timedwait`
- POSIX Threads Programming — Lawrence Livermore National Laboratory
  tutorial (`hpc-tutorials.llnl.gov/posix`)

### AI usage disclosure

An AI assistant was used throughout this project's development in order to explain concurrency concepts.

- **Explaining concurrency concepts** on request (what a lock-order
  deadlock is versus a resource-order deadlock, why a condition variable
  read still needs a mutex, how a heap-based direct hand-off avoids
  starvation) rather than just supplying fixes.
