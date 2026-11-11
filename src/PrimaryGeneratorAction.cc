
#include "PrimaryGeneratorAction.hh"

#include "DetectorConstruction.hh"
#include "RunAction.hh"

#include "G4ParticleGun.hh"
#include "G4ParticleTable.hh"
#include "G4Gamma.hh"
#include "G4Event.hh"
#include "G4RunManager.hh"
#include "G4SystemOfUnits.hh"
#include "G4PhysicalConstants.hh"
#include "G4RandomDirection.hh"
#include "Randomize.hh"

#include <cmath>

namespace B1
{

// =====================================================
// CONSTRUCTOR
// =====================================================

PrimaryGeneratorAction::PrimaryGeneratorAction()
{
    fParticleGun = new G4ParticleGun(1);

    fParticleGun->SetParticleDefinition(
        G4Gamma::Definition()
    );

    fParticleGun->SetParticleEnergy(
        316.508 * keV
    );

    fParticleGun->SetParticleMomentumDirection(
        G4ThreeVector(0., 0., 1.)
    );
}


// =====================================================
// DESTRUCTOR
// =====================================================

PrimaryGeneratorAction::~PrimaryGeneratorAction()
{
    delete fParticleGun;
}


// =====================================================
// GENERATE PRIMARIES
// =====================================================

void PrimaryGeneratorAction::GeneratePrimaries(
    G4Event* event
)
{
    if (!event)
        return;

    auto runManager =
        G4RunManager::GetRunManager();

    const auto detector =
        static_cast<const DetectorConstruction*>(
            runManager->GetUserDetectorConstruction()
        );

    // ================================================
    // 1. SOURCE GEOMETRY
    // ================================================

    const G4ThreeVector sourceCenter =
        detector->GetSourcePosition();

    const G4double sourceRadius =
        detector->GetSourceRadius();

    const G4double sourceHalfLength =
        detector->GetSourceHalfLength();

    // ================================================
    // 2. SAMPLE EMISSION POINT
    // ================================================

    // Uniform sampling inside a cylinder:
    // r = R * sqrt(U)

    const G4double r =
        sourceRadius * std::sqrt(G4UniformRand());

    const G4double phi =
        twopi * G4UniformRand();

    const G4double z =
        sourceHalfLength *
        (2.0 * G4UniformRand() - 1.0);

    const G4ThreeVector localPosition(
        r * std::cos(phi),
        r * std::sin(phi),
        z
    );

    const G4ThreeVector emissionPosition =
        sourceCenter + localPosition;

    fParticleGun->SetParticlePosition(
        emissionPosition
    );

    // ================================================
    // 3. SAMPLE IR-192 GAMMA ENERGY
    // ================================================

    const G4double energy =
        fSpectrum.SampleEnergy();

    fParticleGun->SetParticleEnergy(energy);

    // ================================================
    // 4. ISOTROPIC EMISSION DIRECTION
    // ================================================

    const G4ThreeVector direction =
        G4RandomDirection();

    fParticleGun->SetParticleMomentumDirection(
        direction
    );

    // ================================================
    // 5. GENERATE ONE GAMMA PER EVENT
    // ================================================

    fParticleGun->GeneratePrimaryVertex(event);

    // =====================================================
    // 6. RECORD PRIMARY INFORMATION
    // =====================================================

    const auto* constRunAction =
    dynamic_cast<const RunAction*>(
        runManager->GetUserRunAction()
    );

    if (constRunAction)
    {
    auto* runAction =
        const_cast<RunAction*>(constRunAction);

    runAction->FillPrimaryEnergy(
        event->GetEventID(),
        energy,
        detector->GetDwellIndex(),
        sourceCenter.z()
    );
    }
}

} // namespace B1
