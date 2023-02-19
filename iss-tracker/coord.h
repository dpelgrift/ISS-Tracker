#pragma once
#include <Arduino.h>
#include "math_utils.h"
#include "wgs84.h"

#define deg2rad(angle) (angle * PI / 180.0)
#define rad2deg(angle) (angle * 180.0 / PI)
#define wrapToPi(e)    (fmod(e + PI, 2 * PI) - PI)

const bool RADIANS = true;
const bool DEGREES = false;

const bool NED_TO_BODY = true;
const bool BODY_TO_NED = false;


void earthRad(const float& _lat, const bool& angle_unit, double& R_N, double& R_M);

Vec3 lla2ecef(const Vec3& lla, const bool& angle_unit);

Vec3 ecef2lla(const Vec3& ecef, const bool& angle_unit);

Dcm ecef2ned_dcm(const Vec3& lla, const bool& angle_unit);

Vec3 ecef2ned(const Vec3& ecef,
              const Vec3& lla_ref,
              const bool& angle_unit);


Vec3 ned2AzElRng(const Vec3& ned);

Vec3 eci2ecef(Vec3 eci, double angle);

double calcBearing(double lat1, double lon1, double lat2, double lon2);
