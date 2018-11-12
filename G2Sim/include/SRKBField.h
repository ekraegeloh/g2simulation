#ifndef SRKBField_H
#define SRKBField_H 1

#include "SRKField.h"

////////////////////////////////////////////////////////////////
/// class SRKBField
/// B-field multipoles defined in the x' and y' directions
/// Uses scaleFactor as B0 field strength in T
///
/// Author: Eva Kraegeloh
///////////////////////////////////////////////////////////////

class SRKBField: public SRKField
{
public:
	SRKBField(SRKFieldSettings inpFS);
	~SRKBField(){}
	void addFieldValue(const double localPoint[3], double fieldValue[9]);
};
#endif
