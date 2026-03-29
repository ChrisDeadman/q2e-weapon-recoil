#pragma once

// Recoil tuning constants for Quake II Enhanced machinegun.
//
// Original Q2 ran at 10 Hz; Enhanced runs at ~40 Hz.
// All values are pre-scaled by 0.25 to compensate.

static constexpr float RECOIL_SCALE      = 0.25f;
static constexpr float RECOIL_PITCH      = -1.5f * RECOIL_SCALE;  // upward kick per shot
static constexpr float RECOIL_YAW_JITTER =  0.5f * RECOIL_SCALE;  // random horizontal spread
static constexpr float RECOIL_KICK       = -0.5f * RECOIL_SCALE;  // positional kick

// Usage — insert after the fire_bullet() / fire_lead() call in
// Machinegun_Fire (p_weapon.cpp):
//
//   #include "recoil.h"
//   ...
//   ent->client->kick_angles[PITCH] += RECOIL_PITCH;
//   ent->client->kick_angles[YAW]   += crandom() * RECOIL_YAW_JITTER;
//   ent->client->kick_origin[0]     += RECOIL_KICK;
