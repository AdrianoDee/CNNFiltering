import FWCore.ParameterSet.Config as cms

charmoniumHLT = [
#Phi
'HLT_DoubleMu2_Jpsi_DoubleTrk1_Phi1p05',
#JPsi
'HLT_DoubleMu4_JpsiTrkTrk_Displaced',
'HLT_DoubleMu4_JpsiTrk_Displaced',
#vi'HLT_DoubleMu4_Jpsi_Displaced',
#'HLT_DoubleMu4_3_Jpsi_Displaced',
#'HLT_Dimuon20_Jpsi_Barrel_Seagulls',
#'HLT_Dimuon25_Jpsi',
]

hltList = charmoniumHLT #muoniaHLT

hltpaths = cms.vstring(hltList)

hltpathsV = cms.vstring([h + '_v*' for h in hltList])

kaonmass = 0.493677

phitokk = cms.EDAnalyzer('DiTrack',
         seqNumber          = cms.int32(0),
         tpMap              = cms.InputTag( "tpClusterProducer" ),
         Tracks             = cms.InputTag( "generalTracks"),
		 Trigger 		 	= cms.InputTag("TriggerResults","","RECO"),
         TrakTrakMassCuts   = cms.vdouble(1.0,1.04),
         MassTraks          = cms.vdouble(kaonmass,kaonmass),
         HLTs               = hltpaths
         )

CNNDiTrackSequence = cms.Sequence(phitokk)


#CNNTrackSequence = cms.Sequence(tracksCNN)
