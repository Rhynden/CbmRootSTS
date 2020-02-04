/** @file CbmStsSetup.cxx
 ** @author Volker Friese <v.friese@gsi.de>
 ** @date 27.05.2013
 **/

// Include class header
#include "CbmStsSetup.h"

// Includes from C++
#include <cassert>
#include <fstream>
#include <iostream>
#include <iomanip>

// Includes from ROOT
#include "TFile.h"
#include "TGeoBBox.h"
#include "TGeoManager.h"
#include "TKey.h"
#include "TSystem.h"

// Includes form FairRoot
#include "FairField.h"
#include "FairRun.h"

// Includes from CbmRoot
#include "CbmStsAddress.h"
#include "CbmStsDigitizeParameters.h"
#include "CbmStsModule.h"
#include "CbmStsPhysics.h"
#include "CbmStsSensor.h"
#include "CbmStsSensorDssd.h"
#include "CbmStsSensorDssdStereo.h"
#include "CbmStsSensorDssdOrtho.h"
#include "CbmStsStation.h"

using namespace std;


// -----   Initialisation of static singleton pointer   --------------------
CbmStsSetup* CbmStsSetup::fgInstance = NULL;
// -------------------------------------------------------------------------



// -----   Constructor   ---------------------------------------------------
CbmStsSetup::CbmStsSetup() : CbmStsElement(kSts, kStsSystem),
			     fDigitizer(NULL),
			     fSettings(NULL),
			     fIsInitialised(kFALSE),
			     fIsModulesInit(kFALSE),
           fIsSensorsInit(kFALSE),
			     fIsOld(kFALSE),
			     fNofSensorsDefault(0),
			     fSensorDinact(0.),
			     fSensorPitch(0.),
           fSensorStereoF(0.),
           fSensorStereoB(0.),
			     fSensors(),
			     fModules(),
			     fModuleVector(),
			     fStations() 
{
}
// -------------------------------------------------------------------------



// -----   Assign a sensor object to an address   --------------------------
CbmStsSensor* CbmStsSetup::AssignSensor(Int_t address,
                                        TGeoPhysicalNode* node) {

  CbmStsSensor* sensor = nullptr;
  auto it = fSensors.find(address);
  if ( it != fSensors.end() ) {
    sensor = it->second; // Found in sensor list
    assert (sensor->GetAddress() == address);
    sensor->SetNode(node);
    sensor->Init();
    LOG(debug1) << GetName() << ": Assigning " << sensor->ToString()
        << "\n\t\t to node " << node->GetName();
  }
  else {
    sensor = DefaultSensor(address, node); // Not found; create and add.
    LOG(debug1) << GetName() << ": Assigning default " << sensor->ToString()
        << "\n\t\t to node " << node->GetName();
    fNofSensorsDefault++;
  }

  return sensor;
}
// -------------------------------------------------------------------------



// -----   Create station objects   ----------------------------------------
Int_t CbmStsSetup::CreateStations() {

  // For old geometries: the station corresponds to the unit
  if ( fIsOld ) {

    for (Int_t iUnit = 0; iUnit < GetNofDaughters(); iUnit++) {
      CbmStsElement* unit = GetDaughter(iUnit);

      // Create one station for each unit
      Int_t stationId = unit->GetIndex();
      TString name = Form("STS_S%02i", stationId+1);
      TString title = Form("STS Station %i", stationId+1);
      CbmStsStation* station = new CbmStsStation(name, title,
                                                 unit->GetPnode());
      // Add all ladders of the unit to the station
      for (Int_t iLadder = 0; iLadder < unit->GetNofDaughters(); iLadder++)
        station->AddLadder(unit->GetDaughter(iLadder));
      // Initialise station parameters
      station->Init();
      // Add station to station map
      assert ( fStations.find(stationId) == fStations.end() );
      fStations[stationId] = station;
    } //# units

    return fStations.size();
  } //? is old geometry?


  // Loop over all ladders. They are associated to a station.
  for (Int_t iUnit = 0; iUnit < GetNofDaughters(); iUnit++) {
    CbmStsElement* unit = GetDaughter(iUnit);
    for ( Int_t iLadder = 0; iLadder < unit->GetNofDaughters(); iLadder++) {
      CbmStsElement* ladder = unit->GetDaughter(iLadder);
      // This convention must be followed by the STS geometry
      Int_t nodeNumber = ladder->GetPnode()->GetNode()->GetNumber();
      Int_t stationId = nodeNumber / 100 - 1;
      // Get the station from the map. If not there, create it.
      CbmStsStation* station = NULL;
      if ( fStations.find(stationId) == fStations.end() ) {
        TString name = Form("STS_S%02i", stationId+1);
        TString title = Form("STS Station %i", stationId+1);
        station = new CbmStsStation(name, title);
        fStations[stationId] = station;
      } //? station not yet in map
      else station = fStations[stationId];

      // Add ladder to station
      station->AddLadder(ladder);

    } //# ladders
  } //# units

  // Initialise the stations
  auto it = fStations.begin();
  while ( it != fStations.end() ) {
    it->second->Init();
    it++;
  } //# stations

  // Check that the station number is set consecutively and that the
  // stations are ordered w.r.t. position along the beam
  Bool_t isOk = kTRUE;
  Double_t zPrevious = -999999;
  for (UInt_t iStation = 0; iStation < fStations.size(); iStation++) {
    if ( fStations.find(iStation) == fStations.end() ) {
      LOG(error) << GetName() << ": Number of stations is "
          << fStations.size() << ", but station " << iStation
          << "is not present!";
      isOk = kFALSE;
    } //? station present?
    if ( fStations[iStation]->GetZ() <= zPrevious ) {
      LOG(error) << GetName() << ": Disordered stations. Station "
          << iStation << " is at z = " << fStations[iStation]->GetZ()
          << "cm , previous is at z = " << zPrevious << " cm.";
      isOk = kFALSE;
    } //? disordered in z
  } //# stations
  if ( ! isOk ) LOG(fatal) << GetName() << ": Error in creation of stations.";

  return fStations.size();
}
// -------------------------------------------------------------------------



// -----   Instantiate default sensor   ------------------------------------
CbmStsSensor* CbmStsSetup::DefaultSensor(Int_t address,
                                         TGeoPhysicalNode* node) {

  // There should not already be a sensor object for this address
  assert ( fSensors.find(address) == fSensors.end() );

  // Sensor volume extension in x and y
  assert(node);
  TGeoBBox* shape = dynamic_cast<TGeoBBox*>(node->GetShape());
  assert(shape);
  Double_t volX = 2. * shape->GetDX();
  Double_t volY = 2. * shape->GetDY();

  // Default pitch and stereo angles
  Double_t pitch   = 0.0058;
  Double_t stereoF = 0.;
  Double_t stereoB = 7.5;

  // Size of inactive area in both x and y (total left+right/top+bottom)
  Double_t dInact = 0.24;

  // Number of strips. Calculated from active size in x, except for 6.2 cm
  // sensors, where it is 1024.
  Int_t nStrips = 1024;
  if ( TMath::Abs(volX-6.2) > 0.01 )
    nStrips = Int_t( (volX - dInact) / pitch );

  // Active size in y
  Double_t dy = volY - dInact;

  // Create default sensor type DssdStereo
  CbmStsSensorDssdStereo* sensor
    = new CbmStsSensorDssdStereo(dy, nStrips, pitch, stereoF, stereoB);

  // Assign address and node and add sensor to sensor list
  sensor->SetAddress(address);
  sensor->SetNode(node);
  Bool_t status = sensor->Init();
  assert(status);
  fSensors[address] = sensor;

  return sensor;
}
// -------------------------------------------------------------------------



// -----   Get an element from the STS setup   -----------------------------
CbmStsElement* CbmStsSetup::GetElement(Int_t address, Int_t level) {

	// --- Check for initialisation
	if ( ! fAddress ) LOG(fatal) << fName << ": not initialised!";

	// --- Catch non-STS addresses
	if ( CbmStsAddress::GetSystemId(address) != kSts ) {
		LOG(warn) << fName << ": No STS address " << address;
		return NULL;
	}

	// --- Catch illegal level numbers
	if ( level < 0 || level >= kStsNofLevels ) {
		LOG(warn) << fName << ": Illegal level " << level;
		return NULL;
	}

	CbmStsElement* element = this;
	for (Int_t iLevel = 1; iLevel <= level; iLevel++) {
		element =
				element->GetDaughter(CbmStsAddress::GetElementId(address, iLevel));
		assert(element);
	}

	return element;
}
// -------------------------------------------------------------------------



// -----   Get hierarchy level name   ---------------------------------------
const char* CbmStsSetup::GetLevelName(Int_t level) const {

  // --- Catch legacy (setup with stations)
  if ( fIsOld && level == kStsUnit ) return "station";

  switch(level) {
    case kStsSystem: return "sts"; break;
    case kStsUnit: return "unit"; break;
    case kStsLadder: return "ladder"; break;
    case kStsHalfLadder: return "halfladder"; break;
    case kStsModule: return "module"; break;
    case kStsSensor: return "sensor"; break;
    case kStsSide: return "side"; break;
    default: return ""; break;
  }

}
// -------------------------------------------------------------------------



// -----   Get the station number from an address   ------------------------
Int_t CbmStsSetup::GetStationNumber(Int_t address) {

  // In old, station-based geometries, the station equals the unit
  if ( fIsOld ) return CbmStsAddress::GetElementId(address, kStsUnit);

  // In new, unit-based geometries, the station is obtained from the ladder
  CbmStsElement* ladder = CbmStsSetup::GetElement(address, kStsLadder);
  assert(ladder);
  return ladder->GetPnode()->GetNode()->GetNumber() / 100 - 1;

}
// -------------------------------------------------------------------------



// -----   Initialisation   ------------------------------------------------
Bool_t CbmStsSetup::Init(const char* geoFile, const char* parFile) {

  // Prevent duplicate initialisation
  assert( ! fIsInitialised );

  cout << endl;
  LOG(info) << "==========================================================";
  LOG(info) << "Initialising STS Setup \n";

  // Read sensor parameters from file, if specified
  if ( parFile ) ReadSensorParameters(parFile);

  // --- Set system address
  fAddress = CbmStsAddress::GetAddress();

  // --- If a geometry file was specified, read the geometry from file
  if ( geoFile ) ReadGeometry(geoFile);

  // --- Else, read the geometry from TGeoManager
  else {
    assert(gGeoManager);
    ReadGeometry(gGeoManager);
  }

  // --- Statistics
  LOG(info) << fName << ": Elements in setup: ";
  for (Int_t iLevel = 1; iLevel <= kStsSensor; iLevel++) {
      TString name = GetLevelName(iLevel);
      name += "s";
      LOG(info) << "     " << setw(12) << name << setw(5) << right
                  << GetNofElements(iLevel);
  }
  LOG(info) << GetName() << ": " << fNofSensorsDefault
            << " sensors created from default.";

  // --- Build the module map
  for (Int_t iUnit = 0; iUnit < GetNofDaughters(); iUnit++) {
    CbmStsElement* unit = GetDaughter(iUnit);
    for (Int_t iLad = 0; iLad < unit->GetNofDaughters(); iLad++) {
      CbmStsElement* ladd = unit->GetDaughter(iLad);
      for (Int_t iHla = 0; iHla < ladd->GetNofDaughters(); iHla++) {
        CbmStsElement* hlad = ladd->GetDaughter(iHla);
        for (Int_t iMod = 0; iMod < hlad->GetNofDaughters(); iMod++) {
          CbmStsElement* modu = hlad->GetDaughter(iMod);
          Int_t address = modu->GetAddress();
          if ( fModules.find(address) != fModules.end() ) {
            LOG(fatal) << GetName() << ": Duplicate module address "
                << address << " for " << modu->GetName();
          }
          CbmStsModule* module = dynamic_cast<CbmStsModule*>(modu);
          fModules[address] = module;
          fModuleVector.push_back(module);
        } //# modules in half-ladder
      } //# half-ladders in ladder
    } //# ladders in unit
  } //# units in system

  // --- Create station objects
  Int_t nStations = CreateStations();
  LOG(info) << GetName() << ": Setup contains " << nStations
      << " stations objects.";
  if ( FairLogger::GetLogger()->IsLogNeeded(fair::Severity::debug) ) {
    auto it = fStations.begin();
    while ( it != fStations.end() ) {
      LOG(debug) << "  " << it->second->ToString();
      it++;
    } //# stations
  } //? Debug

  // --- Consistency check
  if ( GetNofSensors() != GetNofElements(kStsSensor) )
    LOG(fatal) << GetName() << ": inconsistent number of sensors! "
                   << GetNofElements(kStsSensor) << " " << GetNofSensors();
  if ( Int_t(fModules.size()) != GetNofElements(kStsModule) )
    LOG(fatal) << GetName() << ": inconsistent number of modules! "
                   << GetNofElements(kStsModule) << " "
                   << fModules.size();

  LOG(info) << "=========================================================="
            << "\n";
  cout << endl;

  fIsInitialised = kTRUE;
  return kTRUE;
}
// -------------------------------------------------------------------------



// -----   Singleton instance   --------------------------------------------
CbmStsSetup* CbmStsSetup::Instance() {
  if ( ! fgInstance ) fgInstance = new CbmStsSetup();
  return fgInstance;
}
// -------------------------------------------------------------------------



// -----   Print list of modules   -----------------------------------------
void CbmStsSetup::ListModules() const {
  for (auto it = fModules.begin(); it != fModules.end(); it++)
    LOG(info) << it->second->ToString();
}
// -------------------------------------------------------------------------



// -----   Modify the strip pitch for all sensors   ------------------------
Int_t CbmStsSetup::ModifyStripPitch(Double_t pitch) {

  Int_t nModified = 0;
  for ( auto it = fSensors.begin(); it != fSensors.end(); it++ ) {

    CbmStsSensorDssd* sensor = dynamic_cast<CbmStsSensorDssd*>(it->second);
    if ( ! sensor ) continue;
    sensor->ModifyStripPitch(pitch);
    nModified++;
  }

  return nModified;
}
// -------------------------------------------------------------------------



// -----   Read geometry from TGeoManager   --------------------------------
Bool_t CbmStsSetup::ReadGeometry(TGeoManager* geo) {

  // --- Catch non-existence of GeoManager
  assert (geo);
  LOG(info) << fName << ": Reading geometry from TGeoManager "
            << geo->GetName();

  // --- Get cave (top node)
  geo->CdTop();
  TGeoNode* cave = geo->GetCurrentNode();

  // --- Get top STS node
  TGeoNode* sts = NULL;
  for (Int_t iNode = 0; iNode < cave->GetNdaughters(); iNode++) {
    TString name = cave->GetDaughter(iNode)->GetName();
     if ( name.Contains("STS", TString::kIgnoreCase) ) {
      sts = cave->GetDaughter(iNode);
      LOG(info) << fName << ": STS top node is " << sts->GetName();
      break;
    }
  }
  if ( ! sts ) {
    LOG(error) << fName << ": No top STS node found in geometry!";
    return kFALSE;
  }

  // --- Create physical node for sts
  TString path = cave->GetName();
  path = path + "/" + sts->GetName();
  fNode = new TGeoPhysicalNode(path);

  // --- Check for old geometry (with stations) or new geometry (with units)
  Bool_t hasStation = kFALSE;
  Bool_t hasUnit = kFALSE;
  for (Int_t iDaughter = 0; iDaughter < fNode->GetNode()->GetNdaughters();
       iDaughter++) {
    TString dName = fNode->GetNode()->GetDaughter(iDaughter)->GetName();
    if ( dName.Contains("station", TString::kIgnoreCase) ) hasStation = kTRUE;
    if ( dName.Contains("unit", TString::kIgnoreCase) ) hasUnit = kTRUE;
  }
  if ( hasUnit && (! hasStation) ) fIsOld = kFALSE;
  else if ( (! hasUnit) && hasStation) fIsOld = kTRUE;
  else if ( hasUnit && hasStation) LOG(fatal) << GetName()
      << ": geometry contains both units and stations!";
  else LOG(fatal) << GetName()
  		<< ": geometry contains neither units nor stations!";
  if ( fIsOld ) LOG(warn) << GetName()
  		<< ": using old geometry (with stations)";

  // --- Recursively initialise daughter elements
  InitDaughters();

  return kTRUE;
}
// -------------------------------------------------------------------------



// -----   Read geometry from geometry file   ------------------------------
Bool_t CbmStsSetup::ReadGeometry(const char* fileName) {

  LOG(info) << fName << ": Reading geometry from file " << fileName;

  // Exit if a TGeoManager is already present
  assert ( ! gGeoManager );

  // --- Open geometry file
  TFile geoFile(fileName);
  if ( ! geoFile.IsOpen() ) {
    LOG(fatal) << GetName() << ": Could not open geometry file "
        << fileName;
    return kFALSE;
  }

  // Create a new TGeoManager
  TGeoManager* stsGeometry = new TGeoManager("StsGeo",
                                             "STS stand-alone geometry");

  // --- Get top volume from file
  TGeoVolume* topVolume = NULL;
  TList* keyList = geoFile.GetListOfKeys();
  TKey* key = NULL;
  TIter keyIter(keyList);
  while ( (key = (TKey*) keyIter() ) ) {
    if ( strcmp( key->GetClassName(), "TGeoVolumeAssembly" ) == 0 ) {
      TGeoVolume* volume = (TGeoVolume*) key->ReadObj();
      if ( strcmp(volume->GetName(), "TOP") == 0 ) {
        topVolume = volume;
        break;
      }  //? volume name is "TOP"
    }    //? object class is TGeoVolumeAssembly
  }
  if ( ! topVolume) {
    LOG(fatal) << GetName() << ": No TOP volume in file!";
    return kFALSE;
  }
  stsGeometry->SetTopVolume(topVolume);

  // --- Get cave (top node)
  stsGeometry->CdTop();
  TGeoNode* cave = stsGeometry->GetCurrentNode();

  // --- Get top STS node
  TGeoNode* sts = NULL;
  for (Int_t iNode = 0; iNode < cave->GetNdaughters(); iNode++) {
    TString name = cave->GetDaughter(iNode)->GetName();
    if ( name.Contains("STS", TString::kIgnoreCase) ) {
      sts = cave->GetDaughter(iNode);
      stsGeometry->CdDown(iNode);
      LOG(info) << fName << ": STS top node is " << sts->GetName();
      break;
    }
  }
  if ( ! sts ) {
    LOG(error) << fName << ": No top STS node found in geometry!";
    return kFALSE;
  }

  // --- Create physical node for sts
  TString path = cave->GetName();
  path = path + "/" + sts->GetName();
  fNode = new TGeoPhysicalNode(path);

  // --- Check for old geometry (with stations) or new geometry (with units)
  TString dName = fNode->GetNode()->GetDaughter(0)->GetName();
  LOG(debug) << "First node is " << dName;
  if ( dName.Contains("station", TString::kIgnoreCase) ) fIsOld = kTRUE;
  else if ( dName.Contains("unit", TString::kIgnoreCase) ) fIsOld = kFALSE;
  else LOG(fatal) << GetName() << ": unknown geometry type; first level name is "
      << dName;
  if ( fIsOld ) LOG(warn) << GetName()
  		<< ": using old geometry (with stations)";

  // --- Recursively initialise daughter elements
  InitDaughters();

  return kTRUE;
}
// -------------------------------------------------------------------------



// -----   Set sensor types   ----------------------------------------------
Int_t CbmStsSetup::ReadSensorParameters(const char* fileName) {

  // Input file
  std::fstream inFile;
  TString inputFile = fileName;

  // Try with argument as is (absolute path or current directory)
  inFile.open(inputFile.Data());

  // If not successful, look in the standard parameter directory
  if ( ! inFile.is_open() ) {
    inputFile = gSystem->Getenv("VMCWORKDIR");
    inputFile += "/parameters/sts/" + TString(fileName);
    inFile.open(inputFile.Data());
  }

  // If still not open, throw an error
  if ( ! inFile.is_open() ) {
    LOG(fatal) << GetName() << ": Cannot read file " << fileName
        << " nor " << inputFile;
    return -1;
  }

  string input;
  TString sName, sType;
  Int_t nSensors = 0;

  while ( kTRUE ) {  // read one line
    if ( inFile.eof() ) break;
    getline(inFile, input);
    if (input.empty() || input[0] == '#') continue;  // Comment line
    std::stringstream line(input);
    line >> sName >> sType;

    // Look for sensor in setup
    Int_t address = CbmStsSensor::GetAddressFromName(sName);

    // Bail out if sensor is already known
    if ( fSensors.find(address) != fSensors.end() ) {
      LOG(error) << GetName() << ": sensor " << sName
          << " is already in the setup!";
      continue;
    }

    // Case sensor type DssdStereo
    if ( sType.EqualTo("DssdStereo", TString::kIgnoreCase) ) {
      Double_t dy      = 0.;  // active length in y
      Int_t    nStrips = 0;   // number of strips
      Double_t pitch   = 0.;  // strip pitch
      Double_t stereoF = 0.;  // stereo angle front side
      Double_t stereoB = 0.;  // stereo angle back side
      line >> dy >> nStrips >> pitch >> stereoF >> stereoB;
      CbmStsSensorDssdStereo* sensor
      = new CbmStsSensorDssdStereo(dy, nStrips, pitch, stereoF, stereoB);
      sensor->SetAddress(address);
      LOG(debug) << "Created " << sensor->ToString();
      fSensors[address] = sensor;
      nSensors++;
    } //? sensor type DssdStereo
    else if ( sType.EqualTo("DssdOrtho", TString::kIgnoreCase) ) {
      Int_t    nStripsX = 0;   // number of strips
      Int_t    nStripsY = 0;   // number of strips
      Double_t pitchX   = 0.;  // strip pitch
      Double_t pitchY   = 0.;  // strip pitch
      line >> nStripsX >> pitchX >> nStripsY >> pitchY;
      CbmStsSensorDssdOrtho* sensor
      = new CbmStsSensorDssdOrtho(nStripsX, pitchX, nStripsY, pitchY);
      sensor->SetAddress(address);
      LOG(debug) << "Created " << sensor->ToString();
      fSensors[address] = sensor;
      nSensors++;
    } //? sensor type DssdOrtho
    else {
      LOG(fatal) << GetName() << ": Unknown sensor type " << sType
          << " for sensor " << sName;
    } //? Unknown sensor type

  } //# input lines

  inFile.close();
  LOG(info) << GetName() << ": Read " << nSensors
            << (nSensors == 1 ? " sensor" : " sensors") << " from "
            << inputFile;
  assert( nSensors = fSensors.size() );

  return nSensors;
}
// -------------------------------------------------------------------------



// -----   Set the default sensor parameters   -----------------------------
void CbmStsSetup::SetDefaultSensorParameters(Double_t dInact,
                                             Double_t pitch,
                                             Double_t stereoF,
                                             Double_t stereoB) {
  assert( ! fIsInitialised );
  assert( dInact >= 0.);
  assert( pitch >= 0. );
  fSensorDinact    = dInact;
  fSensorPitch     = pitch;
  fSensorStereoF   = stereoF;
  fSensorStereoB   = stereoB;
}
// -------------------------------------------------------------------------



// -----   Set parameters for all modules from parameter container   -------
Int_t CbmStsSetup::SetModuleParameters(CbmStsDigitizeParameters* par) {

  // Get conditions from parameter object
  assert(par);
  Double_t dynRange = par->GetDynRange();
  Double_t threshold = par->GetThreshold();
  Int_t nAdc = par->GetNofAdc();
  Double_t tResol = par->GetTimeResolution();
  Double_t tDead = par->GetDeadTime();
  Double_t noise = par->GetNoise();
  Double_t zeroNoiseRate = par->GetZeroNoiseRate();
  Double_t fracDeadChan = par->GetDeadChannelFrac();
  std::set<UChar_t> deadChannelMap = par->GetDeadChannelMap();

  // Set parameters for all modules
  return SetModuleParameters(dynRange, threshold, nAdc, tResol, tDead,
                             noise, zeroNoiseRate, fracDeadChan, deadChannelMap);
}
// -------------------------------------------------------------------------


// -----   Set global parameters for all modules   -------------------------
Int_t CbmStsSetup::SetModuleParameters(Double_t dynRange,
                                       Double_t threshold,
                                       Int_t nAdc, Double_t tResolution,
                                       Double_t tDead, Double_t noise,
                                       Double_t zeroNoiseRate,
                                       Double_t fracDeadChannels,
                                       std::set<UChar_t> deadChannelMap) {

  // Protect against multiple initialisation
  if ( fIsModulesInit ) {
    LOG(warn) << GetName()
        << ": module parameters are already initialised!";
    return 0;
  }

  // Set all module parameters
  Int_t nModules = 0;
  for ( auto it = fModules.begin(); it != fModules.end(); it++) {
    it->second->SetParameters(dynRange, threshold, nAdc, tResolution,
                              tDead, noise, zeroNoiseRate, fracDeadChannels, deadChannelMap);
    nModules++;
  }

  // --- Control output of parameters
  LOG(info) << GetName() << ": Set parameters for " << nModules << " modules:";
  LOG(info) << "\t Dynamic range              " << dynRange << " e";
  LOG(info) << "\t Threshold                  " << threshold << " e";
  LOG(info) << "\t Number of ADC channels     " << nAdc;
  LOG(info) << "\t Time resolution            " << tResolution << " ns";
  LOG(info) << "\t Dead time                  " << tDead << " ns";
  LOG(info) << "\t Noise RMS                  " << noise << " e";
  LOG(info) << "\t Zero-threshold noise rate  " << zeroNoiseRate << " / ns";
  LOG(info) << "\t Fraction of dead channels  " << fracDeadChannels;
  LOG(info) << "\t Number of dead channels    " << deadChannelMap.size();

  fIsModulesInit = kTRUE;
  return nModules;
}
// -------------------------------------------------------------------------



// -----   Set module parameters from file   -------------------------------
Int_t CbmStsSetup::SetModuleParameters(const char* fileName) {

  // Protect against multiple initialisation
  if ( fIsModulesInit ) {
    LOG(warn) << GetName()
        << ": module parameters are already initialised!";
    return 0;
  }

  // Input file
  std::fstream inFile;
  TString inputFile = fileName;

  // Try with argument as is (absolute path or current directory)
  inFile.open(inputFile.Data());

  // If not successful, look in the standard parameter directory
  if ( ! inFile.is_open() ) {
    inputFile = gSystem->Getenv("VMCWORKDIR");
    inputFile += "/parameters/sts/" + TString(fileName);
    inFile.open(inputFile.Data());
  }

  // If still not open, throw an error
  if ( ! inFile.is_open() ) {
    LOG(fatal) << GetName() << ": Cannot read file " << fileName
        << " nor " << inputFile;
    return 0;
  }

  string input;
  TString mName;
  Double_t dynRange        = -1.e10;
  Double_t threshold       = -1.e10;
  Int_t    nAdc            = -1;
  Double_t tResol          = -1.e10;
  Double_t tDead           = -1.e10;
  Double_t noise           = -1.e10;
  Double_t zeroNoise       = -1.e10;
  Double_t fracDead        = -1.e10;
  std::string sDeadChannelMap;
  UInt_t nModules = 0;
  Int_t iAsic = 0;

  std::set<Int_t> moduleSet;

  while ( kTRUE ) {  // read one line
    if ( inFile.eof() ) break;
    getline(inFile, input);
    if (input.empty() || input[0] == '#') continue;  // Comment line
    std::stringstream line(input);
    sDeadChannelMap.clear();
    line >> mName >> iAsic >> dynRange >> threshold >> nAdc >> tResol >> tDead >> noise
      >> zeroNoise >> fracDead >> sDeadChannelMap;

    // Look for module in setup
    Int_t address = CbmStsModule::GetAddressFromName(mName);
    if ( fModules.find(address) == fModules.end() ) {
      LOG(error) << GetName() << ": Module " << mName
          << " not found in the setup!";
      continue;
    }
    CbmStsModule* module = fModules.find(address)->second;
    assert(module);

    // Check for double occurrences of sensors
    if ( module->IsSet() ) {
      LOG(error) << GetName() << ": Parameters of module "
          << module->GetName() << " are already set!";
      continue;
    }

    // Check presence of module parameters
    if ( dynRange < 1.e-9 || threshold < 1.e-9 || nAdc < 0
        || tResol < 1.e-9 || tDead < 1.e-9 || noise < 1.e-9
        || zeroNoise < 1.e-9 || fracDead < 0. || fracDead > 1.) {
      LOG(error) << GetName()
          << ": Missing or illegal parameters for module "
          << module->GetName() << "; " << iAsic << " " << dynRange << " " << threshold << " "
          << nAdc << " " << tResol << " " << tDead << " " << noise << " "
          << zeroNoise << " " << fracDead;
      continue;
    }

    std::replace(sDeadChannelMap.begin(), sDeadChannelMap.end(), ',', ' ');
    std::istringstream iDeadChannelMap(sDeadChannelMap);
    std::set<UChar_t> deadChannelMap = std::set<UChar_t>(std::istream_iterator<int>(iDeadChannelMap), std::istream_iterator<int>());

    std::vector<CbmStsDigitizeParameters>& asics = module->GetParameters();
    // --- Set parameters of module
    asics[iAsic].SetModuleParameters(dynRange, threshold, nAdc, tResol, tDead, noise,
                                     zeroNoise, fracDead, deadChannelMap);
    LOG(debug1) << GetName() << ": Set " << module->ToString() << " Asic: " << iAsic;
    moduleSet.insert(address);

  } //# input lines

  nModules = moduleSet.size();
  inFile.close();
  LOG(info) << GetName() << ": Read parameters of " << nModules
            << (nModules == 1 ? " module" : " modules") << " from "
            << inputFile;

  // Check that all sensors have their conditions set
  if ( nModules!= fModules.size() ) {
    LOG(fatal) << GetName() << ": " << fModules.size()
     << " modules in setup, but parameters for " << nModules
     << " in parameter file!";
  }

  fIsModulesInit = kTRUE;
  return nModules;
}
// -------------------------------------------------------------------------


// -----   Set parameters for certain modules from parameter containers   --
Int_t CbmStsSetup::SetModuleParameterMap(std::map<Int_t, CbmStsDigitizeParameters*> parMap) {

  // Set all module parameters
  for ( auto it = parMap.begin(); it != parMap.end(); it++) {
    CbmStsDigitizeParameters* par = it->second;
    assert(par);
    CbmStsModule* module = fModules[it->first];
    assert(module);
    module->SetParameters(par->GetDynRange(),
                          par->GetThreshold(),
                          par->GetNofAdc(),
                          par->GetTimeResolution(),
                          par->GetDeadTime(),
                          par->GetNoise(),
                          par->GetZeroNoiseRate(),
                          par->GetDeadChannelFrac(),
                          par->GetDeadChannelMap());
  }

  return parMap.size();
}
// -------------------------------------------------------------------------


// -----   Store module parameters to file   -------------------------------
Int_t CbmStsSetup::StoreModuleParameters(const char* fileName) {

  // Output file
  std::ofstream oFile;
  TString outputFile = fileName;

  // Try with argument as is (absolute path or current directory)
  oFile.open(outputFile.Data(), std::ofstream::out | std::ofstream::trunc);

  // If not successful, look in the standard parameter directory
  if ( ! oFile.is_open() ) {
    outputFile = gSystem->Getenv("VMCWORKDIR");
    outputFile += "/parameters/sts/" + TString(fileName);
    oFile.open(outputFile.Data(), std::ofstream::out | std::ofstream::trunc);
  }

  // If still not open, throw an error
  if ( ! oFile.is_open() ) {
    LOG(fatal) << GetName() << ": Cannot write file " << fileName
        << " nor " << outputFile;
    return 0;
  }

  TString mName;
  Double_t dynRange        = -1.e10;
  Double_t threshold       = -1.e10;
  Int_t    nAdc            = -1;
  Double_t tResol          = -1.e10;
  Double_t tDead           = -1.e10;
  Double_t noise           = -1.e10;
  Double_t zeroNoise       = -1.e10;
  Double_t fracDead        = -1.e10;
  std::set<UChar_t> deadChannelMap;
  UInt_t nModules = 0;

  for (auto it = fModules.begin(); it != fModules.end(); it++) {
    CbmStsModule* module = (*it).second;
    assert(module);

    // Check for double occurrences of sensors
    if ( !module->IsSet() ) {
      LOG(debug2) << GetName() << ": Parameters of module "
          << module->GetName() << " were not set!";
    }

    Int_t nChannels = module->GetNofChannels();

    mName = module->GetName();

    std::vector<CbmStsDigitizeParameters>& asics = module->GetParameters();

    for (UInt_t iAsic = 0; iAsic < asics.size(); iAsic++) {
      auto& asic = asics[iAsic];
      dynRange = asic.GetDynRange();
      threshold = asic.GetThreshold();
      nAdc = asic.GetNofAdc();
      tResol = asic.GetTimeResolution();
      tDead = asic.GetDeadTime();
      noise = asic.GetNoise();
      zeroNoise = asic.GetZeroNoiseRate();
      deadChannelMap = asic.GetDeadChannelMap();
      fracDead = deadChannelMap.size()/nChannels;


      std::stringstream deadChannelMapStream;
      std::copy(deadChannelMap.begin(), deadChannelMap.end(), std::ostream_iterator<int>(deadChannelMapStream, ","));

      oFile << mName << "\t" << iAsic << "\t" << dynRange << "\t" << threshold << "\t"
            << nAdc << "\t" << tResol << "\t" << tDead << "\t" << noise
            << "\t" << zeroNoise << "\t" << fracDead << "\t" << deadChannelMapStream.str() << std::endl;
    }
    // --- Store parameters of module
    LOG(debug1) << GetName() << ": Store module parameters " << module->ToString();
    nModules++;

  } //# modules in setup

  oFile.flush();
  oFile.close();
  LOG(info) << GetName() << ": Wrote parameters of " << nModules
            << (nModules == 1 ? " module" : " modules") << " to "
            << outputFile;

  return nModules;
}
// -------------------------------------------------------------------------


// -----   Set conditions for all sensors   --------------------------------
Int_t CbmStsSetup::SetSensorConditions(CbmStsDigitizeParameters* par) {

 // Get conditions from parameter object
  assert(par);
  Double_t vDep = par->GetVdep();           // Full depletion voltage
  Double_t vBias = par->GetVbias();         // Bias voltage
  Double_t temp = par->GetTemperature(); // Temperature
  Double_t cCoup = par->GetCcoup();         // Coupling capacitance
  Double_t cIs = par->GetCis();             // Inter-strip capacitance

  // Set conditions for all sensors
  return SetSensorConditions(vDep, vBias, temp, cCoup, cIs);
}
// -------------------------------------------------------------------------



// -----   Set conditions for all sensors   --------------------------------
Int_t CbmStsSetup::SetSensorConditions(Double_t vDep, Double_t vBias,
                                       Double_t temperature,
                                       Double_t cCoupling,
                                       Double_t cInterstrip) {

  // Protect against multiple initialisation
  if ( fIsSensorsInit ) {
    LOG(warn) << GetName()
        << ": sensor conditions are already initialised!";
    return 0;
  }

  Int_t nSensors = 0;   // Sensor counter

  // --- Set conditions for all sensors
  for ( auto it = fSensors.begin(); it != fSensors.end(); it++ ) {

    // Get sensor centre coordinates in the global c.s.
    Double_t local[3] = { 0., 0., 0.}; // sensor centre in local C.S.
    Double_t global[3];                // sensor centre in global C.S.
    it->second->GetNode()->GetMatrix()->LocalToMaster(local, global);

    // Get the field in the sensor centre
    Double_t field[3] = { 0., 0., 0.};
    if ( FairRun::Instance()->GetField() )
        FairRun::Instance()->GetField()->Field(global, field);
    it->second->SetConditions(vDep, vBias, temperature, cCoupling,
                              cInterstrip,
                              field[0]/10., field[1]/10., field[2]/10.);

    nSensors++;
  } //# sensors

  // --- Control output of parameters
  LOG(info) << GetName() << ": Set conditions for " << nSensors << " sensors:";
  LOG(info) << "\t Full-depletion voltage  " << vDep << " V";
  LOG(info) << "\t Bias voltage            " << vBias << " V";
  LOG(info) << "\t Temperature             " << temperature << " K";
  LOG(info) << "\t Coupling capacitance    " << cCoupling << " pF";
  LOG(info) << "\t Inter-strip capacitance " << cInterstrip << " pF";

  fIsSensorsInit = kTRUE;
  return nSensors;
}
// -------------------------------------------------------------------------



// -----   Set sensor conditions from file   -------------------------------
Int_t CbmStsSetup::SetSensorConditions(const char* fileName) {

  // Protect against multiple initialisation
  if ( fIsSensorsInit ) {
    LOG(warn) << GetName()
        << ": sensor conditions are already initialised!";
    return 0;
  }

  // Input file
  std::fstream inFile;
  TString inputFile = fileName;

  // Try with argument as is (absolute path or current directory)
  inFile.open(inputFile.Data());

  // If not successful, look in the standard parameter directory
  if ( ! inFile.is_open() ) {
    inputFile = gSystem->Getenv("VMCWORKDIR");
    inputFile += "/parameters/sts/" + TString(fileName);
    inFile.open(inputFile.Data());
  }

  // If still not open, throw an error
  if ( ! inFile.is_open() ) {
    LOG(fatal) << GetName() << ": Cannot read file " << fileName
        << " nor " << inputFile;
    return 0;
  }

  string input;
  TString sName;
  Double_t vDep        = -1.e10;
  Double_t vBias       = -1.e10;
  Double_t temperature = -1.e10;
  Double_t cCoupling   = -1.e10;
  Double_t cInterstrip = -1.e10;
  UInt_t nSensors = 0;

  while ( kTRUE ) {  // read one line
    if ( inFile.eof() ) break;
    getline(inFile, input);
    if (input.empty() || input[0] == '#') continue;  // Comment line
    std::stringstream line(input);
    line >> sName >> vDep >> vBias >> temperature >> cCoupling >> cInterstrip;

    // Look for sensor in setup
    Int_t address = CbmStsSensor::GetAddressFromName(sName);
    if ( fSensors.find(address) == fSensors.end() ) {
      LOG(error) << GetName() << ": Sensor " << sName
          << " not found in the setup!";
      continue;
    }
    CbmStsSensor* sensor = fSensors.find(address)->second;
    assert(sensor);

    // Check for double occurrences of sensors
    if ( sensor->GetConditions() ) {
      LOG(error) << GetName() << ": Conditions of sensor "
          << sensor->GetName() << " are already set!";
      continue;
    }

    // Check presence of condition parameters
    if ( vDep < 1.e-9 || vBias < 1.e-9 || temperature < 1.e-9
        || cCoupling < 1.e-9 || cInterstrip < 1.e-9 ) {
      LOG(error) << GetName()
          << ": Missing or illegal condition parameters for sensor "
          << sensor->GetName() << "; " << vDep << " " << vBias << " "
          << temperature << " " << cCoupling << " " << cInterstrip;
      continue;
    }

    // --- Get the field in the sensor centre
    Double_t local[3] = { 0., 0., 0.}; // sensor centre in local C.S.
    Double_t global[3];                // sensor centre in global C.S.
    sensor->GetNode()->GetMatrix()->LocalToMaster(local, global);
    Double_t field[3] = { 0., 0., 0.}; // field in sensor centre
    if ( FairRun::Instance()->GetField() )
        FairRun::Instance()->GetField()->Field(global, field);

    // --- Set conditions of sensor (n.b. conversion from kG to T)
    sensor->SetConditions(vDep, vBias, temperature, cCoupling, cInterstrip,
                          field[0]/10., field[1]/10., field[2]/10.);
    LOG(debug1) << GetName() << ": Conditions of sensor " << sensor->GetName()
        << " " << sensor->GetConditions()->ToString();
    nSensors++;

  } //# input lines

  inFile.close();
  LOG(info) << GetName() << ": Read conditions of " << nSensors
            << (nSensors == 1 ? " sensor" : " sensors") << " from "
            << inputFile;

  // Check that all sensors have their conditions set
  if ( nSensors != fSensors.size() ) {
    LOG(fatal) << GetName() << ": " << fSensors.size()
     << " sensors in setup, but conditions for " << nSensors
     << " in conditions file!";
  }

  return nSensors;
}
// -------------------------------------------------------------------------


ClassImp(CbmStsSetup)

