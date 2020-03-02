/** @file CbmStsDigisToHits.h
 **/


#ifndef CbmStsDigisToHits_H
#define CbmStsDigisToHits_H 1

#include "TStopwatch.h"
#include "FairTask.h"
#include "CbmStsHit.h"
#include "TClonesArray.h"
#include "CbmStsReco.h"

class TClonesArray;
class CbmDigiManager;
class CbmEvent;
class CbmStsClusterAnalysis;
class CbmStsDigisToHitsModule;
class CbmStsDigitizeParameters;
class CbmStsSetup;


/** @class CbmStsDigisToHits
 ** @brief Task class for finding STS clusters
 ** @author Volker Friese <v.friese@gsi.de>
 ** @since 16.06.2014
 ** @date 22.03.2019
 **
 ** This task groups StsDigis into clusters. Digis are first sorted w.r.t.
 ** the module they are registered by; the cluster finding is then performed
 ** in each module.
 **
 ** The task can operate both on time-slice and event input.
 ** Use SetEventMode() to choose event-by-event operation.
 **
 ** The actual cluster finding algorithm is defined in the class
 ** CbmStsDigisToHitsModule.
 **/
class CbmStsDigisToHits : public FairTask
{

  public:

    /** @brief Constructor **/
    CbmStsDigisToHits(ECbmMode mode = kCbmTimeslice, Bool_t ClusterOutputMode = kFALSE, Bool_t Parallelism_enabled = kFALSE);


    /** @brief Destructor  **/
    virtual ~CbmStsDigisToHits();


    /** @brief Task execution **/
    virtual void Exec(Option_t* opt);


    /** @brief End-of-run action **/
    virtual void Finish();


    /** @brief Access to output array of clusters **/
    TClonesArray* GetClusters() { return fClusters; }


    /** @brief Initialisation **/
    virtual InitStatus Init();


        /** @brief Set a fixed absolute value for the time difference between two digis
     ** @param value  Maximal time difference between two digis in a cluster [ns]
     **
     ** Two digis are considered belonging to a cluster if they are in
     ** neighbouring channels and their time difference is smaller than deltaT.
     ** By default, deltaT is calculated from the module parameters (time
     ** resolution). This method allows to override this definition for all
     ** modules by a user-defined value.
     **/
    void SetTimeCutDigisInNs(Double_t value) { fTimeCutDigisInNs = value; }

    
    /** @brief Set a fixed absolute value for the time difference between two clusters
     ** @param value  Maximal time difference between two clusters in a hit [ns]
     **
     ** Two clusters (front and back side) make a hit if they cross geometrically
     ** and their time difference is smaller than deltaT.
     ** By default, deltaT is calculated from the cluster time error. This method
     ** allows to override this definition for all modules by a user-defined value.
     **/
    void SetTimeCutClustersInNs(Double_t value) { fTimeCutClustersInNs = value; }


    /** @brief Set a maximal time difference of two digis in terms of multiples
     *  of its error.
     ** @param value  Maximal time difference
     **
     ** Two digis are considered belonging to a cluster if they are in
     ** neighbouring channels and their time difference is smaller than deltaT.
     ** By default, deltaT is calculated from the module parameters (time
     ** resolution) as deltaT = deltaTSigma * sqrt(2) * tresol.
     **/
    void SetTimeCutDigisInSigma(Double_t value) { fTimeCutDigisInSigma = value; }

    
    /** @brief Set a maximal time difference of two digis in terms of multiples
     *  of its error.
     ** @param value  Maximal time difference
     **
     ** Two digis are considered belonging to a cluster if they are in
     ** neighbouring channels and their time difference is smaller than deltaT.
     ** By default, deltaT is calculated from the module parameters (time
     ** resolution) as deltaT = deltaTSigma * sqrt(2) * tresol.
     **/
    void SetTimeCutClustersInSigma(Double_t value) { fTimeCutClustersInSigma = value; }


    /** @brief Set event-by-event mode
     ** @param choice  If true, event-by-event mode is used
     **
     ** In the event-by-event mode, the event objects in the input tree
     ** are used, and events are processed one after the other.
     ** An event builder has to be run before, creating the event objects.
     ** By default, time-slice mode is applied.
     **/
    void SetEventMode(Bool_t choice = kTRUE) {
      fMode = ( choice ? kCbmEvent : kCbmTimeslice );
    }


    /** @brief Set execution mode
     ** @param mode  Time-slice or event
     **
     ** In the time-slice mode, the entire time-slice (input arrays)
     ** will be processed. In the event mode, events read from the event
     ** branch are processed one after the other.
     **/
    void SetMode(ECbmMode mode) { fMode = mode; }


    /** @brief Define the needed parameter containers **/
    virtual void SetParContainers();


  private:

    TClonesArray* fEvents;            //! Input array of events
    CbmDigiManager* fDigiManager;     //! Interface to digi branch
    TClonesArray* fClusters;          //! Output array of CbmStsCluster
    CbmStsSetup*  fSetup;             //! Instance of STS setup
    CbmStsDigitizeParameters* fDigiPar; //! digi parameters
    CbmStsClusterAnalysis* fAna;      //! Instance of Cluster Analysis tool
    TStopwatch    fTimer;             //! ROOT timer
    ECbmMode fMode;                   ///< Time-slice or event
    Double_t fTimeCutDigisInSigma;         ///< Multiple of error of time difference
    Double_t fTimeCutClustersInSigma;   
    Double_t fTimeCutDigisInNs;                ///< User-set maximum time difference
    Double_t fTimeCutClustersInNs;    

    //DigisToHits
    std::vector<CbmStsDigisToHitsModule*> fModuleIndex;
    Bool_t fClusterOutputMode;
    TClonesArray* fHits;
    std::vector<CbmStsHit>* fHitsVector; //!
    Bool_t fParallelism_enabled;

    // Convert a vector of CbmStsHits to a TClonesArray of those hits
    // Needed for correctness evaluation
    TClonesArray* Convert(std::vector<CbmStsHit> arr)
    {
      TClonesArray* tca;
      tca = new TClonesArray("CbmStsHit", 6e3);
      Int_t entries = arr.size();
      if (entries > 0) {
        for(int i=0; i< entries; ++i) {
          CbmStsHit hit = static_cast<CbmStsHit>(arr[i]);
          //tca->AddAt(&hit, i);
          CbmStsHit* newHit = new ( (*tca)[i] ) CbmStsHit();
          *newHit = hit;
        }
      }
      return tca;
    }


    /** @brief Sort clusters into modules
     ** @param event  Pointer to event object. If null, use entire
     ** time-slice.
     ** @return Number of clusters sorted
     **/
    Int_t SortClusters(CbmEvent* event);

    // --- Run counters CbmStsFindHits
    Double_t fNofHits;         ///< Total number of hits produced


    // --- Counters
    Int_t     fNofTimeslices;   ///< Number of time slices processed
    Int_t     fNofEvents;       ///< Number of events processed
    Double_t  fNofDigis;        ///< Total number of digis processed
    Double_t  fNofDigisUsed;    ///< Total number of used digis
    Double_t  fNofDigisIgnored; ///< Total number of ignored digis
    Double_t  fNofClusters;     ///< Total number of clusters produced
    Double_t  fTimeTot;         ///< Total execution time

    // --- Map from module address to cluster finding module
    std::map<Int_t, CbmStsDigisToHitsModule*> fModules;  //!


    /** @brief Instantiate cluster finding modules
     ** @value Number of modules created
     **/
    Int_t CreateModules();


    /** @brief Initialise the digitisation settings
     **
     ** This method read the digi settings object from file,
     ** sets it to the setup and updates the module parameters.
     */
    void InitSettings();


    /** @brief Process one time slice or event
     ** @param event  Pointer to CbmEvent object
     **
     ** If a NULL event pointer is given, the entire input branch is processed.
     **/
    void ProcessData(CbmEvent* event = NULL);


    /** @brief Process one STS digi
     ** @param index  Index of STS digi in its TClonesArray
     **/
    Bool_t ProcessDigi(Int_t index);


    /** @brief Copy constructor (forbidden) **/
    CbmStsDigisToHits(const CbmStsDigisToHits&) = delete;


    /** @brief Assignment operator (forbidden) **/
    CbmStsDigisToHits operator=(const CbmStsDigisToHits&) = delete;


    ClassDef(CbmStsDigisToHits, 2);
};

#endif

