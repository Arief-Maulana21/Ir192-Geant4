
#ifndef B1DetectorConstruction_h
#define B1DetectorConstruction_h 1

#include "G4VUserDetectorConstruction.hh"
#include "G4LogicalVolume.hh"

class G4VPhysicalVolume;

namespace B1
{

class DetectorConstruction : public G4VUserDetectorConstruction
{
public:

    DetectorConstruction() = default;
    ~DetectorConstruction() override = default;

    G4VPhysicalVolume* Construct() override;

    // Scoring volume
    G4LogicalVolume* GetScoringVolume() const
    {
        return fScoringVolume;
    }

protected:

    G4LogicalVolume* fScoringVolume = nullptr;

};

}

#endif
