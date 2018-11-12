#include "SRKManager.h"

#include <iostream>
#include <time.h>
#include <iomanip>
#include <string>

#include "TMath.h"
#include "TCanvas.h"
#include "TF1.h"
#include "TRandom.h"

#include "SRKGraphics.h"
#include "SRKMacroManager.h"
#include "SRKODEState.h"
#include "SRKMotionState.h"

//#define SRKMANAGERDEBUG 1
//#define SRKMANAGERMOTIONDEBUG 1

using namespace std;

static const int EVENT_STDOUT_ANNOUNCE_RATE = 100;

SRKManager::SRKManager()
{
	resultsFile = nullptr;
	resultsTree = nullptr;
	stepTree = nullptr;
	useDynamicTracking = true;
	recordPeriodicSteps = false;
	useAltStepping = false;
	parallelFields = true;
	theSpinTracker.setGlobalField(&theGlobalField);
	bFieldStrength = 0;
	eFieldStrength = 0;
	b0FieldStrength = 0;
	e0FieldStrength = 0;
	bGradFieldStrength = 0;
	eGradFieldStrength = 0;
	bQuadFieldStrength = 0;
	eQuadFieldStrength = 0;
	bSextFieldStrength = 0;
	eSextFieldStrength = 0;
	bSextAxisDirection.SetXYZ(0, 0, -1);
	eSextAxisDirection.SetXYZ(0, 0, -1);
	bSextDirection.SetXYZ(1, 0, 0);
	eSextDirection.SetXYZ(1, 0, 0);
	dipoleFieldStrength = 0;
	//sxProb=0;
	dipolePosition.SetXYZ(0, 0, 0);
	dipoleDirection.SetXYZ(0, 1, 0);
	e0FieldDirection.SetXYZ(0, 1, 0);
	b0FieldDirection.SetXYZ(0, 1, 0);
	//deltaPhaseMean = deltaPhaseError = phaseMean = phaseError =stdevOut= thetaMean = phi0 = phi = theta0 = theta = time = time0 = 0.;
	time = time0 = magM = magM0 = proj = proj0 = 0.;
	trackID = 0;
	trackFilePath = "!dynamic";
	defaultResultsDir = ""; //only when resultsFilePath is not appropriate
	resultsFilePath = defaultResultsDir + "srk_out.root";
	runID = "RIDX";
	randomSeed = 0;
	//phiStart = 0; //For input via macros
	//thetaStart = 0;
}

SRKManager::~SRKManager()
{
	if(resultsFile != nullptr)
	{
		if(resultsFile->IsOpen())
		{
			closeResultsFile();
		}
		delete resultsFile;
	}
}

void SRKManager::createResultsFile(TString resultsFilePath)
{
	if(resultsFile != nullptr && resultsFile->IsOpen())
	{
		resultsFile->Close();
	}
	resultsFile = new TFile(resultsFilePath, "RECREATE");

	if(resultsFile->IsZombie() || !resultsFile->IsOpen())
	{
		cout << "Error opening results file: " << resultsFilePath << endl;
		return;

	}
	resultsFile->cd();
	resultsTree = new TTree("hitTree", "Initial and final states after reflections and spin tracking");
	resultsTree->Branch("trackID", &trackID, "trackID/I");
	resultsTree->Branch("time0", &time0, "time0/D");
	resultsTree->Branch("pos0", &pos0);
	resultsTree->Branch("vel0", &vel0);
	resultsTree->Branch("M0", &M0);
	resultsTree->Branch("magM0", &magM0, "magM0/D");
	resultsTree->Branch("projection0", &proj0, "proj0/D");
	//resultsTree->Branch("phi0", &phi0, "phi0/D");
	//resultsTree->Branch("theta0", &theta0, "theta0/D");
	resultsTree->Branch("time", &time, "time/D");
	resultsTree->Branch("pos", &pos);
	resultsTree->Branch("vel", &vel);
	resultsTree->Branch("M", &M);
	resultsTree->Branch("magM", &magM, "magM/D");
	resultsTree->Branch("projection", &proj, "proj/D");
	//resultsTree->Branch("phi", &phi, "phi/D");
	//resultsTree->Branch("theta", &theta, "theta/D");

	stepTree = new TTree("stepTree", "Record of each step");
	if(recordPeriodicSteps)
	{
		//stepTree->Branch("sxProb", &sxProb, "sxProb/F");
		stepTree->Branch("pos", &pos);
		stepTree->Branch("vel", &vel);
		stepTree->Branch("M", &M);
		stepTree->Branch("magM", &magM, "magM/D");
		stepTree->Branch("projection", &proj, "proj/D");
		//stepTree->Branch("phi", &phi, "phi/D");
		//stepTree->Branch("theta", &theta, "theta/D");
//		stepTree->Branch("BFieldStrenth", &fieldB);
	}

}

void SRKManager::closeResultsFile()
{

	TList* userInfoList = resultsTree->GetUserInfo(); //Every TTree has a list that you can add TObjects to

	userInfoList->Add(new TNamed("RecordAllSteps", Form("%i", (int) isRecordPeriodicSteps())));
	userInfoList->Add(new TNamed("UseAltStepping", Form("%i", (int) isUseAltStepping())));
	userInfoList->Add(new TNamed("ParallelFields", Form("%i", (int) isParallelFields())));
	userInfoList->Add(new TNamed("ConstStepper", Form("%i", (int) theSpinTracker.isConstStepper())));
	userInfoList->Add(new TNamed("Use2D", Form("%i", (int) theMotionTracker.isUse2D())));
	userInfoList->Add(new TNamed("ManualTracking", Form("%i", (int) theMotionTracker.isManualTracking())));
	userInfoList->Add(new TNamed("B0FieldStrength", Form("%e", getB0FieldStrength())));
	userInfoList->Add(new TNamed("AdditionalRandomVelZ", Form("%e", theMotionTracker.getAdditionalRandomVelZ())));
	userInfoList->Add(new TNamed("E0FieldStrength", Form("%e", getE0FieldStrength())));
	userInfoList->Add(new TNamed("BGradFieldStrength", Form("%e", getBGradFieldStrength())));
	userInfoList->Add(new TNamed("EGradFieldStrength", Form("%e", getEGradFieldStrength())));
	userInfoList->Add(new TNamed("DipoleFieldStrength", Form("%e", getDipoleFieldStrength())));
	userInfoList->Add(new TNamed(TString("TrackFilePath"), getTrackFilePath()));
	userInfoList->Add(new TNamed(TString("ResultsFilePath"), getResultsFilePath()));
	userInfoList->Add(new TNamed(TString("VelProfHistPath"), getVelProfHistPath()));
	userInfoList->Add(new TNamed(TString("RunID"), getRunID()));
	userInfoList->Add(new TNamed("GyromagneticRatio", Form("%e", theSpinTracker.getGyromagneticRatio())));
	userInfoList->Add(new TNamed("Mass", Form("%e", theMotionTracker.getMass())));
	userInfoList->Add(new TNamed("MeanFreePath", Form("%e", theMotionTracker.getMeanFreePath())));
	userInfoList->Add(new TNamed("StepsTaken", Form("%i", theSpinTracker.getStepsTaken())));
	userInfoList->Add(new TNamed("TimeLimit", Form("%e", theMotionTracker.getTimeLimit())));
	userInfoList->Add(new TNamed("EPSAbs", Form("%e", theSpinTracker.getEPSAbs())));
	userInfoList->Add(new TNamed("EPSRel", Form("%e", theSpinTracker.getEPSRel())));
	userInfoList->Add(new TNamed("DiffuseReflectionProb", Form("%e", theMotionTracker.getDiffuseReflectionProb())));
	userInfoList->Add(new TNamed("ChamberRadius", Form("%e", theMotionTracker.getChamberRadius())));
	userInfoList->Add(new TNamed("ChamberHeight", Form("%e", theMotionTracker.getChamberHeight())));
	userInfoList->Add(new TNamed("MeanVel", Form("%e", theMotionTracker.getMeanVel())));
	userInfoList->Add(new TNamed("ReflectionLimit", Form("%i", theMotionTracker.getReflectionLimit())));
	userInfoList->Add(new TNamed("DefaultPos", Form("%f %f %f", theMotionTracker.getDefaultPos().X(), theMotionTracker.getDefaultPos().Y(), theMotionTracker.getDefaultPos().Z())));
	userInfoList->Add(new TNamed("DefaultVel", Form("%f %f %f", theMotionTracker.getDefaultVel().X(), theMotionTracker.getDefaultVel().Y(), theMotionTracker.getDefaultVel().Z())));
	userInfoList->Add(new TNamed("DipolePosition", Form("%f %f %f", getDipolePosition().X(), getDipolePosition().Y(), getDipolePosition().Z())));
	userInfoList->Add(new TNamed("DipoleDirection", Form("%f %f %f", getDipoleDirection().X(), getDipoleDirection().Y(), getDipoleDirection().Z())));
	userInfoList->Add(new TNamed("E0FieldDirection", Form("%f %f %f", getE0FieldDirection().X(), getE0FieldDirection().Y(), getE0FieldDirection().Z())));
	userInfoList->Add(new TNamed("B0FieldDirection", Form("%f %f %f", getB0FieldDirection().X(), getB0FieldDirection().Y(), getB0FieldDirection().Z())));
	userInfoList->Add(new TNamed("PeriodicStopTime", Form("%f", theMotionTracker.getPeriodicStopTime())));

	resultsFile->Write("", TObject::kOverwrite);

	resultsFile->Close();
	delete resultsFile;
	resultsFile = nullptr;
	resultsTree = nullptr;
}

void SRKManager::loadParametersFromResultsFile(TString filePath)
{
	TFile theFile(filePath, "READ");
	TTree* theTree = (TTree*) theFile.Get("hitTree");
	TList* userInfoList = theTree->GetUserInfo();
	SRKMacroManager tempManager(this);

	for (int i = 0; i < userInfoList->GetEntries(); i++)
	{
		TNamed* par = (TNamed*) userInfoList->At(i);
		string command = string("set") + par->GetName();
		string value = par->GetTitle();
		if(command != "setStepsTaken")
		{
			cout << "SRK: " << command << " " << value << endl;
			tempManager.runMacroCommand(command, value);
		}
	}
	theFile.Close();
}

void SRKManager::writeEvent()
{
	magM0 = calcMag(M0);
	magM = calcMag(M);
	proj0 = calcProjection(vel0, M0);
	proj = calcProjection(vel, M);
	resultsTree->Fill();
}

void SRKManager::writePeriodicStep()
{
	magM = calcMag(M);
	proj = calcProjection(vel, M);
	stepTree->Fill();
}

void SRKManager::writeAllSpinSteps(std::vector<SRKODEState>* stepRecord, std::vector<double>* stepTimes)
{
	for (unsigned int i = 0; i < stepRecord->size(); i++)
	{
		(stepRecord->at(i))[10] = stepTimes->at(i); //we record the time in the last slot
		setFinalState(stepRecord->at(i));
		writeEvent();
	}

}

void SRKManager::makeTracks(int numTracks)
{
	theMotionTracker.makeTracks(numTracks, trackFilePath);
}

bool SRKManager::precessSpinsAlongTracks(int numTracks)
{
	clock_t t1, t2;
	t1 = clock();

	if(randomSeed != 0)
	{
		gRandom->SetSeed(randomSeed);
	}
	cout << "Using random seed: " << gRandom->GetSeed() << endl;
	loadFields();

	createResultsFile(resultsFilePath);

	if(useDynamicTracking)
	{
		precessSpinsAlongTracksDynamic(numTracks);
	}
	else
	{
		precessSpinsAlongTracksWithTrackFile(numTracks);
	}

	closeResultsFile();
	//phaseMean = makeMeanPhasePlot(resultsFilePath, "", true, phaseError,stdevOut); //Prints mean and stdev, no plot
	//thetaMean = theta; //for now, only one particle so mean = theta

	cout << "-----------------" << endl;
	cout << "For file: " << resultsFilePath << "    Number of particles measured: " << numTracks << endl;
	//cout << "Mean Phase: " << setprecision(15) << phaseMean << " +/- " << phaseError << endl;
	//cout << "StDeV: " << setprecision(15) << stdevOut << " +/- " << phaseError << endl;
	//cout << "Mean Theta: " << setprecision(15) << thetaMean << endl;
	cout << "-----------------" << endl;

	theMotionTracker.closeTrackFile();

	if(stepRecord != nullptr)
	{
		delete stepRecord;
	}
	if(stepTimes != nullptr)
	{
		delete stepTimes;
	}

	t2 = clock();

	double diff = ((double) t2 - (double) t1) / (double) CLOCKS_PER_SEC;
	cout << "Computation time: " << fixed << setprecision(3) << diff << "   Tracks per Second: " << (double) numTracks / diff << "   Tracks per hour: " << (float) numTracks * 3600 / diff << endl;
	cout.unsetf(ios_base::floatfield);

	return true;

}

void SRKManager::precessSpinsAlongTracksDynamic(int numTracks)
{
	theMotionTracker.makeCylinderGeometry();
	bool lastTrack = false;

	SRKODEState theODEState(10);
//	SRKODEState initialState(9);
	double theODEField[6];

	SRKMotionState currentMotionState, motionStateOut;
	SRKSpinState initialSpinState;
#ifdef SRKMANAGERMOTIONDEBUG
			//Initial pos/vel
			cout << "___________________________________________________________________________________________" << endl;
			cout << "currentMotionState pos:"<< endl;
			currentMotionState.pos.Print();
			cout << "currentMotionState vel:"<< endl;
			currentMotionState.vel.Print();
#endif
	for (int i = 0; i < numTracks; i++)  //Track loop
	{
		theMotionTracker.getInitialState(currentMotionState);
#ifdef SRKMANAGERMOTIONDEBUG
			//Initial pos/vel
			cout << "___________________________________________________________________________________________" << endl;
			cout << "currentMotionState pos:"<< endl;
			currentMotionState.pos.Print();
			cout << "currentMotionState vel:"<< endl;
			currentMotionState.vel.Print();
#endif

		trackID = i;

		updateMotionStatePosVel(theODEState, currentMotionState);
#ifdef SRKMANAGERMOTIONDEBUG
			//Initial pos/vel
			cout << "___________________________________________________________________________________________" << endl;
			cout << "currentMotionState pos:"<< endl;
			currentMotionState.pos.Print();
			cout << "currentMotionState vel:"<< endl;
			currentMotionState.vel.Print();
#endif

		theSpinTracker.getInitialState(initialSpinState);
		updateMotionStateMag(theODEState, initialSpinState);
		//theODEState[6] = phiStart; //Phi
		//theODEState[7] = thetaStart; //Theta
		setInitialState(theODEState);
#ifdef SRKMANAGERDEBUG
		//Initial pos/vel
		cout << "___________________________________________________________________________________________" << endl;
		cout << "Initial Position:"<< endl;
		printMotionState(theODEState);
#endif

		if(i % EVENT_STDOUT_ANNOUNCE_RATE == 0) cout << "Spinning track: " << trackID << endl;

		//Reflection point loop
		do
		{

			lastTrack = theMotionTracker.getNextTrackingPoint(currentMotionState, motionStateOut);
			if(motionStateOut.type == SRKStepPointType::DEPOLARIZED)  //If depolarized by the next step, no need to spin track the step
			{
				//For now we'll continue to track it.  Some concern that theta can approach Pi/2 with this method
				theODEState[6] = gRandom->Rndm() * 2. * TMath::Pi(); //Phi
				double cosThetaRandom=gRandom->Rndm() * 2. - 1.;
				theODEState[7] = acos(cosThetaRandom);//Theta
				if(cosThetaRandom < 0)
				{
					theODEState[7]=-theODEState[7];
				}

			}
			else if(useAltStepping)
			{
				//theSpinTracker.trackSpinAltA(theODEState, motionStateOut.time - static_cast<double>(theODEState[9]), stepRecord, stepTimes); //Runge Kutta on Phi and Theta up to currentTime
			}
			else
			{
				theSpinTracker.trackSpin(theODEState, theODEField, motionStateOut.time - static_cast<double>(theODEState[9]), stepRecord, stepTimes); //Runge Kutta on Phi and Theta up to currentTime
			}
#ifdef SRKMANAGERMOTIONDEBUG
			//Initial pos/vel
			cout << "___________________________________________________________________________________________" << endl;
			cout << "currentMotionState pos:"<< endl;
			currentMotionState.pos.Print();
			cout << "currentMotionState vel:"<< endl;
			currentMotionState.vel.Print();
#endif
//			updateMotionStatePosVel(theODEState, motionStateOut); //Use the next reflection point for next step
			currentMotionState = motionStateOut;
#ifdef SRKMANAGERMOTIONDEBUG
			//Initial pos/vel
			cout << "___________________________________________________________________________________________" << endl;
			cout << "currentMotionState pos:"<< endl;
			currentMotionState.pos.Print();
			cout << "currentMotionState vel:"<< endl;
			currentMotionState.vel.Print();
#endif
#ifdef SRKMANAGERDEBUG
			//Initial pos/vel
			cout << "___________________________________________________________________________________________" << endl;
			cout << "State after step:"<< endl;
			printMotionState(theODEState);
#endif

			if(lastTrack) //Record at last point
			{
				setFinalState(theODEState);
#ifdef SRKMANAGERDEBUG
				//Initial pos/vel
				cout << "___________________________________________________________________________________________" << endl;
				cout << "FinalState:"<< endl;
				printMotionState(theODEState);
#endif
				writeEvent(); //Write the final state only

			}



			if(recordPeriodicSteps && motionStateOut.type == SRKStepPointType::PERIODICSTOP) //Record at last point
			{
				setPeriodicStepState(theODEState);//, theODEField);
#ifdef SRKMANAGERDEBUG
				//Initial pos/vel
				cout << "___________________________________________________________________________________________" << endl;
				cout << "StepState:"<< endl;
				printMotionState(theODEState);
#endif
				writePeriodicStep(); //Write the final state only
			}
		} while (!lastTrack);
	}
}
void SRKManager::precessSpinsAlongTracksWithTrackFile(int numTracks)
{
	theMotionTracker.loadTrackFile(trackFilePath);
	bool lastTrack = false;

	SRKODEState theState(10);
	SRKODEState initialState(10);
	double theODEField[6];

	SRKMotionState currentMotionState;
#ifdef SRKMANAGERMOTIONDEBUG
			//Initial pos/vel
			cout << "___________________________________________________________________________________________" << endl;
			cout << "currentMotionState pos:"<< endl;
			currentMotionState.pos.Print();
			cout << "currentMotionState vel:"<< endl;
			currentMotionState.vel.Print();
#endif
	for (int i = 0; i < numTracks; i++)  //Track loop
	{
		theMotionTracker.getNextTrackTreeEntry(currentMotionState, trackID, lastTrack);

		updateMotionStatePosVel(theState, currentMotionState);
#ifdef SRKMANAGERMOTIONDEBUG
			//Initial pos/vel
			cout << "___________________________________________________________________________________________" << endl;
			cout << "currentMotionState pos:"<< endl;
			currentMotionState.pos.Print();
			cout << "currentMotionState vel:"<< endl;
			currentMotionState.vel.Print();
#endif

		//theState[6] = phiStart; //Phi
		//theState[7] = thetaStart; //Theta
		setInitialState(theState);

		if(i % EVENT_STDOUT_ANNOUNCE_RATE == 0) cout << "Spinning track: " << trackID << endl;

		//Reflection point loop
		do
		{

			theMotionTracker.getNextTrackTreeEntry(currentMotionState, trackID, lastTrack);

			if(useAltStepping)
			{
				//theSpinTracker.trackSpinAltA(theState, currentMotionState.time - static_cast<double>(theState[9]), stepRecord, stepTimes); //Runge Kutta on Phi and Theta up to currentTime
			}
			else
			{
				theSpinTracker.trackSpin(theState, theODEField, currentMotionState.time - static_cast<double>(theState[9]), stepRecord, stepTimes); //Runge Kutta on Phi and Theta up to currentTime
			}
//			updateMotionStatePosVel(theState, currentMotionState); //Use the next reflection point for next step
#ifdef SRKMANAGERMOTIONDEBUG
			//Initial pos/vel
			cout << "___________________________________________________________________________________________" << endl;
			cout << "currentMotionState pos:"<< endl;
			currentMotionState.pos.Print();
			cout << "currentMotionState vel:"<< endl;
			currentMotionState.vel.Print();
#endif

			if(lastTrack) //Record at last point
			{

				setFinalState(theState);
#ifdef SRKMANAGERDEBUG
				//Initial pos/vel
				cout << "___________________________________________________________________________________________" << endl;
				cout << "FinalState:"<< endl;
				printMotionState(theState);
#endif
				writeEvent(); //Write the final state only

			}

			if(recordPeriodicSteps && currentMotionState.type == SRKStepPointType::PERIODICSTOP) //Record at last point
			{
				setPeriodicStepState(theState);//, theODEField);
#ifdef SRKMANAGERDEBUG
				//Initial pos/vel
				cout << "___________________________________________________________________________________________" << endl;
				cout << "StepState:"<< endl;
				printMotionState(theState);
#endif
				writePeriodicStep(); //Write the final state only
			}
		} while (!lastTrack);
	}
	theMotionTracker.closeTrackFile();
}

void SRKManager::setInitialState(SRKODEState& initialState)
{
	//cout << "-----------Initial------------" << endl;
	//printMotionState(initialState);

	pos0.SetXYZ(static_cast<double>(initialState[0]), static_cast<double>(initialState[1]), static_cast<double>(initialState[2]));
	vel0.SetXYZ(static_cast<double>(initialState[3]), static_cast<double>(initialState[4]), static_cast<double>(initialState[5]));
	M0.SetXYZ(static_cast<double>(initialState[6]), static_cast<double>(initialState[7]), static_cast<double>(initialState[8]));
	time0 = static_cast<double>(initialState[9]);
}

void SRKManager::setFinalState(SRKODEState& finalState)
{
	//cout << "-----------Final------------" << endl;
	//printMotionState(finalState);

	pos.SetXYZ(static_cast<double>(finalState[0]), static_cast<double>(finalState[1]), static_cast<double>(finalState[2]));
	vel.SetXYZ(static_cast<double>(finalState[3]), static_cast<double>(finalState[4]), static_cast<double>(finalState[5]));
	M.SetXYZ(static_cast<double>(finalState[6]), static_cast<double>(finalState[7]), static_cast<double>(finalState[8]));
	time = static_cast<double>(finalState[9]);

}

void SRKManager::setPeriodicStepState(SRKODEState& stepState)//, double* fieldState)
{
	//cout << "-----------Final------------" << endl;
	//printMotionState(finalState);

	pos.SetXYZ(static_cast<double>(stepState[0]), static_cast<double>(stepState[1]), static_cast<double>(stepState[2]));
	vel.SetXYZ(static_cast<double>(stepState[3]), static_cast<double>(stepState[4]), static_cast<double>(stepState[5]));
	M.SetXYZ(static_cast<double>(stepState[6]), static_cast<double>(stepState[7]), static_cast<double>(stepState[8]));
	time = static_cast<double>(stepState[9]);
	//fieldB.SetXYZ(fieldState[0], fieldState[1], fieldState[2]);

}

void SRKManager::loadFields()
{
	theGlobalField.setCurrentFieldSettingsToModify(0); //B Field
	theGlobalField.setFieldClass(FIELDCLASS_B);
	theGlobalField.setFieldScalingValue(bFieldStrength);

	theGlobalField.setCurrentFieldSettingsToModify(1); //E Field
	theGlobalField.setFieldType(FIELD_ELECTRIC);
	theGlobalField.setFieldClass(FIELDCLASS_E);
	theGlobalField.setFieldScalingValue(eFieldStrength);
/*	theGlobalField.setCurrentFieldSettingsToModify(0); //B0 Field
	theGlobalField.setFieldScalingValue(b0FieldStrength);
	theGlobalField.setFieldDirection(b0FieldDirection);

	theGlobalField.setCurrentFieldSettingsToModify(1); //E0 Field
	theGlobalField.setFieldType(FIELD_ELECTRIC);
	theGlobalField.setFieldDirection(e0FieldDirection);

	if(parallelFields)
	{
		theGlobalField.setFieldScalingValue(e0FieldStrength);
	}
	else
	{
		theGlobalField.setFieldScalingValue(-e0FieldStrength);
	}

	theGlobalField.setCurrentFieldSettingsToModify(2); //B Grad Field
	theGlobalField.setFieldClass(FIELDCLASS_GRADIENT);
	theGlobalField.setFieldScalingValue(bGradFieldStrength);
	theGlobalField.setFieldDirection(TVector3(0, 0, 1));

	theGlobalField.setCurrentFieldSettingsToModify(3); //B Dipole
	theGlobalField.setFieldClass(FIELDCLASS_DIPOLE);
	theGlobalField.setFieldScalingValue(dipoleFieldStrength);
	theGlobalField.setFieldMoment(dipoleDirection);
	theGlobalField.setFieldCenterPos(dipolePosition);

	theGlobalField.setCurrentFieldSettingsToModify(4); //E Grad Field
	theGlobalField.setFieldType(FIELD_ELECTRIC);
	theGlobalField.setFieldClass(FIELDCLASS_GRADIENT);
	theGlobalField.setFieldScalingValue(eGradFieldStrength);
	theGlobalField.setFieldDirection(TVector3(0, 0, 1));

	theGlobalField.setCurrentFieldSettingsToModify(5); //B Quadrupole Field
	theGlobalField.setFieldType(FIELD_MAGNETIC);
	theGlobalField.setFieldClass(FIELDCLASS_QUADRUPOLE);
	theGlobalField.setFieldScalingValue(bQuadFieldStrength);
	theGlobalField.setFieldDirection(bQuadDirection);
	theGlobalField.setFieldAxis(bQuadAxisDirection);

	theGlobalField.setCurrentFieldSettingsToModify(6); //E Quadrupole Field
	theGlobalField.setFieldType(FIELD_ELECTRIC);
	theGlobalField.setFieldClass(FIELDCLASS_QUADRUPOLE);
	theGlobalField.setFieldScalingValue(eQuadFieldStrength);
	theGlobalField.setFieldDirection(eQuadDirection);
	theGlobalField.setFieldAxis(eQuadAxisDirection);

	theGlobalField.setCurrentFieldSettingsToModify(7); //B Sextupole Field
	theGlobalField.setFieldType(FIELD_MAGNETIC);
	theGlobalField.setFieldClass(FIELDCLASS_SEXTUPOLE);
	theGlobalField.setFieldScalingValue(bSextFieldStrength);
	theGlobalField.setFieldDirection(bSextDirection);
	theGlobalField.setFieldAxis(bSextAxisDirection);

	theGlobalField.setCurrentFieldSettingsToModify(8); //E Sextupole Field
	theGlobalField.setFieldType(FIELD_ELECTRIC);
	theGlobalField.setFieldClass(FIELDCLASS_SEXTUPOLE);
	theGlobalField.setFieldScalingValue(eSextFieldStrength);
	theGlobalField.setFieldDirection(eSextDirection);
	theGlobalField.setFieldAxis(eSextAxisDirection);*/

	theGlobalField.updateField();

}

/*void SRKManager::calcDeltaPhaseMean(TString inpRunID)
{
	double parMean, parError, antiMean, antiError;
	double parSTDevOut,antiSTDevOut;
	parMean = makeMeanPhasePlot(defaultResultsDir + "Results_" + inpRunID + "_P.root", "", true, parError,parSTDevOut); //Prints mean and stdev, no plot
	antiMean = makeMeanPhasePlot(defaultResultsDir + "Results_" + inpRunID + "_A.root", "", true, antiError,antiSTDevOut); //Prints mean and stdev, no plot
	deltaPhaseMean = parMean - antiMean;
	deltaPhaseError = sqrt(parError * parError + antiError * antiError);

}*/

/*SRKRunStats SRKManager::calcResultsFileStats(TString filePath, bool useWrapping)
{
	SRKRunStats theStats;

	TFile* rootFile = new TFile(filePath);

	double phi, theta;
	TTree* theTree = (TTree*) rootFile->Get("hitTree");
	theTree->SetMakeClass(1);
	theTree->SetBranchAddress("phi", &(phi));
	theTree->SetBranchAddress("theta", &(theta));

	gROOT->cd(0);

	theStats.numEvents = theTree->GetEntries();

	vector<double> phiVec;
	vector<double> thetaVec;

	for (int i = 0; i < theStats.numEvents; i++)
	{

		theTree->GetEntry(i, 1);

		phiVec.push_back(phi);
		thetaVec.push_back(theta);

	}
	rootFile->Close();
	delete rootFile;

	if(useWrapping)
	{
		theStats.phiMean = reducePeriodicToMeanInVector(phiVec);
		theStats.thetaMean = reducePeriodicToMeanInVector(thetaVec);
	}
	else
	{
		theStats.phiMean = carefulMeanVector(phiVec);
		theStats.thetaMean = carefulMeanVector(thetaVec);
	}

	theStats.phiStDev = carefullStDevVector(phiVec, true);
	theStats.thetaStDev = carefullStDevVector(thetaVec, true);

	theStats.phiError = theStats.phiStDev / sqrt((double) theStats.numEvents);
	theStats.thetaError = theStats.thetaStDev / sqrt((double) theStats.numEvents);

	TH1D phiHist("phiHist", "phiHist", 10000, theStats.phiMean - theStats.phiStDev * 5., theStats.phiMean + theStats.phiStDev * 5.);
	TH1D thetaHist("thetaHist", "thetaHist", 10000, -theStats.thetaStDev * 5., theStats.thetaStDev * 5.);

	for (int i = 0; i < theStats.numEvents; ++i)
	{
		phiHist.Fill(phiVec[i]);
		thetaHist.Fill(thetaVec[i]);

		theStats.sXDetProb += calculateSxDetectionProbability(phiVec[i] - theStats.phiMean, thetaVec[i]);
	}

	theStats.sXDetProb /= theStats.numEvents;

	theStats.phiKurtosis = phiHist.GetKurtosis(1);
	theStats.phiKurtosisError = phiHist.GetKurtosis(11);
	theStats.phiSkewness = phiHist.GetSkewness(1);
	theStats.phiSkewnessError = phiHist.GetSkewness(11);

	theStats.thetaKurtosis = thetaHist.GetKurtosis(1);
	theStats.thetaKurtosisError = thetaHist.GetKurtosis(11);
	theStats.thetaSkewness = thetaHist.GetSkewness(1);
	theStats.thetaSkewnessError = thetaHist.GetSkewness(11);

	TF1 phiTsallisFunc("phiTsallisFunc", "[0]/pow(1+((x-[3])/[1])*((x-[3])/[1]),[2])", theStats.phiMean - TMath::Pi(), theStats.phiMean + TMath::Pi());
	phiTsallisFunc.SetParNames("Amplitude", "Sigma", "Power", "Mean");
	phiTsallisFunc.SetParameters(phiHist.GetMaximum() * 5, theStats.phiStDev * 0.005, .76, theStats.phiMean);
	phiTsallisFunc.SetParLimits(3, theStats.phiMean, theStats.phiMean);
	phiHist.Fit(&phiTsallisFunc, "VMR+");
	theStats.phiTsallisPower = phiTsallisFunc.GetParameter(2);
	theStats.phiTsallisPowerError = phiTsallisFunc.GetParError(2);

	TF1 thetaTsallisFunc("thetaTsallisFunc", "[0]/pow(1+((x-[3])/[1])*((x-[3])/[1]),[2])", theStats.thetaMean - TMath::Pi(), theStats.thetaMean + TMath::Pi());
	thetaTsallisFunc.SetParNames("Amplitude", "Sigma", "Power", "Mean");
	thetaTsallisFunc.SetParameters(thetaHist.GetMaximum() * 5, theStats.thetaStDev * 0.005, .76, theStats.thetaMean);
	thetaTsallisFunc.SetParLimits(3, theStats.thetaMean, theStats.thetaMean);
	thetaHist.Fit(&thetaTsallisFunc, "VMR+");
	theStats.thetaTsallisPower = thetaTsallisFunc.GetParameter(2);
	theStats.thetaTsallisPowerError = thetaTsallisFunc.GetParError(2);

	return theStats;
}*/

void SRKManager::precessSpinsAlongTracksParAndAnti(int numTracks)
{
	//Run Parallel
	setParallelFields(true);
	resultsFilePath = defaultResultsDir + "Results_" + runID + "_P.root";
	precessSpinsAlongTracks(numTracks);

	//Run Anti-Parallel
	setParallelFields(false);
	resultsFilePath = defaultResultsDir + "Results_" + runID + "_A.root";
	precessSpinsAlongTracks(numTracks);

	//calcDeltaPhaseMean(runID);
	//double deltaOmega = deltaPhaseMean / theMotionTracker.getTimeLimit();
	//double deltaOmegaError = deltaPhaseError / theMotionTracker.getTimeLimit();

	//cout << "Delta \\omega [rad // s]: " << scientific << setprecision(5) << deltaOmega << " +/- " << deltaOmegaError << endl;
	//double scaleFactor = 100. * 6.58211928E-016 / (4. * e0FieldStrength);
	//cout << "False EDM [e cm]: " << scientific << setprecision(5) << deltaOmega * scaleFactor << " +/- " << deltaOmegaError * scaleFactor << endl;
	//double zetaEtaOmega0 = getZeta() * getEta() * getOmega0();

}

double SRKManager::calcMag(TVector3 vec)
{
	double magnitude = sqrt(vec.X()*vec.X() + vec.Y()*vec.Y() + vec.Z()*vec.Z());

	return magnitude;
}

double SRKManager::calcProjection(TVector3 vel, TVector3 M)
{
	double magV = calcMag(vel);
	double magM = calcMag(M);

	double proj = (vel.X()*M.X() + vel.Y()*M.Y() + vel.Z()*M.Z()) / (magV*magM);

	return proj;
}

bool SRKManager::fileExists(TString strFileName)
{
	struct stat stFileInfo;
	return stat(strFileName.Data(), &stFileInfo) == 0;
}

bool SRKManager::fileExistsAndNotZombie(TString strFileName)
{

	TFile theFile(strFileName, "READ");
	bool isZombie = theFile.IsZombie();
	theFile.Close();
	return fileExists(strFileName) && !isZombie;
}

