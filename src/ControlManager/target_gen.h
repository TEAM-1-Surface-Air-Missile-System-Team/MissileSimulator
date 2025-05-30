#ifndef TARGET_GEN_H
#define TARGET_GEN_H
#define intercept_range 100.0
#define R_detect 500000.0
#define R_engage 300000.0
#include "coordinate_transform.h"
typedef struct {
	double Tgt_Lat;
	double Tgt_Lon;
	double Tgt_Alt; // Taget in LLA
	double v0;
	double ECEF_x;
	double ECEF_y;
	double ECEF_z;

	double ECEF_velo_x;
	double ECEF_velo_y;
	double ECEF_velo_z; // Target in ECEF 

	double velo_x;
	double velo_y;
	double velo_z; // Target in Local Coordinate(NED,Body)

	double roll_angle;
	double yaw_angle;
	double pitch_angle;
	double max_time; // maximum flight time 
}Tgt;

double gravity_at_latitude(double lat_deg);

Tgt Ballistic_maneuver(Tgt target, double dt);

Tgt CV_manuever(Tgt target, double dt);

Tgt CTR_manuever(Tgt target, double dt, double g_value, int type, int clk);

Tgt guidance_ECEF_only(Tgt missile, Tgt target, double dt, double N_gain, double pursuit_gain);

void predict_ballistic_position_ecef(double x_prev, double y_prev, double z_prev, double x_curr, double y_curr, double z_curr, double dt, double t_predict, double* x_out, double* y_out, double* z_out, double* vxo, double* vyo, double* vzo);

void predict_CV_position_ecef(double x_prev, double y_prev, double z_prev, double x_curr, double y_curr, double z_curr, double dt, double t_predict, double* x_out, double* y_out, double* z_out);


#endif 