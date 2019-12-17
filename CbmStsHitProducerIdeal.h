// -------------------------------------------------------------------------
// -----                 CbmStsHitProducerIdel header file             -----
// -----                  Created 10/01/06  by V. Friese               -----
// -------------------------------------------------------------------------


/** CbmStsHitProducerIdeal.h
 *@author V.Friese <v.friese@gsi.de>
 **
 ** The ideal hit producer produces hits of type CbmStsMapsHit by copying
 ** the MCPoint position. The position error is set to 1 mum, much 
 ** smaller than can be obtained by any detector. Using the hits from 
 ** this HitProducer is thus equivalent to using MC information
 ** directly, but with the correct data interface.
 **/


#ifndef CBMSTSHITPRODUCERIDEAL_H
#define CBMSTSHITPRODUCERIDEAL_H 1


#include "FairTask.h"

class TClonesArray;



class CbmStsHitProducerIdeal : public FairTask
{

 public:

  /** Default constructor **/  
  CbmStsHitProducerIdeal();


  /** Destructor **/
  ~CbmStsHitProducerIdeal();


  /** Virtual method Init **/
  virtual InitStatus Init();


  /** Virtual method Exec **/
  virtual void Exec(Option_t* opt);


 private:

  /** Input array of CbmStsPoints **/
  TClonesArray* fPointArray;

  /** Output array of CbmStsHits **/
  TClonesArray* fHitArray;  

  CbmStsHitProducerIdeal(const CbmStsHitProducerIdeal&);
  CbmStsHitProducerIdeal& operator=(const CbmStsHitProducerIdeal&);

  ClassDef(CbmStsHitProducerIdeal,1);

};

#endif
