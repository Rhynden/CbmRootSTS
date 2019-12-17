// -------------------------------------------------------------------------
// -----                    CbmStsFitTracks source file                -----
// -----                  Created 18/02/05  by V. Friese               -----
// -------------------------------------------------------------------------
#include "CbmStsFitTracks.h"

#include "FairRootManager.h"
#include "CbmStsTrackFitter.h"

#include "TClonesArray.h"

#include <iostream>
#include <iomanip>

using std::cout;
using std::endl;
using std::right;
using std::left;
using std::setw;
using std::setprecision;
using std::fixed;

// -----   Default constructor   -------------------------------------------
CbmStsFitTracks::CbmStsFitTracks() 
  : FairTask("STSFitTracks"),
    fFitter(NULL),
    fTracks(NULL),
    fTimer(),
    fNEvents(0),
    fNFailed(0),
    fTime(0.),
    fNTracks(0)
{}
// -------------------------------------------------------------------------



// -----   Standard constructor   ------------------------------------------
CbmStsFitTracks::CbmStsFitTracks(CbmStsTrackFitter* fitter, Int_t /*iVerbose*/) 
  : FairTask("STSFitTracks"),
    fFitter(fitter),
    fTracks(NULL),
    fTimer(),
    fNEvents(0),
    fNFailed(0),
    fTime(0.),
    fNTracks(0)
{}
// -------------------------------------------------------------------------



// -----   Constructor with name   -----------------------------------------
CbmStsFitTracks::CbmStsFitTracks(const char* name, CbmStsTrackFitter* fitter, Int_t iVerbose) 
  : FairTask(name, iVerbose),
    fFitter(fitter),
    fTracks(NULL),
    fTimer(),
    fNEvents(0),
    fNFailed(0),
    fTime(0.),
    fNTracks(0)
{}
// -------------------------------------------------------------------------



// -----   Destructor   ----------------------------------------------------
CbmStsFitTracks::~CbmStsFitTracks() { }
// -------------------------------------------------------------------------




// -----   Virtual public method Exec   ------------------------------------
void CbmStsFitTracks::Exec(Option_t* /*opt*/) {
  fTimer.Start();
  

  if ( ! fTracks ) {
    cout << "-E- " << fName << "::Exec: No StsTrack array! " << endl;
    fNFailed++;
    return;
  }

  Int_t nTracks = fTracks->GetEntriesFast();
  for (Int_t iTrack=0; iTrack<nTracks; iTrack++) {
    CbmStsTrack* pTrack = (CbmStsTrack*)fTracks->At(iTrack);
    fFitter->DoFit(pTrack);
  }

  fTimer.Stop();
  if ( fVerbose ) 
    cout << "+ " << setw(15) << left << fName << ": " << setprecision(4) 
	 << setw(8) << fixed << right << fTimer.RealTime()
	 << " s, tracks fitted " << nTracks << endl;

  fNEvents++;
  fTime    += fTimer.RealTime();
  fNTracks += Double_t(nTracks);
  
}
// -------------------------------------------------------------------------



// -----   Virtual private method Init   -------------- --------------------
InitStatus CbmStsFitTracks::Init() {

  // Check for Track fitter
  if (! fFitter) {
    cout << "-E- CbmStsFitTracks: No track fitter selected!" << endl;
    return kERROR;
  }

  // Get and check FairRootManager
  FairRootManager* ioman = FairRootManager::Instance();
  if (! ioman) {
    cout << "-E- CbmStsFitTracks::Init: "
	 << "RootManager not instantised!" << endl;
    return kFATAL;
  }

  // Get StsTrack array
  fTracks  = (TClonesArray*) ioman->GetObject("StsTrack"); //=>SG
  if ( ! fTracks) {
    cout << "-E- CbmStsFitTracks::Init: No StsTrack array!"
	 << endl;
    return kERROR;
  }

  // Call the Init method of the track fitter
  fFitter->Init();

  return kSUCCESS;

}
// -------------------------------------------------------------------------


// -----   Virtual private method Finish   ---------------------------------
void CbmStsFitTracks::Finish() { 

  cout << endl;
  cout << "============================================================"
       << endl;
  cout << "=====   " << GetName() << ": Run summary " << endl;
  cout << "===== " << endl;
  cout << "===== Good events   : " << setw(6) << fNEvents << endl;
  cout << "===== Failed events : " << setw(6) << fNFailed << endl;
  cout << "===== Average time  : " << setprecision(4) << setw(8) << right
       << fTime / Double_t(fNEvents)  << " s" << endl;
  cout << "===== " << endl;
  cout << "===== Fitted tracks per event  : " << fixed << setprecision(0)
       << fNTracks / Double_t(fNEvents) << endl;
  cout << "============================================================"
       << endl;

}
// -------------------------------------------------------------------------




ClassImp(CbmStsFitTracks)
