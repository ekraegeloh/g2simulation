#include <SRKDipoleField.h>
#include "SRKGlobalField.h"

#include <time.h>
#include <iostream>

//#include "SRKBField.h"
#include "SRKInterpolatedField.h"
#include "SRKGradientField.h"
#include "SRKQuadrupoleField.h"
#include "SRKSextupoleField.h"
#include "SRKMultipoleField.h"
//#include "SRKEField.h"

using namespace std;

SRKGlobalField::SRKGlobalField()
{
	currentFieldSettingsToModify = -1;
	Econt = true;
}

SRKGlobalField::~SRKGlobalField()
{
	clear();
}

void SRKGlobalField::updateField()
{

	clear();
	currentFieldSettingsToModify = -1;

	constructFields();

}

void SRKGlobalField::addField(SRKField* f)
{
	theFields.push_back(f);
}

///template <class floatlike1, class floatlike2>
void SRKGlobalField::getFieldValue(const double* point, double* outField) const // pointer is basically equal to array; point is actually array, not memory address; think about pass by ref, using std::array
{
	for (unsigned int i = 0; i < 9; ++i)
	{

		outField[i]=0;

	}
	//point is local point: [2] is azimuth angle
	for (unsigned int i = 0; i < theFields.size(); ++i)
	{

		theFields[i]->addFieldValue(point, outField); //check use of pointer here??

	}
}

//clears the field list NOT the fieldSettingsToLoad
void SRKGlobalField::clear()
{
	for (unsigned int i = 0; i < theFields.size(); ++i)
	{

		delete theFields[i];

	}

	theFields.clear();

}

void SRKGlobalField::setCurrentFieldSettingsToModify(int inp)
{
	if(inp >= 0 && inp < MAX_SRK_NUM_FIELDS) //Is the number within a reasonable range
	{
		currentFieldSettingsToModify = inp;
		if(fieldSettingsToLoad.size() <= (unsigned int) inp) //Do we need to grow the vector
		{
			fieldSettingsToLoad.resize(inp + 1); //New FieldSettings that are created are not loaded because their scaling value is set to zero by default
		}
	}
	else
	{
		cout << "Field to modify not within appropriate range of 0 to " << MAX_SRK_NUM_FIELDS << "." << endl;
	}
}

void SRKGlobalField::constructFields()
{
	for (unsigned int i = 0; i < fieldSettingsToLoad.size(); i++)
	{
		if(fieldSettingsToLoad[i].scalingValue != 0 || fieldSettingsToLoad[i].coefficients.size() != 0 ) //Only load fields if their scaling values are not zero
		{
			if(fieldSettingsToLoad[i].fieldClass == FIELDCLASS_INTERPOLATION)
			{
				addField(new SRKInterpolatedField(fieldSettingsToLoad[i]));
			}
			else if(fieldSettingsToLoad[i].fieldClass == FIELDCLASS_DIPOLE)
			{
				addField(new SRKDipoleField(fieldSettingsToLoad[i]));
			}
			else if(fieldSettingsToLoad[i].fieldClass == FIELDCLASS_GRADIENT)
			{
				addField(new SRKGradientField(fieldSettingsToLoad[i]));
			}
			else if (fieldSettingsToLoad[i].fieldClass == FIELDCLASS_QUADRUPOLE)
			{
				addField(new SRKQuadrupoleField(fieldSettingsToLoad[i]));
			}
			else if(fieldSettingsToLoad[i].fieldClass == FIELDCLASS_SEXTUPOLE)
			{
				addField(new SRKSextupoleField(fieldSettingsToLoad[i]));
			}
//			else if(fieldSettingsToLoad[i].fieldClass == FIELDCLASS_B)
//			{
//				addField(new SRKBField(fieldSettingsToLoad[i]));
//			}
//			else if(fieldSettingsToLoad[i].fieldClass == FIELDCLASS_E)
//			{
//				addField(new SRKEField(fieldSettingsToLoad[i]));
//			}
			else if(fieldSettingsToLoad[i].fieldClass == FIELDCLASS_MULTIPOLE)
			{
				addField(new SRKMultipoleField(fieldSettingsToLoad[i]));
			}
		}

	}
	return;
}
