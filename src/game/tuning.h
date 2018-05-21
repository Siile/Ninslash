

#ifndef GAME_TUNING_H
#define GAME_TUNING_H
#undef GAME_TUNING_H // this file will be included several times

// physics tuning
MACRO_TUNING_PARAM(ControlSpeed, control_speed, 1.0f)
MACRO_TUNING_PARAM(JumpPower, jump_power, 12.2f)



MACRO_TUNING_PARAM(GroundControlSpeed, ground_control_speed, 450.0f / TicksPerSecond) // 11 = fast
MACRO_TUNING_PARAM(GroundControlAccel, ground_control_accel, 75.0f / TicksPerSecond)
MACRO_TUNING_PARAM(GroundFriction, ground_friction, 0.5f)
MACRO_TUNING_PARAM(GroundJumpImpulse, ground_jump_impulse, 11.2f)
MACRO_TUNING_PARAM(WallrunImpulse, wall_run_impulse, 9.0f)
MACRO_TUNING_PARAM(AirControlSpeed, air_control_speed, 450.0f / TicksPerSecond) //  250.0f
MACRO_TUNING_PARAM(AirControlAccel, air_control_accel, 50.0f / TicksPerSecond) // 1.5f
MACRO_TUNING_PARAM(AirFriction, air_friction, 0.95f)
MACRO_TUNING_PARAM(Gravity, gravity, 0.7f) // 0.5f

MACRO_TUNING_PARAM(SlideFriction, slide_friction, 0.8f)
MACRO_TUNING_PARAM(SlideSlopeAcceleration, slide_slope_acceleration, 0.3f)
MACRO_TUNING_PARAM(SlopeDeceleration, slope_deceleration, 0.0f)
MACRO_TUNING_PARAM(SlopeAscendingControlSpeed, slope_ascending_control_speed, 10.0f)
MACRO_TUNING_PARAM(SlopeDescendingControlSpeed, slope_descending_control_speed, 10.0f)
MACRO_TUNING_PARAM(SlideControlSpeed, slide_control_speed, 510.0f / TicksPerSecond) // 14
MACRO_TUNING_PARAM(SlideActivationSpeed, slide_activation_speed, 7.0f)
MACRO_TUNING_PARAM(DashPower, dash_power, 21.0f)

MACRO_TUNING_PARAM(VelrampStart, velramp_start, 550)
MACRO_TUNING_PARAM(VelrampRange, velramp_range, 2000)
MACRO_TUNING_PARAM(VelrampCurvature, velramp_curvature, 1.4f)

// weapon tuning
MACRO_TUNING_PARAM(SwordSpeed, sword_speed, 700.0f)

MACRO_TUNING_PARAM(GunCurvature, gun_curvature, 1.25f)
MACRO_TUNING_PARAM(GunSpeed, gun_speed, 3200.0f)
MACRO_TUNING_PARAM(GunLifetime, gun_lifetime, 2.0f)

MACRO_TUNING_PARAM(WalkerCurvature, walker_curvature, 0.3f)
MACRO_TUNING_PARAM(WalkerSpeed, walker_speed, 2200.0f)
MACRO_TUNING_PARAM(WalkerLifetime, walker_lifetime, 2.0f)

MACRO_TUNING_PARAM(StarDroidCurvature, droid_star_curvature, 0.0f)
MACRO_TUNING_PARAM(StarDroidSpeed, droid_star_speed, 20.0f)
MACRO_TUNING_PARAM(StarDroidLifetime, droid_star_lifetime, 2.0f)

MACRO_TUNING_PARAM(ShotgunCurvature, shotgun_curvature, 1.25f)
MACRO_TUNING_PARAM(ShotgunSpeed, shotgun_speed, 2750.0f)
MACRO_TUNING_PARAM(ShotgunSpeeddiff, shotgun_speeddiff, 0.8f)
MACRO_TUNING_PARAM(ShotgunLifetime, shotgun_lifetime, 0.20f)

MACRO_TUNING_PARAM(GrenadeCurvature, grenade_curvature, 7.0f)
MACRO_TUNING_PARAM(GrenadeSpeed, grenade_speed, 1000.0f)
MACRO_TUNING_PARAM(GrenadeLifetime, grenade_lifetime, 2.0f)

MACRO_TUNING_PARAM(ElectricCurvature, electric_curvature, 2.0f)
MACRO_TUNING_PARAM(ElectricSpeed, electric_speed, 1750.0f)
MACRO_TUNING_PARAM(ElectricLifetime, electric_lifetime, 4.0f)

MACRO_TUNING_PARAM(FlamerCurvature, flamer_curvature, 6.0f)
MACRO_TUNING_PARAM(FlamerSpeed, flamer_speed, 1100.0f)
MACRO_TUNING_PARAM(FlamerLifetime, flamer_lifetime, 2.0f)

MACRO_TUNING_PARAM(LaserReach, laser_reach, 800.0f)
MACRO_TUNING_PARAM(LaserBounceDelay, laser_bounce_delay, 150)
MACRO_TUNING_PARAM(LaserBounceNum, laser_bounce_num, 1)
MACRO_TUNING_PARAM(LaserBounceCost, laser_bounce_cost, 0)
MACRO_TUNING_PARAM(LaserDamage, laser_damage, 5)

MACRO_TUNING_PARAM(PlayerCollision, player_collision, 1)
#endif
