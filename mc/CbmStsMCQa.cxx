#include "CbmStsMCQa.h"

#include "CbmHistManager.h"
#include "CbmStsAddress.h"
#include "CbmStsModule.h"
#include "CbmStsElement.h"
#include "CbmStsSetup.h"
#include "CbmStsPoint.h"
#include "CbmSimulationReport.h"
//#include "CbmStsMCQaReport.h"

#include "FairRootManager.h"
#include "FairLogger.h"

#include "TClonesArray.h"
#include "TH1.h"
#include "TF1.h"
#include "TH1D.h"
#include "TH2.h"
#include "TProfile.h"
#include "TProfile2D.h"

using std::vector;
using std::map;

CbmStsMCQa::CbmStsMCQa():
  FairTask()
  , fHM(new CbmHistManager())
  , fStsPoints(NULL)
  , fMCTracks(NULL)
  , fSetup(NULL)
  , fNofStation(8)
{
}

CbmStsMCQa::~CbmStsMCQa()
{
  if ( fHM ) delete fHM;
}

InitStatus CbmStsMCQa::Init()
{
  fSetup = CbmStsSetup::Instance();
  fNofStation = fSetup -> GetNofStations();
  LOG(info) << "Sts Setup consist of " << fNofStation << " stations.";
    
  ReadDataBranches();
  CreateHistograms();
  return kSUCCESS;
}

void CbmStsMCQa::ReadDataBranches()
{  
  FairRootManager* ioman = FairRootManager::Instance();
  if ( NULL == ioman )
    LOG(fatal) << "No FairRootManager!";
  
  fStsPoints = dynamic_cast<TClonesArray*>(ioman -> GetObject("StsPoint"));
  if ( NULL == fStsPoints )
    LOG(error) << "No StsPoint array!";

  fMCTracks = dynamic_cast<TClonesArray*>(ioman -> GetObject("MCTrack"));
  if ( NULL == fMCTracks )
    LOG(error) << "No MCTrack array!";
}

void CbmStsMCQa::CreateHistograms()
{
  CreateNofObjectsHistograms();
  CreatePointHistograms();
  fHM -> Create1<TH1F>("h_sts_EventNo_MCQa", "h_stsEventNo_MCQa", 1, 0, 1.);
}

void CbmStsMCQa::CreateNofObjectsHistograms()
{

  Int_t nofBins = 100;
  Double_t minX = -0.5;
  Double_t maxX = 99.5;
  string name = "h_sts_NofObjects_";
  fHM -> Create1<TH1F>(name + "Points", name + "Points;Objects per event;Entries", nofBins, minX, maxX);
  
  nofBins = fNofStation;
  minX = -0.5;
  maxX = static_cast<Float_t>(fNofStation)-0.5;
  fHM -> Create1<TH1F>(name + "Points_Station", name + "Points_Station;Station number;Objects per event", nofBins, minX, maxX);
}

void CbmStsMCQa::CreatePointHistograms()
{
  for (Int_t stationId = 0; stationId < fNofStation; stationId++){
    Int_t nofBins = 100;
    Double_t minX = -0.5;
    Double_t maxX = 99.5;
    fHM -> Create1<TH1F>(Form("h_sts_MultPoints_Station%i",stationId),
			 Form("Mult, Station %i;Objects per event;Entries", stationId),
			 nofBins, minX, maxX);

    fHM -> Create2<TH2F>(Form("h_sts_PointsMap_Station%i",stationId),
       Form("StsPoint, Station %i;x, cm;y, cm", stationId),
       200, -10., 10., 200, -10., 10.);
    fHM -> Create2<TH2F>(Form("h_sts_PointsMap_NoOverlap_Station%i",stationId),
       Form("StsPoint, Station %i;x, cm;y, cm", stationId),
       200, -10., 10., 200, -10., 10.);

    fHM -> Create2<TH2F>(Form("h_sts_PointsMapEvent_Station%i",stationId),
       Form("StsPoint/cm^{2}, Station %i;x, cm;y, cm", stationId),
       200, -10., 10., 200, -10., 10.);
    fHM -> Create2<TH2F>(Form("h_sts_PointsMapEvent_NoOverlap_Station%i",stationId),
       Form("StsPoint/cm^{2}, Station %i;x, cm;y, cm", stationId),
       200, -10., 10., 200, -10., 10.);

    fHM -> Create2<TH2F>(Form("h_sts_PointsMapRate_Station%i",stationId),
       Form("StsPoint/cm^{2}/s, Station %i;x, cm;y, cm", stationId),
       200, -10., 10., 200, -10., 10.);
    fHM -> Create2<TH2F>(Form("h_sts_PointsMapRate_NoOverlap_Station%i",stationId),
       Form("StsPoint/cm^{2}/s, Station %i;x, cm;y, cm", stationId),
       200, -10., 10., 200, -10., 10.);
    fHM -> Create1<TH1F>(Form("h_sts_XPos_Station%i",stationId),"X position;x, cm; Entries", 200, -10., 10.);
    fHM -> Create1<TH1F>(Form("h_sts_YPos_Station%i",stationId),"Y position;y, cm; Entries", 200, -10., 10.);


  }
  fHM -> Create1<TH1F>("h_sts_XMom","momentum px; GeV/c; Entries", 100, -5., 5.);
  fHM -> Create1<TH1F>("h_sts_YMom","momentum py; GeV/c; Entries", 100, -5., 5.);
  fHM -> Create1<TH1F>("h_sts_ZMom","momentum pz; GeV/c; Entries", 500, -10., 40.);
  fHM -> Create1<TH1F>("h_sts_ELoss","energy loss; ; Entries", 100, 0., 0.02);

  fHM -> Create1<TH1F>("h_sts_XPos","X position;x, cm; Entries", 200, -10., 10.);
  fHM -> Create1<TH1F>("h_sts_YPos","Y position;y, cm; Entries", 200, -10., 10.);
}

void CbmStsMCQa::Exec(Option_t*){
  ProcessPoints(fStsPoints);
  fHM -> H1("h_sts_EventNo_MCQa") -> Fill(0.5);
}

void CbmStsMCQa::ProcessPoints(const TClonesArray * points)
{
  
  fHM -> H1("h_sts_NofObjects_Points") -> Fill(points -> GetEntriesFast());
  
  Double_t pointX=0.;
  Double_t pointY=0.;

  Double_t pX=0.;
  Double_t pY=0.;
  Double_t pZ=0.;

  std::map<Int_t, vector<Int_t>> used_map = { {0, {}}, {1, {}} };

  for(Int_t iPoint = 0; iPoint < points -> GetEntriesFast(); iPoint++) {
    const CbmStsPoint* stsPoint = static_cast<const CbmStsPoint*>(points -> At(iPoint));
    Int_t stationId = fSetup->GetStationNumber(stsPoint->GetDetectorID());
    fHM -> H1("h_sts_NofObjects_Points_Station") -> Fill(stationId);
    
    pointX = stsPoint -> GetXIn();
    pointY = stsPoint -> GetYIn();

    pX = stsPoint -> GetPx();
    pY = stsPoint -> GetPy();
    pZ = stsPoint -> GetPz();

    fHM -> H1(Form("h_sts_XPos_Station%i", stationId)) -> Fill(pointX);
    fHM -> H1(Form("h_sts_YPos_Station%i", stationId)) -> Fill(pointY);

    fHM -> H2(Form("h_sts_PointsMap_Station%i", stationId)) -> Fill(pointX, pointY);
    fHM -> H2(Form("h_sts_PointsMapEvent_Station%i", stationId)) -> Fill(pointX, pointY);
    fHM -> H2(Form("h_sts_PointsMapRate_Station%i", stationId)) -> Fill(pointX, pointY);
    fHM -> H1("h_sts_XPos") -> Fill(pointX);
    fHM -> H1("h_sts_YPos") -> Fill(pointY);
    fHM -> H1("h_sts_XMom") -> Fill(pX);
    fHM -> H1("h_sts_YMom") -> Fill(pY);
    fHM -> H1("h_sts_ZMom") -> Fill(pZ);
    fHM -> H1("h_sts_ELoss") -> Fill(stsPoint -> GetEnergyLoss());
    
    Int_t mcTrackID = stsPoint -> GetTrackID();
 
    if (std::find(used_map[stationId].begin(), used_map[stationId].end(), mcTrackID) == used_map[stationId].end()) {
      used_map[stationId].push_back(mcTrackID);
      fHM -> H2(Form("h_sts_PointsMap_NoOverlap_Station%i", stationId)) -> Fill(pointX, pointY);
      fHM -> H2(Form("h_sts_PointsMapEvent_NoOverlap_Station%i", stationId)) -> Fill(pointX, pointY);
      fHM -> H2(Form("h_sts_PointsMapRate_NoOverlap_Station%i", stationId)) -> Fill(pointX, pointY);
    }    
  }
  fHM -> H1(Form("h_sts_MultPoints_Station%i",0)) -> Fill(used_map[0].size());
  fHM -> H1(Form("h_sts_MultPoints_Station%i",1)) -> Fill(used_map[1].size());

}

void CbmStsMCQa::Finish(){

  Int_t nofEvents = fHM -> H1("h_sts_EventNo_MCQa") -> GetEntries();

  // Do here some scaling of the histograms to have MCPoint per cm^2

  Int_t xbins = (fHM->H2("h_sts_PointsMap_Station0"))->GetXaxis()->GetNbins();
  Float_t xmax = fHM -> H2("h_sts_PointsMapEvent_Station0") -> GetXaxis()->GetXmax();
  Float_t xmin = fHM -> H2("h_sts_PointsMapEvent_Station0") -> GetXaxis()->GetXmin();
  Float_t scaleX = static_cast<Float_t>(xbins)/(xmax - xmin);

  LOG(info) << "scaleX: " << scaleX;

  Int_t ybins = fHM -> H2("h_sts_PointsMapEvent_Station0") -> GetYaxis()->GetNbins();
  Int_t ymax = fHM -> H2("h_sts_PointsMapEvent_Station0") -> GetYaxis()->GetXmax();
  Int_t ymin = fHM -> H2("h_sts_PointsMapEvent_Station0") -> GetYaxis()->GetXmin();
  Float_t scaleY = static_cast<Float_t>(ybins)/(ymax - ymin);

  LOG(info) << "scaleY: " << scaleY;

  Float_t scale = scaleX * scaleY;
  scale=1.;

  LOG(info) << "Scale factor to cm^2: " << scale;

  for(Int_t i=0; i<fNofStation; ++i) {
    fHM -> Scale(Form("h_sts_PointsMapEvent_Station%i", i),scale/nofEvents);
    fHM -> Scale(Form("h_sts_PointsMapEvent_NoOverlap_Station%i", i),scale/nofEvents);
    fHM -> Scale(Form("h_sts_PointsMapRate_Station%i", i),10000000.*scale/nofEvents);
    fHM -> Scale(Form("h_sts_PointsMapRate_NoOverlap_Station%i", i),10000000*scale/nofEvents);
  }

  gDirectory -> mkdir("QA/StsMCQa");
  gDirectory -> cd("QA/StsMCQa");
  fHM -> WriteToFile();
  gDirectory -> cd("../..");
  //    CbmSimulationReport* report = new CbmStsMCQaReport(fSetup, fDigitizer);
  //    report -> Create(fHM, fOutputDir);
  //    delete report;
  
  // Compare results with defined benchmark results and raise an ERROR if there are differences
  // Either get default histogramms from a benchmark qa file or as parameters from the parameter container
}

ClassImp(CbmStsMCQa);
