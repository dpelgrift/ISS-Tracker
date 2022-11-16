#include "Arduino.h"

/*
* WGS 84 four defining parameters and several commonly used derived parameters.
* All parameters are stored with the exact number of significant digits provided
* by the WGS 84 rublished report.
*/


// WGS 84 Defining Parameters
float a           = 6378137.0;           // Semi - major Axis[m]
float a_sqrd      = pow(a, 2);           // Semi - major Axis[m] squared
float f           = 1.0 / 298.257223563; // Flattening
float omega_E     = 7292115.0e-11;       // Angular velocity of the Earth[rad / s]
float omega_E_GPS = 7292115.1467e-11;    // Angular velocity of the Earth[rad / s]
                                         // According to ICD - GPS - 200

float GM = 3.986004418e14; // Earth's Gravitational Constant [m^3/s^2]
                           // (mass of earth's atmosphere included)

float GM_GPS = 3.9860050e14; // The WGS 84 GM value recommended for GPS receiver usage
                             // by the GPS interface control document(ICD - GPS - 200)
                             // differs from the current refined WGS 84 GM value.

// WGS 84 Ellipsoid Derived Geometric Constants
float b              = 6356752.3142;       // Semi - minor axis[m]
float b_sqrd         = pow(b, 2);          // Semi - minor axis[m] squared
float ecc            = 8.1819190842622e-2; // First eccentricity
float ecc_sqrd       = pow(ecc, 2);        // First eccentricity squared
float ecc_prime      = 8.2094437949696e-2; // Second eccentricity
float ecc_prime_sqrd = pow(ecc_prime, 2);  // Second eccentricity squared
float r              = (2*a + b) / 3;      // Arithmetic mean radius [m]