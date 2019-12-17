// -------------------------------------------------------------------------
// -----                  CbmStsFindTracksQa header file               -----
// -----                  Created 11/01/06  by V. Friese               -----
// -------------------------------------------------------------------------


/** CbmStsFindTracksQa.h
 *@author V.Friese <v.friese@gsi.de>
 **
 ** Quality check task for CbmStsFindTracks
 **/


#ifndef CBMSTSTRACKFINDERQA_H
#define CBMSTSTRACKFINDERQA_H 1


//#include <map>
#include "TStopwatch.h"
#include "TVector3.h"
#include "FairTask.h"

class TH1F;
class CbmGeoPassivePar;
class CbmEvent;
class CbmMCDataArray;
class CbmStsSetup;



class CbmStsFindTracksQa : public FairTask
{

 public:

  /** Default constructor **/
  CbmStsFindTracksQa(Int_t iVerbose = 1);


  /** Standard constructor 
  *@param minHits     Minimal number of StsHits for considered MCTracks
  *@param quota       True/all hits for track to be considered reconstructed
  *@param iVerbose    Verbosity level
  **/
  CbmStsFindTracksQa(Int_t minHits, Double_t quota, Int_t iVerbose);


  /** Destructor **/
  virtual ~CbmStsFindTracksQa();


  /** Set parameter containers **/
  virtual void SetParContainers();


  /** Initialisation **/
  virtual InitStatus Init();


  /** Reinitialisation **/
  virtual InitStatus ReInit();


  /** Execution **/
  virtual void Exec(Option_t* opt);



 private:

  /** Finish **/
  virtual void Finish();

  /** Read the geometry parameters **/
  InitStatus GetGeometry();


  /** Get the target node from the geometry **/
  void GetTargetPosition();


  /** Create histograms **/
  void CreateHistos();


  /** Reset histograms and counters **/
  void Reset();


  /** Fill a map from MCTrack index to number of corresponding StsHits **/
  void FillHitMap(CbmEvent* event);


  /** Fill a map from MCTrack index to matched StsTrack index
   *@param nRec  Number of reconstructed tracks (return)
   *@param nGhosts  Number of ghost tracks (return)
   *@param nClones  Number of clone tracks (return)
   **/
  void FillMatchMap(CbmEvent* event, Int_t& nRec, Int_t& nGhosts, Int_t& nClones);


  /** Divide histograms (reco/all) with correct error for the efficiency
   *@param histo1  reconstructed tracks
   *@param histo2  all tracks (normalisation)
   *@param histo3  efficiency
   **/
  void DivideHistos(TH1* histo1, TH1* histo2, TH1* histo3);


  /** Process one event
   ** @param event  Pointer to event object
   **
   ** If a NULL pointer is given, the entire input array is processed
   ** (legacy mode)
   */
  void ProcessEvent(CbmEvent* event = NULL);


  /** Map from MCTrack index to number of attached StsHits in each station **/
  std::map<Int_t, std::map<Int_t, Int_t>> fHitMap;


  /** Map from MCTrack index to matched StsTrack index **/
  std::map<Int_t, Int_t> fMatchMap;


  /** Map from MCTrack index to percentage of matched hits **/
  std::map<Int_t, Double_t> fQualiMap;


  /** Pointers to data arrays **/
  TClonesArray* fEvents;             //! Event
  CbmMCDataArray* fMCTracks;         //! MCtrack
  CbmMCDataArray* fStsPoints;        //! StsPoints
  TClonesArray* fStsHits;            //! StsHits
  TClonesArray* fStsHitMatch;        //! StsHitMatch
  TClonesArray* fStsTracks;          //! StsTrack
  TClonesArray* fMatches;            //! StsTrackMatch

  /** Flag for legacy mode (event-by-event w/o event objects) **/
  Bool_t fLegacy;


  /** Geometry parameters **/
  CbmGeoPassivePar* fPassGeo;             //! Passive geometry parameters
  TVector3 fTargetPos;                    // Target centre position
  CbmStsSetup* fSetup;                    // STS setup interface
  Int_t fNStations;                       // Number of STS stations


  /** Task parameters **/
  Int_t fMinStations;   // Minimal number of stations with hits for considered MCTrack
  Double_t fQuota;  // True/all hits for track to be considered reconstructed


  /** Histograms **/
  TH1F *fhMomAccAll,  *fhMomRecAll,  *fhMomEffAll;   // eff. vs. p, all
  TH1F *fhMomAccPrim, *fhMomRecPrim, *fhMomEffPrim;  // eff. vs. p, vertex
  TH1F *fhMomAccSec,  *fhMomRecSec,  *fhMomEffSec;   // eff. vs. p, non-vertex
  TH1F *fhNpAccAll,   *fhNpRecAll,   *fhNpEffAll;    // eff. vs. np, all
  TH1F *fhNpAccPrim,  *fhNpRecPrim,  *fhNpEffPrim;   // eff. vs. np, vertex
  TH1F *fhNpAccSec,   *fhNpRecSec,   *fhNpEffSec;    // eff. vs. np, non-vertex
  TH1F *fhZAccSec,    *fhZRecSec,    *fhZEffSec;     // eff. vs. z, non-vertex
  TH1F *fhNhClones,   *fhNhGhosts;              // # hits of clones and ghosts   


  /** List of histograms **/
  TList* fHistoList;


  /** Counters **/
  Int_t fNAccAll, fNAccPrim, fNAccRef, fNAccSec;
  Int_t fNRecAll, fNRecPrim, fNRecRef, fNRecSec;
  Int_t fNGhosts, fNClones;
  Int_t    fNEvents;        /** Number of events with success **/
  Int_t    fNEventsFailed;  /** Number of events with failure **/
  Double_t fTime;           /** Total real time used for good events **/

  /** Timer **/
  TStopwatch fTimer;

  CbmStsFindTracksQa(const CbmStsFindTracksQa&);
  CbmStsFindTracksQa operator=(const CbmStsFindTracksQa&);

  ClassDef(CbmStsFindTracksQa,1);

};


#endif
				 
