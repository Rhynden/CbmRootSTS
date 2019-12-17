/** @file CbmTrackFinderIdeal.h
 ** @author Volker Friese <v.friese@gsi.de>
 ** @since 2.2.2005
 ** @date 23.10.2016
 **/


/** CbmStsTrackFinderIdeal
 *@author V.Friese <v.friese@gsi.de>
 **
 ** Ideal track finder in the STS for simulated data. 
 ** For each MCTrack having at least 3 StsPoints, a StsTrack is created
 ** and the corresponding StsHits are attached using the correspondence
 ** between StsHit and StsPoint.
 **/


#ifndef CBMSTSTRACKFINDERIDEAL
#define CBMSTSTRACKFINDERIDEAL 1


#include "CbmStsTrackFinder.h"

class TClonesArray;


/** @class CbmStsTrackFinderIdeal
 ** @brief MC-based track finding in the STS
 ** @author V.Friese <v.friese@gsi.de>
 ** @since 2.2.2005
 ** @date 23.10.2016
 **
 ** Ideal track finder in the STS for simulated data.
 ** For each MCTrack having at least 3 StsPoints, a StsTrack is created
 ** and the corresponding StsHits are attached using the correspondence
 ** between StsHit and StsPoint.
 **/
class CbmStsTrackFinderIdeal : public CbmStsTrackFinder
{

 public:

  /** Default constructor **/
  CbmStsTrackFinderIdeal();


  /** Standard constructor **/
  CbmStsTrackFinderIdeal(Int_t verbose);


  /** Destructor **/
  virtual ~CbmStsTrackFinderIdeal();


  /** Initialisation **/
  virtual void Init();


  /** Track finding algorithm
   ** This just reads MC truth (MCTracks and MCPoints), creates
   ** one StsTrack for each MCTrack and attaches the hits according
   ** to the MCTrack of the corresponding MCPoint
   **
   *@param mHitArray   Array of CbmStsHit
   *@param trackArray  Array of CbmStsTrack
   **
   *@value Number of tracks created
   **/
 virtual Int_t DoFind();

 virtual Int_t FindTracks(CbmEvent* /*event*/) { return 0; }



 private:

  /** Arrays of MC information **/
  TClonesArray* fMCTrackArray;
  TClonesArray* fMCPointArray;

  CbmStsTrackFinderIdeal(const CbmStsTrackFinderIdeal&);
  CbmStsTrackFinderIdeal operator=(const CbmStsTrackFinderIdeal&);

  ClassDef(CbmStsTrackFinderIdeal,1);

};


#endif
