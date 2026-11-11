
#include "RunAction.hh"

#include "DetectorConstruction.hh"

#include "G4AccumulableManager.hh"
#include "G4LogicalVolume.hh"
#include "G4Run.hh"
#include "G4RunManager.hh"
#include "G4SystemOfUnits.hh"
#include "G4UnitsTable.hh"
#include "G4AnalysisManager.hh"

#include <cmath>
#include <algorithm>

namespace B1
{

// =====================================================
// CONSTRUCTOR
// =====================================================

RunAction::RunAction()
{
    // =================================================
    // 1. DOSE UNITS
    // =================================================

    // Register custom dose units once.
    // This follows the pattern used by Geant4 examples.

    static const G4bool unitsRegistered = []()
    {
        new G4UnitDefinition(
            "milligray", "mGy", "Dose",
            1.e-3 * gray
        );

        new G4UnitDefinition(
            "microgray", "uGy", "Dose",
            1.e-6 * gray
        );

        new G4UnitDefinition(
            "nanogray", "nGy", "Dose",
            1.e-9 * gray
        );

        new G4UnitDefinition(
            "picogray", "pGy", "Dose",
            1.e-12 * gray
        );

        return true;
    }();

    (void)unitsRegistered;


    // =================================================
    // 2. REGISTER ACCUMULABLES
    // =================================================

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


    // =================================================
    // 3. ANALYSIS MANAGER
    // =================================================

    auto analysisManager =
        G4AnalysisManager::Instance();

    analysisManager->SetVerboseLevel(1);

    // Merge worker ntuples into a ROOT output file.
    analysisManager->SetNtupleMerging(true);


    // =================================================
    // 4. SECONDARY NTUPLE (ID = 0)
    // =================================================

    analysisManager->CreateNtuple(
        "Secondary",
        "Particle step information"
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


    // =================================================
    // 5. PRIMARY NTUPLE (ID = 1)
    // =================================================

    analysisManager->CreateNtuple(
        "Primary",
        "Primary Ir-192 gamma information"
    );

    analysisManager->CreateNtupleIColumn("EventID");       // 0
    analysisManager->CreateNtupleDColumn("Energy_keV");    // 1
    analysisManager->CreateNtupleIColumn("DwellIndex");    // 2
    analysisManager->CreateNtupleDColumn("SourceZ_mm");    // 3

    analysisManager->FinishNtuple();


    // =================================================
    // 6. Pb INTERACTIONS NTUPLE (ID = 2)
    // =================================================

    analysisManager->CreateNtuple(
        "PbInteractions",
        "Photon interactions inside PbShield"
    );

    analysisManager->CreateNtupleIColumn("EventID");          // 0
    analysisManager->CreateNtupleIColumn("TrackID");          // 1
    analysisManager->CreateNtupleIColumn("ParentID");         // 2
    analysisManager->CreateNtupleSColumn("Particle");         // 3
    analysisManager->CreateNtupleSColumn("Process");          // 4
    analysisManager->CreateNtupleDColumn("EnergyBefore_keV"); // 5
    analysisManager->CreateNtupleDColumn("EnergyAfter_keV");  // 6
    analysisManager->CreateNtupleDColumn("Edep_keV");         // 7
    analysisManager->CreateNtupleDColumn("X_mm");             // 8
    analysisManager->CreateNtupleDColumn("Y_mm");             // 9
    analysisManager->CreateNtupleDColumn("Z_mm");             // 10
    analysisManager->CreateNtupleIColumn("NSecondaries");     // 11

    analysisManager->FinishNtuple();


    // =================================================
    // 7. EVENT DOSE NTUPLE (ID = 3)
    // =================================================

    analysisManager->CreateNtuple(
        "EventDose",
        "Energy deposition per event in scoring volume"
    );

    analysisManager->CreateNtupleIColumn("EventID");  // 0
    analysisManager->CreateNtupleDColumn("Edep_keV"); // 1

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
    auto analysisManager =
        G4AnalysisManager::Instance();

    const G4int nofEvents =
        run->GetNumberOfEvent();

    if (nofEvents == 0)
    {
        analysisManager->Write();
        analysisManager->CloseFile();
        return;
    }

    // =================================================
    // 1. MERGE ACCUMULABLES
    // =================================================

    auto accumulableManager =
        G4AccumulableManager::Instance();

    accumulableManager->Merge();


    // =================================================
    // 2. WRITE ROOT OUTPUT
    // =================================================

    analysisManager->Write();
    analysisManager->CloseFile();


    // =================================================
    // 3. PRINT ONLY GLOBAL RUN SUMMARY
    // =================================================

    if (!IsMaster())
        return;


    // =================================================
    // 4. SCORING VOLUME MASS
    // =================================================

    const auto* detector =
        static_cast<const DetectorConstruction*>(
            G4RunManager::GetRunManager()
                ->GetUserDetectorConstruction()
        );

    if (!detector)
    {
        G4cout << "ERROR: DetectorConstruction unavailable."
               << G4endl;
        return;
    }

    auto* scoringVolume =
        detector->GetScoringVolume();

    if (!scoringVolume)
    {
        G4cout << "ERROR: Scoring volume unavailable."
               << G4endl;
        return;
    }

    const G4double mass =
        scoringVolume->GetMass();

    if (mass <= 0.0)
    {
        G4cout << "ERROR: Invalid scoring volume mass."
               << G4endl;
        return;
    }


    // =================================================
    // 5. ENERGY DEPOSITION
    // =================================================

    const G4double edep =
        fEdep.GetValue();

    const G4double edep2 =
        fEdep2.GetValue();


    // =================================================
    // 6. STATISTICAL UNCERTAINTY
    // =================================================

    G4double sigmaEdep = 0.0;

    if (nofEvents > 1)
    {
        const G4double varianceNumerator =
            std::max(
                0.0,
                edep2 - edep * edep / nofEvents
            );

        sigmaEdep = std::sqrt(
            varianceNumerator *
            nofEvents / (nofEvents - 1.0)
        );
    }


    // =================================================
    // 7. ABSORBED DOSE
    // =================================================

    const G4double dose =
        edep / mass;

    const G4double doseUncertainty =
        sigmaEdep / mass;

    const G4double relativeUncertainty =
        dose > 0.0
            ? 100.0 * doseUncertainty / dose
            : 0.0;


    // =================================================
    // 8. RUN SUMMARY
    // =================================================

    G4cout << G4endl;
    G4cout << "========================================"
           << G4endl;
    G4cout << "       IR-192 HDR RUN SUMMARY"
           << G4endl;
    G4cout << "========================================"
           << G4endl;

    G4cout << "Number of events : "
           << nofEvents << G4endl;

    G4cout << "Primary particle : gamma"
           << G4endl;

    G4cout << "Source model : Ir-192 sampled spectrum"
           << G4endl;

    G4cout << "Dwell mode : single dwell per run"
           << G4endl;

    G4cout << "Scoring volume : "
           << scoringVolume->GetName()
           << G4endl;

    G4cout << "Scoring mass : "
           << G4BestUnit(mass, "Mass")
           << G4endl;

    G4cout << "Total deposited energy : "
           << G4BestUnit(edep, "Energy")
           << G4endl;

    G4cout << "Absorbed dose : "
           << G4BestUnit(dose, "Dose")
           << G4endl;

    G4cout << "Dose uncertainty (1 sigma) : "
           << G4BestUnit(doseUncertainty, "Dose")
           << G4endl;

    G4cout << "Relative uncertainty : "
           << relativeUncertainty
           << " %" << G4endl;


    // =================================================
    // 9. PARTICLE STATISTICS
    // =================================================

    G4cout << G4endl;
    G4cout << "========== PARTICLE STATISTICS ========="
           << G4endl;

    G4cout << "Primary gamma : "
           << fPrimaryGamma.GetValue()
           << G4endl;

    G4cout << "Secondary gamma : "
           << fSecondaryGamma.GetValue()
           << G4endl;

    G4cout << "Secondary electron : "
           << fElectron.GetValue()
           << G4endl;


    // =================================================
    // 10. Pb INTERACTION STATISTICS
    // =================================================

    G4cout << G4endl;
    G4cout << "========= Pb INTERACTIONS =============="
           << G4endl;

    G4cout << "Compton : "
           << fCompton.GetValue()
           << G4endl;

    G4cout << "Photoelectric : "
           << fPhotoelectric.GetValue()
           << G4endl;

    G4cout << "Rayleigh : "
           << fRayleigh.GetValue()
           << G4endl;

    G4cout << "========================================"
           << G4endl << G4endl;
}


// =====================================================
// ADD EVENT ENERGY DEPOSITION
// =====================================================

void RunAction::AddEdep(G4double edep)
{
    fEdep += edep;
    fEdep2 += edep * edep;
}


// =====================================================
// RECORD PRIMARY GAMMA
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
        1, 0, eventID
    );

    analysisManager->FillNtupleDColumn(
        1, 1, energy / keV
    );

    analysisManager->FillNtupleIColumn(
        1, 2, dwellIndex
    );

    analysisManager->FillNtupleDColumn(
        1, 3, sourceZ / mm
    );

    analysisManager->AddNtupleRow(1);
}


// =====================================================
// RECORD EVENT ENERGY DEPOSITION
// =====================================================

void RunAction::RecordEventDose(
    G4int eventID,
    G4double edep
)
{
    auto analysisManager =
        G4AnalysisManager::Instance();

    analysisManager->FillNtupleIColumn(
        3, 0, eventID
    );

    analysisManager->FillNtupleDColumn(
        3, 1, edep / keV
    );

    analysisManager->AddNtupleRow(3);
}

} // namespace B1
