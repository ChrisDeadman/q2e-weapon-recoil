#include "recoil_model.h"

#include <cmath>
#include <cstring>

namespace recoil_model
{
namespace
{
float ClampFloat(float value, float low, float high)
{
	if (value < low)
		return low;
	if (value > high)
		return high;
	return value;
}

float AngleMod(float angle)
{
	angle = std::fmod(angle, 360.f);
	if (angle < 0.f)
		angle += 360.f;
	return angle;
}

float SignedAngle(float angle)
{
	angle = AngleMod(angle);
	if (angle > 180.f)
		angle -= 360.f;
	return angle;
}

float LengthSquared(const Angles &angles)
{
	return angles.pitch * angles.pitch + angles.yaw * angles.yaw + angles.roll * angles.roll;
}

float LengthSquared(const Origin &origin)
{
	return origin.x * origin.x + origin.y * origin.y + origin.z * origin.z;
}
}

Angles BuildWeaponReleaseDelta(const Angles &applied_delta, float recoil_scale, float release_scale, int shot_increment,
	float yaw_release_scale, float pitch_release_scale)
{
	Angles release_delta = applied_delta;
	if (recoil_scale > 0.f)
		release_delta.yaw *= (yaw_release_scale * release_scale) / recoil_scale;
	if (recoil_scale > 0.f && shot_increment > 0)
		release_delta.pitch *= (pitch_release_scale * release_scale) / (recoil_scale * shot_increment);
	return release_delta;
}

ShotStateResult BuildWeaponShotState(const Angles &view_angles, const Angles &active_kick_angles, const Angles &shot_delta,
	float recoil_scale, float release_scale, int shot_increment)
{
	ShotStateResult result = {};

	// Fire from the current aim before applying this shot's recoil.
	result.shot_angles = view_angles;
	float combined_pitch = ClampFloat(SignedAngle(result.shot_angles.pitch) + active_kick_angles.pitch, -kMaxPitch, kMaxPitch);
	result.shot_angles.pitch = AngleMod(combined_pitch);
	result.shot_angles.yaw = AngleMod(result.shot_angles.yaw + active_kick_angles.yaw);
	result.shot_angles.roll = AngleMod(result.shot_angles.roll + active_kick_angles.roll);

	float old_pitch = combined_pitch;
	float new_pitch = ClampFloat(old_pitch + shot_delta.pitch, -kMaxPitch, kMaxPitch);

	result.applied_delta.pitch = new_pitch - old_pitch;
	result.applied_delta.yaw = shot_delta.yaw;
	result.applied_delta.roll = shot_delta.roll;

	result.next_kick_angles = {
		active_kick_angles.pitch + result.applied_delta.pitch,
		active_kick_angles.yaw + result.applied_delta.yaw,
		active_kick_angles.roll + result.applied_delta.roll,
	};

	Angles release_delta = BuildWeaponReleaseDelta(shot_delta, recoil_scale, release_scale, shot_increment);
	result.release_angles = {
		release_delta.pitch,
		release_delta.yaw,
		release_delta.roll,
	};

	return result;
}

InactiveRecoilState TransitionToInactive(const Angles &accumulated_kick, const Origin &kick_origin)
{
	return { {}, accumulated_kick, kick_origin };
}

float ClampKickPitch(float view_pitch, float kick_pitch)
{
	float signed_view = SignedAngle(view_pitch);
	float total = signed_view + kick_pitch;
	if (total > kMaxPitch)
		return kMaxPitch - signed_view;
	if (total < -kMaxPitch)
		return -kMaxPitch - signed_view;
	return kick_pitch;
}

InactiveRecoilState DecayInactiveWeaponRecoil(const Angles &release_angles, const Origin &kick_origin,
	float recoil_time, float frame_time)
{
	InactiveRecoilState state = {};
	float duration = recoil_time > 0.01f ? recoil_time : 0.01f;
	float blend = ClampFloat(frame_time / duration, 0.f, 1.f);
	float keep = 1.f - blend;

	state.release_angles = {
		release_angles.pitch * keep,
		release_angles.yaw * keep,
		release_angles.roll * keep,
	};
	state.kick_origin = {
		kick_origin.x * keep,
		kick_origin.y * keep,
		kick_origin.z * keep,
	};

	if (LengthSquared(state.release_angles) < 0.0001f)
		state.release_angles = {};
	if (LengthSquared(state.kick_origin) < 0.0001f)
		state.kick_origin = {};

	return state;
}

namespace
{
struct NameDefault
{
	const char *pattern;
	WeaponRecoil recoil;
};

// Canonical source of per-weapon recoil defaults, matched by substring.
// Order matters: more specific patterns must precede shorter ones
// (e.g. "supershotgun" before "shotgun", "chaingun" before "chain").
constexpr NameDefault kNameDefaults[] = {
	{ "grapple",       { 0.0f,  0.0f,  -1.f } },
	{ "chainfist",     { 0.5f,  0.75f,  1.f } },
	{ "chainsaw",      { 0.5f,  0.75f,  1.f } },
	{ "chaingun",      { 0.75f, 1.5f,   1.f } },
	{ "machinegun",    { 1.5f,  1.5f,  -1.f } },
	{ "supershotgun",  { 3.5f,  3.5f,  -1.f } },
	{ "shotgun",       { 2.75f, 2.75f, -1.f } },
	{ "bfg",           { 8.0f,  8.0f,  -1.f } },
	{ "rail",          { 3.0f,  3.0f,  -1.f } },
	{ "rocket",        { 3.75f, 3.75f, -1.f } },
	{ "grenade",       { 3.0f,  3.0f,  -1.f } },
	{ "prox",          { 3.0f,  3.0f,  -1.f } },
	{ "phalanx",       { 3.25f, 3.25f, -1.f } },
	{ "disruptor",     { 3.0f,  3.0f,  -1.f } },
	{ "tracker",       { 3.0f,  3.0f,  -1.f } },
	{ "disintegrator", { 3.0f,  3.0f,  -1.f } },
	{ "ionripper",     { 1.5f,  1.5f,  -1.f } },
	{ "boomer",        { 1.5f,  1.5f,  -1.f } },
	{ "ripper",        { 1.5f,  1.5f,  -1.f } },
	{ "plasmabeam",    { 0.25f, 0.25f, -1.f } },
	{ "heatbeam",      { 0.25f, 0.25f, -1.f } },
	{ "beam",          { 0.25f, 0.25f, -1.f } },
	{ "hyperblaster",  { 0.5f,  0.5f,  -1.f } },
	{ "blaster",       { 0.5f,  0.5f,  -1.f } },
	{ "etf",           { 1.25f, 1.25f, -1.f } },
	{ "flechette",     { 1.25f, 1.25f, -1.f } },
	{ "tesla",         { 0.75f, 0.75f, -1.f } },
	{ "trap",          { 0.75f, 0.75f, -1.f } },
};
}

WeaponRecoil FindWeaponRecoilDefaults(const char *suffix)
{
	if (!suffix)
		return {};
	for (const auto &entry : kNameDefaults)
	{
		if (std::strstr(suffix, entry.pattern))
			return entry.recoil;
	}
	return {};
}

Angles BuildWeaponShotDelta(const WeaponRecoil &recoil, int shot_increment, float random_yaw)
{
	int increment = shot_increment > 0 ? shot_increment : 1;
	return {
		recoil.pitch_sign * kWeaponRecoilVertical * recoil.recoil_scale * increment,
		random_yaw * kWeaponRecoilHorizontal * recoil.recoil_scale,
		0.f,
	};
}
}