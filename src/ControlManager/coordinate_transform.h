#ifndef COORDINATE_TRANSFORM_H
#define	COORDINATE_TRANSFORM_H

#define _CRT_SECURE_NO_WARNINGS
#define _USE_MATH_DEFINES
#include <stdio.h>
#include <math.h>

#define pi M_PI
#define deg2rad (pi / 180.0)
#define rad2deg (180.0 / pi)
#define g0 9.7803253359
#define EARTH_A 6378137.0
#define EARTH_B 6356752.3142
#define EARTH_E2 (1 - (EARTH_B * EARTH_B) / (EARTH_A * EARTH_A))



void LLA_to_ECEF(double lat_deg, double lon_deg, double alt, double* x, double* y, double* z);
void ECEF_to_LLA(double x, double y, double z, double* lat_deg, double* lon_deg, double* alt);
void ned_to_ecef_vector(double lat_deg, double lon_deg, double vn, double ve, double vd, double* vx, double* vy, double* vz);
double calc_dist(double x0, double y0, double z0, double x1, double y1, double z1);
double calc_dist_LLA(double lat, double lon, double alt, double lat1, double lon1, double alt1);

#endif
