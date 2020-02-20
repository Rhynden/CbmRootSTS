/** @file CbmStsElement.h
 ** @author Volker Friese <v.friese@gsi.de>
 ** @date 27.05.2013
 **/

#ifndef CBMSTSELEMENT_H
#define CBMSTSELEMENT_H 1


#include "TGeoPhysicalNode.h"
#include "TNamed.h"
#include "CbmStsAddress.h"



/** @class CbmStsElement
 ** @brief Class representing an element of the STS setup
 ** @author V.Friese <v.friese@gsi.de>
 ** @version 1.0
 **
 ** A CbmStsElement represents an element in the STS setup
 ** hierarchy (e.g., station, ladder, module,...). It has
 ** a unique address, a level (enum CbmStsElementLevel),
 ** a pointer to a TGeoPhysicalNode and an array of daughter
 ** elements. It is thus an alignable object.
 **/
class CbmStsElement : public TNamed
{

  public:

    /** Default constructor **/
    CbmStsElement();


    /** Standard constructor
     ** @param address  Unique element address
     ** @param level    Element level
     ** @param node     Pointer to geometry node
     ** @param mother   Pointer to mother element
     **/
    CbmStsElement(Int_t address, Int_t level,
                  TGeoPhysicalNode* node = nullptr,
                  CbmStsElement* mother = nullptr);


    /** Destructor **/
    virtual ~CbmStsElement() { };


    /** Construct the element name from the address (static)
     ** @param address Unique element address
     ** @param level   Element level (unit, ladder, etc.)
     **/
    static TString ConstructName(Int_t address, EStsElementLevel level);


    /** Get unique address
     ** @return Element address
     **/
    Int_t GetAddress() const { return fAddress; }


    /** Get a daughter element
     ** @param index  Index of daughter element
     **/
    CbmStsElement* GetDaughter(Int_t index) const;


    /** Get the index within the mother element
     ** @return Index of element in mother
     **/
    Int_t GetIndex() const {
      return CbmStsAddress::GetElementId(fAddress, fLevel);
    }


    /** Get the element level
     ** @return Element level (type enum EStsElementLevel)
     **/
    EStsElementLevel GetLevel() const { return fLevel; }


    /** Get the mother element **/
    CbmStsElement* GetMother() const { return fMother; }


    /** Get number of daughter elements
     ** @return Number of daughters
     **/
    Int_t GetNofDaughters() const { return fDaughters.size(); }


    /** Get number of elements at given level
     ** @param level  Element level (see enum EStsElementLevel)
     ** @return Number of elements at given level with this
     **         element as ancestor
     */
    Int_t GetNofElements(Int_t level) const;


    TGeoPhysicalNode* GetPnode() const { return fNode; }


    /** Initialise daughters from geometry **/
    virtual void InitDaughters();


    /** Set the mother element
     ** @param Pointer to mother element
     **/
    void SetMother(CbmStsElement* mother) { fMother = mother; }


    /** Print **/
    virtual void Print(Option_t* opt = "") const;


  protected:

    Int_t fAddress;                        ///< Unique element address
    EStsElementLevel fLevel;               ///< Level in hierarchy
    TGeoPhysicalNode* fNode;               ///< Pointer to geometry
    std::vector<CbmStsElement*> fDaughters;     ///< Array of daughters
    CbmStsElement* fMother;                ///< Mother element


    /** Construct the name of the element **/
    void ConstructName();


    /** Set the element level from integer
     ** Protection against being out of range.
     ** @param  level  Element level
     **/
    void SetLevel(Int_t level);


private:

    // --- Disable copy constructor and assignment operator
    CbmStsElement(const CbmStsElement&) = delete;
    CbmStsElement& operator=(const CbmStsElement&) = delete;


    ClassDef(CbmStsElement, 2);

};

#endif /* CBMSTSELEMENT_H */
