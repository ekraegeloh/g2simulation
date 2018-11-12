#ifndef SRKEField_H
#define SRKEField_H 1

#include "SRKField.h"

////////////////////////////////////////////////////////////////
/// class SRKEField
/// E-field multipole expansion defined in the x' and y' directions
/// The field is zero at Rbar and diverges from there.
/// Uses scaleFactor as quadrupole constant K in V/m
///
/// Author: Eva Kraegeloh
///////////////////////////////////////////////////////////////

class SRKEField: public SRKField
{
public:
	SRKEField(SRKFieldSettings inpFS);
	~SRKEField(){}
	void addFieldValue(const double localPoint[3], double fieldValue[9]);
};
#endif
