#include "SRKMacroManager.h"

#include <iostream>
#include <fstream>
#include <sstream>

#include "TRandom.h"

using namespace std;

SRKMacroManager::SRKMacroManager(SRKManager* inpManager)
{
	theManager = inpManager;
	defineCommands();

}

SRKMacroManager::~SRKMacroManager()
{

}

bool SRKMacroManager::openMacroFile(const TString filePath)
{
	ifstream theFile(filePath);
	TString sBuffer("#");
	if(!theFile.is_open() || theFile.fail() || theFile.eof())
	{
		cout << "Macro file: " << filePath << " not recognized" << endl;
		return false;
	}

	cout << "Opening macro file: " << filePath << endl;
	list<TString> fileCommandStringList;
	list<TString> fileCommandValueStringList;

	while (!theFile.fail() || !theFile.eof())
	{
		skipCommentLines(theFile);
		if(theFile.eof()) break;
		TString commandString, commandValueString;
		theFile >> commandString;
		theFile.ignore(1);
		commandValueString.ReadLine(theFile); //rest is value
		//cout << "Grabbed: " << commandString << "  and value: " << commandValueString << endl;

		fileCommandStringList.push_back(commandString);
		fileCommandValueStringList.push_back(commandValueString);
	}

	theFile.close();

	commandStringList.splice(commandStringList.begin(), fileCommandStringList);
	cout << "Number of commands: " << commandStringList.size() << endl;
	commandValueStringList.splice(commandValueStringList.begin(), fileCommandValueStringList);

	return true;
}

void SRKMacroManager::runMacroCommands()
{
	while (!commandStringList.empty())
	{

		TString command = commandStringList.front();
		TString value = commandValueStringList.front();
		commandStringList.pop_front();
		commandValueStringList.pop_front();

		cout << "SRK: " << command << " " << value << endl;
		if(!runMacroCommand(command.Data(), value.Data()))
		{
			break;  //Break if there is a command issue;
		}

	}
}

void SRKMacroManager::enterInteractiveMode()
{
	cout << endl << endl;
	cout << "*******************************************************" << endl;
	cout << "*  SRK Interactive Mode" << endl;
	cout << "*  Type \"q\" to exit." << endl;
	cout << "*******************************************************" << endl;
	bool endInteractiveMode = false;
	stringstream lineStream;
	while (!endInteractiveMode)
	{
		cout << "SRK: ";
		string theLine;
		getline(cin, theLine);
		if(theLine == "end" || theLine == "quit" || theLine == "exit" || theLine == "q")
		{
			cout << "Exiting Interactive Mode..." << endl;
			endInteractiveMode = true;
			break;
		}
		lineStream.clear();
		lineStream << theLine;
		string commandString, commandValueString;
		lineStream >> commandString;
		lineStream.ignore(1);
		getline(lineStream, commandValueString);
		//cout << "Command: " << commandString << "  Value: " << commandValueString << endl;
		runMacroCommand(commandString, commandValueString);

	}
}

bool SRKMacroManager::runMacroCommand(const string command, const string value)
{
	if(command == "runMacroFile") //Need to follow up and run the commands that got added but still return to interactive mode.
	{
		runMacroCommands();
		return true;
	}
	else if(commandMap.count(command) == 0) //Command doesn't exist?
	{
		cout << "Command: " << command << " doesn't exist!" << endl;
		return false;
	}

	commandMap[command](value);
	return true;
}

bool SRKMacroManager::skipCommentLines(ifstream& inpFileStream)
{

	if(!inpFileStream.is_open() || inpFileStream.fail() || inpFileStream.eof()) return -1;
	char peek = inpFileStream.peek();
	while (peek == '#' || peek == '%' || peek == '\n')
	{
		inpFileStream.ignore(numeric_limits<streamsize>::max(), '\n'); //discards everything in that line
		peek = inpFileStream.peek();
	}

	if(!inpFileStream.is_open() || inpFileStream.fail() || inpFileStream.eof()) return false;

	return true;
}

bool SRKMacroManager::getNonCommentLine(ifstream& inpFileStream, TString& outString)
{
	outString = "";
	if(!inpFileStream.is_open() || inpFileStream.fail() || inpFileStream.eof()) return -1;

	TString sBuffer("#");

	while (sBuffer[0] == '#' || sBuffer[0] == '%')
	{
		sBuffer.ReadLine(inpFileStream);
		if(inpFileStream.fail()) return false;
	}

	outString = sBuffer;
	return true;
}

bool SRKMacroManager::stobool(const string inp)
{
	if(inp == "0" || inp == "false" || inp == "FALSE" || inp == "False")
	{
		return false;
	}
	else if(inp == "1" || inp == "true" || inp == "TRUE" || inp == "True")
	{
		return true;
	}
	else
	{
		cout << "Truth value: " << inp << " not recognized" << endl;
	}
	return false;
}

void SRKMacroManager::sto3double(const string inp, double& x, double& y, double& z)
{
	stringstream aStringStream(inp);
	aStringStream >> x >> y >> z;
	return;

}

TVector3 SRKMacroManager::stoTVector3(const string inp)
{
	double x, y, z;
	sto3double(inp, x, y, z);
	return TVector3(x, y, z);

}

vector<double> SRKMacroManager::stoVector(const string inp)
{
	istringstream inp_stream(inp);
	vector<double> data(istream_iterator<double>(inp_stream),{});
	return data;
}

void SRKMacroManager::defineCommands()
{
	commandMap["setRecordPeriodicSteps"] = [&](string inp){	theManager->setRecordPeriodicSteps(stobool(inp));};
	commandMap["setUseAltStepping"] = [&](string inp){	theManager->setUseAltStepping(stobool(inp));};
	commandMap["setConstStepper"] = [&](string inp){	theManager->getSRKSpinTracker()->setConstStepper(stobool(inp));};
	commandMap["setManualTracking"] = [&](string inp){	theManager->getSRKMotionTracker()->setManualTracking(stobool(inp));};
	commandMap["setGyromagneticRatio"] = [&](string inp){	theManager->getSRKSpinTracker()->setGyromagneticRatio(stod(inp));};
	commandMap["setMass"] = [&](string inp){	theManager->getSRKMotionTracker()->setMass(stod(inp));};
	commandMap["setTimeLimit"] = [&](string inp){	theManager->getSRKMotionTracker()->setTimeLimit(stod(inp)); theManager->setTimeLimit(stod(inp));};
	commandMap["setPeriodicStopTime"] = [&](string inp){	theManager->getSRKMotionTracker()->setPeriodicStopTime(stod(inp)); theManager->setPeriodicStopTime(stod(inp));};
//	commandMap["setMeanVel"] = [&](string inp){	theManager->getSRKMotionTracker()->setMeanVel(stod(inp));};
//	commandMap["setChamberRotation"] = [&](string inp){	double x, y, z; sto3double(inp,x, y, z);theManager->getSRKMotionTracker()->setChamberRotation(x,y,z);};
	commandMap["setRingRadius"] = [&](string inp){ theManager->getSRKMotionTracker()->setRingRadius(stod(inp));};
	commandMap["setStorageDiameter"] = [&](string inp){ theManager->getSRKMotionTracker()->setStorageDiameter(stod(inp));};
//	commandMap["setBFieldStrength"] = [&](string inp){	theManager->setBFieldStrength(stod(inp));};
//	commandMap["setEFieldStrength"] = [&](string inp){	theManager->setEFieldStrength(stod(inp));};
	commandMap["setB0FieldStrength"] = [&](string inp){	theManager->setB0FieldStrength(stoVector(inp));};
//	commandMap["setE0FieldStrength"] = [&](string inp){	theManager->setE0FieldStrength(stod(inp));};
//	commandMap["setBGradFieldStrength"] = [&](string inp){	theManager->setBGradFieldStrength(stod(inp));};
//	commandMap["setEGradFieldStrength"] = [&](string inp){	theManager->setEGradFieldStrength(stod(inp));};
	commandMap["setBQuadFieldStrength"] = [&](string inp){  theManager->setBQuadFieldStrength(stod(inp));};
	commandMap["setEQuadFieldStrength"] = [&](string inp){  theManager->setEQuadFieldStrength(stod(inp));};
	commandMap["setBMultiFieldStrength"] = [&](string inp){  theManager->addBMultiFieldStrength(stoVector(inp));};
	commandMap["setBMultiOrder"] = [&](string inp){  theManager->addBMultiOrder(stoi(inp));};
	commandMap["setBMultiSkew"] = [&](string inp){  theManager->addBMultiSkewness(stobool(inp));};
	commandMap["setEMultiFieldStrength"] = [&](string inp){  theManager->addEMultiFieldStrength(stoVector(inp));};
	commandMap["setEMultiOrder"] = [&](string inp){  theManager->addEMultiOrder(stoi(inp));};
	commandMap["setEContinuous"] = [&](string inp){  theManager->setEcontinuous(stobool(inp));};
	commandMap["setRecordB"] = [&](string inp){  theManager->setRecordB(stobool(inp));};
	commandMap["setRecordE"] = [&](string inp){  theManager->setRecordE(stobool(inp));};
	commandMap["setEPSAbs"] = [&](string inp){	theManager->getSRKSpinTracker()->setEPSAbs(stod(inp));};
	commandMap["setEPSRel"] = [&](string inp){	theManager->getSRKSpinTracker()->setEPSRel(stod(inp));};
	commandMap["setInitialStepSize"] = [&](string inp){	theManager->getSRKSpinTracker()->setInitialStepSize(stod(inp));};
//	commandMap["setE0FieldDirection"] = [&](string inp){	theManager->setE0FieldDirection(stoTVector3(inp));};
	commandMap["setB0FieldDirection"] = [&](string inp){	theManager->setB0FieldDirection(stoTVector3(inp));};
	commandMap["setInitialPos"] = [&](string inp){	theManager->getSRKMotionTracker()->setInitialPos(stoTVector3(inp));};
	commandMap["setInitialVel"] = [&](string inp){	theManager->getSRKMotionTracker()->setInitialVel(stoTVector3(inp));};
	commandMap["setInitialMag"] = [&](string inp){	theManager->getSRKSpinTracker()->setInitialMag(stoTVector3(inp));};
	commandMap["setResultsFilePath"] = [&](string inp){	theManager->setResultsFilePath(inp);};
	commandMap["setRandomSeed"] = [&](string inp){	theManager->setRandomSeed(stoi(inp));};
	commandMap["setDefaultResultsDir"] = [&](string inp){	theManager->setDefaultResultsDir(inp);};
	commandMap["setVelProfHistPath"] = [&](string inp){	theManager->getSRKMotionTracker()->setVelProfHistPath(inp);}; //change to velocity dist of g-2?
	commandMap["setRunID"] = [&](string inp){	theManager->setRunID(inp);};
	commandMap["trackSpins"] = [&](string inp) {	theManager->precessSpinsAlongTracks(stoi(inp));};          // Depricated
	commandMap["precessSpinsAlongTracks"] = [&](string inp){	theManager->precessSpinsAlongTracks(stoi(inp));};
	commandMap["openMacroFile"] = [&](string inp){	openMacroFile(inp);};
//	commandMap["loadParametersFromResultsFile"] = [&](string inp){	theManager->loadParametersFromResultsFile(inp);};
}
