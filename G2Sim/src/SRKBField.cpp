#include "SRKBField.h"
#include <iostream>

//#define SRKBDEBUG 1

SRKBField::SRKBField(SRKFieldSettings inpFS) :
	SRKField(inpFS)
{

}

void SRKBField::addFieldValue(const double localPoint[3], double fieldValue[9])
{
	const float rlim = 0.045;
	double theta = atan2(localPoint[1], localPoint[0]);
	double r = sqrt(localPoint[0]*localPoint[0] + localPoint[1]*localPoint[1]);

	double Brad = 0.0;//scalingFactorWUnits * r * 1e-6 * (0.24*sin(theta)-0.29*cos(theta) + r * ((-0.53)*sin(2*theta)-(-1.09)*cos(2*theta) + r * ((-0.1)*sin(3*theta)-(-0.15)*cos(3*theta) + r * (0.82*sin(4*theta)-0.54*cos(4*theta)) / rlim) / rlim) / rlim) / rlim;
	fieldValue[g4FieldX] += Brad*cos(localPoint[2]);
	fieldValue[g4FieldY] += scalingFactorWUnits; //* (1 + r * 1e-6 * (0.24*cos(theta)-0.29*sin(theta) + r * ((-0.53)*cos(2*theta)-(-1.09)*sin(2*theta) + r * ((-0.1)*cos(3*theta)-(-0.15)*sin(3*theta) + r * (0.82*cos(4*theta)-0.54*sin(4*theta)) / rlim) / rlim) / rlim) / rlim);
	fieldValue[g4FieldZ] += Brad*sin(localPoint[2]);

#ifdef SRKBDEBUG
			//Electric field values at position
			std::cout << "B_rad:"<< Brad << "at phi:" << localPoint[2] << std::endl;
			std::cout << "B_x:"<< double(fieldValue[0]) << std::endl;
			std::cout << "B_y:"<< double(fieldValue[1]) << std::endl;
			std::cout << "B_z:"<< double(fieldValue[2]) << std::endl;
#endif

}
