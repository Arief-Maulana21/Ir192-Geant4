
#include "SteppingAction.hh"
#include "DetectorConstruction.hh"
#include "EventAction.hh"
#include "RunAction.hh"
#include "TrackInformation.hh"

#include "G4Track.hh"
#include "G4Step.hh"
#include "G4StepPoint.hh"
#include "G4ParticleDefinition.hh"
#include "G4VProcess.hh"
#include "G4ProcessType.hh"
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

    const auto* detector=static_cast<const DetectorConstruction*>(G4RunManager::GetRunManager()->GetUserDetectorConstruction());
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



    // =================================================
    // 4. PARTICLE INFORMATION
    // =================================================

    G4String particleName =
        track->GetParticleDefinition()
            ->GetParticleName();

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
    // 6. STEP PROCESS
    // =================================================

    G4String processName = "None";

    if (postStepPoint->GetProcessDefinedStep())
    {
        processName =
            postStepPoint->GetProcessDefinedStep()
                ->GetProcessName();
    }

    const auto* process=postStepPoint->GetProcessDefinedStep();
    const G4bool isPbInteraction=volumeName=="PbShield" && process && process->GetProcessType()==fElectromagnetic;
    auto* trackInfo=dynamic_cast<TrackInformation*>(track->GetUserInformation());
    if(!trackInfo) { trackInfo=new TrackInformation(); track->SetUserInformation(trackInfo); }
    // Pre-step state classifies deposition before the endpoint scattering.
    G4int particle=8;
    if(particleName=="gamma") particle=parentID==0?(trackInfo->scattered?4:3):5;
    else if(parentID>0 && particleName=="e-") particle=6;
    else if(parentID>0 && particleName=="e+") particle=7;
    for(G4int i=0;i<NOrg;++i) if(detector && logicalVolume==detector->GetOrganVolume(i) && fEventAction) {
        fEventAction->AddOrganEdep(i,edepStep*preStepPoint->GetWeight(),trackInfo->HasPbHistory(),particle,parentID==0?9:BirthCategory(trackInfo->birth));
        break;
    }
    if(isPbInteraction) trackInfo->MarkPbHistory();
    if(particleName=="gamma" && (processName=="compt"||processName=="Rayl")) trackInfo->scattered=true;
    // Snapshot lineage at birth; never retroactively tag earlier children.
    const auto* children=step->GetSecondaryInCurrentStep();
    if(children) for(const auto* c:*children) {
        auto* child=const_cast<G4Track*>(c);
        if(!child) continue;
        auto* ci=dynamic_cast<TrackInformation*>(child->GetUserInformation());
        if(!ci) { ci=new TrackInformation(trackInfo->HasPbHistory()); child->SetUserInformation(ci); }
        if(trackInfo->HasPbHistory()) ci->MarkPbHistory();
        ci->birth=volumeName;
    }
    if(particleName!="gamma" && particleName!="e-" && particleName!="e+") return;
    // Exclude transportation-only steps from logging.
    if (processName == "Transportation" ||
        processName == "CoupledTransportation")
    {
        return;
    }

    // =====================================================
    // Pb INTERACTION ANALYSIS
    // =====================================================

    if (volumeName == "PbShield" &&
        particleName == "gamma")
    {
        const bool isCompton =
        (processName == "compt");

        const bool isPhotoelectric =
        (processName == "phot");

        const bool isRayleigh =
        (processName == "Rayl");

    if (isCompton || isPhotoelectric || isRayleigh)
    {
        auto runAction =
            const_cast<RunAction*>(
                static_cast<const RunAction*>(
                    G4RunManager::GetRunManager()
                        ->GetUserRunAction()
                )
            );

        if (runAction)
        {
            if (isCompton)
                runAction->AddCompton();

            if (isPhotoelectric)
                runAction->AddPhotoelectric();

            if (isRayleigh)
                runAction->AddRayleigh();
        }

        const G4ThreeVector interactionPosition =
            postStepPoint->GetPosition();

        const G4double energyBefore =
            preStepPoint->GetKineticEnergy();

        const G4double energyAfter =
            postStepPoint->GetKineticEnergy();

        const auto* secondaries =
            step->GetSecondaryInCurrentStep();

        const G4int nSecondaries =
            secondaries
                ? static_cast<G4int>(secondaries->size())
                : 0;

        auto currentEvent =
            G4RunManager::GetRunManager()
                ->GetCurrentEvent();

        if (currentEvent)
        {
            const G4int eventID =
                currentEvent->GetEventID();

            auto analysis =
                G4AnalysisManager::Instance();

            analysis->FillNtupleIColumn(
                2, 0, eventID);

            analysis->FillNtupleIColumn(
                2, 1, trackID);

            analysis->FillNtupleIColumn(
                2, 2, parentID);

            analysis->FillNtupleSColumn(
                2, 3, particleName);

            analysis->FillNtupleSColumn(
                2, 4, processName);

            analysis->FillNtupleDColumn(
                2, 5, energyBefore / keV);

            analysis->FillNtupleDColumn(
                2, 6, energyAfter / keV);

            analysis->FillNtupleDColumn(
                2, 7, edepStep / keV);

            analysis->FillNtupleDColumn(
                2, 8, interactionPosition.x() / mm);

            analysis->FillNtupleDColumn(
                2, 9, interactionPosition.y() / mm);

            analysis->FillNtupleDColumn(
                2, 10, interactionPosition.z() / mm);

            analysis->FillNtupleIColumn(
                2, 11, nSecondaries);

            analysis->AddNtupleRow(2);
        }
    }
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

    G4String creatorVolume = parentID==0 ? "Primary" : trackInfo->birth;

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
