/**
 * \file CbmStsWkn.h
 * \author Olga Derenovskaia <odenisova@jinr.ru>
 * \date 2019
 **/

#include "CbmStsWkn.h"
#include "FairRootManager.h"

#include "CbmGlobalTrack.h"

#include "TMath.h"
#include "TString.h"
#include "TClonesArray.h"
#include "TSystem.h"
#include "CbmStsTrack.h"
#include "CbmStsCluster.h"
#include "CbmStsDigi.h"
#include "CbmStsHit.h"

#include <iostream>
#include <cmath>
  using std::cout;
  using std::endl;
  using std::map;
  using std::vector;
  using std::fabs;

CbmStsWkn::CbmStsWkn():
    fdegWkn(4),
	fEmp(2.4),
	fXi(0.5),
	fk1(fdegWkn+1),
  fnSet(8),
	fwkn(-1)
{
	Init();
}

CbmStsWkn::~CbmStsWkn()
{

}

void CbmStsWkn::Init()
{
	FairRootManager* ioman = FairRootManager::Instance();
	if (ioman != nullptr) {
        fGlobalTracks = (TClonesArray*) ioman->GetObject("GlobalTrack");
        fStsTracks = (TClonesArray *)  ioman->GetObject("StsTrack");
        fStsHits = (TClonesArray*) ioman->GetObject("StsHit");
        fStsClusterArray=(TClonesArray*) ioman->GetObject("StsCluster");
        fStsDigiArray=(TClonesArray*) ioman->GetObject("StsDigi");
	}
}

Double_t CbmStsWkn::GetStsWkn(Int_t StsTrackIndex)
{
  double dr = 1.;
  std::  vector<float> eLossVector;
  std::  vector<float> dEdxAllveto; 
  
  CbmStsTrack* StsTrack = (CbmStsTrack*) fStsTracks->At(StsTrackIndex);
  if (StsTrack == NULL) return fwkn;

  int nClustersWveto = StsTrack->GetNofStsHits() + StsTrack->GetNofStsHits();//assume all clusters with veto
  if (nClustersWveto < 8) return fwkn;

//cout<<fnSet<<endl;
	for (int iHit = 0; iHit < StsTrack->GetNofStsHits(); ++iHit)
    {
        Int_t StsHitIndex = StsTrack->GetStsHitIndex(iHit);
        CbmStsHit * stsHit = (CbmStsHit*) fStsHits -> At(StsHitIndex);
		
// ***********dx=dr is calculated from the track inclination             
	    double x, y, z, xNext, yNext, zNext;
        x = stsHit -> GetX();
        y = stsHit -> GetY();
        z = stsHit -> GetZ();

        if (iHit != StsTrack->GetNofStsHits()-1)
        {
            CbmStsHit * stsHitNext = (CbmStsHit*) fStsHits -> At(StsTrack->GetStsHitIndex(iHit + 1));
            xNext = stsHitNext -> GetX();
            yNext = stsHitNext -> GetY();
            zNext = stsHitNext -> GetZ();
            dr = sqrt((xNext - x)*(xNext - x) + (yNext - y)*(yNext - y) + (zNext - z)*(zNext - z)) / (zNext - z);// if *300um, you get real reconstructed dr
        } // else dr stay previous
// ***********End of dr calculation	

//************dE is defined as a total cluster charge
	    Int_t FrontClusterId = stsHit -> GetFrontClusterId();	    
        CbmStsCluster * frontCluster = (CbmStsCluster*) fStsClusterArray -> At(FrontClusterId);
	    Int_t BackClusterId = stsHit -> GetBackClusterId();
        CbmStsCluster * backCluster  = (CbmStsCluster*) fStsClusterArray -> At(BackClusterId);
	  
        if (!frontCluster || !backCluster) return fwkn;
	  
       dEdxAllveto.push_back((frontCluster -> GetCharge()) / dr); 
       dEdxAllveto.push_back((backCluster -> GetCharge()) / dr);
    }
    
	if (dEdxAllveto.size() != 0) { 
	  unsigned int NSample = dEdxAllveto.size();
	  
	  for (unsigned int jVec = 0; jVec<NSample; jVec++) 
        dEdxAllveto[jVec]=dEdxAllveto[jVec]/10000;

	  for (unsigned int jVec = 0; jVec<NSample; jVec++) 
        dEdxAllveto[jVec]=(dEdxAllveto[jVec]-fEmp)/fXi-0.225;
		
	  sort(dEdxAllveto.begin(), dEdxAllveto.end());
	  
	  for (unsigned int jVec = 0; jVec<NSample; jVec++) 
        dEdxAllveto[jVec]=TMath::LandauI(dEdxAllveto[jVec]);
		
		
	  for (int iHit = 0; iHit < fnSet; iHit++)
        eLossVector.push_back(dEdxAllveto[NSample-fnSet+iHit]);
	}
     
		Double_t  S = 0, ty = 0, ti = 0;

    for (Int_t i=0;i<fnSet;i++)
     {
       ty = eLossVector[i];  ti = i;
       S += pow((ti-1)/fnSet-ty,fk1)-pow(ti/fnSet-ty,fk1);
    }
	float fwkn0 = pow (fnSet,0.5*fdegWkn)/fk1;
    Double_t result_wkn  = -fwkn0*S;

    return result_wkn;
	
}

Double_t CbmStsWkn::GetStsWkn(const CbmStsTrack* StsTrack)
{
  double dr = 1.;
  std::  vector<float> eLossVector;
  std::  vector<float> dEdxAllveto; 
  
  int nClustersWveto = StsTrack->GetNofStsHits() + StsTrack->GetNofStsHits();//assume all clusters with veto
  if (nClustersWveto < 8) return fwkn;

//cout<<fnSet<<endl;
	for (int iHit = 0; iHit < StsTrack->GetNofStsHits(); ++iHit)
    {
        Int_t StsHitIndex = StsTrack->GetStsHitIndex(iHit);
        CbmStsHit * stsHit = (CbmStsHit*) fStsHits -> At(StsHitIndex);
		
// ***********dx=dr is calculated from the track inclination             
	    double x, y, z, xNext, yNext, zNext;
        x = stsHit -> GetX();
        y = stsHit -> GetY();
        z = stsHit -> GetZ();

        if (iHit != StsTrack->GetNofStsHits()-1)
        {
            CbmStsHit * stsHitNext = (CbmStsHit*) fStsHits -> At(StsTrack->GetStsHitIndex(iHit + 1));
            xNext = stsHitNext -> GetX();
            yNext = stsHitNext -> GetY();
            zNext = stsHitNext -> GetZ();
            dr = sqrt((xNext - x)*(xNext - x) + (yNext - y)*(yNext - y) + (zNext - z)*(zNext - z)) / (zNext - z);// if *300um, you get real reconstructed dr
        } // else dr stay previous
// ***********End of dr calculation	

//************dE is defined as a total cluster charge
	    Int_t FrontClusterId = stsHit -> GetFrontClusterId();	    
        CbmStsCluster * frontCluster = (CbmStsCluster*) fStsClusterArray -> At(FrontClusterId);
	    Int_t BackClusterId = stsHit -> GetBackClusterId();
        CbmStsCluster * backCluster  = (CbmStsCluster*) fStsClusterArray -> At(BackClusterId);
	  
        if (!frontCluster || !backCluster) return fwkn;
	  
       dEdxAllveto.push_back((frontCluster -> GetCharge()) / dr); 
       dEdxAllveto.push_back((backCluster -> GetCharge()) / dr);
    }
    
	if (dEdxAllveto.size() != 0) { 
	  unsigned int NSample = dEdxAllveto.size();
	  
	  for (unsigned int jVec = 0; jVec<NSample; jVec++) 
        dEdxAllveto[jVec]=dEdxAllveto[jVec]/10000;

	  for (unsigned int jVec = 0; jVec<NSample; jVec++) 
        dEdxAllveto[jVec]=(dEdxAllveto[jVec]-fEmp)/fXi-0.225;
		
	  sort(dEdxAllveto.begin(), dEdxAllveto.end());
	  
	  for (unsigned int jVec = 0; jVec<NSample; jVec++) 
        dEdxAllveto[jVec]=TMath::LandauI(dEdxAllveto[jVec]);
		
		
	  for (int iHit = 0; iHit < fnSet; iHit++)
        eLossVector.push_back(dEdxAllveto[NSample-fnSet+iHit]);
	}
     
		Double_t  S = 0, ty = 0, ti = 0;

    for (Int_t i=0;i<fnSet;i++)
     {
       ty = eLossVector[i];  ti = i;
       S += pow((ti-1)/fnSet-ty,fk1)-pow(ti/fnSet-ty,fk1);
    }
	float fwkn0 = pow (fnSet,0.5*fdegWkn)/fk1;
    Double_t result_wkn  = -fwkn0*S;

    return result_wkn;
	
}

ClassImp(CbmStsWkn);
