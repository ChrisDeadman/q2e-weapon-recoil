#pragma once

#include <cstdint>

namespace recoil_model
{
struct Angles
{
	float pitch = 0.f;
	float yaw = 0.f;
	float roll = 0.f;
};

struct Origin
{
	float x = 0.f;
	float y = 0.f;
	float z = 0.f;
};

struct ShotStateResult
{
	Angles shot_angles = {};
	Angles applied_delta = {};
	Angles next_kick_angles = {};
	Angles release_angles = {};
};

struct InactiveRecoilState
{
	Angles kick_angles = {};
	Angles release_angles = {};
	Origin kick_origin = {};
};

constexpr float kMaxPitch = 89.f;
constexpr float kWeaponRecoilVertical = 1.5f;
constexpr float kWeaponRecoilHorizontal = 0.7f;
constexpr float kWeaponRecoilYawReleaseScale = 1.5f;
constexpr float kWeaponRecoilPitchReleaseScale = 3.0f;

struct WeaponRecoil
{
	float recoil_scale = 1.f;
	float release_scale = 1.f;
	float pitch_sign = -1.f;
};

WeaponRecoil FindWeaponRecoilDefaults(const char *suffix);
Angles BuildWeaponShotDelta(const WeaponRecoil &recoil, int shot_increment, float random_yaw);

Angles BuildWeaponReleaseDelta(const Angles &applied_delta, float recoil_scale, float release_scale, int shot_increment,
	float yaw_release_scale = kWeaponRecoilYawReleaseScale,
	float pitch_release_scale = kWeaponRecoilPitchReleaseScale);

ShotStateResult BuildWeaponShotState(const Angles &view_angles, const Angles &active_kick_angles, const Angles &shot_delta,
	float recoil_scale, float release_scale, int shot_increment);

InactiveRecoilState TransitionToInactive(const Angles &accumulated_kick, const Origin &kick_origin);

InactiveRecoilState DecayInactiveWeaponRecoil(const Angles &release_angles, const Origin &kick_origin,
	float recoil_time, float frame_time);

float ClampKickPitch(float view_pitch, float kick_pitch);
}