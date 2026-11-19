*This project has been created as part of the 42 curriculum by mburdein.*

# Codexion

## Description

**Codexion** is a concurrent programming project written in C in accordance with the 42 Norm. It models a simulation of multiple coders working in a circular co-working space around a central shared Quantum Compiler.

To compile their quantum code, each coder requires two USB dongles simultaneously: one on their left side and one on their right side. When a coder finishes compiling, they release both dongles and begin debugging. Once debugging is complete, they enter a refactoring phase before immediately attempting to acquire dongles again.

Key constraints and rules:
- There are as many dongles as coders ($N \ge 1$), placed in a circular ring between adjacent coders.
- A coder must never burn out: if a coder fails to start compiling within `time_to_burnout` milliseconds since the beginning of their last compilation (or the start of the simulation), they burn out, and the simulation terminates.
- After being released, each dongle enters a mandatory **cooldown period** (`dongle_cooldown` in milliseconds) during which it cannot be taken by any coder.
- Resource arbitration is handled by a custom binary min-heap implementing one of two scheduling policies:
  - `fifo` (First In, First Out): grants dongles according to request arrival sequence.
  - `edf` (Earliest Deadline First): grants dongles according to the earliest burnout deadline (`last_compile_start + time_to_burnout`).
- The simulation stops when a coder burns out or when all coders have completed at least `number_of_compiles_required` compilations.
- Global variables are strictly forbidden. All state is encapsulated within clean data structures.

## Instructions

### Requirements
- A POSIX-compliant environment (macOS or Linux).
- Standard C compiler (`cc`) with POSIX threads (`-pthread`).
- `make`.

### Compilation
Compile the executable with the provided Makefile:
```sh
make
```

Available Makefile rules:
- `all` (default): Compiles the `codexion` binary.
- `clean`: Removes object files.
- `fclean`: Removes object files and the `codexion` binary.
- `re`: Recompiles the entire project from scratch.

### Execution
Run the program with exactly eight mandatory arguments:
```sh
./codexion number_of_coders time_to_burnout time_to_compile time_to_debug time_to_refactor number_of_compiles_required dongle_cooldown scheduler
```

#### Arguments Breakdown
1. `number_of_coders`: Number of coders and dongles ($N \ge 1$).
2. `time_to_burnout`: Time limit in milliseconds without compiling before burnout ($> 0$).
3. `time_to_compile`: Time in milliseconds required to compile ($\ge 0$).
4. `time_to_debug`: Time in milliseconds spent debugging ($\ge 0$).
5. `time_to_refactor`: Time in milliseconds spent refactoring ($\ge 0$).
6. `number_of_compiles_required`: Number of compilations each coder must complete ($\ge 0$).
7. `dongle_cooldown`: Cooldown duration in milliseconds after a dongle is released ($\ge 0$).
8. `scheduler`: Arbitration policy, strictly `fifo` or `edf`.

#### Example Runs
- 5 coders, 3 compilations, EDF scheduling with 40ms cooldown:
  ```sh
  ./codexion 5 1000 80 120 220 3 40 edf
  ```
- Single coder test ($N=1$ takes 1 dongle and burns out at 800ms):
  ```sh
  ./codexion 1 800 200 200 200 1 0 fifo
  ```
- 4 coders where burnout is guaranteed (tight deadline):
  ```sh
  ./codexion 4 310 200 100 100 10 0 fifo
  ```

### Log Format
All state changes are output in chronological order using the exact specified format:
```text
timestamp_in_ms X has taken a dongle
timestamp_in_ms X is compiling
timestamp_in_ms X is debugging
timestamp_in_ms X is refactoring
timestamp_in_ms X burned out
```
where `timestamp_in_ms` is the millisecond timestamp relative to simulation start and `X` is the 1-based coder number.

## Resources

### References
- IEEE Std 1003.1 (POSIX Threads Specification) - `pthreads(7)`
- Dijkstra, E. W. (1965), *Cooperating Sequential Processes*, Technological University Eindhoven.
- Coffman, E. G., Elphick, M. J., & Shoshani, A. (1971), *System Deadlocks*, Computing Surveys, 3(2), 67–78.
- Liu, C. L., & Layland, J. W. (1973), *Scheduling Algorithms for Multiprogramming in a Hard-Real-Time Environment*, Journal of the ACM, 20(1), 46–61.
- Cormen, T. H., Leiserson, C. E., Rivest, R. L., & Stein, C., *Introduction to Algorithms (Binary Heaps and Priority Queues)*.

### Use of AI
AI (Antigravity by Google DeepMind) was used as an interactive pair programming partner throughout the project lifecycle:
- **Architectural Design**: Formulating the centralized pair-request scheduler with custom binary min-heap to eliminate distributed head-of-line blocking while strictly respecting the per-dongle mutex synchronization requirements.
- **Concurrency Analysis**: Verifying Coffman's conditions, lock ordering, race conditions, and cooldown arbitration under heavy concurrency.
- **Code Optimization & Norm Compliance**: Ensuring all source files strictly follow 42 Norminette requirements (under 25 lines per function, 5 functions per file, tabs, proper typedefs).
- **Test Suite Formulation**: Designing boundary tests (e.g. single coder $N=1$, 0 compiles required, 100/200 coder scalability tests, ASAN and TSAN validation).

## Blocking cases handled

### 1. Deadlock Prevention and Coffman's Conditions
Deadlock requires all four Coffman conditions to hold simultaneously:
1. **Mutual Exclusion**: Dongles are exclusive resources that cannot be shared simultaneously.
2. **Hold and Wait**: A process holds resources while waiting to acquire additional resources.
3. **No Preemption**: Resources cannot be forcibly revoked from holding threads.
4. **Circular Wait**: A closed loop of threads exists where each thread waits for a resource held by the next.

Our implementation breaks Coffman conditions through a combination of techniques:
- **Elimination of Hold and Wait**: Coders do not lock and hold one dongle while waiting an arbitrary amount of time for the second dongle. Instead, coders register a pair request with the central scheduler. Dongles are only granted when *both* required dongles are completely free and out of cooldown.
- **Elimination of Circular Wait via Strict Lock Hierarchy**: When dongle mutexes are locked (either to mark them in use or to release them), they are always acquired in strictly ascending order: `min(left, right)` followed by `max(left, right)`. Because the resource allocation graph is acyclic, circular wait cannot occur.
- **Handling the $N=1$ Special Case**: With 1 coder, there is only 1 dongle on the table. Coder 1 takes the single dongle, logs the action, and enters a condition wait. The monitor thread detects the lack of compilation and terminates the simulation at `time_to_burnout` within tolerance.

### 2. Starvation Prevention and Liveness under EDF
In traditional Dining Philosophers solutions, neighboring threads can alternate taking resources, causing an intermediate thread to starve indefinitely.
In Codexion:
- Under **EDF (Earliest Deadline First)**, priority is determined by `deadline = last_compile_start + time_to_burnout`.
- Whenever a coder compiles, their `last_compile_start` is refreshed, pushing their next deadline far into the future. Coders that have been waiting retain older, earlier deadlines and rise to the top of the min-heap.
- The greedy dispatcher scans the min-heap and grants disjoint, non-conflicting pairs simultaneously. Requests blocked by active neighbors or cooldowns are deferred with their original deadline and sequence intact, preventing head-of-line blocking and guaranteeing maximal concurrency and liveness.

### 3. Dongle Cooldown Handling
After a coder releases their dongles, each dongle records an `available_at` timestamp equal to `current_time + dongle_cooldown`.
- The dispatcher checks `current_time >= dongle->available_at` before granting access.
- Waiting coder threads use `pthread_cond_timedwait` with short, calculated timeouts, ensuring that waiting coders wake up immediately when cooldowns expire without spinning or busy-waiting.
- In addition, the monitor thread invokes `cx_dispatch_requests` during its sub-millisecond monitoring passes to guarantee prompt resource assignment as cooldowns elapse.

### 4. Precise Burnout Detection (< 10 ms)
The subject mandates that burnout logs be displayed within 10 ms of the actual burnout time:
- A dedicated monitor thread polls coder timestamps at high frequency (`usleep(500)` / 0.5 ms).
- For each coder, the monitor computes `elapsed = now - coder.last_compile_start_ms`.
- The moment `elapsed >= time_to_burnout`, the monitor atomically sets `sim->stopped = 1`, logs the burnout event via `cx_log_burnout`, broadcasts condition signals to all threads, and exits.
- Empirical testing with AddressSanitizer and wall-clock verification shows burnout logs emitted within 0–1 ms of the deadline, comfortably exceeding the 10 ms requirement.

### 5. Log Serialization and Clean Shutdown
- All stdout writes are serialized through a dedicated `pthread_mutex_t log_lock`.
- `cx_log` checks `cx_is_stopped(sim)` under the lock, preventing interleaving lines and ensuring no coder emits messages after burnout or simulation termination.

## Thread synchronization mechanisms

The implementation utilizes POSIX synchronization primitives:

1. **`pthread_mutex_t state_lock`**:
   - Central simulation mutex protecting the `t_sim` shared state: the request heap, coder grant statuses, finished counts, and the `stopped` flag.
   - Eliminates data races during request submission, dispatch evaluation, and completion tracking.

2. **`pthread_mutex_t dongles[i].mutex`**:
   - Individual mutex per dongle protecting `is_in_use`, `holder_id`, and `available_at`.
   - Satisfies the subject rule: *"To prevent coders from duplicating dongles, you must protect each dongle's state with a mutex"*.

3. **`pthread_mutex_t log_lock`**:
   - Serializes console output to prevent race conditions or interleaved characters across concurrent threads.

4. **`pthread_cond_t coders[i].cond` & `pthread_cond_t dongles[i].cond`**:
   - Per-coder condition variables allowing threads to suspend execution while awaiting pair allocation.
   - When dongles are granted or the simulation stops, the scheduler or monitor signals the target coder condition variable.
   - `pthread_cond_timedwait` is used to prevent thread hangs and facilitate timely wakeups upon cooldown expiry.

### Race Condition Prevention and Monitor Communication
- **Coordinated Deadline Recording**: In `cx_commit_grant`, `last_compile_start_ms` is recorded atomically under `state_lock` at the exact moment dongles are assigned. This eliminates any race condition between the coder beginning its routine and the monitor evaluating deadlines.
- **Safe Termination Broadcast**: When the monitor detects a burnout or when all coders achieve `number_of_compiles_required`, `sim->stopped = 1` is written under `state_lock`, followed by a condition broadcast (`pthread_cond_broadcast`) across all coder condition variables. This ensures all waiting threads awaken and cleanly terminate without deadlocks or dangling allocations.
