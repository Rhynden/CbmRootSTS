// -------------------------------------------------------------------------
// -----                CbmStsTrackFitterIdeal source file             -----
// -----                  Created 12/05/06  by D. Kresan               -----
// -------------------------------------------------------------------------

#include "CbmStsTrackFitterIdeal.h"

#include "CbmStsPoint.h"
#include "CbmStsHit.h"
#include "CbmStsTrack.h"

#include "FairRootManager.h"
#include "CbmMCTrack.h"

#include "TClonesArray.h"
#include "TVector3.h"
#include "TParticlePDG.h"

#include <iostream>

using std::cout;
using std::cerr;
using std::endl;

// ---------------------------- Constructor --------------------------------
CbmStsTrackFitterIdeal::CbmStsTrackFitterIdeal()
:  fArrayMCTrack(NULL),
   fArrayStsPoint(NULL),
   fArrayStsHit(NULL),
   dbPDG()
{
}
// -------------------------------------------------------------------------


// ---------------------------- Destructor ---------------------------------
CbmStsTrackFitterIdeal::~CbmStsTrackFitterIdeal()

{
}
// -------------------------------------------------------------------------


// -------------------------- Initialisation -------------------------------
void CbmStsTrackFitterIdeal::Init()
{
    dbPDG = TDatabasePDG::Instance();
    if(NULL == dbPDG) {
	cout << "-W- CbmStsTrackFitterIdeal::Init : "
            << "Database PDG is not instantiated!" << endl;
    }

    FairRootManager* rootMgr = FairRootManager::Instance();
    if(NULL == rootMgr) {
	cerr << "-E- CbmStsTrackFitterIdeal::Init : "
	    << "ROOT manager is not instantiated!" << endl;
        return;
    }

    fArrayMCTrack = (TClonesArray*) rootMgr->GetObject("MCTrack");
    if(NULL == fArrayMCTrack) {
	cout << "-W- CbmStsTrackFitterIdeal::Init : "
            << "no MC track array!" << endl;
    }

    fArrayStsPoint = (TClonesArray*) rootMgr->GetObject("StsPoint");
    if(NULL == fArrayStsPoint) {
	cout << "-W- CbmStsTrackFitterIdeal::Init : "
            << "no STS point array!" << endl;
    }

    fArrayStsHit = (TClonesArray*) rootMgr->GetObject("StsHit");
    if(NULL == fArrayStsHit) {
	cout << "-W- CbmStsTrackFitterIdeal::Init : "
            << "no STS hit array!" << endl;
    }

}
// -------------------------------------------------------------------------



// ------------------------ Fitting algorithm ------------------------------
Int_t CbmStsTrackFitterIdeal::DoFit(CbmStsTrack* pTrack, Int_t /*pidHypo*/)
{
    if(NULL == fArrayStsPoint || NULL == fArrayStsHit ) return 0;

    // Parameters at the first plane
    Int_t hitIndex = -1;
    CbmStsHit* hit;
    Int_t pointIndex = -1;
    if(pTrack->GetNofStsHits() > 0) {
	hitIndex = pTrack->GetHitIndex(0);
	if(hitIndex < 0) return 0;
	hit = (CbmStsHit*) fArrayStsHit->At(hitIndex);
	if(NULL == hit) return 0;
        pointIndex = hit->GetRefId();
    }
    if(pointIndex < 0) return 0;
    CbmStsPoint* point = (CbmStsPoint*) fArrayStsPoint->At(pointIndex);
    if(NULL == point) return 0;
    FairTrackParam parFirst(*pTrack->GetParamFirst());
    SetTrackParam(point, &parFirst);//pTrack->GetParamFirst());
    pTrack->SetParamFirst(&parFirst);

    // Parameters at the last plane
    if(pTrack->GetNofStsHits() > 0) {
	hitIndex = pTrack->GetHitIndex( pTrack->GetNofStsHits()-1 );
	if(hitIndex < 0) return 0;
	hit = (CbmStsHit*) fArrayStsHit->At(hitIndex);
	if(NULL == hit) return 0;
        pointIndex = hit->GetRefId();
    }
    if(pointIndex < 0) return 0;
    point = (CbmStsPoint*) fArrayStsPoint->At(pointIndex);
    if(NULL == point) return 0;
    FairTrackParam parLast(*pTrack->GetParamLast());
    SetTrackParam(point, &parLast, 1);//pTrack->GetParamLast(), 1);
    pTrack->SetParamLast(&parLast);

    return 1;
}
// -------------------------------------------------------------------------


// -------------------------- Set track parameters -------------------------
void CbmStsTrackFitterIdeal::SetTrackParam(CbmStsPoint* point, FairTrackParam* trackParam, Int_t out)
{
    TVector3 pos;
    TVector3 mom;
    if(0 == out) {
	point->Position(pos);
	point->Momentum(mom);
    } else {
	point->PositionOut(pos);
	point->MomentumOut(mom);
    }
    trackParam->SetX(pos.X());
    trackParam->SetY(pos.Y());
    trackParam->SetZ(pos.Z());
    trackParam->SetTx(mom.X()/mom.Z());
    trackParam->SetTy(mom.Y()/mom.Z());
    Int_t mcTrackIndex = point->GetTrackID();
    if(mcTrackIndex < 0) return;
    CbmMCTrack* mcTrack = (CbmMCTrack*) fArrayMCTrack->At(mcTrackIndex);
    if(NULL == mcTrack) return;
    Int_t pdgCode = mcTrack->GetPdgCode();
    TParticlePDG* particle = dbPDG->GetParticle(pdgCode);
    if(NULL == particle) return;
    trackParam->SetQp(particle->Charge() / mom.Mag());
}
// -------------------------------------------------------------------------


ClassImp(CbmStsTrackFitterIdeal)


