// system include files
#include <memory>
#include <iostream>

// user include files
#include "FWCore/Framework/interface/Frameworkfwd.h"
#include "FWCore/Framework/interface/one/EDAnalyzer.h"
#include "FWCore/Framework/interface/Event.h"
#include "FWCore/Framework/interface/EventSetup.h"
#include "FWCore/Framework/interface/MakerMacros.h"
#include "FWCore/ParameterSet/interface/ParameterSet.h"
#include "FWCore/Utilities/interface/InputTag.h"
#include "FWCore/ServiceRegistry/interface/Service.h"

#include "DataFormats/L1CaloTrigger/interface/CICADA.h"

#include "CommonTools/UtilAlgos/interface/TFileService.h"

#include "TTree.h"

#include <string>

class simpleCICADANtuplizer : public edm::one::EDAnalyzer< edm::one::SharedResources >
{
public:
  explicit simpleCICADANtuplizer(const edm::ParameterSet&);
  ~simpleCICADANtuplizer() override = default;

private:
  void beginJob() override {};
  void analyze(const edm::Event&, const edm::EventSetup&) override;
  void endJob() override {};

  edm::EDGetTokenT< l1t::CICADABxCollection > anomalyToken;
  float anomalyScore;

  edm::Service<TFileService> theFileService;
  TTree* triggerTree;
  unsigned int run;
  unsigned int lumi;
  unsigned int evt;
};

simpleCICADANtuplizer::simpleCICADANtuplizer(const edm::ParameterSet& iConfig):
  anomalyToken( consumes< l1t::CICADABxCollection>(iConfig.getParameter<edm::InputTag>("scoreSource")))
{
  usesResource("TFileService");

  triggerTree = theFileService->make<TTree>("simpleCICADANtuplizerTree","CICADA Score Information");
  triggerTree -> Branch("run", &run);
  triggerTree -> Branch("lumi", &lumi);
  triggerTree -> Branch("evt", &evt);
  triggerTree -> Branch("anomalyScore", &anomalyScore);
}

void simpleCICADANtuplizer::analyze(const edm::Event& iEvent, const edm::EventSetup& iSetup)
{
  edm::Handle< l1t::CICADABxCollection > anomalyHandle;
  iEvent.getByToken(anomalyToken, anomalyHandle);

  run = iEvent.id().run();
  lumi = iEvent.id().luminosityBlock();
  evt = iEvent.id().event();

  anomalyScore = anomalyHandle->at(0, 0);

  triggerTree->Fill();
}

DEFINE_FWK_MODULE(simpleCICADANtuplizer);
