/** @file CbmStsSensorPoint.h
 ** @author Volker Friese <v.friese@gsi.de>
 ** @date 08.05.2013
 **/

#ifndef CBMSTSSENSORPOINT_H
#define CBMSTSSENSORPOINT_H 1

#include <string>
#include "Rtypes.h"

/** @class CbmStsSensorPoint
 ** @brief Container class for a local point in a STS sensor
 ** @author V.Friese <v.friese@gsi.de>
 ** @version 1.0
 **
 ** Stores parameters needed for digitisation in the sensor C.S.
 **/
class CbmStsSensorPoint
{

  public:

    /** Default constructor  **/
    CbmStsSensorPoint();


    /** Constructor with parameters
     ** @param x1,y1,z1  Entry coordinates in local C.S. [cm]
     ** @param x2,y2,z2  Exit coordinates in local C.S. [cm]
     ** @param p         Momentum magnitude (entry) [GeV]
     ** @param eLoss     Energy deposit [GeV]
     ** @param time      Time [ns]
     ** @param bx        Mag. Field (x component) at midpoint [T]
     ** @param by        Mag. Field (y component) at midpoint [T]
     ** @param bz        Mag. Field (z component) at midpoint [T]
     ** @param pid       Particle ty[e [PDG code]
     **/
    CbmStsSensorPoint(Double_t x1, Double_t y1, Double_t z1,
                      Double_t x2, Double_t y2, Double_t z2,
                      Double_t p, Double_t eLoss, Double_t time,
                      Double_t bx = 0., Double_t by = 0., Double_t bz = 0.,
                      Int_t pid = 0);

    /** Destructor  **/
    virtual ~CbmStsSensorPoint();


    // --- Accessors --- //
    Double_t GetX1()    const { return fX1; }     ///< Entry x coordinate [cm]
    Double_t GetY1()    const { return fY1; }     ///< Entry y coordinate [cm]
    Double_t GetZ1()    const { return fZ1; }     ///< Entry z coordinate [cm]
    Double_t GetX2()    const { return fX2; }     ///< Exit x coordinate [cm]
    Double_t GetY2()    const { return fY2; }     ///< Exit y coordinate [cm]
    Double_t GetZ2()    const { return fZ2; }     ///< Exit z coordinate [cm]
    Double_t GetP()     const { return fP; }      ///< Momentum magnitude
    Double_t GetELoss() const { return fELoss; }  ///< Energy loss [GeV]
    Double_t GetTime()  const { return fTime; }   ///< Time [ns]
    Double_t GetBx()    const { return fBx; }     ///< Bx-Field at midpoint [T]
    Double_t GetBy()    const { return fBy; }     ///< By-Field at midpoint [T]
    Double_t GetBz()    const { return fBz; }     ///< Bz-Field at midpoint [T]
    Int_t    GetPid()   const { return fPid; }    ///< Particle ID [PDG]


		/** String output **/
		std::string ToString() const;


  private:

    Double_t fX1;      ///< Entry point x [cm]
    Double_t fY1;      ///< Entry point y [cm]
    Double_t fZ1;      ///< Entry point z [cm]
    Double_t fX2;      ///< Exit point x [cm]
    Double_t fY2;      ///< Exit point y [cm]
    Double_t fZ2;      ///< Exit point z [cm]
    Double_t fP;       ///< Momentum magnitude at entry point [GeV]
    Double_t fELoss;   ///< Energy deposit [GeV]
    Double_t fTime;    ///< Time [ns]
    Double_t fBx;      ///< Magnetic field x component at midpoint [T]
    Double_t fBy;      ///< Magnetic field y component at midpoint [T]
    Double_t fBz;      ///< Magnetic field z component at midpoint [T]
    Int_t    fPid;     ///< Particle Type [PDG code]


    ClassDef(CbmStsSensorPoint,2);

};

#endif /* CBMSTSSENSORPOINT_H */
