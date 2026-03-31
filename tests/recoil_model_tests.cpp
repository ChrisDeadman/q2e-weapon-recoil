#include "recoil_model.h"

#include "gtest/gtest.h"

#include <algorithm>
#include <cmath>

namespace
{
float SignedAngle(float angle)
{
	angle = std::fmod(angle, 360.f);
	if (angle < 0.f)
		angle += 360.f;
	if (angle > 180.f)
		angle -= 360.f;
	return angle;
}

void ExpectAnglesNear(const recoil_model::Angles &actual, const recoil_model::Angles &expected, float epsilon = 0.0001f)
{
	EXPECT_NEAR(actual.pitch, expected.pitch, epsilon);
	EXPECT_NEAR(actual.yaw, expected.yaw, epsilon);
	EXPECT_NEAR(actual.roll, expected.roll, epsilon);
}

constexpr float kLimit = recoil_model::kMaxPitch;

float DisplayPitch(float view_pitch, float kick_pitch)
{
	return std::clamp(view_pitch + kick_pitch, -kLimit, kLimit);
}
}

TEST(RecoilModel, ShotUsesPreRecoilAim)
{
	auto shot = recoil_model::BuildWeaponShotState({ 0.f, 0.f, 0.f }, { 20.f, 5.f, 0.f }, { 1.5f, 0.4f, 0.f }, 1.0f, 1.0f, 1);

	ExpectAnglesNear(shot.shot_angles, { 20.f, 5.f, 0.f });
	EXPECT_NEAR(shot.applied_delta.pitch, 1.5f, 0.0001f);
	EXPECT_NEAR(shot.applied_delta.yaw, 0.4f, 0.0001f);
	EXPECT_NEAR(shot.next_kick_angles.pitch, 21.5f, 0.0001f);
	EXPECT_NEAR(shot.next_kick_angles.yaw, 5.4f, 0.0001f);
	EXPECT_NEAR(shot.release_angles.pitch, 4.5f, 0.0001f);
	EXPECT_NEAR(shot.release_angles.yaw, 0.6f, 0.0001f);
}

TEST(RecoilModel, PitchDeltaClampsAtViewLimit)
{
	auto shot = recoil_model::BuildWeaponShotState({ 88.5f, 0.f, 0.f }, {}, { 3.0f, 0.f, 0.f }, 1.0f, 1.0f, 1);

	EXPECT_NEAR(shot.shot_angles.pitch, 88.5f, 0.0001f);
	EXPECT_NEAR(shot.applied_delta.pitch, 0.5f, 0.0001f);
	EXPECT_NEAR(shot.next_kick_angles.pitch, 0.5f, 0.0001f);
	EXPECT_NEAR(shot.release_angles.pitch, 9.0f, 0.0001f);
}

TEST(RecoilModel, SustainedFireClampsAtPitchLimit)
{
	recoil_model::Angles kick = {};
	float previous_pitch = 0.f;

	for (int i = 0; i < 80; ++i)
	{
		auto shot = recoil_model::BuildWeaponShotState({}, kick, { 2.25f, 0.35f, 0.f }, 1.0f, 1.0f, 1);
		EXPECT_GE(shot.next_kick_angles.pitch, previous_pitch);
		EXPECT_LE(shot.next_kick_angles.pitch, kLimit);
		EXPECT_NEAR(shot.shot_angles.pitch, DisplayPitch(0.f, kick.pitch), 0.0001f);
		previous_pitch = shot.next_kick_angles.pitch;
		kick = shot.next_kick_angles;
	}

	EXPECT_NEAR(kick.pitch, kLimit, 0.0001f);
}

TEST(RecoilModel, SustainedFireClampsAtNegativePitchLimit)
{
	recoil_model::Angles kick = {};
	float previous_pitch = 0.f;

	for (int i = 0; i < 80; ++i)
	{
		auto shot = recoil_model::BuildWeaponShotState({}, kick, { -2.25f, -0.35f, 0.f }, 1.0f, 1.0f, 1);
		EXPECT_LE(shot.next_kick_angles.pitch, previous_pitch);
		EXPECT_GE(shot.next_kick_angles.pitch, -kLimit);
		EXPECT_NEAR(SignedAngle(shot.shot_angles.pitch), DisplayPitch(0.f, kick.pitch), 0.0001f);
		previous_pitch = shot.next_kick_angles.pitch;
		kick = shot.next_kick_angles;
	}

	EXPECT_NEAR(kick.pitch, -kLimit, 0.0001f);
}

TEST(RecoilModel, PositiveAndNegativeRecoilStayMonotonic)
{
	recoil_model::Angles positive_kick = {};
	recoil_model::Angles negative_kick = {};

	for (int i = 0; i < 40; ++i)
	{
		auto positive = recoil_model::BuildWeaponShotState({}, positive_kick, { 1.5f, 0.4f, 0.f }, 1.0f, 1.0f, 1);
		auto negative = recoil_model::BuildWeaponShotState({}, negative_kick, { -1.5f, -0.4f, 0.f }, 1.0f, 1.0f, 1);

		EXPECT_GE(positive.next_kick_angles.pitch, positive_kick.pitch);
		EXPECT_LE(negative.next_kick_angles.pitch, negative_kick.pitch);
		positive_kick = positive.next_kick_angles;
		negative_kick = negative.next_kick_angles;
	}
}

TEST(RecoilModel, DisplayPitchNeverWraps)
{
	recoil_model::Angles kick = {};
	for (int i = 0; i < 120; ++i)
	{
		auto shot = recoil_model::BuildWeaponShotState({ (i % 2) ? 15.f : -15.f, 0.f, 0.f }, kick, { 2.25f, 0.0f, 0.f }, 1.0f, 1.0f, 1);
		kick = shot.next_kick_angles;
		float display_pitch = DisplayPitch((i % 2) ? 15.f : -15.f, kick.pitch);
		EXPECT_LE(display_pitch, kLimit);
		EXPECT_GE(display_pitch, -kLimit);
	}
}

TEST(RecoilModel, TransitionPreservesVisualContinuity)
{
	recoil_model::Angles kick = {};
	for (int i = 0; i < 15; ++i)
	{
		auto shot = recoil_model::BuildWeaponShotState({}, kick, { 1.5f, 0.4f, 0.f }, 1.0f, 1.0f, 1);
		kick = shot.next_kick_angles;
	}

	auto transition = recoil_model::TransitionToInactive(kick, { 0.5f, 0.3f, 0.1f });
	ExpectAnglesNear(transition.kick_angles, {});
	ExpectAnglesNear(transition.release_angles, kick);
	EXPECT_NEAR(transition.kick_origin.x, 0.5f, 0.0001f);
	EXPECT_NEAR(transition.kick_origin.y, 0.3f, 0.0001f);
	EXPECT_NEAR(transition.kick_origin.z, 0.1f, 0.0001f);
}

TEST(RecoilModel, InactiveDecayPreservesReleaseThenFades)
{
	auto decayed = recoil_model::DecayInactiveWeaponRecoil({ 10.f, 4.f, 0.f }, { 1.f, 2.f, 3.f }, 0.1f, 0.05f);
	ExpectAnglesNear(decayed.kick_angles, {});
	ExpectAnglesNear(decayed.release_angles, { 5.f, 2.f, 0.f });
	EXPECT_NEAR(decayed.kick_origin.x, 0.5f, 0.0001f);
	EXPECT_NEAR(decayed.kick_origin.y, 1.0f, 0.0001f);
	EXPECT_NEAR(decayed.kick_origin.z, 1.5f, 0.0001f);
}

TEST(RecoilModel, InactiveDecayWithZeroRecoilTimeClearsImmediately)
{
	auto decayed = recoil_model::DecayInactiveWeaponRecoil({ 10.f, 4.f, 0.f }, { 1.f, 2.f, 3.f }, 0.f, 0.01f);
	ExpectAnglesNear(decayed.kick_angles, {});
	ExpectAnglesNear(decayed.release_angles, {});
	EXPECT_NEAR(decayed.kick_origin.x, 0.f, 0.0001f);
	EXPECT_NEAR(decayed.kick_origin.y, 0.f, 0.0001f);
	EXPECT_NEAR(decayed.kick_origin.z, 0.f, 0.0001f);
}

TEST(RecoilModel, FullLifecycle_FireTransitionDecay)
{
	recoil_model::Angles kick = {};
	for (int i = 0; i < 30; ++i)
	{
		auto shot = recoil_model::BuildWeaponShotState({}, kick, { 1.5f, 0.4f, 0.f }, 1.0f, 1.0f, 1);
		kick = shot.next_kick_angles;
	}

	auto transition = recoil_model::TransitionToInactive(kick, {});
	recoil_model::Angles release = transition.release_angles;
	recoil_model::Origin origin = transition.kick_origin;

	EXPECT_NEAR(release.pitch, kick.pitch, 0.0001f);
	for (int i = 0; i < 200; ++i)
	{
		auto decayed = recoil_model::DecayInactiveWeaponRecoil(release, origin, 0.3f, 0.01f);
		release = decayed.release_angles;
		origin = decayed.kick_origin;
	}

	EXPECT_NEAR(release.pitch, 0.f, 0.1f);
}

TEST(RecoilModel, KickDoesNotSpikeWhenCombinedPitchCrosses180)
{
	// Simulate: player looking max down (89°), weapon pushes up (negative delta).
	// Kick accumulates to -178. Then player moves mouse to look slightly up (-3°).
	// The combined pitch -3 + (-178) = -181 crosses the -180 boundary.
	// BUG: SignedAngle wraps the combined to +179, producing a massive applied_delta
	// that spikes kick to -268 and snaps the display to 89° (max down).

	recoil_model::Angles kick = {};
	constexpr float view_down = kLimit;
	constexpr float delta_pitch = -2.25f;

	// Phase 1: accumulate kick while looking max down
	for (int i = 0; i < 120; ++i)
	{
		auto shot = recoil_model::BuildWeaponShotState({ view_down, 0.f, 0.f }, kick, { delta_pitch, 0.f, 0.f }, 1.0f, 1.0f, 1);
		kick = shot.next_kick_angles;
	}

	// Kick should have accumulated to about -178
	EXPECT_LT(kick.pitch, -170.f);

	// Phase 2: player moves mouse to look slightly up, fires one more shot
	constexpr float view_up = 357.f; // -3° signed
	auto shot = recoil_model::BuildWeaponShotState({ view_up, 0.f, 0.f }, kick, { delta_pitch, 0.f, 0.f }, 1.0f, 1.0f, 1);

	// The kick must NOT spike; it should stay close to where it was or decrease
	EXPECT_GT(shot.next_kick_angles.pitch, -200.f)
		<< "Kick spiked from " << kick.pitch << " to " << shot.next_kick_angles.pitch
		<< " — the 180-degree wrap bug corrupted the recoil state";

	// The display pitch must remain within the valid view range
	float display = std::clamp(SignedAngle(view_up) + shot.next_kick_angles.pitch, -kLimit, kLimit);
	EXPECT_GE(display, -kLimit);
	EXPECT_LE(display, kLimit);

	// Critically, the display must NOT snap to the opposite extreme.
	// Before the mouse move, display ≈ -89 (looking up). After a small mouse move
	// down from 89° to -3°, the display should still be near -89, not +89.
	EXPECT_LT(display, 0.f)
		<< "Display snapped to " << display << "° (looking down) — expected near -89° (looking up)";
}

TEST(RecoilModel, KickDoesNotSpikeWhenCombinedPitchCrossesPositive180)
{
	// Mirror scenario: player looking max up (-89° = 271°), weapon pushes down (positive delta).
	// Kick accumulates to +178. Then player moves mouse to look slightly down (3°).
	// Combined 3 + 178 = 181 crosses the +180 boundary.

	recoil_model::Angles kick = {};
	constexpr float view_up = 271.f; // -89° signed
	constexpr float delta_pitch = 2.25f;

	for (int i = 0; i < 120; ++i)
	{
		auto shot = recoil_model::BuildWeaponShotState({ view_up, 0.f, 0.f }, kick, { delta_pitch, 0.f, 0.f }, 1.0f, 1.0f, 1);
		kick = shot.next_kick_angles;
	}

	EXPECT_GT(kick.pitch, 170.f);

	constexpr float view_down = 3.f;
	auto shot = recoil_model::BuildWeaponShotState({ view_down, 0.f, 0.f }, kick, { delta_pitch, 0.f, 0.f }, 1.0f, 1.0f, 1);

	EXPECT_LT(shot.next_kick_angles.pitch, 200.f)
		<< "Kick spiked from " << kick.pitch << " to " << shot.next_kick_angles.pitch;

	float display = std::clamp(SignedAngle(view_down) + shot.next_kick_angles.pitch, -kLimit, kLimit);
	EXPECT_GT(display, 0.f)
		<< "Display snapped to " << display << "° (looking up) — expected near +89° (looking down)";
}

TEST(RecoilModel, ClampKickPitchPreventsRendererWrap)
{
	// Chaingun pushes v_angle to 89° (max down), then weapon_release_angles
	// adds ~4.5° more as kick_angles. Total = 93.5° which exceeds the Q2
	// renderer's ±89° limit and flips the view.
	EXPECT_NEAR(recoil_model::ClampKickPitch(kLimit, 4.5f), 0.f, 0.0001f);
	EXPECT_NEAR(recoil_model::ClampKickPitch(kLimit, 0.f), 0.f, 0.0001f);
	EXPECT_NEAR(recoil_model::ClampKickPitch(85.f, 4.5f), 4.f, 0.0001f);

	// Mirror: looking max up (-89° = 271° unsigned)
	EXPECT_NEAR(recoil_model::ClampKickPitch(271.f, -4.5f), 0.f, 0.0001f);
	EXPECT_NEAR(recoil_model::ClampKickPitch(275.f, -4.5f), -4.f, 0.0001f);

	// Normal case: no clamping needed
	EXPECT_NEAR(recoil_model::ClampKickPitch(0.f, 10.f), 10.f, 0.0001f);
	EXPECT_NEAR(recoil_model::ClampKickPitch(0.f, -10.f), -10.f, 0.0001f);

	// Edge case: exactly at limit
	EXPECT_NEAR(recoil_model::ClampKickPitch(80.f, 9.f), 9.f, 0.0001f);
	EXPECT_NEAR(recoil_model::ClampKickPitch(80.f, 10.f), 9.f, 0.0001f);
}

TEST(RecoilModel, ChaingunSustainedFireDoesNotExceedViewLimit)
{
	// Simulate chaingun: pitch_sign=1.0, recoil_scale=0.5, release_scale=1.0
	// WEAPON_RECOIL_VERTICAL=1.5
	// Each shot: shot_delta.pitch = 1.0 * 1.5 * 0.5 * 1 = 0.75
	constexpr float shot_delta_pitch = 0.75f;
	constexpr float recoil_scale = 0.5f;
	constexpr float release_scale = 1.0f;

	float view_pitch = 0.f;

	for (int i = 0; i < 200; ++i)
	{
		// Model: v_angle gets pushed by the shot delta (clamped to ±89°)
		float new_view = std::clamp(view_pitch + shot_delta_pitch, -kLimit, kLimit);
		view_pitch = new_view;

		// Build the release delta (same as what the game stores in weapon_release_angles)
		auto release = recoil_model::BuildWeaponReleaseDelta(
			{ shot_delta_pitch, 0.f, 0.f }, recoil_scale, release_scale, 1);

		// Clamp kick so total doesn't exceed ±89°
		float clamped_kick = recoil_model::ClampKickPitch(view_pitch, release.pitch);
		float total = view_pitch + clamped_kick;

		EXPECT_LE(total, kLimit)
			<< "Total pitch " << total << "° exceeds +" << kLimit << "° at shot " << i
			<< " (view=" << view_pitch << " kick=" << clamped_kick << ")";
		EXPECT_GE(total, -kLimit);
	}
}

// Simulate Q2 PM_ClampAngles pitch guard (unmodified engine behavior).
float PmClampPitch(float cmd_pitch, float delta_pitch)
{
	float v = cmd_pitch + delta_pitch;
	if (v > 89.f && v < 180.f)
		v = 89.f;
	else if (v < 271.f && v >= 180.f)
		v = 271.f;
	return v;
}

struct SustainedFireConfig
{
	const char *name;
	float pitch_delta;
	float recoil_scale;
	float release_scale;
	int shots;
};

class SustainedFireFightingRecoil : public ::testing::TestWithParam<
	std::tuple<SustainedFireConfig, float /*start_pitch*/, float /*fight_strength*/>>
{
};

TEST_P(SustainedFireFightingRecoil, NeverSnaps)
{
	auto [config, start_pitch, fight_strength] = GetParam();
	// Simulate the full game flow:
	// 1. Each shot pushes v_angle (P_ApplyBulletWeaponAimDelta)
	// 2. delta_angles accumulates as raw signed values
	// 3. PM_ClampAngles (with anglemod on the sum) runs next frame
	// 4. Player fights with mouse by counteracting (fight_strength fraction)

	float v_angle = start_pitch;
	float delta_angles = 0.f;
	float prev_display = SignedAngle(v_angle);

	for (int i = 0; i < config.shots; ++i)
	{
		// --- Shot fires (P_ApplyBulletWeaponAimDelta) ---
		float old_pitch = SignedAngle(v_angle);
		float new_pitch = std::clamp(old_pitch + config.pitch_delta, -kLimit, kLimit);
		float applied = new_pitch - old_pitch;
		delta_angles += applied;

		v_angle = std::fmod(new_pitch, 360.f);
		if (v_angle < 0.f)
			v_angle += 360.f;

		// --- Build release for display ---
		auto release = recoil_model::BuildWeaponReleaseDelta(
			{ config.pitch_delta, 0.f, 0.f }, config.recoil_scale, config.release_scale, 1);
		float kick = recoil_model::ClampKickPitch(v_angle, release.pitch);
		float display = SignedAngle(v_angle) + kick;
		EXPECT_LE(display, kLimit);
		EXPECT_GE(display, -kLimit);

		// Check for discontinuities (no jumps > 20° between frames)
		float jump = std::abs(display - prev_display);
		EXPECT_LT(jump, 20.f)
			<< config.name << " shot " << i << ": display jumped " << jump
			<< "° (from " << prev_display << " to " << display << ")";
		prev_display = display;

		// --- Player fights recoil (next frame's Pmove) ---
		float fight = -applied * fight_strength;
		float desired = SignedAngle(v_angle) + fight;
		v_angle = std::fmod(desired, 360.f);
		if (v_angle < 0.f)
			v_angle += 360.f;

		// PM_ClampAngles (unmodified engine)
		float cmd = v_angle - delta_angles;
		v_angle = PmClampPitch(cmd, delta_angles);
	}
}

static const SustainedFireConfig kWeaponConfigs[] = {
	{ "Hyperblaster", -0.825f, 0.55f, 0.55f, 400 },
	{ "Machinegun",   -1.5f,   1.0f,  1.0f,  300 },
	{ "Chaingun",      0.75f,  0.5f,  1.0f,  400 },
	{ "Blaster",      -0.9f,   0.6f,  0.6f,  200 },
};

static const float kStartPitches[] = { 0.f, 30.f, 350.f, 60.f, 300.f, 80.f, 280.f };
static const float kFightStrengths[] = { 0.0f, 0.5f, 0.8f, 1.0f, 1.2f };

INSTANTIATE_TEST_SUITE_P(
	WeaponRecoilMatrix,
	SustainedFireFightingRecoil,
	::testing::Combine(
		::testing::ValuesIn(kWeaponConfigs),
		::testing::ValuesIn(kStartPitches),
		::testing::ValuesIn(kFightStrengths)
	)
);

int main(int argc, char **argv)
{
	::testing::InitGoogleTest(&argc, argv);
	return RUN_ALL_TESTS();
}