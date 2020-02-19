/** @file CbmStsReco.cxx
 ** @author Volker Friese <v.friese@gsi.de>
 ** @date 11.03.2019
 **/

#include "CbmStsReco.h"

#include "TClonesArray.h"
#include "FairLogger.h"
#include "FairRun.h"
#include "FairRuntimeDb.h"
#include "CbmDigiManager.h"
#include "CbmStsDigitizeParameters.h"
#include "CbmStsFindClusters.h"
#include "CbmStsFindHits.h"
#include "CbmStsDigisToHits.h"
#include "CbmStsFindHitsSingleCluster.h"
#include "CbmStsSetup.h"


// -----   Constructor   ---------------------------------------------------
CbmStsReco::CbmStsReco() :
  FairTask("StsReco", 1),
  fStsRecoMode(1),
  fMode(kCbmTimeslice),
  fUseSingleClusters(kFALSE),
  fSetup(nullptr),
  fDigiPar(nullptr),
  fGlobalPar(),
  fIsSensorConditionsDefault(kTRUE),
  fIsModuleParametersDefault(kTRUE),
	fTimeCutDigisInSigma(3.),
	fTimeCutDigisInNs(-1.),
	fTimeCutClustersInSigma(4.),
	fTimeCutClustersInNs(-1.),
  fSensorsParameterFile(nullptr)
{
}
// -------------------------------------------------------------------------



// -----   Destructor   ----------------------------------------------------
CbmStsReco::~CbmStsReco() {
}
// -------------------------------------------------------------------------



// -----   Hard-code default parameters  -----------------------------------
void CbmStsReco::DefineDefaultParameters() {

  // Sensor conditions
  Double_t vDep   =  70.;    // full-depletion voltage
  Double_t vBias  = 140.;    // bias voltage
  Double_t temp   = 268.;    // temperature
  Double_t cCoup  = 17.5;    // Coupling capacitance
  Double_t cInter =  1.;     // Inter-strip capacitance
  fGlobalPar.SetSensorConditions(vDep, vBias, temp, cCoup, cInter);

  // Module parameters
  Double_t dynRange = 75000.;          // Dynamic range [e]
  Double_t threshold = 3000.;          // Threshold [e]
  Int_t nAdc = 32;                     // Number of ADC channels
  Double_t tResol = 5.;                // Time resolution [ns]
  Double_t deadTime = 800.;            // Single-channel dead time [ns]
  Double_t noise = 1000.;              // Noise RMS [e]
  Double_t zeroNoiseRate = 3.9789e-3;  // Zero-threshold noise rate [1/ns]
  Double_t deadChannelFrac = 0.;       // Fraction of dead channels
  fGlobalPar.SetModuleParameters(dynRange, threshold, nAdc, tResol, deadTime,
                                  noise, zeroNoiseRate, deadChannelFrac);

}
// -------------------------------------------------------------------------



// -----   Initialisation   ------------------------------------------------
InitStatus CbmStsReco::Init() {

	// --- Initialise digi manager
	CbmDigiManager* digiMan = CbmDigiManager::Instance();
	digiMan->Init();

  // --- Something for the screen
  std::cout << std::endl;
  LOG(info) << "==========================================================";
  LOG(info) << GetName() << ": Initialising ";

  // --- Check input branch (StsDigi). If not present, set task inactive.
  if ( ! digiMan->IsPresent(kSts) ) {
    LOG(error) << GetName() << ": No StsDigi input array present; "
        << "task will be inactive.";
    return kERROR;
  }

  // --- In event mode: check input array (Event).
  // --- If not present, set task inactive.
  if ( fMode == kCbmEvent ) {
    auto events = dynamic_cast<TClonesArray*>
    (FairRootManager::Instance()->GetObject("Event"));
    if ( ! events ) {
      LOG(info) << GetName() << ": Event-by-event mode selected, "
          << "but no event branch present.";
      return kFATAL;
    }
  }

  // --- If no parameters are available from the database,
  // --- instantiate the parameter container and set the
  // --- parameters.
  if ( ! fDigiPar ) {
    LOG(info) << GetName() << ": no parameter container from database. "
    		<< "Using default parameters.";
    DefineDefaultParameters();    
    fDigiPar = new CbmStsDigitizeParameters(fGlobalPar);
  }
  
  // --- If parameters are not initialised, use default values
  if ( ! fDigiPar->IsInit() ) {
    LOG(info) << GetName() << ": Parameters not initialised; use default values.";
    fDigiPar->SetDefaults();
  }

  // --- Set physics processes
  CbmStsPhysics::Instance()->SetProcesses(fDigiPar->GetELossModel(),
                                          fDigiPar->GetUseLorentzShift(),
                                          fDigiPar->GetUseDiffusion(),
                                          fDigiPar->GetUseCrossTalk(),
                                          fDigiPar->GetGenerateNoise());
  CbmStsPhysics::Instance()->ShowProcesses();

  // --- Initialise STS setup
  fSetup = CbmStsSetup::Instance();
  fSetup->Init( nullptr, fSensorsParameterFile );
  fSetup->SetSensorConditions(fDigiPar);
  fSetup->SetModuleParameters(fDigiPar);

  // --- Different cases of StsRecoMode

  if(fStsRecoMode==1)  // --- Instantiate DigisToHits without cluster output 
     { 
     LOG(info) << "DigisToHits without cluster output and OpenMP";
     CbmStsDigisToHits* digisToHits = new CbmStsDigisToHits(fMode, kFALSE, kTRUE);
     digisToHits->SetTimeCutDigisInSigma(fTimeCutDigisInSigma);
     if ( fTimeCutDigisInNs >= 0. ) digisToHits->SetTimeCutDigisInNs(fTimeCutDigisInNs);

     if ( ! fUseSingleClusters ) {
       digisToHits->SetTimeCutClustersInNs(fTimeCutClustersInNs);
       digisToHits->SetTimeCutClustersInSigma(fTimeCutClustersInSigma);
       Add(digisToHits);
     }
     else {
       if ( fMode == kCbmTimeslice) Add(new CbmStsFindHitsSingleCluster());
       else LOG(FATAL) << GetName() << ": single-cluster hit finder is not "
           << "available in event-by-event mode";
     }
     }
     
  else if (fStsRecoMode==2)  // --- Instantiate DigisToHits with cluster output 
     {    
     LOG(info) << "DigisToHits with cluster output and without OpenMP";
     CbmStsDigisToHits* digisToHits = new CbmStsDigisToHits(fMode, kTRUE, kFALSE);
     digisToHits->SetTimeCutDigisInSigma(fTimeCutDigisInSigma);
     if ( fTimeCutDigisInNs >= 0. ) digisToHits->SetTimeCutDigisInNs(fTimeCutDigisInNs);

     if ( ! fUseSingleClusters ) {
       digisToHits->SetTimeCutClustersInNs(fTimeCutClustersInNs);
       digisToHits->SetTimeCutClustersInSigma(fTimeCutClustersInSigma);
       Add(digisToHits);
     }
     else {
       if ( fMode == kCbmTimeslice) Add(new CbmStsFindHitsSingleCluster());
       else LOG(FATAL) << GetName() << ": single-cluster hit finder is not "
           << "available in event-by-event mode";
     }
     }
 
  else // --- Instantiate the cluster finder
    {  
    LOG(info) << "DigisToHits starting StsFindClusters and StsFindHits";
    CbmStsFindClusters* findClusters = new CbmStsFindClusters(fMode);
    findClusters->SetTimeCutInSigma(fTimeCutDigisInSigma);
    if ( fTimeCutDigisInNs >= 0. ) findClusters->SetTimeCut(fTimeCutDigisInNs);
    Add(findClusters);

    // --- Instantiate the hit finder
    if ( ! fUseSingleClusters ) {
      CbmStsFindHits* findHits = new CbmStsFindHits(fMode);
      findHits->SetTimeCutInNs(fTimeCutClustersInNs);
      findHits->SetTimeCutInSigma(fTimeCutClustersInSigma);
      Add(findHits);
    }
    else {
      if ( fMode == kCbmTimeslice) Add(new CbmStsFindHitsSingleCluster());
      else LOG(FATAL) << GetName() << ": single-cluster hit finder is not "
          << "available in event-by-event mode";
    }
    }

  return kSUCCESS;
}
// -------------------------------------------------------------------------



// -----   Set parameter containers   --------------------------------------
void CbmStsReco::SetParContainers() {
  FairRuntimeDb* rtdb = FairRun::Instance()->GetRuntimeDb();
  assert(rtdb);
  fDigiPar = static_cast<CbmStsDigitizeParameters*>
    (rtdb->getContainer("CbmStsDigitizeParameters"));
}
// -------------------------------------------------------------------------








ClassImp(CbmStsReco)

