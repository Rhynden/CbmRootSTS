// -------------------------------------------------------------------------
// -----                  CbmStsReconstructionQa header file               -----
// -----                  Created 06/02/07  by R. Karabowicz               -----
// -------------------------------------------------------------------------


/** CbmStsReconstructionQa.h
 *@author R.Karabowicz <r.karabowicz@gsi.de>
 **
 ** Quality check task for CbmStsReconstruction
 **/


#ifndef CBMSTSRECONSTRUCTIONQA_H
#define CBMSTSRECONSTRUCTIONQA_H 1


#include "FairTask.h"

#include "TStopwatch.h"
#include "TVector3.h"

#include <map>
#include <set>
class TCanvas;
class TPad;
class TClonesArray;
class TH1;
class TH1F;
class TH2F;
class TH3F;
class TList;
class CbmGeoPassivePar;
class CbmMCTrack;


class CbmStsReconstructionQa : public FairTask
{

 public:

  /** Default constructor **/
  CbmStsReconstructionQa(Int_t iVerbose = 1);


  /** Standard constructor 
  *@param visualizeBool   Bool to turn visualization on/off
  *@param minHits         Minimal number of StsHits for considered MCTracks
  *@param quota           True/all hits for track to be considered reconstructed
  *@param iVerbose        Verbosity level
  **/
  CbmStsReconstructionQa(Bool_t visualizeBool, Int_t minHits, Double_t quota, Int_t iVerbose);


  /** Destructor **/
  virtual ~CbmStsReconstructionQa();


  /** Set parameter containers **/
  virtual void SetParContainers();

  void SetShowStation1(Int_t stNr) {fShowStation1 = stNr-1; }
  void SetShowStation2(Int_t stNr) {fShowStation2 = stNr-1; }

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


  /** Create histograms **/
  void CreateHistos();


  /** Reset histograms and counters **/
  void Reset();


  /** Fill a map from MCTrack index to number of corresponding StsHits **/
  void FillHitMap();


  /** Fill a map from MCTrack index to matched StsTrack index
   *@param nRec  Number of reconstructed tracks (return)
   *@param nGhosts  Number of ghost tracks (return)
   *@param nClones  Number of clone tracks (return)
   **/
  void FillMatchMap(Int_t& nRec, Int_t& nGhosts, Int_t& nClones);


  /** Divide histograms (reco/all) with correct error for the efficiency
   *@param histo1  reconstructed tracks
   *@param histo2  all tracks (normalisation)
   *@param histo3  efficiency
   **/
  void DivideHistos(TH1* histo1, TH1* histo2, TH1* histo3);


  /** Map from MCTrack index to number of attached StsHits **/
  std::map<Int_t, Int_t> fHitMap;
  std::map<CbmMCTrack*, std::set<Int_t> > fHitTrackMap;

  /** Map from MCTrack index to matched StsTrack index **/
  std::map<Int_t, Int_t> fMatchMap;
  

  /** Map from MCTrack index to percentage of matched hits **/
  std::map<Int_t, Double_t> fQualiMap;


  /** Pointers to data arrays **/
  TClonesArray* fMCTracks;           // MCtrack
  TClonesArray* fStsPoints;          // StsPoints
  TClonesArray* fStsHits;            // StsHits
  TClonesArray* fStsTracks;          // StsTrack
  TClonesArray* fMatches;            // StsTrackMatch
  TClonesArray* fStsDigis;           // StsDigi

  /** Geometry parameters **/
  CbmGeoPassivePar* fPassGeo;             // Passive geometry parameters
  TVector3 fTargetPos;                    // Target centre position
  Int_t fNStations;                       // Number of STS stations
  Int_t fStationNrFromMcId[10000];         // station number from mc id
  Int_t fNSectors[20];                    // number of sectors per station
  Double_t fWidthSectors[20][500];        // width of the sectors to calculate nof channels
  Int_t HitSt [100000][10];
  /** Task parameters **/
  Int_t fMinHits;   // Minimal number of StsHits for considered MCTrack
  Double_t fQuota;  // True/all hits for track to be considered reconstructed


  /** Histograms **/
  TH1F* fhMomAccAll,  *fhMomRecAll,  *fhMomEffAll;   // eff. vs. p, all
  TH1F* fhMomAccPrim, *fhMomRecPrim, *fhMomEffPrim;  // eff. vs. p, vertex
  TH1F* fhMomAccSec,  *fhMomRecSec,  *fhMomEffSec;   // eff. vs. p, non-vertex
  TH1F* fhNpAccAll,   *fhNpRecAll,   *fhNpEffAll;    // eff. vs. np, all
  TH1F* fhNpAccPrim,  *fhNpRecPrim,  *fhNpEffPrim;   // eff. vs. np, vertex
  TH1F* fhNpAccSec,   *fhNpRecSec,   *fhNpEffSec;    // eff. vs. np, non-vertex
  TH1F* fhZAccSec,    *fhZRecSec,    *fhZEffSec;     // eff. vs. z, non-vertex
  TH1F* fhNhClones,   *fhNhGhosts;              // # hits of clones and ghosts   
  
  // acc., eff. vs. track direction, vertex
//   TH2F* fhDirEmiPrimM[25], *fhDirAccPrimM[25], *fhDirAcMPrimM[25], *fhDirRecPrimM[25], *fhDirEffPrimM[25];
//   TH2F* fhDirEmiPrimP[25], *fhDirAccPrimP[25], *fhDirAcMPrimP[25], *fhDirRecPrimP[25], *fhDirEffPrimP[25];
  
  Int_t fPartPdgTable[10];
  TH1F *fhMomAccPart[10], *fhMomRecPart[10], *fhMomEffPart[10];

  TH1F* fhMomClones,   *fhMomGhosts;              // # hits of clones and ghosts   

  TH2F* fhMomResAll;
  TH2F* fhMomResPrim;
  TH2F* fhMomResSec;
  TH1F* fhLowBand;
  TH1F* fhHigBand;

  //  TH2F* fhHitPointCorrelation[100];

  TH3F* fhPrimaryVertex;
  TH1F* fhRefTracks;
  TH1F* fhRecRefTracks;

/*   TH1F* fhNofDigisPChip[20][300][2][8];  // per station, sector, side, chip */
/*   TH1F* fhNofFiredDigis[20][300][2];  // per station, sector, side */
/*   TH1F* fhNofHits      [20][300];     // per station, sector */

/*   TH2F* fhEnergyLoss[20]; */

  // histograms for track reconstruction: chi2, covMatr., etc...
  TH1F* fhStsTrackFPos[3];
  TH1F* fhStsTrackLPos[3];
  TH1F* fhStsTrackFDir[2];
  TH1F* fhStsTrackLDir[2];
//   TH1F* fhStsTrackFCovEl[15];
//   TH1F* fhStsTrackLCovEl[15];
  TH1F* fhStsTrackFMom;
  TH1F* fhStsTrackLMom;
  TH1F* fhStsTrackChiSq;

  /** List of histograms **/
  TList* fHistoList;

  TList* fOccupHList;
  
  /** Counters **/
  Int_t fNAccAll, fNAccPrim, fNAccRef, fNAccSec;
  Int_t fNRecAll, fNRecPrim, fNRecRef, fNRecSec;
  Int_t fNGhosts, fNClones;
  Int_t fNStsTracks;
  Int_t    fNEvents;        /** Number of events with success **/
  Int_t    fNEventsFailed;  /** Number of events with failure **/
  Double_t fTime;           /** Total real time used for good events **/

  Bool_t fOnlineAnalysis;
  TCanvas* fOnlineCanvas;
  TPad*    fOnlinePad[10];
  Int_t fShowStation1;
  Int_t fShowStation2;

  Int_t fNofFiredDigis[20][300][2];  // per station, sector, side
  Int_t fNofDigisPChip[20][300][2][8];  // per station, sector, side, chip
  Int_t fNofHits[20][300];           // per station, sector

  /** Timer **/
  TStopwatch fTimer;

  CbmStsReconstructionQa(const CbmStsReconstructionQa&);
  CbmStsReconstructionQa operator=(const CbmStsReconstructionQa&);

  ClassDef(CbmStsReconstructionQa,1);

};


#endif
				 
