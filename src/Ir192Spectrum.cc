#include "Ir192Spectrum.hh"

#include "Randomize.hh"
#include "G4SystemOfUnits.hh"
#include "G4Exception.hh"
#include "G4ios.hh"

#include <fstream>
#include <sstream>
#include <vector>

Ir192Spectrum::Ir192Spectrum()
{
    ReadSpectrumFile("../data/Ir192Spectrum.dat");
}

//======================================================

void Ir192Spectrum::ReadSpectrumFile(const std::string& filename)
{
    std::ifstream file(filename);

    if(!file.is_open())
    {
        G4Exception(
            "Ir192Spectrum",
            "FileNotFound",
            FatalException,
            "Cannot open Ir192Spectrum.dat");
    }

    std::vector<G4double> intensity;

    std::string line;

    while(std::getline(file,line))
    {
        if(line.empty()) continue;

        if(line[0]=='#') continue;

        std::stringstream ss(line);

        G4double E,I;

        ss >> E >> I;

        fEnergy.push_back(E*keV);

        intensity.push_back(I);
    }

    file.close();

    G4double sum = 0.0;

    for(auto x : intensity)
        sum += x;

    G4double cumulative = 0.0;

    for(auto x : intensity)
    {
        cumulative += x/sum;
        fCDF.push_back(cumulative);
    }

    G4cout << G4endl;
    G4cout << "==================================" << G4endl;
    G4cout << "Ir-192 spectrum loaded successfully" << G4endl;
    G4cout << "Gamma lines : " << fEnergy.size() << G4endl;
    G4cout << "==================================" << G4endl;
}

//======================================================

G4double Ir192Spectrum::SampleEnergy()
{
    G4double r = G4UniformRand();

    for(size_t i=0;i<fCDF.size();i++)
    {
        if(r < fCDF[i])
            return fEnergy[i];
    }

    return fEnergy.back();
}