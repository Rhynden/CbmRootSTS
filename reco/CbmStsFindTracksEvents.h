/** @file CbmFindTracksEvent.h
 ** @author Volker Friese <v.friese@gsi.de>
 ** @since 16.06.2014
 ** @date 23.10.2016
 **/

#ifndef CBMSTSFINDTRACKSEVENTS
#define CBMSTSFINDTRACKSEVENTS 1


#include "TStopwatch.h"
#include "FairTask.h"
#include "CbmStsTrackFinder.h"


class TClonesArray;
class CbmEvent;
class CbmStsTrackFinderIdeal;


/** @class CbmStsFindTracksEvents
 ** @brief Task class for finding STS tracks in an event
 ** @author Volker Friese <v.friese@gsi.de>
 ** @since 16.06.2014
 ** @date 23.10.2016
 ** @version 1.0
 **
 ** This task creates StsTrack objects from a collection of StsHits.
 ** It uses as finding engine a class derived from CVbmStsTrackFinder.
 **/
class CbmStsFindTracksEvents : public FairTask
{

 public:

  /** Constructor
   ** @param finder  Track finder engine. Default: Ideal track finder.
   ** @param useMvd  Include MVD hits in track finding. Default kFALSE.
   **/
  CbmStsFindTracksEvents(CbmStsTrackFinder* finder = NULL,
  		                   Bool_t useMvd = kFALSE);


  /** Destructor **/
  virtual ~CbmStsFindTracksEvents();


  /** Task execution **/
  virtual void Exec(Option_t* opt);


  /** Track finder engine
   ** @value  Pointer to track finding engine
   **/
  CbmStsTrackFinder* GetFinder() { return fFinder; };


  /** Usage of MVD hits
   ** @value  kTRUE if MVD hits are used for tracking
   **/
  Bool_t IsMvdUsed() const { return fUseMvd; }


  /** Set track finding engine
   ** @param finder  Pointer to track finding engine
   **/
  void UseFinder(CbmStsTrackFinder* finder) { 
    if ( fFinder ) delete fFinder;
    fFinder = finder; 
  };


 private:

  Bool_t             fUseMvd;      //  Inclusion of MVD hits
  CbmStsTrackFinder* fFinder;      //  TrackFinder concrete class
  TClonesArray*      fEvents;      //! Array of CbmEvent objects
  TClonesArray*      fMvdHits ;    //! Input array of MVD hits
  TClonesArray*      fStsHits ;    //! Input array of STS hits
  TClonesArray*      fTracks    ;  //! Output array of CbmStsTracks
  TStopwatch         fTimer;       //! Timer
  Int_t    fNofEvents;        ///< Number of events with success
  Double_t fNofHits;          ///< Number of hits
  Double_t fNofTracks;        ///< Number of tracks created
  Double_t fTime;             ///< Total real time used for good events
  

  /** Initialisation at beginning of each event **/
  virtual InitStatus Init();


  /** Finish at the end of each event **/
  virtual void Finish();


  /** Process one event **/
  void ProcessEvent(CbmEvent* event);


  /** Prevent usage of copy constructor and assignment operator **/
  CbmStsFindTracksEvents(const CbmStsFindTracksEvents&);
  CbmStsFindTracksEvents operator=(const CbmStsFindTracksEvents&);


  ClassDef(CbmStsFindTracksEvents,1);

};

#endif
