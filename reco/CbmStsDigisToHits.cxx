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
  fHitsVector.reserve(20000000);
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
        new CbmStsClusterFinderModule(nChannels, fTimeCutDigisInNs, fTimeCutDigisInSigma, fTimeCutClustersInNs, fTimeCutClustersInSigma, name, module, iModule, fAna, clusterOutputMode);

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
  fHits = new TClonesArray("CbmStsHit", 2000000);
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

  /*Int_t nDigisTS = fDigiManager->GetNofDigis(kSts);
  LOG(info) << "nDigis in TimeSlice " << nDigisTS;
  Double_t digiTime = -1000.;
  UShort_t channelTS = fDigiManager->Get<const CbmStsDigi>(1)->GetChannel();
  UInt_t moduleAddressTS = CbmStsAddress::GetMotherAddress(fDigiManager->Get<const CbmStsDigi>(1)->GetAddress(),
																												 kStsModule);
  
  for (Int_t index = 0; index < nDigisTS; index++){
    const CbmStsDigi* digi = fDigiManager->Get<const CbmStsDigi>(index);
    //LOG(info) << "Digi Time = " << digi->GetTime();
    if (channelTS == digi->GetChannel() && moduleAddressTS == digi->GetAddress()) assert(digi->GetTime() >= digiTime);
    channelTS = digi->GetChannel();
    digiTime = digi->GetTime();
    fDigiVector.push_back(digi);
  }
  LOG(info) << "All Digis sorted"; */

  // --- Reset all cluster finder modules
  fTimer.Start();
	Int_t nGood    = 0;
	Int_t nIgnored = 0;

  //Reset even Needed?
  #pragma omp parallel for schedule(static) if(parallelism_enabled)
  for (Int_t it = 0; it < fModules.size(); it++){
    fModuleIndex[it]->Reset();
  }
  fTimer.Stop();
  Double_t time1 = fTimer.RealTime();

  // --- Start index of newly created clusters
  Int_t indexFirst = fHits->GetEntriesFast();
  Int_t indexFirstVector = fHitsVector.size();

  // --- Number of input digis
  fTimer.Start();
  Int_t nDigis = (event ? event->GetNofData(kStsDigi)
      : fDigiManager->GetNofDigis(kSts) );

  // --- Loop over input digis
  Int_t digiIndex = -1;
  #pragma omp parallel for schedule(static) if(parallelism_enabled)
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


  fTimer.Start();
  std::chrono::nanoseconds duration = std::chrono::nanoseconds(0);
  LOG(info) << "nanoseconds before process digis = " << duration.count();
  auto start = std::chrono::high_resolution_clock::now();
  Int_t clusterCount = 0;
  #pragma omp declare reduction(combineHitOutput:TClonesArray*: omp_out->AbsorbObjects(omp_in)) initializer(omp_priv = new TClonesArray("CbmStsHit", 2e6))
  #pragma omp declare reduction(combineHitOutputVector: std::vector<CbmStsHit>: omp_out.insert(omp_out.end(), std::make_move_iterator(omp_in.begin()), std::make_move_iterator(omp_in.end())))
  TClonesArray* fHitsCopy = new TClonesArray("CbmStsHit", 2000000);
  if (!clusterOutputMode) {
    LOG(info) << "ClusterOutputMode deactivated, clusters are NOT saved";
    #pragma omp parallel for reduction(combineHitOutput:fHitsCopy) if(parallelism_enabled)
    for (Int_t it = 0; it < fModules.size(); it++){

        auto start = std::chrono::high_resolution_clock::now();
        //fModuleIndex[it]->ProcessDigis(event);
        fHitsCopy->AbsorbObjects(fModuleIndex[it]->ProcessDigis(event));
        //std::vector<CbmStsHit> temp = fModuleIndex[it]->ProcessDigis(event);
        //fHitsVector.insert(fHitsVector.end(), std::make_move_iterator(temp.begin()), std::make_move_iterator(temp.end()));
        //fModuleIndex[it]->ProcessDigis(event);


        auto stop = std::chrono::high_resolution_clock::now();
        std::chrono::nanoseconds duration = std::chrono::duration_cast<std::chrono::nanoseconds>(stop - start);
        LOG(info) << "Processing digis time in module " << it << " : " << duration.count();
    }
    LOG(info) << "fHitsCopy size = " << fHitsCopy->GetEntriesFast();
    fHits = fHitsCopy;
    LOG(info) << "fHits size = " << fHits->GetEntriesFast();
  }
  //LOG(INFO) << "cluster count = " << clusterCount;
  auto stop = std::chrono::high_resolution_clock::now();
  duration += std::chrono::duration_cast<std::chrono::nanoseconds>(stop - start);
  fTimer.Stop();
  

  //std::chrono::nanoseconds duration = std::chrono::duration_cast<std::chrono::nanoseconds>(stop - start);
  //LOG(info) << "Finding Hits in Module " << fModule << " took " << duration.count() << " milliseconds";
  //std::cout << "Time taken for all ProcessDigis = " << duration.count() << std::endl;

  Double_t time3 = fTimer.RealTime();

  // --- Stop index of newly created clusters
  Int_t indexLast = fHits->GetEntriesFast();
  Int_t indexLastVector = fHitsVector.size();

  Int_t nClusters = 0;

  if (clusterOutputMode) {
    TClonesArray* fClustersCopy = new TClonesArray("CbmStsCluster", 1e6);
    LOG(info) << "ClusterOutputMode activated, saving clusters";
    #pragma omp declare reduction(combineClusterOutput:TClonesArray*: omp_out->AbsorbObjects(omp_in)) initializer(omp_priv = new TClonesArray("CbmStsCluster", 1e1))

    LOG(info) << "Processing Digis to Clusters";
    #pragma omp parallel for schedule(static) reduction(combineClusterOutput:fClustersCopy) if(parallelism_enabled)
    for (Int_t it = 0; it < fModules.size(); it++){

      fClustersCopy->AbsorbObjects(fModuleIndex[it]->ProcessDigisToClusters(event));
      //LOG(INFO) << "Module " << it << " finished " << FairLogger::endl;

    }
    LOG(info) << "fClustersCopy size = " << fClustersCopy->GetEntriesFast();
    fClusters = fClustersCopy;
    LOG(info) << "fClusters size = " << fClusters->GetEntriesFast();

    LOG(info) << "Giving Clusters continous indizes";
    nClusters = fClusters->GetEntriesFast();
    // give each cluster an index
    for (Int_t iCluster = 0; iCluster < nClusters; iCluster++) {

      UInt_t index = (event ? event->GetIndex(kStsCluster, iCluster) : iCluster);
      CbmStsCluster* cluster = static_cast<CbmStsCluster*>(fClusters->At(index));
      assert(cluster);
      // Enable if you want the indices to be continuous
      //LOG(info) << "Giving cluster " << iCluster << " index " << index;
      cluster->SetIndex(index);
      //UInt_t address = cluster->GetAddress();
      //CbmStsModule* module =
      //  static_cast<CbmStsModule*>(fSetup->GetElement(address, kStsModule));

      // --- Assign cluster to module
      //module->AddCluster(cluster);
    }

    LOG(info) << "Processing Clusters To Hits";
    #pragma omp parallel for reduction(combineHitOutput:fHitsCopy) if(parallelism_enabled)
    for (Int_t it = 0; it < fModules.size(); it++){
      fHitsCopy->AbsorbObjects(fModuleIndex[it]->ProcessClustersToHits(event));
    }
    LOG(info) << "fHitsCopy size = " << fHitsCopy->GetEntriesFast();
    fHits = fHitsCopy;
    LOG(info) << "fHits size = " << fHits->GetEntriesFast();
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
  Int_t nHitsVector = indexLastVector - indexFirstVector;
  Double_t realTime = time1 + time2 + time3 + time4 + time5;
  fNofEvents++;
  fNofDigis        += nDigis;
  fNofDigisUsed    += nGood;
  fNofDigisIgnored += nIgnored;
  fNofClusters     += nClusters;
  fNofHits         += nHits;
  fTimeTot         += realTime;

  // --- Screen output
  LOG(info) << GetName() << ": created " << nClusters << " from index "
      << indexFirst << " to " << indexLast;
  LOG(info) << GetName() << ": resetting modules " << time1 << ", adding digis to modules " << time2
      << ", cluster and hit finding " << time3 << ", unused_timer " << time4 << ", register (eventmode) "
      << time5;

  if ( event) LOG(info) << setw(20) << left << GetName() << ": " << "Event "
      << right << setw(6) << event->GetNumber() << ", real time " << fixed
      << setprecision(6) << realTime << " s, digis used: " << nGood
			<< ", ignored: " << nIgnored << ", clusters: " << nClusters;
  else LOG(info) << setw(20) << left << GetName() << ": " << "Time-slice "
      << right << setw(6) << fNofTimeslices << ", real time " << fixed
      << setprecision(6) << realTime << " s, digis used: " << nGood
			<< ", ignored: " << nIgnored << ", clusters: " << nClusters << ", hits: " << nHits  << ", hits Vector: " << nHitsVector;


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
