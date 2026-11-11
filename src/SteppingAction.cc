
#include "SteppingAction.hh"
#include "DetectorConstruction.hh"
#include "EventAction.hh"
#include "RunAction.hh"

#include "G4Track.hh"
#include "G4Step.hh"
#include "G4StepPoint.hh"
#include "G4ParticleDefinition.hh"
#include "G4VProcess.hh"
#include "G4LogicalVolume.hh"
#include "G4VPhysicalVolume.hh"

#include "G4RunManager.hh"
#include "G4Event.hh"
#include "G4AnalysisManager.hh"

#include "G4SystemOfUnits.hh"
#include "G4ios.hh"

namespace B1
{

SteppingAction::SteppingAction(EventAction* eventAction)
    : fEventAction(eventAction)
{
}


// =====================================================
// USER STEPPING ACTION
// =====================================================

void SteppingAction::UserSteppingAction(const G4Step* step)
{
    // =================================================
    // 1. GET SCORING VOLUME
    // =================================================

    if (!fScoringVolume)
    {
        const auto detConstruction =
            static_cast<const DetectorConstruction*>(
                G4RunManager::GetRunManager()
                    ->GetUserDetectorConstruction()
            );

        fScoringVolume =
            detConstruction->GetScoringVolume();
    }


    // =================================================
    // 2. GET TRACK AND STEP INFORMATION
    // =================================================

    auto track = step->GetTrack();

    auto preStepPoint = step->GetPreStepPoint();
    auto postStepPoint = step->GetPostStepPoint();

    auto physicalVolume =
        preStepPoint->GetPhysicalVolume();

    if (!physicalVolume)
        return;

    auto logicalVolume =
        physicalVolume->GetLogicalVolume();

    G4String volumeName =
        physicalVolume->GetName();

    G4double edepStep =
        step->GetTotalEnergyDeposit();


    // =================================================
    // 3. ENERGY DEPOSITION FOR DOSE
    // =================================================

    // Only score deposited energy in DetectorA.
    // This is independent of the ROOT step logging.

    if (logicalVolume == fScoringVolume)
    {
        fEventAction->AddEdep(edepStep);
    }


    // =================================================
    // 4. PARTICLE INFORMATION
    // =================================================

    G4String particleName =
        track->GetParticleDefinition()
            ->GetParticleName();

    // Record gamma, electron and positron steps.
    if (particleName != "gamma" &&
        particleName != "e-" &&
        particleName != "e+")
    {
        return;
    }

    G4int trackID = track->GetTrackID();
    G4int parentID = track->GetParentID();


    // =================================================
    // 5. CREATOR PROCESS
    // =================================================

    G4String creatorProcess = "Primary";

    if (track->GetCreatorProcess())
    {
        creatorProcess =
            track->GetCreatorProcess()
                ->GetProcessName();
    }


    // =================================================
    // COUNT GAMMA INTERACTIONS IN Pb
    // =================================================

if (volumeName == "PbShield" &&
    particleName == "gamma")
    {
    auto runAction =
    const_cast<RunAction*>(
        static_cast<const RunAction*>(
            G4RunManager::GetRunManager()
                ->GetUserRunAction()
        )
    );
    const G4VProcess* process =
    step->GetPostStepPoint()
        ->GetProcessDefinedStep();

    G4String processName = "";

    if (process)
    {
    processName = process->GetProcessName();
    }



    if (runAction)
        {
        if (processName == "compt")
            {
            runAction->AddCompton();
            }
        else if (processName == "phot")
            {
            runAction->AddPhotoelectric();
            }
        else if (processName == "Rayl")
            {
            runAction->AddRayleigh();
            }
        }
    }

    // =================================================
    // 6. STEP PROCESS
    // =================================================

    G4String processName = "None";

    if (postStepPoint->GetProcessDefinedStep())
    {
        processName =
            postStepPoint->GetProcessDefinedStep()
                ->GetProcessName();
    }

    // Exclude transportation-only steps from logging.
    if (processName == "Transportation" ||
        processName == "CoupledTransportation")
    {
        return;
    }


    // =================================================
    // 7. ENERGY AND POSITION
    // =================================================

    G4double energy =
        preStepPoint->GetKineticEnergy();

    G4ThreeVector position =
        preStepPoint->GetPosition();

    G4ThreeVector direction =
        preStepPoint->GetMomentumDirection();

    G4double globalTime =
        preStepPoint->GetGlobalTime();

    G4double weight =
        track->GetWeight();

    G4double trackLength =
        track->GetTrackLength();


    // =================================================
    // 8. CREATOR VOLUME
    // =================================================

    // This field records the current step volume.
    // True particle birth volume requires tracking
    // information recorded when the secondary is created.

    G4String creatorVolume = "Unknown";

    if (parentID == 0)
    {
        creatorVolume = "Primary";
    }
    else
    {
        creatorVolume = "NotTracked";
    }


    // =================================================
    // 9. EVENT ID
    // =================================================

    auto currentEvent =
        G4RunManager::GetRunManager()
            ->GetCurrentEvent();

    if (!currentEvent)
        return;

    G4int eventID =
        currentEvent->GetEventID();


    // =================================================
    // 10. FILL SECONDARY NTUPLE (ID = 0)
    // =================================================

    auto analysisManager =
        G4AnalysisManager::Instance();

    analysisManager->FillNtupleIColumn(
        0, 0, eventID);

    analysisManager->FillNtupleIColumn(
        0, 1, trackID);

    analysisManager->FillNtupleIColumn(
        0, 2, parentID);

    analysisManager->FillNtupleSColumn(
        0, 3, particleName);

    analysisManager->FillNtupleDColumn(
        0, 4, energy / keV);

    analysisManager->FillNtupleSColumn(
        0, 5, creatorProcess);

    analysisManager->FillNtupleSColumn(
        0, 6, processName);

    analysisManager->FillNtupleDColumn(
        0, 7, position.x() / cm);

    analysisManager->FillNtupleDColumn(
        0, 8, position.y() / cm);

    analysisManager->FillNtupleDColumn(
        0, 9, position.z() / cm);

    analysisManager->FillNtupleDColumn(
        0, 10, edepStep / keV);

    analysisManager->FillNtupleSColumn(
        0, 11, volumeName);

    analysisManager->FillNtupleDColumn(
        0, 12, globalTime / ns);

    analysisManager->FillNtupleDColumn(
        0, 13, weight);

    analysisManager->FillNtupleDColumn(
        0, 14, direction.x());

    analysisManager->FillNtupleDColumn(
        0, 15, direction.y());

    analysisManager->FillNtupleDColumn(
        0, 16, direction.z());

    analysisManager->FillNtupleDColumn(
        0, 17, trackLength / mm);

    analysisManager->FillNtupleSColumn(
        0, 18, creatorVolume);

    analysisManager->AddNtupleRow(0);
}

} // namespace B1
