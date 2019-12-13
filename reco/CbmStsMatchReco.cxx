/** @file CbmStsMatchReco.cxx
 ** @author Volker Friese <v.friese@gsi.de>
 ** @date 6.12.2016
 **
 ** Based on CbmMatchRecoToMC by A. Lebedev.
 **/

#include <CbmStsMatchReco.h>

#include <cassert>
#include <iomanip>
#include "TClonesArray.h"
#include "TStopwatch.h"
#include "FairLogger.h"
#include "FairMCPoint.h"
#include "CbmCluster.h"
#include "CbmDigi.h"
#include "CbmDigiManager.h"
#include "CbmLink.h"
#include "CbmMCDataArray.h"
#include "CbmMCDataManager.h"
#include "CbmMCTrack.h"
#include "CbmMatch.h"
#include "CbmStsDigi.h"
#include "CbmStsHit.h"
#include "CbmStsTrack.h"
#include "CbmTrackMatchNew.h"


using namespace std;


// -----   Constructor   ---------------------------------------------------
CbmStsMatchReco::CbmStsMatchReco() :
   FairTask(),
	 fDigiManager(nullptr),
   fMCTracks(NULL),
   fPoints(NULL),
   fDigiMatches(nullptr),
   fClusters(NULL),
   fHits(NULL),
   fTracks(NULL)
{
}
// -------------------------------------------------------------------------



// -----   Destructor   ----------------------------------------------------
CbmStsMatchReco::~CbmStsMatchReco()
{
}
// -------------------------------------------------------------------------



// -----   Task execution   ------------------------------------------------
void CbmStsMatchReco::Exec(Option_t* /*opt*/) {

	 // --- Time
	TStopwatch timer;
	timer.Start();

   // --- Construct cluster match from digi matches
   if ( fClusters ) MatchClusters(fClusters);

   // --- Construct hit match from cluster matches
   if ( fClusters && fHits ) MatchHits(fClusters, fHits);

   // --- Construct track match from hit matches
   if ( fHits && fTracks ) MatchTracks(fHits, fTracks, fPoints);

   timer.Stop();
   static Int_t nEntries = 0;
   nEntries++;

   LOG(info) << "+ " << setw(20) << GetName() << ": Entry " << setw(6)
   		      << right << nEntries
   		      << ", real time " << fixed << setprecision(6)
   		      << timer.RealTime() << " s ";
}
// -------------------------------------------------------------------------



// -----   End-of-run action   ---------------------------------------------
void CbmStsMatchReco::Finish()
{
}
// -------------------------------------------------------------------------



// -----   Initialisation   ------------------------------------------------
InitStatus CbmStsMatchReco::Init()
{
	// --- I/O manager
  FairRootManager* ioman = FairRootManager::Instance();
  if (! ioman) {
     LOG(fatal) << GetName() << ": No FairRootManager";
     return kFATAL;
  }

  // --- MC data manager
  CbmMCDataManager* mcManager =
  		(CbmMCDataManager*)ioman->GetObject("MCDataManager");

  // --- Digi manager
  fDigiManager = CbmDigiManager::Instance();
  fDigiManager->Init();

  // --- Data arrays
  fMCTracks = mcManager->InitBranch("MCTrack");
  fPoints   = mcManager->InitBranch("StsPoint");
  fClusters = (TClonesArray*) ioman->GetObject("StsCluster");
  fHits     = (TClonesArray*) ioman->GetObject("StsHit");
  fTracks   = (TClonesArray*) ioman->GetObject("StsTrack");

  return kSUCCESS;
}
// -------------------------------------------------------------------------



// -----   Match clusters   ------------------------------------------------
void CbmStsMatchReco::MatchClusters(const TClonesArray* clusters) {

	assert (clusters);

	Int_t nClusters = clusters->GetEntriesFast();
  for (Int_t iCluster = 0; iCluster < nClusters; iCluster++) {
      CbmCluster* cluster = static_cast<CbmCluster*>(clusters->At(iCluster));
      CbmMatch* clusterMatch = new CbmMatch();
      Int_t nDigis = cluster->GetNofDigis();
      for (Int_t iDigi = 0; iDigi < nDigis; iDigi++) {
      	const CbmDigi* digi = fDigiManager->Get<CbmStsDigi>(iDigi);
      	assert(digi);
      	const CbmMatch* digiMatch = fDigiManager->GetMatch(kSts, iDigi);
      	assert(digiMatch);
        clusterMatch->AddLinks(*digiMatch);
      }  //# digis
      cluster->SetMatch(clusterMatch);
   } //# clusters

}
// -------------------------------------------------------------------------



// ----   Match hits   -----------------------------------------------------
void CbmStsMatchReco::MatchHits(const TClonesArray* clusters,
      const TClonesArray* hits) {

	assert(clusters);
	assert(hits);

  Int_t nHits = hits->GetEntriesFast();
  for (Int_t iHit = 0; iHit < nHits; iHit++) {
      CbmStsHit* hit = static_cast<CbmStsHit*>(hits->At(iHit));
      assert(hit);
      CbmMatch* hitMatch = new CbmMatch();
      Int_t indexClusterF = hit->GetFrontClusterId();
      CbmCluster* clusterF =
      		static_cast<CbmCluster*>(clusters->At(indexClusterF));
      assert(clusterF);
      hitMatch->AddLinks(*(clusterF->GetMatch()));
      Int_t indexClusterB = hit->GetBackClusterId();
      CbmCluster* clusterB =
      		static_cast<CbmCluster*>(clusters->At(indexClusterB));
      assert(clusterB);
      hitMatch->AddLinks(*(clusterB->GetMatch()));
      hit->SetMatch(hitMatch);
   } //# hits

}
// -------------------------------------------------------------------------



// -----   Match tracks   --------------------------------------------------
void CbmStsMatchReco::MatchTracks(const TClonesArray* hits,
		const TClonesArray* tracks, CbmMCDataArray* points) {

	assert(hits);
	assert(points);
	assert(tracks);

	Int_t nTracks = tracks->GetEntriesFast();
	for (Int_t iTrack = 0; iTrack < nTracks; iTrack++) {
		CbmStsTrack* track = static_cast<CbmStsTrack*>(tracks->At(iTrack));
		assert(track);
		CbmTrackMatchNew* trackMatch = new CbmTrackMatchNew();

		Int_t nHits = track->GetNofStsHits();
		for (Int_t iHit = 0; iHit < nHits; iHit++) {
			Int_t hitId = track->GetHitIndex(iHit);
			CbmStsHit* hit = static_cast<CbmStsHit*>(hits->At(hitId));
			assert(hit);
			CbmMatch* hitMatch = hit->GetMatch();

			for (Int_t iLink = 0; iLink < hitMatch->GetNofLinks(); iLink++) {
				const CbmLink& link = hitMatch->GetLink(iLink);
				const FairMCPoint* point = static_cast<const FairMCPoint*>(points->Get(
						link));
				Int_t mcTrackId = point->GetTrackID();

				// --- Do not link MCTracks with only one STS point
				CbmMCTrack* mcTrack = static_cast<CbmMCTrack*>(fMCTracks->Get(
						link.GetFile(), link.GetEntry(), mcTrackId));
				assert(mcTrack);
				if (mcTrack->GetNPoints(kSts) < 2) continue;

				// --- Link each MCTrack with weight = 1
				trackMatch->AddLink(1., mcTrackId, link.GetEntry(), link.GetFile());

			}  //# links of hit

		} //# hits on track

		// Calculate number of true and wrong hits on track
	  if ( ! trackMatch->GetNofLinks() ) continue;
	  Int_t matchedTrackId = trackMatch->GetMatchedLink().GetIndex();
	  Int_t nTrue  = 0;
	  Int_t nWrong = 0;
    for (Int_t iHit = 0; iHit < nHits; iHit++) {
    	CbmHit* hit = static_cast<CbmHit*>(hits->At(iHit));
      CbmMatch* hitMatch = hit->GetMatch();
      Int_t nLinks = hitMatch->GetNofLinks();
      Bool_t hasTrueLink = kFALSE;
      for (Int_t iLink = 0; iLink < nLinks; iLink++) {
      	const CbmLink& link = hitMatch->GetLink(iLink);
      	FairMCPoint* point = static_cast<FairMCPoint*>(points->Get(link));
      	assert(point);
      	if ( point->GetTrackID() == matchedTrackId ) {
      		hasTrueLink = kTRUE;
      		break;
      	} //? a true link of the hit
      } //# links of hit
      if ( hasTrueLink ) nTrue++;
      else               nWrong++;
    } //# hits on track
	  trackMatch->SetNofTrueHits(nTrue);
	  trackMatch->SetNofWrongHits(nWrong);

	  // Set the match object to the track
	  track->SetMatch(trackMatch);

	} //# tracks

}
// -------------------------------------------------------------------------


ClassImp(CbmStsMatchReco);
