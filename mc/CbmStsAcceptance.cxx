/** @file CbmAcceptance.cxx
 ** @author Volker Friese <v.friese@gsi.de>
 ** @date 04.05.2016
 **/

#include "CbmStsAcceptance.h"

#include <cassert>
#include <sstream>
#include <iomanip>

#include "TClonesArray.h"
#include "FairLogger.h"
#include "CbmMCTrack.h"
#include "CbmStsAddress.h"
#include "CbmStsPoint.h"

using std::stringstream;
using std::right;
using std::setw;
using std::fixed;
using std::setprecision;
using std::map;
using std::string;

// -----  Initialisation of static data members   -----------------------------
Int_t CbmStsAcceptance::fNofInstances = 0;
map<Int_t, map<Int_t, Int_t>>CbmStsAcceptance::fCountMap =
		map<Int_t, map<Int_t, Int_t>>();
// --------------------------------------------------------------------------

// -----   Constructor   -----------------------------------------------------
CbmStsAcceptance::CbmStsAcceptance()
	: FairTask("CbmStsAcceptance"),
	  fPoints(NULL), fTracks(NULL), fTimer(), fNofEvents(0), fNofPointsTot(0), fTimeTot(0.)
{

	// TODO: This is a sloppy way of preventing more than one instance.
	// Should be a real single ton class.
	if ( fNofInstances ) LOG(fatal) << GetName()
			<< ": Instance of this class is already present! Aborting...";

	fNofInstances++;
}
// --------------------------------------------------------------------------



// -----   Destructor   -----------------------------------------------------
CbmStsAcceptance::~CbmStsAcceptance() {
}
// --------------------------------------------------------------------------


// -----   Clear the count map    -------------------------------------------
void CbmStsAcceptance::Clear(Option_t*) {
	map<Int_t, map<Int_t, Int_t>>::iterator it1;
	for (it1 = fCountMap.begin(); it1 != fCountMap.end(); it1++) {
		it1->second.clear();
	}
}
// --------------------------------------------------------------------------



// -----   Task execution   -------------------------------------------------
void CbmStsAcceptance::Exec(Option_t* /*opt*/) {

	fTimer.Start();

	// Reset bookkeeping
	Clear();

	CbmStsPoint* point = NULL;
	Int_t nPoints = fPoints->GetEntriesFast();

	// Loop over points in array
	for (Int_t iPoint = 0; iPoint < nPoints; iPoint++ ) {
		point = dynamic_cast<CbmStsPoint*> (fPoints->At(iPoint));
		assert(point);

		// Track index and station number
		Int_t trackId = point->GetTrackID();
		Int_t address = point->GetDetectorID();
		Int_t stationNr = CbmStsAddress::GetElementId(address, kSts);

		// Increment entry in count map
		Int_t nCounts = fCountMap[trackId][stationNr];
		fCountMap[trackId][stationNr] = ++nCounts;
	} //# StsPoints

	// Perform consistency check
	if ( ! Test() ) LOG(fatal) << GetName() << ": consistency check failed!";

	fTimer.Stop();
  LOG(debug) << ToString();
  LOG(info) << "+ " << setw(20) << GetName() << ": Event " << setw(6)
              << right << fNofEvents << ", time " << fixed << setprecision(6)
              << fTimer.RealTime() << " s, STS points: " << nPoints
              << ", map size " << fCountMap.size() << ", test OK";

  // Counters
  fNofEvents++;
  fNofPointsTot += nPoints;
  fTimeTot += fTimer.RealTime();

}
// --------------------------------------------------------------------------



// -----   End-of-run action   ----------------------------------------------
void CbmStsAcceptance::Finish() {
	std::cout << std::endl;
	LOG(info) << "=====================================";
	LOG(info) << GetName() << ": Run summary";
	LOG(info) << "Events processed    : " << fNofEvents;
	LOG(info) << "StsPoints / event   : " << setprecision(1)
            << fNofPointsTot / Double_t(fNofEvents);
	LOG(info) << "Real time per event : " << setprecision(6)
			      << fTimeTot / Double_t(fNofEvents) << " s";
	LOG(info) << "=====================================";
}
// --------------------------------------------------------------------------



// -----   Total number of StsPoint objects   -------------------------------
Int_t CbmStsAcceptance::GetNofPoints(Int_t trackId) {

	Int_t nPoints = 0;
	map<Int_t, map<Int_t, Int_t>>::iterator it1 = fCountMap.find(trackId);
	if ( it1 == fCountMap.end() ) return 0;
	map<Int_t, Int_t>::iterator it2;
	for (it2 = (it1->second).begin(); it2 != (it1->second).end(); it2++) {
		nPoints += it2->second;
	}

	return nPoints;
}
// --------------------------------------------------------------------------



// -----   Number of StsPoints in a given STS station   ---------------------
Int_t CbmStsAcceptance::GetNofPoints(Int_t trackId, Int_t stationNr) {

	// Note: the implementation avoids automatic instantiation of
	// entries in the outer and inner map as would be the case
	// when using the index operator []. In that way, the size of the
	// inner map always corresponds to the number of activated stations.
	map<Int_t, map<Int_t, Int_t>>::iterator it1 = fCountMap.find(trackId);
	if ( it1 == fCountMap.end() ) return 0;
	map<Int_t, Int_t>::iterator it2 = (it1->second).find(stationNr);
	if ( it2 == (it1->second).end() ) return 0;
	return it2->second;

}
// --------------------------------------------------------------------------



// -----   Number of STS stations the track is registered in   --------------
Int_t CbmStsAcceptance::GetNofStations(Int_t trackId) {
	if ( fCountMap.find(trackId) == fCountMap.end() ) return 0;
	return fCountMap[trackId].size();
}
// --------------------------------------------------------------------------



// -----   Task initialisation   ---------------------------------------------
InitStatus CbmStsAcceptance::Init() {

  // --- Get IO-Manager
  FairRootManager* ioman = FairRootManager::Instance();
  assert (ioman);

  // --- Get input array (StsPoint)
  fPoints = (TClonesArray*) ioman->GetObject("StsPoint");
  if ( ! fPoints ) {
  	LOG(error) << GetName()
  			       << ": No StsPoint array. Task will be deactivated.";
  	SetActive(kFALSE);
  	return kERROR;
  }

  // --- Get input array (MCTrack)
  fTracks = (TClonesArray*) ioman->GetObject("MCTrack");
  if ( ! fTracks ) {
  	LOG(error) << GetName()
  			       << ": No MCTrack array. Task will be deactivated.";
  	SetActive(kFALSE);
  	return kERROR;
  }

  return kSUCCESS;
}
// --------------------------------------------------------------------------



// -----   Test internal consistency   --------------------------------------
Bool_t CbmStsAcceptance::Test() {

	Bool_t result = kTRUE;

	// Loop over MCTracks
	CbmMCTrack* track = 0;
	Int_t nPoints1 = 0;
	Int_t nPoints2 = 0;
	Int_t nTracks = fTracks->GetEntriesFast();
	for (Int_t trackId = 0; trackId < nTracks; trackId++) {
		track = dynamic_cast<CbmMCTrack*>(fTracks->At(trackId));
		assert (track);
		nPoints1 = track->GetNPoints(kSts);
		nPoints2 = GetNofPoints(trackId);
		// The value of 31 is the maximal number that can be stored in CbmMCTrack
		// for the count of StsPoints. Sometimes there are more (spiralling electrons).
		if ( nPoints1 != nPoints2 && nPoints1 < 31) {
			LOG(error) << GetName() << ": Track " << trackId
					<< " points from MCTrack " << nPoints1
					<< ", points from StsAcceptance " << nPoints2;
			LOG(error) << track->ToString();
			result = kFALSE;
		}
	} //# MCTracks

	return result;
}
// --------------------------------------------------------------------------



// -----   Status info   ----------------------------------------------------
string CbmStsAcceptance::ToString() const
{
   stringstream ss;
   Int_t nEntries = fCountMap.size();
   Int_t firstIndex = 0;
   Int_t lastIndex = 0;
   if ( nEntries ) {
  	 firstIndex = fCountMap.begin()->first;
  	 lastIndex =  fCountMap.rbegin()->first;
   }
   ss << "StsAcceptance: map size " << nEntries << " (from " << firstIndex
  		<< " to " << lastIndex << " )" << std::endl;
   return ss.str();
}
// -------------------------------------------------------------------------

ClassImp(CbmStsAcceptance)


