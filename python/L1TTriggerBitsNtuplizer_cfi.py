#Settings for the L1Trigger bit ntuplizer

import FWCore.ParameterSet.Config as cms

L1TTriggerBitsNtuplizer = cms.EDAnalyzer('L1TTriggerBitsNtuplizer',
                                         gtResults    = cms.InputTag("gtStage2Digis"),
                                         verboseDebug = cms.bool(False),
)
