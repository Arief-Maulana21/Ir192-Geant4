
#include "TrackingAction.hh"
#include "RunAction.hh"

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
    if (!fRunAction || !track)
        return;

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

} // namespace B1
