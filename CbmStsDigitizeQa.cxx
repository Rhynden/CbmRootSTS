/* $Id: */

// -------------------------------------------------------------------------
// -----                    CbmStsDigitizeQa source file         -----
// -----                  Created 04.2015  by Hanna Malygina              -----
// -------------------------------------------------------------------------

#include "CbmStsDigitizeQa.h"

#include "CbmStsAddress.h"
#include "CbmStsDigi.h"
#include "CbmStsDigitize.h"
#include "CbmMatch.h"
#include "CbmStsSetup.h"
#include "CbmStsModule.h"
#include "CbmStsElement.h"
#include "CbmStsStation.h"
#include "CbmStsPoint.h"

#include "FairRootManager.h"
#include "FairRunAna.h"
#include "FairRuntimeDb.h"

#include "TClonesArray.h"
#include "TStyle.h"
#include "TCanvas.h"
#include "TPad.h"
#include "TLegend.h"
#include "TPaveText.h"
#include "TPaveStats.h"
#include "TFile.h"
#include "TH1F.h"
#include "TH2F.h"
#include "TList.h"
#include "TAxis.h"
#include "TAttAxis.h"
#include "TGaxis.h"

using std::vector;

// -----   Default constructor   ------------------------------------------
CbmStsDigitizeQa::CbmStsDigitizeQa(CbmStsDigitize * digitizer) 
    : FairTask("STSDigitizeQa"),
    fDigitizer(digitizer),
    fNStations(0),
    fNEvents(0),
    fNTotDigis(0),
    fNTotPoints(0),
    fNTotSignalsF(0),
    fNTotSignalsB(0),
    fMeanDigisPMCpoint(0.),
    fMeanDigisPoints(0.),
    fMeanMCpointsPDigi(0.),
    fNAdc(0),
    fStsPoints(NULL),         // StsPoints
    fStsDigis(NULL),          // StsDigis
    fStsDigiMatches(NULL),    // StsDigiMatches
    fTimer(),
    fSetup(),
    fTime1(0.),
    fhMCpointsPDigi(NULL), 
    fhMCpointElossGeant(NULL), 
    fhDigisPMCpoint(NULL), 
    fhDigiCharge(NULL), 
    fhDigisPEvent(NULL), 
    fhDigisPChannelPModuleAtStation(),
    fHistoList(NULL),
    fOnlineAnalysis(kFALSE),
    digiCanvas(),
    occupCanvas(),
    digiPad(),
    leg(),
    fOutName(),
    fPrint(0)
{  
}
// -------------------------------------------------------------------------


// -----   Standard constructor   ------------------------------------------
CbmStsDigitizeQa::CbmStsDigitizeQa(CbmStsDigitize * digitizer, Bool_t visualizeBool)
    : FairTask("STSDigitizeQa"), 
    fDigitizer(digitizer),
    fNStations(0),
    fNEvents(0),
    fNTotDigis(0),
    fNTotPoints(0),
    fNTotSignalsF(0),
    fNTotSignalsB(0),
    fMeanDigisPMCpoint(0.),
    fMeanDigisPoints(0.),
    fMeanMCpointsPDigi(0.),
    fNAdc(0),
    fStsPoints(NULL),         // StsPoints
    fStsDigis(NULL),          // StsDigis
    fStsDigiMatches(NULL),    // StsDigiMatches
    fTimer(),
    fSetup(),
    fTime1(0.),
    fhMCpointsPDigi(NULL), 
    fhMCpointElossGeant(NULL), 
    fhDigisPMCpoint(NULL), 
    fhDigiCharge(NULL), 
    fhDigisPEvent(NULL), 
    fhDigisPChannelPModuleAtStation(),
    fHistoList(NULL),
    fOnlineAnalysis(visualizeBool),
    digiCanvas(),
    occupCanvas(),
    digiPad(),
    leg(),
    fOutName(),
    fPrint(0)
{ 
}
// -------------------------------------------------------------------------


// -----   Destructor   ----------------------------------------------------
CbmStsDigitizeQa::~CbmStsDigitizeQa() {
    fHistoList -> Delete();
    delete fHistoList;
    delete[] fHistoList;
}
// -------------------------------------------------------------------------


// -----   Private method CreateHistos   -----------------------------------
void CbmStsDigitizeQa::CreateHistos() {

    TGaxis::SetMaxDigits(3);
    // Histogram list
    fHistoList = new TList();

    fhMCpointsPDigi        = new TH1D ("hMCpointsPDigi"      , "MC points per digi"                     
	    ,10,0.5,10.5);
    fhMCpointElossGeant    = new TH1D ("hMCpointElossGeant"  , "MC point energy loss simulated by Geant"
	    ,1000,0,1000);
    fhDigiCharge           = new TH1D ("hDigiCharge"         , "Digi charge"                            
	    ,fNAdc,0,Double_t(fNAdc));
    fhDigisPEvent          = new TH1D ("hDigisPEvent"        , "Digis per event"                        
	    ,35,0,35000);
    fhDigisPMCpoint        = new TH1D ("hDigisPMCpoint"      , "Digis per MC point (both sides)"        
	    ,40,-0.5,39.5);

    for (Int_t iStation = 0; iStation < fNStations; iStation ++) {
	fhDigisPChannelPModuleAtStation[iStation] = new TH2D ("hDigisPChannelPModuleAtStation", Form("Digis per channel at station#%i", iStation + 1), 136,0,136, 200,0,0.2);
	fhDigisPChannelPModuleAtStation[iStation] -> GetXaxis() -> SetTitle("Module number");
	fhDigisPChannelPModuleAtStation[iStation] -> GetXaxis() -> CenterTitle();
	fhDigisPChannelPModuleAtStation[iStation] -> GetXaxis() -> SetTitleSize(0.05);
	fhDigisPChannelPModuleAtStation[iStation] -> GetXaxis() -> SetTitleOffset(0.9);
	fhDigisPChannelPModuleAtStation[iStation] -> GetXaxis() -> SetLabelSize(0.05);
	fhDigisPChannelPModuleAtStation[iStation] -> GetYaxis() -> SetTitle("Digis per channel");
	fhDigisPChannelPModuleAtStation[iStation] -> GetYaxis() -> CenterTitle();
	fhDigisPChannelPModuleAtStation[iStation] -> GetYaxis() -> SetTitleSize(0.05);
	fhDigisPChannelPModuleAtStation[iStation] -> GetYaxis() -> SetTitleOffset(1.5);
	fhDigisPChannelPModuleAtStation[iStation] -> GetYaxis() -> SetLabelSize(0.05);
	fhDigisPChannelPModuleAtStation[iStation] -> GetZaxis() -> SetTitle("Event entries");
    }

    //set histo, axis etc. parameters
    fhMCpointsPDigi     -> GetXaxis() -> SetTitle("Number of MC points");
    fhMCpointsPDigi     -> GetYaxis() -> SetTitle("Digi entries");
    fhMCpointsPDigi     -> GetXaxis() -> SetNdivisions(10,2,0);
    fhMCpointsPDigi     -> SetTitle("MC points per digi");

    fhMCpointElossGeant -> GetXaxis() -> SetTitle("Deposited energy, keV");
    fhMCpointElossGeant -> GetYaxis() -> SetTitle("MC point entries");
    fhMCpointElossGeant -> GetYaxis() -> SetNdivisions(6,5,0);
    fhMCpointElossGeant -> SetTitle("Deposited energy (Geant)");

    fhDigisPMCpoint     -> GetXaxis() -> SetTitle("Number of digis");
    fhDigisPMCpoint     -> GetYaxis() -> SetTitle("MC points entries");
    fhDigisPMCpoint     -> GetXaxis() -> SetNdivisions(10,2,0);
    fhDigisPMCpoint     -> SetTitle("Digis per MC point");

    fhDigiCharge        -> GetXaxis() -> SetTitle("Digi charge, ADC");
    fhDigiCharge        -> GetYaxis() -> SetTitle("Digi entries");
    fhDigiCharge        -> GetYaxis() -> SetNdivisions(6,5,0);
    fhDigiCharge        -> SetTitle("Digi charge");

    fhDigisPEvent       -> GetXaxis() -> SetTitle("Number of digis");
    fhDigisPEvent       -> GetYaxis() -> SetTitle("Event entries");
    fhDigisPEvent       -> SetTitle("Digis per event");

    fHistoList -> Add(fhMCpointsPDigi);
    fHistoList -> Add(fhMCpointElossGeant);
    fHistoList -> Add(fhDigisPMCpoint);
    fHistoList -> Add(fhDigiCharge);
    fHistoList -> Add(fhDigisPEvent);

    if (fOnlineAnalysis){
	TIter next(fHistoList);
	while ( TH1* histo = ((TH1*)next()) ) {
	    histo -> GetXaxis() -> SetTitleSize(0.05);
	    histo -> GetXaxis() -> SetLabelSize(0.05);
	    histo -> GetYaxis() -> SetTitleSize(0.05);
	    histo -> GetYaxis() -> SetLabelSize(0.05);
	    histo -> GetYaxis() -> SetTitleOffset(1.2);
	    histo -> SetLineWidth(2);
	    histo -> SetLineColor(4);
	}
    }
    for (Int_t iStation = 0; iStation < fNStations; iStation ++) fHistoList -> Add(fhDigisPChannelPModuleAtStation[iStation]);

}
// -------------------------------------------------------------------------


// -----   Virtual method Finish   -----------------------------------------
void CbmStsDigitizeQa::Finish() {

    if ( fOnlineAnalysis ) {
	fMeanMCpointsPDigi /= fNEvents;
	digiPad[0] -> cd();
	gStyle -> SetOptStat("e");
	leg[0] -> AddEntry((TObject*)0, Form("mean = %0.3f", fMeanMCpointsPDigi), "");
	leg[0] -> SetTextSize(0.05);
	leg[0] -> Draw();
	digiPad[0] -> Update();

	fMeanDigisPMCpoint /= fNEvents;
	digiPad[1] -> cd();
	gStyle -> SetOptStat("e");
	leg[1] -> AddEntry((TObject*)0, Form("mean = %0.3f", fMeanDigisPMCpoint), "");
	leg[1] -> SetTextSize(0.05);
	leg[1] -> Draw();
	digiPad[1] -> Update();

	if (fPrint){
	    digiCanvas -> Print(Form("%s.png", fOutName));
	    digiCanvas -> Print(Form("%s.eps", fOutName));
	    digiCanvas -> Print(Form("%s.C",   fOutName));
	}

	Double_t maxZaxis = 0;
	for (Int_t iStation = 0; iStation < fNStations; iStation ++)
	    if (fhDigisPChannelPModuleAtStation[iStation] -> GetMaximum() > maxZaxis) 
		maxZaxis = fhDigisPChannelPModuleAtStation[iStation] -> GetMaximum();

	for (Int_t iStation = 0; iStation < fNStations; iStation ++){
	    occupCanvas -> cd (iStation + 1);
	    gStyle -> SetOptStat("em");
	    fhDigisPChannelPModuleAtStation[iStation] -> SetMaximum(maxZaxis);
	    gPad -> Update();
	}

	if (fPrint){
	    occupCanvas -> Print(Form("%sOccup.png", fOutName));
	    occupCanvas -> Print(Form("%sOccup.eps", fOutName));
	    occupCanvas -> Print(Form("%sOccup.C",   fOutName));
	}

    }

    gDirectory -> mkdir("STSDigitizeQA");
    gDirectory -> cd("STSDigitizeQA");
    TIter next(fHistoList);
    while ( TH1* histo = ((TH1*)next()) ) histo -> Write();
    gDirectory->cd("..");

   }					       
// -------------------------------------------------------------------------


// -----   Private method Init   -------------------------------------------
InitStatus CbmStsDigitizeQa::Init() {

    // Get input array
    FairRootManager* ioman = FairRootManager::Instance();
    if ( ! ioman ) Fatal("Init", "No FairRootManager");


    // Get StsPoint array
    fStsPoints = (TClonesArray*) ioman -> GetObject("StsPoint");
    if ( ! fStsPoints ) {
	LOG(error) << GetName() << "::Init: No StsPoint array!";
	return kFATAL;
    }

    // Get StsDigis array
    fStsDigis = (TClonesArray*) ioman -> GetObject("StsDigi");
    if ( ! fStsDigis ) {
	LOG(error) << GetName() << "::Init: No StsDigi array!";
	return kERROR;
    }

    // Get StsDigiMatches array
    fStsDigiMatches = (TClonesArray*) ioman -> GetObject("StsDigiMatch");
    if ( ! fStsDigiMatches ) {
	LOG(error) << GetName() << "::Init: No StsDigiMaches array!";
	return kERROR;
    }

    // Get STS setup interface
    fSetup = CbmStsSetup::Instance();
    fNStations = fSetup -> GetNofDaughters();

    fMeanDigisPMCpoint  = 0.;
    fMeanDigisPoints    = 0.;
    fMeanMCpointsPDigi  = 0.;

    Double_t noise, threshold, timeResolution, deadTime, dynRange, deadChannelFraction;
    deadChannelFraction = fDigitizer -> GetDeadChannelFraction();
    fDigitizer -> GetParameters(dynRange, threshold, fNAdc, timeResolution, deadTime, noise);

    fhDigisPChannelPModuleAtStation.resize(fNStations);
    CreateHistos();
    Reset();

    TStyle * plainStyle = new TStyle("plain", "plain");
    plainStyle -> SetPadColor(0);
    plainStyle -> SetCanvasColor(0);
    plainStyle -> SetOptStat("em");
    plainStyle -> SetStatW(0.4);
    plainStyle -> SetStatH(0.2);
    plainStyle -> SetStatColor(0);
    plainStyle -> SetStatBorderSize(1);
    plainStyle -> SetLegendFillColor(0);
    plainStyle -> SetLegendBorderSize(1);
   // plainStyle -> SetHistLineWidth(2);
  //  plainStyle -> SetHistLineColor(4);
   // plainStyle -> SetTitleSize(0.05,"");
    plainStyle -> SetTitleBorderSize(0);
    plainStyle -> SetTitleColor(0);
    plainStyle -> SetTitleFillColor(0);
   // plainStyle -> SetTitleOffset(1.2, "y");
    plainStyle -> SetPadLeftMargin(0.15);
    plainStyle -> SetPadRightMargin(0.1);
    plainStyle -> SetPadTopMargin(0.13);
    plainStyle -> SetPadBottomMargin(0.18);
    plainStyle -> SetPalette(1,0);

    plainStyle -> cd();

    if ( fOnlineAnalysis ) {
	digiCanvas = new TCanvas("StsDigitizeCanvas","Sts Digitization",10,10,1200,700);
	digiCanvas -> UseCurrentStyle();

	digiPad[0] = new TPad("MCpointsPad","MC points per digi pad"        ,0.00,0.50,0.33,1.00);
	digiPad[1] = new TPad("digisPad","Digis per MC pointpad"            ,0.33,0.50,0.66,1.00);
	digiPad[2] = new TPad("digiChargePad","Digi charge pad"             ,0.66,0.50,1.00,1.00);
	digiPad[3] = new TPad("ElossGeantPad","Deposited energy (Geant) pad",0.00,0.00,0.33,0.50);
	digiPad[4] = new TPad("digisPeventPad","Digis per event pad"        ,0.33,0.00,0.66,0.50);
	digiPad[5] = new TPad("textPad",   "Text pad"                       ,0.66,0.02,1.00,0.22);
	digiPad[6] = new TPad("textPad2",   "Text pad2"                     ,0.66,0.23,1.00,0.50);

	digiPad[0] -> SetLogy();
	digiPad[0] -> SetGridy();
	digiPad[0] -> SetGridx();

	digiPad[1] -> SetGridy();
	digiPad[1] -> SetGridx();

	digiPad[2] -> SetGridy();
	digiPad[2] -> SetGridx();

	digiPad[3] -> SetGridy();
	digiPad[3] -> SetGridx();

	for ( Int_t ipad = 0 ; ipad < fNPads ; ipad++ ) {
	    digiPad[ipad] -> Draw();
	    leg[ipad] = new TLegend (0.58, 0.82, 0.98, 0.89);
	}
	char digiModel[7];
	if (fDigitizer -> GetDigitizeModel() == 0) sprintf(digiModel, "IDEAL");
	if (fDigitizer -> GetDigitizeModel() == 1) sprintf(digiModel, "SIMPLE");
	if (fDigitizer -> GetDigitizeModel() == 2) sprintf(digiModel, "REAL");

	digiPad[6] -> cd();
	digiPad[6] -> SetMargin (0.,0.,0.,0.);
	TPaveText* printoutPave2 = new TPaveText(0.0, 0.1, 1.0, 1.0);
	printoutPave2 -> SetTextAlign(13);
	printoutPave2 -> SetTextColor(1);
	printoutPave2 -> SetTextSize(0.09);
	printoutPave2 -> SetBorderSize(0);
	printoutPave2 -> SetFillColor(0);
	printoutPave2 -> AddText(Form("RUN SUMMARY"));
	printoutPave2 -> AddText(Form("Digitizer model   %s", digiModel));
	printoutPave2 -> AddText(Form("Digitizer parameters:"));
	printoutPave2 -> AddText(Form("   noise                            %.0f e", noise));
	printoutPave2 -> AddText(Form("   threshold                     %.0f e", threshold));
	printoutPave2 -> AddText(Form("   dead channels             %.0f percent", deadChannelFraction));
	printoutPave2 -> AddText(Form("   time resolution            %.0f ns", timeResolution));
	printoutPave2 -> AddText(Form("   dead time                     %.0f ns", deadTime));
	printoutPave2 -> AddText(Form("   adc channel number   %i", fNAdc));
	printoutPave2 -> AddText(Form("   dynamic range             %.0f e", dynRange));
	digiPad[6] -> Clear();
	printoutPave2 -> Draw();
	digiPad[6] -> Update();


	// next canvas
	occupCanvas = new TCanvas("StsDigitizeOccupancyCanvas","Sts Occupancy",10,10,1200,700);
	gStyle -> SetPadBorderMode(0);
	gStyle -> SetFrameBorderMode(0);
	gStyle -> SetPadGridY(1);
	Float_t small = 1.e-5;
	occupCanvas -> Divide(4,2,small,small);

	occupCanvas -> cd (1);
	gPad -> SetRightMargin(small);
	gPad -> SetBottomMargin(small);
	occupCanvas -> cd (2);
	gPad -> SetRightMargin(small);
	gPad -> SetLeftMargin(small);
	gPad -> SetBottomMargin(small);
	occupCanvas -> cd (3);
	gPad -> SetRightMargin(small);
	gPad -> SetLeftMargin(small);
	gPad -> SetBottomMargin(small);
	occupCanvas -> cd (4);
	gPad -> SetLeftMargin(small);
	gPad -> SetBottomMargin(small);

	occupCanvas -> cd (5);
	gPad -> SetRightMargin(small);
	gPad -> SetTopMargin(small);
	occupCanvas -> cd (6);
	gPad -> SetRightMargin(small);
	gPad -> SetLeftMargin(small);
	gPad -> SetTopMargin(small);
	occupCanvas -> cd (7);
	gPad -> SetRightMargin(small);
	gPad -> SetLeftMargin(small);
	gPad -> SetTopMargin(small);
	occupCanvas -> cd (8);
	gPad -> SetLeftMargin(small);
	gPad -> SetTopMargin(small);
    }


    return kSUCCESS;

}
// -------------------------------------------------------------------------


// -----   Public method Exec   --------------------------------------------
void CbmStsDigitizeQa::Exec(Option_t* /*opt*/) {

    fTimer.Start();

    Int_t nofStsPoints = fStsPoints -> GetEntriesFast();
    Int_t nofStsDigis  = fStsDigis  -> GetEntriesFast();

    fhDigisPEvent -> Fill(nofStsDigis);

    vector<Int_t> nDigisPMCpoint (nofStsPoints, 0);
    Double_t meanDigisPMCpoint = 0.;
    Double_t meanMCpointsPDigi = 0.;

    for (Int_t iDigi = 0; iDigi < nofStsDigis; iDigi ++){
	const CbmMatch * digiMatch = static_cast<const CbmMatch*>(fStsDigiMatches -> At(iDigi));
	fhMCpointsPDigi -> Fill (digiMatch -> GetNofLinks());
	meanMCpointsPDigi += digiMatch -> GetNofLinks();
	const CbmStsDigi * digi = static_cast<const CbmStsDigi*>(fStsDigis -> At(iDigi));
	fhDigiCharge -> Fill (digi -> GetCharge());
	for(Int_t iLink = 0; iLink < digiMatch -> GetNofLinks(); iLink ++){
	    Int_t iPoint = digiMatch -> GetLink(iLink).GetIndex();
	    nDigisPMCpoint[iPoint]++;
	}
    }
    meanMCpointsPDigi /= nofStsDigis;
    fMeanMCpointsPDigi += meanMCpointsPDigi;

    for (Int_t iPoint = 0; iPoint < nofStsPoints; iPoint ++){
	const FairMCPoint * stsPoint = static_cast<const FairMCPoint*>(fStsPoints -> At(iPoint));
	fhMCpointElossGeant -> Fill (stsPoint -> GetEnergyLoss() * 1.e6);//keV
	fhDigisPMCpoint  -> Fill(nDigisPMCpoint[iPoint]);
	meanDigisPMCpoint += nDigisPMCpoint[iPoint];
    }
    meanDigisPMCpoint   /= nofStsPoints;
    fMeanDigisPMCpoint  += meanDigisPMCpoint;

    fNTotSignalsF += fDigitizer -> GetNofSignalsF();
    fNTotSignalsB += fDigitizer -> GetNofSignalsB();
    fMeanDigisPoints += nofStsDigis / nofStsPoints;
    fNTotDigis  += nofStsDigis;
    fNTotPoints += nofStsPoints;
    fNEvents++;

    if ( fOnlineAnalysis ) {
	digiPad[0] -> cd();
	fhMCpointsPDigi -> Draw();
	digiPad[0] -> Update();

	digiPad[1] -> cd();
	fhDigisPMCpoint -> Draw();
	digiPad[1] -> Update();

	digiPad[2] -> cd();
	fhDigiCharge -> Draw();
	digiPad[2] -> Update();

	digiPad[3] -> cd();
	fhMCpointElossGeant -> Draw();
	digiPad[3] -> Update();

	digiPad[4] -> cd();
	fhDigisPEvent -> Draw();
	digiPad[4] -> Update();

	digiPad[5] -> cd();
	digiPad[5] -> SetMargin (0.,0.,0.,0.);
	TPaveText* printoutPave = new TPaveText(0.0, 0.1, 1.0, 1.0);
	printoutPave -> SetTextAlign(13);
	printoutPave -> SetTextColor(1);
	printoutPave -> SetTextSize(0.12);
	printoutPave -> SetBorderSize(0);
	printoutPave -> SetFillColor(0);
	printoutPave -> AddText(Form("Events                  %i ",   fNEvents));
	printoutPave -> AddText(Form("SignalsF / event  %3.0f", Double_t (fNTotSignalsF)/Double_t (fNEvents)));
	printoutPave -> AddText(Form("SignalsB / event  %3.0f", Double_t (fNTotSignalsB)/Double_t (fNEvents)));
	printoutPave -> AddText(Form("Digis / point          %3.2f ",fMeanDigisPoints / Double_t(fNEvents)));
	printoutPave -> AddText(Form("Digis / event         %3.0f ",Double_t (fNTotDigis)/Double_t (fNEvents)));
	printoutPave -> AddText(Form("Points / event       %3.0f ",Double_t (fNTotPoints)/Double_t (fNEvents)));

	digiPad[5] -> Clear();
	printoutPave -> Draw();
	digiPad[5] -> Update();
    }

    for (Int_t iStation = 0; iStation < fNStations; iStation ++){
	CbmStsElement * stat = fSetup -> GetDaughter(iStation);
	Int_t moduleNumber = 0;
	for (Int_t iLad = 0; iLad < stat -> GetNofDaughters(); iLad++) {
	    CbmStsElement* ladd = stat -> GetDaughter(iLad);
	    for (Int_t iHla = 0; iHla < ladd -> GetNofDaughters(); iHla++) {
		CbmStsElement* hlad = ladd -> GetDaughter(iHla);
		for (Int_t iMod = 0; iMod < hlad -> GetNofDaughters(); iMod++) {
		    CbmStsModule* modu = dynamic_cast<CbmStsModule*>(hlad -> GetDaughter(iMod));
		    moduleNumber ++;
		    fhDigisPChannelPModuleAtStation[iStation] -> 
			Fill (moduleNumber, Double_t(modu -> GetNofDigis()) / Double_t(modu -> GetNofChannels()));
		}
	    }
	}
	if ( fOnlineAnalysis ) {
	    occupCanvas -> cd (iStation + 1);
	    gStyle -> SetOptStat("em");
	    fhDigisPChannelPModuleAtStation[iStation] -> Draw("colz");
	    gPad -> Update();
	}
    }
    fTime1 += fTimer.RealTime();
}
// -------------------------------------------------------------------------


// -----   Private method ReInit   -----------------------------------------
InitStatus CbmStsDigitizeQa::ReInit() {

    return kSUCCESS;

}
// -------------------------------------------------------------------------


// -----   Private method Reset   ------------------------------------------
void CbmStsDigitizeQa::Reset() {

    TIter next(fHistoList);
    while ( TH1* histo = ((TH1*)next()) ) histo -> Reset();

}
// -------------------------------------------------------------------------

ClassImp(CbmStsDigitizeQa)

