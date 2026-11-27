#ifndef B1DetectorConstruction_h
#define B1DetectorConstruction_h 1

#include "G4VUserDetectorConstruction.hh"
#include "G4ThreeVector.hh"
#include "G4SystemOfUnits.hh"
#include "globals.hh"
#include <array>

class G4LogicalVolume;
class G4VPhysicalVolume;

namespace B1 {

class DetectorConstruction : public G4VUserDetectorConstruction {
public:
    static constexpr G4int kNumOrgans = 6;

    DetectorConstruction() = default;
    ~DetectorConstruction() override = default;
    G4VPhysicalVolume* Construct() override;

    // Backward compatibility for the previous single-volume scorer.
    G4LogicalVolume* GetScoringVolume() const { return fScoringVolume; }
    G4LogicalVolume* GetOrganVolume(G4int id) const {
        return (id >= 0 && id < kNumOrgans) ? fOrganVolumes[id] : nullptr;
    }
    G4ThreeVector GetSourcePosition() const { return fSourcePosition; }
    G4double GetSourceRadius() const { return fSourceRadius; }
    G4double GetSourceHalfLength() const { return fSourceHalfLength; }
    G4int GetDwellIndex() const { return 0; }

private:
    G4LogicalVolume* fScoringVolume = nullptr;
    std::array<G4LogicalVolume*, kNumOrgans> fOrganVolumes{};
    G4ThreeVector fApplicatorPosition{0., 0., 50.0*mm};
    G4ThreeVector fSourcePosition{0., 0., 50.0*mm};
    G4double fSourceRadius = 0.325*mm;
    G4double fSourceHalfLength = 1.75*mm;
};

} // namespace B1
#endif
