
#ifndef B1EventAction_h
#define B1EventAction_h 1

#include "G4UserEventAction.hh"
#include "globals.hh"

class G4Event;

namespace B1
{

class RunAction;

class EventAction : public G4UserEventAction
{
public:

    explicit EventAction(RunAction* runAction);
    ~EventAction() override = default;

    void BeginOfEventAction(const G4Event* event) override;
    void EndOfEventAction(const G4Event* event) override;

    void AddEdep(G4double edep, G4bool pbHistory)
    {
        fEdep += edep;
        if (pbHistory) fEdepPb += edep;
        else fEdepNoPb += edep;
    }

private:

    RunAction* fRunAction = nullptr;

    // Total energy deposition in scoring volume per event
    G4double fEdep = 0.;
    G4double fEdepPb = 0.;
    G4double fEdepNoPb = 0.;

};

} // namespace B1

#endif
