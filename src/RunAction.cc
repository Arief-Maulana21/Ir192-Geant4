
#include "RunAction.hh"

#include "DetectorConstruction.hh"
#include "PrimaryGeneratorAction.hh"

#include "G4AccumulableManager.hh"
#include "G4LogicalVolume.hh"
#include "G4ParticleGun.hh"
#include "G4Run.hh"
#include "G4RunManager.hh"
#include "G4SystemOfUnits.hh"
#include "G4UnitsTable.hh"
#include "G4AnalysisManager.hh"

#include <cmath>
#include <algorithm>

namespace B1
{

RunAction::RunAction()
{
    // =====================================================
    // 1. DOSE UNITS
    // =====================================================

    const G4double milligray = 1.e-3 * gray;
    const G4double microgray = 1.e-6 * gray;
    const G4double nanogray  = 1.e-9 * gray;
    const G4double picogray  = 1.e-12 * gray;

    new G4UnitDefinition(
        "milligray", "milliGy", "Dose", milligray);

    new G4UnitDefinition(
        "microgray", "microGy", "Dose", microgray);

    new G4UnitDefinition(
        "nanogray", "nanoGy", "Dose", nanogray);

    new G4UnitDefinition(
        "picogray", "picoGy", "Dose", picogray);


    // =====================================================
    // 2. REGISTER ACCUMULABLES
    // =====================================================

    auto accumulableManager =
        G4AccumulableManager::Instance();

    accumulableManager->Register(fEdep);
    accumulableManager->Register(fEdep2);

    accumulableManager->Register(fPrimaryGamma);
    accumulableManager->Register(fSecondaryGamma);
    accumulableManager->Register(fElectron);

    accumulableManager->Register(fCompton);
    accumulableManager->Register(fPhotoelectric);
    accumulableManager->Register(fRayleigh);


    // =====================================================
    // 3. ANALYSIS MANAGER
    // =====================================================

    auto analysisManager =
        G4AnalysisManager::Instance();

    analysisManager->SetVerboseLevel(1);
    analysisManager->SetNtupleMerging(true);


    // =====================================================
    // 4. SECONDARY RADIATION NTUPLE (ID = 0)
    // =====================================================

    analysisManager->CreateNtuple(
        "Secondary",
        "Secondary Radiation"
    );

    analysisManager->CreateNtupleIColumn("EventID");        // 0
    analysisManager->CreateNtupleIColumn("TrackID");        // 1
    analysisManager->CreateNtupleIColumn("ParentID");       // 2
    analysisManager->CreateNtupleSColumn("Particle");       // 3
    analysisManager->CreateNtupleDColumn("Energy_keV");     // 4
    analysisManager->CreateNtupleSColumn("Creator");        // 5
    analysisManager->CreateNtupleSColumn("Process");        // 6
    analysisManager->CreateNtupleDColumn("X_cm");           // 7
    analysisManager->CreateNtupleDColumn("Y_cm");           // 8
    analysisManager->CreateNtupleDColumn("Z_cm");           // 9
    analysisManager->CreateNtupleDColumn("Edep_keV");       // 10
    analysisManager->CreateNtupleSColumn("Volume");         // 11
    analysisManager->CreateNtupleDColumn("Time_ns");        // 12
    analysisManager->CreateNtupleDColumn("Weight");         // 13
    analysisManager->CreateNtupleDColumn("DirX");           // 14
    analysisManager->CreateNtupleDColumn("DirY");           // 15
    analysisManager->CreateNtupleDColumn("DirZ");           // 16
    analysisManager->CreateNtupleDColumn("TrackLength_mm"); // 17
    analysisManager->CreateNtupleSColumn("CreatorVolume");  // 18

    analysisManager->FinishNtuple();


    // =====================================================
    // 5. PRIMARY GAMMA NTUPLE (ID = 1)
    // =====================================================

    analysisManager->CreateNtuple(
        "Primary",
        "Primary Gamma"
    );

    analysisManager->CreateNtupleIColumn("EventID");       // 0
    analysisManager->CreateNtupleDColumn("Energy_keV");    // 1
    analysisManager->CreateNtupleIColumn("DwellIndex");    // 2
    analysisManager->CreateNtupleDColumn("SourceZ_mm");    // 3

    analysisManager->FinishNtuple();

    // =====================================================
    // 6. EVENT DOSE NTUPLE (ID = 2)
    // =====================================================

    analysisManager->CreateNtuple(
    "EventDose",
    "Energy deposition per event in Detector A"
    );

    analysisManager->CreateNtupleIColumn(
    "EventID"
    ); // 0

    analysisManager->CreateNtupleDColumn(
    "Edep_keV"
    ); // 1

    analysisManager->FinishNtuple();
}


// =====================================================
// BEGIN OF RUN
// =====================================================

void RunAction::BeginOfRunAction(const G4Run*)
{
    G4RunManager::GetRunManager()
        ->SetRandomNumberStore(false);

    auto accumulableManager =
        G4AccumulableManager::Instance();

    accumulableManager->Reset();

    auto analysisManager =
        G4AnalysisManager::Instance();

    analysisManager->OpenFile(
        "SecondaryRadiation.root"
    );
}


// =====================================================
// END OF RUN
// =====================================================

void RunAction::EndOfRunAction(const G4Run* run)
{
    G4int nofEvents = run->GetNumberOfEvent();

    auto analysisManager =
        G4AnalysisManager::Instance();

    if (nofEvents == 0)
    {
        analysisManager->CloseFile();
        return;
    }

    // Merge accumulables
    auto accumulableManager =
        G4AccumulableManager::Instance();

    accumulableManager->Merge();


    // =================================================
    // ENERGY DEPOSITION AND DOSE
    // =================================================

    const auto* detector = 
        static_cast<const 
    DetectorConstruction*>(G4RunManager :: GetRunManager()
    ->GetUserDetectorConstruction());

    auto* scoringvolume = detector ->GetScoringVolume();

    G4double mass = scoringvolume->GetMass();
    
    
    
    G4double edep = fEdep.GetValue();
    G4double edep2 = fEdep2.GetValue();

    G4double varianceNumerator =
        edep2 - edep * edep / nofEvents;

    varianceNumerator =
        std::max(0.0, varianceNumerator);

    G4double standardErrorEdep = 0.0;

    if (nofEvents > 1)
    {
        standardErrorEdep =
        std::sqrt(
            varianceNumerator *
            nofEvents / (nofEvents - 1.0)
            );
    }

    G4double dose = edep / mass;

    G4double doseUncertainty =
        standardErrorEdep / mass;


    // =================================================
    // RUN CONDITIONS
    // =================================================

    G4String runCondition;

    auto generatorAction =
        static_cast<const PrimaryGeneratorAction*>(
            G4RunManager::GetRunManager()
                ->GetUserPrimaryGeneratorAction()
        );

    if (generatorAction)
    {
        const G4ParticleGun* particleGun =
            generatorAction->GetParticleGun();

        runCondition =
            particleGun->GetParticleDefinition()
                ->GetParticleName();

        runCondition +=
            " (Ir-192 sampled energy spectrum)";
    }


    // =================================================
    // WRITE OUTPUT
    // =================================================

    analysisManager->Write();
    analysisManager->CloseFile();


    // =================================================
    // RUN SUMMARY
    // =================================================

    if (IsMaster())
    {
        G4cout << G4endl
               << "----------- End of Global Run -----------"
               << G4endl;
    }
    else
    {
        G4cout << G4endl
               << "----------- End of Local Run ------------"
               << G4endl;
    }

    G4cout << "Number of events : "
           << nofEvents << G4endl;

    G4cout << "Primary particle : "
           << runCondition << G4endl;

    G4cout << "Scoring volume dose : "
           << G4BestUnit(dose, "Dose") << G4endl;

    G4cout << "Statistical dose uncertainly :"
           << G4BestUnit(doseUncertainty, "Dose") << G4endl;


    // =================================================
    // PARTICLE STATISTICS
    // =================================================

    G4cout << G4endl;
    G4cout << "========== Simulation Summary =========="
           << G4endl;

    G4cout << "Primary gamma      : "
           << fPrimaryGamma.GetValue() << G4endl;

    G4cout << "Secondary gamma    : "
           << fSecondaryGamma.GetValue() << G4endl;

    G4cout << "Secondary electron : "
           << fElectron.GetValue() << G4endl;

    G4cout << G4endl;

    G4cout << "Compton            : "
           << fCompton.GetValue() << G4endl;

    G4cout << "Photoelectric      : "
           << fPhotoelectric.GetValue() << G4endl;

    G4cout << "Rayleigh           : "
           << fRayleigh.GetValue() << G4endl;

    G4cout << "========================================"
           << G4endl << G4endl;
}


// =====================================================
// ADD ENERGY DEPOSITION
// =====================================================

void RunAction::AddEdep(G4double edep)
{
    fEdep += edep;
    fEdep2 += edep * edep;
}


// =====================================================
// FILL PRIMARY GAMMA DATA
// =====================================================

void RunAction::FillPrimaryEnergy(
    G4int eventID,
    G4double energy,
    G4int dwellIndex,
    G4double sourceZ
)
{
    auto analysisManager =
        G4AnalysisManager::Instance();

    analysisManager->FillNtupleIColumn(
        1, 0, eventID);

    analysisManager->FillNtupleDColumn(
        1, 1, energy / keV);

    analysisManager->FillNtupleIColumn(
        1, 2, dwellIndex);

    analysisManager->FillNtupleDColumn(
        1, 3, sourceZ / mm);

    analysisManager->AddNtupleRow(1);
}
    void RunAction::RecordEventDose(
    G4int eventID,
    G4double edep)
    {
    auto analysisManager =
        G4AnalysisManager::Instance();

    analysisManager->FillNtupleIColumn(
        2, 0, eventID
    );

    analysisManager->FillNtupleDColumn(
        2, 1, edep / keV
    );

    analysisManager->AddNtupleRow(2);
    }

} // namespace B1
