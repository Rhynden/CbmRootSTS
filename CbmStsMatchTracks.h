// -------------------------------------------------------------------------
// -----                  CbmStsMatchTracks header file                -----
// -----                  Created 22/11/05  by V. Friese               -----
// -------------------------------------------------------------------------


/** CbmStsMatchTracks.h
 *@author V.Friese <v.friese@gsi.de>
 **
 ** Task class for matching a reconstructed CbmStsTrack with a simulated
 ** CbmMCTrack. The matching criterion is a maximal number of common
 ** hits/points. The task fills the data class CbmStsTrackMatch for
 ** each CbmStsTrack.
 **/


#ifndef CBMSTSMATCHTRACKS_H
#define CBMSTSMATCHTRACKS_H 1


#include "FairTask.h"

#include "TStopwatch.h"

#include <map>

class TClonesArray;



class CbmStsMatchTracks : public FairTask
{

 public:

  /** Default constructor **/
  CbmStsMatchTracks();


  /** Standard constructor
   **
   *@param verbose  Verbosity level
   **/
  CbmStsMatchTracks(Int_t iVerbose );


  /** Constructor with task name
   **
   *@param name     Name of task
   *@param verbose  Verbosity level
   **/
  CbmStsMatchTracks(const char* name, Int_t verbose );


  /** Destructor **/
  virtual ~CbmStsMatchTracks();


  /** Intialisation at beginning of each event **/
  virtual InitStatus Init();


  /** Execution **/
  virtual void Exec(Option_t* opt);


  /** Finishing */
  virtual void Finish();


 private:

  TClonesArray* fTracks;       // Array of CbmStsTracks
  TClonesArray* fPoints;       // Array of CbmStsPoints
  TClonesArray* fHits;         // Array of CbmStsHits
  TClonesArray* fMatches;      // Array of CbmStsTrackMatch
  TStopwatch    fTimer;        // Timer

  /** Map from MCTrackId to number of common hits **/
  std::map<Int_t, Int_t> fMatchMap;

  Int_t    fNEvents;        /** Number of events with success **/
  Int_t    fNEventsFailed;  /** Number of events with failure **/
  Double_t fTime;           /** Total real time used for good events **/
  Double_t fNTrackMatches;  /** Total number of matched tracks **/
  Double_t fNAllHits;       /** Total number of hits **/
  Double_t fNTrueHits;      /** Number pf correctly assigned hits **/
  
  CbmStsMatchTracks(const CbmStsMatchTracks&);
  CbmStsMatchTracks operator=(const CbmStsMatchTracks&);

  ClassDef(CbmStsMatchTracks,1);

};

#endif
