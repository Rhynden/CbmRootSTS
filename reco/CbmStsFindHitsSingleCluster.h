/** @file CbmStsFindHitsSingleCluster.h
 ** @author Volker Friese <v.friese@gsi.de>
 ** @date 20.02.2019
 **/


#ifndef CBMSTSFINDHITSSINGLECLUSTER_H
#define CBMSTSFINDHITSSINGLECLUSTER_H 1

#include <set>
#include "TStopwatch.h"
#include "FairTask.h"
#include "CbmStsModule.h"

class TClonesArray;
class CbmStsSetup;

/** @class CbmStsFindHitsSingleCluster
 ** @brief Task class for finding STS hits
 ** @author Volker Friese <v.friese@gsi.de>
 ** @since 20.02.2019
 ** @version 1.0
 **
 ** This task constructs hits (3-d points) from single clusters.
 ** The hit coordinate along the strip direction is defined as the
 ** centre of the strip / sensor.
 ** The task can be used for STS sensors of which only one side is
 ** read out (as for some in-beam test configuration), such that
 ** the normal hit finder, combining clusters from the front
 ** and from the back side, will produce no hits at all.
 **/
class CbmStsFindHitsSingleCluster : public FairTask
{

	public:

    /** @brief Constructor **/
    CbmStsFindHitsSingleCluster();


    /** @brief Destructor  **/
    virtual ~CbmStsFindHitsSingleCluster();


    /** @brief Task execution **/
    virtual void Exec(Option_t* opt);


    /** @brief End-of-run action. **/
    virtual void Finish();


    /** @brief End-of-event action **/
    virtual void FinishEvent();


    /** @brief Initialisation **/
    virtual InitStatus Init();


	private:

    TClonesArray* fClusters;          ///< Input array of CbmStsCluster
    TClonesArray* fHits;              ///< Output array of CbmStsHits
    CbmStsSetup*  fSetup;             ///< Instance of STS setup
    TStopwatch    fTimer;             ///< ROOT timer

    // --- Run counters
    Int_t    fNofTimeSlices;   ///< Total number of time-slices processed
    Double_t fNofClustersTot;  ///< Total number of clusters processed
    Double_t fNofHitsTot;      ///< Total number of hits produced
    Double_t fTimeTot;         ///< Total execution time

    /** Set of active modules in the current event **/
    std::set<CbmStsModule*> fActiveModules;

    /** @brief Sort clusters into modules
     ** @value Number of clusters sorted
     **/
    Int_t SortClusters();


    /** Prevent usage of copy constructor and assignment operator **/
    CbmStsFindHitsSingleCluster(const CbmStsFindHitsSingleCluster&);
    CbmStsFindHitsSingleCluster operator=(const CbmStsFindHitsSingleCluster&);


    ClassDef(CbmStsFindHitsSingleCluster, 1);
};

#endif
