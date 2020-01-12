#include "SRKMultipoleField.h"
#include <iostream>

//#define SRKMULTIDEBUG 1

SRKMultipoleField::SRKMultipoleField(SRKFieldSettings inpFS) :
	SRKField(inpFS)
{

}

void SRKMultipoleField::addFieldValue(const double localPoint[3], double fieldValue[9])
{
	//const float rlim = 0.045;
	double theta = atan2(localPoint[1], localPoint[0]);
	double r = sqrt(localPoint[0]*localPoint[0] + localPoint[1]*localPoint[1]);
	if(r>3.0){
		std::cout << "r: " << r << ", x: " << localPoint[0] << ", y: " << localPoint[1] << std::endl;
	}
	double a0 = Coefs[0];
	double Scale = a0/2;
	int n = Coefs.size();
	if(n>1){
		for(int i=1;i<=n/2;i++){
			Scale += Coefs[i]*cos(i*localPoint[2]);
			Scale += Coefs[i+(n/2)]*sin(i*localPoint[2]);
		}
	}

	double radField = Scale*pow(r, order)*((1-skew)*sin(order*theta) -skew*cos(order*theta));
	fieldValue[g4FieldX] += radField*cos(localPoint[2]);
	fieldValue[g4FieldY] += Scale*pow(r,order)*((1-skew)*cos(order*theta) +skew*sin(order*theta));
	fieldValue[g4FieldZ] += radField*sin(localPoint[2]);

//	if(order==1 && skew==0){
//		std::cout << "By of order " << order << " skew " << skew << ": " << Scale*pow(r/rlim,order)*((1-skew)*cos(order*theta) +skew*sin(order*theta)) << std::endl;
//		std::cout << "at r: " << r << ", theta: " << theta << std::endl;
//	}

#ifdef SRKMULTIDEBUG
			//Magnetic field values at position
			std::cout << "B Quad x:"<< double(scalingFactorWUnits*Point[1]) << std::endl;
			std::cout << "B Quad y:"<< double(scalingFactorWUnits*Point[0]) << std::endl;
#endif

}
