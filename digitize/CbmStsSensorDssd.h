/** @file CbmStsSensorDssd.h
 ** @author Volker Friese <v.friese@gsi.de>
 ** @date 07.08.2017
 **/

#ifndef CBMSTSSENSORDSSD_H
#define CBMSTSSENSORDSSD_H 1


#include <string>
#include <utility>
#include "TArrayD.h"
#include "CbmStsSensor.h"

class CbmStsPhysics;


/** @class CbmStsSensorDssd
 ** @brief Class describing double-sided silicon strip sensors.
 ** @author V.Friese <v.friese@gsi.de>
 ** @version 1.0
 **
 ** Abstract class describes the (analogue) response of double-sided silicon
 ** strip sensors in the STS.
 **
 ** The active area does not necessarily coincide with the geometric
 ** dimensions of the sensor. It is, however, centred in the latter,
 ** meaning that the width of inactive regions (guard ring) are
 ** the same on the left and on the right side and also the same at
 ** the top and and the bottom.
 **
 ** The response to charged particles is modelled by a uniform charge
 ** distribution along the particle trajectory in the active volume,
 ** which is projected to the readout edge, where it is discretised
 ** on the active strips. The charge is then delivered to the corresponding
 ** channel of the readout module (CbmStsModule).
 **
 ** This is an abstract class, since additional functionality, depending
 ** on where (on which edge) the readout is done. Derived classes have to
 ** implement the pure virtual method PropagateCharge, which has to
 ** properly fill the charge arrays fStripCharge for front and back side,
 ** along with the auxiliary method Diffusion for the thermal diffusion
 ** along the drift to the readout plane. Also, the mapping from the strip
 ** numbers to the (module) channel number has to be implemented in
 ** GetModuleChannel and GetStrip. These methods will e.g. be different
 ** for sensors with stereo angles, where both sides are read out at the
 ** same edge, and for sensor with orthogonal strips, where the back plane
 ** is read out at the left or right edge.
 **/
class CbmStsSensorDssd : public CbmStsSensor
{

  public:

    /** Constructor
     ** @param address Unique element address
     ** @param node    Pointer to geometry node
     ** @param mother  Pointer to mother element (module)
     **/
    CbmStsSensorDssd(Int_t address = 0, TGeoPhysicalNode* node = nullptr,
                     CbmStsElement* mother = nullptr);

     /** Destructor  **/
    virtual ~CbmStsSensorDssd() { };


    /** @brief Find hits from clusters
     ** @param clusters  Vector of clusters
     ** @param hitArray  TClonesArray to store the hits in
     ** @param event     Pointer to current event for registering of hits
     ** @param tCutInNs     Max. time difference of clusters in a hit in ns
     ** @param tCutInSigma  Max. time difference of clusters in multiples of error
     ** @return Number of created hits
     **
     ** This method implements the hit finding. Hits are geometric
     ** intersections of a cluster on the front side with a cluster on the
     ** back side. If tCutInNs is positive, the respective time cut on the time
     ** difference between the two clusters is applied. Else, the cut is set
     ** to tCutInSigma times the error of the time difference,
     ** which is calculated from the cluster time errors.
     ** If both tCutInNs and tCutInSigma are negative, no time cut is applied.
     **/
    virtual Int_t FindHits(std::vector<CbmStsCluster*>& clusters,
                           TClonesArray* hitArray, CbmEvent* event,
													 Double_t tCutInNs, Double_t tCutInSigma);

        virtual Int_t FindHitsVector(std::vector<CbmStsCluster*>& clusters,
                                 std::vector<CbmStsHit>* hitArray, CbmEvent* event,
                                 Double_t tCutInNs,
																 Double_t tCutInSigma);


    /** @brief Number of strips on front and back side
     ** @param side  0 = front side, 1 = back side
     ** @value Number of strips on the specified sensor side
     **/
    virtual Int_t GetNofStrips(Int_t side) const = 0;


    /** @brief Strip pitch on front and back side
     ** @param side  0 = front side, 1 = back side
     ** @value Strip pitch [cm] on the specified sensor side
     **/
    virtual Double_t GetPitch(Int_t side) const = 0;


    /** Make hits from single clusters in the sensor
     ** @param clusters  Vector of clusters
     ** @param hitArray  TClonesArray to store the hits in
     ** @param event     Pointer to current event for registering of hits
     ** @return Number of created hits
     **/
    virtual Int_t MakeHitsFromClusters(std::vector<CbmStsCluster*>& clusters,
                                       TClonesArray* hitArray,
                                       CbmEvent* event);


    /** @brief Modify the strip pitch
     ** @param New strip pitch [cm]
     **
     ** The number of strips is re-calculated accordingly.
     **/
    virtual void ModifyStripPitch(Double_t pitch) = 0;


    /** Print charge status **/
    void PrintChargeStatus() const;


    /** String output **/
    virtual std::string ToString() const = 0;



  protected:


    Double_t fDx;            ///< Dimension of active area in x [cm]
    Double_t fDy;            ///< Dimension of active area in y [cm]
    Double_t fDz;            ///< Thickness in z [cm]
    Bool_t   fIsSet;         ///< Flag whether sensor is properly initialised

    /** Analog charge in strips (for front and back side).
     ** Used during analog response simulation. **/
    TArrayD fStripCharge[2];   //!


    /** @brief Analogue response to a track in the sensor
     ** @param point  Pointer to CbmStsSensorPoint object
     ** @value Number of analogue signals created in the strips
     **
     ** In this method, the analogue response of the sensor to a charged
     ** particle traversing it is implemented. The input is an object of
     ** type CbmStsSensorPoint, giving the geometric intersection of the
     ** track with the sensor in the sensor internal coordinate system.
     ** The method shall create charges in the internal arrays fStripCharge.
     **/
    virtual Int_t CalculateResponse(CbmStsSensorPoint* point);


    /** @brief Create a hit from a single cluster
     ** @param cluster Pointer to CbmStsCluster object
     **
     ** Pure virtual; to be implemented in derived classes.
     **/
    virtual void CreateHitFromCluster(CbmStsCluster* cluster) = 0;


    /** Cross talk
     ** @param ctcoeff  Cross-talk coefficient
     **
     ** Operates on the strip charge arrays and re-distributes charges
     ** between adjacent strips according to the cross-talk coefficient.
     **/
    void CrossTalk(Double_t ctcoeff);


    /** Get the cluster position at the top edge of the sensor.
     ** @param[in]  centre    Cluster centre in (module) channel units
     ** @param[out] xCluster  Cluster position at readout edge
     ** @param[out] side      Sensor side [0 = front, 1 = back]
     **
     ** A correction for the Lorentz shift is applied.
     **/
    void GetClusterPosition(Double_t ClusterCentre,
                            Double_t& xCluster, Int_t& side);


    /** @brief Get the readout channel in the module for a given strip
     ** @param strip     Strip number
     ** @param side      Side (0 = front, 1 = back)
     ** @param sensorId  Index of sensor within module
     ** @return  Channel number in module
     **
     ** This method defines the mapping of the sensor strips to the
     ** readout channels in the module.
     **/
    virtual Int_t GetModuleChannel(Int_t strip, Int_t side,
                                   Int_t sensorId) const = 0;


    /** Get the side of the sensor from the module channel number
     ** The channel number can also be the cluster position, so it needs
     ** not be integer.
     ** @param channel  Channel number
     ** @return Sensor side ( 0 = front, 1 = back)
     **/
    Int_t GetSide(Double_t channel) const {
      return ( channel < Double_t(GetNofStrips(0)) ? 0 : 1 );
    }


    /** Get strip and side from module channel.
     ** @param[in] channel   Channel number in module
     ** @param[in] sensorId  Sensor index in module
     ** @value     Pair of strip number and side
     **
     ** Note: This must be the inverse of GetModuleChannel.
     **/
    virtual std::pair<Int_t, Int_t> GetStrip(Int_t channel,
                                             Int_t sensorId) const = 0;


    /** Find the intersection points of two clusters.
     ** For each intersection point, a hit is created.
     ** @param clusterF    Pointer to cluster on front side
     ** @param clusterB    Pointer to cluster on back side
     ** @param sensor      Pointer to sensor object
     ** @return Number of intersection points inside active area
     **/
    virtual Int_t IntersectClusters(CbmStsCluster* clusterF,
                                    CbmStsCluster* clusterB) = 0;

    virtual Int_t IntersectClustersVector(CbmStsCluster* clusterF,
                                    CbmStsCluster* clusterB) = 0;


    /** Check whether a point (x,y) is inside the active area.
     **
     ** @param x  x coordinate in the local c.s. [cm]
     ** @param y  y coordinate in the local c.s. [cm]
     ** @return  kTRUE if inside active area.
     **
     ** The coordinates have to be given in the local
     ** coordinate system (origin in the sensor centre).
     **/
    Bool_t IsInside(Double_t x, Double_t y);


    /** @brief Lorentz shift in the x coordinate
     ** @param z           Coordinate of charge origin in local c.s. [cm]
     ** @param chargeType  Type of charge carrier (0 = electron, 1 = hole)
     ** @param bY          Magnetic field (y component) [T]
     ** @value Displacement in x due to Lorentz shift [cm]
     **
     ** Calculates the displacement in x of a charge propagating to
     ** the readout plane of the sensor.
     **
     ** TODO: This assumes that the sensor is oriented vertically. It has
     ** to be implemented correctly for arbitrary orientations of the local+
     ** x-y plane.
     **/
    Double_t LorentzShift(Double_t z, Int_t chargeType,Double_t bY) const;


    /** @brief Generate charge as response to a sensor point
     ** @param point  Pointer to sensor point object
     **
     ** Charge is created in the sensor volume as response to the particle
     ** trajectory and is propagated to the read-out edges.
     **/
    void ProduceCharge(CbmStsSensorPoint* point);


    /** Propagate a charge created in the sensor to the readout strips
     ** @param x       x origin of charge in local c.s. [cm]
     ** @param y       y origin of charge in local c.s. [cm]
     ** @param z       z origin of charge in local c.s. [cm]
     ** @param charge  Charge [e]
     ** @param bY      Magnetic field (y component) [T]
     ** @param side    0 = front (n) side; 1 = back (p) side
     ** @param sensor  Pointer to sensor object
     **/
    virtual void PropagateCharge(Double_t x, Double_t y, Double_t z,
                                 Double_t charge, Double_t bY,
                                 Int_t side) = 0;


    /** @brief Register the produced charge in one strip to the module
     ** @param side  0 = front, 1 = back
     ** @param strip strip number
     ** @param charge  charge in strip [e]
     ** @param time    time of registration [ns]
     **
     ** The charge in one strip resulting from the analogue response
     ** simulation is registered to the read-out chip (module).
     **/
    void RegisterCharge(Int_t side, Int_t strip,
                        Double_t charge, Double_t time) const;


    /** Test the consistent implementation of GetModuleChannel and
     ** GetStrip. The latter should be the reverse of the former.
     ** @return kTRUE if successful
     **/
    Bool_t SelfTest();


  private:

    /** Forbid copy constructor **/
    CbmStsSensorDssd(CbmStsSensorDssd& rhs) = delete;


    /** Forbid assignment operator **/
    CbmStsSensorDssd& operator = (const CbmStsSensorDssd& rhs) = delete;



    ClassDef(CbmStsSensorDssd,1);

};


#endif
