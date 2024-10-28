#include "bottle.h"
#define _USE_MATH_DEFINES // for C++  
#include <cmath>  




bottle::bottle(double rad, double h)
{
	radius = rad;
	height = h;
	water_flag = 0;
	water_filled = 0;
}


bottle::~bottle()
{

}

double bottle::calculate_com()
{
	double non_tilted_water_height = height*water_filled;
	double water_volume = M_PI * pow(radius, 2) * non_tilted_water_height;
	return 10000*water_volume;
}

void bottle::calculate_volume()
{
	double non_tilted_water_height = height*water_filled;
	double water_volume = M_PI * pow(radius, 2) * non_tilted_water_height;
	
	water_height1 = 3 * tan(tilted_angle) + non_tilted_water_height;
	water_height2 = 2 * non_tilted_water_height - water_height1;
	
}

void bottle::add_water()
{
	if (water_filled >= 1)
	{
		water_filled = 1;
	}
	else
	{
		water_filled += 0.0001;
	}
}
void bottle::remove_water()
{
	if (water_filled <= 0)
	{
		water_filled = 0;
	}
	else
	{
		water_filled -= 0.0001;
	}
}


int bottle::get_water_flag()
{
	return water_flag;
}

void bottle::set_water_flag(int i)
{
	water_flag = i;
}