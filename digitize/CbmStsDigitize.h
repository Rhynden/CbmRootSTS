/** @file CbmStsDigitize.h
 ** @author Volker Friese <v.friese@gsi.de>
 ** @date 23.05.2014
 **/

#ifndef CBMSTSDIGITIZE_H
#define CBMSTSDIGITIZE_H 1

#include <map>
#include "TStopwatch.h"

#include "CbmDigitize.h"
#include "CbmMatch.h"
#include "CbmStsDigi.h"
#include "CbmStsDigitizeParameters.h"
#include "CbmStsPhysics.h"

class TClonesArray;
class CbmStsPoint;
class CbmStsSetup;

/** @class CbmStsDigitize
 ** @brief Task class for simulating the detector response of the STS
 ** @author Volker Friese <v.friese@gsi.de>
 ** @since 23.05.2014
 ** @version 2.0
 **
 ** The STS digitiser task reads CbmStsPoint from the input and produces
 ** objects of type CbmStsDigi. The StsPoints are distributed to the
 ** respective sensors, where the analogue response is calculated. This
 ** is buffered and digitised by the connected module.
 ** The digitiser task triggers the readout of each module after the end
 ** of each call to Exec(), i.e. after processing one input MC event. All
 ** buffered data prior to the MC time of the current event are read out
 ** and stored in the output.
 **/
class CbmStsDigitize : public CbmDigitize<CbmStsDigi>
{

 public:

  /** Constructor **/
  CbmStsDigitize();


  /** Destructor **/
  virtual ~CbmStsDigitize();


  /** Create a digi and send it for further processing
   ** @param address   Unique channel address
   ** @param time      Absolute time [ns]
   ** @param adc       Digitised charge [ADC channels]
   ** @param match    MC Match object
   **/
  void CreateDigi(Int_t address, UShort_t channel, Long64_t time,
                  UShort_t adc, const CbmMatch& match);


  /** @brief Discard processing of secondary tracks
   ** @param flag  kTRUE if secondaries shall be discarded
   **
   ** This flag enables the user to suppress the digitisation of
   ** StsPoints from secondary tracks for debug purposes. By default,
   ** points from all tracks are processed.
   **/
  void DiscardSecondaries(Bool_t flag = kTRUE) {
    fDigiPar->SetDiscardSecondaries(flag);
  }


  /** @brief Detector system ID
   ** @return kSts
   **/
  Int_t GetSystemId() const { return kSts; }

   /**
    * \brief Inherited from FairTask.
    */
   virtual void SetParContainers();


  /** Execution **/
  virtual void Exec(Option_t* opt);


  /** Get energy loss model
  ** @param eLossModel       0 = ideal, 1 = uniform, 2 = fluctuations
  **/
  Int_t GetELossModel() const{return fDigiPar->GetELossModel();}


  /** Get number of signals front side **/
  Int_t GetNofSignalsF() const {return fNofSignalsF;}


  /** Get number of signals back side **/
  Int_t GetNofSignalsB() const {return fNofSignalsB;}


  /** Initialise the STS setup and the parameters **/
  void InitSetup();


  /** Re-initialisation **/
  virtual InitStatus ReInit();


  /** @brief Set individual module parameters
   ** @param parMap Map of module addresses and corresponding module parameters
   **
   ** These parameters will be applied to individual modules after the global defaults are set.
   **/
  void SetModuleParameterMap(std::map<Int_t, CbmStsDigitizeParameters*> parMap) {
    fModuleParameterMap = parMap;
  }

  /** @brief Set the global module parameters
   ** @param dynRange         Dynamic range [e]
   ** @param threshold        Threshold [e]
   ** @param nAdc             Number of ADC channels
   ** @param timeResolution   Time resolution [ns]
   ** @param deadTime         Channel dead time [ns]
   ** @param noise            Noise RMS [e]
   ** @param zeroNoiseRate    Zero-threshold noise rate [1/ns]
   ** @param fracDeadChan     Fraction of dead channels
   **
   ** These parameters will be applied to all modules when no
   ** parameter file is specified.
   **/
  void SetGlobalModuleParameters(Double_t dynRange, Double_t threshold,
                                 Int_t nAdc, Double_t timeResolution,
                                 Double_t deadTime, Double_t noise,
                                 Double_t zeroNoiseRate,
                                 Double_t fracDeadChan,
                                 std::set<UChar_t> deadChannelMap);


  /** @brief Activate noise generation
   ** @param choice If kTRUE, noise will be generated (only in stream mode).
   **
   ** By default, noise is not generated.
   ** Changing the physics flags is only allowed before Init() is called.
   **/
  void SetGenerateNoise(Bool_t choise = kTRUE);


  /** @brief Set the global sensor conditions
   ** @param vDep        Full-depletion voltage [V]
   ** @param vBias       Bias voltage [V]
   ** @param temperature Temperature [K]
   ** @param cCoupling   Coupling capacitance [pF]
   ** @param cInterstrip Inter-strip capacitance [pF]
   **
   ** These parameters will be applied to all sensors when no
   ** condition file is specified.
   **/
  void SetGlobalSensorConditions(Double_t vDep, Double_t vBias,
                                 Double_t temperature, Double_t cCoupling,
                                 Double_t cInterstrip);


  /** @brief Set the file name with module parameters
   ** @param fileName  File name with module parameters
   **
   ** The format of the file must comply with
   ** CbmStsSetup::ReadModuleParameters(const char*)
   **/
  void SetModuleParameterFile(const char* fileName);


  /** Set physics processes
   ** @param eLossModel       Energy loss model
   ** @param useLorentzShift  If kTRUE, activate Lorentz shift
   ** @param useDiffusion     If kTRUE, activate diffusion
   ** @param useCrossTalk     If kTRUE, activate cross talk
   ** @param generateNoise    If kTRUE, noise will be generated
   **
   ** Changing the physics flags is only allowed before Init() is called.
   **/
  void SetProcesses(ECbmELossModel eLossModel,
  		            Bool_t useLorentzShift = kTRUE,
  		            Bool_t useDiffusion = kTRUE,
  		            Bool_t useCrossTalk = kTRUE,
  		            Bool_t generateNoise = kFALSE);


  /** @brief Set the file name with sensor conditions
   ** @param fileName  File name with sensor conditions
   **
   ** The format of the file must comply with
   ** CbmStsSetup::ReadSensorConditions(const char*)
   **/
  void SetSensorConditionFile(const char* fileName);


  /** @brief Set the file name with sensor parameters
   ** @param fileName  File name with sensor parameters
   **
   ** The format of the file must comply with
   ** CbmStsSetup::ReadSensorParameters(const char*)
   **/
  void SetSensorParameterFile(const char* fileName);


  /** Set the sensor strip pitch
   ** @param  pitch  Strip pitch [cm]
   **
   ** The internal sensor parameters like pitch, stereo angle etc. are normally taken
   ** from a sensor database. This method allows to override the value for the strip
   ** pitch defined there, in order to easily test different sensor layout options
   ** without defining new types in the database. It has effect only for strip sensor types.
   ** The specified strip pitch will be applied for all sensors in the setup.
   **/
  void SetSensorStripPitch(Double_t pitch) {
    fDigiPar->SetStripPitch(pitch);
  }


 private:

  Bool_t fIsInitialised;   ///< kTRUE if Init() was called

  CbmStsDigitizeParameters* fDigiPar; ///< Parameters from/to DB
  CbmStsDigitizeParameters fUserPar;  ///< Parameters, user settings
  std::map<Int_t, CbmStsDigitizeParameters*> fModuleParameterMap; ///< Individual module parameter map
  CbmStsSetup*   fSetup;          ///< STS setup interface
  TClonesArray*  fPoints;         ///< Input array of CbmStsPoint
  TClonesArray*  fTracks;         ///< Input array of CbmMCTrack
  //TClonesArray*  fDigis;          ///< Output array of CbmStsDigi
  //TClonesArray*  fMatches;        ///< Output array of CbmMatch
  TStopwatch     fTimer;          ///< ROOT timer

  // --- Default sensor parameters (apply to SensorDssdStereo)
  Double_t fSensorDinact;     ///< Size of inactive border [cm]
  Double_t fSensorPitch;      ///< Strip pitch [cm]
  Double_t fSensorStereoF;    ///< Stereo angle front side [degrees]
  Double_t fSensorStereoB;    ///< Stereo angle back side [degrees]

  // --- Input parameter files
  TString fSensorParameterFile;  ///< File with sensor parameters
  TString fSensorConditionFile; ///< File with sensor conditions
  TString fModuleParameterFile;  ///< File with module parameters

  // --- Time of last processed StsPoint (for stream mode)
  Double_t fTimePointLast;

  // --- Digi times (for stream mode, in each step)
  Double_t fTimeDigiFirst;      ///< Time of first digi sent to DAQ
  Double_t fTimeDigiLast;       ///< Time of last digi sent to DAQ

  // --- Event counters
  Int_t fNofPoints;    ///< Number of points processed in Exec
  Int_t fNofSignalsF;  ///< Number of signals on front side
  Int_t fNofSignalsB;  ///< Number of signals on back side
  Int_t fNofDigis;     ///< Number of created digis in Exec

  // --- Run counters
  Int_t    fNofEvents;      ///< Total number of events processed
  Double_t fNofPointsTot;   ///< Total number of points processed
  Double_t fNofSignalsFTot; ///< Number of signals on front side
  Double_t fNofSignalsBTot; ///< Number of signals on back side
  Double_t fNofDigisTot;    ///< Total number of digis created
  Double_t fNofNoiseTot;    ///< Total number of noise digis
  Double_t fTimeTot;        ///< Total execution time


  /** @brief Number of signals in the analogue buffers
   ** @value nSignals  Sum of number of signals in all modules
   **/
  Int_t BufferSize() const;


  /** @brief Status of the analogue buffers
   ** @param[out] nSignals  Sum of number of signals in alll modules
   ** @value String output
   **/
  std::string BufferStatus() const;


  /** End-of-run action **/
  virtual void Finish();


  /** Get event information
   ** @param[out]  eventNumber  Number of MC event
   ** @param[out]  inputNumber  Number of input
   ** @param[out]  eventTime    Start time of event [ns]
   **
   ** In case of being run with FairRunAna, input number and time
   ** are taken from FairEventHeader. If the task is run with
   ** FairRunSim, the FairEventHeader is not filled, so input number
   ** and event time are zero.
   ** The event number is always taken from the FairRootManager, because the
   ** event number from FaurEventHeader::GetMCEntryNumber() is
   ** generator-dependent.
   **/
  //void GetEventInfo(Int_t& inputNr, Int_t& eventNr, Double_t& eventTime);
  // TODO: Is now in base class CbmDigitize. Can be removed here after validation.


  /** Initialisation **/
  virtual InitStatus Init();


  /** Process the analog buffers of all modules
   ** @param readoutTime  Time of readout [ns]
   **/
  void ProcessAnalogBuffers(Double_t readoutTime);


  /** Process StsPoints from MCEvent **/
  void ProcessMCEvent();


  /** Process one MCPoint
   ** @param point  Pointer to CbmStsPoint to be processed
   ** @param link   Link to MCPoint
   **/
  void ProcessPoint(const CbmStsPoint* point, Double_t eventTime = 0.,
  		              CbmLink* link = NULL);


  /** @brief Reset event counters **/
  void ResetCounters();


  /** Prevent usage of copy constructor and assignment operator **/
  CbmStsDigitize(const CbmStsDigitize&);
  CbmStsDigitize operator=(const CbmStsDigitize&);



  ClassDef(CbmStsDigitize, 5);

};

#endif


