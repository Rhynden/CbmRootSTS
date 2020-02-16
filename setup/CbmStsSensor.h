/** @file CbmStsSensor.h
 ** @author Volker Friese <v.friese@gsi.de>
 ** @date 03.05.2013
 **
 ** This class is to replace the CbmStsSensor, at which point it will be
 ** renamed to spell correctly.
 **/

#ifndef CBMSTSSENSOR_H
#define CBMSTSSENSOR_H 1


#include <vector>
#include <string>
#include "CbmStsAddress.h"
#include "CbmStsCluster.h"
#include "CbmStsHit.h"
#include "CbmStsElement.h"
#include "CbmStsSensorConditions.h"

class TClonesArray;
class TGeoPhysicalNode;
class CbmEvent;
class CbmLink;
class CbmStsModule;
class CbmStsPoint;
class CbmStsSensorPoint;


/** @class CbmStsSensor
 ** @brief Class representing an instance of a sensor in the CBM-STS.
 ** @author V.Friese <v.friese@gsi.de>
 ** @version 2.0
 **
 ** The sensor is the smallest geometric element in the STS setup.
 ** It is the daughter node of a module, which may contain one sensor
 ** or several daisy-chained ones. The sensor class represents
 ** the physical node through its member fNode, but also performs detector
 ** response simulation through its member fType.
 ** After being instantiated by the initialisation of CbmStsSetup,
 ** the sensor type must be assigned to the sensor. This is currently done
 ** by the digitiser task.
 ** The sensor class performs the coordinate transformation from the
 ** global system to the sensor system, having the sensor midpoint as origin.
 ** The analog response is then modelled by the pure virtual method
 ** CalulateResponse.
 **/
class CbmStsSensor : public CbmStsElement
{

  public:

    /** Constructor
     ** @param address Unique element address
     ** @param node    Pointer to geometry node
     ** @param mother  Pointer to mother element (module)
     **/
    CbmStsSensor(UInt_t address = 0, TGeoPhysicalNode* node = nullptr,
                 CbmStsElement* mother = nullptr);


    /** Destructor  **/
    virtual ~CbmStsSensor() { };


    /** Create a new hit in the output array from two clusters
     ** @param xLocal   hit x coordinate in sensor system [cm]
     ** @param yLocal   hit y coordinate in sensor system [cm]
     ** @param varX     Variance in x [cm^2]
     ** @param varY     Variance in y [cm^2]
     ** @param varXY    Covariance of x and y [cm^2]
     ** @param clusterF pointer to front side cluster
     ** @param clusterB pointer to back side cluster
     ** @param du       Error in u coordinate (across strips front side) [cm]
     ** @param dv       Error in v coordinate (across strips back side) [cm]
     **/
    void CreateHit(Double_t xLocal, Double_t yLocal,
    		       Double_t varX, Double_t varY, Double_t varXY,
    		       CbmStsCluster* clusterF, CbmStsCluster* clusterB,
    		       Double_t du = 0., Double_t dv = 0.);


    /** Find hits in sensor
     ** @param clusters  Vector of clusters
     ** @param hitArray  TClonesArray to store the hits in
     ** @param event     Pointer to current event for registering of hits
     ** @param tCutInNs     Max. time difference of clusters in a hit in ns
     ** @param tCutInSigma  Max. time difference of clusters in multiples of error
     ** @return Number of created hits
     **
     ** If deltaTinNs is positive, the respective time cut on the time
     ** difference between the two clusters is applied. Else, the cut is set
     ** to deltaTinSigma times the error of the time difference,
     ** which is calculated from the cluster time errors.
     ** If both tCutInNs and tCutInSigma are negative, no time cut is applied.
     **/
    virtual Int_t FindHits(std::vector<CbmStsCluster*>& clusters,
                           std::vector<CbmStsHit>* hitArray, CbmEvent* event,
                           Double_t tCutInNs, Double_t tCutInSigma) = 0;


    /** @brief Get the address from the sensor name (static)
     ** @param name Name of sensor
     ** @value Unique element address
     **/
    static UInt_t GetAddressFromName(TString name);


    /** Sensor conditions
     ** @return Pointer to sensor condition object
     **/
    const CbmStsSensorConditions* GetConditions() const { return fConditions; }


    /** Current link object
     ** @return Pointer to current link object (to CbmStsPoint)
     **/
    CbmLink* GetCurrentLink() const { return fCurrentLink; }


    /** Get mother module **/
    CbmStsModule* GetModule() const;


  	/** Get physical node
  	 ** @return Pointer to TGeoPhysicalNode of sensor
     **/
  	TGeoPhysicalNode* GetNode() const {return fNode;}


  	/** @brief Initialise the sensor, if needed
  	 ** @return kTRUE is successfully initialised
  	 **
  	 ** The implementation depends on the concrete sensor class.
  	 **/
  	virtual Bool_t Init() { return kTRUE; }


    /** Get the sensor Id within the module  **/
    Int_t GetSensorId() const {
      return CbmStsAddress::GetElementId(fAddress, kStsSensor); }


    /** Make hits from single clusters in the sensor
     ** @param clusters  Vector of clusters
     ** @param hitArray  TClonesArray to store the hits in
     ** @param event     Pointer to current event for registering of hits
     ** @return Number of created hits
     **/
    virtual Int_t MakeHitsFromClusters(std::vector<CbmStsCluster*>& clusters,
                                       std::vector<CbmStsHit>* hitArray,
                                       CbmEvent* event) = 0;


    /** Process one MC Point
     ** @param point  Pointer to CbmStsPoint object
     ** @return  Status variable, depends on sensor type
     **
     ** The point coordinates are converted into the internal coordinate
     ** system. The appropriate analogue response is then calculated
     ** with the pure virtual method CalculateResponse.
     **/
    Int_t ProcessPoint(const CbmStsPoint* point,
    		               Double_t eventTime = 0., CbmLink* link = NULL);


    /** @brief Set sensor address
     ** @param address STS element address
     **/
    void SetAddress(Int_t address) {
      fAddress = address;
      fName = CbmStsElement::ConstructName(address, kStsSensor);
    }


     /** Set the sensor conditions
      ** @param vFD           Full depletion voltage [V]
      ** @param vBias         Bias voltage [V]
      ** @param temperature   Temperature [K]
      ** @param cCoupling     Coupling capacitance [pF]
      ** @param cInterstrip   Inter-strip capacitance [pF]
      ** @param bX            Magn. field Bx at sensor centre [T]
      ** @param bY            Magn. field By at sensor centre [T]
      ** @param bZ            Magn. field Bz at sensor centre [T]
      **/
     void SetConditions(Double_t vFD, Double_t vBias, Double_t temperature,
                        Double_t cCoupling, Double_t cInterstrip,
                        Double_t bX, Double_t bY, Double_t bZ) {
       fConditions = new CbmStsSensorConditions(vFD, vBias, temperature,
                                                cCoupling, cInterstrip,
                                                bX, bY, bZ);
     }


    /** @brief Set the physical node
     ** @param node  Pointer to associated TGeoPhysicalNode object
     **/
    void SetNode(TGeoPhysicalNode* node) { fNode = node; }


    /** String output **/
    virtual std::string ToString() const = 0;


  protected:

    CbmStsSensorConditions*  fConditions;  ///< Operating conditions
    CbmLink* fCurrentLink;  ///< Link to currently processed MCPoint
    std::vector<CbmStsHit>* fHits;    ///< Output array for hits. Used in hit finding.
    CbmEvent* fEvent;       //! ///< Pointer to current event


    /** Perform response simulation for one MC Point
     ** @param point   Pointer to CbmStsSensorPoint with relevant parameters
     ** @return  Status variable, depends on concrete type
     **
     ** Perform the appropriate action for a particle trajectory in the
     ** sensor characterised by the CbmStsSensorPoint object. This is specific
     ** to the sensor type and has to be implemented in the derived class.
     **/
    virtual Int_t CalculateResponse(CbmStsSensorPoint* point) = 0;


    /** Prevent usage of copy constructor and assignment operator **/
    CbmStsSensor(const CbmStsSensor&) = delete;
    CbmStsSensor& operator=(const CbmStsSensor&) = delete;


    ClassDef(CbmStsSensor,2);

};


#endif
