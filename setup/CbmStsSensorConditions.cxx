/** @file CbmStsSensorConditions.cxx
 ** @author Volker Friese <v.friese@gsi.de>
 ** @date 28.06.2014
 **/

#include <iomanip>
#include <sstream>
#include "FairLogger.h"
#include "CbmStsSensorConditions.h"

using namespace std;


// -----   Default constructor   -------------------------------------------
CbmStsSensorConditions::CbmStsSensorConditions(Double_t vFd,
		                                           Double_t vBias,
		                                           Double_t temperature,
		                                           Double_t cCoupling,
		                                           Double_t cInterstrip,
		                                           Double_t bX,
		                                           Double_t bY,
		                                           Double_t bZ) :
		                                           TObject(),
		                                           fVfd(vFd),
		                                           fVbias(vBias),
		                                           fTemperature(temperature),
		                                           fCcoupling(cCoupling),
		                                           fCinterstrip(cInterstrip),
		                                           fCrossTalk(0.),
		                                           fBx(bX),
		                                           fBy(bY),
		                                           fBz(bZ)
{
	if ( fCinterstrip + fCcoupling != 0. )
		fCrossTalk = cInterstrip / (cInterstrip + cCoupling);
	CalculateHallMobilityParameters();
}
// -------------------------------------------------------------------------


// -----   Destructor   ----------------------------------------------------
CbmStsSensorConditions::~CbmStsSensorConditions() { }
// -------------------------------------------------------------------------



// -----   Set parameters for Hall mobility calculation   ------------------
void CbmStsSensorConditions::CalculateHallMobilityParameters() {

	  // These are the parameters needed for the calculation of the Hall
	  // mobility, i.e. the mobility of charge carriers in the silicon
	  // in the presence of a magnetic field. They depend on the temperature.
	  // Values and formulae are taken from
	  // V. Bartsch et al., Nucl. Instrum. Methods A 497 (2003) 389

    Double_t muLow[2], vSat[2], beta[2], rHall[2], muHall[2];

    // electrons
    muLow[0] = 1417.  * pow (fTemperature / 300., -2.2);// cm^2 / (V s)
    beta[0]  = 1.109  * pow (fTemperature / 300., 0.66);
    vSat[0]  = 1.07e7 * pow (fTemperature / 300., 0.87);// cm / s 
    rHall[0] = 1.15;

    fHallMobilityParametersE[0] = muLow[0];
    fHallMobilityParametersE[1] = beta[0];
    fHallMobilityParametersE[2] = vSat[0];
    fHallMobilityParametersE[3] = rHall[0];

    // holes
    muLow[1] = 470.5   * pow (fTemperature / 300., -2.5);// cm^2 / (V s)
    beta[1]  = 1.213   * pow (fTemperature / 300., 0.17);
    vSat[1]  = 0.837e7 * pow (fTemperature / 300., 0.52);// cm / s 
    rHall[1] = 0.7;

    fHallMobilityParametersH[0] = muLow[1];
    fHallMobilityParametersH[1] = beta[1];
    fHallMobilityParametersH[2] = vSat[1];
    fHallMobilityParametersH[3] = rHall[1];

    //calculate mean shift TODO use CbmStsPhysics in e-field calculation
    Double_t dZ = 0.03, E = (fVbias - fVfd) / dZ + 2 * fVfd / dZ;//dZ - sensor thickness, E - el field [V/cm]
    Int_t nSteps = 1000;
    Double_t deltaZ = dZ / nSteps; 
    Double_t dxMean[2];
    dxMean[0] = dxMean[1] = 0.;
    
    for (Int_t j = 0; j <= nSteps; j++){
	E -= 2 * fVfd / dZ * deltaZ / dZ;//V / cm
	for (Int_t i = 0; i < 2; i++){
	    muHall[i] = rHall[i] * muLow[i] / pow ((1 + pow (muLow[i] * E / vSat[i], beta[i])), 1 / beta[i]);
	    if (i == 1) dxMean[i] += muHall[i] * j * deltaZ; 
	    if (i == 0) dxMean[i] += muHall[i] * (dZ - j * deltaZ); 
	}
    }
    for (Int_t i = 0; i < 2; i++) dxMean[i] /= nSteps;
     fMeanLorentzShift[0] = dxMean[0] * fBy * 1.e-4;
     fMeanLorentzShift[1] = dxMean[1] * fBy * 1.e-4;

}
// -------------------------------------------------------------------------



// -----   Get one of the Hall mobility parameters   -----------------------
Double_t CbmStsSensorConditions::GetHallParameter(Int_t index,
		                                              Int_t chargeType) {
	if ( index < 0 || index > 3 ) {
		LOG(error) << "SensorConditions: Invalid hall parameter index "
				       << index;
		return 0.;
	}
	if      ( chargeType == 0 ) return fHallMobilityParametersE[index];
	else if ( chargeType == 1 ) return fHallMobilityParametersH[index];
	else {
		LOG(error) << "SensorConditions: Invalid charge type " << chargeType;
	}

	return 0.;
}
// -------------------------------------------------------------------------



// -----   Get parameters for Hall mobility calculation into array  --------
void CbmStsSensorConditions::GetHallMobilityParametersInto(Double_t * hallMobilityParameters, Int_t chargeType) const {

   if (chargeType == 0) { // electrons
       for (Int_t i = 0; i < 4; i++) hallMobilityParameters[i] = fHallMobilityParametersE[i];

   } else if (chargeType == 1) { // holes
       for (Int_t i = 0; i < 4; i++) hallMobilityParameters[i] = fHallMobilityParametersH[i];

   } else LOG(error) << GetName() << "Cannot get parameter for Hall mobility. Unknown type of charge carriers. Must be 0 or 1!";

}
// -------------------------------------------------------------------------



// -----   Hall mobility   -------------------------------------------------
Double_t CbmStsSensorConditions::HallMobility(Double_t eField,
		                                          Int_t chargeType) const {

	Double_t muLow = 0.;   // mobility at low electric field
	Double_t beta  = 0.;   // exponent
	Double_t vSat  = 0.;   // saturation velocity
	Double_t rHall = 0.;   // Hall scattering factor

	if ( chargeType == 0 ) {   // electrons
	  muLow = fHallMobilityParametersE[0];
	  beta  = fHallMobilityParametersE[1];
	  vSat  = fHallMobilityParametersE[2];
	  rHall = fHallMobilityParametersE[3];
	}  //? electron
	else if ( chargeType == 1 ) {  // holes
	  muLow = fHallMobilityParametersH[0];
	  beta  = fHallMobilityParametersH[1];
	  vSat  = fHallMobilityParametersH[2];
	  rHall = fHallMobilityParametersH[3];
	}  //? holes
	else {
		LOG(error) << "SensorConditions: illegal charge type "
				       << chargeType;
		return 0.;
	}

	Double_t factor = pow( muLow * eField / vSat, beta );
	Double_t muHall = rHall * muLow / pow( 1 + factor, 1./beta );
	return muHall;
}
// -------------------------------------------------------------------------





// -----   String output   -------------------------------------------------
string CbmStsSensorConditions::ToString() const {
	stringstream ss;
	ss << "VFD = " << fVfd << " V, V(bias) = " << fVbias << " V, T = "
		 << fTemperature << " K, C(coupl.) = " << fCcoupling
		 << " pF, C(int.) = " << fCinterstrip
		 << " pF, cross-talk coeff. = " << fCrossTalk <<  "B = ("
		 << setprecision(3) << fixed << fBx << ", " << fBy << ", " << fBz
		 << ") T";
	return ss.str();
}
// -------------------------------------------------------------------------


ClassImp(CbmStsSensorConditions)
