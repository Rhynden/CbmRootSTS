/** @file CbmStsSensorDssdOrtho.cxx
 ** @author Volker Friese <v.friese@gsi.de>
 ** @date 02.05.2013
 **/

#include "CbmStsSensorDssdOrtho.h"

#include <cassert>

#include "TGeoBBox.h"
#include "TGeoMatrix.h"
#include "TMath.h"

#include "CbmMatch.h"
#include "CbmStsDigitizeParameters.h"
#include "CbmStsPhysics.h"
#include "CbmStsSetup.h"


using namespace std;


// -----   Constructor   ---------------------------------------------------
CbmStsSensorDssdOrtho::CbmStsSensorDssdOrtho(UInt_t address,
                                             TGeoPhysicalNode* node,
                                             CbmStsElement* mother) :
             CbmStsSensorDssd(address, node, mother),
             fNofStrips{ -1, -1 },
             fPitch{ -1., -1. }
{
  SetTitle("DssdOrtho");
}
// -------------------------------------------------------------------------



// -----   Constructor   ---------------------------------------------------
CbmStsSensorDssdOrtho::CbmStsSensorDssdOrtho(Int_t nStripsF, Double_t pitchF,
                                             Int_t nStripsB, Double_t pitchB) :
             CbmStsSensorDssd(),
             fNofStrips{ nStripsF, nStripsB},
             fPitch{ pitchF, pitchB }
{
  SetTitle("DssdOrtho");
}
// -------------------------------------------------------------------------



// -----   Create a hit from a single cluster   ----------------------------
void CbmStsSensorDssdOrtho::CreateHitFromCluster(CbmStsCluster* cluster) {

  // --- Calculate cluster centre position on readout edge
  Int_t side  = -1;
  Double_t posHit = -1.;
  GetClusterPosition(cluster->GetPosition(), posHit, side);
  assert( side == 0 || side == 1);

  Double_t xHit  = -1.0;
  Double_t yHit  = -1.0;
  Double_t dxHit = -1.0;
  Double_t dyHit = -1.0;

  // --- Set correct cluster pointer for the hit
  CbmStsCluster* clusterF = ( side == 0 ? cluster : nullptr);
  CbmStsCluster* clusterB = ( side == 0 ? nullptr : cluster);

  if ( 0 == side ) {
    // --- x coordinate comes from cluster center in strips
    xHit = posHit;
    dxHit = cluster->GetPositionError() * fPitch[ side ];

    // --- y coordinate is defined as sensor centre in y
    yHit  = fDy / 2.;
    // --- Error in y is length of sensor divided by sqrt(12)
    dyHit = fDy / 3.4641;
  } // ? X or Y side = X
    else {
      // --- y coordinate comes from cluster center in strips
      yHit = posHit;
      dyHit = cluster->GetPositionError() * fPitch[ side ];

      // --- x coordinate is defined as sensor centre in x
      xHit  = fDx / 2.;
      // --- Error in x is length of sensor divided by sqrt(12)
      dxHit = fDx / 3.4641;
    } // ? X or Y side = Y

  CreateHit(xHit, yHit, dxHit*dxHit, dyHit*dyHit, 0.,
            clusterF, clusterB, dxHit, dyHit);

}
// -------------------------------------------------------------------------


// -----   Diffusion   -----------------------------------------------------
void CbmStsSensorDssdOrtho::Diffusion(Double_t x, Double_t y,
                                      Double_t sigma, Int_t side,
                                      Double_t& fracL, Double_t& fracC,
                                      Double_t& fracR) {

  // Check side qualifier
  assert( side == 0 || side == 1);

  // Coordinate at the readout edge (y = fDy/2 ).
  // This x is counted from the left / bottom corner.
  Double_t aRo = (side == 0 ? x + fDx / 2. : y + fDy / 2.);

  // Centre strip number
  Int_t iStrip = TMath::FloorNint(aRo / fPitch[side]);

  // Strip boundaries at the readout edge
  Double_t aLeftRo  = Double_t(iStrip) * fPitch[side];
  Double_t aRightRo = aLeftRo + fPitch[side];

  // Distance from strip boundaries across the strip
  Double_t dLeft  = aRo - aLeftRo;
  Double_t dRight = aRightRo - aRo;

  // Charge fractions
  // The value 0.707107 is 1/sqrt(2)
  fracL = 0.;
  if ( dLeft < 3. * sigma )
    fracL = 0.5 * ( 1. - TMath::Erf( 0.707107 * dLeft  / sigma) );
  fracR = 0.;
  if ( dRight < 3. * sigma )
    fracR = 0.5 * ( 1. - TMath::Erf( 0.707107 * dRight / sigma) );
  fracC = 1. - fracL - fracR;

  LOG(debug4) << GetName() << ": Distances to next strip " << dLeft << " / "
      << dRight << ", charge fractions " << fracL << " / " << fracC
      << " / " << fracR;
}
// -------------------------------------------------------------------------



// -----   Get channel number in module   ----------------------------------
Int_t CbmStsSensorDssdOrtho::GetModuleChannel(Int_t strip, Int_t side,
                                              Int_t) const {

  // --- Check side
  assert( side == 0 || side == 1);

  // --- Account for front or back side
  Int_t channel = strip;
  if ( side ) channel += fNofStrips[0];

  return channel;
}
// -------------------------------------------------------------------------



// -----   Get strip and side from channel number   ------------------------
pair<Int_t, Int_t> CbmStsSensorDssdOrtho::GetStrip(Int_t channel,
                                                   Int_t) const  {

  Int_t stripNr = -1;
  Int_t side    = -1;

  // Check channel within bounds
  assert( 0 <= channel && ( channel < fNofStrips[0] + fNofStrips[1] ) );

  // --- Determine front or back side
  if ( channel < fNofStrips[0] ) {          // front side
    side = 0;
    stripNr = channel;
  }
  else {                                 // back side
    side = 1;
    stripNr = channel - fNofStrips[0];
  }

  return ( pair<Int_t, Int_t>(stripNr, side) );
}
// -------------------------------------------------------------------------



// -----   Get strip number from coordinates   -----------------------------
Int_t CbmStsSensorDssdOrtho::GetStripNumber(Double_t x, Double_t y,
                                            Int_t side) const {

  // Cave: This implementation assumes that the centre of the sensor volume
  // is also the centre of the active area, i.e. that the inactive borders
  // (guard ring) are symmetric both and x and y (not necessarily the same
  // in x and y).

  // Check side
  assert( side == 0 || side == 1);

  // Check whether in active area (should have been caught before)
  assert ( TMath::Abs(x) < 0.5 * fDx );
  assert ( TMath::Abs(y) < 0.5 * fDy );

  // Calculate distance from lower left corner of the active area,
  // in x for the front side, in y for the back side.
  // Note: the coordinates are given w.r.t. the centre of the volume.
  Double_t dist = ( side == 0 ? x + 0.5 * fDx : y + 0.5 * fDy);

  // Calculate corresponding strip number
  Int_t iStrip = TMath::FloorNint( dist / fPitch[side] );

  return iStrip;
}
// -------------------------------------------------------------------------



// -----   Initialise   ----------------------------------------------------
Bool_t CbmStsSensorDssdOrtho::Init() {

  // Check presence of node
  if ( ! fNode ) {
    LOG(error) << GetName() << ": No node assigned!";
    return kFALSE;
  }

  // Check whether parameters are assigned
  if ( fNofStrips[0] <= 0 ) {
    LOG(error) << GetName() << ": Parameters are not set!";
    return kFALSE;
  }

  // Geometric shape of the sensor volume
  TGeoBBox* shape = dynamic_cast<TGeoBBox*>(fNode->GetShape());
  assert(shape);

  // Active size in x coordinate
  fDx = Double_t(fNofStrips[0]) * fPitch[0];
  assert( fDx < 2. * shape->GetDX() );  // The strips fit into the volume

  // Active size in y coordinate
  fDy = Double_t(fNofStrips[1]) * fPitch[1];
  assert( fDy < 2. * shape->GetDY() );

  // Active size in z coordinate
  fDz = 2. * shape->GetDZ();

  // Set size of charge arrays
  fStripCharge[0].Set(fNofStrips[0]);
  fStripCharge[1].Set(fNofStrips[1]);

  // Daisy chains are not allowed. Make sure that this is the only sensor
  // connected to the module.
  assert ( GetSensorId() == 0);

  // --- Flag parameters to be set if test is OK
  fIsSet = SelfTest();

  LOG(info) << ToString();

  return fIsSet;
}
// -------------------------------------------------------------------------



// -----   Create hits from two clusters   ---------------------------------
Int_t CbmStsSensorDssdOrtho::IntersectClusters(CbmStsCluster* clusterF,
                                               CbmStsCluster* clusterB) {
  // --- Check pointer validity
  assert(clusterF);
  assert(clusterB);

  // --- Calculate cluster centre position on readout edge
  Int_t side  = -1;
  Double_t xF = -1.;
  Double_t xB = -1.;
  GetClusterPosition(clusterF->GetPosition(), xF, side);
  if ( side != 0 )
    LOG(fatal) << GetName() << ": Inconsistent side qualifier " << side
    << " for front side cluster! ";
  Double_t exF = clusterF->GetPositionError() * fPitch[ side ];
  Double_t du  = exF;
  GetClusterPosition(clusterB->GetPosition(), xB, side);
  if ( side != 1 )
    LOG(fatal) << GetName() << ": Inconsistent side qualifier " << side
    << " for back side cluster! ";
  Double_t exB = clusterB->GetPositionError() * fPitch[ side ];
  Double_t dv  = exB;

  // --- Should be inside active area
  if ( ! ( xF >= 0. || xF <= fDx) ) return 0;
  if ( ! ( xB >= 0. || xB <= fDy) ) return 0;

  // --- Hit counter
  Int_t nHits = 0;

  // In orthogonal sensor, all pairs of (front, back) cluster have
  // a single intersection
  // => exactly one hit!

  // --- Prepare hit coordinates and errors
  // In the coordinate system with origin at the bottom left corner,
  // the coordinates in the orthogonal sensor are straighforward.
  Double_t xC    = xF;   // x coordinate of intersection point
  Double_t yC    = xB;   // y coordinate of intersection point
  Double_t varX  = exF * exF;  // variance of xC
  Double_t varY  = exB * exB;  // variance of yC
  Double_t varXY = 0.; // covariance xC-yC => independent variables!

  // --- Transform into sensor system with origin at sensor centre
  xC -= 0.5 * fDx;
  yC -= 0.5 * fDy;

  // --- Create the hit
  CreateHit( xC, yC, varX, varY, varXY, clusterF, clusterB, du, dv);
  nHits++;

  return nHits;
}
// -------------------------------------------------------------------------


// -----   Create hits from two clusters   ---------------------------------
Int_t CbmStsSensorDssdOrtho::IntersectClustersVector(CbmStsCluster* clusterF,
                                               CbmStsCluster* clusterB) {
  // --- Check pointer validity
  assert(clusterF);
  assert(clusterB);

  // --- Calculate cluster centre position on readout edge
  Int_t side  = -1;
  Double_t xF = -1.;
  Double_t xB = -1.;
  GetClusterPosition(clusterF->GetPosition(), xF, side);
  if ( side != 0 )
    LOG(fatal) << GetName() << ": Inconsistent side qualifier " << side
    << " for front side cluster! ";
  Double_t exF = clusterF->GetPositionError() * fPitch[ side ];
  Double_t du  = exF;
  GetClusterPosition(clusterB->GetPosition(), xB, side);
  if ( side != 1 )
    LOG(fatal) << GetName() << ": Inconsistent side qualifier " << side
    << " for back side cluster! ";
  Double_t exB = clusterB->GetPositionError() * fPitch[ side ];
  Double_t dv  = exB;

  // --- Should be inside active area
  if ( ! ( xF >= 0. || xF <= fDx) ) return 0;
  if ( ! ( xB >= 0. || xB <= fDy) ) return 0;

  // --- Hit counter
  Int_t nHits = 0;

  // In orthogonal sensor, all pairs of (front, back) cluster have
  // a single intersection
  // => exactly one hit!

  // --- Prepare hit coordinates and errors
  // In the coordinate system with origin at the bottom left corner,
  // the coordinates in the orthogonal sensor are straighforward.
  Double_t xC    = xF;   // x coordinate of intersection point
  Double_t yC    = xB;   // y coordinate of intersection point
  Double_t varX  = exF * exF;  // variance of xC
  Double_t varY  = exB * exB;  // variance of yC
  Double_t varXY = 0.; // covariance xC-yC => independent variables!

  // --- Transform into sensor system with origin at sensor centre
  xC -= 0.5 * fDx;
  yC -= 0.5 * fDy;

  // --- Create the hit
  CreateHitVector( xC, yC, varX, varY, varXY, clusterF, clusterB, du, dv);
  nHits++;

  return nHits;
}
// -------------------------------------------------------------------------



// -----   Modify the strip pitch   ----------------------------------------
void CbmStsSensorDssdOrtho::ModifyStripPitch(Double_t pitch) {

  assert(fIsSet);  // Parameters should have been set before

  // Set new pitch and re-calculate number of strips on both sides
  fPitch[0] = pitch;
  fNofStrips[0] = Int_t( fDx / pitch );
  fDx = Double_t(fNofStrips[0]) * pitch;
  fPitch[1] = pitch;
  fNofStrips[1] = Int_t( fDy / pitch );
  fDy = Double_t(fNofStrips[1]) * pitch;

  // Set size of charge arrays
  fStripCharge[0].Set(fNofStrips[0]);
  fStripCharge[1].Set(fNofStrips[1]);

}
// -------------------------------------------------------------------------


// -----   Propagate charge to the readout strips   ------------------------
void CbmStsSensorDssdOrtho::PropagateCharge(Double_t x, Double_t y,
                                            Double_t z, Double_t charge,
                                            Double_t bY, Int_t side) {

  // Check side qualifier
  assert( side == 0 || side == 1);

  Double_t xCharge = x;
  Double_t yCharge = y;
  Double_t zCharge = z;

  // Debug
  LOG(debug4) << GetName() << ": Propagating charge " << charge
      << " from (" << x << ", " << y << ", " << z
      << ") on side " << side << " of sensor " << GetName();

  // Lorentz shift on the drift to the readout plane
  if ( CbmStsPhysics::Instance()->UseLorentzShift() ) {
    xCharge += LorentzShift(z, side, bY);
  }

  LOG(debug4) << GetName() << ": After Lorentz shift: (" << xCharge << ", "
      << yCharge << ", " << zCharge << ") cm";

  // Stop if the charge after Lorentz shift is not in the active area.
  // Diffusion into the active area is not treated.
  if ( ! IsInside(xCharge, yCharge) ) {
    LOG(debug4) << GetName() << ": Charge outside active area";
    return;
  }

  // No diffusion: all charge is in one strip
  if (  ! CbmStsPhysics::Instance()->UseDiffusion() ) {
    Int_t iStrip = GetStripNumber(xCharge, yCharge, side);
    fStripCharge[side][iStrip] += charge;
    LOG(debug4) << GetName() << ": Adding charge " << charge << " to strip "
        << iStrip;
  } //? Do not use diffusion

  // Diffusion: charge is distributed over centre strip and neighbours
  else {
    // Calculate diffusion width
    Double_t diffusionWidth =
        CbmStsPhysics::DiffusionWidth(z + fDz / 2.,  // distance from back side
                                      fDz,
                                      GetConditions()->GetVbias(),
                                      GetConditions()->GetVfd(),
                                      GetConditions()->GetTemperature(),
                                      side);
    assert (diffusionWidth >= 0.);
    LOG(debug4) << GetName() << ": Diffusion width = " << diffusionWidth
        << " cm";
    // Calculate charge fractions in strips
    Double_t fracL = 0.;  // fraction of charge in left neighbour
    Double_t fracC = 1.;  // fraction of charge in centre strip
    Double_t fracR = 0.;  // fraction of charge in right neighbour
    Diffusion(xCharge, yCharge, diffusionWidth, side, fracL, fracC, fracR);
    // Calculate strip numbers
    // Note: In this implementation, charge can diffuse out of the sensitive
    // area only for vertical strips. In case of stereo angle (cross-connection
    // of strips), all charge is assigned to some strip, so the edge effects
    // are not treated optimally.
    Int_t iStripC  = GetStripNumber(xCharge, yCharge, side);  // centre strip
    Int_t iStripL  = iStripC - 1;                             // left neighbour
    Int_t iStripR  = iStripC + 1;                             // right neighbour
    // Collect charge on the readout strips
    if ( fracC > 0. ) {
      fStripCharge[side][iStripC] += charge * fracC;    // centre strip
      LOG(debug4) << GetName() << ": Adding charge " << charge * fracC
          << " to strip " << iStripC;
    }
    if ( fracL > 0. && iStripL >= 0 ) {
      fStripCharge[side][iStripL] += charge * fracL;  // right neighbour
      LOG(debug4) << GetName() << ": Adding charge " << charge * fracL
          << " to strip " << iStripL;
    }
    if ( fracR > 0. && iStripR < fNofStrips[side] ) {
      fStripCharge[side][iStripR] += charge * fracR;  // left neighbour
      LOG(debug4) << GetName() << ": Adding charge " << charge * fracR
          << " to strip " << iStripR;
    }
  } //? Use diffusion

}
// -------------------------------------------------------------------------



// -----   Set internal sensor parameters   --------------------------------
Bool_t CbmStsSensorDssdOrtho::SetParameters(Int_t nStripsF, Double_t pitchF,
                                            Int_t nStripsB, Double_t pitchB) {

  // Geometric shape of the sensor volume
  TGeoBBox* shape = dynamic_cast<TGeoBBox*>(GetPnode()->GetShape());

  // Active size in x coordinate
  fDx = Double_t(nStripsF) * pitchF;
  assert( fDx < 2. * shape->GetDX() );  // The strips fit into the volume

  // Active size in y coordinate
  fDy = Double_t(nStripsB) * pitchB;
  assert( fDy < 2. * shape->GetDY() );

  // Active size in z coordinate
  fDz = 2. * shape->GetDZ();

  // Number of strips, pitch and stereo angles
  fNofStrips[0] = nStripsF;
  fPitch[0]     = pitchF;
  fNofStrips[1] = nStripsB;
  fPitch[1]     = pitchB;

  // Set size of charge arrays
  fStripCharge[0].Set(fNofStrips[0]);
  fStripCharge[1].Set(fNofStrips[1]);

  // Daisy chains are not allowed. Make sure that this is the only sensor
  // connected to the module.
  assert ( GetSensorId() == 0);

  // --- Flag parameters to be set if test is ok
  fIsSet = SelfTest();

  return fIsSet;
}
// -------------------------------------------------------------------------



// -----   String output   -------------------------------------------------
string CbmStsSensorDssdOrtho::ToString() const
{
  stringstream ss;
  ss << "Sensor " << fName << "(type " << GetTitle() << ", addr " << Form("%08x", fAddress) << "): ";
  if ( ! fIsSet ) {
    ss << "parameters are not set";
    return ss.str();
  }
  if ( ! GetPnode() ) ss << "no node assigned; ";
  else {
    std::cout << GetPnode()->GetName() << std::endl;
    GetPnode()->GetMatrix( GetPnode()->GetLevel() )->Print();
  }
  ss << "Dimension (" << fDx << ", " << fDy << ", " << fDz << ") cm, ";
  ss << "# strips " << fNofStrips[0] << "/" << fNofStrips[1] << ", ";
  ss << "pitch " << fPitch[0] << "/" << fPitch[1] << " cm, ";
  return ss.str();
}
// -------------------------------------------------------------------------



ClassImp(CbmStsSensorDssdOrtho)
