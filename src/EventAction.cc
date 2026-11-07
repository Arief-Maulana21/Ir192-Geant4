
#include "EventAction.hh"
#include "RunAction.hh"

namespace B1
{

EventAction::EventAction(RunAction* runAction)
    : fRunAction(runAction)
{
}


// =====================================================
// BEGIN OF EVENT
// =====================================================

void EventAction::BeginOfEventAction(const G4Event*)
{
    // Reset energy deposition for each event
    fEdep = 0.;
}


// =====================================================
// END OF EVENT
// =====================================================

void EventAction::EndOfEventAction(const G4Event*)
{
    // Send total event energy deposition to RunAction.
    // RunAction accumulates Edep and Edep^2 per event.
    if (fRunAction)
    {
        fRunAction->AddEdep(fEdep);
    }
}

} // namespace B1
