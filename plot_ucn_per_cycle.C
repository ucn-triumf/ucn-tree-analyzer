// Macro for finding the number of UCN per cycle.



void plot_ucn_per_cycle(std::string infile){
  
  
  
  ULong64_t eventTot;
  double cycleStart;

  TFile* fin = new TFile(infile.c_str(),"READ");
  if (fin == NULL) {
    std::cout<<"could not open file : "<<infile<<std::endl;
    return;
  } 
  if ( fin->IsOpen() == false ) {
    std::cout<<"file not open : "<<infile<<std::endl;
    
    return;
  }

  fin->ls();
  
  
  TTree* uin = (TTree*) fin->Get("UCNHits_Li-6");
  TTree* truntime = (TTree*) fin->Get("RunTransitions_Li-6");
  TTree* sourceTree = (TTree*) fin->Get("SourceEpicsTree");
  uin->Print();  
  truntime->Print();


  // Loop over the UCN source EPICS reads, saving the temperature
  std::vector<double> sourceReadTimes;
  std::vector<double> ts27_temperaturs;
  int timestamp;
  double UCN_D2O_TS7_RDTEMP;
  sourceTree = (TTree*) fin->Get("SourceEpicsTree");
  sourceTree->SetBranchAddress("timestamp",&timestamp);
  sourceTree->SetBranchAddress("UCN_D2O_TS7_RDTEMP",&UCN_D2O_TS7_RDTEMP);
  
  eventTot = (Double_t)sourceTree->GetEntries();
  for(ULong64_t j=0;j<eventTot;j++) {
    sourceTree->GetEvent(j);
    sourceReadTimes.push_back(timestamp);
    ts27_temperaturs.push_back(UCN_D2O_TS7_RDTEMP);
  }

  
  // Loop over the transitions, save the start of each transition and make a counter.
  std::vector<double> cycleStartTimes;
  std::vector<int> numberEventsPerCycle;
  std::vector<double> temperaturePerCycle;

  truntime->SetBranchAddress("tUnixTimeTransition",&cycleStart);
  eventTot = (Double_t)truntime->GetEntries();
  for(ULong64_t j=0;j<eventTot;j++) {
    truntime->GetEvent(j);
    cycleStartTimes.push_back(cycleStart);
    numberEventsPerCycle.push_back(0);
    // Find the temperature for this cycle.
    double temperature = 0;
    for(unsigned int i = 0; i < sourceReadTimes.size()-1; i++){
      if(cycleStart >= sourceReadTimes[i] && cycleStart < sourceReadTimes[i+1]){
        temperature = ts27_temperaturs[i];
        break;
      }

    }
    temperaturePerCycle.push_back(temperature);
  }


  
  //  Now loop over the UCN hits.  Calculate the right number of hits for each cycle.
  double tUnixTimePrecise;
  Float_t tPSD;  
  UShort_t tChargeL;
  UShort_t tIsUCN, tChannel;
  uin->SetBranchAddress("tUnixTimePrecise",&tUnixTimePrecise);
  uin->SetBranchAddress("tIsUCN",&tIsUCN);
  uin->SetBranchAddress("tChannel",&tChannel);
  uin->SetBranchAddress("tPSD",&tPSD);
  uin->SetBranchAddress("tChargeL",&tChargeL);

  // Make on plot showing PSD vs QL for channel 0.
  TH2F *psd_vs_ql = new TH2F("psd_vs_ql","PSD vs QL",200,0,15000,200,-1,1);

  // pointer for closeSourceEPics reads;
  int source_index = 0;
  
  eventTot = (Double_t)uin->GetEntries();
  for(ULong64_t j=0;j<eventTot;j++) {
    uin->GetEvent(j);
    // Check if this is a UCN hit.
    if(tIsUCN){
      // check which cycle this is in...
      for(unsigned int i = 0; i < cycleStartTimes.size()-1; i++){
	//std::cout << tUnixTimePrecise << " " <<  cycleStartTimes[i] << " " << cycleStartTimes[i+1] << std::endl;
	if(tUnixTimePrecise >= cycleStartTimes[i] && tUnixTimePrecise < cycleStartTimes[i+1]){
	  numberEventsPerCycle[i]++;
	  break;
	}
      }
      
    }

    if(tChannel == 0){
      psd_vs_ql->Fill(tChargeL,tPSD);
    }
    
  }


  // Plot PSD vs QL
  TCanvas *c1 = new TCanvas("c1","PSD vs QL");
  psd_vs_ql->Draw("COL");

  // Plot the UCN counts per cycle
  TGraph *ucn_per_cycle = new TGraph();
  for(unsigned int i = 0; i < cycleStartTimes.size()-1; i++){
    ucn_per_cycle->SetPoint(i,cycleStartTimes[i],numberEventsPerCycle[i]);
  }
  TCanvas *c2 = new TCanvas("c2","UCN vs cycle");

  ucn_per_cycle->Draw("AP*");
  ucn_per_cycle->GetXaxis()->SetTitle("Cycle time");
  ucn_per_cycle->GetYaxis()->SetTitle("Number of UCN");


  // Plot the UCN counts vs temperature
  TGraph *ucn_vs_temperature = new TGraph();
  for(unsigned int i = 0; i < cycleStartTimes.size()-1; i++){
    ucn_vs_temperature->SetPoint(i,temperaturePerCycle[i],numberEventsPerCycle[i]);
  }
  TCanvas *c3 = new TCanvas("c3","UCN counts vs TS27 temperature");

  ucn_vs_temperature->Draw("AP*");
  ucn_vs_temperature->GetXaxis()->SetTitle("TS27 temperature");
  ucn_vs_temperature->GetYaxis()->SetTitle("Number of UCN");





  
  
  
}
