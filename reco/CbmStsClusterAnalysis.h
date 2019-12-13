/** @file CbmStsClusterAnalysis.h
 ** @author Volker Friese <v.friese@gsi.de>
 ** @date 18.10.2016
 **/

#ifndef CBMSTSCLUSTERANALYSIS_H
#define CBMSTSCLUSTERANALYSIS_H 1

#include "TObject.h"
#include "CbmStsPhysics.h"

class TClonesArray;
class CbmStsCluster;
class CbmStsModule;


/** @class CbmStsClusterAnalysis
 ** @brief Determination of cluster parameters
 ** @author V.Friese <v.friese@gsi.de>
 ** @version 1.0
 **
 ** This class implements the determination of STS cluster parameters
 ** (time, position, charge) and their errors from the digis
 ** contained in the cluster.
 **/
class CbmStsClusterAnalysis : public TObject
{

	public:

		/** Constructor **/
		CbmStsClusterAnalysis() : fPhysics(CbmStsPhysics::Instance()) {
		};

                
    CbmStsClusterAnalysis(const CbmStsClusterAnalysis&) = delete;
    CbmStsClusterAnalysis& operator=(const CbmStsClusterAnalysis&) = delete;
                
		/** Destructor **/
		virtual ~CbmStsClusterAnalysis() { };


		/** Algorithm implementation
		 ** @param cluster    Pointer to cluster object
		 ** @param module     Pointer to CbmStsModule to be operated on
		 **/
		void Analyze(CbmStsCluster* cluster, CbmStsModule* module);


	protected:

		CbmStsPhysics* fPhysics;  //! Instance of physics tool


		ClassDef(CbmStsClusterAnalysis, 1);

};

#endif /* CBMSTSCLUSTERANALYSIS_H */
