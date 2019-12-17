// -------------------------------------------------------------------------
// -----                CbmStsHitProducerIdeal source file             -----
// -----                  Created 10/01/06  by V. Friese               -----
// -------------------------------------------------------------------------
#include <iostream>

#include "TClonesArray.h"

#include "FairRootManager.h"

#include "CbmStsHit.h"
#include "CbmStsHitProducerIdeal.h"
#include "CbmStsPoint.h"

using std::cout;
using std::endl;


// -----   Default constructor   -------------------------------------------
CbmStsHitProducerIdeal::CbmStsHitProducerIdeal() 
  : FairTask("Ideal STS Hit Producer"),
    fPointArray(NULL),
    fHitArray()
{ 
}
// -------------------------------------------------------------------------



// -----   Destructor   ----------------------------------------------------
CbmStsHitProducerIdeal::~CbmStsHitProducerIdeal() { }
// -------------------------------------------------------------------------



// -----   Public method Init   --------------------------------------------
InitStatus CbmStsHitProducerIdeal::Init() {

  // Get RootManager
  FairRootManager* ioman = FairRootManager::Instance();
  if ( ! ioman ) {
    cout << "-E- CbmStsHitProducerIdeal::Init: "
	 << "RootManager not instantised!" << endl;
    return kFATAL;
  }

  // Get input array
  fPointArray = (TClonesArray*) ioman->GetObject("StsPoint");
  if ( ! fPointArray ) {
    cout << "-W- CbmStsHitProducerIdeal::Init: "
	 << "No STSPoint array!" << endl;
    return kERROR;
  }

  // Create and register output array
  fHitArray = new TClonesArray("CbmStsHit");
  ioman->Register("StsHit", "STS", fHitArray, IsOutputBranchPersistent("StsHit"));

  cout << "-I- CbmStsHitProducerIdeal: Intialisation successfull" << endl;
  return kSUCCESS;

}
// -------------------------------------------------------------------------



// -----   Public method Exec   --------------------------------------------
void CbmStsHitProducerIdeal::Exec(Option_t* /*opt*/) {

  // Reset output array
  if ( ! fHitArray ) Fatal("Exec", "No StsHit array");

  //  fHitArray->Clear();
  fHitArray->Delete();

  // Declare some variables
  CbmStsPoint* point = NULL;
  Int_t detID   = 0;        // Detector ID
//  Int_t trackID = 0;        // Track index
  Double_t x, y, z;         // Position
  Double_t dx = 0.0001;     // Position error
  TVector3 pos, dpos;       // Position and error vectors

  // Loop over StsPoints
  Int_t nPoints = fPointArray->GetEntriesFast();
  for (Int_t iPoint=0; iPoint<nPoints; iPoint++) {
    point = (CbmStsPoint*) fPointArray->At(iPoint);
    if ( ! point) continue;

    // Detector ID
    detID = point->GetDetectorID();

    // MCTrack ID
//    trackID = point->GetTrackID();

    // Determine hit position (centre plane of station)
    x  = 0.5 * ( point->GetXOut() + point->GetXIn() );
    y  = 0.5 * ( point->GetYOut() + point->GetYIn() );
    z  = 0.5 * ( point->GetZOut() + point->GetZIn() );

    // Create new hit
    pos.SetXYZ(x,y,z);
    dpos.SetXYZ(dx, dx, 0.);
    new ((*fHitArray)[iPoint]) CbmStsHit(detID, pos, dpos, 0., iPoint, iPoint, 0., 0.);
  }   // Loop over MCPoints

  // Event summary
  cout << "-I- CbmStsHitProducerIdeal: " << nPoints << " StsPoints, "
       << nPoints << " Hits created." << endl;

}
// -------------------------------------------------------------------------



ClassImp(CbmStsHitProducerIdeal)
