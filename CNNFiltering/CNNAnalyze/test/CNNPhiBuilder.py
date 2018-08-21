import FWCore.ParameterSet.Config as cms

process.load("CNNFiltering.CNNAnalyze.CNNTracksAnalyze_cfi")

process = cms.Process('phikk')

process.load("TrackingTools.TransientTrack.TransientTrackBuilder_cfi")
process.load('Configuration.StandardSequences.GeometryRecoDB_cff')
process.load("Configuration.StandardSequences.MagneticField_AutoFromDBCurrent_cff")
process.load('Configuration.StandardSequences.FrontierConditions_GlobalTag_condDBv2_cff')
process.load("SimTracker.TrackerHitAssociation.tpClusterProducer_cfi")
from Configuration.AlCa.GlobalTag_condDBv2 import GlobalTag
#process.GlobalTag = GlobalTag(process.GlobalTag, '94X_dataRun2_ReReco_EOY17_v1')
#process.GlobalTag = GlobalTag(process.GlobalTag, '94X_dataRun2_ReReco_EOY17_v2') #F
#process.GlobalTag = GlobalTag(process.GlobalTag, '94X_mc2017_realistic_v11')
process.GlobalTag = GlobalTag(process.GlobalTag, par.gtag)

process.options   = cms.untracked.PSet( wantSummary = cms.untracked.bool(True))

process.load("FWCore.MessageLogger.MessageLogger_cfi")
process.MessageLogger.cerr.FwkReport.reportEvery = 500

process.source = cms.Source("PoolSource",
    fileNames = cms.untracked.vstring(input_file)
)

process.maxEvents = cms.untracked.PSet(input = cms.untracked.int32(-1))

charmoniumHLT = [
#Phi
'HLT_DoubleMu2_Jpsi_DoubleTrk1_Phi1p05',
#JPsi
#'HLT_DoubleMu4_JpsiTrkTrk_Displaced',
#'HLT_DoubleMu4_JpsiTrk_Displaced',
#'HLT_DoubleMu4_Jpsi_Displaced',
#'HLT_DoubleMu4_3_Jpsi_Displaced',
#'HLT_Dimuon20_Jpsi_Barrel_Seagulls',
#'HLT_Dimuon25_Jpsi',
]

hltList = charmoniumHLT #muoniaHLT

hltpaths = cms.vstring(hltList)

hltpathsV = cms.vstring([h + '_v*' for h in hltList])

filters = cms.vstring(
                                #HLT_DoubleMu2_Jpsi_DoubleTrk1_Phi
                                #'hltDoubleMu2JpsiDoubleTrkL3Filtered',
                                'hltDoubleTrkmumuFilterDoubleMu2Jpsi',
								'hltJpsiTkTkVertexFilterPhiDoubleTrk1v1',
                                'hltJpsiTkTkVertexFilterPhiDoubleTrk1v2',
								'hltJpsiTkTkVertexFilterPhiDoubleTrk1v3',
								'hltJpsiTkTkVertexFilterPhiDoubleTrk1v4',
								'hltJpsiTrkTrkVertexProducerPhiDoubleTrk1v1',
								'hltJpsiTrkTrkVertexProducerPhiDoubleTrk1v2',
								'hltJpsiTrkTrkVertexProducerPhiDoubleTrk1v3',
								'hltJpsiTrkTrkVertexProducerPhiDoubleTrk1v4',
                                #HLT_DoubleMu4_JpsiTrkTrk_Displaced_v4
                                # 'hltDoubleMu4JpsiDisplacedL3Filtered'
                                # 'hltDisplacedmumuFilterDoubleMu4Jpsi',
                                # 'hltJpsiTkTkVertexFilterPhiKstar',
                                # #HLT_DoubleMu4_JpsiTrk_Displaced_v12
                                # #'hltDoubleMu4JpsiDisplacedL3Filtered',
                                # 'hltDisplacedmumuFilterDoubleMu4Jpsi',
                                # #'hltJpsiTkVertexProducer',
                                # #'hltJpsiTkVertexFilter',
                                # #HLT_DoubleMu4_Jpsi_Displaced
                                # #'hltDoubleMu4JpsiDisplacedL3Filtered',
                                # #'hltDisplacedmumuVtxProducerDoubleMu4Jpsi',
                                # 'hltDisplacedmumuFilterDoubleMu4Jpsi',
                                # #HLT_DoubleMu4_3_Jpsi_Displaced
                                # #'hltDoubleMu43JpsiDisplacedL3Filtered',
                                # 'hltDisplacedmumuFilterDoubleMu43Jpsi',
                                # #HLT_Dimuon20_Jpsi_Barrel_Seagulls
                                # #'hltDimuon20JpsiBarrelnoCowL3Filtered',
                                # 'hltDisplacedmumuFilterDimuon20JpsiBarrelnoCow',
                                # #HLT_Dimuon25_Jpsi
                                # 'hltDisplacedmumuFilterDimuon25Jpsis'
                                )

process.triggerSelection = cms.EDFilter("TriggerResultsFilter",
                                        triggerConditions = cms.vstring(hltpathsV),
                                        hltResults = cms.InputTag( "TriggerResults", "", "HLT" ),
                                        l1tResults = cms.InputTag( "" ),
                                        throw = cms.bool(False)
                                        )

# process.unpackPatTriggers = cms.EDProducer("PATTriggerObjectStandAloneUnpacker",
#   patTriggerObjectsStandAlone = cms.InputTag( 'slimmedPatTrigger' ), #selectedPatTrigger for MC
#   triggerResults              = cms.InputTag( 'TriggerResults::HLT' ),
#   unpackFilterLabels          = cms.bool( True )
# )


process.tracksCNN = cms.EDAnalyzer('CNNTrackDump',
        processName     = cms.string( "generalTracksCNN"),
        tracks          = cms.InputTag( "generalTracks" ),
        tpMap           = cms.InputTag( "tpClusterProducer" ),
        trMap           = cms.InputTag("trackingParticleRecoTrackAsssociation"),
        genMap          = cms.InputTag("TrackAssociatorByChi2"),
        genParticles    = cms.InputTag("genParticles"),
        traParticles    = cms.InputTag("mix","MergedTrackTruth"),
        beamSpot        = cms.InputTag("offlineBeamSpot"),
        infoPileUp      = cms.InputTag("addPileupInfo")
)

process.kaonTracks = cms.EDFilter("MCTruthDeltaRMatcher",
    pdgId = cms.vint32(321),
    src = cms.InputTag("generalTracks"),
    distMin = cms.double(0.25),
    matched = cms.InputTag("genParticleCandidates")
)

process.kaonsCNN = cms.EDAnalyzer('CNNTrackDump',
        processName     = cms.string( "kaonTracksCNN"),
        tracks          = cms.InputTag( "generalTracks" ),
        tpMap           = cms.InputTag( "tpClusterProducer" ),
        trMap           = cms.InputTag("trackingParticleRecoTrackAsssociation"),
        genMap          = cms.InputTag("TrackAssociatorByChi2"),
        genParticles    = cms.InputTag("genParticles"),
        traParticles    = cms.InputTag("mix","MergedTrackTruth"),
        beamSpot        = cms.InputTag("offlineBeamSpot"),
        infoPileUp      = cms.InputTag("addPileupInfo")
)

process.phitokk = cms.EDAnalyzer('DiTrack',
         tracks             = cms.string( "generalTracks"),
         TrakTrakMassCuts   = cms.vdouble(1.0,1.04),
         MassTraks          = cms.vdouble(kaonmass,kaonmass)
         )


process.p = cms.Path(process.triggerSelection * process.kaonTracks * process.tracksCNN * process.kaonsCNN * process.phitokk)

#CNNTrackSequence = cms.Sequence(tracksCNN)
