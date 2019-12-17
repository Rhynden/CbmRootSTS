//* $Id: */

// -------------------------------------------------------------------------
// -----                     CbmStsDigitizeQa header file              -----
// -----                  Created 2015  by Hanna Malygina              -----
// -------------------------------------------------------------------------


/** CbmStsDigitizeQa
 *@author Hanna Malygina <h.malygina@gsi.de>
 *@since 04.2015
 *@version 1.0
 **
 ** CBM task class for digitization in the STS
 ** 
 ** 
 **/


#ifndef CBMSTSDIGITIZEQA_H
#define CBMSTSDIGITIZEQA_H 1

#include "FairTask.h"

#include "TStopwatch.h"
#include "FairLogger.h"

#include <vector>

class TClonesArray;
class CbmStsSetup;
class CbmStsDigitize;

class TCanvas;
class TLegend;
class TPad;
class TH1D;
class TH2D;
class TList;

class CbmStsDigitizeQa : public FairTask
{
    public :

	/** Default constructor **/
	CbmStsDigitizeQa(CbmStsDigitize* digitizer);


	/** Standard constructor **/
	CbmStsDigitizeQa(CbmStsDigitize* digitizer, Bool_t visualizeBool);

	/** Destructor **/
	virtual ~CbmStsDigitizeQa();

	/** Create histograms **/
	void CreateHistos();

	/** Reset histograms and counters **/
	void Reset();

	/** Execution **/
	virtual void Exec(Option_t* opt);

	/** Virtual method Finish **/
	virtual void Finish();

	/** Set filename for pictures **/
	void SetPrint(Bool_t print, char outName[] ){
	    fPrint = print;
	    if (fPrint){
		sprintf(fOutName, "%s", outName); 
		LOG(info) << GetName() << ": outName for digitizeQa pictures: " << fOutName;
	    } else {
		LOG(info) << GetName() << ": do NOT save digitizeQa pictures";
	    }
	}

    private:

	CbmStsDigitize* fDigitizer;
	const static Int_t fNPads = 7;
	Int_t fNStations;

	Int_t fNEvents;        /** Number of events with success **/
	Int_t fNTotDigis;            // total number of digis for all events
	Int_t fNTotPoints;           // total number of MCpoints for all events
	Int_t fNTotSignalsF;  
	Int_t fNTotSignalsB; 
	Double_t fMeanDigisPMCpoint;// mean number digis per 1 MC point
	Double_t fMeanDigisPoints;//just nofDigis / nofPoints
	Double_t fMeanMCpointsPDigi;// mean number MC points per 1 digi
	Int_t fNAdc;

	/** Pointers to data arrays **/
	TClonesArray* fStsPoints;          // StsPoints
	TClonesArray* fStsDigis;           // StsDigis
	TClonesArray* fStsDigiMatches;     // StsDigiMatches

	TStopwatch fTimer;
	CbmStsSetup * fSetup;

	/** Total real time used for good events **/
	Double_t  fTime1;     

	//histogramms
	TH1D * fhMCpointsPDigi; 
	TH1D * fhMCpointElossGeant; 
	TH1D * fhDigisPMCpoint; 
	TH1D * fhDigiCharge; 
	TH1D * fhDigisPEvent; 
	std::vector<TH2D * > fhDigisPChannelPModuleAtStation;

	/** List of histograms **/
	TList* fHistoList;

	Bool_t fOnlineAnalysis;
	TCanvas* digiCanvas;
	TCanvas* occupCanvas;
	TPad * digiPad[fNPads];
	TLegend * leg[fNPads];

	char fOutName[200];
	Bool_t fPrint;

	/** Intialisation **/
	virtual InitStatus Init();

	/** Reinitialisation **/
	virtual InitStatus ReInit();

	CbmStsDigitizeQa(const CbmStsDigitizeQa&);
	CbmStsDigitizeQa operator=(const CbmStsDigitizeQa&);

	ClassDef(CbmStsDigitizeQa,1);

};

#endif
