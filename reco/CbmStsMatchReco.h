/** @file CbmStsMatchReco.h
 ** @author Volker Friese <v.friese@gsi.de>
 ** @date 6.12.2016
 **
 ** Based on CbmMatchRecoToMC by A. Lebedev.
 **/

#ifndef CBMSTSMATCHRECO_H
#define CBMSTSMATCHRECO_H 1

#include "FairTask.h"
#include <vector>

class TClonesArray;
class CbmDigiManager;
class CbmMCDataArray;



/** @class CbmStsMatchReco
 ** @brief Task class for matching reconstructed STS data to MC objects
 ** @author Volker Friese <v.friese@gsi.de>
 ** @since 6.12.2016
 ** @version 1.0
 **
 ** This task creates objects of type CbmMatch for the reconstructed
 ** STS data (clusters, hits, tracks). They are based on the match
 ** information coming with the CbmStsDigi objects.
 ** Clusters and hits are matched to CbmStsPoints, tracks are
 ** matched to CbmMCTracks.
 **
 **/
class CbmStsMatchReco : public FairTask
{

	public:

		/** Constructor **/
		CbmStsMatchReco();


		/** Destructor **/
		virtual ~CbmStsMatchReco();


	/** Task execution **/
		virtual void Exec(Option_t* opt);


		/** End-of-run action **/
		virtual void Finish();


		/** Initialisation
		 ** @value  Status
		 **/
		virtual InitStatus Init();



	private:

		void MatchClusters(const TClonesArray* clusters);

		void MatchHits(const TClonesArray* clusters, const TClonesArray* hits);

  	void MatchTracks(const TClonesArray* hits,
  			const TClonesArray* tracks, CbmMCDataArray* points);

  	CbmDigiManager* fDigiManager;    //! Interface to digi branch
		CbmMCDataArray* fMCTracks;       //! Monte-Carlo tracks
		CbmMCDataArray* fPoints;         //! CbmStsPoint
    TClonesArray*   fDigiMatches;    //! CbmMatch (digi)
		TClonesArray*   fClusters;       //! CbmStsCluster
		TClonesArray*   fHits;           //! CbmStsHit
		TClonesArray*   fTracks;         //! CbmStsTrack


		CbmStsMatchReco(const CbmStsMatchReco&);
		CbmStsMatchReco& operator=(const CbmStsMatchReco&);

		ClassDef(CbmStsMatchReco, 1);

};

#endif /* CBMSTSMATCH_H */
