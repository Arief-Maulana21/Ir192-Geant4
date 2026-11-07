
#include "PrimaryGeneratorAction.hh"

#include "G4ParticleGun.hh"
#include "G4Gamma.hh"
#include "G4ThreeVector.hh"
#include "G4SystemOfUnits.hh"
#include "G4Event.hh"
#include "Randomize.hh"
#include "RunAction.hh"
#include "G4RunManager.hh"

#include "Ir192Spectrum.hh"

#include "G4AnalysisManager.hh"
#include "G4ios.hh"

namespace B1
{

// =====================================================
// CONSTRUCTOR
// =====================================================

PrimaryGeneratorAction::PrimaryGeneratorAction()
{
    // Create one gamma particle per event
    fParticleGun = new G4ParticleGun(1);

    // Define particle as gamma
    fParticleGun->SetParticleDefinition(G4Gamma::Definition());

    // Applicator center in World coordinates
    // Must match DetectorConstruction.cc
    fApplicatorPosition = G4ThreeVector(
        0.,
        0.,
        5.0 * cm
    );

    // Dwell positions relative to applicator center
    fDwellPositions = {
        -10.0 * mm,
          0.0 * mm,
        +10.0 * mm
    };

    // Load Ir-192 gamma energy spectrum
    fSpectrum = new Ir192Spectrum();

}


// =====================================================
// DESTRUCTOR
// =====================================================

PrimaryGeneratorAction::~PrimaryGeneratorAction()
{
    delete fParticleGun;
    delete fSpectrum;
}


// =====================================================
// GENERATE PRIMARY PARTICLES
// =====================================================

void PrimaryGeneratorAction::GeneratePrimaries(G4Event* event)
{

    // -------------------------------------------------
    // 1. SELECT DWELL POSITION
    // -------------------------------------------------

    // Cycle through dwell positions:
    // Event 0 -> Dwell 1
    // Event 1 -> Dwell 2
    // Event 2 -> Dwell 3
    // Event 3 -> Dwell 1, and so on.

    auto runAction =
    static_cast<const RunAction*>(
        G4RunManager::GetRunManager()
            ->GetUserRunAction()
    );
    
    G4int eventID = event->GetEventID();

    G4int numberOfDwellPositions =
        static_cast<G4int>(fDwellPositions.size());

    G4int dwellIndex =
        eventID % numberOfDwellPositions;

    G4double dwellZ =
        fDwellPositions[dwellIndex];


    // -------------------------------------------------
    // 2. CALCULATE SOURCE POSITION
    // -------------------------------------------------

    G4ThreeVector sourcePosition(
        fApplicatorPosition.x(),
        fApplicatorPosition.y(),
        fApplicatorPosition.z() + dwellZ
    );

    fParticleGun->SetParticlePosition(sourcePosition);


    // -------------------------------------------------
    // 3. SAMPLE GAMMA ENERGY FROM IR-192 SPECTRUM
    // -------------------------------------------------

    G4double energy = fSpectrum->SampleEnergy();

    fParticleGun->SetParticleEnergy(energy);


    // -------------------------------------------------
    // 4. SET ISOTROPIC GAMMA EMISSION DIRECTION
    // -------------------------------------------------

    // Gamma emission is modeled as isotropic.
    G4ThreeVector direction = G4RandomDirection();

    fParticleGun->SetParticleMomentumDirection(direction);


    // -------------------------------------------------
    // 5. FILL ANALYSIS NTUPLE
    // -------------------------------------------------

    auto analysisManager = G4AnalysisManager::Instance();

    analysisManager->FillNtupleIColumn(
        1,
        0,
        eventID
    );

    analysisManager->FillNtupleDColumn(
        1,
        1,
        energy / keV
    );

    // Store dwell position index
    analysisManager->FillNtupleIColumn(
        1,
        2,
        dwellIndex
    );

    // Store source Z position in mm
    analysisManager->FillNtupleDColumn(
        1,
        3,
        sourcePosition.z() / mm
    );

    analysisManager->AddNtupleRow(1);


    // -------------------------------------------------
    // 6. GENERATE PRIMARY VERTEX
    // -------------------------------------------------

    fParticleGun->GeneratePrimaryVertex(event);


    // -------------------------------------------------
    // 7. PRINT INFORMATION
    // -------------------------------------------------

    if (eventID < 10)
    {
        G4cout << "Event ID       : " << eventID << G4endl;

        G4cout << "Dwell position : "
               << dwellIndex + 1 << G4endl;

        G4cout << "Source position: "
               << sourcePosition / mm << " mm" << G4endl;

        G4cout << "Gamma energy   : "
               << energy / keV << " keV" << G4endl;

        G4cout << "================================"
               << G4endl;
    }

}

}
