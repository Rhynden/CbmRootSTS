#ifndef CBMSTSMCQA_H_
#define CBMSTSMCQA_H_

#include "FairTask.h"

#include "Rtypes.h"

class TClonesArray;
class CbmStsSetup;
class CbmHistManager;

class CbmStsMCQa : public FairTask
{
    public:
	CbmStsMCQa();

	virtual ~CbmStsMCQa();

	virtual InitStatus Init();

	virtual void Exec(Option_t*);

	virtual void Finish();

	void CreateHistograms();

	void CreateNofObjectsHistograms();

	void CreatePointHistograms();

	void ProcessPoints(const TClonesArray*);

    private:
	void ReadDataBranches();

	CbmHistManager* fHM;
	TClonesArray* fStsPoints;
	TClonesArray* fMCTracks;
	CbmStsSetup * fSetup;
	Int_t fNofStation;

	CbmStsMCQa(const CbmStsMCQa&);
	CbmStsMCQa& operator=(const CbmStsMCQa&);

	ClassDef(CbmStsMCQa, 1);
};

#endif
