/** @file CbmStsSignal.cxx
 ** @author Volker Friese <v.friese@gsi.de>
 ** @date 28.06.2014
 **/

#include "CbmStsSignal.h"


// -----   Default constructor   -------------------------------------------
CbmStsSignal::CbmStsSignal(Double_t time, Double_t charge,
				                   Int_t index, Int_t entry, Int_t file) :
				                   TObject(),
				                   fTime(time),
				                   fMatch()  {
	fMatch.AddLink(charge, index, entry, file);
}
// -------------------------------------------------------------------------


// -----   Destructor   ----------------------------------------------------
CbmStsSignal::~CbmStsSignal() { }
// -------------------------------------------------------------------------


ClassImp(CbmStsSignal)
