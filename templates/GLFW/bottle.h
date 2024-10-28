#pragma once
class bottle
{
protected:
	double radius;
	double height;
	double water_height1;
	double water_height2;
	int water_flag; //0: no water in/out	1: water in		2: water out
	double water_filled; //water percentage in the bottle 0: empty	1: full
	double tilted_angle; //tilted_angle of bottle
	double angular_velocity; //angular_velocity of 


	double com;

public:
	bottle(double rad, double h);
	~bottle();
	double calculate_com();
	void calculate_volume();
	void add_water();
	void remove_water();
	void set_water_flag(int i);
	int get_water_flag();
	


};

