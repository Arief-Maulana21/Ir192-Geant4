#include "TrackingAction.hh"
#include "RunAction.hh"

#include "G4Track.hh"
#include "G4ParticleDefinition.hh"
#include "G4VProcess.hh"

namespace B1
{

TrackingAction::TrackingAction(RunAction* runAction)
: fRunAction(runAction)
{
}

void TrackingAction::PreUserTrackingAction(const G4Track* track)
{
    auto particle =
        track->GetParticleDefinition()->GetParticleName();

    auto parent =
        track->GetParentID();
    
    auto process =
        track->GetCreatorProcess();
    if(process)
        {
        auto name = process->GetProcessName();

        if(name=="compt")
         fRunAction->AddCompton();

        if(name=="phot")
         fRunAction->AddPhotoelectric();

         if(name=="Rayl")
         fRunAction->AddRayleigh();
        }

    // Primary gamma
    if(parent==0 && particle=="gamma")
    {
        fRunAction->AddPrimaryGamma();
    }

    // Secondary gamma
    if(parent>0 && particle=="gamma")
    {
        fRunAction->AddSecondaryGamma();
    }

    // Secondary electron
    if(parent>0 && particle=="e-")
    {
        fRunAction->AddElectron();
    }
}

} // namespace B1