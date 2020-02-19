/** @file CbmStsDigisToHitsModule.h
 **/

#ifndef CBMSTSDIGISTOHITSMODULE_H
#define CBMSTSDIGISTOHITSMODULE_H 1

#include <vector>
#include <mutex>
#include "TNamed.h"
#include "CbmStsModule.h"
#include "CbmStsHit.h"
#include "TClonesArray.h"

class TClonesArray;
class CbmStsClusterAnalysis;

/** @class CbmStsDigisToHitsModule
 ** @brief Class for finding clusters in one STS module
 ** @developed for (parallel) processing of clusters per module
 ** @version 1.0
 **
 ** A cluster is defined by a group of matching digis. Two digis
 ** are considered matching if they are in neighbouring channels
 ** and their time difference is less than the chosen limit.
 **
 ** Clustering is performed in a streaming way, i.e. on adding
 ** of each digi by the method ProcessDigi. If a cluster in the same
 ** or in a neighbour channel is found which does not match the digi,
 ** it is written to the output. This makes the runtime of the
 ** algorithm independent of input data size, but has as a consequence
 ** that the output clusters are not sorted w.r.t. time.
 **
 ** This implementation is suited for modules connected to
 ** double-sided strip sensors. It is assumed that the first half
 ** of module channels is connected to the front side of the sensor,
 ** the second half of the channels to the back side.
 ** Thus, digis in the channel nChannels/2-1 and nChannels/2 are
 ** never connected to a cluster.
 **
 ** In case of strips with stereo angle and cross-connection by a
 ** double-metal layer, the first and last strip on the respective
 ** sensor side are geometric neighbours. Clustering "round the edge"
 ** (connecting the first and last channel for this side) can be
 ** enabled by the methods ConnectEdgeFront and ConnectEdgeBack.
 **
 ** The digis are connected to the cluster in the order left to right,
 ** i.e. with ascending channel number. In case of clustering round
 ** the edge, the channels at the right edge are considered left neighbours
 ** of the first channel, i.e. the cluster starts with high channel
 ** number.
 **/
class CbmStsDigisToHitsModule : public TNamed
{
  public:

    /** Default constructor **/
    CbmStsDigisToHitsModule();


    /** Constructor with parameters
     ** @param nChannels  Number of channels in the module
     ** @param deltaT     Max. time difference for digis in one cluster
     ** @param name       Class instance name
     ** @param module     Pointer to CbmModule object (in CbmStsSetup)
     ** @param output     Pointer to output array of CbmStsClusters
     */
    CbmStsDigisToHitsModule(UShort_t nChannels, Double_t timeCutDigisInNs, Double_t timeCutDigisInSigma, Double_t timeCutClustersInNs, Double_t timeCutClustersInSigma,
                              const char* name, CbmStsModule* module = NULL,
                              Int_t moduleNumber = 0, CbmStsClusterAnalysis* clusterAna = NULL);

    CbmStsDigisToHitsModule(const CbmStsDigisToHitsModule&) = delete; 
    CbmStsDigisToHitsModule& operator=(const CbmStsDigisToHitsModule&) = delete; 

    /** Destructor **/
    virtual ~CbmStsDigisToHitsModule();


    /** @brief Allow connection of first and last channel on back side
     ** @param connect  If kTRUE, allow connection
     **
     ** If this flag is set, the first and last channel of the back side
     ** can be part of the same cluster. This is e.g. the case for finite
     ** stereo angle of the strips with cross-connection by a
     ** double metal layer.
     **/
    void ConnectEdgeBack(Bool_t connect = kTRUE) {
      fConnectEdgeBack = connect;
    }


    /** @brief Allow connection of first and last channel on front side
     ** @param connect  If kTRUE, allow connection
     **
     ** If this flag is set, the first and last channel of the back side
     ** can be part of the same cluster. This is e.g. the case for finite
     ** stereo angle of the strips with cross-connection by a
     ** double metal layer.
      **/
    void ConnectEdgeFront(Bool_t connect = kTRUE) {
      fConnectEdgeFront = connect;
    }


    /** @brief Get number of channels
     ** @return Number of channels
     **/
    Int_t GetSize() const { return fSize; }


    /** @brief Process the buffer of active channels
     **
     ** At the end of the time slice / event, the remaining active channels
     ** in the buffers have to be processed.
     **/
    void ProcessBuffer();


    /** Process an input digi
     ** @param channel   Channel number
     ** @param time      Digi time [ns]
     ** @param index     Index of digi object in its TClonesArray
     ** @return  kTRUE is digi was successfully processed
     **/
    Bool_t ProcessDigi(UShort_t channel, Double_t time, Int_t index);


    /** Reset the internal bookkeeping **/
    void Reset();

    //DigisToHits
    void AddDigiToQueue(const CbmStsDigi* digi, Int_t digiIndex);

    std::vector<CbmStsHit> ProcessDigisAndAbsorbAsVector(CbmEvent* event)
    {
      ProcessDigis(event);
      return fHitOutputVector;
    }

    TClonesArray* ProcessDigisAndAbsorb(CbmEvent* event)
    {
      ProcessDigis(event);
      return fHitOutput;
    }
    void ProcessDigis(CbmEvent* event);

    TClonesArray* GetClusterOutput() { return fClusterOutput;}
    TClonesArray* GetHitOutput() { return fHitOutput;}


  private:

    UShort_t fSize;               /// Number of channels
    Double_t fTimeCutDigisInSigma;     ///< Multiple of error of time difference
    Double_t fTimeCutDigisInNs;            ///< User-set maximum time difference
    Double_t fTimeCutClustersInNs;
    Double_t fTimeCutClustersInSigma;
    Bool_t fConnectEdgeFront;     /// Round-the edge clustering front side
    Bool_t fConnectEdgeBack;      /// Round-the edge clustering back side
    CbmStsModule* fModule;        //! Pointer to STS module
    TClonesArray* fClusters;      //! Output array for clusters
    std::vector<Int_t> fIndex;    //! Channel -> digi index
    std::vector<Double_t> fTime;  //! Channel -> digi time

    //DigisToHits
    std::vector<std::tuple<const CbmStsDigi*, Int_t>> fDigiQueue;
    Int_t clusterCount = 1;
    Int_t moduleNumber;
    CbmStsClusterAnalysis* fAna;
    TClonesArray* fClusterOutput;
    TClonesArray* fHitOutput;
    std::vector<CbmStsHit> fHitOutputVector;
    std::mutex lock;
    //std::vector<Int_t> fDigiIndex;


    std::vector<CbmStsHit> Convert(TClonesArray* arr)
    {
      std::vector<CbmStsHit> vec;
      Int_t entries = arr->GetEntriesFast();
      if (entries > 0) {
        CbmStsHit* hit = static_cast<CbmStsHit*>(arr->At(0));
        // LOG(info) << "Entries in TCA for data type " << hit->GetName() << ": " << entries;
      }
      for(int i=0; i< entries; ++i) {
        CbmStsHit* hit = static_cast<CbmStsHit*>(arr->At(i));
        vec.emplace_back(*hit);
      }
      return vec;
    }

    TClonesArray* Convert2(std::vector<CbmStsHit> arr)
    {
      TClonesArray* tca;
      tca = new TClonesArray("CbmStsHit", 6e3);
      Int_t entries = arr.size();
      if (entries > 0) {
        for(int i=0; i< entries; ++i) {
          CbmStsHit hit = static_cast<CbmStsHit>(arr[i]);
          //tca->AddAt(&hit, i);
          CbmStsHit* newHit = new ( (*tca)[i] ) CbmStsHit();
          *newHit = hit;
        }
      }
      return tca;
    }


    /** Check for a matching digi in a given channel
     ** @param channel  Channel number
     ** @param time     Time [ns]
     ** @value          kTRUE if matching digi found
     **
     ** The digi is considered matching if the time difference between
     ** the time argument and the time of the active digi in the channel
     ** is within the time window defined by the resolution of the module.
     **/
    Bool_t CheckChannel(UShort_t channel, Double_t time);


    /** Create a cluster object
     ** @param first  First channel of cluster
     ** @param last   Last channel of cluster
     **/
    void CreateCluster(UShort_t first, UShort_t last);


    /** Close an active cluster
     ** @param channel  Channel number
     **/
    void FinishCluster(UShort_t channel);



   ClassDef(CbmStsDigisToHitsModule, 1);

};

#endif /* CBMSTSDIGISTOHITSMODULE_H */
