#ifdef __CINT__

#pragma link off all globals;
#pragma link off all classes;
#pragma link off all functions;


// Setup
//#pragma link C++ class CbmStsAddress;
#pragma link C++ class CbmStsElement;
#pragma link C++ class CbmStsModule;
#pragma link C++ class CbmStsSensor;
#pragma link C++ class CbmStsSensorConditions;
#pragma link C++ class CbmStsSensorPoint;
#pragma link C++ class CbmStsSetup;
#pragma link C++ class CbmStsStation;
#pragma link C++ class CbmHodoSetup;

// Monte-Carlo
#pragma link C++ class CbmStsAcceptance;
#pragma link C++ class CbmStsMC;
#pragma link C++ class CbmStsSensorFactory;
#pragma link C++ class CbmStsTrackStatus;
#pragma link C++ class CbmStsMCQa;

// Digitisation
#pragma link C++ class CbmDigitize<CbmStsDigi>+;
#pragma link C++ class CbmStsDigitize+;
#pragma link C++ class CbmStsDigitizeParameters+;
#pragma link C++ class CbmStsPhysics;
#pragma link C++ class CbmStsSensorDssd;
#pragma link C++ class CbmStsSensorDssdOrtho;
#pragma link C++ class CbmStsSensorDssdStereo;
#pragma link C++ class CbmStsSignal;

// Reconstruction
#pragma link C++ class CbmStsClusterAnalysis;
#pragma link C++ class CbmStsClusterFinderModule;
#pragma link C++ class CbmStsFindClusters;
#pragma link C++ class CbmStsFindHits;
#pragma link C++ class CbmStsDigisToHitsModule;
#pragma link C++ class CbmStsDigisToHits;
#pragma link C++ class CbmStsFindHitsSingleCluster;
#pragma link C++ class CbmStsFindTracksEvents;
#pragma link C++ class CbmStsMatchReco;
#pragma link C++ class CbmStsReco+;
#pragma link C++ class CbmStsRecoQa;
#pragma link C++ class CbmStsTestQa;

// Analysis
#pragma link C++ class CbmStsWkn+;

// Others
#pragma link C++ class CbmStsContFact;
#pragma link C++ class CbmStsFindTracks+;
#pragma link C++ class CbmStsFindTracksQa+;
#pragma link C++ class CbmStsFitTracks+;
#pragma link C++ class CbmStsHitProducerIdeal+;
//#pragma link C++ class CbmStsIdealMatchHits+;
//#pragma link C++ class CbmStsMatchHits+;
#pragma link C++ class CbmStsMatchTracks+;
#pragma link C++ class CbmStsRadTool;
#pragma link C++ class CbmStsTrackFinderIdeal+;
#pragma link C++ class CbmStsTrackFitterIdeal+;
#pragma link C++ class CbmStsSimulationQa+;
//#pragma link C++ class CbmStsReconstructionQa+;
#pragma link C++ class CbmStsDigitizeQa+;
#pragma link C++ class CbmStsDigitizeQaReport+;
//#pragma link C++ class CbmStsTimeBasedQa;
//#pragma link C++ class CbmStsTimeBasedQaReport;



#endif /* __CINT__ */

