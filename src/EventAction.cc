
#include "EventAction.hh"
#include "RunAction.hh"

#include "G4Event.hh"

namespace B1
{

EventAction::EventAction(
    RunAction* runAction
)
    : fRunAction(runAction)
{
}

// =====================================================
// BEGIN OF EVENT
// =====================================================

void EventAction::BeginOfEventAction(
    const G4Event*
)
{
    fEdep = 0.;
}

// =====================================================
// END OF EVENT
// =====================================================

void EventAction::EndOfEventAction(
    const G4Event* event
)
{
    if (!fRunAction || !event)
        return;

    // Accumulate total energy deposition
    fRunAction->AddEdep(fEdep);

    // Record energy deposition for this event
    fRunAction->RecordEventDose(
        event->GetEventID(),
        fEdep
    );
}

} // namespace B1
