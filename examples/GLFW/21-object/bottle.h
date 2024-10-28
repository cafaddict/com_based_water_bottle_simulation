#pragma once
#include <chai3d.h>
#include <GLFW/glfw3.h>
using namespace chai3d;
using namespace std;

class bottle
{
protected:
	
	double tilted_radius;

	double height;

	int water_flag; //0: no water in/out	1: water in		2: water out
	double water_filled; //water percentage in the bottle 0: empty	1: full
	double tilted_angle; //tilted_angle of bottle
	double pi; // need to calculate back cartesian
	double angular_velocity; //angular_velocity of 

	


	cVector3d com;

public:
	bottle(double rad, double h);
	~bottle();
	double radius;
	double water_height1;
	double water_height2;


	double calculate_com();
	void calculate_volume();
	void add_water();
	void remove_water();
	void set_water_flag(int i);
	void set_tilted_angle(double theta, double phi);
	int get_water_flag();
	cVector3d calculate_com_real(double *cx, double *cy);

	cVector3d accel;
	cVector3d cartesian;

	double phi;



};
