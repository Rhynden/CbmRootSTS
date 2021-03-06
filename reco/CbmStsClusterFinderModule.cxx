/** @file CbmStsClusterFindeModule.cxx
 ** @author Volker Friese <v.friese@gsi.de>
 ** @date 05.04.20174
 **/

#include "CbmStsClusterFinderModule.h"

#include <cassert>
#include "TClonesArray.h"
#include "FairLogger.h"
#include "CbmStsAddress.h"
#include "CbmStsCluster.h"



ClassImp(CbmStsClusterFinderModule)


// -----   Default constructor   -------------------------------------------
CbmStsClusterFinderModule::CbmStsClusterFinderModule() :
  TNamed(), fSize(0), fTimeCutInSigma(3.), fTimeCut(-1.),
  fConnectEdgeFront(kFALSE), fConnectEdgeBack(kFALSE),
  fModule(NULL), fClusters(NULL), fIndex(), fTime()
{
}
// -------------------------------------------------------------------------



// -----   Constructor with parameters   -----------------------------------
CbmStsClusterFinderModule::CbmStsClusterFinderModule(UShort_t nChannels,
                                                     Double_t timeCut,
                                                     Double_t timeCutInSigma,
                                                     const char* name,
                                                     CbmStsModule* module,
                                                     TClonesArray* output) :
  TNamed(name, "cluster finder module"),
  fSize(nChannels),
  fTimeCutInSigma(timeCutInSigma),
  fTimeCut(timeCut),
  fConnectEdgeFront(kFALSE),
  fConnectEdgeBack(kFALSE),
  fModule(module),
  fClusters(output),
  fIndex(fSize),
  fTime(fSize)
{
}
// -------------------------------------------------------------------------



// -----   Destructor   ----------------------------------------------------
CbmStsClusterFinderModule::~CbmStsClusterFinderModule() {
}
// -------------------------------------------------------------------------



// ----- Search for a matching cluster for a given channel   ---------------
Bool_t CbmStsClusterFinderModule::CheckChannel(UShort_t channel,
                                               Double_t time) {

  // Check channel number
  assert( channel < fSize );

  // No match if no active digi in the channel
  if ( fIndex[channel] == -1 ) return kFALSE;

  assert( time >= fTime[channel] );

  Double_t deltaT = fTimeCutInSigma * TMath::Sqrt(2.) * fModule->GetAsicParameters(channel).GetTimeResolution();
  if ( fTimeCut > 0. ) deltaT = fTimeCut;

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
void CbmStsClusterFinderModule::CreateCluster(UShort_t first,
                                              UShort_t last) {

  // --- Create cluster object; if possible, in the output array
  CbmStsCluster* cluster = NULL;
  if ( fClusters ) {
    Int_t index = fClusters->GetEntriesFast();
    cluster = new ((*fClusters)[index]) CbmStsCluster();
  } //? cluster array
  else cluster = new CbmStsCluster();

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
  if ( ! fClusters ) delete cluster;

}
// -------------------------------------------------------------------------



// -----   Close a cluster   -----------------------------------------------
void CbmStsClusterFinderModule::FinishCluster(UShort_t channel) {

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
void CbmStsClusterFinderModule::ProcessBuffer() {

  for (UShort_t channel = 0; channel < fSize; channel++) {
    if ( fIndex[channel] == - 1 ) continue;
    FinishCluster(channel);
  }

}
// -------------------------------------------------------------------------



// ----- Process an input digi   -------------------------------------------
Bool_t CbmStsClusterFinderModule::ProcessDigi(UShort_t channel, Double_t time,
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
void CbmStsClusterFinderModule::Reset() {

  fIndex.assign(fSize, -1);
  fTime.assign(fSize, 0.);

}
// -------------------------------------------------------------------------
