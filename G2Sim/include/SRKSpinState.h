#ifndef SRKSPINSTATE_H_
#define SRKSPINSTATE_H_

#include "TVector3.h"
#include "TString.h"

class SRKSpinState
{
public:
	SRKSpinState();
	virtual ~SRKSpinState();

	TVector3 M;
	double time=0.;

	void print() const;
};

#endif /* SRKSPINSTATE_H_ */
