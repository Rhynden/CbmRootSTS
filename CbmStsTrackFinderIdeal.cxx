// -------------------------------------------------------------------------
// -----                CbmStsTrackFinderIdeal source file             -----
// -----                  Created 01/02/05  by V. Friese               -----
// -------------------------------------------------------------------------

#include "CbmStsTrackFinderIdeal.h"

#include "CbmStsHit.h"
#include "CbmStsTrack.h"

// CBM includes
#include "FairMCPoint.h"
#include "CbmMCTrack.h"
#include "FairRootManager.h"

// ROOT includes
#include "TClonesArray.h"

// C++ includes
#include <iostream>
#include <iomanip>
#include <map>

using std::cout;
using std::endl;
using std::map;
using std::setw;
using std::left;


// -----   Default constructor   -------------------------------------------
CbmStsTrackFinderIdeal::CbmStsTrackFinderIdeal() 
  : CbmStsTrackFinder(),
    fMCTrackArray(NULL),
    fMCPointArray(NULL)
{ 
  fVerbose      = 1;
  fName         = "STSTrackFinderIdeal";
}
// -------------------------------------------------------------------------



// -----   Standard constructor   ------------------------------------------
CbmStsTrackFinderIdeal::CbmStsTrackFinderIdeal(Int_t verbose) 
  : CbmStsTrackFinder(),
    fMCTrackArray(NULL),
    fMCPointArray(NULL)
{ 
  fVerbose      = verbose;
  fName         = "STSTrackFinderIdeal";
}
// -------------------------------------------------------------------------
 


// -----   Destructor   ----------------------------------------------------
CbmStsTrackFinderIdeal::~CbmStsTrackFinderIdeal() { }
// -------------------------------------------------------------------------



// -----   Public method Init   --------------------------------------------
void CbmStsTrackFinderIdeal::Init() {

  // Get and check FairRootManager
  FairRootManager* ioman = FairRootManager::Instance();
  if (! ioman) {
    cout << "-E- "<< fName << "::Init: "
	 << "RootManager not instantised!" << endl;
    return;
  }

  // Get MCTrack array
  fMCTrackArray  = (TClonesArray*) ioman->GetObject("MCTrack");
  if ( ! fMCTrackArray) {
    cout << "-E- " << fName << "::Init: No MCTrack array!"
	 << endl;
    return;
  }

  // Get MCPoint array
  fMCPointArray  = (TClonesArray*) ioman->GetObject("StsPoint");
  if ( ! fMCPointArray) {
    cout << "-E- " << fName << "::Init: No MCPoint array!"
	 << endl;
    return;
  }

}
// -------------------------------------------------------------------------



// -----   Public method DoFind   ------------------------------------------
Int_t CbmStsTrackFinderIdeal::DoFind() {

  Bool_t success = kTRUE;

  // Check pointers
  if ( !fMCTrackArray ) {
    cout << "-E- " << fName << "::DoFind: "
	 << "MCTrack array missing! " << endl;
    return -1;
  }
  if ( !fMCPointArray ) {
    cout << "-E- " << fName << "::DoFind: "
	 << "MCPoint array missing! " << endl;
    return -1;
  }
    
  if ( !fStsHits ) {
    cout << "-E- " << fName << "::DoFind: "
	 << "Hit array missing! " << endl;
    return -1;
  }

  if ( !fTracks ) {
    cout << "-E- " << fName << "::DoFind: "
	 << "Track array missing! " << endl;
    return -1;
  }
    
  // Initialise control counters
  Int_t nNoMCTrack   = 0;
  Int_t nNoTrack     = 0;
  Int_t nNoStsPoint  = 0;
  Int_t nNoStsHit    = 0;

  // Create pointers to StsHit and StsPoint
  CbmStsHit*   pHit  = NULL;
  FairMCPoint*  pMCpt = NULL;
  CbmMCTrack*  pMCtr = NULL;
  CbmStsTrack* pTrck = NULL;

  // Number of Sts hits
  Int_t nHits   = fStsHits->GetEntriesFast();;

  // Declare some variables outside the loops
  Int_t ptIndex      = 0;     // MCPoint index
  Int_t mcTrackIndex = 0;     // MCTrack index
  Int_t trackIndex   = 0;     // StsTrack index


  // Create STL map from MCtrack index to number of valid StsHits
  map<Int_t, Int_t> hitMap;

  // Loop over Sts hits
  for (Int_t iHit = 0; iHit<nHits; iHit++) {
    pHit = (CbmStsHit*) fStsHits->At(iHit);
    if ( ! pHit ) continue;
    ptIndex = pHit->GetRefId();
    if (ptIndex < 0) continue;           // fake or background hit
    pMCpt = (FairMCPoint*) (fMCPointArray->At(ptIndex));
    if ( ! pMCpt ) continue;
    mcTrackIndex = pMCpt->GetTrackID();
    hitMap[mcTrackIndex]++;
  }   

  // Create STL map from MCTrack index to StsTrack index
  map<Int_t, Int_t> trackMap;

  // Create StsTracks for reconstructable MCTracks
  Int_t nMCacc  = 0;         // accepted MCTracks
  Int_t nTracks = 0;         // reconstructable MCTracks
  Int_t nMCTracks = fMCTrackArray->GetEntriesFast();
  for (Int_t iMCTrack=0; iMCTrack<nMCTracks; iMCTrack++) {
    pMCtr = (CbmMCTrack*) fMCTrackArray->At(iMCTrack);
    if ( ! pMCtr ) continue;
    if ( pMCtr->GetNPoints(kSts) < 3 ) continue;
    nMCacc++;
    if ( hitMap[iMCTrack] < 3 ) continue;
    new((*fTracks)[nTracks]) CbmStsTrack();
    if (fVerbose>1) cout << "-I- " << fName << ": StsTrack " 
			 << nTracks << " created from MCTrack " 
			 << iMCTrack << " (" << pMCtr->GetNPoints(kSts)
			 << " StsPoints)" << endl;
    trackMap[iMCTrack] = nTracks++;
  }

  if (fVerbose>2) cout << "-I- " << GetName() << ": " << endl;

  // Loop over Sts hits. Get corresponding MCPoint and MCTrack index
  for (Int_t iHit = 0; iHit<nHits; iHit++) {
    pHit = (CbmStsHit*) fStsHits->At(iHit);
    if ( ! pHit ) {
      cout << "-E- " << fName << "::DoFind: Empty slot "
	   << "in StsHitArray at position " << iHit << endl;
      nNoStsHit++;
      success = kFALSE;
      continue;
    }
    ptIndex = pHit->GetRefId();
    if (ptIndex < 0) continue;           // fake or background hit
    pMCpt = (FairMCPoint*) (fMCPointArray->At(ptIndex));
    if ( ! pMCpt ) {
      cout << "-E- " << fName << "::DoFind: No reference "
	   << "MCPoint " << ptIndex << " for hit " << iHit << endl;
      nNoStsPoint++;
      success = kFALSE;
      continue;
    }
    mcTrackIndex = pMCpt->GetTrackID();
    if (mcTrackIndex<0 || mcTrackIndex>nMCTracks) {
      cout << "-E- " << fName << "::DoFind: "
	   << "MCTrack index out of range. " << mcTrackIndex << " "
	   << nMCTracks << endl;
      nNoMCTrack++;
      success = kFALSE;
      continue;
    }
    if (trackMap.find(mcTrackIndex) == trackMap.end()) continue;
    trackIndex = trackMap[mcTrackIndex];
    pTrck = (CbmStsTrack*) fTracks->At(trackIndex);
    if ( ! pTrck ) {
      cout << "-E- " << fName << "::DoFind: "
	   << "No StsTrack pointer. " << iHit << " " << ptIndex 
	   << " " << mcTrackIndex << " " << trackIndex << endl;
      nNoTrack++;
      success = kFALSE;
      continue;
    }
    pTrck->AddHit(iHit, kSTSHIT);
    if (fVerbose>2) cout << "Sts Hit " << iHit << " from StsPoint "
			 << ptIndex << " (MCTrack "
			 << mcTrackIndex << ") added to StsTrack " 
			 << trackIndex << endl;
  }


  if (fVerbose) {
    cout << endl;
    cout << "-------------------------------------------------------" 
	 << endl;
    cout << "-I-    " << fName << endl; 
    cout << "Sts hits: " << nHits << endl;
    cout << "MCTracks: total " << nMCTracks << ", accepted " << nMCacc
	 << ", reconstructable: " << nTracks << endl;
    if (nNoStsHit)   cout << "StsHits not found   : " 
			  << nNoStsHit   << endl;
    if (nNoStsPoint) cout << "StsPoints not found : " 
			  << nNoStsPoint << endl;
    if (nNoMCTrack)  cout << "MCTracks not found  : " 
			  << nNoMCTrack  << endl;
    if (nNoTrack)    cout << "StsTracks not found : " 
			  << nNoTrack    << endl;
    cout << "-------------------------------------------------------" 
	 << endl;
  }
  else 
    if ( success ) cout << "+ ";
    else           cout << "- ";
  cout << setw(15) << left << fName << ": " 
       << nMCTracks << ", acc. " << nMCacc << ", rec. " << nTracks << endl; 

  return nTracks;

}
// -------------------------------------------------------------------------


ClassImp(CbmStsTrackFinderIdeal)

    

  
