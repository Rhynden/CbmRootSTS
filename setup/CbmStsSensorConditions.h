/** @file CbmStsSensorConditions.h
 ** @author Volker Friese <v.friese@gsi.de>
 ** @date 28.11.2014
 **/

#ifndef CBMSTSSENSORCONDITIONS_H
#define CBMSTSSENSORCONDITIONS_H 1

#include <string>
#include "TMath.h"
#include "TObject.h"

/** @class CbmStsSensorConditions
 ** @brief Container for operating condition parameters of a sensor
 ** @author Volker Friese <v.friese@gsi.de>
 ** @since 28.11.2014
 ** @version 1.0
 **/
class CbmStsSensorConditions : public TObject {

	public:

		/** Default constructor
		 ** @param vFD           Full depletion voltage [V]
		 ** @param vBias         Bias voltage [V]
		 ** @param temperature   Temperature [K]
		 ** @param cCoupling     Coupling capacitance [pF]
		 ** @param cInterstrip   Inter-strip capacitance [pF]
		 ** @param bX            Magn. field Bx at sensor centre [T]
		 ** @param bY            Magn. field By at sensor centre [T]
		 ** @param bZ            Magn. field Bz at sensor centre [T]
		 **/
		CbmStsSensorConditions(Double_t vFD = 0., Double_t vBias = 0.,
		                       Double_t temperature = 273.,
		                       Double_t cCoupling = 0., Double_t cInterstrip = 0.,
		                       Double_t bX = 0., Double_t bY = 0.,
		                       Double_t bZ = 0.);


		/** Destructor **/
		virtual ~CbmStsSensorConditions();


		/** Magnetic field at sensor centre **/
		Double_t GetBx() const { return fBx; }
		Double_t GetBy() const { return fBy; }
		Double_t GetBz() const { return fBz; }
		Double_t GetB()  const { return TMath::Sqrt( fBx * fBx +
				                                         fBy * fBy +
				                                         fBz * fBz ); }


		/** Coupling capacitance
		 ** @return Coupling capacitance [pF]
		 **/
		Double_t GetCcoupling() const { return fCcoupling; }


		/** Inter-strip capacitance
		 ** @return Inter-strip capacitance [pF]
		 **/
		Double_t GetCinterstrip() const { return fCinterstrip; }


		/** Cross-talk coefficient
		 ** Is derived from
		 ** @return Cross-talk coefficient
		 **/
		Double_t GetCrossTalk() const { return fCrossTalk; }


		/** Temperature
		 ** @return Temperature [K]
	   **/
		Double_t GetTemperature() const { return fTemperature; }


		/** Bias voltage
		 ** @return Bias voltage [V]
		 **/
		Double_t GetVbias() const { return fVbias; }


		/** Full depletion voltage
		 ** @return Full depletion voltage [V]
		 **/
		Double_t GetVfd() const { return fVfd; }
                
		/** Mean shift due to magnetic field
		 ** @param  Side: 0 - electrons, 1 - holes 
		 ** @return Mean shift[cm] 
		 **/
		Double_t GetMeanLorentzShift(Int_t side) const { return fMeanLorentzShift[side]; }

		Double_t GetHallParameter(Int_t index, Int_t chargeType);

                /** Get parameters for Hall mobility calculation into array**/
		void GetHallMobilityParametersInto(Double_t * hallMobilityParameters, Int_t chargeType) const;



		/** Hall mobility
		 ** @param eField  Electric field [V/cm]
		 ** @param chargeType (0 = electron, 1 = hole)
		 ** @value Hall mobility [cm**2/(Vs)]
		 */
		Double_t HallMobility(Double_t eField, Int_t chargeType) const;


		
		/** Set the magnetic field
		 ** @param bx,by,bz  Magnetic field components in sensor centre [T]
		 **/
		void SetField(Double_t bX, Double_t bY, Double_t bZ) {
			fBx = bX;
			fBy = bY;
			fBz = bZ;
			CalculateHallMobilityParameters();
		}


		/** String output **/
		std::string ToString() const;



	private:

		Double_t fVfd;           ///< Full depletion voltage [V]
		Double_t fVbias;         ///< Bias voltage [V]
		Double_t fTemperature;   ///< Temperature [K]
		Double_t fCcoupling;     ///< Coupling capacitance [pF]
		Double_t fCinterstrip;   ///< Inter-strip capacitance [pF]
		Double_t fCrossTalk;     ///< Cross-talk coefficient
		Double_t fBx;            ///< Mag. field (x comp.) at sensor centre [T]
		Double_t fBy;            ///< Mag. field (y comp.) at sensor centre [T]
		Double_t fBz;            ///< Mag. field (z comp.) at sensor centre [T]
		Double_t fHallMobilityParametersE[4]; ///< Array with parameters for electron Hall mobility calculation
		Double_t fHallMobilityParametersH[4]; ///< Array with parameters for hole     Hall mobility calculation
		Double_t fMeanLorentzShift[2];///< Lorenz shift averaged over the z-coordinate of the chage carrier creation

		/** Parameters for Hall mobility calculation **/
		void CalculateHallMobilityParameters();

		ClassDef(CbmStsSensorConditions, 1);
};

#endif /* CBMSTSSENSSORCONDITIONS_H */
