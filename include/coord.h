#pragma once
#include <Arduino.h>
#include <math_utils.h>
#include "wgs84.h"

#define deg2rad(angle) (angle * PI / 180.0)
#define rad2deg(angle) (angle * 180.0 / PI)
#define wrapToPi(e)    (fmod(e + PI, 2 * PI) - PI)

const bool RADIANS = true;
const bool DEGREES = false;

const bool NED_TO_BODY = true;
const bool BODY_TO_NED = false;


void earthRad(const float& _lat, const bool& angle_unit, double& R_N, double& R_M)
{
  float lat = _lat;

  if (angle_unit == DEGREES)
    lat = deg2rad(lat);

  R_N = a / sqrt(1 - (ecc_sqrd * pow(sin(lat), 2)));
  R_M = (a * (1 - ecc_sqrd)) / pow(1 - (ecc_sqrd * pow(sin(lat), 2)), 1.5);


}

Vec3 lla2ecef(const Vec3& lla, const bool& angle_unit)
{
  double lat = lla.x;
  double lon = lla.y;
  double alt = lla.z;

  double R_N, R_M;

  earthRad(lat, angle_unit,R_N,R_M);

  if (angle_unit == DEGREES)
  {
    lat = deg2rad(lat);
    lon = deg2rad(lon);
  }

  return Vec3{(R_N + alt) * cos(lat) * cos(lon),
              (R_N + alt)* cos(lat)* sin(lon),
              ((1 - ecc_sqrd) * R_N + alt)* sin(lat)};
}

Vec3 ecef2lla(const Vec3& ecef, const bool& angle_unit)
{
  float x = ecef.x;
  float y = ecef.y;
  float z = ecef.z;

  float x_sqrd = pow(x, 2);
  float y_sqrd = pow(y, 2);

  float lon = atan2(y, x);
  float lat = 400;

  float s = sqrt(x_sqrd + y_sqrd);
  float beta = atan2(z, (1 - f) * s);
  float mu_bar = atan2(z + (((ecc_sqrd * (1 - f)) / (1 - ecc_sqrd)) * a * pow(sin(beta), 3)),
                       s - (ecc_sqrd * a * pow(cos(beta), 3)));
  
  while (abs(lat - mu_bar) > 1e-10)
  {
    lat  = mu_bar;
    beta = atan2((1 - f) * sin(lat),
                 cos(lat));
    mu_bar = atan2(z + (((ecc_sqrd * (1 - f)) / (1 - ecc_sqrd)) * a * pow(sin(beta), 3)),
                   s - (ecc_sqrd * a * pow(cos(beta), 3)));
  }

  lat = mu_bar;

  float N = a / sqrt(1 - (ecc_sqrd * pow(sin(lat), 2)));
  float h = (s * cos(lat)) + ((z + (ecc_sqrd * N * sin(lat))) * sin(lat)) - N;

  if (angle_unit == DEGREES)
  {
    lat = rad2deg(lat);
    lon = rad2deg(lon);
  }

  return Vec3{lat,lon,h};
}

Dcm ecef2ned_dcm(const Vec3& lla, const bool& angle_unit)
{
  float lat = lla.x;
  float lon = lla.y;

  if (angle_unit == DEGREES)
  {
    lat = deg2rad(lat);
    lon = deg2rad(lon);
  }

  Dcm C = {};

  C.x00 = -sin(lat) * cos(lon);
  C.x01 = -sin(lat) * sin(lon);
  C.x02 = cos(lat);

  C.x10 = -sin(lon);
  C.x11 = cos(lon);
  C.x12 = 0;

  C.x20 = -cos(lat) * cos(lon);
  C.x21 = -cos(lat) * sin(lon);
  C.x22 = -sin(lat);

  return C;
}

Vec3 ecef2ned(const Vec3& ecef,
              const Vec3& lla_ref,
              const bool& angle_unit)
{
  Vec3 ecef_ref = lla2ecef(lla_ref, angle_unit);
  Dcm C        = ecef2ned_dcm(lla_ref, angle_unit);

  return C * (ecef - ecef_ref);
}


Vec3 ned2AzElRng(const Vec3& ned) {
    double horiz = sqrt(ned.x*ned.x + ned.y*ned.y);

    double az = fmod(rad2deg(atan2(ned.x, ned.y)) + 360, 360);
    double el = rad2deg(asin(ned.z/horiz));
    double rng = norm(ned);

    return Vec3{az,el,rng}; 
}

Vec3 eci2ecef(Vec3 eci, double angle) {
     Dcm eci2ecef = {
      cos(angle),
      sin(angle),
      0,
      -sin(angle),
      cos(angle),
      0,
      0,0,1
    };
    return eci2ecef * eci;
}