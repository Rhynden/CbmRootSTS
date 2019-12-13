/**
 * \file CbmStsWkn.h
 * \author Olga Derenovskaia <odenisova@jinr.ru>
 * \date 2019
 **/

#ifndef CbmStsWkn_H_
#define CbmStsWkn_H_

#include "TObject.h"

class TClonesArray;
class CbmGlobalTrack;
class CbmStsTrack;

class CbmStsWkn: public TObject
{
public:

	CbmStsWkn();
	void Init();
	virtual ~CbmStsWkn();

	/**
	* \brief Return Wkn value 
	* \param[in] StsTrackIndex Index of sts track
	*/
	Double_t GetStsWkn(Int_t StsTrackIndex);
	/**
	* \brief Return Wkn value 
	* \param[in] CbmStsTrac
	*/
	Double_t GetStsWkn(const CbmStsTrack* StsTrack);
	

	/**\brief Set Wkn degree*/
	void SetDegWkn(Int_t deg){ fdegWkn = deg;}
	/**\brief Set Wkn sample size*/
	void SetNsetWkn(Int_t nSet){ fnSet = nSet;}

	/** \brief Return Wkn degree */
	Int_t GetDegWkn() {return fdegWkn;}

	/*** \brief Return Wkn sample size*/
	Int_t GetNsetWkn() { return fnSet;}

private:
    Int_t fdegWkn; //  statistics degree
	float fEmp;
	float fXi;
    float fk1;
    Int_t fnSet;
	Double_t fwkn;
	
    TClonesArray* fGlobalTracks;
    TClonesArray* fStsTracks;
    TClonesArray* fStsHits;
    TClonesArray* fStsClusterArray;
    TClonesArray* fStsDigiArray;

    CbmStsWkn(const CbmStsWkn&);
    CbmStsWkn& operator=(const CbmStsWkn&);
    
    ClassDef(CbmStsWkn, 1);
};

#endif /* CbmStsWkn_H_ */
