/** @file CbmStsTimeBasedQaReport.cxx
 ** @author Grigory Kozlov <g.kozlov@gsi.de>
 ** @date 19.04.2016
 **/

#include "CbmStsTimeBasedQaReport.h"
#include "CbmReportElement.h"
#include "CbmHistManager.h"
#include "CbmDrawHist.h"
#include "CbmUtils.h"
#include "TH1.h"
#include "TF1.h"
#include "TProfile.h"
#include "TCanvas.h"
#include "TStyle.h"
#include <boost/assign/list_of.hpp>
#include "TLatex.h"

using std::vector;
using std::endl;
using std::stringstream;
using std::string;
using boost::assign::list_of;
using Cbm::NumberToString;
using Cbm::Split;
using Cbm::FindAndReplace;

CbmStsTimeBasedQaReport::CbmStsTimeBasedQaReport():
    CbmSimulationReport()
  , fDaq(kTRUE)
{
  SetReportName("tb_sts_clustering_qa");
}

CbmStsTimeBasedQaReport::CbmStsTimeBasedQaReport(Bool_t useDaq):
    CbmSimulationReport()
  , fDaq()
{
  fDaq = useDaq;
}

CbmStsTimeBasedQaReport::~CbmStsTimeBasedQaReport()
{
  //...
}

void CbmStsTimeBasedQaReport::Create()
{
  Out().precision(3);
  Out() << R()->DocumentBegin();
  Out() << R()->Title(0, GetTitle());

  Out() << "Number of events: " << HM()->H1("hen_EventNo_TimeBasedQa")->GetEntries() << endl;

  PrintCanvases();

  Out() << R()->DocumentEnd();
}

void CbmStsTimeBasedQaReport::Draw()
{
  string type;
  if ( fDaq )
    type = "TimeSlice";
  else
    type = "Event";
  ScaleHistograms(type);
  DrawNofObjectsHistograms(type);
  DrawResidualAndPullHistograms(type);
  DrawPointsInHitHistograms(type);
  DrawDigiPerObjectHistograms(type);
//  DrawH1ByPattern("hpa_.*");
  DrawHistograms(type);
  Draw2dHistograms(type);
}

void CbmStsTimeBasedQaReport::Draw2dHistograms(const string& type)
{
  string name = "h2d_";
  if ( !HM()->Exists(name + "Residual_X_vs_ClusterSize_" + type) &&
	!HM()->Exists(name + "Residual_X_vs_SlopeX_" + type) &&  
	!HM()->Exists(name + "ClusterSize_vs_SlopeX_" + type) ) return;
vector<string> par = list_of("Residual_X_vs_ClusterSize")("Residual_X_vs_SlopeX")("ClusterSize_vs_SlopeX");
  string canvasName = GetReportName() + "_Residuals_ClusterSize_Slope";
  TCanvas* canvas = CreateCanvas(canvasName.c_str(), canvasName.c_str(), 2100, 500);
  canvas->Divide(3, 1);
  for(Int_t iBin = 0; iBin < 3; iBin++) {
    string histName = name + par[iBin] + "_" + type;
    canvas->cd(iBin + 1);
    TH2* hist = HM()->H2(histName);
    DrawH2(hist, kLinear, kLinear, kLinear);
  }

}
void CbmStsTimeBasedQaReport::DrawNofObjectsHistograms(const string& type)
{
  string name = "hno_NofObjects_";
  if ( !HM()->Exists(name + "Points_" + type) ) return;
  string canvasName = GetReportName() + "_NofObjects_" + type;
  TCanvas* canvas = CreateCanvas(canvasName.c_str(), canvasName.c_str(), 800, 500);
  canvas->SetGrid();
  canvas->cd();
  vector<string> labels = list_of("Points")("Digis")("Clusters")("Hits")("MatchedHits");
  vector<TH1*> histos = list_of(HM()->H1(name + "Points_" + type))
		  (HM()->H1(name + "Digis_" + type))(HM()->H1(name + "Clusters_" + type))
		  (HM()->H1(name + "Hits_" + type))(HM()->H1(name + "MatchedHits_" + type));
  DrawH1(histos, labels, kLinear, kLinear, true, 0.65, 0.65, 0.9, 0.9);

  vector<TH1*> histos1 = list_of(HM()->H1(name + "Points_Station_" + type))
		  (HM()->H1(name + "Digis_Station_" + type))(HM()->H1(name + "Clusters_Station_" + type))
		  (HM()->H1(name + "Hits_Station_" + type))(HM()->H1(name + "MatchedHits_Station_" + type));
  canvasName = GetReportName() + "_NofObjects_Station_" + type;
  TCanvas* canvas1 = CreateCanvas(canvasName.c_str(), canvasName.c_str(), 800, 500);
  canvas1->SetGrid();
  canvas1->cd();
  DrawH1(histos1, labels, kLinear, kLinear, true, 0.65, 0.65, 0.9, 0.9);

//---
  if ( HM()->H1("hce_PointsInCells")->Integral() ){
    HM()->H1("hce_EffInCells")->Divide(HM()->H1("hce_HitsInCells"), HM()->H1("hce_PointsInCells"), 1., 1., "B");
    DrawH1ByPattern("hce_EffInCells");
  }
//---
}

void CbmStsTimeBasedQaReport::DrawResidualAndPullHistograms(const string& type)
{
  if ( !(HM()->Exists("hrp_Residual_X_" + type) && HM()->Exists("hrp_Residual_Y_" + type)
		  && HM()->Exists("hrp_Pull_X_" +type) && HM()->Exists("hrp_Pull_Y_" + type)) ) return;
  vector<string> par = list_of("Residual_X")("Residual_Y")("Pull_X")("Pull_Y");
  string canvasName = GetReportName() + "_Residuals_and_Pulls";
  TCanvas* canvas = CreateCanvas(canvasName.c_str(), canvasName.c_str(), 1000, 1000);
  canvas->Divide(2, 2);
  for(Int_t iBin = 0; iBin < 4; iBin++) {
    string histName = "hrp_" + par[iBin] + "_" + type;
    canvas->cd(iBin + 1);
    TH1* hist = HM()->H1(histName);
    DrawH1(hist);
    hist->Scale(1. / hist->Integral());
//    DrawH1andFitGauss(HM()->H1(histName));
    hist->Fit("gaus", "Q");
    TF1* func = hist->GetFunction("gaus");
    if( func == 0 ) return;
    func->SetLineColor(kBlack);
    Double_t m = func->GetParameter(1);
    Double_t s = func->GetParameter(2);
    string txt1 = "Mean: " + Cbm::NumberToString<Double_t>(m, 2);
    string txt2 = "Sigma: " + Cbm::NumberToString<Double_t>(s, 2);
    TLatex text;
    text.SetTextAlign(70);
    text.SetTextSize(0.05);
    text.DrawTextNDC(0.6, 0.8, txt1.c_str());
    text.DrawTextNDC(0.6, 0.73, txt2.c_str());
  }
}

void CbmStsTimeBasedQaReport::DrawPointsInHitHistograms(const string& type)
{
  if ( !HM()->Exists("hhp_PointsInHit_" + type) && !HM()->Exists("hhp_PointsInMatchedHit_" + type) ) return;
  string canvasName = GetReportName() + "_PointsInHit_" + type;
  TCanvas* canvas = CreateCanvas(canvasName.c_str(), canvasName.c_str(), 800, 500);
  canvas->SetGrid();
  canvas->cd();
  vector<string> labels = list_of("PointsInHit")("PointsInMatchedHit");
  vector<TH1*> histos = list_of(HM()->H1("hhp_PointsInHit_" + type))
		  (HM()->H1("hhp_PointsInMatchedHit_" + type));
  DrawH1(histos, labels, kLinear, kLog, true, 0.6, 0.7, 0.9, 0.9);
}

void CbmStsTimeBasedQaReport::DrawDigiPerObjectHistograms(const string& type)
{
  if ( !HM()->Exists("hdo_DigisInCluster_" + type) && !HM()->Exists("hdo_DigisByPoint_" + type) ) return;
  string canvasName = GetReportName() + "_DigisPerObject_" + type;
  TCanvas* canvas = CreateCanvas(canvasName.c_str(), canvasName.c_str(), 800, 500);
  canvas->SetGrid();
  canvas->cd();
  vector<string> labels = list_of("DigisInCluster")("DigisByPoint");
  vector<TH1*> histos = list_of(HM()->H1("hdo_DigisInCluster_" + type))
		  (HM()->H1("hdo_DigisByPoint_" + type));
  DrawH1(histos, labels, kLinear, kLog, true, 0.65, 0.7, 0.9, 0.9);
}

void CbmStsTimeBasedQaReport::ScaleHistograms(const string& /*type*/)
{
  Int_t nofEvents = HM()->H1("hen_EventNo_TimeBasedQa")->GetEntries();
  if ( nofEvents == 0 ) nofEvents = 1;

  HM()->ScaleByPattern("hno_NofObjects_.*_Station_.*", 1. / nofEvents);
  HM()->ShrinkEmptyBinsH1ByPattern("hno_NofObjects_.*_Station_.*");
}

void CbmStsTimeBasedQaReport::DrawHistograms(const string& type)
{
  string title = "PointsInDigi";
  string canvasName = GetReportName() + "_" + title + "_" + type;
  TCanvas* canvas1 = CreateCanvas(canvasName.c_str(), canvasName.c_str(), 800, 500);
  canvas1->SetGrid();
  canvas1->cd();
  vector<TH1*> histos1;
  vector<string> labels;
  histos1.push_back(HM()->H1("hpa_PointsInDigi_" + type));
  labels.push_back(title);
  DrawH1(histos1, labels, kLinear, kLog, true, 0.65, 0.75, 0.9, 0.9);
  title = "PointsInCluster";
  canvasName = GetReportName() + "_" + title + "_" + type;
  TCanvas* canvas2 = CreateCanvas(canvasName.c_str(), canvasName.c_str(), 800, 500);
  canvas2->SetGrid();
  canvas2->cd();
  histos1.clear();
  histos1.push_back(HM()->H1("hpa_PointsInCluster_" + type));
  labels.clear();
  labels.push_back(title);
  DrawH1(histos1, labels, kLinear, kLog, true, 0.65, 0.75, 0.9, 0.9);
  title = "ClusterSize";
  canvasName = GetReportName() + "_" + title + "_" + type;
  TCanvas* canvas3 = CreateCanvas(canvasName.c_str(), canvasName.c_str(), 800, 500);
  canvas3->SetGrid();
  canvas3->cd();
  histos1.clear();
  histos1.push_back(HM()->H1("hpa_ClusterSize_" + type));
  labels.clear();
  labels.push_back(title);
  DrawH1(histos1, labels, kLinear, kLog, true, 0.65, 0.75, 0.9, 0.9);
}

ClassImp(CbmStsTimeBasedQaReport)
