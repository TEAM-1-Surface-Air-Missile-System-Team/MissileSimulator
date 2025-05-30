#include "coordinate_transform.h"

void LLA_to_ECEF(double lat_deg, double lon_deg, double alt, double* x, double* y, double* z) {
    double lat = lat_deg * deg2rad;
    double lon = lon_deg * deg2rad;
    double sin_lat = sin(lat), cos_lat = cos(lat);
    double sin_lon = sin(lon), cos_lon = cos(lon);
    double N = EARTH_A / sqrt(1 - EARTH_E2 * sin_lat * sin_lat);
    *x = (N + alt) * cos_lat * cos_lon;
    *y = (N + alt) * cos_lat * sin_lon;
    *z = (N * (1 - EARTH_E2) + alt) * sin_lat;
}

void ECEF_to_LLA(double x, double y, double z, double* lat_deg, double* lon_deg, double* alt) {
    double eps = 1e-9;
    double p = sqrt(x * x + y * y);
    double lon = atan2(y, x);
    double lat = atan2(z, p * (1 - EARTH_E2));
    double N;
    double lat_prev;
    do {
        lat_prev = lat;
        N = EARTH_A / sqrt(1 - EARTH_E2 * sin(lat) * sin(lat));
        lat = atan2(z + EARTH_E2 * N * sin(lat), p);
    } while (fabs(lat - lat_prev) > eps);
    N = EARTH_A / sqrt(1 - EARTH_E2 * sin(lat) * sin(lat));
    *alt = p / cos(lat) - N;
    *lat_deg = lat * rad2deg;
    *lon_deg = lon * rad2deg;
}



void ned_to_ecef_vector(double lat_deg, double lon_deg, double vn, double ve, double vd, double* vx, double* vy, double* vz) {
    double lat = deg2rad * lat_deg;
    double lon = deg2rad * lon_deg;
    double sin_lat = sin(lat), cos_lat = cos(lat);
    double sin_lon = sin(lon), cos_lon = cos(lon);

    *vx = -sin_lat * cos_lon * vn - sin_lon * ve - cos_lat * cos_lon * vd;
    *vy = -sin_lat * sin_lon * vn + cos_lon * ve - cos_lat * sin_lon * vd;
    *vz = cos_lat * vn - sin_lat * vd;
}

double calc_dist(double x0, double y0, double z0, double x1, double y1, double z1) {
    double dist = sqrt((x1 - x0) * (x1 - x0) + (y1 - y0) * (y1 - y0) + (z1 - z0) * (z1 - z0));


    return dist;
}
double calc_dist_LLA(double lat, double lon, double alt, double lat1, double lon1, double alt1) {
    double x0; double y0; double z0;
    LLA_to_ECEF(lat, lon, alt, &x0, &y0, &z0);
    double x1;double y1; double z1;
    LLA_to_ECEF(lat1, lon1, alt1, &x1, &y1, &z1);

    double dist = sqrt((x1 - x0) * (x1 - x0) + (y1 - y0) * (y1 - y0) + (z1 - z0) * (z1 - z0));

    return dist;
}