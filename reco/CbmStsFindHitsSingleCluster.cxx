/** @file CbmStsFindHitsSingleCluster.cxx
 ** @author Volker Friese <v.friese@gsi.de>
 ** @date 16.06.2014
 **/

#include "CbmStsFindHitsSingleCluster.h"

#include <iomanip>
#include <iostream>
#include "TClonesArray.h"
#include "FairEventHeader.h"
#include "FairRunAna.h"
#include "CbmStsSetup.h"

using namespace std;


// -----   Constructor   ---------------------------------------------------
CbmStsFindHitsSingleCluster::CbmStsFindHitsSingleCluster()
    : FairTask("StsFindHitsSingleCluster", 1)
    , fClusters(NULL)
    , fHits(NULL)
    , fSetup(NULL)
    , fTimer()
    , fNofTimeSlices(0.)
    , fNofClustersTot(0.)
    , fNofHitsTot(0.)
    , fTimeTot(0.)
    , fActiveModules()
{
}
// -------------------------------------------------------------------------


// -----   Destructor   ----------------------------------------------------
CbmStsFindHitsSingleCluster::~CbmStsFindHitsSingleCluster() {
}
// -------------------------------------------------------------------------



// -----   Task execution   ------------------------------------------------
void CbmStsFindHitsSingleCluster::Exec(Option_t*) {

	// --- Time-slice number
	Int_t iEvent = fNofTimeSlices;

	// Start timer and counter
	fTimer.Start();

    // --- Clear output arrays
	fHits->Delete();

	// --- Sort clusters into modules
	Int_t nClusters = SortClusters();

	// --- Find hits in modules
	Int_t nHits = 0;
	set<CbmStsModule*>::iterator it;
	//for (it = fActiveModules.begin(); it != fActiveModules.end(); it++) {
	for (Int_t iModule = 0; iModule < fSetup->GetNofModules(); iModule++) {
		CbmStsModule* module = fSetup->GetModule(iModule);
		if ( module->GetNofClusters() == 0 ) continue;
		Int_t nModuleHits = module->MakeHitsFromClusters(fHits);
		LOG(debug1) << GetName() << ": Module " << module->GetName()
    			      << ", clusters: " << module->GetNofClusters()
   		          << ", hits: " << nModuleHits;
		nHits += nModuleHits;
	}

  // --- Counters
  fTimer.Stop();
  fNofTimeSlices++;
  fNofClustersTot += nClusters;
  fNofHitsTot     += nHits;
  fTimeTot        += fTimer.RealTime();

  LOG(info) << "+ " << setw(20) << GetName() << ": Time slice " << setw(6)
  		      << right << iEvent << ", real time " << fixed << setprecision(6)
  		      << fTimer.RealTime() << " s, clusters: " << nClusters
  		      << ", hits: " << nHits;
}
// -------------------------------------------------------------------------



// -----   End-of-run action   ---------------------------------------------
void CbmStsFindHitsSingleCluster::Finish() {
	std::cout << std::endl;
	LOG(info) << "=====================================";
	LOG(info) << GetName() << ": Run summary";
	LOG(info) << "Time slices processed  : " << fNofTimeSlices;
	LOG(info) << "Clusters / time slice  : "
			      << fNofClustersTot / Double_t(fNofTimeSlices);
	LOG(info) << "Hits / time slice      : "
			      << fNofHitsTot / Double_t(fNofTimeSlices);
	LOG(info) << "Hits per cluster  : " << fNofHitsTot / fNofClustersTot;
	LOG(info) << "Time per time slice    : "
	    << fTimeTot / Double_t(fNofTimeSlices);
	LOG(info) << "=====================================";
}
// -------------------------------------------------------------------------


// -----   End-of-event action   -------------------------------------------
void CbmStsFindHitsSingleCluster::FinishEvent() {

	// --- Clear cluster sets for all active modules
	Int_t nModules = 0;
	set<CbmStsModule*>::iterator it;
	for (it = fActiveModules.begin(); it != fActiveModules.end(); it++) {
		(*it)->ClearClusters();
		nModules++;
	}
	fActiveModules.clear();

	LOG(debug) << GetName() << ": Cleared clusters in " << nModules
			       << " modules. ";
}
// -------------------------------------------------------------------------


// -----   Initialisation   ------------------------------------------------
InitStatus CbmStsFindHitsSingleCluster::Init()
{
    // --- Check IO-Manager
    FairRootManager* ioman = FairRootManager::Instance();
    if (NULL == ioman) {
    	LOG(error) << GetName() << ": No FairRootManager!";
    	return kFATAL;
    }

    // --- Get input array (StsDigis)
    fClusters = (TClonesArray*)ioman->GetObject("StsCluster");
    if (NULL == fClusters) {
    	LOG(error) << GetName() << ": No StsCluster array!";
    	return kERROR;
    }

    // --- Register output array
    fHits = new TClonesArray("CbmStsHit", 10000);
    ioman->Register("StsHit", "Hits in STS", fHits,IsOutputBranchPersistent("StsHit"));

    // --- Get STS setup
    fSetup = CbmStsSetup::Instance();

    LOG(info) << GetName() << ": Initialisation successful";

    return kSUCCESS;
}
// -------------------------------------------------------------------------




// ----- Sort digis into module digi maps   --------------------------------
Int_t CbmStsFindHitsSingleCluster::SortClusters() {

	// --- Counters
	Int_t nClusters = 0;

	// --- Loop over clusters in input array
	for (Int_t iCluster = 0;
			 iCluster < fClusters->GetEntriesFast(); iCluster++) {
		CbmStsCluster* cluster = static_cast<CbmStsCluster*> (fClusters->At(iCluster));
		UInt_t address = cluster->GetAddress();
		cluster->SetIndex(iCluster);
		CbmStsModule* module =
				static_cast<CbmStsModule*>(fSetup->GetElement(address, kStsModule));

	  // --- Update set of active modules
		fActiveModules.insert(module);

		// --- Assign cluster to module
		module->AddCluster(cluster);
		nClusters++;

	}  // Loop over cluster array

	// --- Debug output
	LOG(debug) << GetName() << ": sorted " << nClusters << " clusters into "
			       << fActiveModules.size() << " module(s).";
	if ( FairLogger::GetLogger()->IsLogNeeded(fair::Severity::debug3) ) {
		set <CbmStsModule*>::iterator it;
		for (it = fActiveModules.begin(); it != fActiveModules.end() ; it++) {
			CbmStsModule* module = (*it);
			LOG(debug3) << GetName() << ": Module " << module->GetName()
						      << ", clusters " << module->GetNofClusters();
		}  // active module loop
	}  //? DEBUG 3

	return nClusters;
}
// -------------------------------------------------------------------------

ClassImp(CbmStsFindHitsSingleCluster)
