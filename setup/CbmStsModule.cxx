/** @file CbmStsModule.cxx
 ** @author Volker Friese <v.friese@gsi.de>
 ** @date 14.05.2013
 **/

#include "CbmStsModule.h"

#include <cassert>
#include <cmath>
#include "TClonesArray.h"
#include "TGeoManager.h"
#include "TRandom.h"
#include "TString.h"
#include "FairLogger.h"
#include "FairRunAna.h"
#include "CbmMatch.h"
#include "CbmStsAddress.h"
#include "CbmStsCluster.h"
#include "CbmStsDigi.h"
#include "CbmStsDigitize.h"
#include "CbmStsSensorDssd.h"
#include "CbmStsSetup.h"

using namespace std;

// -----   Default constructor   -------------------------------------------
CbmStsModule::CbmStsModule(UInt_t address, TGeoPhysicalNode* node,
                           CbmStsElement* mother) :
        CbmStsElement(address, kStsModule, node, mother),
        fNofChannels(2048),
        fIsSet(kFALSE),
        // fDeadChannels(),
        fAnalogBuffer(),
        fClusters()
{
}
// -------------------------------------------------------------------------



// --- Destructor   --------------------------------------------------------
CbmStsModule::~CbmStsModule() {

  // --- Clean analog buffer
  for (auto chanIt = fAnalogBuffer.begin(); chanIt != fAnalogBuffer.end();
      chanIt++) {
    for ( auto sigIt = (chanIt->second).begin(); sigIt != (chanIt->second).end();
        sigIt++) {
      delete (*sigIt);
    }
  }
}
// -------------------------------------------------------------------------



// -----   Convert ADC channel to analogue charge (channel mean)   ---------
Double_t CbmStsModule::AdcToCharge(UShort_t adc, UShort_t channel) {
  auto& asic = GetAsicParameters(channel);
  return asic.GetThreshold() + asic.GetDynRange() / Double_t(asic.GetNofAdc()) *
      ( Double_t(adc) + 0.5 );
}
// -------------------------------------------------------------------------



// -----   Add a signal to the buffer   ------------------------------------
void CbmStsModule::AddSignal(UShort_t channel, Double_t time,
                             Double_t charge, Int_t index, Int_t entry,
                             Int_t file) {
  // --- Check channel number
  assert( channel < fNofChannels );

  // --- Debug
  LOG(debug3) << GetName() << ": Receiving signal " << charge
      << " in channel " << channel << " at time " << time << " s";

  // --- Discard charge if the channel is dead
  if ( !IsChannelActive(channel) ) {
    LOG(debug) << GetName() << ": discarding signal in dead channel "
        << channel;
    return;
  }

  // --- If the channel is not yet active: create a new set and insert
  // --- new signal into it.
  if ( fAnalogBuffer.find(channel) == fAnalogBuffer.end() ) {
    CbmStsSignal* signal = new CbmStsSignal(time, charge,
                                            index, entry, file);
    fAnalogBuffer[channel].insert(signal);
    LOG(debug4) << GetName() << ": Activating channel " << channel;
    return;
  }  //? Channel not yet active

  // --- The channel is active: there are already signals in.
  // --- Loop over all signals in the channels and compare their time.
  //TODO: Loop over all signals is not needed, since they are time-ordered.
  Bool_t isMerged = kFALSE;
  sigset::iterator it;
  auto& asic = GetAsicParameters(channel);
  for (it = fAnalogBuffer[channel].begin();
      it != fAnalogBuffer[channel].end(); it++) {

    // Time between new and old signal smaller than dead time: merge signals
    if ( TMath::Abs( (*it)->GetTime() - time ) < asic.GetDeadTime() ) {

      // Current implementation of merging signals:
      // Add charges, keep first signal time
      // TODO: Check with STS electronics people on more realistic behaviour.
      LOG(debug4) << GetName() << ": channel " << channel
          << ", new signal at t = " << time
          << " ns is merged with present signal at t = "
          << (*it)->GetTime() << " ns";
      (*it)->SetTime( TMath::Min( (*it)->GetTime(), time) );
      (*it)->AddLink(charge, index, entry, file);
      isMerged = kTRUE;  // mark new signal as merged
      LOG(debug4) << "    New signal: time " << (*it)->GetTime()
                      << ", charge " << (*it)->GetCharge()
                      << ", number of links " << (*it)->GetMatch().GetNofLinks();
      break;  // Merging should be necessary only for one buffer signal

    } //? Time difference smaller than dead time

  } // Loop over signals in buffer for this channel

  // --- If signal was merged: no further action
  if ( isMerged ) return;

  // --- Arriving here, the signal did not interfere with existing ones.
  // --- So, it is added to the analog buffer.
  CbmStsSignal* signal = new CbmStsSignal(time, charge,
                                          index, entry, file);
  fAnalogBuffer[channel].insert(signal);
  LOG(debug4) << GetName() << ": Adding signal at t = " << time
      << " ns, charge " << charge << " in channel " << channel;

}
// -------------------------------------------------------------------------



// -----   Status of analogue buffer   -------------------------------------
void CbmStsModule::BufferStatus(Int_t& nofSignals,
                                Double_t& timeFirst,
                                Double_t& timeLast) {


  Int_t nSignals   = 0;
  Double_t tFirst  = -1.;
  Double_t tLast   = -1.;
  Double_t tSignal = -1.;

  // --- Loop over active channels
  for ( auto chanIt  = fAnalogBuffer.begin();
        chanIt != fAnalogBuffer.end(); chanIt++) {

    // --- Loop over signals in channel
    for ( auto sigIt  = (chanIt->second).begin();
        sigIt != (chanIt->second).end(); sigIt++) {

      tSignal = (*sigIt)->GetTime();
      nSignals++;
      tFirst = tFirst < 0. ? tSignal : TMath::Min(tFirst, tSignal);
      tLast  = TMath::Max(tLast, tSignal);

    } // signals in channel

  } // channels in module

  nofSignals = nSignals;
  timeFirst  = tFirst;
  timeLast   = tLast;

}
// -------------------------------------------------------------------------



// -----   Convert analog charge to ADC channel number   -------------------
Int_t CbmStsModule::ChargeToAdc(Double_t charge, UShort_t channel) {
  auto& asic = GetAsicParameters(channel);

  if ( charge < asic.GetThreshold() ) return -1;
  Int_t adc = Int_t ( (charge - asic.GetThreshold()) * Double_t(asic.GetNofAdc())
                      / asic.GetDynRange() );
  return ( adc < asic.GetNofAdc() ? adc : asic.GetNofAdc() - 1 );
}
// -------------------------------------------------------------------------



// -----   Digitise an analogue charge signal   ----------------------------
void CbmStsModule::Digitize(UShort_t channel, CbmStsSignal* signal) {

  // --- Check channel number
  assert ( channel < fNofChannels );

  auto& asic = GetAsicParameters(channel);

  // --- No action if charge is below threshold
  Double_t charge = signal->GetCharge();
  if ( charge < asic.GetThreshold() ) return;

  // --- Digitise charge
  // --- Prescription according to the information on the STS-XYTER
  // --- by C. Schmidt.
  UShort_t adc = (UShort_t)ChargeToAdc(charge, channel);

  // --- Digitise time
  Double_t  deltaT = gRandom->Gaus(0., asic.GetTimeResolution());
  Long64_t dTime = Long64_t(round(signal->GetTime() + deltaT));

  // --- Send the message to the digitiser task
  LOG(debug4) << GetName() << ": charge " << signal->GetCharge()
                  << ", dyn. range " << asic.GetDynRange() << ", threshold "
                  << asic.GetThreshold() << ", # ADC channels "
                  << asic.GetNofAdc();
  LOG(debug3) << GetName() << ": Sending message. Channel " << channel
      << ", time " << dTime << ", adc " << adc;
  CbmStsDigitize* digitiser = CbmStsSetup::Instance()->GetDigitizer();
  if ( digitiser ) digitiser->CreateDigi(fAddress, channel, dTime, adc,
                                         signal->GetMatch());

  // --- If no digitiser task is present (debug mode): create a digi and
  // --- add it to the digi buffer.
  else {
    LOG(fatal) << GetName() << ": no digitiser task present!";
  }
  return;
}
// -------------------------------------------------------------------------



// -----   Find hits   -----------------------------------------------------
Int_t CbmStsModule::FindHits(std::vector<CbmStsHit>* hitArray, CbmEvent* event,
                             Double_t tCutInNs, Double_t tCutInSigma) {

  // --- Call FindHits method in each daughter sensor
  Int_t nHits = 0;
  for (Int_t iSensor = 0; iSensor < GetNofDaughters(); iSensor++) {
    CbmStsSensor* sensor = dynamic_cast<CbmStsSensor*>(GetDaughter(iSensor));
    nHits += sensor->FindHits(fClusters, hitArray, event,
                              tCutInNs, tCutInSigma);
  }
  LOG(debug2) << GetName() << ": Clusters " << fClusters.size()
                  << ", sensors " << GetNofDaughters() << ", hits "
                  << nHits;
  return nHits;
}
// -------------------------------------------------------------------------


// -----   Find hits   -----------------------------------------------------
Int_t CbmStsModule::MakeHitsFromClusters(std::vector<CbmStsHit>* hitArray,
                                         CbmEvent* event) {

  // --- Call MakeHits method in each daughter sensor
  Int_t nHits = 0;
  for (Int_t iSensor = 0; iSensor < GetNofDaughters(); iSensor++) {
    CbmStsSensor* sensor = dynamic_cast<CbmStsSensor*>(GetDaughter(iSensor));
    nHits += sensor->MakeHitsFromClusters(fClusters, hitArray, event);
  }

  LOG(debug2) << GetName() << ": Clusters " << fClusters.size()
                        << ", sensors " << GetNofDaughters() << ", hits "
                        << nHits;
  return nHits;
}
// -------------------------------------------------------------------------



// -----   Generate noise   ------------------------------------------------
Int_t CbmStsModule::GenerateNoise(Double_t t1, Double_t t2) {

  assert( t2 > t1 );

  Int_t fnNoise = 0;

  Int_t iAsic = 0;
  for (auto& asic : fAsicParameterVector) {
    Int_t channel = Int_t(gRandom->Uniform(Double_t(kiNbAsicChannels)));

    // --- Mean number of digis in [t1, t2]
    Double_t nNoiseMean = asic.GetNoiseRate() * kiNbAsicChannels * ( t2 - t1 );

    // --- Sample number of noise digis
    Int_t nNoise = gRandom->Poisson(nNoiseMean);

    // --- Create noise digis
    for (Int_t iNoise = 0; iNoise < nNoise; iNoise++) {

      // --- Random channel number, time and charge
      Double_t time = gRandom->Uniform(t1, t2);
      Double_t charge = asic.GetNoiseCharge()->GetRandom();

      // --- Insert a signal object (without link index, entry and file)
      // --- into the analogue buffer.
      AddSignal(iAsic * kiNbAsicChannels + channel, time, charge, -1, -1, -1);
    } //# noise digis
    iAsic++;
    fnNoise += nNoise;
  }

  return fnNoise;
}
// -------------------------------------------------------------------------



// -----   Get the unique address from the sensor name (static)   ----------
Int_t CbmStsModule::GetAddressFromName(TString name) {

  Bool_t isValid = kTRUE;
  if ( name.Length() != 16 ) isValid = kFALSE;
  if ( isValid) {
    if ( ! name.BeginsWith("STS") ) isValid = kFALSE;
    if ( name[4] != 'U' )  isValid = kFALSE;
    if ( name[8] != 'L' )  isValid = kFALSE;
    if ( name[13] != 'M' ) isValid = kFALSE;
  }
  if ( ! isValid ) {
    LOG(fatal) << "GetAddressFromName: Not a valid module name "
        << name;
    return 0;
  }

  Int_t unit    = 10 * ( name[5]  - '0') + name[6]  - '0' - 1;
  Int_t ladder  = 10 * ( name[9]  - '0') + name[10] - '0' - 1;
  Int_t hLadder = ( name[11] == 'U' ? 0 : 1);
  Int_t module  = 10 * ( name[14] - '0') + name[15] - '0' - 1;

  return CbmStsAddress::GetAddress(unit, ladder, hLadder, module);
}
// -------------------------------------------------------------------------


// -----  Initialise the analogue buffer   ---------------------------------
std::set<UShort_t> CbmStsModule::GetSetOfDeadChannels() const {
  std::set<UShort_t> deadChannels;
  for (size_t iAsic = 0; iAsic < fAsicParameterVector.size(); iAsic++) {
    auto& asic = fAsicParameterVector.at(iAsic);
    for (auto channel : asic.GetDeadChannelMap()) {
      deadChannels.insert(iAsic * kiNbAsicChannels + channel);
    }
  }
  return deadChannels;
}
// -------------------------------------------------------------------------

// -----  Initialise the analogue buffer   ---------------------------------
void CbmStsModule::InitAnalogBuffer() {

  for (UShort_t channel = 0; channel < fNofChannels; channel++) {
    multiset<CbmStsSignal*, CbmStsSignal::Before> mset;
    fAnalogBuffer[channel] = mset;
  } // channel loop

}
// -------------------------------------------------------------------------



// -----   Initialise daughters from geometry   ----------------------------
void CbmStsModule::InitDaughters() {

  // --- Catch absence of TGeoManager
  assert( gGeoManager );

  // --- Catch physical node not being set
  assert ( fNode);

  TGeoNode* moduleNode = fNode->GetNode();   // This node
  TString   modulePath = fNode->GetName();   // Full path to this node

  for (Int_t iNode = 0; iNode < moduleNode->GetNdaughters(); iNode++) {

    // Check name of daughter node for level name
    TString daughterName = moduleNode->GetDaughter(iNode)->GetName();
    if ( daughterName.Contains("Sensor", TString::kIgnoreCase) ) {

      // Create physical node
      TString daughterPath = modulePath + "/" + daughterName;
      TGeoPhysicalNode* sensorNode = new TGeoPhysicalNode(daughterPath.Data());

      // Get or create element from setup and add it as daughter
      Int_t address = CbmStsAddress::SetElementId(fAddress,
                                                   kStsSensor,
                                                   GetNofDaughters());
      CbmStsSensor* sensor = CbmStsSetup::Instance()->AssignSensor(address,
                                                                   sensorNode);
      sensor->SetMother(this);
      fDaughters.push_back(sensor);

    } //? name of daughter node contains "sensor"

  } //# daughter nodes

  // Set number of channels, which depends on the connected sensor.
  // For sensor DssdStereo, it is 2 * number of strips
  assert(GetDaughter(0)); // check whether a sensor is attached
  TString dType = GetDaughter(0)->GetTitle();
  if ( dType.BeginsWith("Dssd") ) {
    CbmStsSensorDssd* sensor =
        dynamic_cast<CbmStsSensorDssd*>(GetDaughter(0));
    Int_t nStripsF = sensor->GetNofStrips(0); // front side
    Int_t nStripsB = sensor->GetNofStrips(1); // back side
    fNofChannels = 2 * TMath::Max(nStripsF, nStripsB);
  }
  else {
    LOG(fatal) << GetName() << ": No sensor connected!";
    return;
  }

  Int_t nAsics = fNofChannels/kiNbAsicChannels;
  fAsicParameterVector.resize(nAsics);
}
// -------------------------------------------------------------------------



// -----   Process the analogue buffer   -----------------------------------
Int_t CbmStsModule::ProcessAnalogBuffer(Double_t readoutTime) {

  // --- Counter
  Int_t nDigis = 0;

  // Create iterators needed for inner loop
  sigset::iterator sigIt;;
  sigset::iterator oldIt;
  sigset::iterator endIt;

  // --- Iterate over active channels
  for(auto& chanIt: fAnalogBuffer) {

    // Only do something if there are signals for the channel
    if ( ! (chanIt.second).empty() ) {
      auto& asic = GetAsicParameters(chanIt.first);

      // --- Time limit up to which signals are digitised and sent to DAQ.
      // --- Up to that limit, it is guaranteed that future signals do not
      // --- interfere with the buffered ones. The readoutTime is the time
      // --- of the last processed StsPoint. All coming points will be later
      // --- in time. So, the time limit is defined by this time minus
      // --- 5 times the time resolution (maximal deviation of signal time
      // --- from StsPoint time) minus the dead time, within which
      // --- interference of signals can happen.
      Double_t timeLimit = readoutTime - 5. * asic.GetTimeResolution() - asic.GetDeadTime();

      // --- Digitise all signals up to the specified time limit
      sigIt = (chanIt.second).begin();
      oldIt = sigIt;
      endIt = (chanIt.second).end();
      while ( sigIt != endIt ) {

        // --- Exit loop if signal time is larger than time limit
        // --- N.b.: Readout time < 0 means digitise everything
        if ( readoutTime >= 0. && (*sigIt)->GetTime() > timeLimit ) break;

        // --- Digitise signal
        Digitize( chanIt.first, (*sigIt) );
        nDigis++;

        // --- Increment iterator before it becomes invalid
        oldIt = sigIt;
        sigIt++;

        // --- Delete digitised signal
        delete (*oldIt);
        (chanIt.second).erase(oldIt);
      } // Iterate over signals in channel
    } // if there are signals
  } // Iterate over channels

  return nDigis;
}
// -------------------------------------------------------------------------



// -----   Set the module parameters   -------------------------------------
void CbmStsModule::SetParameters(Double_t dynRange, Double_t threshold,
                                 Int_t nAdc, Double_t timeResolution,
                                 Double_t deadTime, Double_t noise,
                                 Double_t zeroNoiseRate,
                                 Double_t fracDeadChannels,
                                 std::set<UChar_t> deadChannelMap) {

  for (auto& asic : fAsicParameterVector) {
    asic.SetModuleParameters(dynRange, threshold, nAdc, timeResolution, deadTime,
                             noise, zeroNoiseRate, fracDeadChannels, deadChannelMap);
  }

  // Initialise the analogue buffer
  InitAnalogBuffer();

  // Mark the module initialised
  fIsSet          = kTRUE;
}
// -------------------------------------------------------------------------



// // -----   Create list of dead channels   ----------------------------------
// Int_t CbmStsModule::SetDeadChannels(Double_t percentage) {

//   Double_t fraction = percentage;

//   // --- Catch illegal percentage values
//   if ( percentage < 0. ) {
//     LOG(warn) << GetName() << ": illegal percentage of dead channels "
//         << percentage << ", is set to 0.";
//     fraction = 0.;
//   }
//   if ( percentage > 100. ) {
//     LOG(warn) << GetName() << ": illegal percentage of dead channels "
//         << percentage << ", is set to 100.";
//     fraction = 100.;
//   }

//   // --- Re-set dead channel list
//   fDeadChannels.clear();

//   // --- Number of dead channels
//   UInt_t nOfDeadChannels = fraction * fNofChannels / 100;

//   // --- Case percentage < 50: randomise inactive channels
//   // --- N.b.: catches also zero fraction (nOfDeadChannels = 0)
//   // --- N.b.: set::insert has no effect if element is already present
//   if ( nOfDeadChannels < (fNofChannels / 2) ) {
//     while ( fDeadChannels.size() < nOfDeadChannels )
//       fDeadChannels.insert( Int_t( gRandom->Uniform(fNofChannels) ) );
//   }

//   // --- Case percentage > 50: randomise active channels
//   // --- N.b.: catches also unity fraction (nOfDeadChannels = fNofChannels)
//   // --- N.b.: set::erase has no effect if element is not present
//   else {
//     for (Int_t channel = 0; channel < fNofChannels; channel++)
//       fDeadChannels.insert(channel);
//     while ( fDeadChannels.size() > nOfDeadChannels )
//       fDeadChannels.erase( Int_t ( gRandom->Uniform(fNofChannels) ) );
//   }

//   return fDeadChannels.size();
// }
// // -------------------------------------------------------------------------


// // -----   Create list of dead channels from std::set   --------------------
// Int_t CbmStsModule::SetDeadChannels(std::set <UShort_t>& deadChannels) {
//   fDeadChannels = deadChannels;

//   return fDeadChannels.size();
// }
// // -------------------------------------------------------------------------


// -----   String output   -------------------------------------------------
string CbmStsModule::ToString() const {
    stringstream ss;
    // sloppy cast, needs better way for output
    auto& asic = const_cast<CbmStsModule*>(this)->GetAsicParameters(0);
    ss << "Module  " << GetName() << ": dynRange " << asic.GetDynRange()
       << "e, thresh. " << asic.GetThreshold() << "e, nAdc " << asic.GetNofAdc()
       << ", time res. " << asic.GetTimeResolution() << "ns, dead time "
       << asic.GetDeadTime() << "ns, noise " << asic.GetNoise() << "e, zero noise rate "
       << asic.GetZeroNoiseRate() << "/ns";
       // <<", dead chan. " << fDeadChannels.size()
       // << " / " << fNofChannels;
    ss << " " << CbmStsAddress::ToString(fAddress);
    return ss.str();
}
// -------------------------------------------------------------------------


ClassImp(CbmStsModule)
