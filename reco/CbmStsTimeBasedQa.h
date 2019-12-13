/** @file CbmStsTimeBasedQa.h
 ** @author Grigory Kozlov <g.kozlov@gsi.de>
 ** @date 19.04.2016
 **/

#ifndef CBMSTSTIMEBASEDQA_H_
#define CBMSTSTIMEBASEDQA_H_

#include "FairTask.h"
#include "CbmStsDigi.h"
#include "CbmTimeSlice.h"
#include "CbmHistManager.h"
#include "CbmMCDataArray.h"

#include <string>
#include <vector>

class TClonesArray;
class CbmStsSetup;

class CbmStsTimeBasedQa : public FairTask
{
public:
  CbmStsTimeBasedQa();

  virtual ~CbmStsTimeBasedQa();

  virtual InitStatus Init();

  virtual void Exec(Option_t* opt);

  virtual void Finish();

  void SetOutputDir(const std::string& outputDir) { fOutputDir = outputDir; }

  void UseDaq(Bool_t daq) { fDaq = daq; }

  void CreateHistograms(const std::string& type);

  void CreateNofObjectsHistograms(const std::string& type);

  void CreateHitParametersHistograms(const std::string& type);

  void Create2dHistograms(const std::string& type);

  void ProcessDigisAndPoints(
		  const std::vector<CbmStsDigi> digis,
		   CbmMCDataArray* points,
		  const std::string& type);
  void ProcessDigisAndPoints(
		  const TClonesArray* digis,
		  const CbmMCDataArray* points,
		  const std::string& type);
  void ProcessClusters(
		  const TClonesArray* clusters,
		  const TClonesArray* clusterMatches,
		  CbmMCDataArray* points,
		  const std::string& type);
  void ProcessHits(
		  const TClonesArray* hits,
		  const TClonesArray* hitMatches,
		  const std::string& type);

  void FillResidualAndPullHistograms(
		  CbmMCDataArray* points,
		  const TClonesArray* hits,
		  const TClonesArray* hitMatches,
		  const std::string& type);

private:
  void ReadDataBranches();

  CbmHistManager* fHM;
  std::string fOutputDir;

  CbmStsSetup* fSetup;

  CbmTimeSlice* fTimeSlice;
  TClonesArray* fStsDigis;
  TClonesArray* fStsDigiMatches;
  TClonesArray* fStsClusters;
  TClonesArray* fStsHits;
  TClonesArray* fStsClusterMatches;
  TClonesArray* fStsHitMatches;
  CbmMCDataArray* fStsPoints;
  std::vector<CbmStsDigi> fStsDigiData;

  Bool_t fDaq;
  Int_t fMaxScale;

  std::vector<Int_t> fMCinCell;
  std::vector<Int_t> fHitsinCell;
  std::vector<Float_t> fEffinCell;

  CbmStsTimeBasedQa(const CbmStsTimeBasedQa&);
  CbmStsTimeBasedQa& operator=(const CbmStsTimeBasedQa&);

  ClassDef(CbmStsTimeBasedQa, 1);
};

#endif
