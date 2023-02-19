/*
  orbit_utils.h - Utilities to handle orbit initialization and propagation, and time conversions
 */
#pragma once
#include <Arduino.h>
#include <time.h>
#include "coord.h"

#define EARTH_ROT_RATE 7.2921159e-5
#define MU_EARTH 3.986004418e14
#define J2000 2451545.5
#define J1900 (J2000 - 36525. - 1.)
#define SECONDS_PER_DAY 86400.
#define SECONDS_PER_DAY_SQ (SECONDS_PER_DAY*SECONDS_PER_DAY)
#define J2U 2440587.5

double getJulianFromUnix( long unixSecs );
long getUnixSecFromJulian(double julian);
double getEraFromJulian(double julian);
double eccAnomalyFromMean(double M0, double ecc);
double trueAnomalyFromEcc(double E, double ecc);
double trueAnomalyFromMean(double M0, double ecc);

// Struct holding orbital elements
struct Orbit {
    double epoch_J;
    uint32_t epochUTC;
    double incl;
    double a;
    double ecc;
    double Omega;
    double omega;
    double M0;
    double n;

    double n_dot;

    void initFromTLE(char* line1, char* line2);
    void calcPosECI(double dt_sec, Vec3& posECI);
    void calcPosECI_UTC(uint64_t UTC_ms, Vec3& posECI);
    void calcPosVelECI(double dt_sec, Vec3& posECI, Vec3& velECI);
    void calcPosVelECI_UTC(uint64_t UTC_ms, Vec3& posECI, Vec3& velECI);
};
