#include <TROOT.h>
#include <TFile.h>
#include <TTree.h>
#include <TH1D.h>
#include <TH2D.h>
#include <TCanvas.h>
#include <TStyle.h>
#include <iostream>

void analysis()
{

gStyle->SetOptStat(111111);

TFile *f = TFile::Open("../build/SecondaryRadiation.root");

if(!f){
    std::cout<<"ROOT file not found"<<std::endl;
    return;
}

TTree *t=(TTree*)f->Get("Secondary");

if(!t){
    std::cout<<"Tree not found"<<std::endl;
    return;
}

std::cout<<"Entries = "<<t->GetEntries()<<std::endl;

//////////////////////////////////////////////////////
// Primary dose in Detector A
//////////////////////////////////////////////////////

TH1D *hPrimaryDose =
new TH1D(
"hPrimaryDose",
"Primary Dose in Detector A;Deposited Energy (keV);Counts",
300,0,700);

t->Draw(
"Edep_keV>>hPrimaryDose",
"ParentID==0 && VolumeName==\"DetectorA\"",
"goff");

TH1D *hPrimaryDose =
new TH1D(
"hPrimaryDose",
"Primary Dose in Detector A;Deposited Energy (keV);Counts",
300,0,400);

t->Draw(
"Edep_keV>>hPrimaryDose",
"ParentID==0 && VolumeName==\"DetectorA\"",
"goff");



//////////////////////////////////////////////////////
// Electron spectrum
//////////////////////////////////////////////////////

TH1D *hElectron =
new TH1D("hElectron",
"Secondary Electron Spectrum;Energy (keV);Counts",
700,0,700);

hElectron->SetLineColor(kBlue+2);
hElectron->SetLineWidth(2);

t->Draw("Energy_keV>>hElectron",
"Particle==\"e-\"",
"goff");

//////////////////////////////////////////////////////
// Gamma spectrum
//////////////////////////////////////////////////////

TH1D *hGamma =
new TH1D("hGamma",
"Secondary Gamma Spectrum;Energy (keV);Counts",
300,0,300);

t->Draw("Energy_keV>>hGamma",
"Particle==\"gamma\"",
"goff");

//////////////////////////////////////////////////////
// Energy deposition
//////////////////////////////////////////////////////

TH1D *hDose =
new TH1D("hDose",
"Energy Deposition;Deposited Energy (keV);Counts",
300,0,300);

t->Draw("Edep_keV>>hDose","","goff");

//////////////////////////////////////////////////////
// Z distribution
//////////////////////////////////////////////////////

TH1D *hZ =
new TH1D("hZ",
"Interaction Depth;Z (cm);Counts",
200,0,5);

t->Draw("Z_cm>>hZ","","goff");

//////////////////////////////////////////////////////
// XY map
//////////////////////////////////////////////////////

TH2D *hXY =
new TH2D("hXY",
"Interaction Position;X (cm);Y (cm)",
200,-5,5,
200,-5,5);

t->Draw("Y_cm:X_cm>>hXY","","goff");

//////////////////////////////////////////////////////
// Track length
//////////////////////////////////////////////////////

TH1D *hTrack =
new TH1D("hTrack",
"Track Length;Length (mm);Counts",
200,0,20);

t->Draw("TrackLength_mm>>hTrack","","goff");

//////////////////////////////////////////////////////
// Save plots
//////////////////////////////////////////////////////

TCanvas *c1=new TCanvas();
hElectron->Draw();
c1->SaveAs("../Result/ElectronSpectrum.png");

TCanvas *c2=new TCanvas();
hGamma->Draw();
c2->SaveAs("../Result/GammaSpectrum.png");

TCanvas *c3=new TCanvas();
hDose->Draw();
c3->SaveAs("../Result/EnergyDeposition.png");

TCanvas *c4=new TCanvas();
hZ->Draw();
c4->SaveAs("../Result/ZDistribution.png");

TCanvas *c5=new TCanvas();
hXY->Draw("COLZ");
c5->SaveAs("../Result/XYDistribution.png");

TCanvas *c6=new TCanvas();
hTrack->Draw();
c6->SaveAs("../Result/TrackLength.png");

//////////////////////////////////////////////////////
// Statistics
//////////////////////////////////////////////////////

std::cout<<std::endl;

std::cout<<"========== SUMMARY =========="<<std::endl;

std::cout<<"Total entries : "<<t->GetEntries()<<std::endl;

Long64_t NPrimary =
t->GetEntries("ParentID==0 && VolumeName==\"DetectorA\"");

std::cout
<< "Primary interactions in Detector A : "
<< NPrimary
<< std::endl;

std::cout
<< "Mean primary deposited energy = "
<< hPrimaryDose->GetMean()
<< " keV"
<< std::endl;

std::cout<<"Electrons : "
<<t->GetEntries("Particle==\"e-\"")
<<std::endl;

std::cout<<"Gammas : "
<<t->GetEntries("Particle==\"gamma\"")
<<std::endl;

std::cout<<"Mean electron energy = "
<<hElectron->GetMean()
<<" keV"<<std::endl;

std::cout<<"Maximum electron energy = "
<<hElectron->GetBinCenter(hElectron->GetMaximumBin())
<<" keV"<<std::endl;

std::cout<<"Mean deposited energy = "
<<hDose->GetMean()
<<" keV"<<std::endl;

std::cout<<"Mean track length = "
<<hTrack->GetMean()
<<" mm"<<std::endl;


std::ofstream out("../Result/SpectrumSummary.txt");

out<<"========== Ir192 Simulation ==========\n";

out<<"Entries : "
<<t->GetEntries()
<<std::endl;

out<<"Electrons : "
<<Nelectron
<<std::endl;

out<<"Gammas : "
<<Ngamma
<<std::endl;

out<<"Mean electron energy = "
<<hElectron->GetMean()
<<std::endl;

out<<"Mean deposited energy = "
<<hDose->GetMean()
<<std::endl;


out<<"Electron Yield = "
<<100.*Nelectron/Nprimary
<<" %"<<std::endl;

out<<"Gamma Yield = "
<<100.*Ngamma/Nprimary
<<" %"<<std::endl;

out.close();

}