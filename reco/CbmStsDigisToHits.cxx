/** @file CbmStsDigisToHits.cxx
 ** @author Volker Friese <v.friese@gsi.de>
 ** @since 16.06.2014
 **/

#include "CbmStsDigisToHits.h"

#include <cassert>
#include <iomanip>
#include "TClonesArray.h"
#include "FairEventHeader.h"
#include "FairRun.h"
#include "FairRuntimeDb.h"
#include "CbmDigiManager.h"
#include "CbmEvent.h"
#include "CbmStsAddress.h"
#include "CbmStsClusterAnalysis.h"
#include "CbmStsClusterFinderModule.h"
#include "CbmStsDigi.h"
#include "CbmStsDigitizeParameters.h"
#include "CbmStsModule.h"
#include "CbmStsSensor.h"
#include "CbmStsSensorDssdStereo.h"
#include "CbmStsSetup.h"

using std::fixed;
using std::left;
using std::right;
using std::setprecision;
using std::setw;


// -----   Constructor   ---------------------------------------------------
CbmStsDigisToHits::CbmStsDigisToHits(ECbmMode mode, Bool_t clusterOutputMode)
    : FairTask("StsDigisToHits", 1)
    , fEvents(nullptr)
    , fDigiManager(nullptr)
    , fClusters(nullptr)
    , fSetup(nullptr)
    , fDigiPar(nullptr)
    , fAna(nullptr)
    , fTimer()
    , fMode(mode)
    , fTimeCutDigisInSigma(3.)
    , fTimeCutDigisInNs(-1.)
    , fTimeCutClustersInNs(-1.)
    , fTimeCutClustersInSigma(4.)
    , fNofTimeslices(0)
    , fNofEvents(0)
    , fNofDigis(0.)
    , fNofDigisUsed(0)
    , fNofDigisIgnored(0)
    , fNofClusters(0.)
    //CbmStsFindHits
    , fNofHits(0.)
    , clusterOutputMode(clusterOutputMode)
    , fTimeTot(0.)
    , fModules()
{
}
// -------------------------------------------------------------------------



// -----   Destructor   ----------------------------------------------------
CbmStsDigisToHits::~CbmStsDigisToHits() {

  // Delete cluster analysis
  if ( fAna ) delete fAna;

  // Delete cluster finder modules
  auto it = fModules.begin();
  while ( it != fModules.end() ) delete it->second;

}
// -------------------------------------------------------------------------



// -----   Initialise the cluster finding modules   ------------------------
Int_t CbmStsDigisToHits::CreateModules() {

  assert( fSetup );
  if ( FairLogger::GetLogger()->IsLogNeeded(fair::Severity::debug1) ) {
    fSetup->ListSensors();
    fSetup->ListModules();
  }

  Int_t nModules = fSetup->GetNofModules();
  fModuleIndex.reserve(nModules);
  for (Int_t iModule = 0; iModule < nModules; iModule++) {
    CbmStsModule* module = fSetup->GetModule(iModule);
    assert(module);
    assert(module->IsSet());
    Int_t address = module->GetAddress();
    const char* name = module->GetName();
    UShort_t nChannels = module->GetNofChannels();
    CbmStsClusterFinderModule* finderModule =
        new CbmStsClusterFinderModule(nChannels, fTimeCutDigisInNs, fTimeCutDigisInSigma, fTimeCutClustersInNs, fTimeCutClustersInSigma, name, module, iModule, fAna);

    // --- Check whether there be round-the corner clustering. This happens
    // --- only for DssdStereo sensors with non-vanishing stereo angle, where
    // --- a double-metal layer horizontally connects strips.
    CbmStsSensorDssdStereo* sensor =
        dynamic_cast<CbmStsSensorDssdStereo*>(module->GetDaughter(0));
    if ( sensor ) {
      if ( TMath::Abs(sensor->GetStereoAngle(0)) > 1. )
        finderModule->ConnectEdgeFront();
      if ( TMath::Abs(sensor->GetStereoAngle(1)) > 1. )
        finderModule->ConnectEdgeBack();
    }
    fModules[address] = finderModule;
    fModuleIndex[iModule] = finderModule;
  }
  LOG(info) << GetName() << ": " << fModules.size()
  		<< " reco modules created.";

  return nModules;
}
// -------------------------------------------------------------------------


void CbmStsDigisToHits::SetParContainers()
{
  fDigiPar = static_cast<CbmStsDigitizeParameters*>(FairRun::Instance()->GetRuntimeDb()->getContainer("CbmStsDigitizeParameters"));
}



// -----   Task execution   ------------------------------------------------
void CbmStsDigisToHits::Exec(Option_t*) {

  // --- Clear output array
  fHits->Delete();

  // --- Reset output array
  fClusters->Delete();

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
void CbmStsDigisToHits::Finish() {

  std::cout << std::endl;
  LOG(info) << "=====================================";
  LOG(info) << GetName() << ": Run summary";
  LOG(info) << "Time slices           : " << fNofTimeslices;

  // --- Time-slice mode
  if ( fMode == kCbmTimeslice ) {
    LOG(info) << "Digis / time slice         : "
        << fNofDigis / Double_t(fNofTimeslices);
    LOG(info) << "Digis used / time slice    : "
        << fNofDigisUsed / Double_t(fNofTimeslices);
    LOG(info) << "Digis ignored / time slice : "
        << fNofDigisIgnored / Double_t(fNofTimeslices);
    LOG(info) << "Clusters / time slice      : "
        << fNofClusters / Double_t(fNofTimeslices);
    LOG(info) << "Digis per cluster          : " << fNofDigisUsed / fNofClusters;
    LOG(info) << "Time per time slice        : "
        << fTimeTot / Double_t(fNofTimeslices) << " s ";
  } //? time-slice mode

  // --- Event-by-event mode
  else {
    LOG(info) << "Events                : " << fNofEvents;
    LOG(info) << "Digis / event         : "
        << fNofDigis / Double_t(fNofEvents);
    LOG(info) << "Digis used / event    : "
        << fNofDigisUsed / Double_t(fNofTimeslices);
    LOG(info) << "Digis ignored / event : "
        << fNofDigisIgnored / Double_t(fNofTimeslices);
    LOG(info) << "Clusters / event      : "
        << fNofClusters / Double_t(fNofEvents);
    LOG(info) << "Digis per cluster     : " << fNofDigisUsed / fNofClusters;
    LOG(info) << "Time per event        : "
        << fTimeTot / Double_t(fNofEvents) << " s ";
  } //? event mode

  LOG(info) << "=====================================";


  //CbmStsFindHits
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
InitStatus CbmStsDigisToHits::Init()
{

  // --- Something for the screen
  std::cout << std::endl;
  LOG(info) << "==========================================================";
  LOG(info) << GetName() << ": Initialising ";

  // --- Check IO-Manager
  FairRootManager* ioman = FairRootManager::Instance();
  assert(ioman);

  // --- Digi Manager
  fDigiManager = CbmDigiManager::Instance();
  fDigiManager->Init();

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

  // --- Check input array (StsDigis)
  if ( ! fDigiManager->IsPresent(kSts) ) LOG(fatal) << GetName()
  		<< ": No StsDigi branch in input!";

  // --- Get input array (StsClusters)
  //fClusters = dynamic_cast<TClonesArray*>(ioman->GetObject("StsCluster"));
  //assert(fClusters);

  // --- Register output array
  fClusters = new TClonesArray("CbmStsCluster", 1e6);
  ioman->Register("StsCluster",
                  "Clusters in STS",
                  fClusters,
                  IsOutputBranchPersistent("StsCluster"));

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

  // --- Instantiate cluster analysis
  fAna = new CbmStsClusterAnalysis();

  // --- Create reconstruction modules
  CreateModules();

  LOG(info) << GetName() << ": Initialisation successful.";
  LOG(info) << "==========================================================";

  return kSUCCESS;
}
// -------------------------------------------------------------------------



// -----   Process one time slice or event   -------------------------------
void CbmStsDigisToHits::ProcessData(CbmEvent* event) {

  // --- Reset all cluster finder modules
  fTimer.Start();
	Int_t nGood    = 0;
	Int_t nIgnored = 0;
  for (Int_t it = 0; it < fModules.size(); it++){
    fModuleIndex[it]->Reset();
  }
  fTimer.Stop();
  Double_t time1 = fTimer.RealTime();

  // --- Start index of newly created clusters
  Int_t indexFirst = fHits->GetEntriesFast();

  // --- Number of input digis
  fTimer.Start();
  Int_t nDigis = (event ? event->GetNofData(kStsDigi)
      : fDigiManager->GetNofDigis(kSts) );

  // --- Loop over input digis
  Int_t digiIndex = -1;
  for (Int_t iDigi = 0; iDigi < nDigis; iDigi++){

    digiIndex = (event ? event->GetIndex(kStsDigi, iDigi) : iDigi);
    

    //    if (ProcessDigi(digiIndex)) nGood++;
    //  else nIgnored++;
    
    const CbmStsDigi* digi = fDigiManager->Get<const CbmStsDigi>(iDigi);
    assert(digi);
    //digi->SetIndex(digiIndex);

    CbmStsClusterFinderModule* module = fModules.at(digi->GetAddress());
    assert(module);

    // --- Digi channel
    UShort_t channel = digi->GetChannel();
    assert ( channel < module->GetSize() );
    
    module->AddDigiToQueue(digi, iDigi);
  }
  fTimer.Stop();
  Double_t time2 = fTimer.RealTime();

  // --- Process remaining clusters in the buffers
  fTimer.Start();
  Int_t clusterCount = 0;
  for (Int_t it = 0; it < fModules.size(); it++){
      //clusterCount += fModuleIndex[it]->ProcessDigis(event);
      fHits->AbsorbObjects(fModuleIndex[it]->ProcessDigis(event));

  }
  //LOG(INFO) << "cluster count = " << clusterCount;
  fTimer.Stop();
  Double_t time3 = fTimer.RealTime();

  // --- Stop index of newly created clusters
  Int_t indexLast = fHits->GetEntriesFast();

  Int_t nClusters = 0;

  if (clusterOutputMode) {
    #pragma omp declare reduction(combineClusterOutput:TClonesArray*: omp_out->AbsorbObjects(omp_in)) initializer(omp_priv = new TClonesArray("CbmStsCluster", 1e1))

    #pragma omp parallel for schedule(static) reduction(combineClusterOutput:fClusters) if(parallelism_enabled)
    for (Int_t it = 0; it < fModules.size(); it++){

      fClusters->AbsorbObjects(fModuleIndex[it]->GetClusterOutput());
      //LOG(INFO) << "Module " << it << " finished " << FairLogger::endl;

    }

    nClusters = fClusters->GetEntriesFast();
    // give each cluster an index
    for (Int_t iCluster = 0; iCluster < nClusters; iCluster++) {

      UInt_t index = (event ? event->GetIndex(kStsCluster, iCluster) : iCluster);
      CbmStsCluster* cluster = static_cast<CbmStsCluster*>(fClusters->At(index));
      assert(cluster);
      // Enable if you want the indices to be continuous
      //cluster->SetIndex(index);
    }
  }

  // --- Determine cluster parameters
  fTimer.Start();
  /*for (Int_t index = indexFirst; index < indexLast; index++) {
    CbmStsCluster* cluster = dynamic_cast<CbmStsCluster*>(fClusters->At(index));
    CbmStsModule* module = dynamic_cast<CbmStsModule*>
    (fSetup->GetElement(cluster->GetAddress(), kStsModule));
    //fAna->Analyze(cluster, module);
  } */
  fTimer.Stop();
  Double_t time4 = fTimer.RealTime();

  // --- In event-by-event mode: register clusters to event
  fTimer.Start();
  if ( event ) {
    for (Int_t index = indexFirst; index < indexLast; index++)
      event->AddData(kStsCluster, index);
  } //? Event object

  fTimer.Stop();
  Double_t time5 = fTimer.RealTime();

  // --- Counters
  Int_t nHits = 0;
  nHits = indexLast - indexFirst;
  Double_t realTime = time1 + time2 + time3 + time4 + time5;
  fNofEvents++;
  fNofDigis        += nDigis;
  fNofDigisUsed    += nGood;
  fNofDigisIgnored += nIgnored;
  fNofClusters     += nClusters;
  fNofHits         += nHits;
  fTimeTot         += realTime;

  // --- Screen output
  LOG(debug) << GetName() << ": created " << nClusters << " from index "
      << indexFirst << " to " << indexLast;
  LOG(debug) << GetName() << ": reset " << time1 << ", process digis " << time2
      << ", process buffers " << time3 << ", analyse " << time4 << ", register "
      << time5;

  if ( event) LOG(info) << setw(20) << left << GetName() << ": " << "Event "
      << right << setw(6) << event->GetNumber() << ", real time " << fixed
      << setprecision(6) << realTime << " s, digis used: " << nGood
			<< ", ignored: " << nIgnored << ", clusters: " << nClusters;
  else LOG(info) << setw(20) << left << GetName() << ": " << "Time-slice "
      << right << setw(6) << fNofTimeslices << ", real time " << fixed
      << setprecision(6) << realTime << " s, digis used: " << nGood
			<< ", ignored: " << nIgnored << ", clusters: " << nClusters << ", hits: " << nHits;


  //CbmStsFindHits
  // --- Clear clusters in modules
  /*fTimer.Start();
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
  nClusters = 0;
  nClusters = SortClusters(event);
  fTimer.Stop();
  Double_t timeSort = fTimer.RealTime();

  // --- Find hits in modules
  fTimer.Start();
  Int_t nHits = 0;
  for (Int_t iModule = 0; iModule < fSetup->GetNofModules(); iModule++) {
    CbmStsModule* module = fSetup->GetModule(iModule);
    if ( module->GetNofClusters() == 0 ) continue;
    Int_t nHitsModule = module->FindHits(fHits, event,
																				 fTimeCutClustersInNs, fTimeCutClustersInSigma);
    LOG(debug1) << GetName() << ": Module " << module->GetName()
         << ", clusters: " << module->GetNofClusters()
         << ", hits: " << nHitsModule;
    nHits += nHitsModule;
  }
  fTimer.Stop();
  Double_t timeFind = fTimer.RealTime();

  // --- Counters
  realTime = 0.0;
  realTime = timeClear + timeSort + timeFind;
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
      << ", find " << timeFind; */

  //return nHits;
}
// -------------------------------------------------------------------------



// -----   Process one digi object   ---------------------------------------
Bool_t CbmStsDigisToHits::ProcessDigi(Int_t index) {

  // --- Get the digi object
  const CbmStsDigi* digi = fDigiManager->Get<CbmStsDigi>(index);
  assert(digi);
  UInt_t moduleAddress = CbmStsAddress::GetMotherAddress(digi->GetAddress(),
																												 kStsModule);

  // --- Get the cluster finder module
  assert( fModules.count(moduleAddress) );
  CbmStsClusterFinderModule* module = fModules[moduleAddress];
  assert(module);

  // --- Digi channel
  UShort_t channel = digi->GetChannel();
  assert ( channel < module->GetSize() );

  // --- Process digi in module
  return module->ProcessDigi(channel, digi->GetTime(), index);

}
// -------------------------------------------------------------------------


// ----- Sort digis into module digi maps   --------------------------------
Int_t CbmStsDigisToHits::SortClusters(CbmEvent* event) {

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

ClassImp(CbmStsDigisToHits)
