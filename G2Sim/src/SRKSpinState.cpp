#include "SRKSpinState.h"

#include <iostream>

using namespace std;

SRKSpinState::SRKSpinState()
{
	// TODO Auto-generated constructor stub

}

SRKSpinState::~SRKSpinState()
{
	// TODO Auto-generated destructor stub
}

void SRKSpinState::print() const
{
	cout << scientific << endl;
	cout << "-------------------------------------------" << endl;
	cout << "Mag: " << M.X() << ", " << M.Y() << ", " << M.Z() << endl;
	cout << "Time: " << time << endl;
	cout << "-------------------------------------------" << endl;

}
