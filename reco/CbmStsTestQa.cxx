/*
 * CbmStsTestQa.cxx
 *
 *  Created on: 07.11.2016
 *      Author: vfriese
 */

#include "CbmStsTestQa.h"

#include <cassert>
#include <fstream>
#include <iomanip>
#include "TClonesArray.h"
#include "TStopwatch.h"
#include "FairLogger.h"
#include "CbmEvent.h"
#include "CbmHistManager.h"
#include "CbmStsAddress.h"
#include "CbmStsCluster.h"
#include "CbmStsHit.h"
#include "CbmStsSetup.h"
#include "CbmStsTrack.h"
#include "CbmVertex.h"

using namespace std;

CbmStsTestQa::CbmStsTestQa() :
	fEvents(NULL),
	fClusters(NULL),
	fHits(NULL),
	fTracks(NULL),
	fVertex(NULL),
	fHistMan(NULL),
	fFileClusters(NULL),
	fFileHits(NULL),
	fFileTracks(NULL),
	fFileVertices(NULL),
	fSetup(NULL)
{
	SetName("StsTestQa");
}

CbmStsTestQa::~CbmStsTestQa() {
	if ( fHistMan ) delete fHistMan;
	if ( fFileClusters ) delete fFileClusters;
	if ( fFileHits ) delete fFileHits;
	if ( fFileTracks ) delete fFileTracks;
	if ( fFileVertices ) delete fFileVertices;
}



// -----   Execution   -----------------------------------------------------
void CbmStsTestQa::Exec(Option_t* /*opt*/) {

	// If there is an event branch: do the event loop
	if ( fEvents ) {
		Int_t nEvents = fEvents->GetEntriesFast();
		LOG(debug) << GetName() << ": found time slice with " << nEvents
				   << " events.";

		for (Int_t iEvent = 0; iEvent < nEvents; iEvent++) {
			CbmEvent* event = dynamic_cast<CbmEvent*>(fEvents->At(iEvent));
			ProcessEvent(event);
		}
	}

	else ProcessEvent();

}
// -------------------------------------------------------------------------



// -----   End-of-run action   ---------------------------------------------
void CbmStsTestQa::Finish() {

	fHistMan->WriteToFile();
	if ( fFileClusters ) fFileClusters->close();
	if ( fFileHits) fFileHits->close();
	if ( fFileTracks) fFileTracks->close();
	if ( fFileVertices) fFileVertices->close();

}
// -------------------------------------------------------------------------



// -----   Initialisation   ------------------------------------------------
InitStatus CbmStsTestQa::Init()
{
    // --- Check IO-Manager
    FairRootManager* ioman = FairRootManager::Instance();
    if (NULL == ioman) {
    	LOG(error) << GetName() << ": No FairRootManager!";
   	return kFATAL;
    }

    // --- Get STS setup
    fSetup = CbmStsSetup::Instance();

    // --- Get input array (events)
    fEvents = (TClonesArray*) ioman->GetObject("Event");

    // --- Get input array (clusters)
	fClusters = (TClonesArray*) ioman->GetObject("StsCluster");
	assert(fClusters);

    // --- Get input array (hits)
	fHits = (TClonesArray*) ioman->GetObject("StsHit");
	assert(fHits);

    // --- Get input array (tracks)
	fTracks = (TClonesArray*) ioman->GetObject("StsTrack");
	assert(fTracks);

	// --- Get event vertex (for old data format)
	//fVertex = (CbmVertex*) ioman->GetObject("PrimaryVertex");
        // Get pointer to PrimaryVertex object from IOManager if it exists
        // The old name for the object is "PrimaryVertex" the new one
        // "PrimaryVertex." Check first for the new name
        fVertex = dynamic_cast<CbmVertex*>(ioman->GetObject("PrimaryVertex."));
        if (nullptr == fVertex) {
          fVertex = dynamic_cast<CbmVertex*>(ioman->GetObject("PrimaryVertex"));
        }
        if (nullptr == fVertex) {
          LOG(fatal) << "No primary vertex";
        }

	// Instantiate histogram manager
	fHistMan = new CbmHistManager();

	// Create histograms
	fHistMan->Create1<TH1F>("Cluster size", "Digis per cluster", 10, -0.5, 10.5);
	fHistMan->Create1<TH1F>("Hit x in station 8", "Hit x in station 8", 100, -100., 100.);
	fHistMan->Create1<TH1F>("pt primary tracks", "pt primary tracks", 100, 0., 5.);
	fHistMan->Create1<TH1F>("z PV", "z PV", 100., -0.1, 0.1);

	// For debug output into file
	if ( FairLogger::GetLogger()->IsLogNeeded(fair::Severity::debug) ) {
		fFileClusters = new std::ofstream("clusters.txt");
		fFileHits     = new std::ofstream("hits.txt");
		fFileTracks   = new std::ofstream("tracks.txt");
		fFileVertices = new std::ofstream("vertices.txt");
	}

 return kSUCCESS;
}
// -------------------------------------------------------------------------



// -----   Process one event   ---------------------------------------------
void CbmStsTestQa::ProcessEvent(CbmEvent* event) {

	// Timer
	TStopwatch timer;
	timer.Start();
	Int_t eventNr = (event ? event->GetNumber() : -1);

	// --- Process clusters
	Int_t nClusters = (event ? event->GetNofData(kStsCluster) :
			                   fClusters->GetEntriesFast());
	for (Int_t iCluster = 0; iCluster < nClusters; iCluster++) {
		Int_t index = (event ? event->GetIndex(kStsCluster, iCluster)
				             : iCluster);
		CbmStsCluster* cluster = dynamic_cast<CbmStsCluster*>(fClusters->At(index));
		assert(cluster);
	    fHistMan->H1("Cluster size")->Fill(Float_t(cluster->GetSize()));
	    if ( fFileClusters ) (*fFileClusters) << cluster->ToString() << "\n";
	} //# clusters in event

	// Process hits
	Int_t nHits = (event ? event->GetNofData(kStsHit)
			             : fHits->GetEntriesFast());
	for (Int_t iHit = 0; iHit < nHits; iHit++) {
		Int_t index = (event ? event->GetIndex(kStsHit, iHit) : iHit);
		CbmStsHit* hit = dynamic_cast<CbmStsHit*>(fHits->At(index));
		assert(hit);
		Int_t station = fSetup->GetStationNumber(hit->GetAddress());
		if ( fFileHits ) (*fFileHits) << hit->ToString() << "\n";
		if ( station != 7) continue;
		fHistMan->H1("Hit x in station 8")->Fill(hit->GetX());
	}

	// Process tracks
	Int_t nTracks = (event ? event->GetNofData(kStsTrack)
			               : fTracks->GetEntriesFast());
	for (Int_t iTrack = 0; iTrack < nTracks; iTrack++) {
		Int_t index = (event ? event->GetIndex(kStsTrack, iTrack) : iTrack);
		CbmStsTrack* track = dynamic_cast<CbmStsTrack*>(fTracks->At(index));
		assert(track);
		if ( fFileTracks ) (*fFileTracks) << track->ToString() << "\n";
		// TODO: Have to find out how to extrapolate the track to the target
	}

	// Process primary vertices
	CbmVertex* vertex = (event ? event->GetVertex() : fVertex);
	assert(vertex);
	if ( fFileVertices ) (*fFileVertices) << vertex->ToString() << "\n";
	fHistMan->H1("z PV")->Fill(vertex->GetZ());

	// Event log
	timer.Stop();
	LOG(info) << "+ " << setw(20) << GetName() << ": Event " << setw(6)
	  		      << right << eventNr
	  		      << ", real time " << fixed << setprecision(6)
	  		      << timer.RealTime() << " s, clusters: " << nClusters
	  		      << ", hits: " << nHits << ", tracks: " << nTracks;

}
// -------------------------------------------------------------------------

ClassImp(CbmStsTestQa)

