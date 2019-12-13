/** @file CbmStsSensorFactory.h
 ** @author Volker Friese <v.friese@gsi.de>
 ** @date 18.11.2014
 **/

#ifndef CBMSTSSENSORFACTORY_H
#define CBMSTSSENSORFACTORY_H 1

#include <vector>
#include "TGeoVolume.h"
#include "TNamed.h"

/** @class CbmStsSensorFactory
 ** @brief Creates available sensor types for STS geometry
 ** @author Volker Friese <v.friese@gsi.de>
 ** @since 18.11.2014
 ** @version 1.0
 **
 ** Singleton factory class as first step towards a STS sensor database.
 ** At present, it comprises only the geometric parameters of the sensors,
 ** to be used when creating the geometry. The idea is to extend the scheme
 ** also to parameters describing the internal configuration of the sensor
 ** as needed by digitisation, such as strip pitch, size of active area etc.
 **/
class CbmStsSensorFactory : public TNamed {

	public:

	  /** Destructor (empty) **/
		virtual ~CbmStsSensorFactory() { };


		/** Define the available sensor types. Parameters are hard-coded here. **/
		Int_t DefineSensors();


		/** Number of sensors in database
		 ** @return Number of available sensor types
		 **/
		Int_t GetNofSensors() const { return fSensors.size(); }


		/** Get a sensor type from the database
		 ** @param index Running index of sensor type in the database
		 ** @return Pointer to sensor volume
		 **/
		TGeoVolume* GetSensor(Int_t index) { return fSensors[index]; }


		/** Static instance
		 ** @return Pointer to static instance of this class
		 **/
		static CbmStsSensorFactory* Instance();


	private:

		/** Constructor **/
		CbmStsSensorFactory();


		/** Copy constructor
		 ** Not implemented to avoid being executed
		 **/
    CbmStsSensorFactory(const CbmStsSensorFactory&);


    /** Assignment operator
		 ** Not implemented to avoid being executed
		 **/
    CbmStsSensorFactory operator=(const CbmStsSensorFactory&);


    /** Pointer to singleton instance **/
		static CbmStsSensorFactory* fgInstance;


		/** Create a TGeoVolume from the sensor parameters
		 ** @param name       Volume name
		 ** @param material   Name of medium
		 ** @param xSize      Extension in x
		 ** @param ySize      Extension in y
		 ** @param thickness  Extension in z
		 ** @param colour     Colour for display (default kRED)
		 ** @return Success status
		 **
		 ** The TGeoVolume for the sensor will be instantiated and added to
		 ** the current TGeoManager. The medium specified by the material name
		 ** must already exist in the TGeoManager, otherwise a NULL pointer will be
		 ** returned.
		 **/
		Bool_t CreateSensor(TString& name, TString& material, Double_t xSize,
				Double_t ySize, Double_t thickness, EColor color = kRed);


		/** Vector with pointers to available sensor volumes **/
		std::vector<TGeoVolume*> fSensors;


		ClassDef(CbmStsSensorFactory,1);
};

#endif /* CBMSTSSENSORFACTORY_H */
