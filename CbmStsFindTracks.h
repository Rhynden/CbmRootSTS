// -------------------------------------------------------------------------
// -----                   CbmStsFindTracks header file                -----
// -----                  Created 02/02/05  by V. Friese               -----
// -------------------------------------------------------------------------


/** CbmStsFindTracks
 *@author V.Friese <v.friese@gsi.de>
 **
 ** Task class for track finding in the STS. 
 ** Input: TClonesArray of CbmStsHit
 ** Output: TClonesArray of CbmStsTrack
 **
 ** Uses as track finding algorithm classes derived from CbmStsTrackFinder.
 **/


#ifndef CBMSTSFINDTRACKS
#define CBMSTSFINDTRACKS 1

#include "CbmStsTrackFinder.h"

#include "FairTask.h"

#include "TStopwatch.h"

class TClonesArray;
class FairField;
class CbmGeoStsPar;
class CbmStsDigiPar;
class CbmStsTrackFinderIdeal;


class CbmStsFindTracks : public FairTask
{

 public:

  /** Default constructor **/
  CbmStsFindTracks();


  /** Standard constructor
   *@param verbose  Verbosity level
   *@param name     Task name
   *@param finder   Pointer to STS track finder concrete class
   *@param useMvd   Include MVD hits in track finding
   **/
  CbmStsFindTracks(Int_t iVerbose, 
		   CbmStsTrackFinder* finder = NULL, 
		   Bool_t useMvd = kTRUE,
		   const char* name = "STSFindTracks");


  /** Destructor **/
  virtual ~CbmStsFindTracks();


  /** Task execution **/
  virtual void Exec(Option_t* opt);


  /** Accessors **/
  CbmStsTrackFinder* GetFinder() { return fFinder; };

  /** Return if Mvd is used or not **/
  Bool_t MvdUsage() const { return fUseMvd; } 

  /** Set concrete track finder **/
  void UseFinder(CbmStsTrackFinder* finder) { 
    if ( fFinder ) delete fFinder;
    fFinder = finder; 
  };



 private:

  Bool_t             fUseMvd;      // Inclusion of MVD hits
  CbmGeoStsPar*      fGeoPar;      // STS geometry parameters
  CbmStsDigiPar*     fDigiPar;     // STS digitisation parameters
  //CbmStsDigiScheme*  fDigiScheme;  // STS digitisation scheme
  FairField*         fField;       // Magnetic field
  CbmStsTrackFinder* fFinder;      // TrackFinder concrete class
  TClonesArray*      fMvdHits ;    // Input array of MVD hits
  TClonesArray*      fStsHits ;    // Input array of STS hits
  TClonesArray*      fTracks    ;  // Output array of CbmStsTracks
  TStopwatch         fTimer;       // Timer
  Int_t    fNEvents;        /** Number of events with success **/
  Int_t    fNEventsFailed;  /** Number of events with failure **/
  Double_t fTime;           /** Total real time used for good events **/
  Double_t fNTracks;        /** Number of tracks created **/
  


  /** Get parameter containers **/
  virtual void SetParContainers();


  /** Initialisation at beginning of each event **/
  virtual InitStatus Init();


  /** Finish at the end of each event **/
  virtual void Finish();

  CbmStsFindTracks(const CbmStsFindTracks&);
  CbmStsFindTracks operator=(const CbmStsFindTracks&);

  ClassDef(CbmStsFindTracks,1);

};

#endif
