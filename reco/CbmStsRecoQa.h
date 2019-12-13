/*
 * CbmStsTestQa.h
 *
 *  Created on: 17.03.2017
 *  Very simple ana macro to check reconstruction in STS. Calculates average number of hits per track
 *  and average momentum.
 *      Author: vfriese
 */

#ifndef CBMSTSRECOQA_H_
#define CBMSTSRECOQA_H 1

#include <FairTask.h>

#include <fstream>

class CbmEvent;
class CbmVertex;
class CbmHistManager;
class TClonesArray;

class CbmStsRecoQa: public FairTask {

public:
	CbmStsRecoQa();

        CbmStsRecoQa(const CbmStsRecoQa&) = delete;
        CbmStsRecoQa& operator=(const CbmStsRecoQa&) = delete;

	virtual ~CbmStsRecoQa();


    /** Task execution **/
    virtual void Exec(Option_t* opt);


    /** End-of-run action **/
    virtual void Finish();


    /** End-of-event action **/
    virtual void FinishEvent() { };


    /** Initialisation **/
    virtual InitStatus Init();


private:

    TClonesArray* fEvents;   //!
    TClonesArray* fTracks;  //!

    Int_t fNofTs;
    Int_t fNofEvents;
    Double_t fNofTracksTot;
    Double_t fNofGoodTracks;
    Double_t fNofHitsTot;
    Double_t fPTot;
    Double_t fTimeTot;


    void ProcessEvent(CbmEvent* event = NULL);

    ClassDef(CbmStsRecoQa, 1);


};

#endif /* CBMSTSTESTQA_H */
