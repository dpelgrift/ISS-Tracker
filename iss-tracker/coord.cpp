/*
  coord.cpp - Coordinate frame conversion functions
    Originally taken from the navduino library: https://github.com/PowerBroker2/navduino
    Reimplemented to use custom Vec3 and Dcm implementations due to Arduino Eigen not compiling for SAMD architecture

    Unless otherwise noted all latitude & longitude inputs & outputs are assumed to be in decimal degrees.
    Altitudes, Earth-Centered-Earth-Fixed (ECEF) Positions, and North-East-Down (NED) Positions are assumed to be
    in meters.
 */
#include "coord.h"

void earthRad(const float& _lat, const bool& angle_unit, double& R_N, double& R_M)
{
    float lat = _lat;

    if (angle_unit == DEGREES)
      lat *= DEG_TO_RAD;

    R_N = a / sqrt(1 - (ecc_sqrd * pow(sin(lat), 2)));
    R_M = (a * (1 - ecc_sqrd)) / pow(1 - (ecc_sqrd * pow(sin(lat), 2)), 1.5);
}

// Convert Lat-Lon-Alt (LLA) position to ECEF position
Vec3 lla2ecef(const Vec3& lla, const bool& angle_unit)
{
    double lat = lla.x;
    double lon = lla.y;
    double alt = lla.z;

    double R_N, R_M;

    earthRad(lat, angle_unit,R_N,R_M);

    if (angle_unit == DEGREES)
    {
        lat *= DEG_TO_RAD;
        lon *= DEG_TO_RAD;
    }

    return Vec3{(R_N + alt) * cos(lat) * cos(lon),
                (R_N + alt)* cos(lat)* sin(lon),
                ((1 - ecc_sqrd) * R_N + alt)* sin(lat)};
}

// Convert ECEF position to LLA position
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
    
    size_t iter = 0;
    while (abs(lat - mu_bar) > 1e-10)
    {
        lat  = mu_bar;
        beta = atan2((1 - f) * sin(lat),
                    cos(lat));
        mu_bar = atan2(z + (((ecc_sqrd * (1 - f)) / (1 - ecc_sqrd)) * a * pow(sin(beta), 3)),
                      s - (ecc_sqrd * a * pow(cos(beta), 3)));
        iter++;
        if (iter > 1e3) break;
    }

    lat = mu_bar;

    float N = a / sqrt(1 - (ecc_sqrd * pow(sin(lat), 2)));
    float h = (s * cos(lat)) + ((z + (ecc_sqrd * N * sin(lat))) * sin(lat)) - N;

    if (angle_unit == DEGREES)
    {
        lat *= RAD_TO_DEG;
        lon *= RAD_TO_DEG;
    }

    return Vec3{lat,lon,h};
}

// Calculate DCM to rotate between ECEF and local NED coordinates for a given LLA position
Dcm ecef2ned_dcm(const Vec3& lla, const bool& angle_unit)
{
    float lat = lla.x;
    float lon = lla.y;

    if (angle_unit == DEGREES)
    {
        lat *= DEG_TO_RAD;
        lon *= DEG_TO_RAD;
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

// Convert position from ECEF frame to local NED frame at a given LLA reference position
Vec3 ecef2ned(const Vec3& ecef,
              const Vec3& lla_ref,
              const bool& angle_unit)
{
    Vec3 ecef_ref = lla2ecef(lla_ref, angle_unit);
    Dcm C        = ecef2ned_dcm(lla_ref, angle_unit);

    return C * (ecef - ecef_ref);
}

// Convert position in NED to equivalent Azimuth, Elevation, & Range
// Azimuth is defined as 0 degrees northward and increases clockwise
// Azimuth & Elevation are returned in units of degrees
Vec3 ned2AzElRng(const Vec3& ned) {
    double horiz = sqrt(ned.x*ned.x + ned.y*ned.y);

    double az = fmod((atan2(ned.y, ned.x) * RAD_TO_DEG) + 360, 360);
    double el = atan2(-ned.z,horiz) * RAD_TO_DEG;
    double rng = norm(ned);

    return Vec3{az,el,rng}; 
}

// Convert Earth-Centered-Inertial (ECI) position to an ECEF position for a given Earth-Rotation-Angle
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

// Compute spherical bearing between two Lat/Lon positions
double calcBearing(double lat1, double lon1, double lat2, double lon2) {
    // Convert latitude and longitude to 
    // spherical coordinates in radians.
    lat1 *= DEG_TO_RAD;
    lat2 *= DEG_TO_RAD;
    lon1 *= DEG_TO_RAD;
    lon2 *= DEG_TO_RAD;

    double cosLat2 = cos(lat2);

    // Compute the bearing angle.
    double bearing = atan2(
        sin(lon2 - lon1) * cosLat2,
        cos(lat1) * sin(lat2) -
        sin(lat1) * cosLat2 *
        cos(lon2 - lon1)
    );

    // Convert the bearing angle from radians to degrees.
    bearing *= RAD_TO_DEG;

    // Make sure the bearing angle is between 0 and 360 degrees.
    bearing = fmod(bearing + 360, 360);

    return bearing;
}
