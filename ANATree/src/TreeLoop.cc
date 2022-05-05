#include "TreeLoop.h"


//construtor
TreeLoop::TreeLoop(TString inputfile, TString outputfile){

  if(strstr(inputfile,"MuonEG") || strstr(inputfile,"DoubleMuon") || strstr(inputfile,"SingleMuon") || strstr(inputfile,"SingleElectron") || strstr(inputfile,"MuonEG")){
    isData = true;
  }else{
    isMC = true;
  }
  //df = new ROOT:
  oldfile = new TFile(inputfile);
  if(verbose){
    if(oldfile){
      cout<<"[INFO] open oldfile in "<<inputfile<<endl;
    } else{
      cout<<"[ERROR] can not open file"<<endl;
    }
  }
  //TTreeReader myreader("Ana/passedEvents",oldfile);
  myreader = new TTreeReader("Ana/passedEvents",oldfile);
  TH1F *temph = (TH1F*)oldfile->Get("Ana/sumWeights");
  SumWeight = temph->GetBinContent(1);
  //if(verbose){
  //  if(myreader->Next()){
  //    cout<<"[INFO] setup tree reader "<<endl;
  //  } else{
  //    cout<<"[ERROR] can not setup tree reader"<<endl;
  //  }
  //}

  outfile = new TFile(outputfile,"recreate");
  passedEventsTree_All = new TTree("passedEvents","passedEvents");

  if(verbose){cout<<"[INFO] construtor done"<<endl;}


}

//destructor
TreeLoop::~TreeLoop(){}

//==============================================================================================================//
void TreeLoop::Loop(){
  if(verbose){cout<<"[INFO] start to event loop"<<endl;}

  if(doMela){
    if(verbose){cout<<"[INFO] setup MELA"<<endl;}
    SetMEsFile();
  }else{
    if(verbose){cout<<"[INFO] skip MELA"<<endl;}
  }

  //===================start event loop==========================================
  setTree();
  if(verbose){cout<<"[INFO]============loop tree reader========================"<<endl;}
  int nevents_negative = 0;
  int nevents_nassociatedjets_lesstwo = 0;
  while (myreader->Next()) {

    if(!(*(*passedTrig))){
      if(verbose){cout<<"[INFO] failed pass trigger,skip to next tree loop"<<endl;}
      continue;
    }else{
      if(verbose){cout<<"[INFO] pass trigger"<<endl;}
      passed_trig++;
    }

    if(verbose){
      if(lep_pt->GetSize()>0){
        if(verbose) cout<<"[INFO] this lep pt = "<<(*lep_pt)[0]<<endl;
      }

    }

    initialize(); // initialize all menber datas

    Met = *(*met);
    Met_phi = *(*met_phi);

    if(verbose){cout<<"[INFO] try to find higgs candidate"<<endl;}
    findZ1LCandidate();
    findZ2JCandidata();
    findZ2MergedCandidata();
    if(verbose){cout<<"[INFO] find higgs candidate done"<<endl;}
    /*
    if((!foundZ1LCandidate) || (!foundZ2JCandidate)){
      if(verbose){cout<<"[INFO] no higgs candidate in this event, skip to next event loop"<<endl;}
      continue;
    } //found Higg 2L2Q candidate
    if(verbose){cout<<"[INFO] higgs candidate is found in this event"<<endl;}
    */

    if(isMC){
      if(verbose) cout<<"[INFO] start GEN analysis"<<endl;
      SetVBFGen();
      if(verbose) cout<<"[INFO] GEN analysis done"<<endl;
    }

    //at least have a leptoinc Z in evnets
    if(!found2lepCandidate) continue;
    run = *(*Run);
    event = *(*Event);
    lumiSect = *(*LumiSect);

    //fill event weight
    EventWeight = *(*eventWeight)*(*lep_dataMC)[lep_Z1index[0]]*(*lep_dataMC)[lep_Z1index[1]];
    GenWeight = *(*genWeight);
    PileupWeight = *(*pileupWeight);
    PrefiringWeight = *(*prefiringWeight);

    if(abs((*lep_id)[lep_Z1index[0]])==13 && abs((*lep_id)[lep_Z1index[1]])==13){ isMuMu = true;}
    if(abs((*lep_id)[lep_Z1index[0]])==11 && abs((*lep_id)[lep_Z1index[1]])==11){ isEE = true;}

    if(foundZ2MergedCandidata && isMC){
      int nbhadron = (*mergedjet_nbHadrons)[merged_Z1index];
      int nchadron = (*mergedjet_ncHadrons)[merged_Z1index];

      if(nbhadron>=1){ isbjet = true; }
      else if(nchadron>=1 && nbhadron==0){ iscjet = true; }
      else if(nbhadron==0 && nchadron==0) { islightjet = true;}

    }

    //find resolved higgs
    if(foundZ1LCandidate && foundZ2JCandidate){
      TLorentzVector p4_ZZ, Lep1, Lep2, Jet1,Jet2;
      Lep1.SetPtEtaPhiM((*lepFSR_pt)[lep_Z1index[0]],(*lepFSR_eta)[lep_Z1index[0]],(*lep_phi)[lep_Z1index[0]],(*lepFSR_mass)[lep_Z1index[0]]);
      Lep2.SetPtEtaPhiM((*lepFSR_pt)[lep_Z1index[1]],(*lepFSR_eta)[lep_Z1index[1]],(*lep_phi)[lep_Z1index[1]],(*lepFSR_mass)[lep_Z1index[1]]);
      Jet1.SetPtEtaPhiM((*jet_pt)[jet_Z1index[0]],(*jet_eta)[jet_Z1index[0]],(*jet_phi)[jet_Z1index[0]],(*jet_mass)[jet_Z1index[0]]);
      Jet2.SetPtEtaPhiM((*jet_pt)[jet_Z1index[1]],(*jet_eta)[jet_Z1index[1]],(*jet_phi)[jet_Z1index[1]],(*jet_mass)[jet_Z1index[1]]);
      p4_ZZ = Lep1+Lep2+Jet1+Jet2;
      mass2l2jet = p4_ZZ.M();
    }

    //find merged higgs
    if(foundZ1LCandidate && foundZ2MergedCandidata){
      nsubjet = (*mergedjet_nsubjet)[merged_Z1index];

      TLorentzVector p4_ZZ,Lep1,Lep2,Jet1,Jet2,Mergedjet;
      Lep1.SetPtEtaPhiM((*lepFSR_pt)[lep_Z1index[0]],(*lepFSR_eta)[lep_Z1index[0]],(*lep_phi)[lep_Z1index[0]],(*lepFSR_mass)[lep_Z1index[0]]);
      Lep2.SetPtEtaPhiM((*lepFSR_pt)[lep_Z1index[1]],(*lepFSR_eta)[lep_Z1index[1]],(*lep_phi)[lep_Z1index[1]],(*lepFSR_mass)[lep_Z1index[1]]);
      Jet1.SetPtEtaPhiM((*mergedjet_subjet_pt)[merged_Z1index][0],(*mergedjet_subjet_eta)[merged_Z1index][0],(*mergedjet_subjet_phi)[merged_Z1index][0],(*mergedjet_subjet_mass)[merged_Z1index][0]);
      Jet1.SetPtEtaPhiM((*mergedjet_subjet_pt)[merged_Z1index][1],(*mergedjet_subjet_eta)[merged_Z1index][1],(*mergedjet_subjet_phi)[merged_Z1index][1],(*mergedjet_subjet_mass)[merged_Z1index][1]);
      Mergedjet.SetPtEtaPhiM((*mergedjet_pt)[merged_Z1index],(*mergedjet_eta)[merged_Z1index],(*mergedjet_phi)[merged_Z1index],(*mergedjet_subjet_softDropMass)[merged_Z1index]);

      p4_ZZ = Lep1+Lep2+Mergedjet;
      mass2lj = p4_ZZ.M();
    }

    //find resovled only higgs
    if(foundZ1LCandidate && foundZ2JCandidate && !foundZ2MergedCandidata){
      passed_resovedonlyHiggs++;
      foundresolvedOnly = true;
    }
    //find merged only higgs
    if(foundZ1LCandidate && foundZ2MergedCandidata && !foundZ2JCandidate){
      passed_MergedolonlyHiggs++;
      foundmergedOnly = true;
    }
    //find combination higgs
    if(foundZ1LCandidate && foundZ2MergedCandidata && foundZ2JCandidate){
      passed_combinHiggs++;
      if(pt2l>200 && ptmerged>300){
        passed_combineMerged ++;
        foundmergedCombine = true;
      } else{
        passed_combineResolved ++;
        foundresolvedCombine = true;
      }

    }

    //full resovled higgs and do MALE

    if(foundresolvedOnly || foundresolvedCombine){
      if(verbose) cout<<"[INFO] this is resovled higgs"<<endl;
      passed_fullresolved++;
      passedfullresolved = true;
      //cout<<"passedfullresolved"<<endl;


      if (doMela){
        //cout<<"fullresolved Mela"<<endl;
        //check number of associated jets
        int njets = jet_pt->GetSize();
        int nassociatedjets = 0;
        vector<TLorentzVector> associatedjets;
        for(int i=0; i<njets; i++){

          //find jet that are not signal and put it into associated
          if(i==jet_Z1index[0] || i==jet_Z1index[1]){continue;}

          TLorentzVector thisjet;
          thisjet.SetPtEtaPhiM((*jet_pt)[i],(*jet_eta)[i],(*jet_phi)[i],(*jet_mass)[i]);

          /*
          bool dropit = false;
          for(int i=0; i<2; i++){
            double temp_DR = deltaR(thisjet.Eta(),thisjet.Phi(),(*lepFSR_eta)[lep_Z1index[i]],(*lepFSR_phi)[lep_Z1index[i]]);
            DR_selectedleps_asccoiacted.push_back(temp_DR);
            if(temp_DR<0.4){
              dropit = true;
            }
          }
          if(dropit) continue;
          */

          nassociatedjets++;
          associatedjets.push_back(thisjet);
          associatedjets_index.push_back(i);
        }
        if(nassociatedjets>=2){ //compute KD_jjVBF
          //cout<<" nassociatedjets>=2 "<<endl;

          if(verbose) cout<<"[INFO] found at least two nassociatedjets"<<endl;
          passedNassociated = true;
          //MEs
          //signal like objets
          TLorentzVector selectedLep1,selectedLep2,selectedJet1,selectedJet2;
          selectedLep1.SetPtEtaPhiM((*lepFSR_pt)[lep_Z1index[0]],(*lepFSR_eta)[lep_Z1index[0]],(*lep_phi)[lep_Z1index[0]],(*lepFSR_mass)[lep_Z1index[0]]);
          selectedLep2.SetPtEtaPhiM((*lepFSR_pt)[lep_Z1index[1]],(*lepFSR_eta)[lep_Z1index[1]],(*lep_phi)[lep_Z1index[1]],(*lepFSR_mass)[lep_Z1index[1]]);
          selectedJet1.SetPtEtaPhiM((*jet_pt)[jet_Z1index[0]],(*jet_eta)[jet_Z1index[0]],(*jet_phi)[jet_Z1index[0]],(*jet_mass)[jet_Z1index[0]]);
          selectedJet2.SetPtEtaPhiM((*jet_pt)[jet_Z1index[1]],(*jet_eta)[jet_Z1index[1]],(*jet_phi)[jet_Z1index[1]],(*jet_mass)[jet_Z1index[1]]);

          SimpleParticleCollection_t daughters;
          daughters.push_back(SimpleParticle_t((*lep_id)[lep_Z1index[0]], selectedLep1));
          daughters.push_back(SimpleParticle_t((*lep_id)[lep_Z1index[1]], selectedLep2));
          daughters.push_back(SimpleParticle_t(0, selectedJet1));
          daughters.push_back(SimpleParticle_t(0, selectedJet2));

          //cout<<" fill daughters done "<<endl;

          //associated objets
          SimpleParticleCollection_t associated;
          //sort associatedjets as pt order
          int leading_index[2]={0,0};
          double temp_leadingpt=0.0;
          for(int i=0; i<nassociatedjets; i++){
            if(associatedjets[i].Pt()>temp_leadingpt){
              temp_leadingpt = associatedjets[i].Pt();
              leading_index[0]= i;
            }
          }
          if(verbose) cout<<" sort leading done "<<endl;
          double temp_subleadingpt=0.0;
          for(int i=0; i<nassociatedjets; i++){
            if(i==leading_index[0]) continue; //do not count leading jet

            if(associatedjets[i].Pt()>temp_subleadingpt){
              temp_subleadingpt = associatedjets[i].Pt();
              leading_index[1]= i;
            }
          }
          if(verbose) cout<<" sort done "<<endl;
          //sort done
          for(int i=0; i<2; i++){ //put two leading jets into associated
            associated.push_back(SimpleParticle_t(0, associatedjets[leading_index[i]]));
            associatedjet_pt.push_back(associatedjets[leading_index[i]].Pt());
            associatedjet_eta.push_back(associatedjets[leading_index[i]].Eta());
            associatedjet_phi.push_back(associatedjets[leading_index[i]].Phi());
            associatedjet_mass.push_back(associatedjets[leading_index[i]].M());
          }

          if(verbose) cout<<"[INFO] associatedjet set done"<<endl;

          time_associatedjet_eta = associatedjets[leading_index[0]].Eta()*associatedjets[leading_index[1]].Eta();

          //match to truth
          if(isMC){
            if((associatedjets_index[leading_index[0]] == match1_recoindex || associatedjets_index[leading_index[0]] == match2_recoindex) && (associatedjets_index[leading_index[1]] == match1_recoindex || associatedjets_index[leading_index[1]] == match2_recoindex)){
              passedmatchtruthVBF=true;
            }
          }

          //if(verbose) cout<<"[INFO] associatedjet set done"<<endl;

          //cout<<" fill VBF jet done "<<endl;

          IvyMELAHelpers::melaHandle->setCandidateDecayMode(TVar::CandidateDecay_ZZ);
          IvyMELAHelpers::melaHandle->setInputEvent(&daughters, &associated, nullptr, false);
          MEblock.computeMELABranches();
          MEblock.pushMELABranches();

          IvyMELAHelpers::melaHandle->resetInputEvent();

          //KD
          //retrieve MEs for KD constructing
          unordered_map<string,float> ME_Kfactor_values;
          MEblock.getBranchValues(ME_Kfactor_values);
          //unordered_map<string, float>::iterator iter;
          //for(iter = ME_Kfactor_values.begin(); iter != ME_Kfactor_values.end(); iter++){
          //  cout << "[INFO] this ME_Kfactor_values = "<<iter->first << " : " << iter->second << endl;
          //}
          //VBF
          vector<DiscriminantClasses::Type> KDtypes{DiscriminantClasses::kDjjVBF};
          unsigned int const nKDs = KDtypes.size();
          vector<DiscriminantClasses::KDspecs> KDlist;
          KDlist.reserve(nKDs);
          for (auto const& KDtype:KDtypes) KDlist.emplace_back(KDtype);
          DiscriminantClasses::constructDiscriminants(KDlist, 169*121, "JJVBFTagged");
          //Update discriminants
          for (auto& KDspec:KDlist){

            std::vector<float> KDvars; KDvars.reserve(KDspec.KDvars.size());
            for (auto const& strKDvar:KDspec.KDvars){
              //cout<<typeid(strKDvar).name()<<endl;
              //cout<<"this strKDva = "<<strKDvar<<endl;
              //cout<<"this ME_Kfactor_values = "<<ME_Kfactor_values[static_cast<string>(strKDvar)]<<endl;
              KDvars.push_back(ME_Kfactor_values[static_cast<string>(strKDvar)]);
            } // ME_Kfactor_values here is just a map of TString->float. You should have something equivalent.
            //float temp_KD = KDspec.KD->update(KDvars, mass2l2jet);
            //cout<<"KDvars[0] = "<<KDvars[0]<<endl;
            //cout<<"type of mass2l2jet is "<<typeid(mass2l2jet).name()<<"  value of mass2jet  = "<<mass2l2jet<<endl;
            //cout<<"type of return value of KD function is :"<<typeid(KDspec.KD->update(KDvars, mass2l2jet)).name()<<"  "<<"value of KD function = "<<KDspec.KD->update(KDvars, mass2l2jet)<<endl;
            KDspec.KD->update(KDvars, mass2l2jet); // Use mZZ!
            //cout<<"value of KD for this evnet  = "<<*(KDspec.KD)<<endl;
            KD_jjVBF = *(KDspec.KD);
            if(nassociatedjets<2){
              nevents_nassociatedjets_lesstwo++;
              //cout<<"[WARNNING] There is less than 2 associated jets in this event. There number of associated jets in this events = "<<nassociatedjets<<endl;
              //cout<<"In this event, KD = "<<KD_jjVBF<<endl;
            }
            if(KD_jjVBF<0){
              nevents_negative++;
            }

          }
          for (auto& KDspec:KDlist) KDspec.resetKD();

        }
      }

    }

    //full merged higgs and do MALE
    if(foundmergedOnly || foundmergedCombine ){
      if(verbose) cout<<"[INFO] this is merged higgs"<<endl;
      passed_fullmerged++;
      passedfullmerged = true;

      //DR
      if(isMC){
        if(passedRecoquarkMatch){
          double this1_DR = 0.0;
          this1_DR = deltaR((*mergedjet_eta)[merged_Z1index],(*mergedjet_phi)[merged_Z1index],Reco_quark1_match_eta,Reco_quark1_match_phi);
          DR_merged_VBF1_matched = this1_DR;

          double this2_DR = 0.0;
          this2_DR = deltaR((*mergedjet_eta)[merged_Z1index],(*mergedjet_phi)[merged_Z1index],Reco_quark2_match_eta,Reco_quark2_match_phi);
          DR_merged_VBF2_matched = this2_DR;
        }
      }

      if (doMela){
        int njets = jet_pt->GetSize();
        int nassociatedjets = 0;
        vector<TLorentzVector> associatedjets;
        for(int i=0; i<njets; i++){

          if(foundZ2JCandidate){
            if(i==jet_Z1index[0] || i==jet_Z1index[1]){continue;}
          }

          TLorentzVector thisjet;

          thisjet.SetPtEtaPhiM((*jet_pt)[i],(*jet_eta)[i],(*jet_phi)[i],(*jet_mass)[i]);
          //check this deltaR between this jet and selected merged jet
          double temp_DR = deltaR(thisjet.Eta(),thisjet.Phi(),(*mergedjet_eta)[merged_Z1index],(*mergedjet_phi)[merged_Z1index]);
          DR_merged_asccoiacted.push_back(temp_DR);
          if(temp_DR<0.8) {
            continue;
          }

          /*
          bool dropit = false;
          for(int i=0; i<2; i++){
            double temp_DR = deltaR(thisjet.Eta(),thisjet.Phi(),(*lepFSR_eta)[lep_Z1index[i]],(*lepFSR_phi)[lep_Z1index[i]]);
            DR_selectedleps_asccoiacted.push_back(temp_DR);
            if(temp_DR<0.4){
              dropit = true;
            }
          }
          if(dropit) continue;
          */

          nassociatedjets++;
          associatedjets.push_back(thisjet);
          associatedjets_index.push_back(i);
        }

        if(nassociatedjets>=2){ //compute KD_jjVBF

          passedNassociated = true;
          //MEs
          //signal like objets
          TLorentzVector selectedLep1,selectedLep2,selectedJet1,selectedJet2;
          selectedLep1.SetPtEtaPhiM((*lepFSR_pt)[lep_Z1index[0]],(*lepFSR_eta)[lep_Z1index[0]],(*lep_phi)[lep_Z1index[0]],(*lepFSR_mass)[lep_Z1index[0]]);
          selectedLep2.SetPtEtaPhiM((*lepFSR_pt)[lep_Z1index[1]],(*lepFSR_eta)[lep_Z1index[1]],(*lep_phi)[lep_Z1index[1]],(*lepFSR_mass)[lep_Z1index[1]]);
          selectedJet1.SetPtEtaPhiM((*mergedjet_subjet_pt)[merged_Z1index][0],(*mergedjet_subjet_eta)[merged_Z1index][0],(*mergedjet_subjet_phi)[merged_Z1index][0],(*mergedjet_subjet_mass)[merged_Z1index][0]);
          selectedJet1.SetPtEtaPhiM((*mergedjet_subjet_pt)[merged_Z1index][1],(*mergedjet_subjet_eta)[merged_Z1index][1],(*mergedjet_subjet_phi)[merged_Z1index][1],(*mergedjet_subjet_mass)[merged_Z1index][1]);


          SimpleParticleCollection_t daughters;
          daughters.push_back(SimpleParticle_t((*lep_id)[lep_Z1index[0]], selectedLep1));
          daughters.push_back(SimpleParticle_t((*lep_id)[lep_Z1index[1]], selectedLep2));
          daughters.push_back(SimpleParticle_t(0, selectedJet1));
          daughters.push_back(SimpleParticle_t(0, selectedJet2));

          //associated objets
          SimpleParticleCollection_t associated;
          //sort associatedjets as pt order
          int leading_index[2]={0,0};
          double temp_leadingpt=0.0;
          for(int i=0; i<nassociatedjets; i++){
            if(associatedjets[i].Pt()>temp_leadingpt){
              temp_leadingpt = associatedjets[i].Pt();
              leading_index[0]= i;
            }
          }
          double temp_subleadingpt=0.0;
          for(int i=0; i<nassociatedjets; i++){
            if(i==leading_index[0]) continue; //do not count leading jet

            if(associatedjets[i].Pt()>temp_subleadingpt){
              temp_subleadingpt = associatedjets[i].Pt();
              leading_index[1]= i;
            }
          }
          //sort done

          for(int i=0; i<2; i++){ //put two leading jets into associated
            associated.push_back(SimpleParticle_t(0, associatedjets[leading_index[i]]));
            associatedjet_pt.push_back(associatedjets[leading_index[i]].Pt());
            associatedjet_eta.push_back(associatedjets[leading_index[i]].Eta());
            associatedjet_phi.push_back(associatedjets[leading_index[i]].Phi());
            associatedjet_mass.push_back(associatedjets[leading_index[i]].M());
          }
          time_associatedjet_eta = associatedjets[leading_index[0]].Eta()*associatedjets[leading_index[1]].Eta();

          //match to truth
          if(isMC){
            if((associatedjets_index[leading_index[0]] == match1_recoindex || associatedjets_index[leading_index[0]] == match2_recoindex) && (associatedjets_index[leading_index[1]] == match1_recoindex || associatedjets_index[leading_index[1]] == match2_recoindex)){
              passedmatchtruthVBF=true;
            }
          }


          /*

          TLorentzVector VBF_jet1,VBF_jet2;
          VBF_jet1.SetPtEtaPhiM(Reco_quark1_match_pt,Reco_quark1_match_eta,Reco_quark1_match_phi,Reco_quark1_match_mass);
          VBF_jet2.SetPtEtaPhiM(Reco_quark2_match_pt,Reco_quark2_match_eta,Reco_quark2_match_phi,Reco_quark2_match_mass);
          associated.push_back(SimpleParticle_t(0, VBF_jet1));
          associated.push_back(SimpleParticle_t(0, VBF_jet2));
          */



          IvyMELAHelpers::melaHandle->setCandidateDecayMode(TVar::CandidateDecay_ZZ);
          IvyMELAHelpers::melaHandle->setInputEvent(&daughters, &associated, nullptr, false);
          MEblock.computeMELABranches();
          MEblock.pushMELABranches();
          IvyMELAHelpers::melaHandle->resetInputEvent();

          //KD
          //retrieve MEs for KD constructing
          unordered_map<string,float> ME_Kfactor_values;
          MEblock.getBranchValues(ME_Kfactor_values);
          //unordered_map<string, float>::iterator iter;
          //for(iter = ME_Kfactor_values.begin(); iter != ME_Kfactor_values.end(); iter++){
          //  cout << "[INFO] this ME_Kfactor_values = "<<iter->first << " : " << iter->second << endl;
          //}
          //VBF
          vector<DiscriminantClasses::Type> KDtypes{DiscriminantClasses::kDjjVBF};
          unsigned int const nKDs = KDtypes.size();
          vector<DiscriminantClasses::KDspecs> KDlist;
          KDlist.reserve(nKDs);
          for (auto const& KDtype:KDtypes) KDlist.emplace_back(KDtype);
          DiscriminantClasses::constructDiscriminants(KDlist, 169*121, "JJVBFTagged");
          //Update discriminants
          for (auto& KDspec:KDlist){

            std::vector<float> KDvars; KDvars.reserve(KDspec.KDvars.size());
            for (auto const& strKDvar:KDspec.KDvars){
              //cout<<typeid(strKDvar).name()<<endl;
              //cout<<"this strKDva = "<<strKDvar<<endl;
              //cout<<"this ME_Kfactor_values = "<<ME_Kfactor_values[static_cast<string>(strKDvar)]<<endl;
              KDvars.push_back(ME_Kfactor_values[static_cast<string>(strKDvar)]);
            } // ME_Kfactor_values here is just a map of TString->float. You should have something equivalent.

            KDspec.KD->update(KDvars, mass2lj); // Use mZZ!
            //cout<<"value of KD for this evnet  = "<<*(KDspec.KD)<<endl;
            KD_jjVBF = *(KDspec.KD);
            if(nassociatedjets<2){
              nevents_nassociatedjets_lesstwo++;
            }
            if(KD_jjVBF<0){
              nevents_negative++;
            }

          }
          for (auto& KDspec:KDlist) KDspec.resetKD();

        }

      }

    }

    //SetVBFGen();//set Gen and match atfer reco-jet selected



    //==========fill output tree branch==========================================
    passedEventsTree_All->Fill();

  }
  //=========================loop done==========================================
  cout<<"[INFO] there are "<<nevents_negative<<" "<<"events have negative KD value"<<endl;
  cout<<"[INFO] there are "<<nevents_nassociatedjets_lesstwo<<" "<<"events with number of associated jets less than two"<<endl;

  outfile->cd();
  //passedEventsTree_All->Write();
  TH1F h("sumWeights","sum Weights of Sample",2,0,2);
  h.SetBinContent(1,SumWeight);
  outfile->Write();

  //print cut table infor
  cout<<"passed_trig = "<<passed_trig<<endl;
  cout<<"passed_n_2leps = "<<passed_n_2leps<<endl;
  cout<<"passed_flavor_charge = "<<passed_flavor_charge<<endl;
  cout<<"passed_lepZpt_40_25 = "<<passed_lepZpt_40_25<<endl;
  cout<<"passed_dR_lilj0point2 = "<<passed_dR_lilj0point2<<endl;
  cout<<"passed_Mll_4 = "<<passed_Mll_4<<endl;
  cout<<"passed_lepisolation = "<<passed_lepisolation<<endl;
  cout<<"passed_leptightID = "<<passed_leptightID<<endl;
  cout<<"passed_lepZmass40_180 = "<<passed_lepZmass40_180<<endl;

  cout<<"passed_n_2ak4jets = "<<passed_n_2ak4jets<<endl;
  cout<<"passed_lepclean = "<<passed_lepclean<<endl;
  cout<<"passed_jetpt30 = "<<passed_jetpt30<<endl;
  cout<<"passed_jeteta2opint4 = "<<passed_jeteta2opint4<<endl;
  cout<<"passed_dijetpt100 = "<<passed_dijetpt100<<endl;
  cout<<"passed_ak4jetZm_40_180 = "<<passed_ak4jetZm_40_180<<endl;

  cout<<"passed_resovedonlyHiggs = "<<passed_resovedonlyHiggs<<endl;

  cout<<"passed_nmergedjets_lepclean = "<<passed_nmergedjets_lepclean<<endl;
  cout<<"passed_mergedjetsPt200 = "<<passed_mergedjetsPt200<<endl;
  cout<<"passed_mergedjetEta2opint4 = "<<passed_mergedjetEta2opint4<<endl;
  cout<<"passed_mergedmass_40_180 = "<<passed_mergedmass_40_180<<endl;
  cout<<"passed_particleNetZvsQCD0opint9 = "<<passed_particleNetZvsQCD0opint9<<endl;

  cout<<"passed_MergedolonlyHiggs = "<<passed_MergedolonlyHiggs<<endl;
  cout<<"passed_combinHiggs = "<<passed_combinHiggs<<endl;
  cout<<"passed_combineMerged = "<<passed_combineMerged<<endl;
  cout<<"passed_combineResolved = "<<passed_combineResolved<<endl;


}

//=================find two lepton==============================================
void TreeLoop::findZ1LCandidate(){
  if(verbose){cout<<"[INFO] loop leptons in this event"<<endl;}
  unsigned int Nlep = lepFSR_pt->GetSize();
  if(Nlep<2){return;}
  passed_n_2leps++;
  if(verbose){cout<<"[INFO] found at least tow leptons"<<endl;}

  // First, make all Z candidates including any FSR photons
  int n_Zs=0;
  vector<int> Z_Z1L_lepindex1;
  vector<int> Z_Z1L_lepindex2;

  //variables for cut table
  bool pass_flavor_charge = false;

  for(unsigned int i=0; i<Nlep; i++){

    for(unsigned int j=i+1; j<Nlep; j++){


      // same flavor opposite charge for Z+jet CR. different sgin with different flavor for TTjet CR
      if( (((*lep_id)[i]+(*lep_id)[j])!=0) && ((((*lep_id)[i]*(*lep_id)[j])>0) || (abs((*lep_id)[i])==abs((*lep_id)[j]))) ) continue;
      pass_flavor_charge = true;

      TLorentzVector li, lj;
      li.SetPtEtaPhiM((*lep_pt)[i],(*lep_eta)[i],(*lep_phi)[i],(*lep_mass)[i]);
      lj.SetPtEtaPhiM((*lep_pt)[j],(*lep_eta)[j],(*lep_phi)[j],(*lep_mass)[j]);

      TLorentzVector lifsr, ljfsr;
      lifsr.SetPtEtaPhiM((*lepFSR_pt)[i],(*lepFSR_eta)[i],(*lepFSR_phi)[i],(*lepFSR_mass)[i]);
      ljfsr.SetPtEtaPhiM((*lepFSR_pt)[j],(*lepFSR_eta)[j],(*lepFSR_phi)[j],(*lepFSR_mass)[j]);

      TLorentzVector liljfsr = lifsr+ljfsr;

      TLorentzVector Z, Z_noFSR;
      Z = lifsr+ljfsr;
      Z_noFSR = li+lj;

      if (Z.M()>0.0) {
        n_Zs++;
        Z_Z1L_lepindex1.push_back(i);
        Z_Z1L_lepindex2.push_back(j);
      }
    } // lep i
  } // lep j
  if(verbose){cout<<"[INFO] find leptons pairs"<<endl;}
  if(pass_flavor_charge) passed_flavor_charge++;

  //variables for cut table
  bool pass_lepZpt_40_25 = false;
  bool pass_dR_lilj0point2 = false;
  bool pass_Mll_4 = false;
  bool pass_lepisolation = false;
  bool pass_leptightID = false;
  bool pass_lepZmass40_180 = false;
  // Consider all Z candidates
  double minZ1DeltaM=9999.9;

  if(verbose){cout<<"[INFO] checkout all leptons pairs if is Z candidate"<<endl;}
  for (int i=0; i<n_Zs; i++) {

    int i1 = Z_Z1L_lepindex1[i]; int i2 = Z_Z1L_lepindex2[i];

    TLorentzVector lep_i1, lep_i2;
    lep_i1.SetPtEtaPhiM((*lepFSR_pt)[i1],(*lepFSR_eta)[i1],(*lepFSR_phi)[i1],(*lepFSR_mass)[i1]);
    lep_i2.SetPtEtaPhiM((*lepFSR_pt)[i2],(*lepFSR_eta)[i2],(*lepFSR_phi)[i2],(*lepFSR_mass)[i2]);

    TLorentzVector lep_i1_nofsr, lep_i2_nofsr;
    lep_i1_nofsr.SetPtEtaPhiM((*lep_pt)[i1],(*lep_eta)[i1],(*lep_phi)[i1],(*lep_mass)[i1]);
    lep_i2_nofsr.SetPtEtaPhiM((*lep_pt)[i2],(*lep_eta)[i2],(*lep_phi)[i2],(*lep_mass)[i2]);

    TLorentzVector Zi;
    Zi = lep_i1+lep_i2;

    TLorentzVector Z1 = Zi;
    double Z1DeltaM = abs(Zi.M()-Zmass);
    int Z1_lepindex[2] = {0,0};
    if (lep_i1.Pt()>lep_i2.Pt()) { Z1_lepindex[0] = i1;  Z1_lepindex[1] = i2; }
    else { Z1_lepindex[0] = i2;  Z1_lepindex[1] = i1; }

    // Check Leading and Subleading pt Cut
    vector<double> allPt;
    allPt.push_back(lep_i1.Pt()); allPt.push_back(lep_i2.Pt());
    std::sort(allPt.begin(), allPt.end());
    if (allPt[1]<leadingPtCut || allPt[0]<subleadingPtCut ) continue;
    pass_lepZpt_40_25 = true;

    // Check dR(li,lj)>0.02 for any i,j
    vector<double> alldR;
    alldR.push_back(deltaR(lep_i1.Eta(),lep_i1.Phi(),lep_i2.Eta(),lep_i2.Phi()));
    if (*min_element(alldR.begin(),alldR.end())<0.02) continue;
    pass_dR_lilj0point2 = true;

    // Check M(l+,l-)>4.0 GeV for any OS pair
    // Do not include FSR photons
    vector<double> allM;
    TLorentzVector i1i2;
    i1i2 = (lep_i1_nofsr)+(lep_i2_nofsr);
    allM.push_back(i1i2.M());
    if (*min_element(allM.begin(),allM.end())<4.0) {continue;}
    pass_Mll_4 = true;

    // Check isolation cut (without FSR ) for Z1 leptons
    if ((*lep_RelIsoNoFSR)[Z1_lepindex[0]]>((abs((*lep_id)[Z1_lepindex[0]])==11) ? isoCutEl : isoCutMu)) continue; // checking iso with FSR removed
    if ((*lep_RelIsoNoFSR)[Z1_lepindex[1]]>((abs((*lep_id)[Z1_lepindex[1]])==11) ? isoCutEl : isoCutMu)) continue; // checking iso with FSR removed
    pass_lepisolation = true;
    // Check tight ID cut for Z1 leptons
    if (!((*lep_tightId)[Z1_lepindex[0]])) continue; // checking tight lepton ID
    if (!((*lep_tightId)[Z1_lepindex[1]])) continue; // checking tight lepton ID
    pass_leptightID = true;

    if(Z1.Pt()<100) continue;

    if ( (Z1.M() < mZ1Low) || (Z1.M() > mZ1High) ) continue;
    pass_lepZmass40_180 = true;
    if(verbose){cout<<"[INFO] found Z1 leptons pairs candidate"<<endl;}

    //if (verbose) cout<<"good Z1L candidate, Z1DeltaM: "<<Z1DeltaM<<" minZ1DeltaM: "<<minZ1DeltaM<<endl;
    // Check if this candidate has the best Z1 and highest scalar sum of Z2 lepton pt

    if ( Z1DeltaM<=minZ1DeltaM ) {


      minZ1DeltaM = Z1DeltaM;

      lep_Z1index[0] = Z1_lepindex[0];
      lep_Z1index[1] = Z1_lepindex[1];

      mass2l = Z1.M();
      pt2l = Z1.Pt();

      //if (verbose) cout<<" new best Z1L candidate: massZ1: "<<massZ1<<" (mass3l: "<<mass3l<<")"<<endl;
      found2lepCandidate = true;
      if( ((*lep_id)[lep_Z1index[0]]+(*lep_id)[lep_Z1index[1]])==0 ) foundZ1LCandidate=true;
      if( (((*lep_id)[lep_Z1index[0]]*(*lep_id)[lep_Z1index[1]])<0) && (abs((*lep_id)[lep_Z1index[0]])!=abs((*lep_id)[lep_Z1index[1]])) ) foundTTCRCandidate = true;
    }
  }

  if(pass_lepZpt_40_25) passed_lepZpt_40_25++;
  if(pass_dR_lilj0point2) passed_dR_lilj0point2++;
  if(pass_Mll_4) passed_Mll_4++;
  if(pass_lepisolation) passed_lepisolation++;
  if(pass_leptightID) passed_leptightID++;
  if(pass_lepZmass40_180) passed_lepZmass40_180++;
  if(foundZ1LCandidate) passed_lepZ++;


  if(verbose){
    if(foundZ1LCandidate){cout<<"[INFO] found leptoinc Z candidate."<<endl;}
    else{cout<<"[INFO] no leptoinc Z candidate in this evnets"<<endl;}
  }


}

//==================find two resovled jets=========================================
void TreeLoop::findZ2JCandidata(){
  if(verbose){cout<<"[INFO] loop jets in this events"<<endl;}
  unsigned int Njets = jet_pt->GetSize();
  if(Njets<2){ return; } //found at least two jets
  passed_n_2ak4jets++;
  if(verbose){cout<<"[INFO] find at least two jets. number of jets = "<<Njets<<" in this events"<<endl;}


  int n_Zs=0;
  vector<int> Z_Z2J_jetindex1;
  vector<int> Z_Z2J_jetindex2;
  for(unsigned int i=0; i<Njets; i++){
    for(unsigned int j=i+1; j<Njets; j++){

      TLorentzVector jet_i1, jet_i2;
      jet_i1.SetPtEtaPhiM((*jet_pt)[i],(*jet_eta)[i],(*jet_phi)[i],(*jet_mass)[i]);
      jet_i2.SetPtEtaPhiM((*jet_pt)[j],(*jet_eta)[j],(*jet_phi)[j],(*jet_mass)[j]);

      TLorentzVector Zjet;
      Zjet = jet_i1+jet_i2;

      if (Zjet.M()>0.0) {
        n_Zs++;
        Z_Z2J_jetindex1.push_back(i);
        Z_Z2J_jetindex2.push_back(j);
      }
    } //jet 1
  }//jet 2


  //variables for cut table
  bool pass_jetpt30 = false;
  bool pass_jeteta2opint4 = false;
  bool pass_lepclean = false;
  bool pass_dijetpt100 = false;
  bool pass_ak4jetZm_40_180 = false;

  // Consider all Z candidates
  //double minZ1DeltaM=9999.9;
  double temp_pt = 0.0;

  if(verbose) cout<<"[INFO] start to check each ak4 pair"<<endl;
  for (int i=0; i<n_Zs; i++){

    int i1 = Z_Z2J_jetindex1[i]; int i2 = Z_Z2J_jetindex2[i];
    //int this_jetindex[2] = {i1,i2};

    TLorentzVector jet_i1, jet_i2;
    jet_i1.SetPtEtaPhiM((*jet_pt)[i1],(*jet_eta)[i1],(*jet_phi)[i1],(*jet_mass)[i1]);
    jet_i2.SetPtEtaPhiM((*jet_pt)[i2],(*jet_eta)[i2],(*jet_phi)[i2],(*jet_mass)[i2]);


    TLorentzVector Zi;
    Zi = jet_i1+jet_i2;

    TLorentzVector Z1 = Zi;
    //double Z1DeltaM = abs(Zi.M()-Zmass);

    //all jet must not overlap with tight leptons
    /*
    bool isclean_jet = true;
    unsigned int thisNjets = 2;
    unsigned int Nleptons = lep_pt->GetSize();
    for(unsigned int i=0; i<thisNjets; i++){
      for (unsigned int j=0; j<Nleptons; j++){

        bool passed_idiso=true;
        if (abs((*lep_id)[j])==13 && (*lep_RelIsoNoFSR)[j]>isoCutMu) passed_idiso=false;
        if (abs((*lep_id)[j])==11 && (*lep_RelIsoNoFSR)[j]>isoCutEl) passed_idiso=false;
        if (!((*lep_tightId)[j])) passed_idiso=false;
        //for (int l=0; l<4; l++) {
        //    if ((int)i==lep_Hindex[l]) cand_lep=true;
        //}
        //if (!(passed_idiso || cand_lep)) continue;
        if (!passed_idiso) continue;
        TLorentzVector thisLep;
        thisLep.SetPtEtaPhiM((*lep_pt)[j],(*lep_eta)[j],(*lep_phi)[j],(*lep_mass)[j]);

        double tempDeltaR=999.0;
        tempDeltaR=deltaR((*jet_eta)[this_jetindex[i]],(*jet_phi)[this_jetindex[i]],thisLep.Eta(),thisLep.Phi());
        if (tempDeltaR<0.4){
          isclean_jet = false;
        }
      }
    }
    if(!isclean_jet){continue;}
    */
    pass_lepclean = true;

    //sort jet in terms of Pt order
    int Z2_jetindex[2] = {0,0};
    if (jet_i1.Pt()>jet_i2.Pt()) { Z2_jetindex[0] = i1;  Z2_jetindex[1] = i2; }
    else { Z2_jetindex[0] = i2;  Z2_jetindex[1] = i1; }

    //check each jet pt and eta
    if(jet_i1.Pt()<30 || jet_i2.Pt()<30) {continue;}
    pass_jetpt30 = true;
    if(abs(jet_i1.Eta())>2.4 || abs(jet_i2.Eta())>2.4) {continue;}
    pass_jeteta2opint4 = true;

    //check dijet pt
    if(Zi.Pt()<dijetPtCut){ continue;}
    pass_dijetpt100 = true;

    //check loose dijet mass for 40-180Gev
    if(Zi.M()<mZ1Low || Zi.M()>mZ1High){ continue;}
    pass_ak4jetZm_40_180 = true;
    /*
    if ( Z1DeltaM<=minZ1DeltaM ) {

      minZ1DeltaM = Z1DeltaM;

      jet_Z1index[0] = Z2_jetindex[0];
      jet_Z1index[1] = Z2_jetindex[1];

      mass2jet = Zi.M();
      pt2jet = Zi.Pt();

      //if (verbose) cout<<" new best Z1L candidate: massZ1: "<<massZ1<<" (mass3l: "<<mass3l<<")"<<endl;
      foundZ2JCandidate=true;
    }
    */
    //select as di-jet pt order
    if(Zi.Pt()>temp_pt){
      temp_pt = Zi.Pt();

      jet_Z1index[0] = Z2_jetindex[0];
      jet_Z1index[1] = Z2_jetindex[1];

      mass2jet = Zi.M();
      pt2jet = Zi.Pt();

      foundZ2JCandidate=true;
      if(verbose) cout<<"[INFO] find resovled jet candidates"<<endl;

    }
  }

  if(pass_jetpt30) passed_jetpt30++;
  if(pass_jeteta2opint4) passed_jeteta2opint4++;
  if(pass_lepclean) passed_lepclean++;
  if(pass_dijetpt100) passed_dijetpt100++;
  if(pass_ak4jetZm_40_180) passed_ak4jetZm_40_180++;
}

//==================find mergedjets==================================================
void TreeLoop::findZ2MergedCandidata(){
  if(verbose) cout<<"[INFO] loop mergedjets in this event"<<endl;

  int nmergedjets = mergedjet_pt->GetSize();
  if(verbose) cout<<"nmergedjets = "<<nmergedjets<<endl;

  //cut table
  bool pass_nmergedjets = false;
  bool pass_mergedjetsPt200 = false;
  bool pass_mergedjetEta2opint4 = false;
  bool pass_mergedmass_40_180 = false;
  bool pass_particleNetZvsQCD0opint9 = false;
  if(nmergedjets<1) return;
  pass_nmergedjets = true;
  if(pass_nmergedjets) passed_nmergedjets_lepclean++;

  float this_pt = 0.0;
  for(int i=0; i<nmergedjets; i++){
    if(verbose) cout<<"(*mergedjet_pt)[i] = "<<(*mergedjet_pt)[i]<<endl;
    if((*mergedjet_pt)[i]<200) continue;
    pass_mergedjetsPt200 = true;

    if(verbose) cout<<"(*mergedjet_eta)[i] = "<<(*mergedjet_eta)[i]<<endl;
    if(abs((*mergedjet_eta)[i])>2.4) continue;
    pass_mergedjetEta2opint4 = true;

    if(verbose) cout<<"*mergedjet_subjet_softDropMass)[i] = "<<(*mergedjet_subjet_softDropMass)[i]<<endl;
    if((*mergedjet_subjet_softDropMass)[i]<mZ1Low || (*mergedjet_subjet_softDropMass)[i]>mZ1High) continue;
    pass_mergedmass_40_180 = true;

    if(verbose) cout<<"(*mergedjet_Net_Xbb_de)[i] = "<<(*mergedjet_Net_Xbb_de)[i]<<endl;
    if(verbose) cout<<"(*mergedjet_Net_Xcc_de)[i] = "<<(*mergedjet_Net_Xcc_de)[i]<<endl;
    if(verbose) cout<<"(*mergedjet_Net_Xqq_de)[i] = "<<(*mergedjet_Net_Xqq_de)[i]<<endl;
    float temp_particleNetZvsQCD = (*mergedjet_Net_Xbb_de)[i]+(*mergedjet_Net_Xcc_de)[i]+(*mergedjet_Net_Xqq_de)[i];
    float temp_particleNetZbbvslight = (*mergedjet_Net_Xbb_de)[i]/temp_particleNetZvsQCD;
    //if(temp_particleNetZvsQCD<0.9) continue;
    //cout<<"this passed ZvsQCD = "<<temp_particleNetZvsQCD<<endl;
    //pass_particleNetZvsQCD0opint9 = true;

    foundZ2MergedCandidata = true;
    if((*mergedjet_pt)[i]>this_pt){
      this_pt = (*mergedjet_pt)[i];
      ptmerged = this_pt;
      massmerged = (*mergedjet_subjet_softDropMass)[i];
      merged_Z1index = i;
      particleNetZvsQCD = temp_particleNetZvsQCD;
      particleNetZbbvslight = temp_particleNetZbbvslight;
      if(verbose) cout<<"found merged jet candidates"<<endl;
    }

  }


  if(pass_mergedjetsPt200) passed_mergedjetsPt200++;
  if(pass_mergedjetEta2opint4) passed_mergedjetEta2opint4++;
  if(pass_mergedmass_40_180) passed_mergedmass_40_180++;
  if(pass_particleNetZvsQCD0opint9) passed_particleNetZvsQCD0opint9++;
  //cout<<"this passed_particleNetZvsQCD0opint9 = "<<passed_particleNetZvsQCD0opint9<<endl;

}

//======================GEN VBF=======================================================
void TreeLoop::SetVBFGen(){

  //VBF quark
  int nGEN_VBF  = GEN_VBF_pt->GetSize();
  if(nGEN_VBF>1){
    GEN_associated1_pt = (*GEN_VBF_pt)[0];
    GEN_associated1_eta = (*GEN_VBF_eta)[0];
    GEN_associated1_phi = (*GEN_VBF_phi)[0];
    GEN_associated1_mass = (*GEN_VBF_mass)[0];
    GEN_associated2_pt = (*GEN_VBF_pt)[1];
    GEN_associated2_eta = (*GEN_VBF_eta)[1];
    GEN_associated2_phi = (*GEN_VBF_phi)[1];
    GEN_associated2_mass = (*GEN_VBF_mass)[1];
    GEN_associated12_Deta = GEN_associated1_eta-GEN_associated2_eta;

    TLorentzVector GEN_associated1,GEN_associated2;
    GEN_associated1.SetPtEtaPhiM(GEN_associated1_mass,GEN_associated1_eta,GEN_associated1_phi,GEN_associated1_mass);
    GEN_associated2.SetPtEtaPhiM(GEN_associated2_mass,GEN_associated2_eta,GEN_associated2_phi,GEN_associated2_mass);
    GEN_associated12_mass = (GEN_associated1+GEN_associated2).M();

    //match VBF quark to GEN_jet
    int nGEN_jet = GENjet_pt->GetSize();
    int match1 = 0;
    int match2 = 0;
    int match1_index = -1;
    int match2_index = -1;
    for(int i=0; i<nGEN_jet; i++){
      double this1_dr;
      this1_dr = deltaR((*GENjet_eta)[i],(*GENjet_phi)[i],GEN_associated1_eta,GEN_associated1_phi);
      if(this1_dr<0.4){
        match1++;
        match1_index = i;
      }

      double this2_dr;
      this2_dr = deltaR((*GENjet_eta)[i],(*GENjet_phi)[i],GEN_associated2_eta,GEN_associated2_phi);
      if(this2_dr<0.4){
        match2++;
        match2_index = i;
      }
    }
    if(match1>1){
      if(verbose) cout<<"[!!!!] match1 = "<<match1<<endl;
    }
    if(match2>1){
      if(verbose) cout<<"[!!!!] match2 = "<<match2<<endl;
    }
    if(match1!=0 && match2!=0){
      passedGenquarkMatch = true;
      if(match1_index==match2_index){
        if(verbose) cout<<"[!!!!!] match to same gen jet"<<endl;
      }
    }

    if(passedGenquarkMatch){
      GEN_quark1_match_pt = (*GENjet_pt)[match1_index];
      GEN_quark1_match_eta = (*GENjet_eta)[match1_index];
      GEN_quark1_match_phi = (*GENjet_phi)[match1_index];
      GEN_quark1_match_mass = (*GENjet_mass)[match1_index];

      GEN_quark2_match_pt = (*GENjet_pt)[match2_index];
      GEN_quark2_match_eta = (*GENjet_eta)[match2_index];
      GEN_quark2_match_phi = (*GENjet_phi)[match2_index];
      GEN_quark2_match_mass = (*GENjet_mass)[match2_index];

      TLorentzVector match1_GENjet, match2_GENjet;
      match1_GENjet.SetPtEtaPhiM(GEN_quark1_match_pt,GEN_quark1_match_eta,GEN_quark1_match_phi,GEN_quark1_match_mass);
      match2_GENjet.SetPtEtaPhiM(GEN_quark2_match_pt,GEN_quark2_match_eta,GEN_quark2_match_phi,GEN_quark2_match_mass);
      GEN_quark12_match_mass = (match1_GENjet+match2_GENjet).M();
    }

    //match VBF quark to reco-jet
    int njet = jet_pt->GetSize();
    int match1_reco = 0;
    int match2_reco = 0;
    for(int i=0; i<njet; i++){
      double this1_dr;
      this1_dr = deltaR((*jet_eta)[i],(*jet_phi)[i],GEN_associated1_eta,GEN_associated1_phi);
      if(this1_dr<0.4){
        match1_reco++;
        match1_recoindex = i;
      }

      double this2_dr;
      this2_dr = deltaR((*jet_eta)[i],(*jet_phi)[i],GEN_associated2_eta,GEN_associated2_phi);
      if(this2_dr<0.4){
        match2_reco++;
        match2_recoindex = i;
      }
    }
    if(match1_reco>1){
      if(verbose) cout<<"[!!!!] match1 reco = "<<match1_reco<<endl;
    }
    if(match2_reco>1){
      if(verbose) cout<<"[!!!!] match2 reco= "<<match2_reco<<endl;
    }
    if(match1_reco!=0 && match2_reco!=0){
      passedRecoquarkMatch = true;
      if(match1_recoindex==match2_recoindex){
        if(verbose) cout<<"[!!!!!] match to same gen quark"<<endl;
      }
    }

    if(passedRecoquarkMatch){
      Reco_quark1_match_pt = (*jet_pt)[match1_recoindex];
      Reco_quark1_match_eta = (*jet_eta)[match1_recoindex];
      Reco_quark1_match_phi = (*jet_phi)[match1_recoindex];
      Reco_quark1_match_mass = (*jet_mass)[match1_recoindex];

      Reco_quark2_match_pt = (*jet_pt)[match2_recoindex];
      Reco_quark2_match_eta = (*jet_eta)[match2_recoindex];
      Reco_quark2_match_phi = (*jet_phi)[match2_recoindex];
      Reco_quark2_match_mass = (*jet_mass)[match2_recoindex];

      Reco_quark12_match_DEta = abs(Reco_quark1_match_eta-Reco_quark2_match_eta);

      TLorentzVector match1_Recojet, match2_Recojet;
      match1_Recojet.SetPtEtaPhiM(Reco_quark1_match_pt,Reco_quark1_match_eta,Reco_quark1_match_phi,Reco_quark1_match_mass);
      match2_Recojet.SetPtEtaPhiM(Reco_quark2_match_pt,Reco_quark2_match_eta,Reco_quark2_match_phi,Reco_quark2_match_mass);
      Reco_quark12_match_mass = (match1_Recojet+match2_Recojet).M();
    }

  }

  //match Z quark to reco-jet




  //match Z quark to selected reco-jet
  /*
  if(foundmergedOnly){
    int n_GENZ = GEN_Zq_pt->GetSize();
    for(int i=0; i<n_GENZ; i++){
      double this_DR = deltaR((*GEN_Zq_eta)[i],(*GEN_Zq_phi)[i],(*mergedjet_eta)[merged_Z1index],(*mergedjet_phi)[merged_Z1index]);
      DR_merged_GenZ.push_back(this_DR);
      if(this_DR<0.8){
        DR_merged_GenZ_matched.push_back(this_DR);
        matched_merged_GEN_Z = true;
      }
    }

    for(int i=0; i<n_GENZ; i++){
      double this_DR = deltaR((*GEN_Zq_eta)[i],(*GEN_Zq_phi)[i],associatedjet_eta[0],associatedjet_phi[0]);
      DR_associatedjet1_GenZ.push_back(this_DR);
      if(this_DR<0.4){
        DR_associatedjet1_GenZ_matched.push_back(this_DR);
        matched_associatedjet1_GEN_Z = true;
      }
    }

    for(int i=0; i<n_GENZ; i++){
      double this_DR = deltaR((*GEN_Zq_eta)[i],(*GEN_Zq_phi)[i],associatedjet_eta[1],associatedjet_phi[1]);
      DR_associatedjet2_GenZ.push_back(this_DR);
      if(this_DR<0.4){
        DR_associatedjet2_GenZ_matched.push_back(this_DR);
        matched_associatedjet2_GEN_Z = true;
      }
    }

    if(matched_associatedjet2_GEN_Z && matched_associatedjet1_GEN_Z){
      matched_associatedjet_GEN_Z = true;
    }else if((!matched_associatedjet1_GEN_Z && matched_associatedjet2_GEN_Z) || (matched_associatedjet1_GEN_Z && !matched_associatedjet2_GEN_Z)){
      matched_associatedjetone_GEN_Z = true;
    }


  }

  if(foundresolvedOnly){
    int n_GENZ = GEN_Zq_pt->GetSize();
    for(int i=0; i<n_GENZ; i++){
      double this_DR = deltaR((*GEN_Zq_eta)[i],(*GEN_Zq_phi)[i],(*jet_eta)[jet_Z1index[0]],(*jet_phi)[jet_Z1index[0]]);
      DR_resovled1_GenZ.push_back(this_DR);

      if(this_DR<0.4){
        DR_resovled1_GenZ_matched.push_back(this_DR);
        matched_resovled1_GEN_Z = true;
      }
    }

    for(int i=0; i<n_GENZ; i++){
      double this_DR = deltaR((*GEN_Zq_eta)[i],(*GEN_Zq_phi)[i],(*jet_eta)[jet_Z1index[1]],(*jet_phi)[jet_Z1index[1]]);
      DR_resovled2_GenZ.push_back(this_DR);

      if(this_DR<0.4){
        DR_resovled2_GenZ_matched.push_back(this_DR);
        matched_resovled2_GEN_Z = true;
      }
    }

    if(matched_resovled2_GEN_Z && matched_resovled1_GEN_Z){
      matched_resovled_GEN_Z = true;
    }else if((!matched_resovled2_GEN_Z && matched_resovled1_GEN_Z) || (matched_resovled2_GEN_Z && !matched_resovled1_GEN_Z)){
      matched_resovledone_GEN_Z = true;
    }

  }
  */

}

//=========================set MEs from file==========================================
void TreeLoop::SetMEsFile(){// Set the MEs
  // ME lists
  setMatrixElementListFromFile(
    "${CMSSW_BASE}/src/HZZAnalysis/ANATree/data/RecoProbabilities.me",
    "AJetsVBFProbabilities_SpinZero_JHUGen,AJetsQCDProbabilities_SpinZero_JHUGen",
    //"AJetsVBFProbabilities_SpinZero_JHUGen,AJetsQCDProbabilities_SpinZero_JHUGen,AJetsVHProbabilities_SpinZero_JHUGen,PMAVJJ_SUPERDIJETMELA",
    false
  );

  // Build the MEs if they are specified
  if (!lheMElist.empty() || !recoMElist.empty()){
    // Set up MELA (done only once inside IvyMELAHelpers)
    IvyMELAHelpers::setupMela(year, 125., MiscUtils::INFO);
    // If there are output trees, set the output trees of the MEblock.
    // Do this before building the branches.
    IVYout << "Setting up ME block output trees..." << endl;
    MEblock.addRefTree(passedEventsTree_All);
    //for (auto const& outtree_:productTreeList){
    //  IVYout << "\t- Extracting valid output trees" << endl;
    //  if (!outtree_) cout << "Output tree is NULL!" << endl;
    //  std::vector<TTree*> const& treelist_ = outtree_->getValidTrees();
    //  IVYout << "\t- Registering " << treelist_.size() << " trees" << endl;
    //  for (auto const& tree_:treelist_) MEblock.addRefTree(passedEventsTree_All);
    //  IVYout << "\t- Done" << endl;
    //}
    // Build the MEs
    IVYout << "Building the MEs..." << endl;
    if (!lheMElist.empty()) this->MEblock.buildMELABranches(lheMElist, true);
    if (!recoMElist.empty()) this->MEblock.buildMELABranches(recoMElist, false);
  }
}

//==============================================================================================================//
void TreeLoop::setMatrixElementList(std::vector<std::string> const& MElist, bool const& isGen){
  IVYout << "BaseTreeLooper::setMatrixElementList: Setting " << (isGen ? "gen." : "reco.") << " matrix elements:" << endl;
  for (auto const& sme:MElist) IVYout << '\t' << sme << endl;
  if (isGen) lheMElist = MElist;
  else recoMElist = MElist;
}

//==============================================================================================================//
void TreeLoop::setMatrixElementListFromFile(std::string fname, std::string const& MElistTypes, bool const& isGen){
  if (MElistTypes==""){
    if (this->verbosity>=MiscUtils::ERROR) IVYerr << "BaseTreeLooper::setMatrixElementListFromFile: The ME list types must be specified." << endl;
    assert(0);
  }
  HostHelpers::ExpandEnvironmentVariables(fname);
  if (!HostHelpers::FileReadable(fname.data())){
    if (this->verbosity>=MiscUtils::ERROR) IVYerr << "BaseTreeLooper::setMatrixElementListFromFile: File " << fname << " is not readable." << endl;
    assert(0);
  }

  std::vector<std::string> MEtypes;
  HelperFunctions::splitOptionRecursive(MElistTypes, MEtypes, ',', true);

  // Read the file and collect the MEs
  std::vector<std::string> MElist;
  ifstream fin;
  fin.open(fname.c_str());
  if (fin.good()){
    bool acceptString = false;
    while (!fin.eof()){
      std::string str_in="";
      getline(fin, str_in);
      HelperFunctions::lstrip(str_in);
      HelperFunctions::lstrip(str_in, "\"\'");
      HelperFunctions::rstrip(str_in); HelperFunctions::rstrip(str_in, ",\"\'");

      if (str_in=="" || str_in.find('#')==0) continue;
      else if (str_in.find(']')!=std::string::npos){
        acceptString = false;
        continue;
      }

      bool isMEline = (str_in.find("Name")!=std::string::npos);
      for (auto const& MEtype:MEtypes){
        if (str_in.find(MEtype)!=std::string::npos){
          isMEline = false;
          acceptString = true;
        }
      }

      if (isMEline && acceptString){
        if (isGen && str_in.find("isGen:1")==std::string::npos){
          if (this->verbosity>=MiscUtils::ERROR) IVYerr << "BaseTreeLooper::setMatrixElementListFromFile: ME string " << str_in << " is not a gen. ME while the acquisition is done for gen. MEs!" << endl;
          continue;
        }
        MElist.push_back(str_in);
      }
    }
  }
  fin.close();

  if (!MElist.empty()) setMatrixElementList(MElist, isGen);
  else{
    //if (this->verbosity>=MiscUtils::ERROR) IVYerr << "BaseTreeLooper::setMatrixElementListFromFile: File " << fname << " does not contain any of the ME types " << MEtypes << "." << endl;
  }
}

//=========================set input and output tree==================================
void TreeLoop::setTree(){
  if(verbose){cout<<"[INFO] set input tree variables"<<endl;}
  //oldfile->cd();
  lep_id = new TTreeReaderArray<int>(*myreader, "lep_id");
  lepFSR_pt = new TTreeReaderArray<double>(*myreader, "lepFSR_pt");
  lepFSR_eta  = new TTreeReaderArray<double>(*myreader, "lepFSR_eta");
  lepFSR_phi = new TTreeReaderArray<double>(*myreader, "lepFSR_phi");
  lepFSR_mass  = new TTreeReaderArray<double>(*myreader, "lepFSR_mass");
  lep_pt = new TTreeReaderArray<double>(*myreader, "lep_pt");
  lep_eta  = new TTreeReaderArray<double>(*myreader, "lep_eta");
  lep_phi = new TTreeReaderArray<double>(*myreader, "lep_phi");
  lep_mass  = new TTreeReaderArray<double>(*myreader, "lep_mass");
  lep_tightId = new TTreeReaderArray<int>(*myreader, "lep_tightId");
  lep_RelIsoNoFSR = new TTreeReaderArray<float>(*myreader, "lep_RelIsoNoFSR");
  lep_dataMC = new TTreeReaderArray<float>(*myreader,"lep_dataMC");
  passedTrig = new TTreeReaderValue<bool>(*myreader,"passedTrig");

  eventWeight = new TTreeReaderValue<float>(*myreader,"eventWeight");
  genWeight = new TTreeReaderValue<float>(*myreader,"genWeight");
  pileupWeight = new TTreeReaderValue<float>(*myreader,"pileupWeight");
  prefiringWeight = new TTreeReaderValue<float>(*myreader,"prefiringWeight");
  Run = new TTreeReaderValue<ULong64_t>(*myreader,"Run");
  Event = new TTreeReaderValue<ULong64_t>(*myreader,"Event");
  LumiSect = new TTreeReaderValue<ULong64_t>(*myreader,"LumiSect");


  jet_pt = new TTreeReaderArray<double>(*myreader, "jet_pt");
  jet_eta = new TTreeReaderArray<double>(*myreader, "jet_eta");
  jet_phi = new TTreeReaderArray<double>(*myreader, "jet_phi");
  jet_mass = new TTreeReaderArray<double>(*myreader, "jet_mass");

  mergedjet_pt = new TTreeReaderArray<float>(*myreader,"mergedjet_pt");
  mergedjet_eta = new TTreeReaderArray<float>(*myreader,"mergedjet_eta");
  mergedjet_phi = new TTreeReaderArray<float>(*myreader,"mergedjet_phi");
  mergedjet_subjet_softDropMass = new TTreeReaderArray<float>(*myreader,"mergedjet_subjet_softDropMass");
  mergedjet_Net_Xbb_de = new TTreeReaderArray<float>(*myreader,"mergedjet_Net_Xbb_de");
  mergedjet_Net_Xcc_de = new TTreeReaderArray<float>(*myreader,"mergedjet_Net_Xcc_de");
  mergedjet_Net_Xqq_de = new TTreeReaderArray<float>(*myreader,"mergedjet_Net_Xqq_de");
  mergedjet_nsubjet = new TTreeReaderArray<int>(*myreader,"mergedjet_nsubjet");
  mergedjet_subjet_pt = new TTreeReaderArray<vector<float>>(*myreader,"mergedjet_subjet_pt");
  mergedjet_subjet_eta = new TTreeReaderArray<vector<float>>(*myreader,"mergedjet_subjet_eta");
  mergedjet_subjet_phi = new TTreeReaderArray<vector<float>>(*myreader,"mergedjet_subjet_phi");
  mergedjet_subjet_mass = new TTreeReaderArray<vector<float>>(*myreader,"mergedjet_subjet_mass");
  mergedjet_nbHadrons = new TTreeReaderArray<int>(*myreader,"mergedjet_nbHadrons");
  mergedjet_ncHadrons = new TTreeReaderArray<int>(*myreader,"mergedjet_ncHadrons");

  met = new TTreeReaderValue<float>(*myreader,"met");
  met_phi = new TTreeReaderValue<float>(*myreader,"met_phi");


  GEN_Zq_pt = new TTreeReaderArray<double>(*myreader,"GEN_Zq_pt");
  GEN_Zq_eta = new TTreeReaderArray<double>(*myreader,"GEN_Zq_eta");
  GEN_Zq_phi = new TTreeReaderArray<double>(*myreader,"GEN_Zq_phi");
  GEN_Zq_mass = new TTreeReaderArray<double>(*myreader,"GEN_Zq_mass");

  GEN_q_pt = new TTreeReaderArray<double>(*myreader,"GEN_q_pt");
  GEN_q_eta = new TTreeReaderArray<double>(*myreader,"GEN_q_eta");
  GEN_q_phi = new TTreeReaderArray<double>(*myreader,"GEN_q_phi");
  GEN_q_mass = new TTreeReaderArray<double>(*myreader,"GEN_q_mass");

  GENjet_pt = new TTreeReaderArray<double>(*myreader,"GENjet_pt");
  GENjet_eta = new TTreeReaderArray<double>(*myreader,"GENjet_eta");
  GENjet_phi = new TTreeReaderArray<double>(*myreader,"GENjet_phi");
  GENjet_mass = new TTreeReaderArray<double>(*myreader,"GENjet_mass");

  GEN_Zq_id = new TTreeReaderArray<int>(*myreader,"GEN_Zq_id");
  GEN_q_id = new TTreeReaderArray<int>(*myreader,"GEN_q_id");
  GEN_q_status = new TTreeReaderArray<int>(*myreader,"GEN_q_status");
  GEN_q_Momid = new TTreeReaderArray<int>(*myreader,"GEN_q_Momid");
  GEN_q_MomMomid = new TTreeReaderArray<int>(*myreader,"GEN_q_MomMomid");
  GEN_q_nDaughters = new TTreeReaderArray<int>(*myreader,"GEN_q_nDaughters");

  GEN_qdau_id = new TTreeReaderArray<vector<int>>(*myreader,"GEN_qdau_id");
  GEN_qdau_status = new TTreeReaderArray<vector<int>>(*myreader,"GEN_qdau_status");

  GEN_qdau_pt = new TTreeReaderArray<vector<double>>(*myreader,"GEN_qdau_pt");
  GEN_qdau_eta = new TTreeReaderArray<vector<double>>(*myreader,"GEN_qdau_eta");
  GEN_qdau_phi = new TTreeReaderArray<vector<double>>(*myreader,"GEN_qdau_phi");
  GEN_qdau_mass = new TTreeReaderArray<vector<double>>(*myreader,"GEN_qdau_mass");
  GEN_VBF_pt = new TTreeReaderArray<double>(*myreader,"GEN_VBF_pt");
  GEN_VBF_eta = new TTreeReaderArray<double>(*myreader,"GEN_VBF_eta");
  GEN_VBF_phi = new TTreeReaderArray<double>(*myreader,"GEN_VBF_phi");
  GEN_VBF_mass = new TTreeReaderArray<double>(*myreader,"GEN_VBF_mass");


  passedEventsTree_All->Branch("mass2jet",&mass2jet);
  passedEventsTree_All->Branch("pt2jet",&pt2jet);
  passedEventsTree_All->Branch("mass2l",&mass2l);
  passedEventsTree_All->Branch("pt2l",&pt2l);
  passedEventsTree_All->Branch("mass2l2jet",&mass2l2jet);
  passedEventsTree_All->Branch("mass2lj",&mass2lj);
  passedEventsTree_All->Branch("KD_jjVBF",&KD_jjVBF);
  passedEventsTree_All->Branch("KD_jVBF",&KD_jVBF);
  passedEventsTree_All->Branch("particleNetZvsQCD",&particleNetZvsQCD);
  passedEventsTree_All->Branch("particleNetZbbvslight",&particleNetZbbvslight);
  passedEventsTree_All->Branch("massmerged",&massmerged);
  passedEventsTree_All->Branch("ptmerged",&ptmerged);
  passedEventsTree_All->Branch("nsubjet",&nsubjet);
  passedEventsTree_All->Branch("foundZ1LCandidate",&foundZ1LCandidate);
  passedEventsTree_All->Branch("foundZ2JCandidate",&foundZ2JCandidate);
  passedEventsTree_All->Branch("foundZ2MergedCandidata",&foundZ2MergedCandidata);
  passedEventsTree_All->Branch("passedfullmerged",&passedfullmerged);
  passedEventsTree_All->Branch("passedfullresolved",&passedfullresolved);
  passedEventsTree_All->Branch("passedNassociated",&passedNassociated);
  passedEventsTree_All->Branch("found2lepCandidate",&found2lepCandidate);
  passedEventsTree_All->Branch("foundTTCRCandidate",&foundTTCRCandidate);
  passedEventsTree_All->Branch("isEE",&isEE);
  passedEventsTree_All->Branch("isMuMu",&isMuMu);

  passedEventsTree_All->Branch("EventWeight",&EventWeight);
  passedEventsTree_All->Branch("GenWeight",&GenWeight);
  passedEventsTree_All->Branch("PileupWeight",&PileupWeight);
  passedEventsTree_All->Branch("PrefiringWeight",&PrefiringWeight);
  passedEventsTree_All->Branch("SumWeight",&SumWeight);
  passedEventsTree_All->Branch("run",&run,"run/l");
  passedEventsTree_All->Branch("event",&event,"event/l");
  passedEventsTree_All->Branch("lumiSect",&lumiSect,"lumiSect/l");

  passedEventsTree_All->Branch("Met",&Met);
  passedEventsTree_All->Branch("Met_phi",&Met_phi);


  passedEventsTree_All->Branch("lep_Z1index",&lep_Z1index,"lep_Z1index[2]/I");
  passedEventsTree_All->Branch("jet_Z1index",&jet_Z1index,"jet_Z1index[2]/I");
  passedEventsTree_All->Branch("merged_Z1index",&merged_Z1index);

  passedEventsTree_All->Branch("associatedjet_pt",&associatedjet_pt);
  passedEventsTree_All->Branch("associatedjet_eta",&associatedjet_eta);
  passedEventsTree_All->Branch("associatedjet_phi",&associatedjet_phi);
  passedEventsTree_All->Branch("associatedjet_mass",&associatedjet_mass);
  passedEventsTree_All->Branch("associatedjets_index",&associatedjets_index);

  passedEventsTree_All->Branch("isbjet",&isbjet);
  passedEventsTree_All->Branch("iscjet",&iscjet);
  passedEventsTree_All->Branch("islightjet",&islightjet);

  passedEventsTree_All->Branch("GEN_H1_pt",&GEN_H1_pt);
  passedEventsTree_All->Branch("GEN_H1_eta",&GEN_H1_eta);
  passedEventsTree_All->Branch("GEN_H1_phi",&GEN_H1_phi);
  passedEventsTree_All->Branch("GEN_H1_mass",&GEN_H1_mass);
  passedEventsTree_All->Branch("GEN_H2_pt",&GEN_H2_pt);
  passedEventsTree_All->Branch("GEN_H2_eta",&GEN_H2_eta);
  passedEventsTree_All->Branch("GEN_H2_phi",&GEN_H2_phi);
  passedEventsTree_All->Branch("GEN_H2_mass",&GEN_H2_mass);
  passedEventsTree_All->Branch("GEN_DR_H1_Mom",&GEN_DR_H1_Mom);
  passedEventsTree_All->Branch("GEN_DR_H2_Mom",&GEN_DR_H2_Mom);

  passedEventsTree_All->Branch("GEN_H1_Mom_pt",&GEN_H1_Mom_pt);
  passedEventsTree_All->Branch("GEN_H1_Mom_eta",&GEN_H1_Mom_eta);
  passedEventsTree_All->Branch("GEN_H1_Mom_phi",&GEN_H1_Mom_phi);
  passedEventsTree_All->Branch("GEN_H1_Mom_mass",&GEN_H1_Mom_mass);
  passedEventsTree_All->Branch("GEN_H2_Mom_pt",&GEN_H2_Mom_pt);
  passedEventsTree_All->Branch("GEN_H2_Mom_eta",&GEN_H2_Mom_eta);
  passedEventsTree_All->Branch("GEN_H2_Mom_phi",&GEN_H2_Mom_phi);
  passedEventsTree_All->Branch("GEN_H2_Mom_mass",&GEN_H2_Mom_mass);

  passedEventsTree_All->Branch("GEN_H1_Bro_pt",&GEN_H1_Bro_pt);
  passedEventsTree_All->Branch("GEN_H1_Bro_eta",&GEN_H1_Bro_eta);
  passedEventsTree_All->Branch("GEN_H1_Bro_phi",&GEN_H1_Bro_phi);
  passedEventsTree_All->Branch("GEN_H1_Bro_mass",&GEN_H1_Bro_mass);
  passedEventsTree_All->Branch("GEN_DR_H1_Bro",&GEN_DR_H1_Bro);
  passedEventsTree_All->Branch("GEN_DR_H1Mom_Bro",&GEN_DR_H1Mom_Bro);
  passedEventsTree_All->Branch("GEN_H2_Bro_pt",&GEN_H1_Bro_pt);
  passedEventsTree_All->Branch("GEN_H2_Bro_eta",&GEN_H2_Bro_eta);
  passedEventsTree_All->Branch("GEN_H2_Bro_phi",&GEN_H2_Bro_phi);
  passedEventsTree_All->Branch("GEN_H2_Bro_mass",&GEN_H2_Bro_mass);
  passedEventsTree_All->Branch("GEN_DR_H2_Bro",&GEN_DR_H2_Bro);
  passedEventsTree_All->Branch("GEN_DR_H2Mom_Bro",&GEN_DR_H2Mom_Bro);
  passedEventsTree_All->Branch("GEN_DR_Bro12",&GEN_DR_Bro12);
  passedEventsTree_All->Branch("GEN_DEta_H1_Bro",&GEN_DEta_H1_Bro);
  passedEventsTree_All->Branch("GEN_DEta_H1Mom_Bro",&GEN_DEta_H1Mom_Bro);
  passedEventsTree_All->Branch("GEN_DEta_H2_Bro",&GEN_DEta_H2_Bro);
  passedEventsTree_All->Branch("GEN_DEta_H2Mom_Bro",&GEN_DEta_H2Mom_Bro);
  passedEventsTree_All->Branch("GEN_DEta_Bro12",&GEN_DEta_Bro12);
  passedEventsTree_All->Branch("passedGENH",&passedGENH);

  passedEventsTree_All->Branch("DR_merged_GenZ",&DR_merged_GenZ);
  passedEventsTree_All->Branch("matched_merged_GEN_Z",&matched_merged_GEN_Z);
  passedEventsTree_All->Branch("DR_merged_GenZ_matched",&DR_merged_GenZ_matched);
  passedEventsTree_All->Branch("DR_resovled1_GenZ",&DR_resovled1_GenZ);
  passedEventsTree_All->Branch("DR_resovled2_GenZ",&DR_resovled2_GenZ);
  passedEventsTree_All->Branch("DR_resovled2_GenZ_matched",&DR_resovled2_GenZ_matched);
  passedEventsTree_All->Branch("DR_resovled1_GenZ_matched",&DR_resovled1_GenZ_matched);
  passedEventsTree_All->Branch("matched_resovled1_GEN_Z",&matched_resovled1_GEN_Z);
  passedEventsTree_All->Branch("matched_resovled2_GEN_Z",&matched_resovled2_GEN_Z);
  passedEventsTree_All->Branch("matched_resovled_GEN_Z",&matched_resovled_GEN_Z);
  passedEventsTree_All->Branch("matched_resovledone_GEN_Z",&matched_resovledone_GEN_Z);

  passedEventsTree_All->Branch("matched_associatedjet1_GEN_Z",&matched_associatedjet1_GEN_Z);
  passedEventsTree_All->Branch("matched_associatedjet2_GEN_Z",&matched_associatedjet2_GEN_Z);
  passedEventsTree_All->Branch("matched_associatedjet2_GEN_Z",&matched_associatedjet_GEN_Z);
  passedEventsTree_All->Branch("matched_associatedjetone_GEN_Z",&matched_associatedjetone_GEN_Z);
  passedEventsTree_All->Branch("DR_associatedjet1_GenZ",&DR_associatedjet1_GenZ);
  passedEventsTree_All->Branch("DR_associatedjet2_GenZ",&DR_associatedjet2_GenZ);
  passedEventsTree_All->Branch("DR_associatedjet1_GenZ_matched",&DR_associatedjet1_GenZ_matched);
  passedEventsTree_All->Branch("DR_associatedjet2_GenZ_matched",&DR_associatedjet2_GenZ_matched);

  //VBF
  passedEventsTree_All->Branch("GEN_associated1_pt",&GEN_associated1_pt);
  passedEventsTree_All->Branch("GEN_associated1_eta",&GEN_associated1_eta);
  passedEventsTree_All->Branch("GEN_associated1_phi",&GEN_associated1_phi);
  passedEventsTree_All->Branch("GEN_associated1_mass",&GEN_associated1_mass);
  passedEventsTree_All->Branch("GEN_associated2_pt",&GEN_associated2_pt);
  passedEventsTree_All->Branch("GEN_associated2_eta",&GEN_associated2_eta);
  passedEventsTree_All->Branch("GEN_associated2_phi",&GEN_associated2_phi);
  passedEventsTree_All->Branch("GEN_associated2_mass",&GEN_associated2_mass);
  passedEventsTree_All->Branch("GEN_associated12_mass",&GEN_associated12_mass);
  passedEventsTree_All->Branch("GEN_associated12_Deta",&GEN_associated12_Deta);

  passedEventsTree_All->Branch("GEN_quark1_match_pt",&GEN_quark1_match_pt);
  passedEventsTree_All->Branch("GEN_quark1_match_eta",&GEN_quark1_match_eta);
  passedEventsTree_All->Branch("GEN_quark1_match_phi",&GEN_quark1_match_phi);
  passedEventsTree_All->Branch("GEN_quark1_match_mass",&GEN_quark1_match_mass);
  passedEventsTree_All->Branch("GEN_quark2_match_pt",&GEN_quark2_match_pt);
  passedEventsTree_All->Branch("GEN_quark2_match_eta",&GEN_quark2_match_eta);
  passedEventsTree_All->Branch("GEN_quark2_match_phi",&GEN_quark2_match_phi);
  passedEventsTree_All->Branch("GEN_quark2_match_mass",&GEN_quark2_match_mass);
  passedEventsTree_All->Branch("GEN_quark12_match_mass",&GEN_quark12_match_mass);
  passedEventsTree_All->Branch("passedGenquarkMatch",&passedGenquarkMatch);

  passedEventsTree_All->Branch("Reco_quark1_match_pt",&Reco_quark1_match_pt);
  passedEventsTree_All->Branch("Reco_quark1_match_eta",&Reco_quark1_match_eta);
  passedEventsTree_All->Branch("Reco_quark1_match_phi",&Reco_quark1_match_phi);
  passedEventsTree_All->Branch("Reco_quark1_match_mass",&Reco_quark1_match_mass);
  passedEventsTree_All->Branch("Reco_quark2_match_pt",&Reco_quark2_match_pt);
  passedEventsTree_All->Branch("Reco_quark2_match_eta",&Reco_quark2_match_eta);
  passedEventsTree_All->Branch("Reco_quark2_match_phi",&Reco_quark2_match_phi);
  passedEventsTree_All->Branch("Reco_quark2_match_mass",&Reco_quark2_match_mass);
  passedEventsTree_All->Branch("Reco_quark12_match_mass",&Reco_quark12_match_mass);
  passedEventsTree_All->Branch("Reco_quark12_match_DEta",&Reco_quark12_match_DEta);
  passedEventsTree_All->Branch("passedRecoquarkMatch",&passedRecoquarkMatch);
  passedEventsTree_All->Branch("match2_recoindex",&match2_recoindex);
  passedEventsTree_All->Branch("match1_recoindex",&match1_recoindex);
  passedEventsTree_All->Branch("DR_merged_VBF1_matched",&DR_merged_VBF1_matched);
  passedEventsTree_All->Branch("DR_merged_VBF2_matched",&DR_merged_VBF2_matched);
  passedEventsTree_All->Branch("DR_merged_asccoiacted",&DR_merged_asccoiacted);
  passedEventsTree_All->Branch("DR_selectedleps_asccoiacted",&DR_selectedleps_asccoiacted);
  passedEventsTree_All->Branch("passedmatchtruthVBF",&passedmatchtruthVBF);
  passedEventsTree_All->Branch("time_associatedjet_eta",&time_associatedjet_eta);




}

//========================initialize=======================
void TreeLoop::initialize(){
  //initialize
  foundZ1LCandidate = false;
  foundZ2JCandidate = false;
  found4lCandidate = false;
  foundZ2MergedCandidata = false;
  foundmergedOnly = false;
  foundresolvedOnly = false;
  foundresolvedCombine = false;
  foundmergedCombine = false;
  found2lepCandidate = false;
  foundTTCRCandidate = false;
  passedfullmerged = false;
  passedfullresolved = false;
  passedNassociated = false;
  isEE = false; isMuMu = false;
  for (int i=0; i<2; ++i) {lep_Z1index[i]=-1;}
  for (int i=0; i<2; ++i) {jet_Z1index[i]=-1;}
  for (int i=0; i<2; ++i) {lep_Hindex[i]=-1;}
  merged_Z1index = -1;

  EventWeight = 1.0; GenWeight = 1.0; PileupWeight = 1.0; PrefiringWeight = 1.0;
  mass2jet=-999.99;
  pt2jet=-999.99;
  mass2l=-999.99;
  pt2l=-999.99;
  mass2l2jet=-999.99;
  KD_jjVBF = -999.99;
  massmerged = -999.99;
  ptmerged = -999.99;
  nsubjet = -999.99;
  mass2lj = -999.99;

  particleNetZvsQCD = -999.99;
  particleNetZbbvslight = -999.99;

  passedGENH = false;
  isbjet = false; iscjet = false; islightjet = false;

  Met = 0.0; Met_phi = 0.0;
  GEN_H1_pt=-999.0; GEN_H1_eta=-999.0; GEN_H1_phi=-999.0; GEN_H1_mass=-999.0;
  GEN_H2_pt=-999.0; GEN_H2_eta=-999.0; GEN_H2_phi=-999.0; GEN_H2_mass=-999.0;
  GEN_DR_H1_Mom = -999.0; GEN_DR_H2_Mom=-999.0;
  GEN_H1_Mom_pt=-999.0; GEN_H1_Mom_eta=-999.0; GEN_H1_Mom_phi=-999.0; GEN_H1_Mom_mass=-999.0;
  GEN_H2_Mom_pt=-999.0; GEN_H2_Mom_eta=-999.0; GEN_H2_Mom_phi=-999.0; GEN_H2_Mom_mass=-999.0;

  GEN_H1_Bro_pt.clear(); GEN_H1_Bro_eta.clear(); GEN_H1_Bro_phi.clear(); GEN_H1_Bro_mass.clear(); GEN_DR_H1_Bro.clear(); GEN_DR_H1Mom_Bro.clear();
  GEN_H2_Bro_pt.clear(); GEN_H2_Bro_eta.clear(); GEN_H2_Bro_phi.clear(); GEN_H2_Bro_mass.clear(); GEN_DR_H2_Bro.clear(); GEN_DR_H2Mom_Bro.clear();
  GEN_DR_Bro12.clear();
  GEN_DEta_H1_Bro.clear(); GEN_DEta_H1Mom_Bro.clear(); GEN_DEta_H2_Bro.clear(); GEN_DEta_H2Mom_Bro.clear();
  GEN_DEta_Bro12.clear();
  DR_merged_GenZ.clear(); DR_merged_GenZ_matched.clear(); DR_resovled2_GenZ_matched.clear(); DR_resovled1_GenZ_matched.clear();
  DR_resovled1_GenZ.clear(); DR_resovled2_GenZ.clear();
  DR_associatedjet1_GenZ.clear(); DR_associatedjet2_GenZ.clear(); DR_associatedjet1_GenZ_matched.clear(); DR_associatedjet2_GenZ_matched.clear();
  matched_merged_GEN_Z = false; matched_resovled1_GEN_Z=false; matched_resovled2_GEN_Z = false; matched_resovled_GEN_Z = false; matched_resovledone_GEN_Z=false;
  matched_associatedjet1_GEN_Z = false; matched_associatedjet2_GEN_Z = false; matched_associatedjet_GEN_Z = false; matched_associatedjetone_GEN_Z = false;
  associatedjet_pt.clear(); associatedjet_eta.clear(); associatedjet_phi.clear(); associatedjet_mass.clear();
  associatedjets_index.clear();

  //VBF
  GEN_associated1_pt=-999.0; GEN_associated1_eta=-999.0; GEN_associated1_phi=-999.0; GEN_associated1_mass=-999.0;
  GEN_associated2_pt=-999.0; GEN_associated2_eta=-999.0; GEN_associated2_phi=-999.0; GEN_associated2_mass=-999.0;
  GEN_associated12_mass=-999.0; GEN_associated12_Deta=-999.0;


  GEN_quark1_match_pt=-999.0; GEN_quark1_match_eta=-999.0; GEN_quark1_match_phi=-999.0; GEN_quark1_match_mass=-999.0;
  GEN_quark2_match_pt=-999.0; GEN_quark2_match_eta=-999.0; GEN_quark2_match_phi=-999.0; GEN_quark2_match_mass=-999.0;
  GEN_quark12_match_mass=-999.0;
  passedGenquarkMatch=false;

  Reco_quark1_match_pt=-999.0; Reco_quark1_match_eta=-999.0; Reco_quark1_match_phi=-999.0; Reco_quark1_match_mass=-999.0;
  Reco_quark2_match_pt=-999.0; Reco_quark2_match_eta=-999.0; Reco_quark2_match_phi=-999.0; Reco_quark2_match_mass=-999.0;
  Reco_quark12_match_mass=-999.0;
  passedRecoquarkMatch = false;
  match2_recoindex=-999; match1_recoindex=-999;
  Reco_quark12_match_DEta=-999.9;
  DR_merged_VBF1_matched = -999.9; DR_merged_VBF2_matched=-999.0;
  DR_merged_asccoiacted.clear();
  DR_selectedleps_asccoiacted.clear();
  passedmatchtruthVBF = false;
  time_associatedjet_eta = -999.0;
  //initialize done

}
