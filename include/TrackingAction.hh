#ifndef TrackingAction_h
#define TrackingAction_h 1

#include "G4UserTrackingAction.hh"

class G4Track;

namespace B1
{

class RunAction;

class TrackingAction : public G4UserTrackingAction
{
public:

    TrackingAction(RunAction*);
    virtual ~TrackingAction() = default;

    virtual void PreUserTrackingAction(const G4Track*) override;

private:

    RunAction* fRunAction;

};

} // namespace B1

#endif