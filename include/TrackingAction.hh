
#ifndef B1TrackingAction_h
#define B1TrackingAction_h 1

#include "G4UserTrackingAction.hh"

class G4Track;

namespace B1
{

class RunAction;

class TrackingAction : public G4UserTrackingAction
{
public:

    explicit TrackingAction(RunAction* runAction);

    ~TrackingAction() override = default;

    void PreUserTrackingAction(
        const G4Track* track
    ) override;

private:

    RunAction* fRunAction = nullptr;

};

} // namespace B1

#endif
