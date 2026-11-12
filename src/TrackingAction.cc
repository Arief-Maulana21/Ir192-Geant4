
#include "TrackingAction.hh"
#include "RunAction.hh"
#include "TrackInformation.hh"
#include "G4TrackingManager.hh"

#include "G4Track.hh"
#include "G4Gamma.hh"
#include "G4Electron.hh"
#include "G4Positron.hh"
#include "G4VProcess.hh"

namespace B1
{

TrackingAction::TrackingAction(
    RunAction* runAction
)
    : fRunAction(runAction)
{
}


void TrackingAction::PreUserTrackingAction(
    const G4Track* track
)
{
    if (!track) return;

    // Every track receives its own metadata. For secondaries,
    // metadata may already have been assigned by PostUserTrackingAction.
    if (!track->GetUserInformation())
    {
        const_cast<G4Track*>(track)->SetUserInformation(
            new TrackInformation());
    }

    if (!fRunAction) return;

    auto particle =
        track->GetParticleDefinition();

    G4int parentID =
        track->GetParentID();

    // ================================================
    // PRIMARY GAMMA
    // ================================================

    if (particle == G4Gamma::Definition() &&
        parentID == 0)
    {
        fRunAction->AddPrimaryGamma();
    }


    // ================================================
    // SECONDARY GAMMA
    // ================================================

    if (particle == G4Gamma::Definition() &&
        parentID > 0)
    {
        fRunAction->AddSecondaryGamma();
    }


    // ================================================
    // SECONDARY ELECTRON
    // ================================================

    if (particle == G4Electron::Definition() &&
        parentID > 0)
    {
        fRunAction->AddElectron();
    }
}

void TrackingAction::PostUserTrackingAction(const G4Track* track)
{
    if (!track || !fpTrackingManager) return;

    const auto* info = dynamic_cast<const TrackInformation*>(
        track->GetUserInformation());
    if (!info) return;

    auto* secondaries = fpTrackingManager->GimmeSecondaries();
    if (!secondaries) return;

    for (auto* child : *secondaries)
    {
        if (!child) continue;
        // Child may have been tagged at the producing step.
        auto* childInfo = dynamic_cast<TrackInformation*>(
            child->GetUserInformation());
        if (!childInfo)
        {
            child->SetUserInformation(
                new TrackInformation(info->HasPbHistory()));
        }
        else if (info->HasPbHistory())
        {
            childInfo->MarkPbHistory();
        }
    }
}

} // namespace B1
