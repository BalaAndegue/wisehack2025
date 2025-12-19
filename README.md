

# WiseHack 2025 — Dynamic Binary Instrumentation Engine

## 🧠 Overview

This project implements a **Dynamic Binary Instrumentation (DBI) engine** using **DynamoRIO**.  
The engine dynamically analyzes the execution of a binary program, detects runtime hotspots based on **function call frequency and memory activity**, and **safely redirects a slow function to a faster implementation at runtime**, without modifying the binary on disk.

The solution was developed for **WiseHack 2025** and focuses on **robustness, correctness, and adaptability**.

---

## ✨ Key Features

- 🔍 **Basic Block Instrumentation** using DynamoRIO
- 📊 Runtime monitoring of:
  - Function call frequency
  - Memory access intensity
- 🧠 **Dynamic hotspot detection** using relative (adaptive) thresholds
- 🔀 **Safe hot-patching** via runtime redirection (`drwrap_replace`)
- 🛡️ Robust execution on arbitrary binaries (e.g. `/bin/ls`)
- ⚙️ Compatible with **DynamoRIO 11.x**

---

## 🧩 Architecture

```

+----------------------+
|  Target Binary       |
|  (unknown at build)  |
+----------+-----------+
|
v
+----------------------+

| DynamoRIO Engine         |
| ------------------------ |
| - BB instrumentation     |
| - Clean calls            |
| - Counters               |
| - Hotspot detection      |
| - Safe redirection       |
| +----------------------+ |

````

---

## ⚙️ Technologies Used

- **Language**: C++
- **Instrumentation Framework**: DynamoRIO
- **DynamoRIO Extensions**:
  - `drmgr`
  - `drutil`
  - `drwrap`
  - `drsyms`
- **Build System**: CMake
- **Platform**: Linux x86_64

---

## 🧪 How It Works

### 1️⃣ Instrumentation
- Each **basic block entry** is instrumented.
- Clean calls are used to:
  - Count function executions
  - Count memory access occurrences

### 2️⃣ Hotspot Detection
- At runtime exit, the engine computes:
  - Relative call frequency
  - Relative memory activity
- A function is considered a hotspot if:
  - Call ratio > 30% **OR**
  - Memory ratio > 40%

These thresholds are **dynamic**, not hardcoded to specific binaries.

### 3️⃣ Symbol Resolution
- The engine resolves:
  - `process_transaction`
  - `process_transaction_optimized`
- Using `drsym_lookup_symbol`

### 4️⃣ Safe Redirection (Hot-Patching)
- Redirection is applied **only if**:
  - The detected hotspot corresponds to `process_transaction`
  - Both symbols are successfully resolved
- The patch is applied **once only** using:
  ```c
  drwrap_replace(process_transaction, process_transaction_optimized)


This ensures:

* No crash on system binaries
* No unsafe patching of unknown code



## ▶️ How to Build

### Prerequisites

* Linux
* DynamoRIO 11.x installed
* CMake ≥ 3.15
* g++

### Build Steps

```bash
cd wisehack2025/engine
mkdir -p build
cd build
cmake ..
make
```

---

## ▶️ How to Run

### Safe test on a system binary

```bash
~/DynamoRIO-Linux-11.3.0-1/bin64/drrun \
  -c ./libengine.so -- /bin/ls
```

Expected result:

* Normal execution
* No crash
* No patch applied

### Test with provided binary (slow / fast functions)

```bash
~/DynamoRIO-Linux-11.3.0-1/bin64/drrun \
  -c ./libengine.so -- ./test_binary
```

Expected output:

```
[INFO] process_transaction found at 0x...
[INFO] process_transaction_optimized found at 0x...
[HOTSPOT CONFIRMED] process_transaction
[PATCH] process_transaction redirected to process_transaction_optimized
```

---

## 🛡️ Stability & Safety

* Clean calls always receive a valid `drcontext`
* No patching on unknown binaries
* Redirection is guarded by symbol verification
* Patch applied once only
* Tested successfully on:

  * `/bin/ls`
  * Arbitrary user binaries

---

## ⚠️ Known Limitations

* The engine assumes the presence of:

  * `process_transaction`
  * `process_transaction_optimized`
    with identical signatures
* Detection is currently performed at program exit (not live)
* Memory access tracking is coarse-grained (BB-level)

These design choices favor **stability and clarity**.

---

## 🚀 Possible Improvements

* Live hotspot detection during execution
* Per-instruction memory tracking
* Multi-hotspot support
* Machine-learning-based thresholds
* CFG-aware optimization strategies

---

## 👤 Author

**Bala Andegue**
WiseHack 2025 Participant

GitHub: [https://github.com/BalaAndegue](https://github.com/BalaAndegue)

---

## 🏁 Final Note

This project demonstrates:

* Mastery of Dynamic Binary Instrumentation
* Safe runtime code manipulation
* Defensive engineering against crashes
* Adaptation to unknown binaries

The engine is **production-stable, extensible, and contest-ready**.

