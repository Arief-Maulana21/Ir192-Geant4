
#ifndef B1DetectorConstruction_h
#define B1DetectorConstruction_h 1

#include "G4VUserDetectorConstruction.hh"
#include "G4ThreeVector.hh"
#include "globals.hh"
#include "G4SystemOfUnits.hh"

class G4LogicalVolume;
class G4VPhysicalVolume;

namespace B1
{

class DetectorConstruction
    : public G4VUserDetectorConstruction
{
public:
    DetectorConstruction() = default;
    ~DetectorConstruction() override = default;

    G4VPhysicalVolume* Construct() override;

    // Scoring volume used by SteppingAction/RunAction
    G4LogicalVolume* GetScoringVolume() const
    {
        return fScoringVolume;
    }

    // World coordinates of the active Ir-192 source center
    G4ThreeVector GetSourcePosition() const
    {
        return fSourcePosition;
    }

    // Dimensions of the active source
    G4double GetSourceRadius() const
    {
        return fSourceRadius;
    }

    G4double GetSourceHalfLength() const
    {
        return fSourceHalfLength;
    }

    // One fixed dwell position per run
    G4int GetDwellIndex() const
    {
        return 0;
    }

private:
    G4LogicalVolume* fScoringVolume = nullptr;

    // Applicator centered at z = 50 mm
    G4ThreeVector fApplicatorPosition =
        G4ThreeVector(0., 0., 50.0 * mm);

    // One fixed source position
    G4ThreeVector fSourcePosition =
        G4ThreeVector(0., 0., 50.0 * mm);

    // Active source dimensions
    G4double fSourceRadius = 0.30 * mm;
    G4double fSourceHalfLength = 1.50 * mm;
};

} // namespace B1

#endif
