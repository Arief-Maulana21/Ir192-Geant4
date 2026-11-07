
#ifndef B1SteppingAction_h
#define B1SteppingAction_h 1

#include "G4UserSteppingAction.hh"

class G4LogicalVolume;
class G4Step;

namespace B1
{

class EventAction;

class SteppingAction : public G4UserSteppingAction
{
public:

    explicit SteppingAction(EventAction* eventAction);
    ~SteppingAction() override = default;

    void UserSteppingAction(const G4Step* step) override;

private:

    EventAction* fEventAction = nullptr;

    G4LogicalVolume* fScoringVolume = nullptr;

};

} // namespace B1

#endif
