#include <SRKODEState.h>
#include <iostream>

using namespace std;


void setMotionState(SRKODEState& outState, const TVector3* pos, const TVector3* vel, const TVector3* M)
{
	outState[0] = SRKSpinFloat(pos->X());
	outState[1] = SRKSpinFloat(pos->Y());
	outState[2] = SRKSpinFloat(pos->Z());
	outState[3] = SRKSpinFloat(vel->X());
	outState[4] = SRKSpinFloat(vel->Y());
	outState[5] = SRKSpinFloat(vel->Z());
	outState[6] = SRKSpinFloat(M->X());
	outState[7] = SRKSpinFloat(M->Y());
	outState[8] = SRKSpinFloat(M->Z());
}

//Motion doesn't need the precision spin does
void updateMotionStatePosVel(SRKODEState& outState, const SRKMotionState& inpMotionState)
{
	outState[0] = SRKSpinFloat(inpMotionState.pos.X());
	outState[1] = SRKSpinFloat(inpMotionState.pos.Y());
	outState[2] = SRKSpinFloat(inpMotionState.pos.Z());
	outState[3] = SRKSpinFloat(inpMotionState.vel.X());
	outState[4] = SRKSpinFloat(inpMotionState.vel.Y());
	outState[5] = SRKSpinFloat(inpMotionState.vel.Z());
	outState[9] = SRKSpinFloat(inpMotionState.time);
}

void updateMotionStateMag(SRKODEState& outState, const SRKSpinState& inpMotionState)
{
	outState[6] = SRKSpinFloat(inpMotionState.M.X());
	outState[7] = SRKSpinFloat(inpMotionState.M.Y());
	outState[8] = SRKSpinFloat(inpMotionState.M.Z());
}

void printMotionState(const SRKODEState& theState)
{
	cout << "----------------------------" << endl;
	cout << "Pos: " << static_cast<double>(theState[0]) << ", " << static_cast<double>(theState[1]) << ", " << static_cast<double>(theState[2]) << endl;
	cout << "Vel: " << static_cast<double>(theState[3]) << ", " << static_cast<double>(theState[4]) << ", " << static_cast<double>(theState[5]) << endl;
	cout << "Mag: " << static_cast<double>(theState[6]) << ", " << static_cast<double>(theState[7]) << ", " << static_cast<double>(theState[8]) << endl;
	cout <<	"Time: " << static_cast<double>(theState[9]) << endl;
	cout << "----------------------------" << endl;
}
