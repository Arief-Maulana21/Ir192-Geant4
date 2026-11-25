// ROOT 6 macro. Run from analysis/ or supply explicit paths.

#include <TFile.h>

#include <TTree.h>
#include <TLeafC.h>
#include <TLeaf.h>
#include <cstring>

#include <TH1D.h>

#include <TH2D.h>

#include <TCanvas.h>

#include <TLegend.h>

#include <TStyle.h>

#include <TSystem.h>

#include <TString.h>

#include <TROOT.h>

#include <string>

#include <array>

#include <vector>

#include <fstream>

#include <iostream>

#include <iomanip>

#include <cmath>

#include <algorithm>

#include <limits>

#include <memory>

#include <stdexcept>

namespace IrAnalysis {

    const int O=6,C=17;

    const char* organs[O]= {
        "Tongue","OralCavity","Mandible","SpinalCord","SalivaryLeft","SalivaryRight"
    };

    const char* cats[C]= {
        "Total","PbHistory","NoPbHistory","PrimaryGammaUnscattered","PrimaryGammaScattered","SecondaryGamma","SecondaryElectron","SecondaryPositron","OtherParticle","PrimaryBirth","PbGenerated","CapsuleGenerated","ApplicatorGenerated","SourceGenerated","TissueGenerated","PhantomGenerated","OtherGenerated"
    };

    struct Stat {
        long double s=0,q=0;
        void add(double x) {
            s+=x;
            q+=(long double)x*x;
        }
        double err(Long64_t n) const {
            return n>1?std::sqrt(std::max(0.L,(q-s*s/n)*n/(n-1))):std::numeric_limits<double>::quiet_NaN();
        }
    };

    void require(TTree* t,const char* b) {
        if(!t->GetBranch(b)) throw std::runtime_error(std::string("Missing branch: ")+b);
    }

    template<class T> void bind(TTree* t,const char* b,T* p) {
        require(t,b);
        if(t->SetBranchAddress(b,p)<0)throw std::runtime_error(std::string("Branch type mismatch: ")+b);
    }

    void save(TH1* h,const TString& path,const char* opt="HIST") {
        TCanvas c("plot","plot",1000,650);
        c.SetLeftMargin(.14);
        c.SetBottomMargin(.16);
        h->Draw(opt);
        c.SaveAs(path);
    }

}

void analysis(const char* input="../build/SecondaryRadiation.root",const char* output="../Result",const char* masses="organ_masses.csv",
double activity_GBq=370.0, double photonYield=2.084663,
double dwellTime_s=100.0) {

    using namespace IrAnalysis;

    try {

        std::unique_ptr<TFile> f(TFile::Open(input,"READ"));
        if(!f||f->IsZombie())throw std::runtime_error("Cannot open ROOT input");

        auto* t=dynamic_cast<TTree*>(f->Get("OrganEventDose"));
        auto* p=dynamic_cast<TTree*>(f->Get("Primary"));

        if(!t||!p)throw std::runtime_error("Need OrganEventDose and Primary from multi-organ run");

        const Long64_t n=p->GetEntries();
        if(n<2||t->GetEntries()!=O*n)throw std::runtime_error("Unexpected event/organ row count");

        if (!std::isfinite(activity_GBq) || activity_GBq <= 0 ||
        !std::isfinite(photonYield) || photonYield <= 0 ||
        !std::isfinite(dwellTime_s) || dwellTime_s <= 0)
        throw std::runtime_error("Normalization parameters must be positive and finite");

        const double photonRate = activity_GBq * 1e9 * photonYield;

        const double rateFactor = photonRate / n;

        const double normFactor = rateFactor * dwellTime_s;

        if (!std::isfinite(normFactor) || normFactor <= 0)
        throw std::runtime_error("Invalid normalization factor");

        // Masses must be copied from the SAME run summary, in grams, in ID order.

        std::ifstream mf(masses);
        std::array<double,O> mass {
        };
        std::string line;

        for(int i=0;i<O;++i) {
            if(!std::getline(mf,line))throw std::runtime_error("Missing mass row");
            auto pos=line.find(',');
            if(pos==std::string::npos||line.substr(0,pos)!=organs[i])throw std::runtime_error("Mass file must contain organ name,mass_g in ID order, no header");
            mass[i]=std::stod(line.substr(pos+1));
            if(!std::isfinite(mass[i])||mass[i]<=0)throw std::runtime_error("Invalid mass");
        }

        gSystem->mkdir(output,true);
        TString out(output);
        gStyle->SetOptStat(0);
        TH1::AddDirectory(false);

        Int_t event=0,id=0;
        double e[C]= {
        };
        bind(t,"EventID",&event);
        bind(t,"OrganID",&id);
        std::array<TLeaf*,C> energyLeaves{};
        for(int j=0;j<C;++j) {
            const std::string name=std::string(cats[j])+"_keV";
            energyLeaves[j]=t->GetLeaf(name.c_str());
            if(!energyLeaves[j]) throw std::runtime_error("Missing leaf: "+name);
        }
        Long64_t positiveRows=0;
        std::array<Long64_t,O> positiveByOrgan{};

        std::array<std::array<Stat,C>,O> a {
        };
        std::vector<unsigned char> seen(n,0);
        Long64_t bad=0;
        double maxResidual=0;

        for(Long64_t r=0;r<t->GetEntries();++r) {
            if(t->GetEntry(r)<=0)throw std::runtime_error("Cannot read organ row");
            if(event<0||event>=n||id<0||id>=O)throw std::runtime_error("Invalid event or organ ID (single run required)");
            unsigned char bit=1u<<id;
            if(seen[event]&bit)throw std::runtime_error("Duplicate event-organ pair");
            seen[event]|=bit;

            for(int j=0;j<C;++j) {
                e[j]=energyLeaves[j]->GetValue();
                if(!std::isfinite(e[j])||e[j]<0)throw std::runtime_error("Invalid deposited energy");
                a[id][j].add(e[j]);
            }

            if(e[0]>0) { ++positiveRows; ++positiveByOrgan[id]; }

            double residuals[3]= {
                e[0]-e[1]-e[2],e[0],e[0]
            };
            for(int j=3;j<=8;++j)residuals[1]-=e[j];
            for(int j=9;j<C;++j)residuals[2]-=e[j];
            for(double x:residuals) {
                maxResidual=std::max(maxResidual,std::abs(x));
                if(std::abs(x)>1e-8*std::max(1.,e[0]))++bad;
            }
        }

        if(bad)throw std::runtime_error("Energy partition validation failed");
        t->ResetBranchAddresses();
        const Long64_t expectedPositive=t->GetEntries("Total_keV>0");
        if(expectedPositive<0 || expectedPositive!=positiveRows)
            throw std::runtime_error("Positive energy row count mismatch");
        if(positiveRows==0) throw std::runtime_error("All organ energy deposits are zero");
        std::cout<<"Positive organ rows: "<<positiveRows<<" / "<<O*n<<"\n";
        for(int i=0;i<O;++i) {
            std::cout<<organs[i]<<": Edep = "<<a[i][0].s
                     <<" keV; positive events = "<<positiveByOrgan[i]<<"\n";
        }

        t->ResetBranchAddresses();

        std::ofstream csv((out+"/OrganDose.csv").Data());
        csv<<std::setprecision(12)<<"Organ,Category,Events,Mass_g,Edep_keV,Dose_Gy,Sigma_Gy,DosePerPrimary_Gy,SigmaPerPrimary_Gy,RelativeUncertainty_pct,Fraction_pct,Activity_GBq,PhotonYield,DwellTime_s,NormalizationFactor,DoseRate_Gy_s,DoseRate_Gy_min,NormalizedDose_Gy,NormalizedSigma_Gy\n";

        TH1D normalized("normalized","Normalized mean organ dose;Organ;Dose (Gy)",O,0,O);

        TH1D rate("rate","Mean organ dose rate;Organ;Dose rate (Gy/min)",O,0,O);

        TH1D total("total","Mean absorbed dose per primary;Organ;Dose (pGy/primary)",O,0,O);
        TH1D pb("pb","Pb history fraction;Organ;Fraction (%)",O,0,O);

        TH2D part("part","Depositing particle contribution (%);Organ;Particle category",O,0,O,6,0,6);
        TH2D birth("birth","Immediate birth volume contribution (%);Organ;Birth category",O,0,O,8,0,8);

        for(int i=0;i<O;++i) {
            double k=1.602176634e-13/mass[i];
            // keV to J / grams to kg

            normalized.GetXaxis()->SetBinLabel(i+1,organs[i]);

            rate.GetXaxis()->SetBinLabel(i+1,organs[i]);

            total.GetXaxis()->SetBinLabel(i+1,organs[i]);
            pb.GetXaxis()->SetBinLabel(i+1,organs[i]);
            part.GetXaxis()->SetBinLabel(i+1,organs[i]);
            birth.GetXaxis()->SetBinLabel(i+1,organs[i]);

            for(int j=0;j<C;++j) {
                double d=a[i][j].s*k,u=a[i][j].err(n)*k,frac=a[i][0].s>0?100*a[i][j].s/a[i][0].s:NAN;
                csv<<organs[i]<<','<<cats[j]<<','<<n<<','<<mass[i]<<','<<a[i][j].s<<','<<d<<','<<u<<','<<d/n<<','<<u/n<<','<<(d>0?100*u/d:NAN)<<','<<frac<<','<<activity_GBq<<','<<photonYield<<','<<dwellTime_s<<','<<normFactor<<','<<d*rateFactor<<','<<d*rateFactor*60<<','<<d*normFactor<<','<<u*normFactor<<'\n';
                if(j==0) {
                    normalized.SetBinContent(i+1,d*normFactor);
                    normalized.SetBinError(i+1,u*normFactor);
                    rate.SetBinContent(i+1,d*rateFactor*60);
                    rate.SetBinError(i+1,u*rateFactor*60);
                    total.SetBinContent(i+1,d/n*1e12);
                    total.SetBinError(i+1,u/n*1e12);
                }
                if(j==1)pb.SetBinContent(i+1,frac);
                if(j>=3&&j<=8)part.SetBinContent(i+1,j-2,frac);
                if(j>=9)birth.SetBinContent(i+1,j-8,frac);
            }
        }

        for(int j=3;j<=8;++j)part.GetYaxis()->SetBinLabel(j-2,cats[j]);
        for(int j=9;j<C;++j)birth.GetYaxis()->SetBinLabel(j-8,cats[j]);

        save(&normalized,out+"/OrganNormalizedDose.png","E1");

        save(&rate,out+"/OrganDoseRate.png","E1");

        save(&total,out+"/OrganDosePerPrimary.png","E1");
        save(&pb,out+"/PbHistoryFraction.png","BAR");

        {
            TCanvas c("heat","heat",1300,750);
            c.SetLeftMargin(.24);
            c.SetRightMargin(.15);
            c.SetBottomMargin(.16);
            gStyle->SetPaintTextFormat(".2f");
            part.Draw("COLZ TEXT");
            c.SaveAs(out+"/ParticleContribution.png");
            birth.Draw("COLZ TEXT");
            c.SaveAs(out+"/BirthContribution.png");
        }

        TH1D primary("primary","Primary gamma spectrum;Energy (keV);Photons",1600,0,1600);
        double en=0;
        bind(p,"Energy_keV",&en);
        bind(p,"EventID",&event);
        std::fill(seen.begin(),seen.end(),0);
        for(Long64_t r=0;r<n;++r) {
            if(p->GetEntry(r)<=0)throw std::runtime_error("Cannot read primary row");
            if(event<0||event>=n||seen[event]++)throw std::runtime_error("Primary must contain one photon per event");
            if(!std::isfinite(en)||en<=0)throw std::runtime_error("Invalid primary energy");
            primary.Fill(en);
        }
        p->ResetBranchAddresses();
        save(&primary,out+"/PrimarySpectrum.png");

        auto* s=dynamic_cast<TTree*>(f->Get("Secondary"));
        if(s) {
            TH1D g("g","Secondary gamma logged steps;Pre-step energy (keV);Steps",400,0,1600),el("el","Secondary electron logged steps;Pre-step energy (keV);Steps",400,0,1600);
            TH2D xy("xy","Logged step energy deposition;X (cm);Y (cm)",100,-10,10,100,-10,10);
            Int_t parent=0;
            double x=0,y=0,dep=0,w=0;
            auto* particle=dynamic_cast<TLeafC*>(s->GetLeaf("Particle"));
            if(!particle) throw std::runtime_error("Particle is not a ROOT C-string leaf");
            bind(s,"ParentID",&parent);
            bind(s,"Energy_keV",&en);
            bind(s,"X_cm",&x);
            bind(s,"Y_cm",&y);
            bind(s,"Edep_keV",&dep);
            bind(s,"Weight",&w);
            for(Long64_t r=0;r<s->GetEntries();++r) {
                if(s->GetEntry(r)<=0)throw std::runtime_error("Cannot read secondary row");
                if(parent>0) {
                    if(std::strcmp(particle->GetValueString(),"gamma")==0)g.Fill(en);
                    if(std::strcmp(particle->GetValueString(),"e-")==0)el.Fill(en);
                }
                if(dep>0)xy.Fill(x,y,dep*w);
            }
            s->ResetBranchAddresses();
            save(&g,out+"/SecondaryGammaStepSpectrum.png");
            save(&el,out+"/ElectronStepSpectrum.png");
            save(&xy,out+"/LoggedStepEdepXY.png","COLZ");
        }

        auto* b=dynamic_cast<TTree*>(f->Get("PbInteractions"));
        if(b) {
            TH1D h("interactions","Recorded Pb photon interactions;Process;Interactions",3,0,3);
            const char* names[]= {
                "compt","phot","Rayl"
            };
            auto* process=dynamic_cast<TLeafC*>(b->GetLeaf("Process"));
            if(!process) throw std::runtime_error("Process is not a ROOT C-string leaf");
            for(int i=0;i<3;++i)h.GetXaxis()->SetBinLabel(i+1,names[i]);
            for(Long64_t r=0;r<b->GetEntries();++r) {
                if(b->GetEntry(r)<=0)throw std::runtime_error("Cannot read Pb row");
                for(int i=0;i<3;++i)if(std::strcmp(process->GetValueString(),names[i])==0)h.Fill(i+.5);
            }
            b->ResetBranchAddresses();
            save(&h,out+"/PbInteractions.png","BAR");
        }

        std::ofstream v((out+"/Validation.txt").Data());
        v<<std::setprecision(12)<<"Activity (GBq): "<<activity_GBq<<"\nPhoton yield: "<<photonYield<<"\nDwell time (s): "<<dwellTime_s<<"\nNormalization factor: "<<normFactor<<"\nStatistical uncertainty only; single dwell; one photon per event.\nEvents: "<<n<<"\nOrgan rows: "<<O*n<<"\nMaximum partition residual (keV): "<<maxResidual<<"\nFailed partitions: "<<bad<<"\n";
        v<<"Positive organ rows: "<<positiveRows<<"\n";
        for(int i=0;i<O;++i) v<<organs[i]<<": Edep (keV) = "<<a[i][0].s<<", positive events = "<<positiveByOrgan[i]<<"\n";
        std::cout<<"Analysis complete: "<<out<<"\n";

        std::cout<<"Normalization factor: "<<normFactor<<"\n";

        for(int i=0;i<O;++i) {

            const double k=1.602176634e-13/mass[i];

            std::cout<<organs[i]<<": "<<a[i][0].s*k*normFactor
            <<" +/- "<<a[i][0].err(n)*k*normFactor<<" Gy\n";

        }

    }
    catch(const std::exception& e) {
        std::cerr<<"Analysis failed: "<<e.what()<<"\n";
    }

}
