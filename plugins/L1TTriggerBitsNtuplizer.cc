//system include files
#include <memory>
#include <iostream>

//user includes
#include "FWCore/Framework/interface/Frameworkfwd.h"
#include "FWCore/Framework/interface/one/EDAnalyzer.h"
#include "FWCore/Framework/interface/Event.h"
#include "FWCore/Framework/interface/EventSetup.h"
#include "FWCore/Framework/interface/MakerMacros.h"
#include "FWCore/Framework/interface/ConsumesCollector.h"
#include "FWCore/ParameterSet/interface/ParameterSet.h"
#include "FWCore/Utilities/interface/InputTag.h"
#include "FWCore/Utilities/interface/ESGetToken.h"
#include "FWCore/ServiceRegistry/interface/Service.h"

#include "CondFormats/DataRecord/interface/L1TUtmTriggerMenuRcd.h"
#include "CondFormats/L1TObjects/interface/L1TUtmTriggerMenu.h"

#include "CondFormats/L1TObjects/interface/L1TGlobalPrescalesVetos.h"
#include "CondFormats/DataRecord/interface/L1TGlobalPrescalesVetosRcd.h"

#include "CondFormats/L1TObjects/interface/L1TGlobalPrescalesVetosFract.h"
#include "CondFormats/DataRecord/interface/L1TGlobalPrescalesVetosFractRcd.h"

#include "CommonTools/UtilAlgos/interface/TFileService.h"

#include "DataFormats/L1TGlobal/interface/GlobalAlgBlk.h"

#include "TTree.h"

#include <string>
#include <map>

class L1TTriggerBitsNtuplizer : public edm::one::EDAnalyzer< edm::one::SharedResources >
{
public:
  explicit L1TTriggerBitsNtuplizer(const edm::ParameterSet&);
  ~L1TTriggerBitsNtuplizer() override;

private: 
  void beginJob() override {};
  void analyze(const edm::Event&, const edm::EventSetup&) override;
  void endJob() override; 

  edm::Service<TFileService> theFileService;
  TTree* l1BitsTree;
                  
  edm::ESGetToken<L1TUtmTriggerMenu, L1TUtmTriggerMenuRcd> L1TUtmTriggerMenuEventToken;
  edm::ESGetToken<L1TGlobalPrescalesVetos, L1TGlobalPrescalesVetosRcd> L1TGlobalPrescalesVetoToken;
  edm::ESGetToken<L1TGlobalPrescalesVetosFract, L1TGlobalPrescalesVetosFractRcd> L1TGlobalPrescalesVetoFractToken;
  const L1TUtmTriggerMenu* l1GtMenu;
  const std::map<std::string, L1TUtmAlgorithm>* algorithmMap;
  //bool L1_SingleMu22;
  //bool L1_SingleJet180;
  //bool L1_HTTer450;
  //bool L1_ZeroBias;

  unsigned int run;
  unsigned int lumi;
  unsigned int evt;
  unsigned int nBits = 0;
  std::string menuName;

  std::map< string, std::unique_ptr<bool> > triggerResults;
  std::map< string, std::unique_ptr<double> > prescaleResults;

  bool verboseDebug;

  // access to the results block from uGT
  edm::EDGetTokenT< BXVector<GlobalAlgBlk> > gtAlgBlkToken;
  edm::Handle< BXVector<GlobalAlgBlk> > gtAlgBlkHandle;
};

L1TTriggerBitsNtuplizer::L1TTriggerBitsNtuplizer(const edm::ParameterSet& iConfig):
  gtAlgBlkToken( consumes< BXVector<GlobalAlgBlk> >(iConfig.getParameter< edm::InputTag >("gtResults")) )
{
  usesResource("TFileService");
  L1TUtmTriggerMenuEventToken = consumesCollector().esConsumes<L1TUtmTriggerMenu, L1TUtmTriggerMenuRcd>();
  L1TGlobalPrescalesVetoToken = consumesCollector().esConsumes<L1TGlobalPrescalesVetos, L1TGlobalPrescalesVetosRcd>();
  L1TGlobalPrescalesVetoFractToken = consumesCollector().esConsumes<L1TGlobalPrescalesVetosFract, L1TGlobalPrescalesVetosFractRcd>();
  
  verboseDebug = iConfig.exists("verboseDebug") ? iConfig.getParameter<bool>("verboseDebug"): false;

  //setup the bits tree
  l1BitsTree = theFileService->make<TTree>("L1TTriggerBits","Emulator L1 Trigger Bits");
  l1BitsTree -> Branch("run", &run);
  l1BitsTree -> Branch("lumi", &lumi);
  l1BitsTree -> Branch("evt", &evt);
  l1BitsTree -> Branch("menuName", &menuName);
}

L1TTriggerBitsNtuplizer::~L1TTriggerBitsNtuplizer()

{
}

void L1TTriggerBitsNtuplizer::analyze(const edm::Event& iEvent, const edm::EventSetup& iSetup)
{
  using namespace edm;

  auto prescaleRcd = iSetup.get<L1TGlobalPrescalesVetosRcd>();
  auto prescales = prescaleRcd.get(L1TGlobalPrescalesVetoToken);

  auto fractPrescaleRcd = iSetup.get<L1TGlobalPrescalesVetosFractRcd>();
  auto fractPrescales = fractPrescaleRcd.get(L1TGlobalPrescalesVetoFractToken);
  if (verboseDebug)
  {
    int outputPrescaleColumn = 0;
    std::cout<<"Prescales table size: "<<prescales.prescale_table_.size()<<std::endl;
    for (auto row: prescales.prescale_table_)
    {
      std::cout<<"Prescale column: "<<outputPrescaleColumn<<std::endl;
      outputPrescaleColumn++;
      std::cout<<"Row size: "<<row.size()<<std::endl;
      for (auto tableEntry: row)
      {
        std::cout<<tableEntry<<" ";
      }
      std::cout<<" "<<std::endl;
    }

    int outputFractPrescaleColumn = 0;
    std::cout<<"Fract Prescales table size: "<<fractPrescales.prescale_table_.size()<<std::endl;
    for (auto row: fractPrescales.prescale_table_)
    {
      std::cout<<"Prescale column: "<<outputFractPrescaleColumn<<std::endl;
      outputFractPrescaleColumn++;
      std::cout<<"Row size: "<<row.size()<<std::endl;
      for (double tableEntry: row)
      {
        std::cout<<tableEntry<<" ";
      }
      std::cout<<" "<<std::endl;
    }

  }
  
  run  = iEvent.id().run();
  lumi = iEvent.id().luminosityBlock();
  evt  = iEvent.id().event();

  //Start the laborious process of accessing the in-event L1 Menu
  auto menuRcd = iSetup.get<L1TUtmTriggerMenuRcd>();
  l1GtMenu = &menuRcd.get(L1TUtmTriggerMenuEventToken);
  menuName = l1GtMenu->getName();
  if (verboseDebug)
    std::cout<<"Menu name: "<<menuName<<std::endl;
  algorithmMap = &(l1GtMenu->getAlgorithmMap());

  //Okay. What we would like to do, is on the 
  //First event, get all of the L1 path names
  //Assign variables for them
  //So use a map of L1Name and a pointer to a boolean.
  //We can provide the pointer to the branching function

  iEvent.getByToken(gtAlgBlkToken, gtAlgBlkHandle);
  //First we make sure we even have a valid AlgBlk, otherwise we are going nowhere fast
  if(gtAlgBlkHandle.isValid())
    {
      //Want BX0? Even the l1GtUtils doesn't seem sure about this?
      std::vector<GlobalAlgBlk>::const_iterator algBlk = gtAlgBlkHandle->begin(0);
      //
      if(verboseDebug)
	    {
        std::cout<<"BxVector First BX: "<<gtAlgBlkHandle->getFirstBX()<<std::endl;
        std::cout<<"BxVector Last BX: "<<gtAlgBlkHandle->getLastBX()<<std::endl;
	    }
      if(algBlk != gtAlgBlkHandle->end(0))
      {
        //Now let's see if we can loop over the bits in the path and see if we can print out
        //The names of the L1 seeds
        if(verboseDebug)
        {
          std::cout<<"Ntuplizing specific algorithms"<<std::endl;
        }
        for (std::map<std::string, L1TUtmAlgorithm>::const_iterator itAlgo = algorithmMap->begin();
            itAlgo != algorithmMap->end();
            itAlgo++) 
          {
            std::string algName = itAlgo->first;
            int algBit = itAlgo->second.getIndex();
            int prescaleColumn = algBlk->getPreScColumn();
            bool initialDecision = algBlk->getAlgoDecisionInitial(algBit);
            bool intermDecision = algBlk->getAlgoDecisionInterm(algBit);
            bool decisionFinal = algBlk->getAlgoDecisionFinal(algBit);

            if (verboseDebug)
            {
              std::cout<<"L1 Path: "<<algName<<" Bit: "<<algBit<<" Initial Decision: "<<initialDecision<<" Interm Decision: "<<intermDecision<<" Final Decision: "<<decisionFinal<<std::endl;
              std::cout<<"Prescale Column retrieval: "<<prescaleColumn<<std::endl;
            }

            int prescaleValue = fractPrescales.prescale_table_.at(prescaleColumn).at(algBit);

            if (verboseDebug) std::cout<<"Prescale: "<<prescaleValue<<std::endl;

            if(triggerResults.find(algName) == triggerResults.end())
            {
              //The algorithm name was not in the triger map record.
              //Create a new map entry for this bit
              triggerResults.insert(std::pair< string, std::unique_ptr<bool> >(algName, std::make_unique<bool>()));
              l1BitsTree->Branch(algName.c_str(), triggerResults[algName].get(), (algName+"/O").c_str()); //this feels like it shouldn't work

              //let's insert a prescale column as well
              prescaleResults.insert(std::pair< string, std::unique_ptr<double> >(algName, std::make_unique<double>()));
              l1BitsTree->Branch((algName+"_prescale").c_str(), prescaleResults[algName].get(), (algName+"_prescale/D").c_str());
              nBits++;
            }
            *triggerResults[algName] = decisionFinal;
            *prescaleResults[algName] = prescaleValue;

            if (verboseDebug) std::cout<<"trigger result: "<<triggerResults[algName].get()<<" "<<*triggerResults[algName]<<std::endl; 

          }

      }
          else
      {
        std::cout<<"algBlk was found to be the end of the BX vector (empty)!"<<std::endl;
      }
    }
  else
    {
      std::cout<<"Found Invalid AlgBlk!"<<std::endl;
    }
  l1BitsTree->Fill();
}

void L1TTriggerBitsNtuplizer::endJob()
{
  if (verboseDebug)
    std::cout<<"Number of bits ntuplized: "<<nBits<<std::endl;
}

DEFINE_FWK_MODULE(L1TTriggerBitsNtuplizer);
