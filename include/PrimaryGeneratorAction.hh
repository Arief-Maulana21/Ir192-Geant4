
#ifndef PrimaryGeneratorAction_h
#define PrimaryGeneratorAction_h

#include "G4VUserPrimaryGeneratorAction.hh"
#include "G4ParticleGun.hh"
#include "globals.hh"
#include "G4ThreeVector.hh"

#include <vector>

class G4Event;
class Ir192Spectrum;

namespace B1
{

class PrimaryGeneratorAction : public G4VUserPrimaryGeneratorAction
{
public:

    PrimaryGeneratorAction();
    ~PrimaryGeneratorAction() override;

    void GeneratePrimaries(G4Event* event) override;

    const G4ParticleGun* GetParticleGun() const
    {
        return fParticleGun;
    }

private:

    G4ParticleGun* fParticleGun = nullptr;
    Ir192Spectrum* fSpectrum = nullptr;

    // Dwell positions relative to the applicator center
    std::vector<G4double> fDwellPositions;

    // Position of the applicator center in World coordinates
    G4ThreeVector fApplicatorPosition;

};

}

#endif
