// -------------------------------------------------------------------------
// -----                  CbmStsFindTracksQa source file               -----
// -----                  Created 11/01/06  by V. Friese               -----
// -------------------------------------------------------------------------

// Includes class header
#include "CbmStsFindTracksQa.h"

// Includes from C++
#include <cassert>
#include <iomanip>

// Includes from ROOT
#include "TClonesArray.h"
#include "TGeoManager.h"
#include "TH1F.h"

// Includes from FairRoot
#include "FairEventHeader.h"
#include "FairLogger.h"
#include "FairRunAna.h"
#include "FairRuntimeDb.h"

// Includes from CbmRoot
#include "CbmEvent.h"
#include "CbmGeoPassivePar.h"
#include "CbmMCDataArray.h"
#include "CbmMCDataManager.h"
#include "CbmMCTrack.h"
#include "CbmStsHit.h"
#include "CbmStsPoint.h"
#include "CbmStsSetup.h"
#include "CbmStsTrack.h"
#include "CbmTrackMatchNew.h"


using std::fixed;
using std::right;
using std::setprecision;
using std::setw;


// -----   Default constructor   -------------------------------------------
CbmStsFindTracksQa::CbmStsFindTracksQa(Int_t iVerbose)
  : FairTask("STSFindTracksQA", iVerbose),
    fHitMap(),
    fMatchMap(),
    fQualiMap(),
    fEvents(),
    fMCTracks(NULL),
    fStsPoints(NULL),
    fStsHits(NULL),
    fStsHitMatch(NULL),
    fStsTracks(NULL),
    fMatches(NULL),
    fLegacy(kFALSE),
    fPassGeo(NULL),
    fTargetPos(0.,0.,0.),
    fSetup(NULL),
    fNStations(0),
    fMinStations(3),
    fQuota(0.7),
    fhMomAccAll(new TH1F()),
    fhMomRecAll(new TH1F()),
    fhMomEffAll(new TH1F()),
    fhMomAccPrim(new TH1F()),
    fhMomRecPrim(new TH1F()),
    fhMomEffPrim(new TH1F()),
    fhMomAccSec(new TH1F()),
    fhMomRecSec(new TH1F()),
    fhMomEffSec(new TH1F()),
    fhNpAccAll(new TH1F()),
    fhNpRecAll(new TH1F()),
    fhNpEffAll(new TH1F()),
    fhNpAccPrim(new TH1F()),
    fhNpRecPrim(new TH1F()),
    fhNpEffPrim(new TH1F()),
    fhNpAccSec(new TH1F()),
    fhNpRecSec(new TH1F()),
    fhNpEffSec(new TH1F()),
    fhZAccSec(new TH1F()),
    fhZRecSec(new TH1F()),
    fhZEffSec(new TH1F()),
    fhNhClones(new TH1F()),
    fhNhGhosts(new TH1F()),
    fHistoList(new TList()),
    fNAccAll(0),
    fNAccPrim(0),
    fNAccRef(0),
    fNAccSec(0),
    fNRecAll(0),
    fNRecPrim(0),
    fNRecRef(0),
    fNRecSec(0),
    fNGhosts(0),
    fNClones(0),
    fNEvents(0),
    fNEventsFailed(0),
    fTime(0.),
    fTimer()
{}

// -------------------------------------------------------------------------



// -----   Standard constructor   ------------------------------------------
CbmStsFindTracksQa::CbmStsFindTracksQa(Int_t minStations, Double_t quota,
				       Int_t iVerbose)
  : FairTask("STSFindTracksQA", iVerbose),
    fHitMap(),
    fMatchMap(),
    fQualiMap(),
    fEvents(NULL),
    fMCTracks(NULL),
    fStsPoints(NULL),
    fStsHits(NULL),
    fStsHitMatch(NULL),
    fStsTracks(NULL),
    fMatches(NULL),
    fLegacy(kFALSE),
    fPassGeo(NULL),
    fTargetPos(0.,0.,0.),
    fSetup(NULL),
    fNStations(0),
    fMinStations(minStations),
    fQuota(quota),
    fhMomAccAll(new TH1F()),
    fhMomRecAll(new TH1F()),
    fhMomEffAll(new TH1F()),
    fhMomAccPrim(new TH1F()),
    fhMomRecPrim(new TH1F()),
    fhMomEffPrim(new TH1F()),
    fhMomAccSec(new TH1F()),
    fhMomRecSec(new TH1F()),
    fhMomEffSec(new TH1F()),
    fhNpAccAll(new TH1F()),
    fhNpRecAll(new TH1F()),
    fhNpEffAll(new TH1F()),
    fhNpAccPrim(new TH1F()),
    fhNpRecPrim(new TH1F()),
    fhNpEffPrim(new TH1F()),
    fhNpAccSec(new TH1F()),
    fhNpRecSec(new TH1F()),
    fhNpEffSec(new TH1F()),
    fhZAccSec(new TH1F()),
    fhZRecSec(new TH1F()),
    fhZEffSec(new TH1F()),
    fhNhClones(new TH1F()),
    fhNhGhosts(new TH1F()),
    fHistoList(new TList()),
    fNAccAll(0),
    fNAccPrim(0),
    fNAccRef(0),
    fNAccSec(0),
    fNRecAll(0),
    fNRecPrim(0),
    fNRecRef(0),
    fNRecSec(0),
    fNGhosts(0),
    fNClones(0),
    fNEvents(0),
    fNEventsFailed(0),
    fTime(0.),
    fTimer()
{}
// -------------------------------------------------------------------------



// -----   Destructor   ----------------------------------------------------
CbmStsFindTracksQa::~CbmStsFindTracksQa() {

  fHistoList->Delete();
  delete fHistoList;
}
// -------------------------------------------------------------------------



// -----   Task execution   ------------------------------------------------
void CbmStsFindTracksQa::Exec(Option_t* /*opt*/) {

  // If there is an event branch: do the event loop
  if ( fEvents ) {
      Int_t nEvents = fEvents->GetEntriesFast();
      LOG(debug) << GetName() << ": found time slice with " << nEvents
                 << " events.";

      for (Int_t iEvent = 0; iEvent < nEvents; iEvent++) {
          CbmEvent* event = dynamic_cast<CbmEvent*>(fEvents->At(iEvent));
          assert(event);
          ProcessEvent(event);
      }
  }

  // If there is no event branch, process the entire tree
  else {
    ProcessEvent();
  }

}
// -------------------------------------------------------------------------



// -----   Public method SetParContainers   --------------------------------
void CbmStsFindTracksQa::SetParContainers() {

  LOG(info) << GetName() << ": SetParContainers";

  // Get Run
  FairRunAna* run = FairRunAna::Instance();
  assert(run);

  // Get Runtime Database
  FairRuntimeDb* runDb = run->GetRuntimeDb();
  assert(runDb);

  // Get passive geometry parameters
  fPassGeo = (CbmGeoPassivePar*) runDb->getContainer("CbmGeoPassivePar");
  assert(fPassGeo);
}
// -------------------------------------------------------------------------



// -----   Public method Init   --------------------------------------------
InitStatus CbmStsFindTracksQa::Init() {

  LOG(info) << "\n\n====================================================";
  LOG(info) << GetName() << ": Initialising...";

  // Get RootManager
  FairRootManager* ioman = FairRootManager::Instance();
  assert(ioman);

  // Get STS setup
  fSetup = CbmStsSetup::Instance();

  // Get MCDataManager
  CbmMCDataManager* mcManager =
      dynamic_cast<CbmMCDataManager*>(ioman->GetObject("MCDataManager"));
  assert(mcManager);

  // Get MCTrack array
  fMCTracks = mcManager->InitBranch("MCTrack");
  assert(fMCTracks);

  // Get StsPoint array
  fStsPoints = mcManager->InitBranch("StsPoint");
  assert(fStsPoints);

  // Get Event array
  fEvents = dynamic_cast<TClonesArray*>(ioman->GetObject("Event"));

  // Get StsHit array
  fStsHits = (TClonesArray*) ioman->GetObject("StsHit");
  assert(fStsHits);

  // Get StsHitMatch array
  fStsHitMatch = (TClonesArray*) ioman->GetObject("StsHitMatch");
  assert(fStsHitMatch);

  // Get StsTrack array
  fStsTracks = (TClonesArray*) ioman->GetObject("StsTrack");
  assert(fStsTracks);

  // Get StsTrackMatch array
  fMatches = (TClonesArray*) ioman->GetObject("StsTrackMatch");
  assert(fMatches);


  // Get the geometry of target and STS
  InitStatus geoStatus = GetGeometry();
  if ( geoStatus != kSUCCESS ) {
    LOG(error) << GetName() << "::Init: Error in reading geometry!";
    return geoStatus;
  }

  // Create histograms
  CreateHistos();
  Reset();

  // Output
  LOG(info) << "   Number of STS stations : " << fNStations;
  LOG(info) << "   Target position ( " << fTargetPos.X() << ", "
       << fTargetPos.Y() << ", " << fTargetPos.Z() << ") cm";
  LOG(info) << "   Minimum number of STS stations   : " << fMinStations;
  LOG(info) << "   Matching quota               : " << fQuota;
  LOG(info) << "====================================================";

  return geoStatus;
}
// -------------------------------------------------------------------------



// -----   Public method ReInit   ------------------------------------------
InitStatus CbmStsFindTracksQa::ReInit() {

  LOG(info) << "\n\n====================================================";
  LOG(info) << GetName() << ": Re-initialising...";

  // Get the geometry of target and STS
  InitStatus geoStatus = GetGeometry();
  if ( geoStatus != kSUCCESS ) {
    LOG(error) << GetName() << "::Init: Error in reading geometry!";
    return geoStatus;
  }

  // --- Screen log
  LOG(info) << "   Number of STS stations : " << fNStations;
  LOG(info) << "   Target position ( " << fTargetPos.X() << ", "
       << fTargetPos.Y() << ", " << fTargetPos.Z() << ") cm";
  LOG(info) << "   Minimum number of STS stations   : " << fMinStations;
  LOG(info) << "   Matching quota               : " << fQuota;
  LOG(info) << "====================================================";

  return geoStatus;

}
// -------------------------------------------------------------------------



// -----   Public method Exec   --------------------------------------------
void CbmStsFindTracksQa::ProcessEvent(CbmEvent* event) {

  // --- Event number. Note that the FairRun counting start with 1.
  Int_t eventNumber = ( event ? event->GetNumber()
      : FairRun::Instance()->GetEventHeader()->GetMCEntryNumber() - 1);

  LOG(debug) << GetName() << ": Process event " << eventNumber;

  // Timer
  fTimer.Start();

  // Eventwise counters
//  Int_t nMCTracks = 0;
  Int_t nTracks   = 0;
  Int_t nGhosts   = 0;
  Int_t nClones   = 0;
  Int_t nAll      = 0;
  Int_t nAcc      = 0;
  Int_t nRecAll   = 0;
  Int_t nPrim     = 0;
  Int_t nRecPrim  = 0;
  Int_t nRef      = 0;
  Int_t nRecRef   = 0;
  Int_t nSec      = 0;
  Int_t nRecSec   = 0;
  TVector3 momentum;
  TVector3 vertex;

  // Fill hit and track maps
  FillHitMap(event);
  FillMatchMap(event, nTracks, nGhosts, nClones);

  // Loop over MCTracks
  Int_t nMcTracks = fMCTracks->Size(0, eventNumber);
  for (Int_t mcTrackId = 0; mcTrackId < nMcTracks; mcTrackId++) {
    CbmMCTrack* mcTrack =
        dynamic_cast<CbmMCTrack*>(fMCTracks->Get(0, eventNumber, mcTrackId));
    assert(mcTrack);

    // Continue only for reconstructible tracks
    nAll++;
    if ( fHitMap.find(mcTrackId) == fHitMap.end() ) continue; // No hits
    Int_t nStations = fHitMap[mcTrackId].size();
    if ( nStations < fMinStations ) continue;        // Too few stations
    nAcc++;

    // Check origin of MCTrack
    // TODO: Track origin should rather be compared to MC event vertex
    // But that is not available from MCDataManager
    mcTrack->GetStartVertex(vertex);
    Bool_t isPrim = kFALSE;
    if ( TMath::Abs( vertex.Z() - fTargetPos.Z() ) < 1. ) {
      isPrim = kTRUE;
      nPrim++;
    }
    else nSec++;

    // Get momentum
    mcTrack->GetMomentum(momentum);
    Double_t mom = momentum.Mag();
    Bool_t isRef = kFALSE;
    if ( mom > 1. && isPrim) {
      isRef = kTRUE;
      nRef++;
    }

    // Fill histograms for reconstructible tracks
    fhMomAccAll->Fill(mom);
    fhNpAccAll->Fill(Double_t(nStations));
    if ( isPrim) {
      fhMomAccPrim->Fill(mom);
      fhNpAccPrim->Fill(Double_t(nStations));
    }
    else {
      fhMomAccSec->Fill(mom);
      fhNpAccSec->Fill(Double_t(nStations));
      fhZAccSec->Fill(vertex.Z());
    }

    // Get matched StsTrack
    Int_t    trackId  = -1;
    Double_t quali    =  0.;
//    Bool_t   isRec    = kFALSE;
    if (fMatchMap.find(mcTrackId) != fMatchMap.end() ) {
      trackId  = fMatchMap[mcTrackId];
//      isRec = kTRUE;
      CbmStsTrack* stsTrack = (CbmStsTrack*) fStsTracks->At(trackId);
      assert(stsTrack);
      quali = fQualiMap[mcTrackId];
      assert ( quali >= fQuota );
      CbmTrackMatchNew* match = (CbmTrackMatchNew*) fMatches->At(trackId);
      assert(match);
      Int_t nTrue  = match->GetNofTrueHits();
      Int_t nWrong = match->GetNofWrongHits();
      //Int_t nFake  = match->GetNofFakeHits();
      Int_t nFake  = 0;
      Int_t nAllHits  = stsTrack->GetNofStsHits();
      assert ( nTrue + nWrong + nFake == nAllHits );

      // Verbose output
      LOG(debug1) << GetName() << ": MCTrack " << mcTrackId << ", stations "
          << nStations << ", hits " << nAllHits << ", true hits " << nTrue;

      // Fill histograms for reconstructed tracks
      nRecAll++;
      fhMomRecAll->Fill(mom);
      fhNpRecAll->Fill(Double_t(nAllHits));
      if ( isPrim ) {
	nRecPrim++;
	fhMomRecPrim->Fill(mom);
	fhNpRecPrim->Fill(Double_t(nAllHits));
	if ( isRef ) nRecRef++;
      }
      else {
	nRecSec++;
	fhMomRecSec->Fill(mom);
	fhNpRecSec->Fill(Double_t(nAllHits));
	fhZRecSec->Fill(vertex.Z());
      }

    }  // Match found in map?

  } // Loop over MCTracks


  // Calculate efficiencies
  Double_t effAll  = Double_t(nRecAll)  / Double_t(nAcc);
  Double_t effPrim = Double_t(nRecPrim) / Double_t(nPrim);
  Double_t effRef  = Double_t(nRecRef)  / Double_t(nRef);
  Double_t effSec  = Double_t(nRecSec)  / Double_t(nSec);

  fTimer.Stop();


  // Event summary
  LOG(info) << "+ " << setw(20) << GetName() << ": Event " << setw(6)
    << right << fNEvents << ", real time " << fixed << setprecision(6)
    << fTimer.RealTime() << " s, MC tracks: all " << nMcTracks
    << ", acc. " << nAcc << ", rec. " << nRecAll << ", eff. "
    << setprecision(2) << 100.*effAll << " %";
  if ( FairLogger::GetLogger()->IsLogNeeded(fair::Severity::debug) ) {
    LOG(debug) << "----------   StsFindTracksQa : Event summary   ------------";
    LOG(debug) << "MCTracks   : " << nAll << ", reconstructible: " << nAcc
	 << ", reconstructed: " << nRecAll;
    LOG(debug) << "Vertex     : reconstructible: " << nPrim << ", reconstructed: "
	 << nRecPrim << ", efficiency " << effPrim*100. << "%";
    LOG(debug) << "Reference  : reconstructible: " << nRef  << ", reconstructed: "
	 << nRecRef  << ", efficiency " << effRef*100. << "%";
    LOG(debug) << "Non-vertex : reconstructible: " << nSec << ", reconstructed: "
	 << nRecSec << ", efficiency " << effSec*100. << "%";
    LOG(debug) << "STSTracks " << nTracks << ", ghosts " << nGhosts
	 << ", clones " << nClones;
    LOG(debug) << "-----------------------------------------------------------\n";
  }


  // Increase counters
  fNAccAll  += nAcc;
  fNAccPrim += nPrim;
  fNAccRef  += nRef;
  fNAccSec  += nSec;
  fNRecAll  += nRecAll;
  fNRecPrim += nRecPrim;
  fNRecRef  += nRecRef;
  fNRecSec  += nRecSec;
  fNGhosts  += nGhosts;
  fNClones  += nClones;
  fNEvents++;
  fTime += fTimer.RealTime();

}
// -------------------------------------------------------------------------



// -----   Private method Finish   -----------------------------------------
void CbmStsFindTracksQa::Finish() {

  // Divide histograms for efficiency calculation
  DivideHistos(fhMomRecAll,  fhMomAccAll,  fhMomEffAll);
  DivideHistos(fhMomRecPrim, fhMomAccPrim, fhMomEffPrim);
  DivideHistos(fhMomRecSec,  fhMomAccSec,  fhMomEffSec);
  DivideHistos(fhNpRecAll,   fhNpAccAll,   fhNpEffAll);
  DivideHistos(fhNpRecPrim,  fhNpAccPrim,  fhNpEffPrim);
  DivideHistos(fhNpRecSec,   fhNpAccSec,   fhNpEffSec);
  DivideHistos(fhZRecSec,    fhZAccSec,    fhZEffSec);

  // Normalise histos for clones and ghosts to one event
  if ( fNEvents ) {
    fhNhClones->Scale(1./Double_t(fNEvents));
    fhNhGhosts->Scale(1./Double_t(fNEvents));
  }

  // Calculate integrated efficiencies and rates
  Double_t effAll  = Double_t(fNRecAll)  / Double_t(fNAccAll);
  Double_t effPrim = Double_t(fNRecPrim) / Double_t(fNAccPrim);
  Double_t effRef  = Double_t(fNRecRef)  / Double_t(fNAccRef);
  Double_t effSec  = Double_t(fNRecSec)  / Double_t(fNAccSec);
  Double_t rateGhosts = Double_t(fNGhosts) / Double_t(fNEvents);
  Double_t rateClones = Double_t(fNClones) / Double_t(fNEvents);

  // Run summary to screen
  std::cout << std::endl;
  LOG(info) << "=====================================";
  LOG(info) << fName << ": Run summary ";
  LOG(info) << "Events processed      : " << fNEvents
      << setprecision(2);
  LOG(info) << "Eff. all tracks       : " << effAll*100 << " % ("
       << fNRecAll << "/" << fNAccAll <<")";
  LOG(info) << "Eff. vertex tracks    : " << effPrim*100 << " % ("
       << fNRecPrim << "/" << fNAccPrim <<")";
  LOG(info) << "Eff. reference tracks : " << effRef*100 << " % ("
       << fNRecRef << "/" << fNAccRef <<")";
  LOG(info) << "Eff. secondary tracks : " << effSec*100 << " % ("
       << fNRecSec << "/" << fNAccSec <<")";
  LOG(info) << "Ghost rate            : " << rateGhosts
      << " per event";
  LOG(info) << "Clone rate            : " << rateClones
      << " per event";
  LOG(info) << "Time per event        : " << setprecision(6)
    << fTime / Double_t(fNEvents)  << " s";
  LOG(info) << "=====================================";

  // Write histos to output
  gDirectory->mkdir("STSFindTracksQA");
  gDirectory->cd("STSFindTracksQA");
  TIter next(fHistoList);
  while ( TH1* histo = ((TH1*)next()) ) histo->Write();
  gDirectory->cd("..");
}
// -------------------------------------------------------------------------



// -----   Private method GetGeometry   ------------------------------------
InitStatus CbmStsFindTracksQa::GetGeometry() {

  // Get target geometry
  GetTargetPosition();

  fNStations = CbmStsSetup::Instance()->GetNofStations();


  return kSUCCESS;

}
// -------------------------------------------------------------------------


// -----   Get target node   -----------------------------------------------
void CbmStsFindTracksQa::GetTargetPosition() {

  TGeoNode* target=NULL;

  gGeoManager->CdTop();
  TGeoNode* cave = gGeoManager->GetCurrentNode();
  for (Int_t iNode1 = 0; iNode1 < cave->GetNdaughters(); iNode1++) {
	  TString name = cave->GetDaughter(iNode1)->GetName();
	  if ( name.Contains("pipe", TString::kIgnoreCase) ) {
		  LOG(debug) << "Found pipe node " << name;
		  gGeoManager->CdDown(iNode1);
		  break;
	  }
  }
  for (Int_t iNode2 = 0; iNode2 < gGeoManager->GetCurrentNode()->GetNdaughters(); iNode2++) {
	  TString name = gGeoManager->GetCurrentNode()->GetDaughter(iNode2)->GetName();
	  if ( name.Contains("pipevac1", TString::kIgnoreCase) ) {
		  LOG(debug) << "Found vacuum node " << name;
		  gGeoManager->CdDown(iNode2);
		  break;
	  }
  }
  for (Int_t iNode3 = 0; iNode3 < gGeoManager->GetCurrentNode()->GetNdaughters(); iNode3++) {
	  TString name = gGeoManager->GetCurrentNode()->GetDaughter(iNode3)->GetName();
	  if ( name.Contains("target", TString::kIgnoreCase) ) {
		  LOG(debug) << "Found target node " << name;
		  gGeoManager->CdDown(iNode3);
		  target = gGeoManager->GetCurrentNode();
		  break;
	  }
  }
  if ( ! target ) {
	  fTargetPos[0] = 0.;
	  fTargetPos[1] = 0.;
	  fTargetPos[2] = 0.;
  } else {
          TGeoHMatrix* glbMatrix = gGeoManager->GetCurrentMatrix();
          Double_t* pos = glbMatrix->GetTranslation();
          fTargetPos[0] = pos[0];
          fTargetPos[1] = pos[1];
          fTargetPos[2] = pos[2];
  }

  gGeoManager->CdTop();
}
// -------------------------------------------------------------------------







// -----   Private method CreateHistos   -----------------------------------
void CbmStsFindTracksQa::CreateHistos() {

  // Histogram list
  fHistoList = new TList();

  // Momentum distributions
  Double_t minMom   =  0.;
  Double_t maxMom   = 10.;
  Int_t    nBinsMom = 40;
  fhMomAccAll  = new TH1F("hMomAccAll", "all reconstructable tracks",
			 nBinsMom, minMom, maxMom);
  fhMomRecAll  = new TH1F("hMomRecAll", "all reconstructed tracks",
			 nBinsMom, minMom, maxMom);
  fhMomEffAll  = new TH1F("hMomEffAll", "efficiency all tracks",
			 nBinsMom, minMom, maxMom);
  fhMomAccPrim = new TH1F("hMomAccPrim", "reconstructable vertex tracks",
			 nBinsMom, minMom, maxMom);
  fhMomRecPrim = new TH1F("hMomRecPrim", "reconstructed vertex tracks",
			 nBinsMom, minMom, maxMom);
  fhMomEffPrim = new TH1F("hMomEffPrim", "efficiency vertex tracks",
			 nBinsMom, minMom, maxMom);
  fhMomAccSec  = new TH1F("hMomAccSec", "reconstructable non-vertex tracks",
			 nBinsMom, minMom, maxMom);
  fhMomRecSec  = new TH1F("hMomRecSec", "reconstructed non-vertex tracks",
			 nBinsMom, minMom, maxMom);
  fhMomEffSec  = new TH1F("hMomEffSec", "efficiency non-vertex tracks",
			 nBinsMom, minMom, maxMom);
  fHistoList->Add(fhMomAccAll);
  fHistoList->Add(fhMomRecAll);
  fHistoList->Add(fhMomEffAll);
  fHistoList->Add(fhMomAccPrim);
  fHistoList->Add(fhMomRecPrim);
  fHistoList->Add(fhMomEffPrim);
  fHistoList->Add(fhMomAccSec);
  fHistoList->Add(fhMomRecSec);
  fHistoList->Add(fhMomEffSec);

  // Number-of-points distributions
  Double_t minNp   = -0.5;
  Double_t maxNp   = 15.5;
  Int_t    nBinsNp = 16;
  fhNpAccAll  = new TH1F("hNpAccAll", "all reconstructable tracks",
			 nBinsNp, minNp, maxNp);
  fhNpRecAll  = new TH1F("hNpRecAll", "all reconstructed tracks",
			 nBinsNp, minNp, maxNp);
  fhNpEffAll  = new TH1F("hNpEffAll", "efficiency all tracks",
			 nBinsNp, minNp, maxNp);
  fhNpAccPrim = new TH1F("hNpAccPrim", "reconstructable vertex tracks",
			 nBinsNp, minNp, maxNp);
  fhNpRecPrim = new TH1F("hNpRecPrim", "reconstructed vertex tracks",
			 nBinsNp, minNp, maxNp);
  fhNpEffPrim = new TH1F("hNpEffPrim", "efficiency vertex tracks",
			 nBinsNp, minNp, maxNp);
  fhNpAccSec  = new TH1F("hNpAccSec", "reconstructable non-vertex tracks",
			 nBinsNp, minNp, maxNp);
  fhNpRecSec  = new TH1F("hNpRecSec", "reconstructed non-vertex tracks",
			 nBinsNp, minNp, maxNp);
  fhNpEffSec  = new TH1F("hNpEffSec", "efficiency non-vertex tracks",
			 nBinsNp, minNp, maxNp);
  fHistoList->Add(fhNpAccAll);
  fHistoList->Add(fhNpRecAll);
  fHistoList->Add(fhNpEffAll);
  fHistoList->Add(fhNpAccPrim);
  fHistoList->Add(fhNpRecPrim);
  fHistoList->Add(fhNpEffPrim);
  fHistoList->Add(fhNpAccSec);
  fHistoList->Add(fhNpRecSec);
  fHistoList->Add(fhNpEffSec);

  // z(vertex) distributions
  Double_t minZ    =  0.;
  Double_t maxZ    = 50.;
  Int_t    nBinsZ  = 50;
  fhZAccSec = new TH1F("hZAccSec", "reconstructable non-vertex tracks",
			 nBinsZ, minZ, maxZ);
  fhZRecSec = new TH1F("hZRecSecl", "reconstructed non-vertex tracks",
			 nBinsZ, minZ, maxZ);
  fhZEffSec = new TH1F("hZEffRec", "efficiency non-vertex tracks",
			 nBinsZ, minZ, maxZ);
  fHistoList->Add(fhZAccSec);
  fHistoList->Add(fhZRecSec);
  fHistoList->Add(fhZEffSec);

  // Number-of-hit distributions
  fhNhClones  = new TH1F("hNhClones", "number of hits for clones",
			 nBinsNp, minNp, maxNp);
  fhNhGhosts  = new TH1F("hNhGhosts", "number of hits for ghosts",
			 nBinsNp, minNp, maxNp);
  fHistoList->Add(fhNhClones);
  fHistoList->Add(fhNhGhosts);

}
// -------------------------------------------------------------------------



// -----   Private method Reset   ------------------------------------------
void CbmStsFindTracksQa::Reset() {

  TIter next(fHistoList);
  while ( TH1* histo = ((TH1*)next()) ) histo->Reset();

  fNAccAll = fNAccPrim = fNAccRef = fNAccSec = 0;
  fNRecAll = fNRecPrim = fNRecRef = fNRecSec = 0;
  fNGhosts = fNClones = fNEvents = 0;

}
// -------------------------------------------------------------------------



// -----   Private method FillHitMap   -------------------------------------
void CbmStsFindTracksQa::FillHitMap(CbmEvent* event) {

  // --- Event number. Note that the FairRun counting starts with 1.
  Int_t eventNumber = ( event ? event->GetNumber()
      : FairRun::Instance()->GetEventHeader()->GetMCEntryNumber() - 1 );

  // --- Fill hit map ( mcTrack -> ( station -> number of hits ) )
  fHitMap.clear();
  Int_t nHits = (event ? event->GetNofData(kStsHit)
      : fStsHits->GetEntriesFast());
  for (Int_t iHit = 0; iHit < nHits; iHit++) {
    Int_t hitIndex = (event ? event->GetIndex(kStsHit, iHit) : iHit);
    CbmStsHit* hit = (CbmStsHit*) fStsHits->At(hitIndex);
    CbmMatch* hitMatch = (CbmMatch*) fStsHitMatch->At(hitIndex);
    Int_t pointIndex = hitMatch->GetMatchedLink().GetIndex();
    assert(pointIndex >= 0);
    CbmStsPoint* stsPoint =
        dynamic_cast<CbmStsPoint*>(fStsPoints->Get(0, eventNumber, pointIndex));
    assert(stsPoint);
    Int_t mcTrackIndex = stsPoint->GetTrackID();
    Int_t station = fSetup->GetStationNumber(hit->GetAddress());
    fHitMap[mcTrackIndex][station]++;
  }
  LOG(debug) << GetName() << ": Filled hit map from " << nHits
      << " STS hits for " << fHitMap.size() << " MCTracks.";
}
// -------------------------------------------------------------------------



// ------   Private method FillMatchMap   ----------------------------------
void CbmStsFindTracksQa::FillMatchMap(CbmEvent* event, Int_t& nRec,
                                      Int_t& nGhosts, Int_t& nClones) {

  // Clear matching maps
  fMatchMap.clear();
  fQualiMap.clear();

  // Loop over StsTracks. Check matched MCtrack and fill maps.
  nGhosts = 0;
  nClones = 0;
  Int_t nTracks  = (event ? event->GetNofData(kStsTrack)
                          : fStsTracks->GetEntriesFast());

  for (Int_t iTrack = 0; iTrack < nTracks; iTrack++) {

    // --- StsTrack
    Int_t trackIndex = (event ? event->GetIndex(kStsTrack, iTrack)
                              : iTrack);
    CbmStsTrack* stsTrack = (CbmStsTrack*) fStsTracks->At(trackIndex);
    assert(stsTrack);
    Int_t nHits = stsTrack->GetNofStsHits();

    // --- TrackMatch
    CbmTrackMatchNew* match = (CbmTrackMatchNew*) fMatches->At(trackIndex);
    assert(match);
    Int_t nTrue = match->GetNofTrueHits();

    // --- Matched MCTrack
    Int_t mcTrackId = -1;
    if ( nTrue > 0 ) mcTrackId = match->GetMatchedLink().GetIndex();
    if ( mcTrackId < 0 ) {
      fhNhGhosts->Fill(nHits);
      nGhosts++;
      continue;
    }

    // Check matching criterion (quota)
    Double_t quali = Double_t(nTrue) / Double_t(nHits);
    if ( quali >= fQuota ) {

      // No previous match for this MCTrack
      if ( fMatchMap.find(mcTrackId) == fMatchMap.end() ) {
        fMatchMap[mcTrackId] = iTrack;
        fQualiMap[mcTrackId] = quali;
      } //? no previous match

      // Previous match; take the better one
      else {
        if ( fQualiMap[mcTrackId] < quali ) {
          CbmStsTrack* oldTrack
          = (CbmStsTrack*) fStsTracks->At(fMatchMap[mcTrackId]);
          fhNhClones->Fill(Double_t(oldTrack->GetNofStsHits()));
          fMatchMap[mcTrackId] = iTrack;
          fQualiMap[mcTrackId] = quali;
        } //? new track matches better to MCTrack

        else fhNhClones->Fill(nHits);
        nClones++;
      } //? previous match found

    } //? true match ratio > quota

    // If not matched, it's a ghost
    else {
      fhNhGhosts->Fill(nHits);
      nGhosts++;
    }

  }   // Loop over StsTracks
  nRec = nTracks;
  LOG(debug) << GetName() << ": Filled match map for " << nRec
      << " STS tracks. Ghosts " << nGhosts << " Clones " << nClones;
}
// -------------------------------------------------------------------------



// -----   Private method DivideHistos   -----------------------------------
void CbmStsFindTracksQa::DivideHistos(TH1* histo1, TH1* histo2,
				      TH1* histo3) {

  if ( !histo1 || !histo2 || !histo3 ) {
    LOG(fatal) << GetName() << "::DivideHistos: "
	 << "NULL histogram pointer";
  }

  Int_t nBins = histo1->GetNbinsX();
  if ( histo2->GetNbinsX() != nBins || histo3->GetNbinsX() != nBins ) {
    LOG(error) << GetName() << "::DivideHistos: "
	 << "Different bin numbers in histos";
    LOG(error) << histo1->GetName() << " " << histo1->GetNbinsX();
    LOG(error) << histo2->GetName() << " " << histo2->GetNbinsX();
    LOG(error) << histo3->GetName() << " " << histo3->GetNbinsX();
   return;
  }

  Double_t c1, c2, c3, ce;
  for (Int_t iBin=0; iBin<nBins; iBin++) {
    c1 = histo1->GetBinContent(iBin);
    c2 = histo2->GetBinContent(iBin);
    if ( c2 != 0. ) {
      c3 = c1 / c2;
      Double_t c4=(c3 * ( 1. - c3 ) / c2);
      if ( c4 >= 0.) {
        ce = TMath::Sqrt( c3 * ( 1. - c3 ) / c2 );
      } else {
       ce=0;
      }
    }
    else {
      c3 = 0.;
      ce = 0.;
    }
    histo3->SetBinContent(iBin, c3);
    histo3->SetBinError(iBin, ce);
  }

}
// -------------------------------------------------------------------------



ClassImp(CbmStsFindTracksQa)
