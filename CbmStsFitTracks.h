// -------------------------------------------------------------------------
// -----                    CbmStsFitTracks header file                -----
// -----                  Created 18/02/05  by V. Friese               -----
// -------------------------------------------------------------------------


/** CbmStsFitTracks
 *@author V.Friese <v.friese@gsi.de>
 **
 ** Task class for track fitting in the STS.
 ** Input: TClonesArray of CbmStsTrack
 ** Parameters of these objects are updated
 **
 ** Uses as track fitting algorithm classes derived from CbmStsTrackFitter.
 **/


#ifndef CBMSTSFITTRACKS
#define CBMSTSFITTRACKS 1


#include "FairTask.h"

#include "TStopwatch.h"

class CbmStsTrackFitter;
class TClonesArray;

class CbmStsFitTracks : public FairTask
{

 public:

  /** Default constructor **/
  CbmStsFitTracks();


  /** Standard constructor 
   **
   *@param finder    Pointer to STS track finder concrete class
   *@param iVerbose  Verbosity level
   **/
  CbmStsFitTracks(CbmStsTrackFitter* fitter, Int_t iVerbose);


  /** Constructor with name
   **
   *@param name      Name of task
   *@param finder    Pointer to STS track finder concrete class
   *@param iVerbose  Verbosity level
   **/
  CbmStsFitTracks(const char* name, CbmStsTrackFitter* fitter,
		  Int_t iVerbose);


  /** Destructor **/
  virtual ~CbmStsFitTracks();


  /** Initialisation at beginning of each event **/
  virtual InitStatus Init();


  /** Task execution **/
  virtual void Exec(Option_t* opt);


  /** Finish at the end of each event **/
  virtual void Finish();


  /** Accessors **/
  CbmStsTrackFitter* GetFitter() { return fFitter; };


  /** Set concrete track finder **/
  void UseFitter(CbmStsTrackFitter* fitter) { fFitter = fitter; };



 private:

  CbmStsTrackFitter* fFitter;    // Pointer to TrackFinder concrete class
  TClonesArray* fTracks;         // Input array of STS tracks
  TStopwatch fTimer;             // Timer
  Int_t fNEvents;                // Number of processed events
  Int_t fNFailed;                // Number of failed events
  Double_t fTime;                // Total real time used
  Double_t fNTracks;             // Number of fitted tracks

  CbmStsFitTracks(const CbmStsFitTracks&);
  CbmStsFitTracks operator=(const CbmStsFitTracks&);


  ClassDef(CbmStsFitTracks,1);

};

#endif
