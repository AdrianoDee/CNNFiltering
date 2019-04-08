# Auto generated configuration file
# using: 
# Revision: 1.19 
# Source: /local/reps/CMSSW/CMSSW/Configuration/Applications/python/ConfigBuilder.py,v 
# with command line options: step3 --conditions 102X_upgrade2018_design_v9 --pileup_input das:/RelValMinBias_13/CMSSW_10_2_0-102X_upgrade2018_design_v6_gcc7-v1/GEN-SIM -n 10 --era Run2_2018 --eventcontent RECOSIM,DQM --runUnscheduled -s RAW2DIGI,RECO:reconstruction_trackingOnly,VALIDATION:@trackingOnlyValidation,DQM:@trackingOnlyDQM --datatier GEN-SIM-RECO,DQMIO --geometry DB:Extended --filein file:step2.root --fileout file:step3.root --no_exec --pileup AVE_50_BX_25ns --mc
import FWCore.ParameterSet.Config as cms
from FWCore.ParameterSet.VarParsing import VarParsing
from Configuration.StandardSequences.Eras import eras

process = cms.Process('RECOPatatrack',eras.Run2_2018)

# import of standard configurations
process.load('Configuration.StandardSequences.Services_cff')
process.load('SimGeneral.HepPDTESSource.pythiapdt_cfi')
process.load('FWCore.MessageService.MessageLogger_cfi')
process.load('Configuration.EventContent.EventContent_cff')
process.load('SimGeneral.MixingModule.mix_POISSON_average_cfi')
process.load('Configuration.StandardSequences.GeometryRecoDB_cff')
process.load('Configuration.StandardSequences.MagneticField_cff')
process.load('Configuration.StandardSequences.RawToDigi_cff')
process.load('Configuration.StandardSequences.Reconstruction_cff')
process.load('Configuration.StandardSequences.Validation_cff')
process.load('DQMOffline.Configuration.DQMOfflineMC_cff')
process.load('Configuration.StandardSequences.FrontierConditions_GlobalTag_cff')
process.load('CNNFiltering.CNNAnalyze.CNNAnalyze_cfi')

options = VarParsing ('analysis')
options.register ('pileUp',50,VarParsing.multiplicity.singleton,VarParsing.varType.int,"Pileup")
options.register ('skipEvent',0,VarParsing.multiplicity.singleton,VarParsing.varType.int,"Skip Events")
options.register ('numEvents',100,VarParsing.multiplicity.singleton,VarParsing.varType.int,"Max Events")
options.register ('numFile',2,VarParsing.multiplicity.singleton,VarParsing.varType.int,"File Number")
options.parseArguments()

fileName = 'file:step3_opendata_ttbar_' + str(options.numEvents) + '_pu_' + str(options.pileUp) + '_skip_' + str(options.skipEvent) + '_file_' + str(options.numFile)

allfiles = [
"file:/lustre/cms/store/user/adiflori/OpenData/opendatas/017E1C89-5F83-D94E-8FE2-93CBE7369C11.root",
"file:/lustre/cms/store/user/adiflori/OpenData/opendatas/18987864-CE50-9341-88A2-47FD4ACD1AE9.root",
"file:/lustre/cms/store/user/adiflori/OpenData/opendatas/2EE8FE78-2D94-A642-B040-395425C4B375.root",
"file:/lustre/cms/store/user/adiflori/OpenData/opendatas/3C46FFB5-F658-BF41-B0D4-D7871A4A3B3A.root",
"file:/lustre/cms/store/user/adiflori/OpenData/opendatas/6F37A259-87EC-1649-ADFE-629C9A2760F6.root",
"file:/lustre/cms/store/user/adiflori/OpenData/opendatas/7541414E-9DEC-8B47-80AE-BE56559E863B.root",
"file:/lustre/cms/store/user/adiflori/OpenData/opendatas/A4375E15-0D11-5E48-8731-3AF905396769.root",
"file:/lustre/cms/store/user/adiflori/OpenData/opendatas/AE74C057-F9B6-9D4C-BE3D-3DD43D64B34E.root",
"file:/lustre/cms/store/user/adiflori/OpenData/opendatas/E14A7EB4-BE89-F044-9DAF-E8B143EFBACB.root"
]


process.Timing = cms.Service("Timing",
  summaryOnly = cms.untracked.bool(False),
  useJobReport = cms.untracked.bool(True)
)

process.maxEvents = cms.untracked.PSet(
        input = cms.untracked.int32(options.numEvents),
)

# Input source
process.source = cms.Source("PoolSource",
    fileNames = cms.untracked.vstring(allfiles[options.numFile]),
    secondaryFileNames = cms.untracked.vstring(),
    skipEvents=cms.untracked.uint32(options.skipEvent),
)



process.options = cms.untracked.PSet(

)

# Production Info
process.configurationMetadata = cms.untracked.PSet(
    annotation = cms.untracked.string('step3 nevts:10'),
    name = cms.untracked.string('Applications'),
    version = cms.untracked.string('$Revision: 1.19 $')
)

# Output definition

process.RECOSIMoutput = cms.OutputModule("PoolOutputModule",
    dataset = cms.untracked.PSet(
        dataTier = cms.untracked.string('GEN-SIM-RECO'),
        filterName = cms.untracked.string('')
    ),
    fileName = cms.untracked.string(fileName + '.root'),
    outputCommands = process.RECOSIMEventContent.outputCommands,
    splitLevel = cms.untracked.int32(0)
)

process.DQMoutput = cms.OutputModule("DQMRootOutputModule",
    dataset = cms.untracked.PSet(
        dataTier = cms.untracked.string('DQMIO'),
        filterName = cms.untracked.string('')
    ),
    fileName = cms.untracked.string(fileName + '_inDQM.root'),
    outputCommands = process.DQMEventContent.outputCommands,
    splitLevel = cms.untracked.int32(0)
)

# Additional output definition

# Other statements
process.mix.input.nbPileupEvents.averageNumber = cms.double(50.000000)
process.mix.bunchspace = cms.int32(25)
process.mix.minBunch = cms.int32(-3)
process.mix.maxBunch = cms.int32(3)

process.mix.playback = True
process.mix.digitizers = cms.PSet()
for a in process.aliases: delattr(process, a)
process.RandomNumberGeneratorService.restoreStateLabel=cms.untracked.string("randomEngineStateProducer")
from Configuration.AlCa.GlobalTag import GlobalTag
process.GlobalTag = GlobalTag(process.GlobalTag, '102X_upgrade2018_design_v9', '')

# Path and EndPath definitions
process.raw2digi_step = cms.Path(process.RawToDigi)
process.reconstruction_step = cms.Path(process.reconstruction_trackingOnly)
process.prevalidation_step = cms.Path(process.globalPrevalidationTrackingOnly)
process.validation_step = cms.EndPath(process.globalValidationTrackingOnly)
process.dqmoffline_step = cms.EndPath(process.DQMOfflineTracking)
process.dqmofflineOnPAT_step = cms.EndPath(process.PostDQMOffline)
process.RECOSIMoutput_step = cms.EndPath(process.RECOSIMoutput)
process.DQMoutput_step = cms.EndPath(process.DQMoutput)
process.CNN_step = cms.Path(process.CNNDoubletsSequence)

# Schedule definition
process.schedule = cms.Schedule(process.raw2digi_step,process.reconstruction_step,process.prevalidation_step,process.validation_step,process.dqmoffline_step,process.dqmofflineOnPAT_step,process.RECOSIMoutput_step,process.DQMoutput_step,process.CNN_step)
from PhysicsTools.PatAlgos.tools.helpers import associatePatAlgosToolsTask
associatePatAlgosToolsTask(process)

# customisation of the process.

# Automatic addition of the customisation function from SimGeneral.MixingModule.fullMixCustomize_cff
from SimGeneral.MixingModule.fullMixCustomize_cff import setCrossingFrameOn 

#call to customisation function setCrossingFrameOn imported from SimGeneral.MixingModule.fullMixCustomize_cff
process = setCrossingFrameOn(process)

# End of customisation functions
#do not add changes to your config after this point (unless you know what you are doing)
from FWCore.ParameterSet.Utilities import convertToUnscheduled
process=convertToUnscheduled(process)


# Customisation from command line

#Have logErrorHarvester wait for the same EDProducers to finish as those providing data for the OutputModule
from FWCore.Modules.logErrorHarvester_cff import customiseLogErrorHarvesterUsingOutputCommands
process = customiseLogErrorHarvesterUsingOutputCommands(process)

# Add early deletion of temporary data products to reduce peak memory need
from Configuration.StandardSequences.earlyDeleteSettings_cff import customiseEarlyDelete
process = customiseEarlyDelete(process)
# End adding early deletion
