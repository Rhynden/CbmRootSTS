/** @file CbmStsSetup.h
 ** @author Volker Friese <v.friese@gsi.de>
 ** @date 04.05.2016
 **/

#ifndef CBMSTSACCEPTANCE_H
#define CBMSTSACCEPTANCE_H 1

#include <map>
#include <string>
#include "TStopwatch.h"
#include "FairTask.h"

class TClonesArray;

/** @class CbmStsAcceptance
 ** @brief Singleton task class for easy access to the acceptance in the STS
 ** @author Volker Friese <v.friese@gsi.de>
 ** @since 04.05.2016
 ** @version 1.0
 **
 ** This tool provides access to the number of StsPoints for a given MCTrack,
 ** specified by its index in the MCTrack array, in each station of the STS.
 ** It thus allows to check the acceptance of a track in the STS according
 ** to user-specified acceptance criteria. The standard acceptance definition
 ** (to have points in at least three STS station) is also implemented in the
 ** method IsAccepted().
 **
 ** Access to the number of STS points is provided by the static method.
 ** Int_t CbmStsAcceptance::GetNofPoints(Int_t, Int_t). There are several
 ** other static methods for convenience of analysis, like GetNofStations
 ** or IsInStation.
 **
 ** The task has to be registered in the run macro before any task using its
 ** functionality. It is not a singleton, but there is a protection against
 ** its being instantiated more than once.
 **/
class CbmStsAcceptance : public FairTask {


	public:

		/** Constructor **/
		CbmStsAcceptance();


		/** Destructor **/
		virtual ~CbmStsAcceptance();


		/** Task execution
		 **
		 ** Loops through the StsPoint array and fills the count map.
		 **/
		virtual void Exec(Option_t* opt);


	  /** End-of-run action **/
	  virtual void Finish();


		/** Total number of StsPoints for this MCTrack
		 ** @param trackId    Index of MCTrack in array
		 ** @value Number of StsPoints for this track
		 */
		static Int_t GetNofPoints(Int_t trackId);


		/** Number of StsPoints of a MCTrack in a given STS station
		 ** @param trackId    Index of MCTrack in array
		 ** @param stationNr  STS Station number
		 ** @value Number of StsPoints in the specified station
		 **/
		static Int_t GetNofPoints(Int_t trackId, Int_t stationNr);


		/** Number of stations in which a track is registered
		 ** @param trackId    Index of MCTrack in array
		 ** @value Number of stations in which the track has StsPoints
		 */
		static Int_t GetNofStations(Int_t trackId);


		/** Task initialisation **/
		virtual InitStatus Init();


		/** Check for STS acceptance
		 ** @param trackId       Index of MCTrack in array
		 ** @param nMinStations  Minimum number of station required to be accepted
		 ** @value Acceptance decision
		 */
		static Bool_t IsAccepted(Int_t trackId, Int_t nMinStations = 3) {
			return ( GetNofStations(trackId) >= nMinStations );
		}


		/** Check whether a track is registered in a STS station
		 ** @param trackId    Index of MCTrack in array
		 ** @param stationNr  STS Station number
		 ** @value kTRUE if and only if at least one StsPoint in this station
		 **/
		static Bool_t IsInStation(Int_t trackId, Int_t stationNr) {
			return ( GetNofPoints(trackId, stationNr) > 0 );
		}


		/** Status info **/
		std::string ToString() const;



	private:

		TClonesArray* fPoints;  ///< Input array of CbmStsPoint objects
		TClonesArray* fTracks;  ///< Input array of CbmMCTrack objects
		TStopwatch fTimer;      ///< Performance monitoring
		static std::map<Int_t, std::map<Int_t, Int_t>> fCountMap;  ///< Internal bookkeeping
		static Int_t fNofInstances; ///< Number of instances of this class

	  // --- Run counters
	  Int_t          fNofEvents;      ///< Total number of events processed
	  Double_t       fNofPointsTot;   ///< Total number of points processed
	  Double_t       fTimeTot;        ///< Total execution time

		/** Clear map entries **/
		void Clear(Option_t* ="");

		/** Test consistency
		 ** @value kTRUE is test successful; else kFLASE
		 ** The test compares for each MCTrack the number of StsPoints obtained
		 ** from the CbmMCTrack object to that obtained from this class.
		 **/
		Bool_t Test();


                CbmStsAcceptance(const CbmStsAcceptance&);
                CbmStsAcceptance& operator=(const CbmStsAcceptance&);
                
		ClassDef(CbmStsAcceptance,1);
};

#endif /* CBMSTSACCEPTANCE_H */
