#include "bottle.h"

#define _USE_MATH_DEFINES // for C++  
#include <cmath>  
#include "chai3d.h"
#include <GLFW/glfw3.h>
#include <mutex>

std::mutex mm;

using namespace chai3d;
using namespace std;

bottle::bottle(double rad, double h)
{
	radius = rad;
	height = h;
	water_flag = 0;
	water_filled = 0.0;
	water_height1 = 0;
	water_height2 = 0;
	tilted_radius = radius;
}


bottle::~bottle()
{

}

cVector3d bottle::calculate_com_real(double *cx, double *cy)
{
	cVector3d p0;
	cVector3d p1;
	cVector3d p2;
	cVector3d p3;

	p0 = cVector3d(-radius, 0.0, 0.0);
	p1 = cVector3d(radius, 0.0, 0.0);
	p2 = cVector3d(radius, water_height1, 0.0);
	p3 = cVector3d(-radius, water_height2, 0.0);


	std::vector<cVector3d> points;
	points.push_back(p0);
	points.push_back(p1);
	points.push_back(p2);
	points.push_back(p3);

	//double signed_area = 0;
	//for (int i = 0; i < points.size(); i++)
	//{
	//	int j = (i + 1) % points.size();
	//	signed_area += (points[i].x() * points[j].y()) - (points[j].x()*points[i].y());

	//}

	//signed_area = signed_area / 2.0;

	//*cx = 0;
	//*cy = 0;

	//for (int i = 0; i < points.size(); i++)
	//{
	//	int j = (i + 1) % points.size();
	//	*cx += (points[i].x() + points[j].x())*(points[i].x()*points[j].y() - points[j].x()*points[i].y());
	//	*cy += (points[i].y() + points[j].y())*(points[i].x()*points[j].y() - points[j].x()*points[i].y());

	//}

	//*cx = *cx / 6.0 / signed_area;
	//*cy = *cy / 6.0 / signed_area;

	double cxr, cyr;
	double cxt, cyt;

	cxr = 2*tilted_radius/2.0;
	cyr = water_height2 / 2.0;

	cxt = 2.0 * tilted_radius / 3.0;
	cyt = (water_height1 - water_height2) / 3;
	/*cout << "radius: " << radius << " temp_radius: " << tilted_radius << endl;*/

	if (water_height2 <= 0)
	{
		*cx = 2.0 * tilted_radius / 3.0;
		*cy = water_height1 / 3;
	}
	else
	{
		*cx = (cxr * 2 * tilted_radius*water_height2 + cxt*(water_height1 - water_height2) * 2 * tilted_radius / 2.0) / (2 * tilted_radius*water_height2 + (water_height1 - water_height2) * 2 * tilted_radius / 2.0);
		*cy = (cyr * 2 * tilted_radius*water_height2 + cyt*(water_height1 - water_height2) * 2 * tilted_radius / 2.0) / (2 * tilted_radius*water_height2 + (water_height1 - water_height2) * 2 * tilted_radius / 2.0);
		
	}
	*cx = *cx - radius;
	if (water_height1 <= 0)
	{
		*cx = 0;
		*cy = 0;
	}
	double rr = sqrt(pow(*cx, 2) + pow(*cy, 2));

	
	
	//cout << *cx << endl;
	//*cx = 0.0;
	//if (water_height2 == 0)
	//{
	//	*cy = water_height1 / 3.0;
	//}
	//else
	//{
	//	*cy = (water_height1 + water_height2) / 4.0;
	//}



	double angle = atan(*cx / *cy);
	//angle = angle + tilted_angle;
	double len = sqrt(pow(*cx, 2) + pow(*cy, 2));

	double x = len*sin(tilted_angle)*cos(pi);
	double y = len*sin(tilted_angle)*sin(pi);
	double z = len*cos(tilted_angle);


	//cout << "cx:  " << *cx * rr*sin(angle)*cos(phi) << endl;
	//cout << "cy:  " << *cy * rr*cos(angle) << endl;




	//cout << "x:  " << x << "  y:  " << y << "  z:  " << z << endl;
	//cout << "water height1: " << water_height1 << "water_height2: " << water_height2 << endl;
	com = cVector3d(x, y, z);
	double non_tilted_water_height = height*water_filled;
	double water_volume = M_PI * pow(radius, 2) * non_tilted_water_height;
	water_volume = 400 * water_volume;
	cVector3d torque = len *sin(tilted_angle+angle) * cVector3d(water_volume,0.0, 0.0);
	/*cout << len *sin(tilted_angle + angle) << endl;*/
	
	return torque;
}


double bottle::calculate_com()
{


	double non_tilted_water_height = height*water_filled;
	double water_volume = M_PI * pow(radius, 2) * non_tilted_water_height;
	return 400.0 * water_volume;
}

void bottle::calculate_volume()
{
	mm.lock();
	double non_tilted_water_height = height*water_filled;
	double water_volume = M_PI * pow(radius, 2) * non_tilted_water_height;

	
	if (tilted_angle * 180.0 / M_PI >= 90.0)
	{
		//water_height1 = 0;
		//water_height2 = 0;
		
	}
	else
	{
		//cout << tan(tilted_angle) << endl;
		//cout << tan(tilted_angle) << endl;
		water_height1 = non_tilted_water_height + radius * tan(tilted_angle);
		water_height2 = non_tilted_water_height - radius * tan(tilted_angle);
		
		if (water_volume == 0)
		{
			water_height2 = 0;
			water_height1 = 0;
		}
		else
		{
			double temp_radius = cbrt(water_volume / (M_PI*tan(tilted_angle)));
			
			/*if (tilted_angle >= atan(water_volume / (M_PI*pow(radius, 3))))*/
			if (temp_radius < radius)
			{
				water_height2 = 0;
				tilted_radius = temp_radius;
				water_height1 = 2 * tilted_radius*tan(tilted_angle);
				//cout << tilted_radius << endl;

			}
		}
		


		//if (water_height2 <= 0)
		//{
		//	water_height2 = 0;
		//	tilted_radius = pow(water_volume/pow(tan(tilted_angle), 2)/M_PI*3.0/4.0,1.0/4.0);
		//	water_height1 = 2 * tilted_radius*tan(tilted_angle);
		//	cout << tilted_radius << endl;
		//}


		double loss_radius;
		double loss_volume;

		double loss_rate;

		if (water_height1 < height)
		{
			loss_radius = 0;
			loss_volume = 0;
		}
		else
		{
			
			loss_radius = ((water_height1 - height) * tan(tilted_angle));
			loss_volume = M_PI*pow(loss_radius, 2)*(water_height1 - height) / 2;


			//manning
			loss_rate = pow(loss_radius, 2.0 / 3.0)*cbrt(tan(tilted_angle))/1000000.0;

			//Darcy-Weisbach
			//Average velocity of section
			double choco_density = 100;
			double choco_viscousity = 100000;
			double choco_unit_weight = choco_density * 9.8;
			//double avg_vel = choco_unit_weight / choco_viscousity*sin(tilted_angle)*loss_radius*loss_radius / 3.0;
			double avg_vel = choco_unit_weight / choco_viscousity*sin(tilted_angle)*(2*radius)*(2*loss_radius) / 3.0;
			double Reynold_number = avg_vel * height * choco_density / choco_viscousity;
			double darcy_factor = 64.0 / Reynold_number;

			loss_rate = sqrt(8.0 / darcy_factor)*sqrt(9.8*loss_radius*tan(tilted_angle))/1000;

			



			//cout << Reynold_number << endl;




			//loss_rate = 0.00000001;
			loss_volume = loss_rate *  M_PI*pow(loss_radius, 2);
			
			//cout << water_volume << endl;
			
			//loss_volume = M_PI * pow(loss_radius, 2) / 3.0 * (water_height1 - height);
		}
		if (loss_volume > water_volume)
		{

			//cout << tilted_radius << endl;
			water_volume = 0;
			
		}
		else
		{
			//cout << "water_volume: " << water_volume*1000 << " loss_volume: " << loss_volume*1000 << endl;
			water_volume = water_volume - loss_volume;
			
		}
			
		non_tilted_water_height = water_volume / M_PI / pow(radius, 2);
		water_filled = non_tilted_water_height / height;
	}
	mm.unlock();
	/*cout << tilted_radius << endl;*/


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

void bottle::set_tilted_angle(double theta, double phi)
{
	tilted_angle = theta;
	double acc_angle = atan(sqrt(pow(accel.x(),2)+pow(accel.y(),2)) / (9.8+accel.z()));
	//if (accel.x() <= 0)
	//{
	//	acc_angle = -acc_angle;
	//}
	tilted_angle += acc_angle;
	pi = phi;
}