
#include "SteppingAction.hh"

#include "DetectorConstruction.hh"
#include "EventAction.hh"
#include "G4Track.hh"
#include "G4ParticleDefinition.hh"
#include "G4VProcess.hh"
#include "G4SystemOfUnits.hh"
#include "G4ios.hh"
#include "G4Event.hh"
#include "G4LogicalVolume.hh"
#include "G4RunManager.hh"
#include "G4Step.hh"
#include "G4AnalysisManager.hh"

namespace B1
{

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

SteppingAction::SteppingAction(EventAction* eventAction) : fEventAction(eventAction) {}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

void SteppingAction::UserSteppingAction(const G4Step* step)
{
  if (!fScoringVolume) {
    const auto detConstruction = static_cast<const DetectorConstruction*>(
      G4RunManager::GetRunManager()->GetUserDetectorConstruction());
    fScoringVolume = detConstruction->GetScoringVolume();
  }

  // get volume of the current step
  G4LogicalVolume* volume =
    step->GetPreStepPoint()->GetTouchableHandle()->GetVolume()->GetLogicalVolume();
  // check if we are in scoring volume
  if (volume -> GetName()=="Lead") return;

  auto track = step->GetTrack();
  
  G4int trackID = track->GetTrackID();
  G4int parentID = track->GetParentID();


  G4String particleName =
    track->GetParticleDefinition()->GetParticleName();
 
    if(particleName!="gamma" &&
   particleName!="e-" &&
   particleName!="e+")
    return;
  G4String volumeName =
    volume->GetName();
  
  if(volumeName!="DetectorA")
      return;
  
  G4String creatorProcess = "Primary"; 

  if(track->GetCreatorProcess())
{
  creatorProcess = 
          track->GetCreatorProcess()->GetProcessName();
}
  G4String processName = "None";
  
  if(step->GetPostStepPoint()->GetProcessDefinedStep())

    processName =
      step->GetPostStepPoint()
          ->GetProcessDefinedStep()
          ->GetProcessName();

   G4double energy =
    step->GetPreStepPoint()->GetKineticEnergy();
   G4double edepStep = step->GetTotalEnergyDeposit();
   fEventAction->AddEdep(edepStep);
   G4String creatorVolume =
    step->GetPreStepPoint()
        ->GetTouchableHandle()
        ->GetVolume()
        ->GetName();
 
   auto eventID =
    G4RunManager::GetRunManager()
    ->GetCurrentEvent()
    ->GetEventID();
  
  
if( processName == "Transportation")
{
    return;
}
{
  auto analysisManager = G4AnalysisManager::Instance();
   auto pos = step->GetPreStepPoint()->GetPosition();
   auto dir = step->GetPreStepPoint()->GetMomentumDirection();
   
    analysisManager->FillNtupleIColumn(0,eventID);

    analysisManager->FillNtupleIColumn(1,trackID);

    analysisManager->FillNtupleIColumn(2,parentID);

    analysisManager->FillNtupleSColumn(3,particleName);

    analysisManager->FillNtupleDColumn(4,energy/keV);

    analysisManager->FillNtupleSColumn(5,creatorProcess);

    analysisManager->FillNtupleSColumn(6,processName);

    analysisManager->FillNtupleDColumn(7,pos.x()/cm);

    analysisManager->FillNtupleDColumn(8,pos.y()/cm);

    analysisManager->FillNtupleDColumn(9,pos.z()/cm);

    analysisManager->FillNtupleDColumn(10,edepStep/keV);

    analysisManager->FillNtupleSColumn(11,volumeName);
    
    analysisManager->FillNtupleDColumn(12,
    track->GetGlobalTime()/ns);

    analysisManager->FillNtupleDColumn(13,
    track->GetWeight());
   
    analysisManager->FillNtupleDColumn(14,dir.x());

    analysisManager->FillNtupleDColumn(15,dir.y());

    analysisManager->FillNtupleDColumn(16,dir.z());
    
    analysisManager->FillNtupleDColumn(17,track->GetTrackLength()/mm);

    analysisManager->FillNtupleSColumn(18,creatorVolume);

    analysisManager->AddNtupleRow();

} 
  }

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

}  // namespace B1
