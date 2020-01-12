#ifndef SRKMANAGER_H_
#define SRKMANAGER_H_

#include "SRKMotionTracker.h"
#include "SRKSpinTracker.h"
#include "SRKGlobalField.h"
#include "SRKMotionTracker.h"
#include "SRKRunStats.h"
#include "SRKODEState.h"

#include <string>

#include "TString.h"
#include "TGraphErrors.h"
#include "TList.h"

//const TString SRKHISTSDIR = "/home/work/G2Out/hists/";
//const TString SRKGRAPHSDIR = "/home/work/G2Out/graphs/";
//const TString SRKTRACKSDIR = "/home/work/G2Out/tracks/";
//const TString SRKMACROSDIR = "/home/work/G2Out/macros/";


////////////////////////////////////////////////////////////////
/// SRKManager
///
/// A bit monolithic class which runs and manages the simulation
///
/// Author: Matthew Bales
///////////////////////////////////////////////////////////////

class SRKManager
{
public:
	SRKManager();
	~SRKManager();

	bool precessSpinsAlongTracks(int numTracks); /// Track/simulate/precess a number of tracks through the geometry

	//Getters
	inline const bool isRecordPeriodicSteps(){return recordPeriodicSteps;}
	inline const bool isRecordB(){return recordB;}
	inline const bool isRecordE(){return recordE;}
	inline const bool isUseAltStepping(){ return useAltStepping;}
	inline const int getRandomSeed(){ return randomSeed;}
	inline const double getB0FieldStrength(){ return b0FieldStrength;}
//	inline const double getE0FieldStrength(){ return e0FieldStrength;}
//	inline const double getBGradFieldStrength(){ return bGradFieldStrength;}
//	inline const double getEGradFieldStrength(){ return eGradFieldStrength;}
	//inline const double getPhaseMean(){return phaseMean;}
	//inline const double getPhaseError(){return phaseError;}
	//inline const double getPhiStart(){return phiStart;}
	//inline const double getThetaStart(){return thetaStart;}
	inline const double getOmega0(){return theSpinTracker.getGyromagneticRatio()*b0FieldStrength;}
//	inline const TVector3 getE0FieldDirection(){ return e0FieldDirection;}
	inline const TVector3 getB0FieldDirection(){ return b0FieldDirection;}
	inline const TString getResultsFilePath() { return resultsFilePath;}
	inline const TString getRunID() { return runID;}
	inline const TString getVelProfHistPath(){return theMotionTracker.getVelProfHistPath();}
	inline SRKMotionTracker* getSRKMotionTracker(){return &theMotionTracker;}
	inline SRKSpinTracker* getSRKSpinTracker(){return &theSpinTracker;}

	//Setters
	inline void setRecordPeriodicSteps(bool inp){recordPeriodicSteps=inp;}
	inline void setRecordB(bool inp){recordB=inp;}
	inline void setRecordE(bool inp){recordE=inp;}
	inline void setUseAltStepping(bool inp){useAltStepping=inp;}
	inline void setEcontinuous(bool inp){theGlobalField.Econt=inp;}
	inline void setRandomSeed(const int inp){ randomSeed=inp;} /// If set to non zero, same seed will be used every simulation run until set again
//	inline void setBFieldStrength(double inp){bFieldStrength=inp;}
//	inline void setEFieldStrength(double inp){eFieldStrength=inp;}
	inline void setB0FieldStrength(std::vector<double> inp){b0FieldStrength=inp[0];}
//	inline void setE0FieldStrength(double inp){e0FieldStrength=inp;}
	inline void setBQuadFieldStrength(double inp){bQuadFieldStrength=inp;}
	inline void setEQuadFieldStrength(double inp){eQuadFieldStrength=inp;}
	inline void addBMultiFieldStrength(std::vector<double> inp){B_multipole_coefs.push_back(inp);}
	inline void addBMultiOrder(int inp){B_multipole_ns.push_back(inp);}
	inline void addBMultiSkewness(bool inp){B_multipole_skews.push_back(inp);}
	inline void addEMultiFieldStrength(std::vector<double> inp){E_multipole_coefs.push_back(inp);}
	inline void addEMultiOrder(int inp){E_multipole_ns.push_back(inp);}
//	inline void setBQuadFieldDirection(const std::string tag, TVector3 inp){if(tag=="axis") bQuadAxisDirection=inp; else if(tag=="direction") bQuadDirection=inp;}
//	inline void setEQuadFieldDirection(const std::string tag, TVector3 inp){if(tag=="axis") eQuadAxisDirection=inp; else if(tag=="direction") eQuadDirection=inp;}
	//inline void setPhiStart(const double inp){phiStart=inp;}
	//inline void setThetaStart(const double inp){thetaStart=inp;}
//	inline void setBGradFieldStrength(double inp){bGradFieldStrength=inp;}
//	inline void setEGradFieldStrength(double inp){eGradFieldStrength=inp;}
	inline void setResultsFilePath(TString inp) {resultsFilePath=inp;}
	inline void setDefaultResultsDir(TString inp) {defaultResultsDir=inp; resultsFilePath = defaultResultsDir + runID+".root";}
	inline void setRunID(TString inp) {runID=inp; resultsFilePath = defaultResultsDir + runID+".root";}
//	inline void setE0FieldDirection(TVector3 inp){e0FieldDirection=inp;}
	inline void setB0FieldDirection(TVector3 inp){b0FieldDirection=inp;}
	inline void setPeriodicStopTime(const double inp){periodicStopTime=inp;}
	inline void setTimeLimit(const double inp){timeLimit=inp;}

	SRKGlobalField theGlobalField; /// Electric and magnetic field manager


protected:
	void createResultsFile(TString resultsFilePath); /// Create and leave open the root file which stores the tree of results and the macrofile used to create them
	void closeResultsFile();  /// Closes the result file
	void setInitialState(SRKODEState& initialState); /// Takes an initial motion state and loads it into variables used by resultsTree
	void setFinalState(SRKODEState& finalState); /// Takes a final motion state and loads it into variables used by resultsTree
	void setPeriodicStepState(SRKODEState& stepState, double* fieldState); /// Takes a step state and loads it only into variables used by stepTree
	void writeEvent();  /// Fills event tree
	void writeAllSpinSteps(std::vector<SRKODEState>* stepRecord, std::vector<double>* stepTimes);  /// Takes a vector of the results of each step and writes them to the tree
	void loadFields();  /// Load the electric and magnetic fields (could load ROOT files for interpolation)
	//SRKRunStats calcResultsFileStats(TString filePath, bool useWrapping); /// Calculates some statistics for run file
	double calcMag(TVector3 vec);
	double calcProjection(TVector3 vel, TVector3 M);
	bool fileExists(TString filePath);
	bool fileExistsAndNotZombie(TString strFileName);

	void precessSpinsAlongTracksDynamic(int numTracks);

	TVector3 pos0, pos, vel0, vel, M0, M, fieldB, fieldE; /// For recording
	double magM, magM0, proj, proj0;//phi0, phi, theta0, theta, /// For recording
	double magB, magE;
	//Float_t sxProb; /// For recording

	//double phiStart,thetaStart;
	double time0, time; /// Initial time and current/final time
	int trackID;  /// Current track/event
	int randomSeed;

	bool recordPeriodicSteps; /// Should all steps of the simulation be recorded to the results file
	bool recordB;
	bool recordE;
	double periodicStopTime;
	double timeLimit;

	bool useAltStepping; /// Whether to use an alternate stepping method

	TString resultsFilePath; /// Path for results of simulation
	TString defaultResultsDir; /// The default directory results are stored in
	TString runID; /// An identifying number/string for the simulation run

	TFile* resultsFile; /// File where results are stored
	TTree* resultsTree; /// Tree where results are stored

	//double phaseMean, phaseError,stdevOut, thetaMean;
	//double deltaPhaseMean,deltaPhaseError;

//?	SRKGlobalField theGlobalField; /// Electric and magnetic field manager
	SRKSpinTracker theSpinTracker; /// Tracks spins along a path
	SRKMotionTracker theMotionTracker; /// Creates motion tracks in a geometry

	/// Magnetic field settings that eventually should be moved to SRKGlobalField
//	double bFieldStrength;
//	double eFieldStrength;
	double b0FieldStrength;  /// Constant magnetic field
//	double e0FieldStrength;  /// Constant electric field
	TVector3 b0FieldDirection; /// Direction for b0Field
//	TVector3 e0FieldDirection; /// direction for e0Field
//	double bGradFieldStrength; /// Strength of magnetic gradient field (z direction only for now)
//	double eGradFieldStrength; /// Strength of electric gradient field (z direction only for now)
	double bQuadFieldStrength; //Strength of magnetic quadrupole field
	double eQuadFieldStrength; //Strength of electric quadrupole field
//	TVector3 bQuadAxisDirection; //Direction of axis of radial symmetric magnetic quadrupole field
//	TVector3 eQuadAxisDirection; //Direction of axis of radial symmetric electric quadrupole field
//	TVector3 bQuadDirection; //Primary direction of magnetic quadrupole field
//	TVector3 eQuadDirection; //Primary direction of electric quadrupole field
	std::vector<std::vector<double>> B_multipole_coefs;
	std::vector<int> B_multipole_ns;
	std::vector<bool> B_multipole_skews;
	std::vector<std::vector<double>> E_multipole_coefs;
	std::vector<int> E_multipole_ns;

	std::vector<SRKODEState>* stepRecord = nullptr; /// Used for recording all steps in the spin tracker (not used atm)
	std::vector<double>* stepTimes = nullptr; /// Used for recording all steps in the spin tracker (not used atm)

};

#endif /* SRKMANAGER_H_ */
