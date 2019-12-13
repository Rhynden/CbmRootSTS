/** @file CbmStsSignal.h
 ** @author Volker Friese <v.friese@gsi.de>
 ** @date 28.06.2014
 **/

#ifndef CBMSTSSIGNAL_H
#define CBMSTSSIGNAL_H 1

#include "TObject.h"
#include "CbmMatch.h"


/** @class CbmStsSignal
 ** @brief Data class for an analog signal in the STS
 ** @author Volker Friese <v.friese@gsi.de>
 ** @since 28.06.2014
 ** @version 1.0
 **
 ** Simple data class used in the digitisation process of the STS. It describes
 ** an analog charge signal produced in the STS sensors and arriving at the
 ** readout. It contains time and charge information (the latter through the
 ** total weight member of the CbmMatch member), and references to the MCPoints
 ** having caused the charge.
 ** In the most general case, a signal can be produced by more than one
 ** MCPoint; that is why the MC reference is of type CbmMatch and not CbmLink.
 **/
class CbmStsSignal : public TObject {

	public:

		/** Default constructor
		 ** @param time    Signal time [ns]
		 ** @param charge  Analog charge [e]
		 ** @param index   Index of CbmStsPoint
		 ** @param entry   Entry in input TTree
		 ** @param file    Number of input file
		 **/
		CbmStsSignal(Double_t time = 0., Double_t charge = 0.,
				         Int_t index = 0, Int_t entry = -1, Int_t file = -1);


		/** Destructor **/
		virtual ~CbmStsSignal();


		/** Add a link to MCPoint to the match member
		 ** @param charge  Analog charge [e]
		 ** @param index   Index of CbmStsPoint
		 ** @param entry   Entry in input TTree
		 ** @param file    Number of input file
		 **/
		void AddLink(Double_t charge, Int_t index,
				         Int_t entry = -1, Int_t file = -1) {
			fMatch.AddLink(charge, index, entry, file);
		}


		/** Charge
		 ** @return Signal analog charge [e]
		 **/
		Double_t GetCharge() const { return fMatch.GetTotalWeight(); }


		/** Match object
		 ** @return const. reference to match object
		 **/
		const CbmMatch& GetMatch() const { return fMatch; }


		/** Time
		 ** @return Signal time [ns]
		 **/
		Double_t GetTime() const { return fTime; }


    /** Set time
     ** @param time  New signal time [ns]
     **/
    void SetTime(Double_t time) { fTime = time; }


		/** Less operator
		 ** @param Reference to CbmSignal object to compare to
		 ** @return true if signal time is less than time of comparison signal
		 **/
		bool operator < (const CbmStsSignal& otherSignal) const {
			return ( fTime < otherSignal.GetTime() );
		}


		/** Comparator for pointer objects
		 ** Needed to store objects by pointer in sorted containers
		 ** Sorting criterion is the signal time.
		 **/
		struct Before {
				bool operator () (CbmStsSignal* signal1, CbmStsSignal* signal2) const {
					return ( signal1->GetTime() < signal2->GetTime() );
				}
		};

	private:

		Double_t fTime;    ///< Signal time [ns]
		CbmMatch fMatch;   ///< Match object (total weight = charge)

		ClassDef(CbmStsSignal, 1);
};

#endif /* CBMSTSSIGNAL_H */
