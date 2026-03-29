Alright—this is absolutely doable, and you’re thinking in the right direction. The key is: **you’re not “toggling recoil back on,” you’re re-implementing it in the rerelease game DLL.**

Here’s a **clear, execution-ready plan** you could hand to a sub-agent (or follow yourself).

---

# 🧠 Objective

Create a minimal gameplay mod for Quake II Enhanced:

> **Name:** `Q2E-Restore-Recoil`
> **Goal:** Restore original Quake II machine gun recoil behavior (view kick / upward drift)
> **Scope:** Single weapon (Machinegun), minimal invasiveness

---

# ⚙️ High-Level Strategy

1. **Get rerelease SDK (64-bit compatible)**
2. **Locate machinegun firing logic**
3. **Reintroduce classic recoil using `kick_angles`**
4. **Tune behavior to match original feel**
5. **Compile as 64-bit DLL**
6. **Package as a standalone mod folder**

---

# 📦 Step 1 — Environment Setup

### Requirements

* Visual Studio (2022 recommended)
* CMake (if SDK uses it)
* Quake II Enhanced installed

### Get SDK

* Nightdive released a **rerelease-compatible SDK** (KEX-based game code)
* Clone/download it

👉 Important:

* You are NOT using old id Tech 2 source directly
* You are modifying the **rerelease game layer**

---

# 🔍 Step 2 — Find Machinegun Code

You’re looking for something equivalent to:

```
weapon_machinegun_fire()
```

or:

```
Machinegun_Fire()
```

Inside:

* `g_weapon.c`
* or similar (may be reorganized in rerelease)

---

# 🧨 Step 3 — Reintroduce Recoil Logic

## What original Quake II did

Each shot applied:

```c
ent->client->kick_angles[0] -= 1.5; // pitch up
ent->client->kick_origin[0] -= 0.5; // slight positional kick
```

Plus:

* Accumulation over sustained fire
* Random jitter for realism

---

## What Enhanced removed

Nightdive essentially:

* **Removed or zeroed out kick_angles**
* Possibly replaced with mild screen shake or nothing

---

## What you add back

### Minimal viable recoil patch:

```c
// Apply vertical recoil (pitch)
ent->client->kick_angles[0] -= 1.5f;

// Optional horizontal jitter
ent->client->kick_angles[1] += crandom() * 0.5f;

// Optional positional kick
ent->client->kick_origin[0] -= 0.5f;
```

---

## ⚠️ Important: Tickrate Adjustment

Original Quake II:

* 10 Hz simulation

Enhanced:

* ~40 Hz

👉 If you blindly copy values, recoil will feel **4x stronger**

### Fix:

Scale it:

```c
float recoil_scale = 0.25f;

ent->client->kick_angles[0] -= 1.5f * recoil_scale;
```

---

# 🎯 Step 4 — Match Original Feel

This is the part most people skip—and why mods feel wrong.

You want:

* Gradual upward climb
* Recoverable aim
* Not pure randomness

### Suggested tuning loop:

1. Start with:

   * `pitch = -0.4f`
2. Test sustained fire
3. Adjust until:

   * ~1 second burst noticeably climbs
   * tap firing is controllable

Optional:

* Add slight randomness:

```c
ent->client->kick_angles[0] -= (0.3f + crandom() * 0.1f);
```

---

# 🏗️ Step 5 — Build the DLL

Compile as:

* **64-bit**
* Output something like:

```
gamex64.dll
```

---

# 📁 Step 6 — Mod Packaging

Directory structure:

```
Quake II/
└── rerelease/
    └── Q2E-Restore-Recoil/
        ├── gamex64.dll
        └── description.txt
```

### description.txt example:

```
Q2E-Restore-Recoil
Restores original Quake II machinegun recoil behavior.
```

---

# ▶️ Step 7 — Run the Mod

Launch with:

```
+set game Q2E-Restore-Recoil
```

or via in-game mod menu (if supported)

---

# 🧪 Step 8 — Validation Checklist

* [ ] Machinegun climbs upward during sustained fire
* [ ] Burst firing is more accurate
* [ ] No recoil on other weapons (unless intended)
* [ ] No crashes in multiplayer or save/load

---

# 🔧 Optional Enhancements

If you want to go beyond basic restoration:

### 1. Add cvar toggle

```c
if (g_restore_recoil->value)
```

### 2. Per-weapon recoil profiles

* Chaingun stronger recoil
* Hyperblaster none

### 3. “Classic mode” pack

Bundle:

* recoil
* original weapon spread
* damage tweaks

---

# 💥 Hard Truth

This is a **~1–2 hour job** for someone comfortable in C once the SDK builds.

The real friction is:

* finding the right function
* compiling the DLL correctly

Not the recoil logic itself—that’s trivial.

---

# If you want next step

I can:

* Point you to the exact function in the rerelease SDK (if you share which repo you're using)
* Or write a near drop-in patch you can paste into the weapon code

That would eliminate almost all guesswork.
