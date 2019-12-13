/** @file CbmStsDigitizeParameters.cxx
 ** @author Florian Uhlig <f.uhlig@gsi.de>
 ** @date 12.06.2017
 **/

#include "CbmStsDigitizeParameters.h"

#include "FairParamList.h"
#include "FairDetParIo.h"
#include "FairParIo.h"

#include <sstream>
#include <iomanip>

using std::fixed;
using std::setprecision;
using std::setw;
using std::right;

ClassImp(CbmStsDigitizeParameters)


// -----   Constructor   ----------------------------------------------------
CbmStsDigitizeParameters::CbmStsDigitizeParameters(const char* name,
                                                   const char* title,
                                                   const char* context)
  : FairParGenericSet(name, title, context)
{
}
// --------------------------------------------------------------------------



// -----   Set default values   ---------------------------------------------
void CbmStsDigitizeParameters::SetDefaults() {

  // --- Processes
  fELossModel         = kELossUrban;
  fUseLorentzShift    = kTRUE;
  fUseDiffusion       = kTRUE;
  fUseCrossTalk       = kTRUE;
  fGenerateNoise      = kFALSE;
  fDiscardSecondaries = kFALSE;

  // --- Sensor conditions
  fVdep        =  70.;   // V
  fVbias       = 140.;   // V
  fTemperature = 268.;   // K
  fCcoup       =  17.5;  // pF
  fCis         =   1.;   // pF

  // --- Module parameters
  fDynRange        = 75000.;        // e
  fThreshold       =  3000.;        // e
  fNofAdc          =    32;
  fTimeResolution  =     5.;        // ns
  fDeadTime        =   800.;        // ns
  fNoise           =  1000.;        // e
  fZeroNoiseRate   =     3.9789e-3; // 1/ns
  fDeadChannelFrac =     0.;        // %
  fDeadChannelMap = {};        //

  // --- Strip pitch
  fStripPitch = -1.;   // No user-defined pitch

  // --- Flags
  fIsInit    = kTRUE;
  fIsDefault = kTRUE;

  fNoiseRate = 0.;
  if (fNoiseCharge) {
    delete fNoiseCharge;
    fNoiseCharge = nullptr;
  }
}
// --------------------------------------------------------------------------
void CbmStsDigitizeParameters::SetModuleParameters(Double_t dynRange, Double_t threshold,
                                                   Int_t nAdc, Double_t timeResol,
                                                   Double_t deadTime, Double_t noise,
                                                   Double_t zeroNoiseRate,
                                                   Double_t deadChannelFrac,
                                                   std::set<UChar_t> deadChannelMap)
{
  // Assert validity of parameters
  assert( dynRange > 0. );
  assert( threshold > 0. );
  assert( nAdc > 0 );
  assert( timeResol > 0. );
  assert( deadTime >= 0. );
  assert( noise >= 0. );
  assert( zeroNoiseRate >= 0. );
  assert( deadChannelFrac >= 0. && deadChannelFrac <= 1.);

  fDynRange        = dynRange;
  fThreshold       = threshold;
  fNofAdc          = nAdc;
  fTimeResolution  = timeResol;
  fDeadTime        = deadTime;
  fNoise           = noise;
  fZeroNoiseRate   = zeroNoiseRate;
  fDeadChannelFrac = deadChannelFrac;
  fDeadChannelMap  = deadChannelMap;

  fNoiseRate = 0.;
  if (fNoiseCharge) {
    delete fNoiseCharge;
    fNoiseCharge = nullptr;
  }

  setChanged();
  setInputVersion(-2,1);
  fIsInit = kTRUE;
}

// ----- String output   ----------------------------------------------------
std::string CbmStsDigitizeParameters::ToString() const {
  std::stringstream ss;

  ss << GetTitle() << ":";
  if ( ! fIsInit ) {
    ss << " not initialised";
  }
  else {
    if ( fIsDefault ) ss << " default settings";
    ss << "\n\t  Energy loss model ";
    switch (fELossModel) {
      case kELossIdeal: ss << "IDEAL"; break;
      case kELossUniform: ss << "UNIFORM"; break;
      case kELossUrban: ss << "Urban model"; break;
      default: ss << "!!! UNKNOWN !!!"; break;
    }
    ss << "\n\t  Lorentz shift     " << (fUseLorentzShift ? "ON" : "OFF");
    ss << "\n\t  Diffusion         " << (fUseDiffusion ? "ON" : "OFF");
    ss << "\n\t  Cross-talk        " << (fUseCrossTalk ? "ON" : "OFF");
    ss << "\n\t  Noise             " << (fGenerateNoise ? "ON" : "OFF");

    ss << "\n\t  Sensor operation conditions :\n";
    ss << "\t\t Depletion voltage         "
        << fVdep << " V\n";
    ss << "\t\t Bias voltage              "
        << fVbias << " V\n";
    ss << "\t\t Temperature               "
        << fTemperature << " K\n";
    ss << "\t\t Coupling capacitance      "
        << fCcoup << " pF\n";
    ss << "\t\t Inter-strip capacitance   "
        << fCis << " pF\n";

    ss << "\t  ASIC parameters :\n";
    ss << "\t\t Dynamic range             "
        << fDynRange << " e\n";
    ss << "\t\t Threshold                 "
        << fThreshold << " e\n";
    ss << "\t\t ADC channels              "
        << fNofAdc << " \n";
    ss << "\t\t Time resolution           "
        << fTimeResolution << " ns\n";
    ss << "\t\t Dead time                 "
        << fDeadTime << " ns\n";
    ss << "\t\t Noise (RMS)               "
        << fNoise << " e\n";
    ss << "\t\t Zero noise rate           "
        << fZeroNoiseRate << " / ns\n";
    ss << "\t\t Fraction of dead channels "
        << fDeadChannelFrac << "\n";

    ss << "\t\t Number of dead channels   "
        << fDeadChannelMap.size();

    if ( fDiscardSecondaries ) ss << "\n\t!!! Secondaries will be discarded!!!";
    if ( fStripPitch > 0. ) ss << "\n\t!!! Overriding strip pitch with "
        << fStripPitch << " cm !!!";
  }

   return ss.str();
}
// --------------------------------------------------------------------------


// -----   Public method clear   -------------------------------------------
void CbmStsDigitizeParameters::clear()
{
  status = kFALSE;
  fIsInit = kFALSE;
  fIsDefault = kFALSE;
  resetInputVersions();
}
// -------------------------------------------------------------------------



// -----   Write parameters to ASCII file   --------------------------------
void CbmStsDigitizeParameters::putParams(FairParamList* l)
{
    if (!l) return;

    l->add("ELossModel", fELossModel);
    l->add("UseLorentzShift", static_cast<Int_t>(fUseLorentzShift));
    l->add("UseDiffusion", static_cast<Int_t>(fUseDiffusion));
    l->add("UseCrossTalk", static_cast<Int_t>(fUseCrossTalk));
    l->add("GenerateNoise", static_cast<Int_t>(fGenerateNoise));
    l->add("Vdep", fVdep);
    l->add("Vbias", fVbias);
    l->add("Temperature", fTemperature);
    l->add("Ccoup", fCcoup);
    l->add("Cis", fCis);
    l->add("DynRange", fDynRange);
    l->add("Threshold", fThreshold);
    l->add("NofAdc", fNofAdc);
    l->add("TimeResolution", fTimeResolution);
    l->add("DeadTime", fDeadTime);
    l->add("Noise", fNoise);
    l->add("ZeroNoiseRate", fZeroNoiseRate);
    l->add("StripPitch", fStripPitch);
    l->add("DiscardSecondaries", static_cast<Int_t>(fDiscardSecondaries));
    l->add("IsInit", static_cast<Int_t>(fIsInit));
    l->add("IsDefault", static_cast<Int_t>(fIsDefault));

    std::stringstream deadChannelMapStream;
    std::copy(fDeadChannelMap.begin(), fDeadChannelMap.end(), std::ostream_iterator<int>(deadChannelMapStream, ","));

    l->add("DeadChannelMap", (Text_t*)deadChannelMapStream.str().c_str());
}
// -------------------------------------------------------------------------



// -----   Read parameters from ASCII file   -------------------------------
Bool_t CbmStsDigitizeParameters::getParams(FairParamList* l)
{

    if (!l) return kFALSE;

    Int_t iTemp;

    if ( ! l->fill("ELossModel", &iTemp) ) return kFALSE;
    switch(iTemp) {
      case kELossIdeal: fELossModel = kELossIdeal; break;
      case kELossUniform: fELossModel = kELossUniform; break;
      case kELossUrban: fELossModel = kELossUrban; break;
      default: std::cout << "StsDigiParamaters: unknown energy loss model "
          << iTemp << std::endl; return kFALSE; break;
    } //? iTemp

    if ( ! l->fill("UseLorentzShift", &iTemp) ) return kFALSE;
    fUseLorentzShift = ( 1 == iTemp? kTRUE : kFALSE);

    if ( ! l->fill("UseDiffusion", &iTemp) ) return kFALSE;
    fUseDiffusion = ( 1 == iTemp? kTRUE : kFALSE);

    if ( ! l->fill("UseCrossTalk", &iTemp) ) return kFALSE;
    fUseCrossTalk = ( 1 == iTemp? kTRUE : kFALSE);

    if ( ! l->fill("GenerateNoise", &iTemp) ) return kFALSE;
    fGenerateNoise = ( 1 == iTemp? kTRUE : kFALSE);

    if ( ! l->fill("Vdep", &fVdep) ) return kFALSE;
    if ( ! l->fill("Vbias", &fVbias) ) return kFALSE;
    if ( ! l->fill("Temperature", &fTemperature) ) return kFALSE;
    if ( ! l->fill("Ccoup", &fCcoup) ) return kFALSE;
    if ( ! l->fill("Cis", &fCis) ) return kFALSE;
    if ( ! l->fill("DynRange", &fDynRange) ) return kFALSE;
    if ( ! l->fill("Threshold", &fThreshold) ) return kFALSE;
    if ( ! l->fill("NofAdc", &fNofAdc) ) return kFALSE;
    if ( ! l->fill("TimeResolution", &fTimeResolution) ) return kFALSE;
    if ( ! l->fill("DeadTime", &fDeadTime) ) return kFALSE;
    if ( ! l->fill("Noise", &fNoise) ) return kFALSE;
    if ( ! l->fill("ZeroNoiseRate", &fZeroNoiseRate) ) return kFALSE;
    if ( ! l->fill("StripPitch", &fStripPitch) ) return kFALSE;

    if ( ! l->fill("DiscardSecondaries", &iTemp) ) return kFALSE;
    fDiscardSecondaries = ( 1 == iTemp? kTRUE : kFALSE);

    std::string sDeadChannelMap;
    sDeadChannelMap.resize(4*128);
    l->fill("DeadChannelMap", (Text_t*)sDeadChannelMap.c_str(), 4*128);
    std::replace(sDeadChannelMap.begin(), sDeadChannelMap.end(), ',', ' ');
    std::istringstream iDeadChannelMap(sDeadChannelMap);
    fDeadChannelMap = std::set<UChar_t>(std::istream_iterator<int>(iDeadChannelMap), std::istream_iterator<int>());

    fIsInit = kTRUE;
    fIsDefault = kFALSE;
    return kTRUE;
}
// -------------------------------------------------------------------------
