/** @file CbmReco.h
 ** @author Volker Friese <v.friese@gsi.de>
 ** @date 11.03.2019
 **/


#ifndef CBMSTSRECO_H
#define CBMSTSRECO_H 1

#include "FairTask.h"
#include "CbmStsDigitizeParameters.h"

class CbmStsSetup;


/** @enum ECbmMode
 ** @brief Time-slice or event-by-event mode
 **/
enum ECbmMode{kCbmTimeslice, kCbmEvent};


/** @class CbmStsReco
 ** @brief Task class for local reconstruction in STS
 ** @author Volker Friese <v.friese@gsi.de>
 ** @since 11.03.2019
 ** @date 11.03.2019
 **
 ** Local reconstruction in the CBM-STS. On presence of the
 ** respective input data, the cluster and hit finder tasks
 ** will be instantiated an registered.
 **/
class CbmStsReco : public FairTask
{

  public:

    /** @brief Constructor **/
    CbmStsReco();


    /** @brief Destructor  **/
    virtual ~CbmStsReco();


    /** @brief Initialisation
     ** @return Success of initialisation
     **
     ** Inherited from FairTask.
     **/
    virtual InitStatus Init();


    /** @brief Change the global module parameters
     ** @param nChannels        Number of readout channels
     ** @param dynRange         Dynamic range [e]
     ** @param threshold        Threshold [e]
     ** @param nAdc             Number of ADC channels
     ** @param timeResol        Time resolution [ns]
     ** @param deadTime         Channel dead time [ns]
     ** @param noise            Noise RMS
     ** @param deadChannelFrac  Fraction of dead channels [%]
     **
     ** These parameters replace the default ones defined in
     ** DefineDefaultParameters. They will be used if a
     ** parameter container is not available.
     */
    void SetGlobalModuleParameters(Double_t dynRange, Double_t threshold,
                                   Int_t nAdc, Double_t timeResol,
                                   Double_t deadTime, Double_t noise,
                                   Double_t zeroNoiseRate,
                                   Double_t deadChannelFrac) {
      fGlobalPar.SetModuleParameters(dynRange, threshold, nAdc, timeResol,
                                     deadTime, noise, zeroNoiseRate,
                                     deadChannelFrac);
      fIsModuleParametersDefault = kFALSE;
    }


    /** @brief Change the global sensor conditions
     ** @param vDep   Full depletion voltage [V]
     ** @param vBias  Bias voltage [V]
     ** @param temp   Temperature [K]
     ** @param cCoup  Coupling capacitance [pF]
     ** @param cInter Inter-strip capacitance [pF]
     **
     ** These parameters replace the default ones defined in
     ** DefineDefaultParameters. They will be used if a
     ** parameter container is not available.
     */
    void SetGlobalSensorConditions(Double_t vDep, Double_t vBias,
                                   Double_t temp, Double_t cCoup,
                                   Double_t cInter) {
      fGlobalPar.SetSensorConditions(vDep, vBias, temp, cCoup, cInter);
      fIsSensorConditionsDefault = kFALSE;
    }

    /** @brief Set the maximal time difference of two clusters in a hit
     ** in terms of multiples of its error.
     ** @param value  Maximal time difference (multiple of its error)
     **
     ** Two clusters can make a hit if their time difference is below a cut
     ** value. If no absolute cut value is defined by the method
     ** SetTimeCutClustersInNs, the cut is defined as the argument to this
     ** method (default is 4) times the error of the time difference,
     ** which is calculated from the cluster time errors.
     **
     ** If the argument is set to a negative value and no absolute time cut
     ** is defined, no time cut will be applied. This is only advisable
     ** for event-by-event reconstruction, not for a full time slice.
     **/
    void SetTimeCutClustersInSigma(Double_t value) {
    	fTimeCutClustersInSigma = value;
    }


    /** @brief Set the maximal time difference of two clusters in a hit
     ** @param value  Maximal time difference
     **
     ** Two clusters can make a hit if their time difference is below a cut
     ** value. This method allows to define an absolute value for this time
     ** cut.
     **
     ** If the argument is not positive, the time cut will be defined as
     ** a multiple of the error of the time difference as calculated from
     ** the cluster time errors. This is the default behaviour. The multiple
     ** is by default 4 and can be changed by the method
     ** SetTimeCutClustersInSigma.
     **/
    void SetTimeCutClustersInNs(Double_t value) {
    	fTimeCutClustersInNs = value;
    }


    /** @brief Set the maximal time difference of two digis in a cluster
     *  in terms of multiples of its error.
     ** @param value  Maximal time difference
     **
     ** Two digis are considered belonging to a cluster if they are in
     ** neighbouring channels and their time difference is smaller than MaxDeltaT.
     ** MaxDeltaT is calculated from the module parameters (time resolution)
     ** as deltaT = deltaTSigma * sqrt(2) * tresol.
     **
     ** Default is 3.
     **/
    void SetTimeCutDigisInSigma(Double_t value) {
    	fTimeCutDigisInSigma = value;
    }


    /** @brief Set the maximal time difference of two digis in a cluster
     ** @param value  Maximal time difference
     **
     ** Two digis are considered belonging to a cluster if they are in
     ** neighbouring channels and their time difference is smaller than MaxDeltaT.
     ** By default, MaxDeltaT is calculated from the module parameters
     ** (time resolution) as deltaT = deltaTSigma * sqrt(2) * tresol.
     ** The user can override this with this method by specifying an absolute
     ** value in ns.
     **/
    void SetTimeCutDigisInNs(Double_t value) {
    	fTimeCutDigisInNs = value;
    }


    /** @brief Set the path for a sensor parameter file
     ** @param value  Path to the file
     **
     ** The format of the file must comply with
     ** CbmStsSetup::ReadSensorParameters(const char*)
     **/
    void SetSensorsParFile(const char* value) {
    	fSensorsParameterFile = value;
    }


    /** @brief Set processing mode
     ** @param mode  Processing mode (time slice or event)
     **
     ** Default is time-slice processing. In case event mode
     ** is selected, an event branch has to be present.
     **/
    void SetMode(ECbmMode mode) { fMode = mode; }


    /** @brief Set parameter containers
     **
     ** Specify the needed parameter containers (CbmStsDigitizeParameters).
     ** Inherited from FairTask.
     **/
    virtual void SetParContainers();


    /** @brief Use single-cluster hit finder
     ** @param choice If true, single-cluster hit finder will be used.
     **
     ** By default, the normal hit finder, constructing hits from a combination
     ** of a front and a back side cluster, is used. This option, if activated,
     ** will produce a hit from each single cluster.
     **/
    void UseSingleClusters(Bool_t choice = kTRUE) { fUseSingleClusters = choice; }


  private:

    Int_t fStsRecoMode;                   ///< Switch between Cluster/HitFinder including time-sorting, DigisToHits with or without and cluster output
    ECbmMode fMode;                       ///< time-slice or event
    Bool_t fUseSingleClusters;            ///< Construct hits from single clusters
    CbmStsSetup* fSetup;                  //! Setup instance
    CbmStsDigitizeParameters* fDigiPar;   ///< Parameters
    CbmStsDigitizeParameters fGlobalPar;  ///< User defined global defaults
    Bool_t   fIsSensorConditionsDefault;  ///< Flag if parameters were changed
    Bool_t   fIsModuleParametersDefault;  ///< Flag if parameters were changed
    Double_t fTimeCutDigisInSigma;        ///< Time cut for digis in cluster in sigma
    Double_t fTimeCutDigisInNs;               ///< Absolute time cut for digis in cluster [ns]
    Double_t fTimeCutClustersInSigma;     ///< Time cut for clusters in hit in sigma
    Double_t fTimeCutClustersInNs   ;     ///< Time cut for clusters in hit in ns
    const char*  fSensorsParameterFile;   ///< Optional path for file with non default sensor settings
    Int_t StsRecoMode;                    ///< Switch between Cluster/HitFinder,time-sorting, DigisToHits and cluster output

    /** @brief Copy constructor forbidden **/
    CbmStsReco(const CbmStsReco&) = delete;


    /** @brief Assignment operator forbidden **/
    CbmStsReco operator=(const CbmStsReco&) = delete;


    /** @brief Set default parameters for sensors and modules
     **
     ** The default values are hard-coded here. Used from the constructor.
     **/
    void DefineDefaultParameters();


    /** @brief Set global sensor conditions (same for all sensors)
     ** @param digiPar  Parameter container
     **
     ** If the parameter container is not present, default values
     ** will be used.
     **/
    void SetSensorConditions(CbmStsDigitizeParameters* par = nullptr);


    ClassDef(CbmStsReco, 1);
};

#endif
