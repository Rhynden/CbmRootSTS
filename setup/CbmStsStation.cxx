/** @file CbmStsStation.cxx
 ** @author Volker Friese <v.friese@gsi.de>
 ** @date 04.03.2015
 **/


#include "CbmStsStation.h"

#include <cassert>
#include <sstream>
#include "TGeoBBox.h"
#include "TGeoManager.h"
#include "TGeoMatrix.h"
#include "TGeoVolume.h"
#include "CbmStsSensor.h"
#include "CbmStsSensorDssd.h"
#include "CbmStsSensorDssdStereo.h"

using std::stringstream;
using std::string;


// -----   Default constructor   -------------------------------------------
CbmStsStation::CbmStsStation() :
    TNamed(),
    fZ(0.),
    fXmin(0.), fXmax(0.), fYmin(0.), fYmax(0.), fSensorD(0.), fSensorRot(0.),
    fNofSensors(0),
    fDiffSensorD(kFALSE),
    fFirstSensor(NULL),
    fNode(NULL),
    fLadders()
{
}
// -------------------------------------------------------------------------



// -----   Standard constructor   ------------------------------------------
CbmStsStation::CbmStsStation(const char* name, const char* title,
		                                 TGeoPhysicalNode* node) :
	TNamed(name, title),
	fZ(0.),
    fXmin(0.), fXmax(0.), fYmin(0.), fYmax(0.), fSensorD(0.), fSensorRot(0.),
    fNofSensors(0),
    fDiffSensorD(kFALSE),
    fFirstSensor(NULL),
    fNode(node),
    fLadders()
{
}
// -------------------------------------------------------------------------



// -----   Destructor   ----------------------------------------------------
CbmStsStation::~CbmStsStation() {
}
// -------------------------------------------------------------------------


// -----   Add a ladder to the station   -----------------------------------
void CbmStsStation::AddLadder(CbmStsElement* ladder) {

  // Check whether argument really is a ladder
  assert(ladder);
  assert(ladder->GetLevel() == kStsLadder);

  // Add to daughter array
  fLadders.push_back(ladder);

}
// -------------------------------------------------------------------------


// -----   Initialise the station properties from sensors   ----------------
void CbmStsStation::CheckSensorProperties() {

	Int_t nSensors = 0;         // sensor counter
	Double_t zMin  =  999999.;  // sensor z minimum
	Double_t zMax  = -999999.;  // sensor z maximum

	// --- Loop over ladders
    for (UInt_t iLad = 0; iLad < fLadders.size(); iLad++) {
      CbmStsElement* ladd = fLadders.at(iLad);

  	// --- Loop over half-ladders
 	for (Int_t iHla = 0; iHla < ladd->GetNofDaughters(); iHla++) {
  		CbmStsElement* hlad = ladd->GetDaughter(iHla);

  		// --- Loop over modules
  		for (Int_t iMod = 0; iMod < hlad->GetNofDaughters(); iMod++) {
  			CbmStsElement* modu = hlad->GetDaughter(iMod);

  			// --- Loop over sensors
  			for (Int_t iSen = 0; iSen < modu->GetNofDaughters(); iSen++) {
  			    CbmStsSensor* sensor =
  			 			dynamic_cast<CbmStsSensor*>(modu->GetDaughter(iSen));

  				// Set first sensor
  				if ( ! nSensors ) fFirstSensor = sensor;

  				// Get sensor z position
  		    TGeoPhysicalNode* sensorNode = sensor->GetPnode();
  		    // --- Transform sensor centre into global C.S.
  		    Double_t local[3] = {0., 0., 0.};  // sensor centre, local c.s.
   		    Double_t global[3];                // sensor centre, global c.s.
    		  sensorNode->GetMatrix()->LocalToMaster(local, global);
    		  if ( ! nSensors ) {  // first sensor
    		  	zMin = global[2];
    		  	zMax = global[2];
    		  }
    		  else {
    		  	zMin = TMath::Min(zMin, global[2]);
    		  	zMax = TMath::Max(zMax, global[2]);
    		  }

    		  // Get sensor thickness
    		  TGeoBBox* sBox = dynamic_cast<TGeoBBox*>(sensorNode->GetShape());
    		  if ( ! sBox )
    		  	LOG(fatal) << GetName() << ": sensor shape is not a box!";
    		  	Double_t sD = 2. * sBox->GetDZ();
    		    if ( ! nSensors ) fSensorD = sD; // first sensor
    		    else {
    		    	if ( TMath::Abs(sD - fSensorD) > 0.0001 )
    		    		fDiffSensorD = kTRUE;
    		    }

    		  nSensors++;
  			} // # sensors
  		} // # modules
  	} // # half-ladders
  } // # ladders

  fZ = 0.5 * (zMin + zMax);
  fNofSensors = nSensors;
}
// -------------------------------------------------------------------------



// -----   Strip pitch    --------------------------------------------------
Double_t CbmStsStation::GetSensorPitch(Int_t side) const {

  assert( side == 0 || side == 1);

  if ( ! fFirstSensor ) {
    LOG(warn) << GetName() << ": No sensors connected to station!";
    return 0.;
  }

  // Action only for Dssd sensors
  CbmStsSensorDssd* sensor = dynamic_cast<CbmStsSensorDssd*>(fFirstSensor);
  Double_t pitch = -1.;
  if ( sensor ) pitch = sensor->GetPitch(side);
  else {
    LOG(warn) << GetName() << ": Cannot get pitch for non-Dssd sensor.";
  }

  return pitch;
}
// -------------------------------------------------------------------------



// -----   Stereo angle    -------------------------------------------------
Double_t CbmStsStation::GetSensorStereoAngle(Int_t side) const {

  assert ( side == 0 || side == 1);

  if ( ! fFirstSensor ) {
     LOG(warn) << GetName() << ": No sensors connected to station!";
     return 0.;
  }

  // Action only for Dssd sensors
  CbmStsSensorDssdStereo* sensor =
    dynamic_cast<CbmStsSensorDssdStereo*>(fFirstSensor);
  Double_t stereo = 0.;
  if ( sensor ) stereo = sensor->GetStereoAngle(side);
  else {
    LOG(warn) << GetName()
        << ": Cannot get stereo angle for non-DssdStereo sensor.";
  }

  return stereo;
}
// -------------------------------------------------------------------------



// -----   Initialise station parameters   ---------------------------------
void CbmStsStation::Init() {

	// Determine x and y extensions of the station, in case it is present
	// as TGeoNode (for old geometries). This implementation assumes that
	// the shape of the station volume derives from TGeoBBox and that it is
	// not rotated in the global c.s.
	if ( fNode ) {
		TGeoBBox* box = dynamic_cast<TGeoBBox*>(fNode->GetShape());
		if ( ! box )
			LOG(fatal) << GetName() << ": shape is not box! ";
		Double_t local[3] = { 0., 0., 0.};
		Double_t global[3];
	  fNode->GetMatrix()->LocalToMaster(local, global);
	  fXmin = global[0] - box->GetDX();
	  fXmax = global[0] + box->GetDX();
	  fYmin = global[1] - box->GetDY();
	  fYmax = global[1] + box->GetDY();
	}

	// For new geometries with units instead of stations, the station element
	// is not a node in the geometry. To obtain its extensions in x and y,
	// a station volume is transiently made as TGeoVolumeAssembly, composed
	// of its ladder daughters.
	else {
	  TGeoVolumeAssembly* statVol = new TGeoVolumeAssembly("myStation");
	  for (UInt_t iLadder = 0; iLadder < fLadders.size(); iLadder++) {
	  	TGeoVolume* ladVol = fLadders.at(iLadder)->GetPnode()->GetVolume();
	  	TGeoHMatrix* ladMat = fLadders.at(iLadder)->GetPnode()->GetMatrix();
	  	statVol->AddNode(ladVol, iLadder, ladMat);
	  } // # ladders in station
	  statVol->GetShape()->ComputeBBox();
	  TGeoBBox* statShape = dynamic_cast<TGeoBBox*>(statVol->GetShape());
	  const Double_t* origin = statShape->GetOrigin();
	  fXmin = origin[0] - statShape->GetDX();
	  fXmax = origin[0] + statShape->GetDX();
	  fYmin = origin[1] - statShape->GetDY();
	  fYmax = origin[1] + statShape->GetDY();
	}

    // The z position of the station is obtained from the sensor positions,
	// not from the station node. This is more flexible, because it does not
	// assume the station to be symmetric.
	CheckSensorProperties();

	// Warning if varying sensor properties are found
	if ( fDiffSensorD )
		LOG(warn) << GetName() << ": Different values for sensor thickness!";

	// Determine the rotation (in x-y) of the first sensor
	assert(fFirstSensor);
	TGeoPhysicalNode* sensorNode = fFirstSensor->GetNode();
	assert(sensorNode);
	// Transform unit vector on local x axis into global c.s.
	Double_t unitLocal[3] = {1., 0., 0.};
	Double_t unitGlobal[3];
	sensorNode->GetMatrix()->LocalToMaster(unitLocal, unitGlobal);
	// Subtract translation vector of local origin
	Double_t* translation = sensorNode->GetMatrix()->GetTranslation();
	unitGlobal[0] -= translation[0];
	unitGlobal[1] -= translation[1];
	unitGlobal[2] -= translation[2];
	// Calculate angle between unit x vector in global and local c.s.
	fSensorRot = atan2(unitGlobal[1], unitGlobal[0]);

}
// --------------------------------------------------------------------------



// -----   Info   -----------------------------------------------------------
string CbmStsStation::ToString() const
{
   stringstream ss;
   ss << GetName() << ": " << fNofSensors << " sensors, z = " << fZ
  	  << " cm, x = " << fXmin << " to " << fXmax << " cm, y = " << fYmin
  	  << " to " << fYmax << " cm " << "\n\t\t"
  	  << " rotation " << fSensorRot * 180. / 3.1415927 << " degrees,"
  	  << " sensor thickness " << fSensorD << " cm,"
  	  << " pitch " << GetSensorPitch(0) << " cm / " << GetSensorPitch(1)
  	  << " cm, stereo angle " << GetSensorStereoAngle(0) << " / "
  	  << GetSensorStereoAngle(1);

   return ss.str();
}
// --------------------------------------------------------------------------

ClassImp(CbmStsStation)

