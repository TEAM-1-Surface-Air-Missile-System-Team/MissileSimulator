#include "target_gen.h"
#include "coordinate_transform.h"

double gravity_at_latitude(double lat_deg) {
    double lat = deg2rad * lat_deg;
    double sin_lat = sin(lat);
    double sin2 = sin_lat * sin_lat;
    return g0 * (1 + 0.00193185265241 * sin2) / sqrt(1 - EARTH_E2 * sin2);
}
Tgt Ballistic_maneuver(Tgt target, double dt) {
    double px, py, pz;
    double vx, vy, vz;

    LLA_to_ECEF(target.Tgt_Lat, target.Tgt_Lon, target.Tgt_Alt, &px, &py, &pz);
    vx = target.ECEF_velo_x;
    vy = target.ECEF_velo_y;
    vz = target.ECEF_velo_z;


    double lat_now, lon_now, alt_now;
    int idx = 0;

    idx += 1;

    double r = sqrt(px * px + py * py + pz * pz);
    ECEF_to_LLA(px, py, pz, &lat_now, &lon_now, &alt_now);
    double g = gravity_at_latitude(lat_now);

    double ax = -g * (px / r);
    double ay = -g * (py / r);
    double az = -g * (pz / r);

    vx += ax * dt;
    vy += ay * dt;
    vz += az * dt;

    px += vx * dt;
    py += vy * dt;
    pz += vz * dt;

    ECEF_to_LLA(px, py, pz, &lat_now, &lon_now, &alt_now);

    target.ECEF_x = px;
    target.ECEF_y = py;
    target.ECEF_z = pz;

    target.Tgt_Lat = lat_now;
    target.Tgt_Lon = lon_now;
    target.Tgt_Alt = alt_now;

    target.ECEF_velo_x = vx;
    target.ECEF_velo_y = vy;
    target.ECEF_velo_z = vz;

    return target;
}

void predict_ballistic_position_ecef(double x_prev, double y_prev, double z_prev, double x_curr, double y_curr, double z_curr, double dt, double t_predict, double* x_out, double* y_out, double* z_out, double* vxo, double* vyo, double* vzo) {
    // init velo.
    double vx = (x_curr - x_prev) / dt;
    double vy = (y_curr - y_prev) / dt;
    double vz = (z_curr - z_prev) / dt;

    // init pos.
    double x = x_curr;
    double y = y_curr;
    double z = z_curr;

    // calc. init gravity
    double lat_deg, lon_deg, alt;
    ECEF_to_LLA(x, y, z, &lat_deg, &lon_deg, &alt);
    double G = gravity_at_latitude(lat_deg);

    // calc loop num
    int steps = (int)(t_predict / dt);

    for (int i = 0; i < steps; ++i) {
        // calc. gravity acc.
        double r = sqrt(x * x + y * y + z * z);
        double ax = -G * (x / r);
        double ay = -G * (y / r);
        double az = -G * (z / r);

        // update velocity
        vx += ax * dt;
        vy += ay * dt;
        vz += az * dt;

        // update pos
        x += vx * dt;
        y += vy * dt;
        z += vz * dt;

        // re-calculate gravity
        ECEF_to_LLA(x, y, z, &lat_deg, &lon_deg, &alt);
        G = gravity_at_latitude(lat_deg);
    }

    *x_out = x;
    *y_out = y;
    *z_out = z;
    *vxo = vx;
    *vyo = vy;
    *vzo = vz;
}
Tgt CV_manuever(Tgt target, double dt) {
    double x; double y; double z;
    double vx; double vy; double vz;
    //printf("vd: %.9f m \n", target.velo_z);
    //printf("lat lon alt: %.2f %.2f %.2f \n", target.Tgt_Lat, target.Tgt_Lon, target.Tgt_Alt);
    LLA_to_ECEF(target.Tgt_Lat, target.Tgt_Lon, target.Tgt_Alt, &x, &y, &z);
    target.ECEF_x = x;
    target.ECEF_y = y;
    target.ECEF_z = z;

    //printf("vx vy vz: %.2f %.2f %.2f \n", target.velo_x, target.velo_y, target.velo_z);
    ned_to_ecef_vector(target.Tgt_Lat, target.Tgt_Lon, target.velo_x, target.velo_y, target.velo_z, &vx, &vy, &vz);

    target.ECEF_velo_x = vx;
    target.ECEF_velo_y = vy;
    target.ECEF_velo_z = vz;

    target.ECEF_x = target.ECEF_x + target.ECEF_velo_x * dt;
    target.ECEF_y = target.ECEF_y + target.ECEF_velo_y * dt;
    target.ECEF_z = target.ECEF_z + target.ECEF_velo_z * dt;
    double lat; double lon; double alt;
    ECEF_to_LLA(target.ECEF_x, target.ECEF_y, target.ECEF_z, &lat, &lon, &alt);


    target.Tgt_Lat = lat;
    target.Tgt_Lon = lon;
    target.Tgt_Alt = alt;



    return target;
}
void predict_CV_position_ecef(double x_prev, double y_prev, double z_prev, double x_curr, double y_curr, double z_curr, double dt, double t_predict, double* x_out, double* y_out, double* z_out) {
    double vx = (x_curr - x_prev) / dt;
    double vy = (y_curr - y_prev) / dt;
    double vz = (z_curr - z_prev) / dt;

    // init pos.
    double x = x_curr;
    double y = y_curr;
    double z = z_curr;
    // calc loop num
    int steps = (int)(t_predict / dt);

    for (int i = 0; i < steps; ++i) {
        x += vx * dt;
        y += vy * dt;
        z += vz * dt;
    }

    *x_out = x;
    *y_out = y;
    *z_out = z;
}

Tgt CTR_manuever(Tgt target, double dt, double g_value, int type, int clk) {
    double x; double y; double z;
    double v0;
    v0 = target.v0;

    //printf("vd: %.9f m \n", target.velo_z);
    //printf("lat lon alt: %.2f %.2f %.2f \n", target.Tgt_Lat, target.Tgt_Lon, target.Tgt_Alt);
    LLA_to_ECEF(target.Tgt_Lat, target.Tgt_Lon, target.Tgt_Alt, &x, &y, &z);
    target.ECEF_x = x;
    target.ECEF_y = y;
    target.ECEF_z = z;
    // ned w.r.t roll yaw pitch angle 
    double yaw_rate; double roll_rate; double pitch_rate;
    roll_rate = (g_value * g0) / (target.v0);
    yaw_rate = (g_value * g0) / (target.v0);
    pitch_rate = (g_value * g0) / (target.v0);

    if (type == 1) { // yaw 
        target.yaw_angle = (double)target.yaw_angle + (double)(clk * rad2deg * yaw_rate * dt);
        //printf("yaw rate: %.3f \n", (double)(clk * rad2deg * yaw_rate * dt));
        //printf("yaw angle: %.3f \n",target.yaw_angle);
    }
    else if (type == 2) { // roll
        target.roll_angle = target.roll_angle + (double)(clk * rad2deg * roll_rate * dt);
    }
    else { // pitch 
        target.pitch_angle = target.pitch_angle + (double)(clk * rad2deg * pitch_rate * dt);
    }
    double ve = v0 * cos(deg2rad * target.pitch_angle) * sin(target.yaw_angle * deg2rad);
    double vn = v0 * cos(deg2rad * target.pitch_angle) * cos(target.yaw_angle * deg2rad);
    double vd = -v0 * sin(deg2rad * target.pitch_angle);  // down is positive in NED

    double vx, vy, vz;
    ned_to_ecef_vector(target.Tgt_Lat, target.Tgt_Lon, vn, ve, vd, &vx, &vy, &vz);
    target.velo_x = vn;
    target.velo_y = ve;
    target.velo_z = vd;

    //printf("vx vy vz: %.2f %.2f %.2f \n", target.velo_x, target.velo_y, target.velo_z);
    ned_to_ecef_vector(target.Tgt_Lat, target.Tgt_Lon, target.velo_x, target.velo_y, target.velo_z, &vx, &vy, &vz);

    target.ECEF_velo_x = vx;
    target.ECEF_velo_y = vy;
    target.ECEF_velo_z = vz;

    target.ECEF_x = target.ECEF_x + target.ECEF_velo_x * dt;
    target.ECEF_y = target.ECEF_y + target.ECEF_velo_y * dt;
    target.ECEF_z = target.ECEF_z + target.ECEF_velo_z * dt;
    double lat; double lon; double alt;
    ECEF_to_LLA(target.ECEF_x, target.ECEF_y, target.ECEF_z, &lat, &lon, &alt);

    target.Tgt_Lat = lat;
    target.Tgt_Lon = lon;
    target.Tgt_Alt = alt;



    return target;
}


Tgt guidance_ECEF_only(Tgt missile, Tgt target, double dt, double N_gain, double pursuit_gain) {
    // relative pos vector R = Target - Missile
    double rx = target.ECEF_x - missile.ECEF_x;
    double ry = target.ECEF_y - missile.ECEF_y;
    double rz = target.ECEF_z - missile.ECEF_z;

    double r_norm_sq = rx * rx + ry * ry + rz * rz;
    double r_norm = sqrt(r_norm_sq);
    if (r_norm < 1e-6) {
        r_norm = 1e-6;
    }// avoid div. by zero

    // relative velocity
    double vx_rel = target.ECEF_velo_x - missile.ECEF_velo_x;
    double vy_rel = target.ECEF_velo_y - missile.ECEF_velo_y;
    double vz_rel = target.ECEF_velo_z - missile.ECEF_velo_z;

    // LOS rotation rate
    double wx = (ry * vz_rel - rz * vy_rel) / r_norm_sq;
    double wy = (rz * vx_rel - rx * vz_rel) / r_norm_sq;
    double wz = (rx * vy_rel - ry * vx_rel) / r_norm_sq;

    // norm vector of velocity of missile  
    double vmx = missile.ECEF_velo_x;
    double vmy = missile.ECEF_velo_y;
    double vmz = missile.ECEF_velo_z;
    double v_missile_norm = sqrt(vmx * vmx + vmy * vmy + vmz * vmz);

    if (v_missile_norm < 1e-6) {
        v_missile_norm = 1e-6;
    }// avoid div. by zero

    double vxn = vmx / v_missile_norm;
    double vyn = vmy / v_missile_norm;
    double vzn = vmz / v_missile_norm;

    // closed velocity
    double v_closing = -(rx * vx_rel + ry * vy_rel + rz * vz_rel) / r_norm;

    // PN accleration
    double ax_pn = N_gain * v_closing * (wy * vzn - wz * vyn);
    double ay_pn = N_gain * v_closing * (wz * vxn - wx * vzn);
    double az_pn = N_gain * v_closing * (wx * vyn - wy * vxn);

    // pursuit acceleration
    double nx = rx / r_norm;
    double ny = ry / r_norm;
    double nz = rz / r_norm;

    double ax_pursuit = pursuit_gain * nx;
    double ay_pursuit = pursuit_gain * ny;
    double az_pursuit = pursuit_gain * nz;

    // total accel = PN_acceleration + pursuit acceleration
    double ax = ax_pn + ax_pursuit;
    double ay = ay_pn + ay_pursuit;
    double az = az_pn + az_pursuit;

    // update velocity
    missile.ECEF_velo_x += ax * dt;
    missile.ECEF_velo_y += ay * dt;
    missile.ECEF_velo_z += az * dt;

    // update position
    missile.ECEF_x += missile.ECEF_velo_x * dt;
    missile.ECEF_y += missile.ECEF_velo_y * dt;
    missile.ECEF_z += missile.ECEF_velo_z * dt;

    // update LLA
    double lat, lon, alt;
    ECEF_to_LLA(missile.ECEF_x, missile.ECEF_y, missile.ECEF_z, &lat, &lon, &alt);
    missile.Tgt_Lat = lat;
    missile.Tgt_Lon = lon;
    missile.Tgt_Alt = alt;

    return missile;
}

