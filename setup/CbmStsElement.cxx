
/** @file CbmStsElement.cxx
 ** @author Volker Friese <v.friese@gsi.de>
 ** @date 27.05.2013
 **/

#include "CbmStsElement.h"

#include <cassert>
#include "TGeoManager.h"
#include "CbmStsModule.h"
#include "CbmStsSetup.h"
#include "CbmStsStation.h"

#include <iomanip>

using std::left;
using std::right;
using std::setw;
using std::pair;
using std::multimap;

// -----   Default constructor   -------------------------------------------
CbmStsElement::CbmStsElement() : TNamed(),
                                 fAddress(0),
                                 fLevel(kStsNofLevels),
                                 fNode(NULL),
                                 fDaughters(),
                                 fMother(NULL)
{
}
// -------------------------------------------------------------------------



// -----   Standard constructor   ------------------------------------------
CbmStsElement::CbmStsElement(Int_t address, Int_t level,
                             TGeoPhysicalNode* node,
                             CbmStsElement* mother) :
    TNamed(),
    fAddress(address),
    fLevel(kStsSystem),
    fNode(node),
    fDaughters(),
    fMother(mother)
{
  SetLevel(level);
  SetName(ConstructName(address, fLevel).Data());
}
// -------------------------------------------------------------------------



// ----- Construct the name of an element   --------------------------------
void CbmStsElement::ConstructName() {

	// Set the name for the STS system
	if ( GetLevel() == kStsSystem ) {
		SetName("STS");
		return;
	}

	// Special case half-ladder ("U"p or "D"own)
	if ( GetLevel() == kStsHalfLadder ) {
		TString label;
		switch ( CbmStsAddress::GetElementId(fAddress, kStsHalfLadder) ) {
			case 0: label = "U"; break;
			case 1: label = "D"; break;
			default: break;
		}
		SetName( fMother->GetName() + label );
		return;
	}

	// For other levels: Expand the name of the mother
	TString label;
	switch ( GetLevel() ) {
		case kStsUnit:   label = "_U"; break;
		case kStsLadder: label = "_L"; break;
		case kStsModule: label = "_M"; break;
		case kStsSensor: label = "_S"; break;
		default: break;
	}
	label += Form("%02i",
			          CbmStsAddress::GetElementId(fAddress, GetLevel()) + 1 );
	SetName( fMother->GetName() + label );

}
// -------------------------------------------------------------------------



// -----   Construct name from address   -----------------------------------
TString CbmStsElement::ConstructName(Int_t address,
                                     EStsElementLevel level) {

  TString result = "STS";
  if ( level >= kStsUnit ) {
    Int_t unit = CbmStsAddress::GetElementId(address, kStsUnit);
    result += Form("_U%02i", unit + 1);
    if ( level >= kStsLadder ) {
      Int_t ladder = CbmStsAddress::GetElementId(address, kStsLadder);
      result += Form("_L%02i", ladder + 1);
      if ( level >= kStsHalfLadder ) {
        Int_t hladder = CbmStsAddress::GetElementId(address, kStsHalfLadder);
        result += (hladder == 0 ? "U" : "D");
        if ( level >= kStsModule ) {
          Int_t module = CbmStsAddress::GetElementId(address, kStsModule);
          result += Form("_M%02i", module + 1);
          if ( level >= kStsSensor ) {
            Int_t sensor = CbmStsAddress::GetElementId(address, kStsSensor);
            result += Form("_S%02i", sensor + 1);
          }  //? sensor
        }  //? module
      }  //? halfladder
    }  //? ladder
  }  //? unit

  return result;
}
// -------------------------------------------------------------------------



// -----   Get a daughter element   ----------------------------------------
CbmStsElement* CbmStsElement::GetDaughter(Int_t index) const {
  if ( index < 0 || index >=GetNofDaughters() ) return NULL;
  return fDaughters[index];
}
// -------------------------------------------------------------------------



// -----   Get number of elements at lower hierarchy levels   --------------
Int_t CbmStsElement::GetNofElements(Int_t level) const {

	Int_t nElements = 0;
	if ( level <= fLevel ) nElements = 0;
	else if ( level == fLevel + 1) nElements = GetNofDaughters();
	else
		for (Int_t iDaughter = 0; iDaughter < GetNofDaughters(); iDaughter++)
			nElements += GetDaughter(iDaughter)->GetNofElements(level);

	return nElements;
}
// -------------------------------------------------------------------------



// -----   Recursively read daughters from geometry   ----------------------
void CbmStsElement::InitDaughters() {

  // --- Catch absence of TGeoManager
  assert( gGeoManager );

  // --- No daughter elements below sensor level
  if ( fLevel > kStsSensor ) return;

  // --- Catch physical node not being set
  if ( ! fNode ) {
    LOG(error) << fName << ": physical node is not set!";
    return;
  }

  TGeoNode* mNode = fNode->GetNode();   // This node
  TString   mPath = fNode->GetName();   // Full path to this node

  for (Int_t iNode = 0; iNode < mNode->GetNdaughters(); iNode++) {

    // Check name of daughter node for level name
    TString dName = mNode->GetDaughter(iNode)->GetName();
    if ( dName.Contains(CbmStsSetup::Instance()->GetLevelName(fLevel + 1),
                        TString::kIgnoreCase ) ) {

      // Create physical node
      TString dPath = mPath + "/" + dName;
      TGeoPhysicalNode* pNode = new TGeoPhysicalNode(dPath.Data());

      // Create element and add it as daughter
      UInt_t address = CbmStsAddress::SetElementId(fAddress,
                                                   fLevel + 1,
                                                   GetNofDaughters());
      CbmStsElement* dElement = NULL;
      switch ( fLevel) {
      	case kStsHalfLadder:
      		dElement = new CbmStsModule(address, pNode, this); break;
     	default:
      		dElement = new CbmStsElement(address, fLevel+1, pNode, this); break;
      }
      fDaughters.push_back(dElement);

      // Call init method recursively for the daughter elements
      dElement->InitDaughters();

    } // name of daughter node

  } // daughter node loop

}
// -------------------------------------------------------------------------



// -----   Print   ---------------------------------------------------------
void CbmStsElement::Print(Option_t* opt) const {
  LOG(info) << setw(10) << right << fAddress << "  "
		    << setw(12) << left << fName
		    << "  type " << setw(22) << fTitle << "  path "
		    << fNode->GetName() << "  " << fNode->GetTitle();
  if ( opt[0] == 'R' ) {
	  for (Int_t iDaughter = 0; iDaughter < GetNofDaughters(); iDaughter++)
		  GetDaughter(iDaughter)->Print("R");
  }
}
// -------------------------------------------------------------------------



// -----   Set element level   ---------------------------------------------
void CbmStsElement::SetLevel(Int_t level) {
  switch (level) {
    case kStsSystem:     fLevel = kStsSystem;     break;
    case kStsUnit   :    fLevel = kStsUnit;       break;
    case kStsLadder:     fLevel = kStsLadder;     break;
    case kStsHalfLadder: fLevel = kStsHalfLadder; break;
    case kStsModule:     fLevel = kStsModule;     break;
    case kStsSensor:     fLevel = kStsSensor;     break;
    default: LOG(fatal) << fName << ": Illegal element level "
    		<< level; break;
  }
}
// -------------------------------------------------------------------------


ClassImp(CbmStsElement)

