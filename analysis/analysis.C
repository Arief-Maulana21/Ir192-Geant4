// ROOT 6 macro. Run from analysis/ or supply explicit paths.
#include <TFile.h>
#include <TTree.h>
#include <TH1D.h>
#include <TH2D.h>
#include <TCanvas.h>
#include <TLegend.h>
#include <TStyle.h>
#include <TSystem.h>
#include <TString.h>
#include <TROOT.h>
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
const char* organs[O]={"Tongue","OralCavity","Mandible","SpinalCord","SalivaryLeft","SalivaryRight"};
const char* cats[C]={"Total","PbHistory","NoPbHistory","PrimaryGammaUnscattered","PrimaryGammaScattered","SecondaryGamma","SecondaryElectron","SecondaryPositron","OtherParticle","PrimaryBirth","PbGenerated","CapsuleGenerated","ApplicatorGenerated","SourceGenerated","TissueGenerated","PhantomGenerated","OtherGenerated"};
struct Stat { long double s=0,q=0; void add(double x){s+=x;q+=(long double)x*x;} double err(Long64_t n) const {return n>1?std::sqrt(std::max(0.L,(q-s*s/n)*n/(n-1))):std::numeric_limits<double>::quiet_NaN();} };
void require(TTree* t,const char* b){if(!t->GetBranch(b)) throw std::runtime_error(std::string("Missing branch: ")+b);}
template<class T> void bind(TTree* t,const char* b,T* p){require(t,b);if(t->SetBranchAddress(b,p)<0)throw std::runtime_error(std::string("Branch type mismatch: ")+b);}
void save(TH1* h,const TString& path,const char* opt="HIST"){TCanvas c("plot","plot",1000,650);c.SetLeftMargin(.14);c.SetBottomMargin(.16);h->Draw(opt);c.SaveAs(path);}
}
void analysis(const char* input="../build/SecondaryRadiation.root",const char* output="../Result",const char* masses="organ_masses.csv"){
 using namespace IrAnalysis;
 try {
 std::unique_ptr<TFile> f(TFile::Open(input,"READ")); if(!f||f->IsZombie())throw std::runtime_error("Cannot open ROOT input");
 auto* t=dynamic_cast<TTree*>(f->Get("OrganEventDose")); auto* p=dynamic_cast<TTree*>(f->Get("Primary"));
 if(!t||!p)throw std::runtime_error("Need OrganEventDose and Primary from multi-organ run");
 const Long64_t n=p->GetEntries(); if(n<2||t->GetEntries()!=O*n)throw std::runtime_error("Unexpected event/organ row count");
 // Masses must be copied from the SAME run summary, in grams, in ID order.
 std::ifstream mf(masses); std::array<double,O> mass{}; std::string line;
 for(int i=0;i<O;++i){if(!std::getline(mf,line))throw std::runtime_error("Missing mass row");auto pos=line.find(',');if(pos==std::string::npos||line.substr(0,pos)!=organs[i])throw std::runtime_error("Mass file must contain organ name,mass_g in ID order, no header");mass[i]=std::stod(line.substr(pos+1));if(!std::isfinite(mass[i])||mass[i]<=0)throw std::runtime_error("Invalid mass");}
 gSystem->mkdir(output,true); TString out(output); gStyle->SetOptStat(0); TH1::AddDirectory(false);
 Int_t event=0,id=0; double e[C]={}; bind(t,"EventID",&event);bind(t,"OrganID",&id);for(int j=0;j<C;++j)bind(t,Form("%s_keV",cats[j]),&e[j]);
 std::array<std::array<Stat,C>,O> a{}; std::vector<unsigned char> seen(n,0); Long64_t bad=0; double maxResidual=0;
 for(Long64_t r=0;r<t->GetEntries();++r){if(t->GetEntry(r)<=0)throw std::runtime_error("Cannot read organ row");if(event<0||event>=n||id<0||id>=O)throw std::runtime_error("Invalid event or organ ID (single run required)");unsigned char bit=1u<<id;if(seen[event]&bit)throw std::runtime_error("Duplicate event-organ pair");seen[event]|=bit;
 for(int j=0;j<C;++j){if(!std::isfinite(e[j])||e[j]<0)throw std::runtime_error("Invalid deposited energy");a[id][j].add(e[j]);}
 double residuals[3]={e[0]-e[1]-e[2],e[0],e[0]};for(int j=3;j<=8;++j)residuals[1]-=e[j];for(int j=9;j<C;++j)residuals[2]-=e[j];for(double x:residuals){maxResidual=std::max(maxResidual,std::abs(x));if(std::abs(x)>1e-8*std::max(1.,e[0]))++bad;}}
 if(bad)throw std::runtime_error("Energy partition validation failed");
 t->ResetBranchAddresses();
 std::ofstream csv((out+"/OrganDose.csv").Data());csv<<std::setprecision(12)<<"Organ,Category,Events,Mass_g,Edep_keV,Dose_Gy,Sigma_Gy,DosePerPrimary_Gy,SigmaPerPrimary_Gy,RelativeUncertainty_pct,Fraction_pct\n";
 TH1D total("total","Mean absorbed dose per primary;Organ;Dose (pGy/primary)",O,0,O); TH1D pb("pb","Pb history fraction;Organ;Fraction (%)",O,0,O);
 TH2D part("part","Depositing particle contribution (%);Organ;Particle category",O,0,O,6,0,6); TH2D birth("birth","Immediate birth volume contribution (%);Organ;Birth category",O,0,O,8,0,8);
 for(int i=0;i<O;++i){double k=1.602176634e-13/mass[i]; // keV to J / grams to kg
 total.GetXaxis()->SetBinLabel(i+1,organs[i]);pb.GetXaxis()->SetBinLabel(i+1,organs[i]);part.GetXaxis()->SetBinLabel(i+1,organs[i]);birth.GetXaxis()->SetBinLabel(i+1,organs[i]);
 for(int j=0;j<C;++j){double d=a[i][j].s*k,u=a[i][j].err(n)*k,frac=a[i][0].s>0?100*a[i][j].s/a[i][0].s:NAN;csv<<organs[i]<<','<<cats[j]<<','<<n<<','<<mass[i]<<','<<a[i][j].s<<','<<d<<','<<u<<','<<d/n<<','<<u/n<<','<<(d>0?100*u/d:NAN)<<','<<frac<<'\n';if(j==0){total.SetBinContent(i+1,d/n*1e12);total.SetBinError(i+1,u/n*1e12);}if(j==1)pb.SetBinContent(i+1,frac);if(j>=3&&j<=8)part.SetBinContent(i+1,j-2,frac);if(j>=9)birth.SetBinContent(i+1,j-8,frac);}}
 for(int j=3;j<=8;++j)part.GetYaxis()->SetBinLabel(j-2,cats[j]);for(int j=9;j<C;++j)birth.GetYaxis()->SetBinLabel(j-8,cats[j]);
 save(&total,out+"/OrganDosePerPrimary.png","E1");save(&pb,out+"/PbHistoryFraction.png","BAR");
 {TCanvas c("heat","heat",1300,750);c.SetLeftMargin(.24);c.SetRightMargin(.15);c.SetBottomMargin(.16);gStyle->SetPaintTextFormat(".2f");part.Draw("COLZ TEXT");c.SaveAs(out+"/ParticleContribution.png");birth.Draw("COLZ TEXT");c.SaveAs(out+"/BirthContribution.png");}
 TH1D primary("primary","Primary gamma spectrum;Energy (keV);Photons",1600,0,1600); double en=0;bind(p,"Energy_keV",&en);bind(p,"EventID",&event);std::fill(seen.begin(),seen.end(),0);for(Long64_t r=0;r<n;++r){p->GetEntry(r);if(event<0||event>=n||seen[event]++)throw std::runtime_error("Primary must contain one photon per event");primary.Fill(en);}p->ResetBranchAddresses();save(&primary,out+"/PrimarySpectrum.png");
 auto* s=dynamic_cast<TTree*>(f->Get("Secondary"));if(s){TH1D g("g","Secondary gamma logged steps;Pre-step energy (keV);Steps",400,0,1600),el("el","Secondary electron logged steps;Pre-step energy (keV);Steps",400,0,1600);TH2D xy("xy","Logged step energy deposition;X (cm);Y (cm)",100,-10,10,100,-10,10);Int_t parent=0;double x=0,y=0,dep=0,w=0;std::string* particle=nullptr;bind(s,"ParentID",&parent);bind(s,"Particle",&particle);bind(s,"Energy_keV",&en);bind(s,"X_cm",&x);bind(s,"Y_cm",&y);bind(s,"Edep_keV",&dep);bind(s,"Weight",&w);for(Long64_t r=0;r<s->GetEntries();++r){s->GetEntry(r);if(parent>0&&particle){if(*particle=="gamma")g.Fill(en);if(*particle=="e-")el.Fill(en);}if(dep>0)xy.Fill(x,y,dep*w);}s->ResetBranchAddresses();save(&g,out+"/SecondaryGammaStepSpectrum.png");save(&el,out+"/ElectronStepSpectrum.png");save(&xy,out+"/LoggedStepEdepXY.png","COLZ");}
 auto* b=dynamic_cast<TTree*>(f->Get("PbInteractions"));if(b){TH1D h("interactions","Recorded Pb photon interactions;Process;Interactions",3,0,3);const char* names[]={"compt","phot","Rayl"};std::string* process=nullptr;bind(b,"Process",&process);for(int i=0;i<3;++i)h.GetXaxis()->SetBinLabel(i+1,names[i]);for(Long64_t r=0;r<b->GetEntries();++r){b->GetEntry(r);for(int i=0;i<3;++i)if(process&&*process==names[i])h.Fill(i+.5);}b->ResetBranchAddresses();save(&h,out+"/PbInteractions.png","BAR");}
 std::ofstream v((out+"/Validation.txt").Data());v<<"Events: "<<n<<"\nOrgan rows: "<<O*n<<"\nMaximum partition residual (keV): "<<maxResidual<<"\nFailed partitions: "<<bad<<"\n";std::cout<<"Analysis complete: "<<out<<"\n";
 }catch(const std::exception& e){std::cerr<<"Analysis failed: "<<e.what()<<"\n";}
}
