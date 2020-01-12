#ifndef SRKMultiField_H
#define SRMultiField_H 1

#include "SRKField.h"

////////////////////////////////////////////////////////////////
/// class SRKQuadrupoleField
/// Quadrupole field defined in the x and z directions
/// The field is zero at centerPos and diverges from there.
/// Uses scaleFactor as quadrupole constant K in [FIELDUNITS]/m
///
/// Author: Eva Kraegeloh
///////////////////////////////////////////////////////////////

class SRKMultipoleField: public SRKField
{
public:
	SRKMultipoleField(SRKFieldSettings inpFS);
	~SRKMultipoleField(){}
	void addFieldValue(const double localPoint[3], double fieldValue[9]);
};
#endif
