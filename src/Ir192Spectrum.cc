
#include "Ir192Spectrum.hh"

#include "Randomize.hh"
#include "G4SystemOfUnits.hh"
#include "G4Exception.hh"
#include "G4ios.hh"

#include <fstream>
#include <sstream>
#include <algorithm>
#include <cmath>

Ir192Spectrum::Ir192Spectrum()
{
    ReadSpectrumFile("../data/Ir192Spectrum.dat");
}

// =====================================================
// READ IR-192 GAMMA SPECTRUM
// =====================================================

void Ir192Spectrum::ReadSpectrumFile(
    const std::string& filename)
{
    std::ifstream file(filename);

    if (!file.is_open())
    {
        G4ExceptionDescription msg;
        msg << "Cannot open spectrum file: "
            << filename;

        G4Exception(
            "Ir192Spectrum::ReadSpectrumFile",
            "FileNotFound",
            FatalException,
            msg
        );

        return;
    }

    fEnergy.clear();
    fProbability.clear();
    fCDF.clear();

    std::string line;
    G4double sum = 0.0;

    while (std::getline(file, line))
    {
        // Skip blank and comment lines
        auto first = line.find_first_not_of(
            " \t\r\n"
        );

        if (first == std::string::npos)
            continue;

        if (line[first] == '#')
            continue;

        std::stringstream ss(line);

        G4double energy = 0.0;
        G4double intensity = 0.0;

        if (!(ss >> energy >> intensity))
        {
            G4Exception(
                "Ir192Spectrum::ReadSpectrumFile",
                "InvalidData",
                FatalException,
                "Invalid spectrum data row."
            );

            return;
        }

        if (!std::isfinite(energy) ||
            !std::isfinite(intensity) ||
            energy <= 0.0 ||
            intensity < 0.0)
        {
            G4Exception(
                "Ir192Spectrum::ReadSpectrumFile",
                "InvalidValue",
                FatalException,
                "Invalid energy or intensity."
            );

            return;
        }

        fEnergy.push_back(energy * keV);
        fProbability.push_back(intensity);

        sum += intensity;
    }

    file.close();

    if (fEnergy.empty() || sum <= 0.0 ||
        !std::isfinite(sum))
    {
        G4Exception(
            "Ir192Spectrum::ReadSpectrumFile",
            "EmptySpectrum",
            FatalException,
            "Spectrum is empty or has invalid total intensity."
        );

        return;
    }

    // Normalize intensities and build CDF
    G4double cumulative = 0.0;

    for (auto& probability : fProbability)
    {
        probability /= sum;

        cumulative += probability;

        fCDF.push_back(cumulative);
    }

    // Avoid rounding problems
    fCDF.back() = 1.0;

    G4cout << G4endl;
    G4cout << "=================================="
           << G4endl;
    G4cout << "Ir-192 gamma spectrum loaded"
           << G4endl;
    G4cout << "Gamma lines : "
           << fEnergy.size()
           << G4endl;
    G4cout << "Total input intensity : "
           << sum
           << G4endl;
    G4cout << "Normalized CDF : "
           << fCDF.back()
           << G4endl;
    G4cout << "=================================="
           << G4endl;
}

// =====================================================
// SAMPLE GAMMA ENERGY
// =====================================================

G4double Ir192Spectrum::SampleEnergy()
{
    G4double r = G4UniformRand();

    auto it = std::upper_bound(
        fCDF.begin(),
        fCDF.end(),
        r
    );

    if (it == fCDF.end())
        return fEnergy.back();

    auto index = static_cast<size_t>(
        std::distance(fCDF.begin(), it)
    );

    return fEnergy[index];
}
