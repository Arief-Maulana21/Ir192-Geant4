
#ifndef B1PrimaryGeneratorAction_h
#define B1PrimaryGeneratorAction_h 1

#include "G4VUserPrimaryGeneratorAction.hh"
#include "G4ThreeVector.hh"
#include "globals.hh"

#include "Ir192Spectrum.hh"

class G4ParticleGun;
class G4Event;

namespace B1
{

class PrimaryGeneratorAction
    : public G4VUserPrimaryGeneratorAction
{
public:
    PrimaryGeneratorAction();

    ~PrimaryGeneratorAction() override;

    void GeneratePrimaries(
        G4Event* event
    ) override;

    const G4ParticleGun* GetParticleGun() const
    {
        return fParticleGun;
    }

private:
    G4ParticleGun* fParticleGun = nullptr;

    Ir192Spectrum fSpectrum;
};

} // namespace B1

#endif
