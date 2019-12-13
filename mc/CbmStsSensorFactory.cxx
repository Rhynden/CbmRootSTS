/** @file CbmStsSensorFactory.cxx
 ** @author Volker Friese <v.friese@gsi.de>
 ** @date 18.11.2014
 **/

#include <iomanip>

#include "TGeoManager.h"

#include "FairLogger.h"

#include "mc/CbmStsSensorFactory.h"


// -----   Initialisation of static singleton pointer   --------------------
CbmStsSensorFactory* CbmStsSensorFactory::fgInstance = NULL;
// -------------------------------------------------------------------------



// -----   Constructor (private)   -----------------------------------------
CbmStsSensorFactory::CbmStsSensorFactory()
  : TNamed("StsSensorFactory", ""),
    fSensors()
{
	Int_t nSensors = DefineSensors();
	LOG(info) << GetName() << ": " << nSensors << " sensors created.";
}
// -------------------------------------------------------------------------



// -----   Create sensor volume (private)   --------------------------------
Bool_t CbmStsSensorFactory::CreateSensor(TString& name, TString& material,
		                                     Double_t xSize, Double_t ySize,
		                                     Double_t thickness,
		                                     EColor color) {

	// --- Check presence of TGeoManager
	if ( ! gGeoManager ) {
		LOG(error) << GetName() << ": no TGeoManager present!";
	  return kFALSE;
	}

	// --- Check presence of material
	TGeoMedium* medium = gGeoManager->GetMedium(material.Data());
	if ( ! medium ) {
		LOG(error) << GetName() << ": medium " << material << " not found!";
		return kFALSE;
	}

	// --- Construct the sensor volume
	TGeoVolume* sensor = gGeoManager->MakeBox(name, medium,
			                                      xSize/2., ySize/2, thickness/2.);
	sensor->SetLineColor(color);
	fSensors.push_back(sensor);
	LOG(info) << GetName() << ": creating sensor "
			      << std::setw(10) << name
			      << ", material " << std::setw(10) << material
			      << ", size (" << std::fixed << std::setprecision(5) << xSize
			      << ", " << ySize << ", " << thickness << ") cm";
	return kTRUE;

}
// -------------------------------------------------------------------------



// -----   Sensor type definition (private)   ------------------------------
Int_t CbmStsSensorFactory::DefineSensors() {

	// --- Sensor counter
	Int_t nSensors = 0;

	// --- Check presence of TGeoManager
	if ( ! gGeoManager ) {
		LOG(error) << "StsSensorFactory: no TGeoManeger present!";
	  return 0;
	}

	// --- Sensor properties variables
	TString  sensorName = "";
	TString  material   = "";
	Double_t xSize      = 0.;
	Double_t ySize      = 0.;
	Double_t thickness  = 0.;
	EColor   colour     = kRed;

  // --- sensor01: Half small sensor (4 cm x 2.2 cm)
	sensorName = "Sensor01";
  material   = "silicon";
  xSize      = 4.0;
  ySize      = 2.2;
  thickness  = 0.03;
  colour      = kYellow;
  if ( CreateSensor(sensorName, material, xSize, ySize, thickness, colour) )
  	nSensors++;

  // --- sensor02: small sensor (6.2 cm x 2.2 cm)
	sensorName = "Sensor02";
  material   = "silicon";
  xSize      = 6.1992;
  ySize      = 2.2;
  thickness  = 0.03;
  colour      = kRed;
  if ( CreateSensor(sensorName, material, xSize, ySize, thickness, colour) )
  	nSensors++;

  // --- sensor03: medium sensor (6.2 cm x 4.2 cm)
	sensorName = "Sensor03";
  material   = "silicon";
  xSize      = 6.1992;
  ySize      = 4.2;
  thickness  = 0.03;
  colour      = kGreen;
  if ( CreateSensor(sensorName, material, xSize, ySize, thickness, colour) )
  	nSensors++;

  // --- sensor04: big sensor (6.2 cm x 6.2 cm)
	sensorName = "Sensor04";
  material   = "silicon";
  xSize      = 6.1992;
  ySize      = 6.2;
  thickness  = 0.03;
  colour      = kBlue;
  if ( CreateSensor(sensorName, material, xSize, ySize, thickness, colour) )
  	nSensors++;

  // --- sensor05: "in-hole" sensor (3.1 cm x 3.1 cm)
	sensorName = "Sensor05";
  material   = "silicon";
  xSize      = 3.1;
  ySize      = 3.1;
  thickness  = 0.03;
  colour      = kYellow;
  if ( CreateSensor(sensorName, material, xSize, ySize, thickness, colour) )
  	nSensors++;

  // --- sensor06: mini-medium sensor (1.5 cm x 4.2 cm)
	sensorName = "Sensor06";
  material   = "silicon";
  xSize      = 1.5;
  ySize      = 4.2;
  thickness  = 0.03;
  colour      = kYellow;
  if ( CreateSensor(sensorName, material, xSize, ySize, thickness, colour) )
  	nSensors++;


  return nSensors;
}
// -------------------------------------------------------------------------



// ----- Get static instance   ---------------------------------------------
CbmStsSensorFactory* CbmStsSensorFactory::Instance() {
	  if ( ! fgInstance ) fgInstance = new CbmStsSensorFactory();
	  return fgInstance;
}
// -------------------------------------------------------------------------



ClassImp(CbmStsSensorFactory)


