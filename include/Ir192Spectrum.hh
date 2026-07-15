#ifndef Ir192Spectrum_h
#define Ir192Spectrum_h

#include <vector>
#include <string>

#include "globals.hh"

class Ir192Spectrum
{
public:

    Ir192Spectrum();

    

    G4double SampleEnergy();

private:

    void ReadSpectrumFile(const std::string& filename);
    
    std::vector<G4double> fEnergy;
    std::vector<G4double> fProbability; 
    std::vector<G4double> fCDF;

};

#endif