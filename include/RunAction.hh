
#ifndef B1RunAction_h
#define B1RunAction_h 1

#include "G4UserRunAction.hh"
#include "G4Accumulable.hh"
#include "globals.hh"

class G4Run;

namespace B1
{

class RunAction : public G4UserRunAction
{
public:

    RunAction();
    ~RunAction() override = default;

    void BeginOfRunAction(const G4Run*) override;
    void EndOfRunAction(const G4Run*) override;

    // Energy deposition
    void AddEdep(G4double edep);

    // Particle counters
    void AddPrimaryGamma()   { ++fPrimaryGamma; }
    void AddSecondaryGamma() { ++fSecondaryGamma; }
    void AddElectron()       { ++fElectron; }

    // Interaction counters
    void AddCompton()       { ++fCompton; }
    void AddPhotoelectric() { ++fPhotoelectric; }
    void AddRayleigh()      { ++fRayleigh; }

    // Primary gamma information
    void FillPrimaryEnergy(
        G4int eventID,
        G4double energy,
        G4int dwellIndex,
        G4double sourceZ
    );

private:

    G4Accumulable<G4double> fEdep{0.};
    G4Accumulable<G4double> fEdep2{0.};

    G4Accumulable<G4int> fPrimaryGamma{0};
    G4Accumulable<G4int> fSecondaryGamma{0};
    G4Accumulable<G4int> fElectron{0};

    G4Accumulable<G4int> fCompton{0};
    G4Accumulable<G4int> fPhotoelectric{0};
    G4Accumulable<G4int> fRayleigh{0};

};

} // namespace B1

#endif
