#include "PrimaryGeneratorAction.hh"

#include "G4ParticleGun.hh"
#include "G4Gamma.hh"
#include "G4ThreeVector.hh"
#include "G4SystemOfUnits.hh"
#include "G4GeneralParticleSource.hh"
#include "G4Event.hh"
#include "Ir192Spectrum.hh"
#include "RunAction.hh"
#include "G4RunManager.hh"
#include "G4AnalysisManager.hh"

namespace B1
{

PrimaryGeneratorAction::PrimaryGeneratorAction()
{  
    fParticleGun = new G4ParticleGun(1);

    fParticleGun->SetParticleDefinition(G4Gamma::Definition());

    fParticleGun->SetParticlePosition(G4ThreeVector(0,0.,0.));

    fParticleGun->SetParticleMomentumDirection(G4ThreeVector(0.,0.,1.));

    fSpectrum = new Ir192Spectrum();
}

PrimaryGeneratorAction::~PrimaryGeneratorAction()
{
    delete fParticleGun;
    delete fSpectrum;
}

void PrimaryGeneratorAction::GeneratePrimaries(G4Event* event)
{
   G4double energy = fSpectrum->SampleEnergy();

        auto analysisManager = G4AnalysisManager::Instance();

    analysisManager->FillNtupleIColumn(
        1,
        0,
        event->GetEventID());

    analysisManager->FillNtupleDColumn(
        1,
        1,
        energy/keV);

    analysisManager->AddNtupleRow(1);

    fParticleGun->SetParticleEnergy(energy);

    fParticleGun->GeneratePrimaryVertex(event);
}

}
