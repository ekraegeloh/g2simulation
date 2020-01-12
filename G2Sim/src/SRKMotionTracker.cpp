#include "SRKMotionTracker.h"

#include "TROOT.h"
#include "TMath.h"
#include "TRandom.h"
#include "TFile.h"
#include "TPolyLine3D.h"
#include "TView3D.h"
#include "TCanvas.h"
#include "TGeoTube.h"

#include <iostream>
#include <cfloat>
#include <vector>
#include <limits>
#include <cassert>

using namespace std;

//#define SRKTRACKDEBUG 1

SRKMotionTracker::SRKMotionTracker() :
	theGeoManager("theManager", "SRK Simulation Geometry")
{
	trackID = 0;
	trackTree = nullptr;
	useGravity = false;
	mass = 1.883531594e-28; //muon mass kg
//	meanVel = 193; //in m/s
//	chamberPhi = 0;
//	chamberTheta = 0;
//	chamberPsi = 0;
	ringRadius = 7.112;
	storageDiameter = 0.09;
	timeLimit = 100;
	numTracks = 0;
	posForBranch = new TVector3();
	velForBranch = new TVector3();
	lastTrack = false;
	currentEntry = 0;
	manualTracking = false;
//	defaultState.vel.SetXYZ(meanVel, 0, 0);
	velProfHistPath = "";
	velProfHist = nullptr;
	temperature = 0;
	theRotation.SetAngles(0, 0, 0);
	vacMat = new TGeoMaterial("Vacuum", 0, 0, 0); //Removed with GeoManager
	vacMed = new TGeoMedium("Vacuum", 1, vacMat); //Removed with GeoManager
	safety = 1e-6;
	maxTrackSize = -1000; //defined as negative number for proper normal vector determination
	periodicStopTime = numeric_limits<double>::max();
	nextPeriodicStop = numeric_limits<double>::max();

}

SRKMotionTracker::~SRKMotionTracker()
{
	gROOT->cd();
	delete posForBranch;
	delete velForBranch;
	delete velProfHist;

}

void SRKMotionTracker::makeStorageRing()
{
	theGeoManager.ClearPhysicalNodes(true);
	double maxDist = 2.01 * sqrt(pow(ringRadius + storageDiameter*0.5, 2) + pow(storageDiameter*0.5, 2));
	TGeoVolume* top = theGeoManager.MakeBox("world", vacMed, maxDist, maxDist, maxDist);

	TGeoRotation *r1 = new TGeoRotation();
	r1->SetAngles(0, 90, 0, 0, 270, 90); // all angles in degrees; is rotation around x axis so y is the vertical
	TGeoVolume* chamber = theGeoManager.MakeTorus("chamber", vacMed, ringRadius, ringRadius - storageDiameter*0.5, ringRadius + storageDiameter*0.5); //how oriented in xyz?

	theGeoManager.SetTopVolume(top);
	top->AddNode(chamber, 1, r1);
	cout << "Torus Geometry made with r: " << ringRadius << " d: " << storageDiameter << " with coordinate system rotated by 90 degrees around x. " << endl;
	theGeoManager.CloseGeometry();

}

void SRKMotionTracker::makeTrackTree()
{
	trackTree = new TTree("trackTree", "Initial points and points of reflection for particle in trap");
	trackTree->Branch("trackID", &trackID, "trackID/I");
	trackTree->Branch("lastTrack", &lastTrack, "lastTrack/O");
	trackTree->Branch("time", &currentState.time, "time/D");
	trackTree->Branch("pos", "TVector3", &posForBranch);
	trackTree->Branch("vel", "TVector3", &velForBranch);
}

void SRKMotionTracker::getNextTrackTreeEntry(SRKMotionState& stateOut, int& trackIDOut, bool& lastTrackOut)
{
	trackTree->GetEntry(currentEntry);

	stateOut.pos = *posForBranch;
	stateOut.vel = *velForBranch;

	trackIDOut = trackID;
	lastTrackOut = lastTrack;
	currentEntry++;
}

void SRKMotionTracker::fillTrackTree()
{
	*posForBranch = currentState.pos;
	*velForBranch = currentState.vel;
	trackTree->Fill();
}

void SRKMotionTracker::makeTracks(int numTracksToAdd)
{
	for (int i = 0; i < numTracksToAdd; i++)
	{

		makeTrack(i);
		numTracks++;
		if((i + 1) % 1000 == 0 || i + 1 == numTracksToAdd) cout << i + 1 << " tracks made." << endl;
	}
//	cout << "Average Reflections: " << (double) totalReflections / (double) numTracksToAdd << endl;
}

TVector3 SRKMotionTracker::getRandomVelocityVector()
{
	TVector3 velOut = getRandomDirection();

	if(velProfHist != nullptr)
	{
		velOut.SetMag(velProfHist->GetRandom());
	}
	else
	{
		//use maxwell distribution with the rest of the path being the temp
		if(temperature > 0)
		{
			double maxwellVariance = TMath::Sqrt(TMath::K() * temperature / mass);
			for (int i = 0; i < 3; i++)
			{
				velOut[i] = gRandom->Gaus(0, maxwellVariance);
			}
		}
		else
		{
			velOut.SetMag(0);//need to change that!
		}
	}
	return velOut;
}

void SRKMotionTracker::getRandomVelocityVectorAndPosition(SRKMotionState& stateOut)
{
	//Begin with random points
	stateOut.pos = getRandomPointInCrossSection(); //Eventually need to make this account for gravity based density
	stateOut.vel = getRandomVelocityVector();  //Also does not account for gravity
}

bool SRKMotionTracker::loadVelProfHist()
{
	delete velProfHist;
	velProfHist = nullptr;
	temperature = 0;
	if(velProfHistPath == "") return true;
	if(velProfHistPath[0] == '!')
	{
		TString temp = velProfHistPath;
		temp.Remove(0, 1);
		temperature = temp.Atof();
		cout << "Using Maxwell distribution at temperature " << temperature << endl;
		return true;
	}
	TFile velProfFile(velProfHistPath, "READ");
	if(velProfFile.IsZombie() || !velProfFile.IsOpen())
	{
		cout << "Error opening track file: " << velProfHistPath << endl;
		return false;
	}

	velProfHist = (TH1D*) velProfFile.Get("VelProfHist");
	velProfHist->SetDirectory(0);
	velProfFile.Close();

	cout << "Loaded velocity profile histogram, " << velProfHist->GetTitle() << ", from " << velProfHistPath << endl;

	return true;
}

void SRKMotionTracker::getInitialState(SRKMotionState& stateOut)
{
	if(!manualTracking)
	{
		getRandomVelocityVectorAndPosition(stateOut);
		stateOut.time = 0;
	}
	else
	{
		stateOut = initialState;
	}
	nextPeriodicStop = periodicStopTime;
	lastTrack = false;
}

void SRKMotionTracker::makeTrack(int inpTrackID)
{
	trackID = inpTrackID;
	getInitialState(currentState);

	fillTrackTree(); //Record initial point

#ifdef SRKTRACKDEBUG
	//Initial pos/vel
	cout << "___________________________________________________________________________________________" << endl;
	cout << "Initial Position:"<< endl;
	currentState.print();
#endif

	SRKMotionState stateOut;
	for (currentEntry = 0; !lastTrack; currentEntry++)
	{
		getNextTrackingPoint(currentState, stateOut);
		currentState = stateOut;
		fillTrackTree();
	}

	return;
}

bool SRKMotionTracker::getNextTrackingPoint(const SRKMotionState& stateIn, SRKMotionState& stateOut)
{
	lastTrack = false;
#ifdef SRKTRACKDEBUG
	cout << "Initial tracking: " << endl;
	stateIn.print();
#endif

	//This needs to change, need to actually integrate path
	stateOut.time = nextPeriodicStop;
	if(stateOut.time > timeLimit)
	{
		stateOut.time = timeLimit;
		lastTrack = true;
	}
	stateOut.pos = stateIn.pos + stateIn.vel * (stateOut.time - stateIn.time);
	stateOut.vel = stateIn.vel;
	nextPeriodicStop += periodicStopTime;

#ifdef SRKTRACKDEBUG
	cout << "After reflection: " << endl;
	stateOut.print();
	cout << "___________________________________________________________________________________________" << endl;
#endif

	return lastTrack;
}

//Returns time to point
double SRKMotionTracker::getTimeIntersectVecInCircle(TVector2 pos0, TVector2 vel0, double radius)
{
	TVector2 posTrans = pos0.Rotate(-vel0.Phi());

	posTrans.Set(sqrt(radius * radius - posTrans.Y() * posTrans.Y()), posTrans.Y());

	TVector2 posOut = posTrans.Rotate(vel0.Phi());

	double time = (posOut.X() - pos0.X()) / vel0.X(); //Could have used X or Y here.  Should be the same
	return time;
}

TVector3 SRKMotionTracker::getRandomDirection()
{
	double phi = gRandom->Rndm() * 2. * TMath::Pi();
	double cosTheta = gRandom->Rndm() * 2. - 1.;
	double sinTheta = sqrt(1. - cosTheta * cosTheta);
	return TVector3(sinTheta * cos(phi), sinTheta * sin(phi), cosTheta);
}

TVector3 SRKMotionTracker::getRandomPointInCrossSection()
{
	double r = sqrt(gRandom->Rndm()) * storageDiameter * 0.5;
	double theta = gRandom->Rndm() * 2. * TMath::Pi();

	//starting at x = r, z = 0 in ring
	return TVector3(ringRadius + r*cos(theta), r*sin(theta), 0);
}

void SRKMotionTracker::drawTrack(int trackIDToDraw)
{
	//First need to convert track to array
	TVector3* pos;
	int trackID;

	trackTree->SetBranchAddress("trackID", &trackID);
	trackTree->SetBranchAddress("pos", &pos);

	const int numEntries = trackTree->GetEntries();
	vector<double> xVector;
	vector<double> yVector;
	vector<double> zVector;

	bool found = false;
	for (int i = 0; i < numEntries; i++)
	{
		trackTree->GetEntry(i);
		if(trackIDToDraw == trackID)
		{
			xVector.push_back(pos->X());
			yVector.push_back(pos->Y());
			zVector.push_back(pos->Z());
			found = true;
		}
		else
		{
			if(found)
			{
				break;  //Break loop if we found them all (they should all be in a row)
			}
		}

	}

	TPolyLine3D* pl1 = new TPolyLine3D(xVector.size(), &xVector[0], &yVector[0], &zVector[0]);
	TCanvas* c1 = new TCanvas("c1");
	c1->cd();
	TView3D* view = (TView3D*) TView::CreateView(1);
	view->SetRange(-ringRadius-storageDiameter, -storageDiameter, -ringRadius-storageDiameter, ringRadius+storageDiameter, storageDiameter, ringRadius+storageDiameter);
	view->RotateView(0,90);//adjust to have diagonal view?

	pl1->SetLineColor(kBlue);
	pl1->Draw();
	theGeoManager.Draw("same");
}
