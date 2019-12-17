// -------------------------------------------------------------------------
// -----                  CbmStsSimulationQa header file               -----
// -----                  Created 02/02/07  by R. Karabowicz           -----
// -------------------------------------------------------------------------


/** CbmStsSimulationQa.h
 *@author R.Karabowicz <r.karabowicz@gsi.de>
 **
 ** Quality check task for CbmStsSimulation
 **/


#ifndef CBMSTSSIMULATIONQA_H
#define CBMSTSSIMULATIONQA_H 1

#include "FairTask.h"

#include "TVector3.h"

class TCanvas;
class TPad;
class TClonesArray;
class TH1F;
class TH2F;
class TH3F;
class TList;
class CbmGeoPassivePar;



class CbmStsSimulationQa : public FairTask
{

 public:

  /** Default constructor **/
  CbmStsSimulationQa();


  /** Standard constructor 
  *@param visualizeBool   Bool to turn visualization on/off
  **/
  CbmStsSimulationQa(Bool_t visualizeBool, Int_t iVerbose = 1);


  /** Destructor **/
  virtual ~CbmStsSimulationQa();


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


  /** Create histograms **/
  void CreateHistos();


  /** Reset histograms and counters **/
  void Reset();


  /** Pointers to data arrays **/
  TClonesArray* fMCTracks;           // MCtrack
  TClonesArray* fSTSPoints;          // MCpoints


  /** Geometry parameters **/
  CbmGeoPassivePar* fPassGeo;             // Passive geometry parameters
  TVector3 fTargetPos;                    // Target centre position
  Int_t fNStations;                       // Number of STS stations
  Int_t fStationsMCId[10];                // MC ID of the STS stations
  Int_t fStationNrFromMcId[10000];         // station number from mc id
  Float_t fStationRadius[10];
  Int_t fNSectors[10];                    // Number of STS sectors per station

  
  /** Histograms **/ 
  TH1F* fhMomAll; 
  TH2F *fhYPtMapAll; 
  TH1F *fhPdgCodeAll; 
  TH1F *fhStsPointsAll; 
  TH1F* fhMomRec; 
  TH2F *fhYPtMapRec; 
  TH1F *fhPdgCodeRec; 
  TH1F *fhStsPointsRec; 
  TH2F *fhMomStsPoints; 
  TH3F *fhStsPointsPosition; 
  TH2F *fhStationPoints[10]; 

  TH1F *fhNofEvents;
  TH1F *fhNofStsStations;

  TH1F* fhDistIn;
  TH1F* fhDistOut;

  /** List of histograms **/
  TList* fHistoList;


  /** Counters **/
  Int_t fNEvents;

  Bool_t fOnlineAnalysis;
  TCanvas* fOnlineCanvas;
  TPad*    fOnlinePad[10];

  CbmStsSimulationQa(const CbmStsSimulationQa&);
  CbmStsSimulationQa operator=(const CbmStsSimulationQa&);

  ClassDef(CbmStsSimulationQa,1);

};


#endif
				 
