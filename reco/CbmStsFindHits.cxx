/** @file CbmStsFindHits.cxx
 ** @author Volker Friese <v.friese@gsi.de>
 ** @since 17.06.2014
 **/


#include "CbmStsFindHits.h"

#include <iomanip>
#include <iostream>
#include "TClonesArray.h"
#include "FairEventHeader.h"
#include "FairRunAna.h"
#include "CbmEvent.h"
#include "CbmStsSetup.h"

using namespace std;


// -----   Constructor   ---------------------------------------------------
CbmStsFindHits::CbmStsFindHits(ECbmMode mode)
    : FairTask("StsFindHits", 1)
    , fEvents(nullptr)
    , fClusters(nullptr)
    , fHits(nullptr)
    , fSetup(nullptr)
    , fTimer()
    , fMode(mode)
    , fTimeCutInSigma(4.)
    , fTimeCutInNs(-1.)
    , fNofTimeslices(0)
    , fNofEvents(0.)
    , fNofClusters(0.)
    , fNofHits(0.)
    , fTimeTot(0.)
    , fActiveModules()
{
}
// -------------------------------------------------------------------------



// -----   Destructor   ----------------------------------------------------
CbmStsFindHits::~CbmStsFindHits() {
}
// -------------------------------------------------------------------------



// -----   Task execution   ------------------------------------------------
void CbmStsFindHits::Exec(Option_t*) {

  // --- Clear output array
  fHits->Delete();

  // --- Time-slice mode: process entire array
  if ( fMode == kCbmTimeslice ) ProcessData(nullptr);

  // --- Event mode: loop over events
  else {
    assert(fEvents);
    Int_t nEvents = fEvents->GetEntriesFast();
    LOG(info) << setw(20) << left << GetName() << ": Processing time slice "
        << fNofTimeslices << " with " << nEvents
        << (nEvents == 1 ? " event" : " events");
    for (Int_t iEvent = 0; iEvent < nEvents; iEvent++) {
      CbmEvent* event = dynamic_cast<CbmEvent*>(fEvents->At(iEvent));
      assert(event);
      ProcessData(event);
    } //# events
  } //? event mode

  fNofTimeslices++;

}
// -------------------------------------------------------------------------



// -----   End-of-run action   ---------------------------------------------
void CbmStsFindHits::Finish() {

  std::cout << std::endl;
  LOG(info) << "=====================================";
  LOG(info) << GetName() << ": Run summary";
  LOG(info) << "Time slices           : " << fNofTimeslices;

  // --- Time-slice mode
  if ( fMode == kCbmTimeslice ) {
    LOG(info) << "Clusters / time slice : "
        << fNofClusters / Double_t(fNofTimeslices);
    LOG(info) << "Hits / time slice     : "
        << fNofHits / Double_t(fNofTimeslices);
    LOG(info) << "Clusters per hit      : " << fNofClusters / fNofHits;
    LOG(info) << "Time per time slice   : "
        << fTimeTot / Double_t(fNofTimeslices) << " s ";
  } //? time-slice mode

  // --- Event-by-event mode
  else {
    LOG(info) << "Events                : " << fNofEvents;
    LOG(info) << "Clusters / event      : "
        << fNofClusters / Double_t(fNofEvents);
    LOG(info) << "Hits / event          : "
        << fNofHits / Double_t(fNofEvents);
    LOG(info) << "Clusters per hit      : " << fNofClusters / fNofHits;
    LOG(info) << "Time per event        : "
        << fTimeTot / Double_t(fNofEvents) << " s ";
  } //? event mode

  LOG(info) << "=====================================";

}
// -------------------------------------------------------------------------



// -----   Initialisation   ------------------------------------------------
InitStatus CbmStsFindHits::Init() {

  // --- Something for the screen
  std::cout << std::endl;
  LOG(info) << "==========================================================";
  LOG(info) << GetName() << ": Initialising ";

  // --- Check IO-Manager
  FairRootManager* ioman = FairRootManager::Instance();
  assert(ioman);

  // --- In event mode: get input array (CbmEvent)
  if ( fMode == kCbmEvent ) {
    LOG(info) << GetName() << ": Using event-by-event mode";
    fEvents = dynamic_cast<TClonesArray*>(ioman->GetObject("Event"));
    if ( ! fEvents ) {
      LOG(warn) << GetName()
                              << ": Event mode selected but no event array found!";
      return kFATAL;
    } //? Event branch not present
  } //? Event mode
  else LOG(info) << GetName() << ": Using time-based mode";

  // --- Get input array (StsClusters)
  fClusters = dynamic_cast<TClonesArray*>(ioman->GetObject("StsCluster"));
  assert(fClusters);

  // --- Register output array
  fHits = new TClonesArray("CbmStsHit", 10000);
  ioman->Register("StsHit",
                  "Hits in STS",
                  fHits,
                  IsOutputBranchPersistent("StsHit"));

  // --- Check StsSetup instance
  fSetup = CbmStsSetup::Instance();
  assert(fSetup->IsInit());
  assert(fSetup->IsModulesInit());
  assert(fSetup->IsSensorsInit());

  LOG(info) << GetName() << ": Initialisation successful";
  LOG(info) << "==========================================================";

  return kSUCCESS;
}
// -------------------------------------------------------------------------



// -----   Process event or time-slice   -----------------------------------
Int_t CbmStsFindHits::ProcessData(CbmEvent* event) {

  // --- Clear clusters in modules
  fTimer.Start();
  Int_t nModules = 0;
  for (Int_t iModule = 0; iModule < fSetup->GetNofModules(); iModule++) {
    CbmStsModule* module = fSetup->GetModule(iModule);
    if ( module->GetNofClusters() == 0 ) continue;
    module->ClearClusters();
    nModules++;
  }
  fTimer.Stop();
  Double_t timeClear = fTimer.RealTime();
  LOG(debug) << GetName() << ": Cleared clusters in " << nModules
      << " modules. ";

  // --- Sort clusters into modules
  fTimer.Start();
  Int_t nClusters = SortClusters(event);
  fTimer.Stop();
  Double_t timeSort = fTimer.RealTime();

  // --- Find hits in modules
  fTimer.Start();
  Int_t nHits = 0;
  for (Int_t iModule = 0; iModule < fSetup->GetNofModules(); iModule++) {
    CbmStsModule* module = fSetup->GetModule(iModule);
    if ( module->GetNofClusters() == 0 ) continue;
    Int_t nHitsModule = module->FindHits(fHits, event,
																				 fTimeCutInNs, fTimeCutInSigma);
    LOG(debug1) << GetName() << ": Module " << module->GetName()
         << ", clusters: " << module->GetNofClusters()
         << ", hits: " << nHitsModule;
    nHits += nHitsModule;
  }
  fTimer.Stop();
  Double_t timeFind = fTimer.RealTime();

  // --- Counters
  Double_t realTime = timeClear + timeSort + timeFind;
  fNofEvents++;
  fNofClusters += nClusters;
  fNofHits     += nHits;
  fTimeTot     += realTime;

  // --- Log
  if ( event) LOG(info) << setw(20) << left << GetName() << ": " << "Event "
      << right << setw(6) << event->GetNumber() << ", real time " << fixed
      << setprecision(6) << realTime << " s, clusters: " << nClusters
      << ", hits: " << nHits;
  else LOG(info) << setw(20) << left << GetName() << ": " << "Time-slice "
      << right << setw(6) << fNofTimeslices << ", real time " << fixed
      << setprecision(6) << realTime << " s, clusters: " << nClusters
      << ", hits: " << nHits;
  LOG(debug) << GetName() << ": clear " << timeClear << ", sort " << timeSort
      << ", find " << timeFind;

  return nHits;
}
// -------------------------------------------------------------------------



// ----- Sort digis into module digi maps   --------------------------------
Int_t CbmStsFindHits::SortClusters(CbmEvent* event) {

  // --- Number of clusters
  Int_t nClusters = 0;
  if ( event ) nClusters = event->GetNofData(kStsCluster);
  else         nClusters = fClusters->GetEntriesFast();

  // --- Loop over input clusters
  for (Int_t iCluster = 0; iCluster < nClusters; iCluster++) {

    UInt_t index = (event ? event->GetIndex(kStsCluster, iCluster) : iCluster);
    CbmStsCluster* cluster = static_cast<CbmStsCluster*>(fClusters->At(index));
    assert(cluster);
    UInt_t address = cluster->GetAddress();
    cluster->SetIndex(index);
    CbmStsModule* module =
        static_cast<CbmStsModule*>(fSetup->GetElement(address, kStsModule));

    // --- Assign cluster to module
    module->AddCluster(cluster);

  } //# clusters

  // --- Debug output
  if ( FairLogger::GetLogger()->IsLogNeeded(fair::Severity::debug) ) {
    Int_t nActiveModules = 0;
    for (Int_t iModule = 0; iModule < fSetup->GetNofModules(); iModule++) {
      CbmStsModule* module = fSetup->GetModule(iModule);
      if ( module->GetNofClusters() == 0 ) continue;
      nActiveModules++;
      LOG(debug3) << GetName() << ": Module " << module->GetName()
                                << ", clusters " << module->GetNofClusters();
    } //# modules in setup
    LOG(debug) << GetName() << ": sorted " << nClusters << " clusters into "
        << nActiveModules << " module(s).";
  } //? DEBUG

  return nClusters;
}
// -------------------------------------------------------------------------


ClassImp(CbmStsFindHits)
