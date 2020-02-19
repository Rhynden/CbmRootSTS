/** @file CbmStsDigisToHitsModule.cxx
 **/

#include "CbmStsDigisToHitsModule.h"
#include <tuple>

#include <cassert>
#include "TClonesArray.h"
#include "FairLogger.h"
#include "CbmStsAddress.h"
#include "CbmStsCluster.h"
#include "CbmStsClusterAnalysis.h"



ClassImp(CbmStsDigisToHitsModule)


// -----   Default constructor   -------------------------------------------
CbmStsDigisToHitsModule::CbmStsDigisToHitsModule() 
  : TNamed("CbmStsDigisToHitsModule", "CbmStsDigisToHitsModule")
  , fSize(0)
  , fTimeCutDigisInSigma(3.)
  , fTimeCutDigisInNs(-1.)
  , fTimeCutClustersInNs(-1.)
  , fTimeCutClustersInSigma(4.)
  , fConnectEdgeFront(kFALSE)
  , fConnectEdgeBack(kFALSE)
  , fModule(nullptr)
  , fClusters(nullptr)
  , fIndex()
  , fTime()
  , fDigiQueue()
  , moduleNumber()
  , fAna(nullptr)
  , fClusterOutput(new TClonesArray("CbmStsCluster", 6e3))
  , fHitOutput(new TClonesArray("CbmStsHit", 6e3))  
{
  fDigiQueue.reserve(60000);
}
// -------------------------------------------------------------------------



// -----   Constructor with parameters   -----------------------------------
CbmStsDigisToHitsModule::CbmStsDigisToHitsModule(UShort_t nChannels,
                                                     Double_t timeCutDigisInNs,
                                                     Double_t timeCutDigisInSigma,
                                                     Double_t timeCutClustersInNs,
                                                     Double_t timeCutClustersInSigma,
                                                     const char* name,
                                                     CbmStsModule* module,
                                                     Int_t mNumber,
                                                     CbmStsClusterAnalysis* clusterAna) 
  : TNamed(name, "CbmStsDigisToHitsModule")
  , fSize(nChannels)
  , fTimeCutDigisInSigma(timeCutDigisInSigma)
  , fTimeCutDigisInNs(timeCutDigisInNs)
  , fTimeCutClustersInNs(timeCutClustersInNs)
  , fTimeCutClustersInSigma(timeCutClustersInSigma)
  , fConnectEdgeFront(kFALSE)
  , fConnectEdgeBack(kFALSE)
  , fModule(module)
  , fClusters(nullptr)
  , fIndex(fSize)
  , fTime(fSize)
  , fDigiQueue()
  , moduleNumber(mNumber)
  , fAna(clusterAna)
  , fClusterOutput(new TClonesArray("CbmStsCluster", 6e3))
  , fHitOutput(new TClonesArray("CbmStsHit", 6e3))  
{
  fDigiQueue.reserve(60000);
  fHitOutputVector.reserve(100000);
}
// -------------------------------------------------------------------------



// -----   Destructor   ----------------------------------------------------
CbmStsDigisToHitsModule::~CbmStsDigisToHitsModule() {
}
// -------------------------------------------------------------------------



// ----- Search for a matching cluster for a given channel   ---------------
Bool_t CbmStsDigisToHitsModule::CheckChannel(UShort_t channel,
                                               Double_t time) {

  // Check channel number
  assert( channel < fSize );

  // No match if no active digi in the channel
  if ( fIndex[channel] == -1 ) return kFALSE;

  assert( time >= fTime[channel] );

  Double_t deltaT = fTimeCutDigisInSigma * TMath::Sqrt(2.) * fModule->GetAsicParameters(channel).GetTimeResolution();
  if ( fTimeCutDigisInNs > 0. ) deltaT = fTimeCutDigisInNs;

  // Channel is active, but time is not matching: close cluster
  // and return no match.
  if ( time - fTime[channel] > deltaT ) {
    FinishCluster(channel);
    return kFALSE;
  }

  // Matching digi found
  return kTRUE;
}
// -------------------------------------------------------------------------



// -----   Create a cluster object   ---------------------------------------
void CbmStsDigisToHitsModule::CreateCluster(UShort_t first,
                                              UShort_t last) {

  // --- Create cluster object; if possible, in the output array
  CbmStsCluster* cluster = NULL;
  /*if ( fClusters ) {
    Int_t index = fClusters->GetEntriesFast();
    cluster = new ((*fClusters)[index]) CbmStsCluster();
  } //? cluster array
  else cluster = new CbmStsCluster(); */
  //DigisToHits
  Int_t index = fClusterOutput->GetEntriesFast();
  cluster = new ((*fClusterOutput)[index]) CbmStsCluster();

  cluster->SetIndex(index);

  // Register cluster in module
  fModule->AddCluster(cluster);

  // --- Add digis to cluster and reset the respective channel
  UShort_t channel = first;
  while ( kTRUE ) {
    assert( fIndex[channel] > - 1 );
    cluster->AddDigi(fIndex[channel]);
    fIndex[channel] = -1;
    fTime[channel] = 0.;
    if ( channel == last ) break;
    channel++;
    if ( last < first && channel == fSize/2 ) channel = 0; // round the edge, front side
    if ( last < first && channel == fSize ) channel = fSize/2; // round the edge, back side
  }

  if ( fModule ) cluster->SetAddress(fModule->GetAddress());

  // --- Delete cluster object if no output array is there
  //if ( ! fClusters ) delete cluster;

  // Analyse cluster
  //LOG(INFO) << "Analysing cluster";
  fAna->Analyze(cluster, fModule);
}
// -------------------------------------------------------------------------



// -----   Close a cluster   -----------------------------------------------
void CbmStsDigisToHitsModule::FinishCluster(UShort_t channel) {

  // Find start and stop channel of cluster
  UShort_t start = channel;
  UShort_t stop = channel;
  UShort_t testChannel;

  // Case: front-side channel
  if ( channel < fSize/2 ) {

    // Normal clustering
    if ( ! fConnectEdgeFront ) {
      while ( start > 0 && fIndex[start-1] > -1 ) start--;
      while ( stop < fSize/2 - 1 && fIndex[stop+1] > -1 ) stop++;
    } //? normal clustering

    // Clustering round-the-edge
    else {
      testChannel = ( channel ? channel - 1 : fSize/2 - 1);
      while ( fIndex[testChannel] > -1 ) {
        start = testChannel;
        testChannel = ( start ? start - 1 : fSize/2 - 1);
      }
      testChannel = ( channel == fSize/2 - 1 ? 0 : channel + 1);
      while ( fIndex[testChannel] > -1 ) {
        stop = testChannel;
        testChannel = ( stop == fSize/2 - 1 ? 0 : stop + 1);
      }
    } //? clustering round the edge

  } //? Front-side channel


  // Case: back-side channel
  else  {

    // Normal clustering
    if ( ! fConnectEdgeBack ) {
      while ( start > fSize/2 && fIndex[start-1] > -1 ) start--;
      while ( stop < fSize - 1 && fIndex[stop+1] > -1 ) stop++;
    }

    // Clustering round-the-edge
    else {
      testChannel = ( channel == fSize/2 ? fSize - 1 : channel - 1);
      while ( fIndex[testChannel] > -1 ) {
        start = testChannel;
        testChannel = ( start == fSize/2 ? fSize - 1 : start - 1);
      }
      testChannel = ( channel == fSize - 1 ? fSize/2 : channel + 1);
      while ( fIndex[testChannel] > -1 ) {
        stop = testChannel;
        testChannel = ( stop == fSize - 1 ? fSize/2 : stop + 1);
      }
    } //? clustering round the edge

  }//? back-side channel

  // Create a cluster object
  CreateCluster(start, stop);

  // Reset channels added to the cluster
  for (UShort_t iChannel = start; iChannel <= stop; iChannel++) {
    assert( iChannel <= fSize );
    fIndex[iChannel] = -1;
    fTime[iChannel] = 0.;
  }

}
// -------------------------------------------------------------------------



// -----   Process active clusters   ---------------------------------------
void CbmStsDigisToHitsModule::ProcessBuffer() {

  for (UShort_t channel = 0; channel < fSize; channel++) {
    if ( fIndex[channel] == - 1 ) continue;
    FinishCluster(channel);
  }

}
// -------------------------------------------------------------------------


// --------------- Add single digi to buffer ------------------------------
void CbmStsDigisToHitsModule::AddDigiToQueue(const CbmStsDigi* digi, Int_t digiIndex) {

  lock.lock();

  fDigiQueue.push_back(std::make_tuple(digi, digiIndex));
  //fDigiIndex.push_back(digiIndex);


  lock.unlock();
}
// -------------------------------------------------------------------------


// -----   Process all digis of module   -----------------------------------
void CbmStsDigisToHitsModule::ProcessDigis(CbmEvent* event) {

  // Sort the Digi Buffer by time
  //LOG(INFO) << "Sorting digiQueue" << FairLogger::endl;
  std::sort(fDigiQueue.begin(), fDigiQueue.end(), [] (std::tuple<const CbmStsDigi*, Int_t> digi1, std::tuple<const CbmStsDigi*, Int_t> digi2) {return std::get<1>(digi1) < std::get<1>(digi2);});

  //Process each individual digi
  //LOG(INFO) << "Processing individual digis" << FairLogger::endl;
  for (UInt_t iDigi = 0; iDigi < fDigiQueue.size(); iDigi++){
    ProcessDigi(std::get<0>(fDigiQueue[iDigi])->GetChannel(), std::get<0>(fDigiQueue[iDigi])->GetTime(), std::get<1>(fDigiQueue[iDigi]));
  }

  //LOG(INFO) << "Processing remaining digis" << FairLogger::endl;
  // Process Remaining Digis
  for (UShort_t channel = 0; channel < fSize; channel++) {
    if ( fIndex[channel] == - 1 ) continue;
    FinishCluster(channel);
  }

  //LOG(INFO) << "Sorting Cluster in Modules" << FairLogger::endl;
  // Process Clusters to Hits
  
  fModule->SortClustersByTime();
  //LOG(INFO) << "Calculating hits from clusters" << FairLogger::endl;
  // Int_t clusters = fModule->GetNofClsters();
  //LOG(INFO) << "Clusters in Module" << moduleNumber << " is " << fModule->GetClusters().size();
  //LOG(INFO) << "CutInNs = " << fTimeCutClustersInNs << " CutInSigma = " << fTimeCutClustersInSigma;
  LOG(DEBUG) << "Processing module number " << fModule;
  //  Int_t nModuleHits = fModule->FindHits(fHitOutput, event, fTimeCutClustersInNs, fTimeCutClustersInSigma);
  //fModule->FindHits(fHitOutput, event, fTimeCutClustersInNs, fTimeCutClustersInSigma);
  fModule->FindHitsVector(&fHitOutputVector, event, fTimeCutClustersInNs, fTimeCutClustersInSigma);
  //fHitOutput->AbsorbObjects(Convert2(fHitOutputVector));

  //return fDigiQueue.size(); 
  //return fClusterOutput->GetEntriesFast();
  //return fHitOutput->GetEntriesFast();
  //LOG(INFO) << "nModule Hits = " << nModuleHits;
  //return nModuleHits;
//  return fHitOutput;
}
// -------------------------------------------------------------------------


// ----- Process an input digi   -------------------------------------------
Bool_t CbmStsDigisToHitsModule::ProcessDigi(UShort_t channel, Double_t time,
                                              Int_t index) {

  // Assert channel number
  assert ( channel < fSize );

  // Check for matching digi in the same channel (can only happen if
  // time resolution is not much smaller than the dead time.)
  // In this case, the digi is ignored.
  if ( CheckChannel(channel, time) ) {
  	/*
    LOG(warn) << GetName() << ": Channel " << channel << " time "
        << fTime[channel] << " ns, new digi at t = " << time
        << " ns is within dead time! (delta(T) = " << time - fTime[channel]
				<< ")";
				*/
    return kFALSE;
    //FinishCluster(channel);
  }

  // Check for matching digi in the left neighbour channel
  if ( channel != 0 && channel != fSize/2 ) CheckChannel(channel-1, time);

  // Check for matching digi in the right neighbour channel
  if ( channel != fSize/2 - 1 && channel != fSize -1 )
    CheckChannel(channel+1, time);

  // Set channel active
  fIndex[channel] = index;
  fTime[channel] = time;

  return kTRUE;
}
// -------------------------------------------------------------------------



// -----   Reset the channel vectors   -------------------------------------
void CbmStsDigisToHitsModule::Reset() {

  fIndex.assign(fSize, -1);
  fTime.assign(fSize, 0.);

  //DigisToHits
  fModule->ClearClusters();
  fDigiQueue.clear();
  fHitOutput->Clear();
  fClusterOutput->Clear();
}
// -------------------------------------------------------------------------
