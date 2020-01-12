#ifndef SRKTRACK_H_
#define SRKTRACK_H_

#include <array>
#include <vector>

#include "TFile.h"
#include "TTree.h"
#include "TString.h"
#include "TVector3.h"
#include "TVector2.h"
#include "TGeoShape.h"
#include "TGeoTube.h"
#include "TGeoMatrix.h"
#include "TGeoManager.h"
#include "TH1.h"

#include "SRKMotionState.h"

//enum class SRKPointType { TIMELIMIT, PERIODICSTOP, WALL, GASSCATTER, COUNT};

////////////////////////////////////////////////////////////////
/// class SRKMotionTracker
///
/// Tracks particle motion inside a geometry (i.e. chamber)
///
/// Author: Matthew Bales
///////////////////////////////////////////////////////////////
class SRKMotionTracker
{

public:
	SRKMotionTracker();
	virtual ~SRKMotionTracker();

	void makeTracks(int numTracksToAdd); /// Make tracks to a local tree
	void drawTrack(int trackID);  /// Draw a track to a TCanvas

	void makeStorageRing();  /// Make storage ring geometry (torus)
	bool getNextTrackingPoint(const SRKMotionState& stateIn, SRKMotionState& stateOut); // Gets the next tracking point (typically point of reflection) returns true if it's the last track

	TVector3 getRandomDirection();  /// Get's a random direction
	TVector3 getRandomPointInCrossSection();  /// Get's a randomly sampled point in the storage ring cross section
	void getRandomVelocityVectorAndPosition(SRKMotionState& stateOut);  /// gets a random velocity vector and position
	TVector3 getRandomVelocityVector();
	void getInitialState(SRKMotionState& stateOut); //If manualTracking, then sets state with defaults, otherwise gets it randomly

	void getNextTrackTreeEntry(SRKMotionState& stateOut, int& trackIDOut, bool& lastTrackOut);  /// Getting data from trackTree based on currentEntry

	inline const double getTimeLimit(){return timeLimit;}
	inline const double getRingRadius(){return ringRadius;}
	inline const double getStorageDiameter(){return storageDiameter;}
//	inline const double getMeanVel(){return meanVel;}
	inline const int getTrackTreeEntries(){return trackTree->GetEntries();}
	inline const TVector3 getInitialPos(){return initialState.pos;}
	inline const TVector3 getInitialVel(){return initialState.vel;}
	inline const bool isManualTracking(){return manualTracking;}
	inline const TString getVelProfHistPath(){return velProfHistPath;}
	inline const double getMass(){return mass;}
	inline const double getPeriodicStopTime(){return periodicStopTime;}

	inline void setTimeLimit(double inp){timeLimit=inp;}
//	inline void setMeanVel(double inp){meanVel=inp;}
	inline void setRingRadius(double inp){ringRadius=inp;}
	inline void setStorageDiameter(double inp){storageDiameter=inp;}
//	inline void setChamberRotation(double phi,double theta, double psi){chamberPhi=phi;chamberTheta=theta;chamberPsi=psi;}
	inline void setInitialPos(const TVector3& inp){ initialState.pos=inp;}
	inline void setInitialVel(const TVector3& inp){ initialState.vel=inp;}
	inline void setManualTracking(const bool inp){ manualTracking=inp;}
	inline void setVelProfHistPath(const TString inp){velProfHistPath=inp; loadVelProfHist();}
	inline void setMass(const double inp){mass=inp;}
	inline void setPeriodicStopTime(const double inp){periodicStopTime=inp;}

protected:

	double getTimeIntersectVecInCircle(TVector2 pos0, TVector2 vel0, double radius);  /// Determine based on velocity and position and radius when particle would reach boundary.
	bool loadVelProfHist();  /// Load the velocity profile histogram to sample from
	double getRandomVelFromProfHist();  /// Randomly sample the velProfHist
	void fillTrackTree();


	void makeTrack(int inpTrackID);  /// make a single track

	void makeTrackTree();  /// Set up the track tree

	TTree* trackTree; /// Track tree containing position and reflection information
	int currentEntry; /// Current entry being tracked
	int numTracks;   /// Number of tracks to simulate
	double timeLimit; /// Stop after a time limit
	bool useGravity;  /// Whether gravity is enabled (currently not)
	bool manualTracking; /// Instead of random

	//Particle
	double mass;
//	double meanVel;
	double temperature; /// Temperature of the particles (for Maxwell distributions)
	TH1* velProfHist;
	TString velProfHistPath; /// the file path for velProfHist.  Note that if it begins with "!"  the number following it will be used as temperature in Kelvin for a Maxwell distribution

	//Geometry
//	double chamberPhi;  /// Euler angle for chamber rotation
//	double chamberTheta; /// Euler angle for chamber rotation
//	double chamberPsi; /// Euler angle for chamber rotation
	double ringRadius;
	double storageDiameter;
	TGeoRotation theRotation; /// The chamber rotation
	TGeoManager theGeoManager;  /// ROOT based geomanager
	TGeoMaterial* vacMat;  /// Vaccuum material
	TGeoMedium* vacMed; /// Vacuum medium
	double safety;  /// A small distance unimportant for the physical problem to mitigate floating point errors
	double maxTrackSize;  /// Current implementation with TGeoManger requires a negative number

//	TFile* trackFile;

	//Tracking Variables
	SRKMotionState currentState;
	SRKMotionState initialState;
	int trackID; /// current track id number
	bool lastTrack; /// Whether this is the last track
	double periodicStopTime; /// Max time per track
	double nextPeriodicStop; // Last time a periodic stop was made

	//For branch addresses for trees...This is dumb, but necessary
	TVector3* posForBranch;
	TVector3* velForBranch;
};

#endif /* SRKTRACK_H_ */
