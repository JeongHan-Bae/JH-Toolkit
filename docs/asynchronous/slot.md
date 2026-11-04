# 🌀 **JH Toolkit — `jh::async::slot` System Reference**

📁 **Header:** `<jh/asynchronous/slot.h>`  
📦 **Namespace:** `jh::async`  
📅 **Version:** 1.4.0 (2025)  
👤 **Author:** JeongHan-Bae `<mastropseudo@gmail.com>`

<div align="right">

[![Back to README](https://img.shields.io/badge/%20Back%20to%20README-blue?style=flat-square)](../../README.md)
[![Back to Module](https://img.shields.io/badge/%20Back%20to%20Module-green?style=flat-square)](overview.md)

</div>

---

## 🏷️ Overview

The **slot–listener–signal** system is a **coroutine-based abstraction for
eager, time-sensitive signal handling**.

It is designed for **engineering scenarios where responsiveness matters more
than completeness**:

* events must be handled *now* or *never*,
* stale or redundant events must be **discarded**,
* implicit buffering is harmful,
* implicit synchronization is forbidden.

This system is **not** a queue, **not** a pub/sub framework, and **not**
a scheduling runtime.

It is a structured way to write **asynchronous state machines** that react
to external signals under **explicit backpressure and timeout expectations**.

---

## 🧭 Design Philosophy

### Asynchronous Means Asynchronous

This system follows one core rule:

> **Do not mix asynchronous logic with implicit synchronization.**

If multiple operations are logically related, they **must be synchronized
explicitly** by the user *before* entering the slot system.

The slot system will never:

* align events from different sources,
* infer barriers or rounds,
* buffer values to "wait for the rest".

If synchronization is required, it must be **external and explicit**.
Once synchronized, the result should be delivered as **one logical event**.

---

## 🚫 Why There Are No Queues

Traditional event systems rely on **physical queues**.
This design deliberately rejects them.

### Why queues are harmful here

In reactive systems, queues cause pathological behavior:

* outdated events are stored,
* repeated erroneous triggers accumulate,
* newer and more relevant events are delayed,
* the system reacts to history instead of reality.

This is especially dangerous when:

* signals represent *intent* rather than data,
* operations may invalidate each other,
* frequency spikes indicate overload or failure.

In such systems, **late execution is worse than no execution**.

---

## ⏱️ Backpressure by Rejection, Not Buffering

Instead of queues, this system uses **timed admission**:

* emitters attempt to deliver an event,
* if delivery cannot happen within a timeout,
* the event is **rejected immediately**.

Semantics:

* emitters effectively queue **themselves**, not their data,
* if delivery succeeds:

  ```
  deliver → resume → consume → clear
  ```
* if delivery fails:

    * nothing is stored,
    * the emitter is informed.

This enforces a strict rule:

> **If events arrive too frequently, they must be dropped.**

Frequent timeouts are a **signal**, not a failure:
they indicate overload, instability, or misdesigned upstream logic.

---

## 🧱 Core Components

### 🔹 `slot`

A `slot` is a **coroutine-based state machine**.

Properties:

* it is the **only execution context** in the system,
* it runs on the thread where `spawn()` is called,
* it has no thread of its own,
* it advances only when explicitly resumed by events.

A slot defines:

* phases,
* state transitions,
* routing,
* filtering,
* fan-out logic.

---

### 🔹 `slot_hub`

A `slot_hub` defines a **synchronization and lifetime domain**.

Rules:

* exactly **one slot per hub**,
* any number of listeners may be created,
* all listeners forward events to the same slot.

The hub enforces:

* timeout policy,
* admission control,
* unified lifetime.

---

### 🔹 `listener<T>`

A `listener<T>` defines **what constitutes one logical input step**.

Important properties:

* a listener holds **at most one in-flight value**,
* it is not a queue,
* it aggregates fan-in from multiple signals,
* it resumes the slot synchronously.

Listeners are **awaited**, not polled.

---

### 🔹 `event_signal<T>`

An `event_signal` is a **push-only injector**.

It:

* does not buffer,
* does not fan out,
* does not route,
* simply attempts to deliver a value to its listener.

All higher-level logic belongs to the slot.

---

## 🧠 Listener Payload Semantics

Different listener payload types express **different intent**.

### `listener<std::tuple<A, B>>`

> **Multiple values are required simultaneously for one logical step.**

* `A` and `B` are produced together.
* Any synchronization is handled externally.
* The slot observes one atomic input.

Use this when inputs are **inherently synchronous**.

---

### `listener<std::tuple<ID, Value>>`

> **Multiple sources, same value type (fan-in).**

* many producers,
* same payload type,
* source identity encoded via `ID`.

Each emission is an independent step.

---

### `listener<std::tuple<ID, std::variant<...>>>`

> **Multiple sources, heterogeneous payloads.**

* different producers,
* different data types,
* unified delivery channel.

Type-safe multiplexing without parallel awaits.

---

## ⚠️ Await Discipline (Hard Rule)

A slot may await **exactly one listener at a time**.

This is **forbidden**:

```cpp
auto a = co_await A;
auto b = co_await B; // ❌
```

Why:

* this assumes loose synchronization between sources,
* it requires one source not to advance faster than the other,
* such guarantees do not exist in asynchronous systems.

If one source advances faster:

* the state machine desynchronizes,
* behavior becomes undefined.

This is **not an implementation limitation**.
It is a semantic rule.

---

## 🔄 Phase-Based Listening

Although a hub may create multiple listeners, they are used **across phases**, not in parallel.

Correct pattern:

```cpp
// Phase 1
for (;;) {
auto v = co_await listener_A;
if (v == STOP) break;
co_yield {};
}

// Phase 2
for (;;) {
auto s = co_await listener_B;
co_yield {};
}
```

Listener switching represents **explicit state transitions**.

---

## 🧩 Example — Eager Signal Handling with `slot`

This example demonstrates a **time-sensitive signal–slot setup** where:

* events are handled immediately or dropped,
* no buffering is involved,
* the `slot` acts as a state machine reacting to signals.

---

### 🔹 Step 1 — Create the Hub

The hub defines the **synchronization domain** and the **timeout policy**.

```cpp
using namespace std::chrono_literals;

jh::async::slot_hub hub(50ms);   // reject events that cannot be delivered within 50ms
```

---

### 🔹 Step 2 — Create a Listener

Listeners are created from the hub and define **one logical input step**.

```cpp
auto li = hub.make_listener<int>();
```

---

### 🔹 Step 3 — Define the Slot Coroutine

The slot is a coroutine that:

* waits for events via `co_await listener`,
* processes exactly one event per iteration,
* yields control explicitly.

```cpp
auto make_slot = [&](jh::async::listener<int>& li) -> jh::async::slot {
    std::puts("[slot] started");

    for (;;) {
        int v = co_await li;          // wait for next event
        std::printf("[slot] value = %d\n", v);

        // state machine logic goes here

        co_yield {};                  // explicit step boundary
    }
};
```

---

### 🔹 Step 4 — Bind Signal to Listener

Signals are **push-only** and must be connected before emitting.

```cpp
jh::async::event_signal<int> sig;
sig.connect(&li);
```

---

### 🔹 Step 5 — Create and Bind the Slot

Each hub may bind **exactly one slot**.

```cpp
jh::async::slot s = make_slot(li);
hub.bind_slot(s);
```

---

### 🔹 Step 6 — Spawn the Slot

Spawning:

* starts the coroutine,
* binds it permanently to the calling thread.

```cpp
s.spawn();
```

From this point on, any successful `emit()` will resume the slot coroutine.

---

### 🔹 Step 7 — Emit Events

```cpp
for (int i = 0; i < 10; ++i) {
    bool accepted = sig.emit(i);

    if (!accepted) {
        std::puts("[emit] dropped (timeout)");
    }

}
```

Semantics:

* if the slot is ready and the hub lock is acquired, the event is delivered,
* if the lock cannot be acquired within the configured timeout, delivery fails.

Successful `emit()` performs delivery and coroutine resumption before it
returns. Tests should assert its returned status and observed event, using an
explicit phase signal when several emitter threads coordinate. Sleeping
between emits is not needed to make delivery correct and should not be used
to force a hoped-for scheduling order.

---

## 🎯 Intended Use Cases

This system is designed for:

* reactive control logic,
* protocol state machines,
* signal-driven workflows,
* UI / input handling,
* systems where **freshness beats completeness**.

It is **not** designed for:

* logging,
* job queues,
* batch processing,
* guaranteed delivery pipelines.

---

## 🧠 Summary

* Slots are **state machines**, not workers.
* Listeners define **one logical input step**.
* Fan-in is explicit; fan-out is user-defined.
* Synchronization is **never implicit**.
* Backpressure is enforced by **dropping events**, not buffering them.
* If something arrives too often, it is **wrong**, and must be discarded.
