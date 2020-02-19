/** @file CbmStsDigitize.cxx
 ** @author Volker Friese <v.friese@gsi.de>
 ** @date 23.05.2014
 **/

// Include class header
#include "CbmStsDigitize.h"

// Includes from C++
#include <cassert>
#include <cstring>
#include <fstream>
#include <sstream>
#include <sstream>
#include <iomanip>
#include <iostream>
#include <sstream>

// Includes from ROOT
#include "TClonesArray.h"
#include "TGeoBBox.h"
#include "TGeoMatrix.h"
#include "TGeoPhysicalNode.h"
#include "TGeoVolume.h"

// Includes from FairRoot
#include "FairEventHeader.h"
#include "FairField.h"
#include "FairLink.h"
#include "FairLogger.h"
#include "FairMCEventHeader.h"
#include "FairMCPoint.h"
#include "FairRunAna.h"
#include "FairRunSim.h"
#include "FairRuntimeDb.h"

// Includes from CbmRoot
#include "CbmMCTrack.h"
#include "CbmStsDigi.h"
#include "CbmStsPoint.h"

// Includes from STS
#include "setup/CbmStsModule.h"
#include "setup/CbmStsSensor.h"
#include "setup/CbmStsSensorConditions.h"
#include "setup/CbmStsSetup.h"
#include "digitize/CbmStsPhysics.h"
#include "digitize/CbmStsDigitizeParameters.h"

using std::fixed;
using std::left;
using std::right;
using std::setprecision;
using std::setw;
using std::string;
using std::stringstream;



// -----   Standard constructor   ------------------------------------------
CbmStsDigitize::CbmStsDigitize() :
  CbmDigitize<CbmStsDigi>("StsDigitize"),
  fIsInitialised(kFALSE),
  fDigiPar(nullptr),
  fUserPar(),
  fModuleParameterMap(),
  fSetup(nullptr),
  fPoints(nullptr),
  fTracks(nullptr),
  fTimer(),
  fSensorDinact(0.12),
  fSensorPitch(0.0058),
  fSensorStereoF(0.),
  fSensorStereoB(7.5),
  fSensorParameterFile(),
  fSensorConditionFile(),
  fModuleParameterFile(),
  fTimePointLast(-1.),
  fTimeDigiFirst(-1.),
  fTimeDigiLast(-1.),
  fNofPoints(0),
  fNofSignalsF(0),
  fNofSignalsB(0),
  fNofDigis(0),
  fNofEvents(0),
  fNofPointsTot(0.),
  fNofSignalsFTot(0.),
  fNofSignalsBTot(0.),
  fNofDigisTot(0.),
  fNofNoiseTot(0.),
  fTimeTot()
{ 
  ResetCounters();
  fBranchName = "StsDigi";
}
// -------------------------------------------------------------------------



// -----   Destructor   ----------------------------------------------------
CbmStsDigitize::~CbmStsDigitize() {
}
// -------------------------------------------------------------------------



// -----   Content of analogue buffers   -----------------------------------
Int_t CbmStsDigitize::BufferSize() const {
  Int_t    nSignals =  0;
  Int_t    nSigModule;
  Double_t t1Module;
  Double_t t2Module;

  for (Int_t iModule = 0; iModule < fSetup->GetNofModules(); iModule++) {
    fSetup->GetModule(iModule)->BufferStatus(nSigModule, t1Module, t2Module);
    nSignals += nSigModule;
  } //# modules in setup

  return nSignals;
}
// -------------------------------------------------------------------------



// -----   Print the status of the analogue buffers   ----------------------
string CbmStsDigitize::BufferStatus() const {

  Int_t    nSignals =  0;
  Double_t t1       = -1;
  Double_t t2       = -1.;

  Int_t    nSigModule;
  Double_t t1Module;
  Double_t t2Module;

  for (Int_t iModule = 0; iModule < fSetup->GetNofModules(); iModule++) {
    fSetup->GetModule(iModule)->BufferStatus(nSigModule, t1Module, t2Module);
    if ( nSigModule ) {
      nSignals += nSigModule;
      t1 = t1 < 0. ? t1Module : TMath::Min(t1, t1Module);
      t2  = TMath::Max(t2, t2Module);
    } //? signals in module buffer?
  } //# modules in setup

  std::stringstream ss;
  ss << nSignals << ( nSignals == 1 ? " signal " : " signals " )
		         << "in analogue buffers";
  if ( nSignals ) ss << " ( from " << fixed << setprecision(3)
			                       << t1 << " ns to " << t2 << " ns )";
  return ss.str();
}
// -------------------------------------------------------------------------



// -----   Create a digi object   ------------------------------------------
void CbmStsDigitize::CreateDigi(Int_t address, UShort_t channel,
                                Long64_t time,
                                UShort_t adc,
                                const CbmMatch& match) {

  // Update times of first and last digi
  fTimeDigiFirst = fNofDigis ?
      TMath::Min(fTimeDigiFirst, Double_t(time)) : time;
  fTimeDigiLast  = TMath::Max(fTimeDigiLast, Double_t(time));

  // Create digi and (if required) match and send them to DAQ
  CbmStsDigi* digi = new CbmStsDigi(address, channel, time, adc);
  if ( fCreateMatches ) {
    CbmMatch* digiMatch = new CbmMatch(match);
    SendData(digi, digiMatch);
  }
  else SendData(digi);

  fNofDigis++;
}
// -------------------------------------------------------------------------



// -----   Task execution   ------------------------------------------------
void CbmStsDigitize::Exec(Option_t* /*opt*/) {

  // --- Start timer and reset counters
  fTimer.Start();
  ResetCounters();

  // --- For debug: status of analogue buffers
  if ( gLogger->IsLogNeeded(fair::Severity::debug) ) {
    std::cout << std::endl;
    LOG(debug) << GetName() << ": " << BufferStatus();
  }

  // --- Store previous event time.  Get current event time.
  Double_t eventTimePrevious = fCurrentEventTime;
  GetEventInfo();

  // --- Generate noise from previous to current event time
  if ( fDigiPar->GetGenerateNoise() ) {
    Int_t nNoise = 0;
    Double_t tNoiseStart = fNofEvents ? eventTimePrevious : 0.;
    Double_t tNoiseEnd   = fCurrentEventTime;
    for (Int_t iModule = 0; iModule < fSetup->GetNofModules(); iModule++)
      nNoise += fSetup->GetModule(iModule)->GenerateNoise(tNoiseStart,
                                                          tNoiseEnd);
    fNofNoiseTot += Double_t(nNoise);
    LOG(info) << "+ " << setw(20) << GetName() << ": Generated  " << nNoise
        << " noise signals from t = " << tNoiseStart << " ns to "
        << tNoiseEnd << " ns";
  }

  // --- Analogue response: Process the input array of StsPoints
  ProcessMCEvent();
  LOG(debug) << GetName() << ": " << fNofSignalsF + fNofSignalsB
      << " signals generated ( "
      << fNofSignalsF << " / " << fNofSignalsB << " )";
  // --- For debug: status of analogue buffers
  if ( gLogger->IsLogNeeded(fair::Severity::debug)) {
    LOG(debug) << GetName() << ": " << BufferStatus();
  }

  // --- Readout time: in stream mode the time of the current event.
  // --- Analogue buffers will be digitised for signals at times smaller than
  // --- that time minus a safety margin depending on the module properties
  // --- (dead time and time resolution). In event mode, the readout time
  // --- is set to -1., meaning to digitise everything in the readout buffers.
  Double_t readoutTime = fEventMode ? -1. : fCurrentEventTime;

  // --- Digital response: Process buffers of all modules
  ProcessAnalogBuffers(readoutTime);

  // --- Check status of analogue module buffers
  if ( gLogger->IsLogNeeded(fair::Severity::debug)) {
    LOG(debug) << GetName() << ": " << BufferStatus();
  }

  // --- Event log
  LOG(info) << left << setw(15) << GetName() << "["
            << fixed << setprecision(3) << fTimer.RealTime() << " s]"
            << " Points: " << fNofPoints << ", signals: " << fNofSignalsF
            << " / " << fNofSignalsB << ", digis: " << fNofDigis;

  // --- Counters
  fTimer.Stop();
  fNofEvents++;
  fNofPointsTot   += fNofPoints;
  fNofSignalsFTot += fNofSignalsF;
  fNofSignalsBTot += fNofSignalsB;
  fNofDigisTot    += fNofDigis;
  fTimeTot        += fTimer.RealTime();

}
// -------------------------------------------------------------------------



// -----   Finish run    ---------------------------------------------------
void CbmStsDigitize::Finish() {

  // --- Start timer and reset counters
  fTimer.Start();
  ResetCounters();

  // --- In event-by-event mode, the analogue buffers should be empty.
  if ( fEventMode ) {
    if ( BufferSize() ) {
      LOG(info) << fName << BufferStatus();
      LOG(fatal) << fName << ": Non-empty analogue buffers at end of event "
          << " in event-by-event mode!";
    } //? buffers not empty
  } //? event-by-event mode

  // ---  In time-based mode: process the remaining signals in the buffers
  else {
    std::cout << std::endl;
    LOG(info) << GetName() << ": Finish run";
    LOG(info) << GetName() << ": " << BufferStatus();
    LOG(info) << GetName() << ": Processing analogue buffers";

    // --- Loop over all modules in the setup and process their buffers
    for (Int_t iModule = 0; iModule < fSetup->GetNofModules(); iModule++)
      fSetup->GetModule(iModule)->ProcessAnalogBuffer(-1.);

    // --- Screen output
    stringstream ss;
    ss << GetName() << ": " << fNofDigis
       << ( fNofDigis == 1 ? " digi " :  " digis " )
       << "created and sent to DAQ ";
    if ( fNofDigis ) ss << "( from " << fixed
        << setprecision(3) << fTimeDigiFirst << " ns to "
        << fTimeDigiLast << " ns )";
    LOG(info) << ss.str();
    LOG(info) << GetName() << ": " << BufferStatus();
  }

  fTimer.Stop();
  fNofPointsTot   += fNofPoints;
  fNofSignalsFTot += fNofSignalsF;
  fNofSignalsBTot += fNofSignalsB;
  fNofDigisTot    += fNofDigis;
  fTimeTot        += fTimer.RealTime();

  std::cout << std::endl;
  LOG(info) << "=====================================";
  LOG(info) << GetName() << ": Run summary";
  LOG(info) << "Events processed    : " << fNofEvents;
  LOG(info) << "StsPoint / event    : " << fixed << setprecision(1)
			                  << fNofPointsTot / Double_t(fNofEvents);
  LOG(info) << "Signals / event     : "
      << fNofSignalsFTot / Double_t(fNofEvents)
      << " / " << fNofSignalsBTot / Double_t(fNofEvents);
  LOG(info) << "StsDigi / event     : "
      << fNofDigisTot  / Double_t(fNofEvents);
  LOG(info) << "Digis per point     : " << setprecision(6)
			                  << fNofDigisTot / fNofPointsTot;
  LOG(info) << "Digis per signal    : "
      << fNofDigisTot / ( fNofSignalsFTot + fNofSignalsBTot );
  LOG(info) << "Noise digis / event : " << fNofNoiseTot / Double_t(fNofEvents);
  LOG(info) << "Noise fraction      : " << fNofNoiseTot / fNofDigisTot;
  LOG(info) << "Real time per event : " << fTimeTot / Double_t(fNofEvents)
			                  << " s";
  LOG(info) << "=====================================";
}
// -------------------------------------------------------------------------



// -----   Get parameter container from runtime DB   -----------------------
void CbmStsDigitize::SetParContainers()
{
  assert(FairRunAna::Instance());
  FairRuntimeDb* rtdb = FairRunAna::Instance()->GetRuntimeDb();
  fDigiPar = static_cast<CbmStsDigitizeParameters*>
    (rtdb->getContainer("CbmStsDigitizeParameters"));
}
// -------------------------------------------------------------------------



// -----   Initialisation    -----------------------------------------------
InitStatus CbmStsDigitize::Init() {

  // Screen output
  std::cout << std::endl;
  LOG(info) << "==========================================================";
  LOG(info) << GetName() << ": Initialisation \n\n";
  if ( fEventMode ) LOG(info) << GetName() << ": Using event-by-event mode";

  // Set digitization parameter container
  // TODO: Currently, we avoid that the digitization parameters are taken
  // from the database. They have to be initialised by the user (fUserPar).
  // If not, default values are taken.
  if ( ! fUserPar.IsInit() ) {
    LOG(info) << GetName() << ": Using default parameters";
    fUserPar.SetDefaults();
  }
  *fDigiPar = fUserPar;
  fDigiPar->setChanged();
  fDigiPar->setInputVersion(-2, 1);

  // Deactivate noise in event-by-event mode
  if ( fDigiPar->GetGenerateNoise() && fEventMode )
    fDigiPar->SetGenerateNoise(kFALSE);

  // Instantiate and set StsPhysics
  CbmStsPhysics::Instance()->SetProcesses(fDigiPar->GetELossModel(),
                                          fDigiPar->GetUseLorentzShift(),
                                          fDigiPar->GetUseDiffusion(),
                                          fDigiPar->GetUseCrossTalk(),
                                          fDigiPar->GetGenerateNoise());

  // --- Screen output of settings
  LOG(info) << GetName() << ": " << fDigiPar->ToString();

  // Get and initialise the STS setup interface
  InitSetup();

  // --- Get FairRootManager instance
  FairRootManager* ioman = FairRootManager::Instance();
  assert ( ioman );

  // --- Get input array (CbmStsPoint)
  fPoints = (TClonesArray*) ioman->GetObject("StsPoint");
  assert ( fPoints );

  // --- Get input array (CbmMCTrack)
  fTracks = (TClonesArray*) ioman->GetObject("MCTrack");
  assert ( fTracks );

  RegisterOutput();

  // --- Screen output
  LOG(info) << GetName() << ": Initialisation successful";
  LOG(info) << "==========================================================";
  std::cout << std::endl;

  // Set static initialisation flag
  fIsInitialised = kTRUE;

  return kSUCCESS;
}
// -------------------------------------------------------------------------



// -----   Initialisation of setup    --------------------------------------
void CbmStsDigitize::InitSetup() {

  // Get the STS setup interface
  fSetup = CbmStsSetup::Instance();

  // Register this task and the parameter container to the setup
  fSetup->SetDigitizer(this);

  // Set or read sensor parameters
  fSetup->SetDefaultSensorParameters(fSensorDinact, fSensorPitch,
                                     fSensorStereoF, fSensorStereoB);
  if ( fSensorParameterFile.IsNull() ) fSetup->Init();
  else fSetup->Init(nullptr, fSensorParameterFile);

  // Set sensor conditions, global or from file
  if ( fSensorConditionFile.IsNull() )
    fSetup->SetSensorConditions(fDigiPar);
  else
    fSetup->SetSensorConditions(fSensorConditionFile);

  // Set module parameters, global or from file
  if ( fModuleParameterFile.IsNull() )
    fSetup->SetModuleParameters(fDigiPar);
  else
    fSetup->SetModuleParameters(fModuleParameterFile);

  // Individual configuration
  fSetup->SetModuleParameterMap(fModuleParameterMap);
}
// -------------------------------------------------------------------------



// -----   Process the analogue buffers of all modules   -------------------
void CbmStsDigitize::ProcessAnalogBuffers(Double_t readoutTime) {

  // --- Debug
  LOG(debug) << GetName() << ": Processing analog buffers with readout "
      << "time " << readoutTime << " ns";

  // --- Loop over all modules in the setup and process their buffers
  for (Int_t iModule = 0; iModule < fSetup->GetNofModules(); iModule++)
    fSetup->GetModule(iModule)->ProcessAnalogBuffer(readoutTime);

  // --- Debug output
  stringstream ss;
  ss << GetName() << ": " << fNofDigis
      << ( fNofDigis == 1 ? " digi " :  " digis " )
      << "created and sent to DAQ ";
  if ( fNofDigis ) ss << "( from " << fixed
      << setprecision(3) << fTimeDigiFirst << " ns to "
      << fTimeDigiLast << " ns )";
  LOG(debug) << ss.str();

}
// -------------------------------------------------------------------------



// -----   Process points from MC event    ---------------------------------
void CbmStsDigitize::ProcessMCEvent() {

  // --- MC Event info (input file, entry number, start time)
  LOG(debug) << GetName() << ": Processing event " << fCurrentEvent
      << ", entry " << fCurrentMCEntry
      << " from input " << fCurrentInput << " at t = " << fCurrentEventTime
      << " ns with " << fPoints->GetEntriesFast() << " StsPoints ";

  // --- Loop over all StsPoints and execute the ProcessPoint method
  assert ( fPoints );
  for (Int_t iPoint=0; iPoint<fPoints->GetEntriesFast(); iPoint++) {
    const CbmStsPoint* point = (const CbmStsPoint*) fPoints->At(iPoint);
    CbmLink* link = new CbmLink(1., iPoint, fCurrentMCEntry, fCurrentInput);

    // --- Discard secondaries if the respective flag is set
    if ( fDigiPar->GetDiscardSecondaries() ) {
      Int_t iTrack = point->GetTrackID();
      if ( iTrack >= 0 ) {  // MC track is present
        CbmMCTrack* track = (CbmMCTrack*) fTracks->At(iTrack);
        assert ( track );
        if ( track->GetMotherId() >= 0 ) continue;
      } //? MC track present
    } //? discard secondaries

    ProcessPoint(point, fCurrentEventTime, link);
    fNofPoints++;
    delete link;
  }  //# StsPoints

}
// -------------------------------------------------------------------------



// -----  Process a StsPoint   ---------------------------------------------
void CbmStsDigitize::ProcessPoint(const CbmStsPoint* point,
                                  Double_t eventTime, CbmLink* link) {

  // Debug
  if ( FairLogger::GetLogger()->IsLogNeeded(fair::Severity::debug2) ) point->Print();
  LOG(debug2) << GetName() << ": Point coordinates: in (" << point->GetXIn()
			            << ", " << point->GetYIn() << ", " << point->GetZIn() << ")"
			            << ", out (" << point->GetXOut() << ", " << point->GetYOut()
			            << ", " << point->GetZOut() << ")";


  // --- Get the sensor the point is in
  Int_t address = point->GetDetectorID();
  CbmStsSensor* sensor = dynamic_cast<CbmStsSensor*>
  (fSetup->GetElement(address, kStsSensor));
  if ( ! sensor ) {
  	stringstream ss;
    ss << GetName() << ": No sensor for address " << address;
    ss << "Unit " << CbmStsAddress::GetElementId(address, kStsUnit);
    ss << " Ladder " << CbmStsAddress::GetElementId(address, kStsLadder);
    ss << " Half-ladder " << CbmStsAddress::GetElementId(address, kStsHalfLadder);
    ss << " Module " << CbmStsAddress::GetElementId(address, kStsModule);
    ss << " Sensor " << CbmStsAddress::GetElementId(address, kStsSensor);
    LOG(info) << ss.str();
  }
  if ( ! sensor ) LOG(error) << GetName() << ": Sensor of StsPoint not found!";
  assert(sensor);
  LOG(debug2) << GetName() << ": Sending point to sensor "
      << sensor->GetName() << " ( " << sensor->GetAddress() << " ) ";

  // --- Process the point on the sensor
  Int_t status = sensor->ProcessPoint(point, eventTime, link);

  // --- Statistics
  Int_t nSignalsF = status / 1000;
  Int_t nSignalsB = status - 1000 * nSignalsF;
  LOG(debug2) << GetName() << ": Produced signals: "
      << nSignalsF + nSignalsB << " ( " << nSignalsF << " / "
      << nSignalsB << " )";
  fNofSignalsF += nSignalsF;
  fNofSignalsB += nSignalsB;

}
// -------------------------------------------------------------------------



// -----   Private method ReInit   -----------------------------------------
InitStatus CbmStsDigitize::ReInit() {

  fSetup = CbmStsSetup::Instance();

  return kERROR;

}
// -------------------------------------------------------------------------



// -----   Reset event counters   ------------------------------------------
void CbmStsDigitize::ResetCounters() {
  fTimeDigiFirst = fTimeDigiLast = -1.;
  fNofPoints = fNofSignalsF = fNofSignalsB = fNofDigis = 0;
}
// -------------------------------------------------------------------------



// -----   Set the global module parameters   ------------------------------
void CbmStsDigitize::SetGlobalModuleParameters(Double_t dynRange,
                                               Double_t threshold,
                                               Int_t nAdc,
                                               Double_t timeResolution,
                                               Double_t deadTime,
                                               Double_t noise,
                                               Double_t zeroNoiseRate,
                                               Double_t fracDeadChan,
                                               std::set<UChar_t> deadChannelMap) {
  assert( ! fIsInitialised );
  assert( nAdc > 0 );
  assert( fracDeadChan >= 0. && fracDeadChan <= 1.);
  fUserPar.SetModuleParameters(dynRange,
                               threshold,
                               nAdc,
                               timeResolution,
                               deadTime,
                               noise,
                               zeroNoiseRate,
                               fracDeadChan,
                               deadChannelMap);
}
// -------------------------------------------------------------------------



// -----   Set the global sensor conditions   ------------------------------
void CbmStsDigitize::SetGlobalSensorConditions(Double_t vDep,
                                               Double_t vBias,
                                               Double_t temperature,
                                               Double_t cCoupling,
                                               Double_t cInterstrip) {
  assert( ! fIsInitialised );
  fUserPar.SetSensorConditions(vDep,
                               vBias,
                               temperature,
                               cCoupling,
                               cInterstrip);
}
// -------------------------------------------------------------------------



// -----   Activate noise generation   -------------------------------------
void CbmStsDigitize::SetGenerateNoise(Bool_t choice) {

  if ( fIsInitialised ) {
    LOG(error) << GetName() << ": physics processes must be set before "
        << "initialisation! Statement will have no effect.";
    return;
  }

  fUserPar.SetGenerateNoise(choice);
}
// -------------------------------------------------------------------------



// -----   Set sensor parameter file   -------------------------------------
void CbmStsDigitize::SetModuleParameterFile(const char* fileName) {

  assert( ! fIsInitialised );
  fModuleParameterFile = fileName;

}
// -------------------------------------------------------------------------



// -----   Set physical processes for the analogue response  ---------------
void CbmStsDigitize::SetProcesses(ECbmELossModel eLossModel,
                                  Bool_t useLorentzShift,
                                  Bool_t useDiffusion,
                                  Bool_t useCrossTalk,
                                  Bool_t generateNoise) {
  if ( fIsInitialised ) {
    LOG(error) << GetName() << ": physics processes must be set before "
        << "initialisation! Statement will have no effect.";
    return;
  }
  fUserPar.SetProcesses(eLossModel, useLorentzShift, useDiffusion,
                        useCrossTalk, generateNoise);
}
// -------------------------------------------------------------------------



// -----   Set sensor condition file   -------------------------------------
void CbmStsDigitize::SetSensorConditionFile(const char* fileName) {

  if ( fIsInitialised ) {
    LOG(fatal) << GetName()
            <<": sensor conditions must be set before initialisation!";
    return;
  }
  fSensorConditionFile = fileName;

}
// -------------------------------------------------------------------------



// -----   Set sensor parameter file   -------------------------------------
void CbmStsDigitize::SetSensorParameterFile(const char* fileName) {

  if ( fIsInitialised ) {
    LOG(fatal) << GetName()
            <<": sensor parameters must be set before initialisation!";
    return;
  }
  fSensorParameterFile = fileName;

}
// -------------------------------------------------------------------------



ClassImp(CbmStsDigitize)

