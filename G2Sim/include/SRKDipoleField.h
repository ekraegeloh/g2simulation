#ifndef SRKDipoleField_HH
#define SRKDipoleField_HH

#include <TVector3.h>

#include "SRKField.h"

////////////////////////////////////////////////////////////////
/// class SRKUniformField
///
/// A uniform electric/magnetic field in an arbitrary direction
///
/// Author: Matthew Bales
///////////////////////////////////////////////////////////////

class SRKDipoleField: public SRKField
{
public:
	SRKDipoleField(SRKFieldSettings inpFS);

	virtual ~SRKDipoleField();

	void addFieldValue(const double localPoint[3], double fieldValue[9]);

private:
	double fieldComponents[3];
};

#endif
