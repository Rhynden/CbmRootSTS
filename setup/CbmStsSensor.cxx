/** @file CbmStsSensor.cxx
 ** @author Volker Friese <v.friese@gsi.de>
 ** @date 03.05.2013
 **/


// Include class header
#include "CbmStsSensor.h"

// Includes from c++
#include <cassert>
#include <sstream>

// Includes from ROOT
#include "TClonesArray.h"
#include "TGeoBBox.h"
#include "TGeoMatrix.h"
#include "TMath.h"

// Includes from FairRoot
#include "FairField.h"
#include "FairRunAna.h"

// Includes from CbmRoot
#include "CbmEvent.h"
#include "CbmLink.h"
#include "CbmStsHit.h"
#include "CbmStsPoint.h"

// Includes from STS
#include "setup/CbmStsModule.h"
#include "setup/CbmStsSensorConditions.h"
#include "setup/CbmStsSensorPoint.h"
#include "setup/CbmStsSetup.h"

using std::vector;


// -----   Constructor   ---------------------------------------------------
CbmStsSensor::CbmStsSensor(UInt_t address, TGeoPhysicalNode* node,
                           CbmStsElement* mother) :
    CbmStsElement(address, kStsSensor, node, mother),
    fConditions(nullptr),
    fCurrentLink(nullptr),
    fHits(nullptr),
    fEvent(nullptr)
{
}
// -------------------------------------------------------------------------



// -----   Create a new hit   ----------------------------------------------
void CbmStsSensor::CreateHit(Double_t xLocal, Double_t yLocal, Double_t varX,
		                     Double_t varY, Double_t varXY,
		                     CbmStsCluster* clusterF, CbmStsCluster* clusterB,
		                     Double_t du, Double_t dv) {

  // ---  Check output array
  assert(fHits);

	// --- If a TGeoNode is attached, transform into global coordinate system
	Double_t local[3] = { xLocal, yLocal, 0.};
	Double_t global[3];
	if ( fNode ) fNode->GetMatrix()->LocalToMaster(local, global);
	else {
		global[0] = local[0];
		global[1] = local[1];
		global[2] = local[2];
	} //? no TGeoNode available

	// We assume here that the local-to-global transformations is only translation
	// plus maybe rotation upside down or front-side back. In that case, the
	// global covariance matrix is the same as the local one.
	Double_t error[3] = { TMath::Sqrt(varX), TMath::Sqrt(varY), 0.};


	// --- Calculate hit time (average of cluster times)
	Double_t fTime = ( clusterF ? clusterF->GetTime() : 0. );
    Double_t bTime = ( clusterB ? clusterB->GetTime() : 0. );
    Double_t hitTime = 0.5 * ( fTime + bTime);
	Double_t etF = ( clusterF ? clusterF->GetTimeError() : 0. );
	Double_t etB = ( clusterB ? clusterB->GetTimeError() : 0. );
	Double_t hitTimeError = 0.5 * TMath::Sqrt( etF*etF + etB*etB );

	// --- Create hit
	Int_t index = fHits->GetEntriesFast();
	Int_t indexF = ( clusterF ? clusterF->GetIndex() : -1 );
    Int_t indexB = ( clusterB ? clusterB->GetIndex() : -1 );
	new ( (*fHits)[index] )
			CbmStsHit(GetAddress(),              // address
					      global,                // coordinates
					      error,                 // coordinate error
					      varXY,                 // covariance xy
					      indexF,                // front cluster index
					      indexB,                // back cluster index
					      hitTime,               // hit time
					      hitTimeError,          // hit time error
					      du, dv);               // errors in u and v
	if ( fEvent) fEvent->AddData(kStsHit, index);

	LOG(debug2) << GetName() << ": Creating hit at (" << global[0] << ", "
			        << global[1] << ", " << global[2] << ")";
	return;
}
// -------------------------------------------------------------------------

// -----   Create a new hit   ----------------------------------------------
void CbmStsSensor::CreateHitInVector(Double_t xLocal, Double_t yLocal, Double_t varX,
		                     Double_t varY, Double_t varXY,
		                     CbmStsCluster* clusterF, CbmStsCluster* clusterB,
		                     Double_t du, Double_t dv) {

  // ---  Check output array
  assert(fHitsVector);

	// --- If a TGeoNode is attached, transform into global coordinate system
	Double_t local[3] = { xLocal, yLocal, 0.};
	Double_t global[3];
	if ( fNode ) fNode->GetMatrix()->LocalToMaster(local, global);
	else {
		global[0] = local[0];
		global[1] = local[1];
		global[2] = local[2];
	} //? no TGeoNode available

	// We assume here that the local-to-global transformations is only translation
	// plus maybe rotation upside down or front-side back. In that case, the
	// global covariance matrix is the same as the local one.
	Double_t error[3] = { TMath::Sqrt(varX), TMath::Sqrt(varY), 0.};


	// --- Calculate hit time (average of cluster times)
	Double_t fTime = ( clusterF ? clusterF->GetTime() : 0. );
    Double_t bTime = ( clusterB ? clusterB->GetTime() : 0. );
    Double_t hitTime = 0.5 * ( fTime + bTime);
	Double_t etF = ( clusterF ? clusterF->GetTimeError() : 0. );
	Double_t etB = ( clusterB ? clusterB->GetTimeError() : 0. );
	Double_t hitTimeError = 0.5 * TMath::Sqrt( etF*etF + etB*etB );

	// --- Create hit
	Int_t index = fHitsVector->size();
	Int_t indexF = ( clusterF ? clusterF->GetIndex() : -1 );
    Int_t indexB = ( clusterB ? clusterB->GetIndex() : -1 );
	fHitsVector->emplace_back(
						  GetAddress(),              // address
					      global,                // coordinates
					      error,                 // coordinate error
					      varXY,                 // covariance xy
					      indexF,                // front cluster index
					      indexB,                // back cluster index
					      hitTime,               // hit time
					      hitTimeError,          // hit time error
					      du, dv);
	/*new ( (*fHits)[index] )
			CbmStsHit(GetAddress(),              // address
					      global,                // coordinates
					      error,                 // coordinate error
					      varXY,                 // covariance xy
					      indexF,                // front cluster index
					      indexB,                // back cluster index
					      hitTime,               // hit time
					      hitTimeError,          // hit time error
					      du, dv);*/               // errors in u and v

	//LOG(info) << "fHitsVector size after create hit " << fHitsVector.size();

	if ( fEvent) fEvent->AddData(kStsHit, index);

	LOG(debug2) << GetName() << ": Creating hit at (" << global[0] << ", "
			        << global[1] << ", " << global[2] << ")";
	return;
}
// -------------------------------------------------------------------------



// -----   Get the unique address from the sensor name (static)   ----------
UInt_t CbmStsSensor::GetAddressFromName(TString name) {

  Int_t unit    = 10 * ( name[5]  - '0') + name[6]  - '0' - 1;
  Int_t ladder  = 10 * ( name[9]  - '0') + name[10] - '0' - 1;
  Int_t hLadder = ( name[11] == 'U' ? 0 : 1);
  Int_t module  = 10 * ( name[14] - '0') + name[15] - '0' - 1;
  Int_t sensor  = 10 * ( name[18] - '0') + name[19] - '0' - 1;

  return CbmStsAddress::GetAddress(unit, ladder, hLadder, module, sensor);
}
// -------------------------------------------------------------------------



// -----  Get the mother module   ------------------------------------------
CbmStsModule* CbmStsSensor::GetModule() const {
	 return dynamic_cast<CbmStsModule*> ( GetMother() );
}
// -------------------------------------------------------------------------



// -----   Process a CbmStsPoint  ------------------------------------------
Int_t CbmStsSensor::ProcessPoint(const CbmStsPoint* point,
		                             Double_t eventTime, CbmLink* link) {

  // --- Set current link
	fCurrentLink = link;

  // --- Transform start coordinates into local C.S.
  Double_t global[3];
  Double_t local[3];
  global[0] = point->GetXIn();
  global[1] = point->GetYIn();
  global[2] = point->GetZIn();
  fNode->GetMatrix()->MasterToLocal(global, local);
  Double_t x1 = local[0];
  Double_t y1 = local[1];
  Double_t z1 = local[2];

  // --- Transform stop coordinates into local C.S.
  global[0] = point->GetXOut();
  global[1] = point->GetYOut();
  global[2] = point->GetZOut();
  fNode->GetMatrix()->MasterToLocal(global, local);
  Double_t x2 = local[0];
  Double_t y2 = local[1];
  Double_t z2 = local[2];

  // --- Average track direction in local c.s.
  Double_t tXav = 0.;
  Double_t tYav = 0.;
//  Int_t    tZav = 0;
  if ( z2 - z1 != 0. ) {
  	tXav = ( x2 - x1 ) / (z2 - z1);
  	tYav = ( y2 - y1 ) / (z2 - z1);
//  	tZav = 1;
  }

  // --- Normally, the entry and exit coordinates are slightly outside of
  // --- the active node, which is a feature of the transport engine.
  // --- We correct here for this, in case a track was entering or
  // --- exiting the sensor (not for tracks newly created or stopped
  // --- in the sensor volume).
  // --- We here consider only the case of tracks leaving through the front
  // --- or back plane. The rare case of tracks leaving through the sensor
  // --- sides is caught by the digitisation procedure.
  Double_t dZ = dynamic_cast<TGeoBBox*>(fNode->GetShape())->GetDZ();

  // --- Correct start coordinates in case of entry step
  if ( point->IsEntry() ) {

 		// Get track direction in local c.s.
 		global[0] = point->GetPx();
 		global[1] = point->GetPy();
 		global[2] = point->GetPz();
 		Double_t* rot;
 		rot = fNode->GetMatrix()->GetRotationMatrix();
 		TGeoHMatrix rotMat;
 		rotMat.SetRotation(rot);
 		rotMat.MasterToLocal(global,local);
 		if ( local[2] != 0.) {;  // should always be; else no correction
 			Double_t	tX = local[0] / local[2]; // px/pz
 			Double_t	tY = local[1] / local[2]; // py/pz

 			// New start coordinates
 			Double_t xNew = 0.;
 			Double_t yNew = 0.;
 			Double_t zNew = 0.;
 			if ( z1 > 0. ) zNew = dZ - 1.e-4; // front plane, safety margin 1 mum
 			else           zNew = 1.e-4 - dZ; // back plane, safety margin 1 mum
 			xNew = x1 + tX * (zNew - z1);
 			yNew = y1 + tY * (zNew - z1);

 			x1 = xNew;
 			y1 = yNew;
 			z1 = zNew;
 		} //? pz != 0.

  }  //? track has entered

  // --- Correct stop coordinates in case of being outside the sensor
  if ( TMath::Abs(z2) > dZ ) {

  	// Get track direction in local c.s.
  	global[0] = point->GetPxOut();
 		global[1] = point->GetPyOut();
 		global[2] = point->GetPzOut();
 		Double_t* rot;
  	rot = fNode->GetMatrix()->GetRotationMatrix();
 		TGeoHMatrix rotMat;
 		rotMat.SetRotation(rot);
 		rotMat.MasterToLocal(global,local);
 		Double_t tX = 0.;
 		Double_t tY = 0.;
 		// Use momentum components for track direction, if available
 		if ( local[2] != 0. ) {
 			tX = local[0] / local[2]; // px/pz
 			tY = local[1] / local[2]; // py/pz
 		}
 		// Sometimes, a track is stopped outside the sensor volume.
 		// Then we take the average track direction as best approximation.
 		// Note that there may be cases where entry and exit coordinates are
 		// the same. In this case, tXav = tYav = 0; there will be no correction
 		// of the coordinates.
 		else {
 		  tX = tXav;  // (x2-x1)/(z2-z1) or 0 if z2 = z1
 		  tY = tYav;  // (y2-y1)/(z2-z1) or 0 if z2 = z1
 		}

 		// New coordinates
 		Double_t xNew = 0.;
 		Double_t yNew = 0.;
 		Double_t zNew = 0.;
 		if ( z2 > 0. ) zNew = dZ - 1.e-4; // front plane, safety margin 1 mum
 		else           zNew = 1.e-4 - dZ; // back plane, safety margin 1 mum
 		xNew = x2 + tX * (zNew - z2);
 		yNew = y2 + tY * (zNew - z2);

 		x2 = xNew;
 		y2 = yNew;
 		z2 = zNew;

  } //? track step outside sensor


  // --- Momentum magnitude
  Double_t px = 0.5 * ( point->GetPx() + point->GetPxOut() );
  Double_t py = 0.5 * ( point->GetPy() + point->GetPyOut() );
  Double_t pz = 0.5 * ( point->GetPz() + point->GetPzOut() );
  Double_t p = TMath::Sqrt( px*px + py*py + pz*pz );

  // --- Get magnetic field
  global[0] = 0.5 * ( point->GetXIn() + point->GetXOut() );
  global[1] = 0.5 * ( point->GetYIn() + point->GetYOut() );
  global[2] = 0.5 * ( point->GetZIn() + point->GetZOut() );
  Double_t bField[3] = { 0., 0., 0.};
  if ( FairRun::Instance() -> GetField())
  	FairRun::Instance()->GetField()->Field(global, bField);

  // --- Absolute time of StsPoint
  Double_t pTime = eventTime + point->GetTime();

  // --- Create SensorPoint
  // Note: there is a conversion from kG to T in the field values.
  CbmStsSensorPoint* sPoint = new CbmStsSensorPoint(x1, y1, z1, x2, y2, z2, p,
                                                    point->GetEnergyLoss(),
                                                    pTime,
                                                    bField[0] / 10.,
                                                    bField[1] / 10.,
                                                    bField[2] / 10.,
                                                    point->GetPid());
  LOG(debug2) << GetName() << ": Local point coordinates are (" << x1
  		        << ", " << y1 << "), (" << x2 << ", " << y2 << ")";
  LOG(debug2) << point->IsEntry() << " " << point->IsExit();

  // --- Call ProcessPoint method from sensor type
  Int_t result = CalculateResponse(sPoint);
  delete sPoint;

  return result;
}
// -------------------------------------------------------------------------



ClassImp(CbmStsSensor)
