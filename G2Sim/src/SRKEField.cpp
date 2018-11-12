#include "SRKEField.h"
#include <iostream>

//#define SRKEDEBUG 1

SRKEField::SRKEField(SRKFieldSettings inpFS) :
	SRKField(inpFS)
{

}

void SRKEField::addFieldValue(const double localPoint[3], double fieldValue[9])
{
	double Erad = scalingFactorWUnits*(2*localPoint[0]);// - 256*pow(localPoint[0],3) + 768*localPoint[0]*pow(localPoint[1],2) - 6144*pow(localPoint[0],5) + 61440*pow(localPoint[0],3)*pow(localPoint[1],2) - 30720*pow(localPoint[1],4)*localPoint[0]);
	fieldValue[g4FieldX] += Erad*cos(localPoint[2]);
	fieldValue[g4FieldY] += scalingFactorWUnits*(-2*localPoint[1]);// - 256*pow(localPoint[1],3) + 768*pow(localPoint[0],2)*localPoint[1] + 6144*pow(localPoint[1],5) - 61440*pow(localPoint[1],3)*pow(localPoint[0],2) + 30720*pow(localPoint[0],4)*localPoint[1]);
	fieldValue[g4FieldZ] += Erad*sin(localPoint[2]);

#ifdef SRKEDEBUG
			//Electric field values at position
			std::cout << "E_rad:"<< Erad << "at phi:" << localPoint[2] << std::endl;
			std::cout << "E_x:"<< double(fieldValue[3]) << std::endl;
			std::cout << "E_y:"<< double(fieldValue[4]) << std::endl;
			std::cout << "E_z:"<< double(fieldValue[5]) << std::endl;
#endif

}

//turn on and off with phi = localPoint[2]
// scalingFactor in V/m^2
