/** @file CbmStsMC.h
 ** @author Volker Friese <v.friese@gsi.de>
 ** @date 17.02.2014 Major revision: Rename from CbmSts.
 ** Use CbmStsSetup and address scheme, remove GeoPars completely.
 ** Unique address is now set in StsPoint as detectorId.
 **/

#ifndef CBMSTSMC_H
#define CBMSTSMC_H


#include <map>
#include "TClonesArray.h"
#include "FairDetector.h"
#include "FairRootManager.h"
#include "CbmStsTrackStatus.h"

class FairVolume;
class CbmStsPoint;
class CbmStsSetup;
class TGeoNode;
class TGeoCombiTrans;

/** @class CbmStsMC
 ** @brief Class for the MC transport of the CBM-STS
 ** @author V.Friese <v.friese@gsi.de>
 ** @version 2.0
 **
 ** The CbmStsMC defines the behaviour of the STS system during
 ** transport simulation. It constructs the STS transport geometry
 ** and creates objects of type CbmStsPoints.
 **/
class CbmStsMC : public FairDetector
{
	public:

		/** Constructor
		 ** @param active   If set true, ProcessHits will be called and
		 **                 CbmStsPoints will be created.
		 ** @param name     Name of detector object
   	     **/
		CbmStsMC(Bool_t active = kTRUE, const char* name = "STSMC");


		/** Destructor **/
		virtual ~CbmStsMC();


		/** @brief Check whether a volume is sensitive.
		 **
		 ** @param(name)  Volume name
		 ** @value        kTRUE if volume is sensitive, else kFALSE
		 **
		 ** The decision is based on the volume name (has to contain "Sensor").
		 ** Virtual from FairModule.
		 **/
		virtual Bool_t CheckIfSensitive(std::string name) {
			return ( TString(name).Contains("Sensor") ? kTRUE : kFALSE );
		}


		/** @brief Construct the STS geometry in the TGeoManager.
		 **
		 ** Only ROOT geometries are supported. The method
		 ** FairModule::ConstructRootGeometry() is called.
		 ** Virtual from FairModule.
		 **/
		virtual void ConstructGeometry();


		/** @brief Action at end of event
		 **
		 ** Short status log and Reset().
		 ** Virtual from FairDetector.
		 **/
		virtual void EndOfEvent();


		/**  @brief Initialisation
		 **
		 ** The output array is created and the map from full node path
		 ** to unique address is filled from CbmStsSetup. Then, the base
		 ** class method FairDetector::Initialize() is called.
		 ** Virtual from FairDetector.
		 **/
		virtual void Initialize();


		/** @brief Get array of CbmStsPoints
		 ** @param iColl  number of point collection
		 ** @return Pointer to StsPoint array. NULL if iColl > 0.
		 **
		 ** Abstract from FairDetector.
		 **/
		virtual TClonesArray* GetCollection(Int_t iColl) const {
			return ( iColl ? NULL : fStsPoints );
		}


		/** @brief Screen log
		 ** Prints current number of StsPoints in array.
		 ** Virtual from TObject.
		 **/
		virtual void Print(Option_t* opt = "") const;


		/** @brief Action for a track step in a sensitive node of the STS
		 **
		 ** @param vol  Pointer to the active volume
		 ** @return kTRUE
		 **
		 ** The track status is registered when entering or exiting. For all
		 ** steps, the energy loss is accumulated. When the track exits the
		 ** sensitive node (sensor), a CbmStsPoint is created (see CreatePoint()),
		 ** if the total energy loss in the sensor is non-vanishing (e.g., no
		 ** neutral tracks are registered).
		 ** Abstract from FairDetector.
		 **/
		virtual Bool_t ProcessHits(FairVolume* vol = 0);


		/** @brief Register output array (StsPoint) to the I/O manager
		 **
		 ** Abstract from FairDetector.
		 **/
		virtual void Register() {
			FairRootManager::Instance()->Register("StsPoint", GetName(),
							      fStsPoints, kTRUE);
		}


		/** @brief Create StsPoints also for neutral particles
		 ** @param choice  If kTRUE, StsPoints are created also for neutrals
		 **
		 ** By default, StsPoints are only created if there is non-vanishing
		 ** energy loss for the particle in the detector. Neutral particles
		 ** do normally not deposit energy, such that no StsPoints are
		 ** created. For some applications however, e.g. for the calculation
		 ** of the radiation dose, the neutron flux is required. For such
		 ** cases, the creation of StsPoints for neutrals can be activated
		 ** by this method.
		 **/
		void ProcessNeutrals(Bool_t choice = kTRUE) {
		  fProcessNeutrals = choice;
		}


		/** @brief Clear output array and reset current track status
		 **
		 ** Abstract from FairDetector.
		 **/
		virtual void Reset();

    virtual void        ConstructRootGeometry(TGeoMatrix* shift = NULL);
    void                ExpandStsNodes(TGeoNode* fN);
  private:

    CbmStsTrackStatus fStatusIn;   //! Track status at entry of sensor
    CbmStsTrackStatus fStatusOut;  //! Track status at exit of sensor
    Double_t          fEloss;      //! Accumulated energy loss for current track
    std::map<TString, Int_t> fAddressMap;  ///< Map from full path to unique address
    TClonesArray*     fStsPoints;  //!  Output array (CbmStsPoint)
    CbmStsSetup*      fSetup;      //! Pointer to static instance of CbmStsSetup
    TGeoCombiTrans*   fCombiTrans; //! Transformation matrix for geometry positioning
    Bool_t fProcessNeutrals;      ///< Create points also for neutral particles

    /** @brief Create a new StsPoint
     ** Creates a new CbmStsPoint using the current track status information
     ** and adds it to the output TClonesArray.
     **/
    CbmStsPoint* CreatePoint();

    
    /** @brief Set the current track status
     *  Set the current track status (in or out) with parameters obtained
     ** from TVirtualMC (track ID, address, position, momentum, time, length).
     */
    void SetStatus(CbmStsTrackStatus& status);


    /** @brief Check how the TGeoVolume in file was produced
     *  Check how the TGeoVolume in the geometry file was produced.
     *  The new way is to export the volume with the Export function
     *  of TGeoVolume together with a TGeoMatrix.
     *  To identify a file of new type check for TGeoVolume and a TGeoMatrix
     *  derived class in the file.
     */
    Bool_t IsNewGeometryFile(TString filename);


    /** Copy constructor: usage prevented **/
    CbmStsMC(const CbmStsMC&);


    /** Assignment operator: usage prevented **/
    CbmStsMC operator=(const CbmStsMC&);


    ClassDef(CbmStsMC,1);
};

#endif /* CBMSTSMC_H */
