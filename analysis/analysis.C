
#include <TROOT.h>
#include <TFile.h>
#include <TTree.h>
#include <TH1D.h>
#include <TH2D.h>
#include <TCanvas.h>
#include <TStyle.h>
#include <TSystem.h>
#include <TDirectory.h>

#include <iostream>
#include <fstream>
#include <set>
#include <utility>
#include <string>

void analysis()
{
    // ==========================================
    // 1. INITIALIZATION
    // ==========================================

    gStyle->SetOptStat(111111);

    gSystem->mkdir("../Result", true);

    TFile* f = TFile::Open(
        "../build/SecondaryRadiation.root",
        "READ"
    );

    if (!f || f->IsZombie())
    {
        std::cerr << "ROOT file not found\n";
        return;
    }

    TTree* t = nullptr;
    f->GetObject("Secondary", t);

    if (!t)
    {
        std::cerr << "Secondary tree not found\n";
        f->Close();
        return;
    }

    std::cout << "Entries = "
              << t->GetEntries()
              << std::endl;

    // Check required branches
    const char* required[] = {
        "EventID", "TrackID", "ParentID",
        "Particle", "Energy_keV", "Process",
        "Volume", "Edep_keV",
        "X_cm", "Y_cm", "Z_cm",
        "TrackLength_mm"
    };

    for (const char* name : required)
    {
        if (!t->GetBranch(name))
        {
            std::cerr
                << "Missing branch: "
                << name << std::endl;

            f->Close();
            return;
        }
    }

    // ==========================================
    // 2. SELECTION DEFINITIONS
    // ==========================================

    TString detector =
        "Volume==\"DetectorA\"";

    TString primaryTrack =
        "ParentID==0 && Particle==\"gamma\"";

    TString secondaryTrack =
        "ParentID>0";

    TString pb =
        "Volume==\"PbShield\"";

    // ==========================================
    // 3. PRIMARY-TRACK ENERGY DEPOSITION
    // ==========================================

    TH1D* hPrimaryEdep = new TH1D(
        "hPrimaryEdep",
        "Primary Gamma Track Edep in Detector A;"
        "Edep per step (keV);Step entries",
        300, 0, 700
    );

    t->Draw(
        "Edep_keV>>hPrimaryEdep",
        detector + " && " + primaryTrack
                 + " && Edep_keV>0",
        "goff"
    );

    // ==========================================
    // 4. ELECTRON ENERGY SPECTRUM
    // ==========================================

    TH1D* hElectron = new TH1D(
        "hElectron",
        "Electron Step Energy Spectrum;"
        "Kinetic energy (keV);Step entries",
        700, 0, 1200
    );

    hElectron->SetLineColor(kBlue+2);
    hElectron->SetLineWidth(2);

    t->Draw(
        "Energy_keV>>hElectron",
        "Particle==\"e-\"",
        "goff"
    );

    // ==========================================
    // 5. GAMMA ENERGY SPECTRUM
    // ==========================================

    TH1D* hGamma = new TH1D(
        "hGamma",
        "Gamma Step Energy Spectrum;"
        "Kinetic energy (keV);Step entries",
        600, 0, 1200
    );

    t->Draw(
        "Energy_keV>>hGamma",
        "Particle==\"gamma\"",
        "goff"
    );

    // ==========================================
    // 6. ENERGY DEPOSITION IN DETECTOR A
    // ==========================================

    TH1D* hDose = new TH1D(
        "hDose",
        "Energy Deposition in Detector A;"
        "Edep per step (keV);Step entries",
        300, 0, 700
    );

    t->Draw(
        "Edep_keV>>hDose",
        detector + " && Edep_keV>0",
        "goff"
    );

    // ==========================================
    // 7. Z POSITION DISTRIBUTION
    // ==========================================

    TH1D* hZ = new TH1D(
        "hZ",
        "Interaction Position Along Z;"
        "Z (cm);Step entries",
        250, -5, 15
    );

    t->Draw(
        "Z_cm>>hZ",
        "",
        "goff"
    );

    // ==========================================
    // 8. XY DISTRIBUTION
    // ==========================================

    TH2D* hXY = new TH2D(
        "hXY",
        "XY Interaction Position;"
        "X (cm);Y (cm)",
        200, -15, 15,
        200, -15, 15
    );

    t->Draw(
        "Y_cm:X_cm>>hXY",
        "",
        "goff"
    );

    // ==========================================
    // 9. TRACK LENGTH
    // ==========================================

    TH1D* hTrack = new TH1D(
        "hTrack",
        "Track Length at Recorded Steps;"
        "Cumulative track length (mm);Step entries",
        200, 0, 200
    );

    t->Draw(
        "TrackLength_mm>>hTrack",
        "",
        "goff"
    );

    // ==========================================
    // 10. PB INTERACTIONS
    // ==========================================

    TH1D* hPbEnergy = new TH1D(
        "hPbEnergy",
        "Gamma Interactions in Pb;"
        "Gamma kinetic energy (keV);Step entries",
        600, 0, 1200
    );

    t->Draw(
        "Energy_keV>>hPbEnergy",
        pb + " && Particle==\"gamma\"",
        "goff"
    );

    Long64_t nCompton = t->GetEntries(
        pb + " && Particle==\"gamma\""
             " && Process==\"compt\""
    );

    Long64_t nPhotoelectric = t->GetEntries(
        pb + " && Particle==\"gamma\""
             " && Process==\"phot\""
    );

    Long64_t nRayleigh = t->GetEntries(
        pb + " && Particle==\"gamma\""
             " && Process==\"Rayl\""
    );

    // ==========================================
    // 11. COUNT RECORDED UNIQUE TRACKS
    // ==========================================

    std::set<std::pair<Long64_t, Long64_t>>
        primaryIDs, electronIDs, gammaIDs;

    Long64_t eventID = 0;
    Long64_t trackID = 0;
    Long64_t parentID = 0;

    // TTree::Draw is used here to avoid assuming
    // the integer branch storage type.
    Long64_t nRows = t->Draw(
        "EventID:TrackID:ParentID:"
        "(Particle==\"gamma\"):"
        "(Particle==\"e-\")",
        "",
        "goff"
    );

    if (nRows < 0)
    {
        std::cerr << "Failed to read track data\n";
        f->Close();
        return;
    }

    for (Long64_t i = 0; i < nRows; ++i)
    {
        eventID = static_cast<Long64_t>(
            t->GetVal(0)[i]
        );

        trackID = static_cast<Long64_t>(
            t->GetVal(1)[i]
        );

        parentID = static_cast<Long64_t>(
            t->GetVal(2)[i]
        );

        bool isGamma = t->GetVal(3)[i] != 0;
        bool isElectron = t->GetVal(4)[i] != 0;

        auto key = std::make_pair(
            eventID, trackID
        );

        if (isGamma && parentID == 0)
            primaryIDs.insert(key);

        if (isGamma && parentID > 0)
            gammaIDs.insert(key);

        if (isElectron && parentID > 0)
            electronIDs.insert(key);
    }

    const auto nPrimary =
        primaryIDs.size();

    const auto nSecondaryGamma =
        gammaIDs.size();

    const auto nSecondaryElectron =
        electronIDs.size();

    // ==========================================
    // 12. ENERGY DEPOSITION SUMMARY
    // ==========================================

    // Includes recorded gamma, electron,
    // and positron steps only.
    Double_t totalEdepKeV =
        t->GetEntries() > 0
        ? t->GetEntries(detector)
          * 0.0
        : 0.0;

    // Sum energy deposition using a 1-bin
    // weighted histogram.
    TH1D* hTotalEdep = new TH1D(
        "hTotalEdep",
        "Total Recorded Edep",
        1, 0, 1
    );

    t->Draw(
        "0.5>>hTotalEdep",
        "(" + detector + ")*Edep_keV",
        "goff"
    );

    totalEdepKeV =
        hTotalEdep->GetBinContent(1);

    // ==========================================
    // 13. SAVE PLOTS
    // ==========================================

    auto savePlot = [](
        TH1* hist,
        const char* filename,
        const char* option = ""
    )
    {
        TCanvas canvas(
            "canvas", "Analysis", 900, 650
        );

        hist->Draw(option);
        canvas.SaveAs(filename);
    };

    savePlot(
        hPrimaryEdep,
        "../Result/PrimaryTrackEdep.png"
    );

    savePlot(
        hElectron,
        "../Result/ElectronSpectrum.png"
    );

    savePlot(
        hGamma,
        "../Result/GammaSpectrum.png"
    );

    savePlot(
        hDose,
        "../Result/EnergyDeposition.png"
    );

    savePlot(
        hZ,
        "../Result/ZDistribution.png"
    );

    savePlot(
        hXY,
        "../Result/XYDistribution.png",
        "COLZ"
    );

    savePlot(
        hTrack,
        "../Result/TrackLength.png"
    );

    savePlot(
        hPbEnergy,
        "../Result/PbInteractionEnergy.png"
    );

    // ==========================================
    // 14. SUMMARY
    // ==========================================

    std::cout
        << "\n========== SUMMARY ==========\n"
        << "Recorded step entries : "
        << t->GetEntries() << "\n"
        << "Recorded primary gamma tracks : "
        << nPrimary << "\n"
        << "Recorded secondary gamma tracks : "
        << nSecondaryGamma << "\n"
        << "Recorded secondary electron tracks : "
        << nSecondaryElectron << "\n"
        << "Compton steps in Pb : "
        << nCompton << "\n"
        << "Photoelectric steps in Pb : "
        << nPhotoelectric << "\n"
        << "Rayleigh steps in Pb : "
        << nRayleigh << "\n"
        << "Recorded Edep in Detector A : "
        << totalEdepKeV << " keV\n"
        << "=============================\n";

    // ==========================================
    // 15. SAVE TEXT SUMMARY
    // ==========================================

    std::ofstream out(
        "../Result/SpectrumSummary.txt"
    );

    out << "========== Ir-192 Simulation ==========\n";

    out << "Recorded step entries : "
        << t->GetEntries() << "\n";

    out << "Recorded primary gamma tracks : "
        << nPrimary << "\n";

    out << "Recorded secondary gamma tracks : "
        << nSecondaryGamma << "\n";

    out << "Recorded secondary electron tracks : "
        << nSecondaryElectron << "\n";

    out << "Compton steps in Pb : "
        << nCompton << "\n";

    out << "Photoelectric steps in Pb : "
        << nPhotoelectric << "\n";

    out << "Rayleigh steps in Pb : "
        << nRayleigh << "\n";

    out << "Recorded Edep in Detector A (keV) : "
        << totalEdepKeV << "\n";

    out.close();

    f->Close();

    std::cout
        << "\nAnalysis completed.\n"
        << "Results saved in ../Result/\n";
}
