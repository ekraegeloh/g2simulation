#include <SRKEquationOfMotion.h>
#include <iostream>
#include <iomanip>
#include <cmath>

using namespace std;

//#define SRKEQOFMOTIONDEBUG 1

SRKEquationOfMotion::SRKEquationOfMotion()
{
	theGlobalField=nullptr;
	gyromagneticRatio = 0.00116592091;  //a_mu (unitless)
}

SRKEquationOfMotion::SRKEquationOfMotion(SRKGlobalField* inpGlobalField)
{
	theGlobalField = inpGlobalField;
	gyromagneticRatio=0;
}

void SRKEquationOfMotion::operator()(const SRKODEState& x, SRKODEState& dxdt, const SRKSpinFloat t)
{
	SRKEqOfMotion(x,dxdt,t);
}

bool SRKEquationOfMotion::isQuad(const double phi)
{
	if(phi>=M_PI_4 && phi<M_PI/5 + M_PI_4){
		return false;
	} else if(phi>=3*M_PI_4 && phi<M_PI/5 + 3*M_PI_4){
		return false;
	} else if(phi>=-M_PI_4 && phi<M_PI/5 - M_PI_4){
		return false;
	} else if(phi>=-3*M_PI_4 && phi<M_PI/5 - 3*M_PI_4){
		return false;
	} else{
		return true;
	}
}

void SRKEquationOfMotion::SRKEqOfMotion(const SRKODEState& x, SRKODEState& dxdt, const SRKSpinFloat /* t */)
{
	posX = static_cast<double> (x[0]);
	posZ = static_cast<double> (x[2]);
	localPos[0] = sqrt(posX*posX + posZ*posZ) - 7.112;//ring radius
	localPos[1] = static_cast<double> (x[1]);
	localPos[2] = atan2(posZ,posX);

	theGlobalField->getFieldValue(localPos, fieldDouble);
	for(int i=0;i<3;i++)
	{
		theField[i]=fieldDouble[i];
	}
	if(theGlobalField->isNotEcont() && !isQuad(localPos[2])){    //sectional
		theField[3] = theField[4] = theField[5] = 0.;
	}
	else{
		for(int i=3;i<6;i++)
		{
			theField[i]=fieldDouble[i];
		}
	}
	for(int i=6;i<9;i++)
	{
		theField[i]=fieldDouble[i];
	}
/*	#ifdef SRKEQOFMOTIONDEBUG
		cout << "B field: ";
		cout << theField[0] << " " << theField[1] << " " << theField[2] << endl;
		cout << "E Field: ";
		cout << theField[3] << " " << theField[4] << " " << theField[5] << endl;
	#endif
*/
	const double motionalCoeff = 1. / (299792458. * 299792458.);
	const int polarity = 1;
	const double chargeToMassRatio = polarity*1.6021766208e-19 / 1.883531594e-28;
	SRKSpinFloat gamma = 1/sqrt(1 - motionalCoeff*(x[3]*x[3] + x[4]*x[4] + x[5]*x[5]) );

	#ifdef SRKEQOFMOTIONDEBUG
		cout << "Gamma: " << gamma << endl;
	#endif

	SRKSpinFloat vDotE = x[3]*theField[3] + x[4]*theField[4] + x[5]*theField[5];

	dxdt[0] = x[3];
	dxdt[1] = x[4];
	dxdt[2] = x[5];
	//acceleration
	dxdt[3] = chargeToMassRatio * (theField[3] + x[4]*theField[2] - x[5]*theField[1] - motionalCoeff*x[3]*vDotE) / gamma;
 	dxdt[4] = chargeToMassRatio * (theField[4] + x[5]*theField[0] - x[3]*theField[2] - motionalCoeff*x[4]*vDotE) / gamma;
	dxdt[5] = chargeToMassRatio * (theField[5] + x[3]*theField[1] - x[4]*theField[0] - motionalCoeff*x[5]*vDotE) / gamma;

	//omega_s according to BMT eq.
	SRKSpinFloat w[3];
	SRKSpinFloat vDotB = x[3]*theField[0] + x[4]*theField[1] + x[5]*theField[2];

	w[0] = chargeToMassRatio * ((gyromagneticRatio + 1/gamma) * theField[0] - (gyromagneticRatio + 1/(gamma + 1)) * motionalCoeff * (x[4]*theField[5] - x[5]*theField[4]) - gyromagneticRatio*gamma/(gamma + 1) * motionalCoeff * x[3] * vDotB);
	w[1] = chargeToMassRatio * ((gyromagneticRatio + 1/gamma) * theField[1] - (gyromagneticRatio + 1/(gamma + 1)) * motionalCoeff * (x[5]*theField[3] - x[3]*theField[5]) - gyromagneticRatio*gamma/(gamma + 1) * motionalCoeff * x[4] * vDotB);
	w[2] = chargeToMassRatio * ((gyromagneticRatio + 1/gamma) * theField[2] - (gyromagneticRatio + 1/(gamma + 1)) * motionalCoeff * (x[3]*theField[4] - x[4]*theField[3]) - gyromagneticRatio*gamma/(gamma + 1) * motionalCoeff * x[5] * vDotB);


	//magnetization
	dxdt[6] = x[7]*w[2] - x[8]*w[1];
	dxdt[7] = x[8]*w[0] - x[6]*w[2];
	dxdt[8] = x[6]*w[1] - x[7]*w[0];


#ifdef SRKEQOFMOTIONDEBUG
	cout << "InitialState: ";
	printMotionState(x);
	cout << "Delta: ";
	printMotionState(dxdt);
#endif

}
