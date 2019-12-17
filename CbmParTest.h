#ifndef CBMPARTEST_H
#define CBMPARTEST_H

#include "FairParGenericSet.h"

#include "TH1F.h"

class CbmParTest : public FairParGenericSet {
public:
  Float_t p1;
  Int_t ai[5000];
  TH1F* histo1;

  CbmParTest(const char* name="CbmParTest",
             const char* title="Test class for parameter io",
             const char* context="TestDefaultContext");
  ~CbmParTest(void);
  void clear(void);
  virtual Bool_t init(FairParIo*){return kTRUE;}
  virtual Int_t write(FairParIo*){return 0;}

  void putParams(FairParamList*);
  Bool_t getParams(FairParamList*);

  CbmParTest(const CbmParTest&);
  CbmParTest operator=(const CbmParTest&);

  ClassDef(CbmParTest,1)
};

#endif /* !CBMPARTEST_H */
