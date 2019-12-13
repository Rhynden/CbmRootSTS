/** @file CbmStsFindHits.h
 ** @author Volker Friese <v.friese@gsi.de>
 ** @since 17.06.2014
 **/


#ifndef CBMSTSFINDHITS_H
#define CBMSTSFINDHITS_H 1

#include <set>
#include "TStopwatch.h"
#include "FairTask.h"
#include "CbmStsModule.h"
#include "CbmStsReco.h"

class TClonesArray;
class CbmEvent;
class CbmStsSetup;

/** @class CbmStsFindHits
 ** @brief Task class for finding STS hits
 ** @author Volker Friese <v.friese@gsi.de>
 ** @since 17.06.2014
 ** @date 22.03.2019
 **
 ** This task constructs hits (3-d points) from clusters. In each module,
 ** the intersection points from each pair of front and back side
 ** clusters are calculated and stored as hit.
 **/
class CbmStsFindHits : public FairTask
{

  public:

    /** @brief Constructor
     ** @param mode  Time-slice or event mode
     **/
    CbmStsFindHits(ECbmMode mode = kCbmTimeslice);


    /** @brief Copy constructor (forbidden) **/
    CbmStsFindHits(const CbmStsFindHits&) = delete;


    /** @brief Assignment operator (forbidden) **/
    CbmStsFindHits operator=(const CbmStsFindHits&) = delete;


    /** @brief Destructor  **/
    virtual ~CbmStsFindHits();


    /** @brief Task execution **/
    virtual void Exec(Option_t* opt);


    /** @brief End-of-run action **/
    virtual void Finish();


    /** @brief Initialisation **/
    virtual InitStatus Init();


    /** @brief Set operation mode
     ** @param mode  Time-slice or event mode
     **
     ** In the (default) time-slice mode, the entire input array of clusters
     ** will be processed. In the event mode, event objects are read
     ** and processed one after the other.
     **/
    void SetMode(ECbmMode mode) { fMode = mode; }


    /** @brief Set a fixed absolute value for the time difference between two clusters
     ** @param value  Maximal time difference between two clusters in a hit [ns]
     **
     ** Two clusters (front and back side) make a hit if they cross geometrically
     ** and their time difference is smaller than deltaT.
     ** By default, deltaT is calculated from the cluster time error. This method
     ** allows to override this definition for all modules by a user-defined value.
     **/
    void SetTimeCutInNs(Double_t value) { fTimeCutInNs = value; }


    /** @brief Set a maximal time difference of two digis in terms of multiples
     *  of its error.
     ** @param value  Maximal time difference
     **
     ** Two digis are considered belonging to a cluster if they are in
     ** neighbouring channels and their time difference is smaller than deltaT.
     ** By default, deltaT is calculated from the module parameters (time
     ** resolution) as deltaT = deltaTSigma * sqrt(2) * tresol.
     **/
    void SetTimeCutInSigma(Double_t value) { fTimeCutInSigma = value; }


  private:

    TClonesArray* fEvents;        ///< Input array of CbmEvent
    TClonesArray* fClusters;      ///< Input array of CbmStsCluster
    TClonesArray* fHits;          ///< Output array of CbmStsHits
    CbmStsSetup*  fSetup;         ///< Instance of STS setup
    TStopwatch    fTimer;         ///< ROOT timer
    ECbmMode fMode;               ///< Mode (time-slice or event)
    Double_t fTimeCutInSigma;     ///< Max. cluster timer difference in sigma
    Double_t fTimeCutInNs;        ///< Max. cluster timer difference in ns


    // --- Run counters
    Int_t fNofTimeslices;      ///< Number of processed time-slices
    Int_t fNofEvents;          ///< Number of processed events
    Double_t fNofClusters;     ///< Total number of clusters processed
    Double_t fNofHits;         ///< Total number of hits produced
    Double_t fTimeTot;         ///< Total execution time

    /** Set of active modules in the current event **/
    std::set<CbmStsModule*> fActiveModules;


    /** @brief Process an event or time-slice
     ** @param event  Pointer to event object. If null, process entire
     ** time-slice.
     ** @return Number of created hits
     **/
    Int_t ProcessData(CbmEvent* event);


    /** @brief Sort clusters into modules
     ** @param event  Pointer to event object. If null, use entire
     ** time-slice.
     ** @return Number of clusters sorted
     **/
    Int_t SortClusters(CbmEvent* event);


    ClassDef(CbmStsFindHits, 2);
};

#endif
