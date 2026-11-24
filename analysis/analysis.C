
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
        for(int j=0;j<C;++j)bind(t,Form("%s_keV",cats[j]),&e[j]);

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
                if(!std::isfinite(e[j])||e[j]<0)throw std::runtime_error("Invalid deposited energy");
                a[id][j].add(e[j]);
            }

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