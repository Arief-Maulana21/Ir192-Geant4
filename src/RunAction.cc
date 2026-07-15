
/// \file B1/src/RunAction.cc
/// \brief Implementation of the B1::RunAction class

#include "RunAction.hh"

#include "DetectorConstruction.hh"
#include "PrimaryGeneratorAction.hh"

#include "G4AccumulableManager.hh"
#include "G4LogicalVolume.hh"
#include "G4ParticleDefinition.hh"
#include "G4ParticleGun.hh"
#include "G4Run.hh"
#include "G4RunManager.hh"
#include "G4SystemOfUnits.hh"
#include "G4UnitsTable.hh"
#include "G4Accumulable.hh"
#include "G4AnalysisManager.hh"

namespace B1
{

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

RunAction::RunAction()
{
  // add new units for dose
  //
  const G4double milligray = 1.e-3 * gray;
  const G4double microgray = 1.e-6 * gray;
  const G4double nanogray = 1.e-9 * gray;
  const G4double picogray = 1.e-12 * gray;

  new G4UnitDefinition("milligray", "milliGy", "Dose", milligray);
  new G4UnitDefinition("microgray", "microGy", "Dose", microgray);
  new G4UnitDefinition("nanogray", "nanoGy", "Dose", nanogray);
  new G4UnitDefinition("picogray", "picoGy", "Dose", picogray);

  // Register accumulable to the accumulable manager
  auto accumulableManager = G4AccumulableManager::Instance();
  accumulableManager->Register(fEdep);
  accumulableManager->Register(fEdep2);

  accumulableManager->Register(fPrimaryGamma);
  accumulableManager->Register(fSecondaryGamma);
  accumulableManager->Register(fElectron);

  accumulableManager->Register(fCompton);
  accumulableManager->Register(fPhotoelectric);
  accumulableManager->Register(fRayleigh);

  auto analysisManager = G4AnalysisManager::Instance();

    analysisManager->SetVerboseLevel(1);
    analysisManager->SetNtupleMerging(true);

    analysisManager->CreateNtuple("Secondary","Secondary Radiation");

    // Column 0
    analysisManager->CreateNtupleIColumn("EventID");

    // Column 1
    analysisManager->CreateNtupleIColumn("TrackID");

    // Column 2
    analysisManager->CreateNtupleIColumn("ParentID");

    // Column 3
    analysisManager->CreateNtupleSColumn("Particle");

    // Column 4
    analysisManager->CreateNtupleDColumn("Energy_keV");

    // Column 5
    analysisManager->CreateNtupleSColumn("Creator");

    // Column 6
    analysisManager->CreateNtupleSColumn("Process");

    // Column 7
    analysisManager->CreateNtupleDColumn("X_cm");

    // Column 8
    analysisManager->CreateNtupleDColumn("Y_cm");

    // Column 9
    analysisManager->CreateNtupleDColumn("Z_cm");

    // Column 10
    analysisManager->CreateNtupleDColumn("Edep_keV");

    // Column 11
    analysisManager->CreateNtupleSColumn("Volume");

    analysisManager->CreateNtupleDColumn("Time_ns");

    analysisManager->CreateNtupleDColumn("Weight");

    analysisManager->CreateNtupleDColumn("DirX");

    analysisManager->CreateNtupleDColumn("DirY");

    analysisManager->CreateNtupleDColumn("DirZ");

    analysisManager->CreateNtupleDColumn("TrackLength_mm");

    analysisManager->CreateNtupleSColumn("CreatorVolume");

    analysisManager->FinishNtuple();

    analysisManager->CreateNtuple("Primary","Primary Gamma");

    analysisManager->CreateNtupleIColumn("EvetID");
    analysisManager->CreateNtupleDColumn("Energy_keV");

    analysisManager->FinishNtuple();
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

void RunAction::BeginOfRunAction(const G4Run*)
{
  // inform the runManager to save random number seed
  G4RunManager::GetRunManager()->SetRandomNumberStore(false);
  
  // reset accumulables to their initial values
  G4AccumulableManager* accumulableManager = G4AccumulableManager::Instance();
  accumulableManager->Reset();
  // Reset statistics
      fPrimaryGamma = 0;
      fSecondaryGamma = 0;
      fElectron = 0;

      fCompton = 0;
      fPhotoelectric = 0;
      fRayleigh = 0;  
  auto analysisManager = G4AnalysisManager::Instance();
    analysisManager->OpenFile("SecondaryRadiation.root");
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

void RunAction::EndOfRunAction(const G4Run* run)
{
  G4int nofEvents = run->GetNumberOfEvent();
  if (nofEvents == 0) return;

  // Merge accumulables
  G4AccumulableManager* accumulableManager = G4AccumulableManager::Instance();
  accumulableManager->Merge();

  // Compute dose = total energy deposit in a run and its variance
  //
  G4double edep = fEdep.GetValue();
  G4double edep2 = fEdep2.GetValue();

  G4double rms = edep2 - edep * edep / nofEvents;
  if (rms > 0.)
    rms = std::sqrt(rms);
  else
    rms = 0.;

  const auto detConstruction = static_cast<const DetectorConstruction*>(
    G4RunManager::GetRunManager()->GetUserDetectorConstruction());
  G4double mass = detConstruction->GetScoringVolume()->GetMass();
  G4double dose = edep / mass;
  G4double rmsDose = rms / mass;

  // Run conditions
  //  note: There is no primary generator action object for "master"
  //        run manager for multi-threaded mode.
  const auto generatorAction = static_cast<const PrimaryGeneratorAction*>(
    G4RunManager::GetRunManager()->GetUserPrimaryGeneratorAction());
  G4String runCondition;
  if (generatorAction) {
    const G4ParticleGun* particleGun = generatorAction->GetParticleGun();
    runCondition += particleGun->GetParticleDefinition()->GetParticleName();
    runCondition += " of ";
    G4double particleEnergy = particleGun->GetParticleEnergy();
    runCondition += G4BestUnit(particleEnergy, "Energy");
  }

  auto analysisManager = G4AnalysisManager::Instance();

    analysisManager->Write();
    analysisManager->CloseFile();
  // Print run summary
  if (IsMaster()) {
      G4cout << G4endl
            << "--------------------End of Global Run-----------------------";
  }
  else {
      G4cout << G4endl
            << "--------------------End of Local Run------------------------";
  }

  G4cout << G4endl
        << " The run consists of " << nofEvents << " " << runCondition << G4endl
        << " Cumulated dose per run, in scoring volume : "
        << G4BestUnit(dose, "Dose")
        << "    rms = "
        << G4BestUnit(rmsDose, "Dose")
        << G4endl
        << "------------------------------------------------------------"
        << G4endl;

  G4cout << G4endl;
  G4cout << "================ Simulation Summary ================" << G4endl;

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

  G4cout << "===================================================="
        << G4endl << G4endl;
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

void RunAction::AddEdep(G4double edep)
{
  fEdep += edep;
  fEdep2 += edep * edep;
}

void B1::RunAction::FillPrimaryEnergy(G4int eventID,
                                      G4double energy)
{
    auto analysisManager = G4AnalysisManager::Instance();

    analysisManager->FillNtupleIColumn(1,0,eventID);

    analysisManager->FillNtupleDColumn(1,1,energy/keV);

    analysisManager->AddNtupleRow(1);
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

}  // namespace B1
