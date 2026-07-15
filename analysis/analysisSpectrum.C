void analysisSpectrum()
{
    TFile *f =
    TFile::Open("../build/SecondaryRadiation.root");

    TTree *t =
    (TTree*)f->Get("Primary");

    TCanvas *c =
    new TCanvas("c","Spectrum",900,600);

    t->Draw("Energy_keV>>h(700,0,700)");

    c->SaveAs("../Result/GeneratedSpectrum.png");
}