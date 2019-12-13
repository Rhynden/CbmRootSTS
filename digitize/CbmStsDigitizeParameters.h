/** @file CbmStsDigitizeParameters.h
 ** @author Florian Uhlig <f.uhlig@gsi.de>
 ** @date 12.06.2017
 **/

#ifndef CBMSTSDIGITIZEPARAMETERS_H
#define CBMSTSDIGITIZEPARAMETERS_H 1

#include <iostream>
#include <string>
#include <set>
#include "TF1.h"

#include "FairParGenericSet.h"
#include "CbmStsPhysics.h"

class FairParamList;

/** @class CbmStsDigitizeParameters
 ** @brief Parameters for STS digitization
 ** @author Florian Uhlig <f.uhlig@gsi.de>
 ** @since 31.05.2017
 ** @version 1.0
 **
 ** This class collects all parameters relevant for STS digitization
 ** such that they can be made persistent for use e.g. in reconstruction.
 ** The parameters are stored via the FairRuntimeDb
 **/
class CbmStsDigitizeParameters : public FairParGenericSet
{

  public:


    /** Constructor **/
    CbmStsDigitizeParameters(
                const char* name    = "CbmStsDigitizeParameters",
                const char* title   = "STS parameters",
                const char* context = "Default");


    /** Destructor **/
    virtual ~CbmStsDigitizeParameters() {
      if (fNoiseCharge) delete fNoiseCharge;
    };

    /** Reset all parameters **/
    virtual void clear();

    void putParams(FairParamList*);
    Bool_t getParams(FairParamList*);

    /** Accessors **/
    Double_t GetCcoup() const { return fCcoup; }
    Double_t GetCis() const { return fCis; }
    Double_t GetDeadChannelFrac() const { return fDeadChannelFrac; }
    std::set<UChar_t> GetDeadChannelMap() const { return fDeadChannelMap; }
    Double_t GetDeadTime() const { return fDeadTime; }
    Bool_t   GetDiscardSecondaries() const { return fDiscardSecondaries; }
    Double_t GetDynRange() const { return fDynRange; }
    ECbmELossModel GetELossModel() const { return fELossModel; }
    Bool_t   GetGenerateNoise() const { return fGenerateNoise; }
    Int_t    GetNofAdc() const { return fNofAdc; }
    Int_t    GetNoise() const { return fNoise; }
    Double_t GetStripPitch() const { return fStripPitch; }
    Double_t GetTemperature() const { return fTemperature; }
    Double_t GetThreshold() const { return fThreshold; }
    Double_t GetTimeResolution() const { return fTimeResolution; }
    Bool_t   GetUseCrossTalk() const { return fUseCrossTalk; }
    Bool_t   GetUseDiffusion() const { return fUseDiffusion; }
    Bool_t   GetUseLorentzShift() const { return fUseLorentzShift; }
    Double_t GetVbias() const { return fVbias; }
    Double_t GetVdep() const { return fVdep; }
    Double_t GetZeroNoiseRate() const { return fZeroNoiseRate; }

    Double_t GetNoiseRate() {
      if (fNoiseRate == 0)
        fNoiseRate = 0.5 * fZeroNoiseRate
          * TMath::Exp( -0.5 * fThreshold * fThreshold / (fNoise * fNoise) );

      return fNoiseRate;
    }

    TF1* GetNoiseCharge() {
      if (!fNoiseCharge) {
        fNoiseCharge = new TF1("Noise Charge", "TMath::Gaus(x, [0], [1])",
                         fThreshold, 10. * fNoise);
        fNoiseCharge->SetParameters(0., fNoise);
      }

      return fNoiseCharge;
    }

    /** @brief Parameters default status
     ** @return kTRUE is values were taken from default
     **/
    Bool_t IsDefault() const { return fIsDefault; }


    /** @brief Initialisation status
     ** @return kTRUE is values were initialised
     **/
    Bool_t IsInit() const { return fIsInit; }


    /** @brief Set defaults for all values
     **
     ** Default values are hard-coded here.
     **/
    void SetDefaults();


    /** @brief Flag whether secondary tracks are discarded during digitisation
     ** @param if kTRUE, points from secondary tracks are not digitised.
     **/
    void SetDiscardSecondaries(Bool_t choice = kTRUE) {
      fDiscardSecondaries = choice;
      setChanged();
      setInputVersion(-2,1);
    }


    /** @brief Switch noise generation on/off (is deactivated by default).
     ** @param If kTRUE, noise will be generated in stream mode.
     **/
    void SetGenerateNoise(Bool_t choice = kTRUE) { 
      fGenerateNoise = choice;
      setChanged();
      setInputVersion(-2,1);
    }


    /** @brief Set digital      response parameters
     ** @param nChannels        Number of readout channels
     ** @param dynRange         Dynamic range [e]
     ** @param threshold        Threshold [e]
     ** @param nAdc             Number of ADC channels
     ** @param timeResol        Time resolution [ns]
     ** @param deadTime         Channel dead time [ns]
     ** @param noise            Noise RMS
     ** @param deadChannelFrac  Fraction of dead channels [%]
     ** @param deadChannelMap   Map of dead channels
     **/
    void SetModuleParameters(Double_t dynRange, Double_t threshold,
                             Int_t nAdc, Double_t timeResol,
                             Double_t deadTime, Double_t noise,
                             Double_t zeroNoiseRate,
                             Double_t deadChannelFrac,
                             std::set<UChar_t> deadChannelMap = {});


    /** @brief Switch analogue response processes on or off
     ** @param eLossModel Energy loss model (0=ideal, 1=uniform, 2=fluctuations)
     ** @param useLorentzShift  Lorentz shift on/off
     ** @param useDiffusion     Diffusion on/off
     ** @param useCrossTalk     Cross-talk on/off
     **/
    void SetProcesses(ECbmELossModel eLossModel, Bool_t useLorentzShift,
                      Bool_t useDiffusion, Bool_t useCrossTalk,
                      Bool_t generateNoise = kFALSE) {
      fELossModel = eLossModel;
      fUseLorentzShift = useLorentzShift;
      fUseDiffusion = useDiffusion;
      fUseCrossTalk = useCrossTalk;
      fGenerateNoise = generateNoise;
      setChanged();
      setInputVersion(-2,1);
    }


    /** brief Set sensor properties
     ** @param vDep   Depletion voltage [V]
     ** @param vBias  Bias voltage [V]
     ** @param temp   Temperature [K]
     ** @param cCoup  Coupling capacitance [pF]
     ** @param cIs    Inter-strip capacitance [pF]
     **/
    void SetSensorConditions(Double_t vDep, Double_t vBias, Double_t temp,
                             Double_t cCoup, Double_t cIs) {
      fVdep = vDep;
      fVbias = vBias;
      fTemperature = temp;
      fCcoup = cCoup;
      fCis = cIs;
      setChanged();
      setInputVersion(-2,1);
    }


    /** @brief Override the strip pitch taken from the sensor database.
     ** @value pitch  Strip pitch [cm]
     **
     ** This value will be applied for all sensors in the setup.
     **/
    void SetStripPitch(Double_t pitch) {
      fStripPitch = pitch;
      setChanged();
      setInputVersion(-2,1);
    }


    /** String output **/
    virtual std::string ToString() const;


  private:

    // --- Physics processes
    ECbmELossModel  fELossModel{kELossUrban}; ///< Energy loss model
    Bool_t fUseLorentzShift{kTRUE};     ///< Lorentz shift on/off
    Bool_t fUseDiffusion{kTRUE};        ///< Thermal diffusion on/off
    Bool_t fUseCrossTalk{kTRUE};        ///< Cross-talk on/off
    Bool_t fGenerateNoise{kTRUE};       ///< Noise on/off

    // --- Sensor conditions (analogue response)
    Double_t fVdep{0.};                 ///< Depletion voltage [V]
    Double_t fVbias{0.};                ///< Bias voltage [V]
    Double_t fTemperature{0.};          ///< Temperature [K]
    Double_t fCcoup{0.};                ///< Coupling capacitance [pF]
    Double_t fCis{0.};                  ///< Inter-strip capacitance [pF]

    // --- Read-out ASIC properties (digital response)
    Double_t fDynRange{0.};             ///< Dynamic range [e]
    Double_t fThreshold{0.};            ///< Threshold [e]
    Int_t    fNofAdc{0};                ///< Number of ADC channels
    Double_t fTimeResolution{0.};       ///< Time resolution [ns]
    Double_t fDeadTime{0.};             ///< Channel dead time [ns]
    Double_t fNoise{0.};                ///< RMS of noise [e]
    Double_t fZeroNoiseRate{0.};        ///< Zero noise rate [1/ns]
    Double_t fDeadChannelFrac{0.};      ///< Fraction of dead channels [%]
    std::set<UChar_t> fDeadChannelMap{};  ///< Map of dead channels

    // --- Strip pitch. If not -1, this value overrides the strip pitch
    // --- defined in the sensor database. It will then be the same for
    // --- all sensors.
    Double_t fStripPitch{0.};           ///< Strip pitch in sensors

    // --- If this variable is set to kTRUE, points from secondary
    // --- tracks will be discarded. For debug purposes.
    Bool_t fDiscardSecondaries{kFALSE}; ///< If kTRUE, discard points from secondary tracks

    // --- Status flags
    Bool_t fIsInit{kFALSE};             // Is initialised
    Bool_t fIsDefault{kFALSE};          // Is default

    Double_t fNoiseRate{0.};            ///! Noise rate, calculated from other parameters
    TF1* fNoiseCharge{nullptr};         ///! Noise distribution

    ClassDef(CbmStsDigitizeParameters, 4);
};

#endif /* CBMSTSDIGITIZEPARAMETERS_H */
