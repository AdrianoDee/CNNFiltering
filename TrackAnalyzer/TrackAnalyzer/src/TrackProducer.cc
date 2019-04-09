#include "../interface/TrackProducer.h"

//Headers for the data items
#include <DataFormats/TrackReco/interface/TrackFwd.h>
#include <DataFormats/TrackReco/interface/Track.h>
#include <DataFormats/MuonReco/interface/MuonFwd.h>
#include <DataFormats/MuonReco/interface/Muon.h>
#include <DataFormats/Common/interface/View.h>
#include <DataFormats/HepMCCandidate/interface/GenParticle.h>
#include <DataFormats/PatCandidates/interface/Muon.h>
#include <DataFormats/VertexReco/interface/VertexFwd.h>
#include "DataFormats/Math/interface/deltaR.h"

//Headers for services and tools
#include "TrackingTools/TransientTrack/interface/TransientTrackBuilder.h"
#include "TrackingTools/Records/interface/TransientTrackRecord.h"

#include "RecoVertex/KalmanVertexFit/interface/KalmanVertexFitter.h"
#include "RecoVertex/VertexTools/interface/VertexDistanceXY.h"
#include "RecoVertex/KinematicFit/interface/KinematicParticleVertexFitter.h"
#include "RecoVertex/KinematicFit/interface/KinematicParticleFitter.h"
#include "RecoVertex/KinematicFit/interface/MassKinematicConstraint.h"
#include "RecoVertex/KinematicFitPrimitives/interface/RefCountedKinematicParticle.h"
#include "RecoVertex/KinematicFitPrimitives/interface/TransientTrackKinematicParticle.h"
#include "RecoVertex/KinematicFitPrimitives/interface/KinematicParticleFactoryFromTransientTrack.h"

#include "TMath.h"
#include "Math/VectorUtil.h"
#include "TVector3.h"
#include "../interface/DiMuonVtxReProducer.h"
#include "TLorentzVector.h"

#include <iostream>
#include <string>
#include <fstream>

#include "MagneticField/Engine/interface/MagneticField.h"
#include "MagneticField/Records/interface/IdealMagneticFieldRecord.h"
#include "TrackingTools/PatternTools/interface/TwoTrackMinimumDistance.h"
#include "TrackingTools/IPTools/interface/IPTools.h"
#include "TrackingTools/PatternTools/interface/ClosestApproachInRPhi.h"
#include "DataFormats/TrackerRecHit2D/interface/BaseTrackerRecHit.h"

TrackProducerPAT::TrackProducerPAT(const edm::ParameterSet& iConfig):
TrackGenMap_(consumes<edm::Association<reco::GenParticleCollection>>(iConfig.getParameter<edm::InputTag>("TrackMatcher"))),
/*muons_(consumes<edm::View<pat::Muon>>(iConfig.getParameter<edm::InputTag>("muons"))),
muonPtCut_(iConfig.existsAs<double>("MuonPtCut") ? iConfig.getParameter<double>("MuonPtCut") : 0.7),
thebeamspot_(consumes<reco::BeamSpot>(iConfig.getParameter<edm::InputTag>("beamSpotTag"))),

TriggerCollection_(consumes<std::vector<pat::TriggerObjectStandAlone>>(iConfig.getParameter<edm::InputTag>("TriggerInput"))),
triggerResults_Label(consumes<edm::TriggerResults>(iConfig.getParameter<edm::InputTag>("TriggerResults"))),
dimuonSelection_(iConfig.existsAs<std::string>("dimuonSelection") ? iConfig.getParameter<std::string>("dimuonSelection") : ""),
addCommonVertex_(iConfig.getParameter<bool>("addCommonVertex")),
addMuonlessPrimaryVertex_(iConfig.getParameter<bool>("addMuonlessPrimaryVertex")),
resolveAmbiguity_(iConfig.getParameter<bool>("resolvePileUpAmbiguity")),
addMCTruth_(iConfig.getParameter<bool>("addMCTruth")),
HLTFilters_(iConfig.getParameter<std::vector<std::string>>("HLTFilters"))
*/
TrakCollection_(consumes<edm::View<pat::PackedCandidate>>(iConfig.getParameter<edm::InputTag>("PFCandidates"))),
Muons_(consumes<edm::View<pat::Muon>>(iConfig.getParameter<edm::InputTag>("muons")))
{
  produces<pat::CompositeCandidateCollection>();

}


TrackProducerPAT::~TrackProducerPAT()
{

  // do anything here that needs to be done at desctruction time
  // (e.g. close files, deallocate resources etc.)

}


// ------------ method called to produce the data  ------------

void
TrackProducerPAT::produce(edm::Event& iEvent, const edm::EventSetup& iSetup)
{
  using namespace edm;
  using namespace std;
  using namespace reco;
  typedef Candidate::LorentzVector LorentzVector;
  std::cout<<"Track CNN Analyzer - "<<std::endl;

  std::unique_ptr<pat::CompositeCandidateCollection> oniaOutput(new pat::CompositeCandidateCollection);

  edm::Handle<edm::Association<reco::GenParticleCollection>> theGenMap;
  iEvent.getByToken(TrackGenMap_,theGenMap);

  int eveNumber = iEvent.id().event();
  int runNumber = iEvent.id().run();
  int lumNumber = iEvent.id().luminosityBlock();


  std::string fileName = "tracks/" + std::to_string(lumNumber) +"_"+std::to_string(runNumber) +"_"+std::to_string(eveNumber);
  fileName += "_PF_DNNTracks.txt";
  std::ofstream outTrack(fileName, std::ofstream::app);
  ///// DATA STRUCTURE
  // std::vector < std::array <float,8> >  hitCoords_;
  // //n x y z r phi ax1 ax2
  // std::vector < std::array <float,13> > pixelInfos_;
  // //c_x c_y size size_x size_y charge ovfx ovfy ratio isBig isBad isOnEdge id
  // std::vector < std::array <float,20> > pixelADC_, pixelADCx_, pixelADCy_;
  //
  // std::vector < std::array <float,7> > stripInfos_;
  // //dimension first ampsize charge merged splitClusterError id
  // std::vector < std::array <float,20> > stripADC_;

  edm::Handle<edm::View<pat::PackedCandidate> > track;
  iEvent.getByToken(TrakCollection_,track);

  edm::Handle< View<pat::Muon> > muons;
  iEvent.getByToken(Muons_,muons);

  std::cout << "Tracks" << std::endl;

  int muonTracks = 0;

  for (size_t i = 0; i < track->size(); i++) {

    auto t = track->at(i);
    if(!(t.trackHighPurity())) continue;
    if(!(t.hasTrackDetails())) continue;


    auto refTrack = track->refAt(i);
    int pdgId = -9999, momPdgId = -9999;

    if(theGenMap->contains(refTrack.id()))
    {
      if(((*theGenMap)[edm::Ref<edm::View<pat::PackedCandidate>>(track, i)]).isNonnull())
      {
        auto genParticle = ((*theGenMap)[edm::Ref<edm::View<pat::PackedCandidate>>(track, i)]);
        // auto genParticle = dynamic_cast <const reco::GenParticle *>(((*theGenMap)[edm::Ref<edm::View<pat::PackedCandidate>>(track, i)]));
        pdgId = genParticle->pdgId();
        if(genParticle->numberOfMothers()>0)
        if(genParticle->motherRef().isNonnull())
        momPdgId = genParticle->motherRef()->pdgId();
      }
    }

    if(pdgId>-9999)
    {
      if(pdgId == 13 || pdgId == -13) muonTracks++;
      std::cout << i << " -> ";

      std::cout << t.hitCoords_.size() << " - ";
      std::cout << t.pixelInfos_.size() << " - ";
      std::cout << t.pixelADC_.size() << " - ";
      std::cout << t.pixelADCx_.size() << " - ";
      std::cout << t.pixelADCy_.size() << " - ";
      std::cout << t.stripInfos_.size() << " - ";
      std::cout << t.stripADC_.size() << " - ";

      std::cout << pdgId << " - " << momPdgId << std::endl;
    }


    int noHits = t.hitCoords_.size();
    int maxHits = 25;
    int minHits = -std::max(maxHits,noHits);
    int featureSize = t.pixelInfos_[0].size() + t.pixelADC_[0].size() + t.pixelADCx_[0].size() + t.pixelADCy_[0].size() + t.stripInfos_[0].size() + t.stripADC_[0].size();

    // assert(t.hitCoords_.size() == t.pixelInfos_.size());
    // assert(t.hitCoords_.size() == t.pixelADC_.size());
    // assert(t.hitCoords_.size() == t.pixelADCx_.size());
    // assert(t.hitCoords_.size() == t.pixelADCy_.size());
    // assert(t.hitCoords_.size() == t.stripInfos_.size());
    // assert(t.hitCoords_.size() == t.stripADC_.size());

    for(int j = 0; j<minHits;j++)
    {
      auto coords  = t.hitCoords_[j];
      auto pixinf  = t.pixelInfos_[j];
      auto pixadc  = t.pixelADC_[j];
      auto pixadx  = t.pixelADCx_[j];
      auto pixady  = t.pixelADCy_[j];
      auto strinf  = t.stripInfos_[j];
      auto stradc  = t.stripADC_[j];

      for (auto const &x : coords)
      outTrack << x << "\t";
      for (auto const &x : pixinf)
      outTrack << x << "\t";
      for (auto const &x : pixadc)
      outTrack << x << "\t";
      for (auto const &x : pixadx)
      outTrack << x << "\t";
      for (auto const &x : pixady)
      outTrack << x << "\t";
      for (auto const &x : strinf)
      outTrack << x << "\t";
      for (auto const &x : stradc)
      outTrack << x << "\t";

    }

    for(int j = minHits; j<maxHits;j++)
    for (int i = 0; i < featureSize; i++)
    outTrack << -9999.0 << "\t";

    outTrack << 5421369 << std::endl;


  }
  int muonCounter = 0;
  for (size_t i = 0; i < muons->size(); i++) {

      auto m = muons->at(i);
      // both must pass low quality
      if (!(m.track().isNonnull())) continue;
      if (!(m.innerTrack().isNonnull())) continue;

      muonCounter++;
  }

  std::cout << "Track muon pdg   : " << muonTracks << std::endl;
  std::cout << "Muons with track : " << muonCounter << std::endl;


  /*
  edm::Handle< edm::TriggerResults > triggerResults_handle;
  iEvent.getByToken( triggerResults_Label , triggerResults_handle);

  const edm::TriggerNames & names = iEvent.triggerNames( *triggerResults_handle );


  Vertex thePrimaryV;
  Vertex theBeamSpotV;

  edm::ESHandle<MagneticField> magneticField;
  iSetup.get<IdealMagneticFieldRecord>().get(magneticField);
  const MagneticField* field = magneticField.product();

  edm::Handle<BeamSpot> theBeamSpot;
  iEvent.getByToken(thebeamspot_,theBeamSpot);
  BeamSpot bs = *theBeamSpot;
  theBeamSpotV = Vertex(bs.position(), bs.covariance3D());

  edm::Handle<VertexCollection> priVtxs;
  iEvent.getByToken(thePVs_, priVtxs);
  if ( priVtxs->begin() != priVtxs->end() ) {
  thePrimaryV = Vertex(*(priVtxs->begin()));
}
else {
thePrimaryV = Vertex(bs.position(), bs.covariance3D());
}

edm::Handle< View<pat::Muon> > muons;
iEvent.getByToken(muons_,muons);

edm::Handle<std::vector<pat::TriggerObjectStandAlone>> trig;
iEvent.getByToken(TriggerCollection_,trig);

edm::ESHandle<TransientTrackBuilder> theTTBuilder;
iSetup.get<TransientTrackRecord>().get("TransientTrackBuilder",theTTBuilder);
KalmanVertexFitter vtxFitter(true);
TrackCollection muonLess;

ParticleMass muon_mass = 0.10565837; //pdg mass
//ParticleMass psi_mass = 3.096916;
float muon_sigma = muon_mass*1.e-6;

std::map<size_t,UInt_t> muonFilters;
std::map<size_t,double> muonDeltaR,muonDeltaPt;
pat::TriggerObjectStandAloneCollection filteredColl;
std::map<size_t,pat::TriggerObjectStandAlone> matchedColl;
//pat::TriggerObjectStandAloneCollection triggerColl;
std::vector < UInt_t > filterResults;

//std::cout << "Debug  1" << std::endl;

for ( size_t iTrigObj = 0; iTrigObj < trig->size(); ++iTrigObj ) {

pat::TriggerObjectStandAlone unPackedTrigger( trig->at( iTrigObj ) );

unPackedTrigger.unpackPathNames( names );
unPackedTrigger.unpackFilterLabels(iEvent,*triggerResults_handle);

bool filtered = false;
UInt_t thisFilter = 0;

for (size_t i = 0; i < HLTFilters_.size(); i++)
if(unPackedTrigger.hasFilterLabel(HLTFilters_[i]))
{
thisFilter += (1<<i);
filtered = true;
}

if(filtered)
{
filteredColl.push_back(unPackedTrigger);
filterResults.push_back(thisFilter);
}
}

//std::cout << "Debug  2" << std::endl;

for (size_t i = 0; i < muons->size(); i++) {

auto t = muons->at(i);

bool matched = false;
for (std::vector<pat::TriggerObjectStandAlone>::const_iterator trigger = filteredColl.begin(), triggerEnd=filteredColl.end(); trigger!= triggerEnd; ++trigger)
for ( size_t iTrigObj = 0; iTrigObj < filteredColl.size(); ++iTrigObj )
{
auto thisTrig = filteredColl.at(iTrigObj);
if(MatchByDRDPt(t,filteredColl[iTrigObj]))
{
if(matched)
{
if(muonDeltaR[i] > DeltaR(t,thisTrig))
{
muonFilters[i] = filterResults[iTrigObj];
matchedColl[i] = thisTrig;
muonDeltaR[i] = fabs(DeltaR(t,thisTrig));
muonDeltaPt[i] = fabs(DeltaPt(t,thisTrig));
}
}else
{
muonFilters[i] = filterResults[iTrigObj];
matchedColl[i] = thisTrig;
muonDeltaR[i] = fabs(DeltaR(t,thisTrig));
muonDeltaPt[i] = fabs(DeltaPt(t,thisTrig));
}

matched = true;
}
}
if(!matched)
{
muonFilters[i] = 0;
muonDeltaR[i] = -1.0;
muonDeltaPt[i] = -1.0;
}

}
//std::cout << "Debug  3" << std::endl;

//for(View<pat::Muon>::const_iterator m = muons->begin(), itend = muons->end(); m != itend; ++m)
// for (size_t i = 0; i < muons->size(); i++)
// {
//   auto m = muons->at(i);
//   UInt_t M = isTriggerMatched(m);
//   muonFilters[i] = M;
//   if(M > 0)
//     matchedColl[i] = BestTriggerMuon(m);
//
// }

//std::cout << "Debug  4" << std::endl;
// MuMu candidates only from muons
//for(View<pat::Muon>::const_iterator it = muons->begin(), itend = muons->end(); it != itend; ++it){
for (size_t i = 0; i < muons->size(); i++) {

auto m1 = muons->at(i);
// both must pass low quality
if (!(m1.track().isNonnull())) continue;
if (!(m1.innerTrack().isNonnull())) continue;
if (!(m1.track()->pt()>muonPtCut_)) continue;
// if(!lowerPuritySelection_(*it)) continue;
//std::cout << "First muon quality flag" << std::endl;
for (size_t j = i+1; j < muons->size(); j++) {

auto m2 = muons->at(j);
if(i == j) continue;
// both must pass low quality
// if(!lowerPuritySelection_(*it2)) continue;
//std::cout << "Second muon quality flag" << std::endl;
// one must pass tight quality
// if (!(higherPuritySelection_(*it) || higherPuritySelection_(*it2))) continue;

// ---- fit vertex using Tracker tracks (if they have tracks) ----
if (!(m2.track().isNonnull())) continue;
if (!(m2.innerTrack().isNonnull())) continue;
if (!(m2.track()->pt()>muonPtCut_)) continue;

pat::CompositeCandidate mumucand;
vector<TransientVertex> pvs;
// std::cout << "Debug  5" << std::endl;
// ---- no explicit order defined ----
if(m1.pt() > m2.pt())
{
mumucand.addDaughter(m1, "highMuon");
mumucand.addDaughter(m2,"lowMuon");
} else
{
mumucand.addDaughter(m1, "lowMuon");
mumucand.addDaughter(m2,"highMuon");
}

if(m1.pt() > m2.pt())
{
mumucand.addUserInt("highMuonTMatch",muonFilters[i]);
mumucand.addUserInt("lowMuonTMatch",muonFilters[j]);
mumucand.addUserInt("highMuonDeltaR",muonDeltaR[i]);
mumucand.addUserInt("lowMuonDeltaR",muonDeltaR[j]);
mumucand.addUserInt("highMuonDeltaPt",muonDeltaPt[i]);
mumucand.addUserInt("lowMuonDeltaPt",muonDeltaPt[j]);
if(muonFilters[i]>0)
mumucand.addDaughter(matchedColl[i],"highMuonTrigger");
if(muonFilters[j]>0)
mumucand.addDaughter(matchedColl[j],"lowMuonTrigger");

} else
{
mumucand.addUserInt("lowMuonTMatch",muonFilters[i]);
mumucand.addUserInt("highMuonTMatch",muonFilters[j]);
mumucand.addUserInt("highMuonDeltaR",muonDeltaR[j]);
mumucand.addUserInt("lowMuonDeltaR",muonDeltaR[i]);
mumucand.addUserInt("highMuonDeltaPt",muonDeltaPt[j]);
mumucand.addUserInt("lowMuonDeltaPt",muonDeltaPt[i]);
if(muonFilters[i]>0)
mumucand.addDaughter(matchedColl[i],"lowMuonTrigger");
if(muonFilters[j]>0)
mumucand.addDaughter(matchedColl[j],"highMuonTrigger");

}

// ---- define and set candidate's 4momentum  ----
LorentzVector mumu = m1.p4() + m2.p4();
TLorentzVector mu1, mu2,mumuP4;
mu1.SetXYZM(m1.track()->px(),m1.track()->py(),m1.track()->pz(),muon_mass);
mu2.SetXYZM(m2.track()->px(),m2.track()->py(),m2.track()->pz(),muon_mass);
// LorentzVector mumu;

mumuP4=mu1+mu2;
mumucand.setP4(mumu);
mumucand.setCharge(m1.charge()+m2.charge());

float deltaRMuMu = reco::deltaR2(m1.eta(),m1.phi(),m2.eta(),m2.phi());
mumucand.addUserFloat("deltaR",deltaRMuMu);
mumucand.addUserFloat("mumuP4",mumuP4.M());
// ---- apply the dimuon cut ----

if(!dimuonSelection_(mumucand)) continue;
//std::coutug  6" << std::endl;

vector<TransientTrack> muon_ttks;
muon_ttks.push_back(theTTBuilder->build(m1.track()));  // pass the reco::Track, not  the reco::TrackRef (which can be transient)
muon_ttks.push_back(theTTBuilder->build(m2.track())); // otherwise the vertex will have transient refs inside.

//Vertex Fit W/O mass constrain

TransientVertex mumuVertex = vtxFitter.vertex(muon_ttks);
CachingVertex<5> VtxForInvMass = vtxFitter.vertex( muon_ttks );

Measurement1D MassWErr(mumu.M(),-9999.);
if ( field->nominalValue() > 0 ) {
MassWErr = massCalculator.invariantMass( VtxForInvMass, muMasses );
} else {
mumuVertex = TransientVertex();                      // with no arguments it is invalid
}

mumucand.addUserFloat("MassErr",MassWErr.error());


if (mumuVertex.isValid()) {

float vChi2 = mumuVertex.totalChiSquared();
float vNDF  = mumuVertex.degreesOfFreedom();
float vProb(TMath::Prob(vChi2,(int)vNDF));

mumucand.addUserFloat("vNChi2",vChi2/vNDF);
mumucand.addUserFloat("vProb",vProb);

TVector3 vtx,vtx3D;
TVector3 pvtx,pvtx3D;
VertexDistanceXY vdistXY;

vtx.SetXYZ(mumuVertex.position().x(),mumuVertex.position().y(),0);
vtx3D.SetXYZ(mumuVertex.position().x(),mumuVertex.position().y(),mumuVertex.position().z());
TVector3 pperp(mumu.px(), mumu.py(), 0);
TVector3 pperp3D(mumu.px(), mumu.py(), mumu.pz());
AlgebraicVector3 vpperp(pperp.x(),pperp.y(),0);
AlgebraicVector3 vpperp3D(pperp.x(),pperp.y(),pperp.z());
//std::coutug  7" << std::endl;
if (resolveAmbiguity_) {

float minDz = 999999.;
TwoTrackMinimumDistance ttmd;
bool status = ttmd.calculate( GlobalTrajectoryParameters(
GlobalPoint(mumuVertex.position().x(), mumuVertex.position().y(), mumuVertex.position().z()),
GlobalVector(mumucand.px(),mumucand.py(),mumucand.pz()),TrackCharge(0),&(*magneticField)),
GlobalTrajectoryParameters(
GlobalPoint(bs.position().x(), bs.position().y(), bs.position().z()),
GlobalVector(bs.dxdz(), bs.dydz(), 1.),TrackCharge(0),&(*magneticField)));
float extrapZ=-9E20;
if (status) extrapZ=ttmd.points().first.z();

for(VertexCollection::const_iterator itv = priVtxs->begin(), itvend = priVtxs->end(); itv != itvend; ++itv){
float deltaZ = fabs(extrapZ - itv->position().z()) ;
if ( deltaZ < minDz ) {
minDz = deltaZ;
thePrimaryV = Vertex(*itv);
}
}
}

Vertex theOriginalPV = thePrimaryV;

muonLess.clear();
muonLess.reserve(thePrimaryV.tracksSize());

// if( addMuonlessPrimaryVertex_  && thePrimaryV.tracksSize()>2) {
//   // Primary vertex matched to the dimuon, now refit it removing the two muons
//   DiMuonVtxReProducer revertex(priVtxs, iEvent);
//   edm::Handle<reco::TrackCollection> pvtracks;
//   iEvent.getByToken(revtxtrks_,   pvtracks);
//   if( !pvtracks.isValid()) { std::cout << "pvtracks NOT valid " << std::endl; }
//   else {
//     edm::Handle<reco::BeamSpot> pvbeamspot;
//     iEvent.getByToken(revtxbs_, pvbeamspot);
//     if (pvbeamspot.id() != theBeamSpot.id()) edm::LogWarning("Inconsistency") << "The BeamSpot used for PV reco is not the same used in this analyzer.";
//     // I need to go back to the reco::Muon object, as the TrackRef in the pat::Muon can be an embedded ref.
//     const reco::Muon *rmu1 = dynamic_cast<const reco::Muon *>(m1.originalObject());
//     const reco::Muon *rmu2 = dynamic_cast<const reco::Muon *>(m2.originalObject());
//     // check that muons are truly from reco::Muons (and not, e.g., from PF Muons)
//     // also check that the tracks really come from the track collection used for the BS
//     if (rmu1 != nullptr && rmu2 != nullptr && rmu1->track().id() == pvtracks.id() && rmu2->track().id() == pvtracks.id()) {
//       // Save the keys of the tracks in the primary vertex
//       // std::vector<size_t> vertexTracksKeys;
//       // vertexTracksKeys.reserve(thePrimaryV.tracksSize());
//       if( thePrimaryV.hasRefittedTracks() ) {
//         // Need to go back to the original tracks before taking the key
//         std::vector<reco::Track>::const_iterator itRefittedTrack = thePrimaryV.refittedTracks().begin();
//         std::vector<reco::Track>::const_iterator refittedTracksEnd = thePrimaryV.refittedTracks().end();
//         for( ; itRefittedTrack != refittedTracksEnd; ++itRefittedTrack )
//         {
//           if( thePrimaryV.originalTrack(*itRefittedTrack).key() == rmu1->track().key() ) continue;
//           if( thePrimaryV.originalTrack(*itRefittedTrack).key() == rmu2->track().key() ) continue;
//           // vertexTracksKeys.push_back(thePrimaryV.originalTrack(*itRefittedTrack).key());
//           muonLess.push_back(*(thePrimaryV.originalTrack(*itRefittedTrack)));
//         }
//       }
//       else {
//         std::vector<reco::TrackBaseRef>::const_iterator itPVtrack = thePrimaryV.tracks_begin();
//         for( ; itPVtrack != thePrimaryV.tracks_end(); ++itPVtrack ) if (itPVtrack->isNonnull()) {
//           if( itPVtrack->key() == rmu1->track().key() ) continue;
//           if( itPVtrack->key() == rmu2->track().key() ) continue;
//           // vertexTracksKeys.push_back(itPVtrack->key());
//           muonLess.push_back(**itPVtrack);
//         }
//       }
//       if (muonLess.size()>1 && muonLess.size() < thePrimaryV.tracksSize()){
//         pvs = revertex.makeVertices(muonLess, *pvbeamspot, iSetup) ;
//         if (!pvs.empty()) {
//           Vertex muonLessPV = Vertex(pvs.front());
//           thePrimaryV = muonLessPV;
//         }
//       }
//     }
//   }
// }

// count the number of high Purity tracks with pT > 900 MeV attached to the chosen vertex
double vertexWeight = -1., sumPTPV = -1.;
int countTksOfPV = -1;
const reco::Muon *rmu1 = dynamic_cast<const reco::Muon *>(m1.originalObject());
const reco::Muon *rmu2 = dynamic_cast<const reco::Muon *>(m2.originalObject());
try{
for(reco::Vertex::trackRef_iterator itVtx = theOriginalPV.tracks_begin(); itVtx != theOriginalPV.tracks_end(); itVtx++) if(itVtx->isNonnull()){
const reco::Track& track = **itVtx;
if(!track.quality(reco::TrackBase::highPurity)) continue;
if(track.pt() < 0.5) continue; //reject all rejects from counting if less than 900 MeV
TransientTrack tt = theTTBuilder->build(track);
pair<bool,Measurement1D> tkPVdist = IPTools::absoluteImpactParameter3D(tt,thePrimaryV);
if (!tkPVdist.first) continue;
if (tkPVdist.second.significance()>3) continue;
if (track.ptError()/track.pt()>0.1) continue;
// do not count the two muons
if (rmu1 != nullptr && rmu1->innerTrack().key() == itVtx->key())
continue;
if (rmu2 != nullptr && rmu2->innerTrack().key() == itVtx->key())
continue;

vertexWeight += theOriginalPV.trackWeight(*itVtx);
if(theOriginalPV.trackWeight(*itVtx) > 0.5){
countTksOfPV++;
sumPTPV += track.pt();
}
}
} catch (std::exception & err) {std::cout << " muon Selection failed " << std::endl; return ; }

mumucand.addUserInt("countTksOfPV", countTksOfPV);
mumucand.addUserFloat("vertexWeight", (float) vertexWeight);
mumucand.addUserFloat("sumPTPV", (float) sumPTPV);
//std::coutug  8" << std::endl;
///DCA
TrajectoryStateClosestToPoint mu1TS = muon_ttks[0].impactPointTSCP();
TrajectoryStateClosestToPoint mu2TS = muon_ttks[1].impactPointTSCP();
float dca = 1E20;
if (mu1TS.isValid() && mu2TS.isValid()) {
ClosestApproachInRPhi cApp;
cApp.calculate(mu1TS.theState(), mu2TS.theState());
if (cApp.status() ) dca = cApp.distance();
}
mumucand.addUserFloat("DCA", dca );
///end DCA

if (addMuonlessPrimaryVertex_)
mumucand.addUserData("muonlessPV",Vertex(thePrimaryV));

mumucand.addUserData("PVwithmuons",Vertex(theOriginalPV));

// lifetime using PV
float cosAlpha, cosAlpha3D, ppdlPV, ppdlErrPV, l_xyz, l_xy, lErr_xyz, lErr_xy;

//2D
pvtx.SetXYZ(thePrimaryV.position().x(),thePrimaryV.position().y(),0);
TVector3 vdiff = vtx - pvtx;
cosAlpha = vdiff.Dot(pperp)/(vdiff.Perp()*pperp.Perp());

Measurement1D distXY = vdistXY.distance(Vertex(mumuVertex), thePrimaryV);
ppdlPV = distXY.value()*cosAlpha * mumucand.mass()/pperp.Perp();

GlobalError v1e = (Vertex(mumuVertex)).error();
GlobalError v2e = thePrimaryV.error();
AlgebraicSymMatrix33 vXYe = v1e.matrix()+ v2e.matrix();
ppdlErrPV = sqrt(ROOT::Math::Similarity(vpperp,vXYe))*mumucand.mass()/(pperp.Perp2());

AlgebraicVector3 vDiff;
vDiff[0] = vdiff.x(); vDiff[1] = vdiff.y(); vDiff[2] = 0 ;
l_xy = vdiff.Perp();
lErr_xy = sqrt(ROOT::Math::Similarity(vDiff,vXYe)) / vdiff.Perp();

/// 3D
pvtx3D.SetXYZ(thePrimaryV.position().x(), thePrimaryV.position().y(), thePrimaryV.position().z());
TVector3 vdiff3D = vtx3D - pvtx3D;
cosAlpha3D = vdiff3D.Dot(pperp3D)/(vdiff3D.Mag()*vdiff3D.Mag());
l_xyz = vdiff3D.Mag();

AlgebraicVector3 vDiff3D;
vDiff3D[0] = vdiff3D.x(); vDiff3D[1] = vdiff3D.y(); vDiff3D[2] = vdiff3D.z() ;
lErr_xyz = sqrt(ROOT::Math::Similarity(vDiff3D,vXYe)) / vdiff3D.Mag();

mumucand.addUserFloat("ppdlPV",ppdlPV);
mumucand.addUserFloat("ppdlErrPV",ppdlErrPV);
mumucand.addUserFloat("cosAlpha",cosAlpha);
mumucand.addUserFloat("cosAlpha3D",cosAlpha3D);

mumucand.addUserFloat("l_xy",l_xy);
mumucand.addUserFloat("lErr_xy",lErr_xy);

mumucand.addUserFloat("l_xyz",l_xyz);
mumucand.addUserFloat("lErr_xyz",lErr_xyz);

// lifetime using BS
float cosAlphaBS, cosAlphaBS3D, ppdlBS, ppdlErrBS, l_xyBS, lErr_xyBS, l_xyzBS, lErr_xyzBS;

//2D
pvtx.SetXYZ(theBeamSpotV.position().x(),theBeamSpotV.position().y(),0);
vdiff = vtx - pvtx;
cosAlphaBS = vdiff.Dot(pperp)/(vdiff.Perp()*pperp.Perp());
distXY = vdistXY.distance(Vertex(mumuVertex), theBeamSpotV);
//double ppdlBS = distXY.value()*cosAlpha*3.09688/pperp.Perp();

ppdlBS = distXY.value()*cosAlpha*mumucand.mass()/pperp.Perp();

GlobalError v1eB = (Vertex(mumuVertex)).error();
GlobalError v2eB = theBeamSpotV.error();
AlgebraicSymMatrix33 vXYeB = v1eB.matrix()+ v2eB.matrix();
//double ppdlErrBS = sqrt(vXYeB.similarity(vpperp))*3.09688/(pperp.Perp2());
ppdlErrBS = sqrt(ROOT::Math::Similarity(vpperp,vXYeB))*mumucand.mass()/(pperp.Perp2());

vDiff[0] = vdiff.x(); vDiff[1] = vdiff.y(); vDiff[2] = 0 ;
l_xyBS = vdiff.Perp();
lErr_xyBS = sqrt(ROOT::Math::Similarity(vDiff,vXYe)) / vdiff.Perp();

/// 3D
pvtx3D.SetXYZ(theBeamSpotV.position().x(), theBeamSpotV.position().y(), theBeamSpotV.position().z());
vdiff3D = vtx3D - pvtx3D;
cosAlphaBS3D = vdiff3D.Dot(pperp3D)/(vdiff3D.Mag()*vdiff3D.Mag());
l_xyzBS = vdiff3D.Mag();
vDiff3D[0] = vdiff3D.x(); vDiff3D[1] = vdiff3D.y(); vDiff3D[2] = vdiff3D.z() ;
lErr_xyzBS = sqrt(ROOT::Math::Similarity(vDiff3D,vXYe)) / vdiff3D.Mag();

//std::coutug  9" << std::endl;
mumucand.addUserFloat("ppdlBS",ppdlBS);
mumucand.addUserFloat("ppdlErrBS",ppdlErrBS);
mumucand.addUserFloat("cosAlphaBS",cosAlphaBS);
mumucand.addUserFloat("cosAlphaBS3D",cosAlphaBS3D);

mumucand.addUserFloat("l_xyBS",l_xyBS);
mumucand.addUserFloat("lErr_xyBS",lErr_xyBS);

mumucand.addUserFloat("l_xyzBS",l_xyzBS);
mumucand.addUserFloat("lErr_xyzBS",lErr_xyzBS);

mumucand.addUserData("commonVertex",Vertex(mumuVertex));

} else {

continue;

mumucand.addUserFloat("vNChi2",-1);
mumucand.addUserFloat("vProb", -1);

mumucand.addUserFloat("ppdlPV",-100);
mumucand.addUserFloat("ppdlErrPV",-100);
mumucand.addUserFloat("cosAlpha",-100);
mumucand.addUserFloat("cosAlpha3D",-100);

mumucand.addUserFloat("l_xy",-100);
mumucand.addUserFloat("lErr_xy",-100);

mumucand.addUserFloat("l_xyz",-100);
mumucand.addUserFloat("lErr_xyz",-100);

mumucand.addUserFloat("ppdlBS",-100);
mumucand.addUserFloat("ppdlErrBS",-100);
mumucand.addUserFloat("cosAlphaBS",-100);
mumucand.addUserFloat("cosAlphaBS3D",-100);

mumucand.addUserFloat("l_xyBS",-100);
mumucand.addUserFloat("lErr_xyBS",-100);

mumucand.addUserFloat("l_xyzBS",-100);
mumucand.addUserFloat("lErr_xyzBS",-100);

mumucand.addUserFloat("DCA", -1 );

if (addCommonVertex_) {
mumucand.addUserData("commonVertex",Vertex());
}
if (addMuonlessPrimaryVertex_) {
mumucand.addUserData("muonlessPV",Vertex());
} else {
mumucand.addUserData("PVwithmuons",Vertex());
}

}
//std::coutug  10" << std::endl;
//Muon mass Vertex Refit
float refittedMass = -1.0, mumuVtxCL = -1.0;

KinematicParticleFactoryFromTransientTrack pFactory;
vector<RefCountedKinematicParticle> muonParticles;

float kinChi = 0.;
float kinNdf = 0.;

muonParticles.push_back(pFactory.particle(muon_ttks[0],muon_mass,kinChi,kinNdf,muon_sigma));
muonParticles.push_back(pFactory.particle(muon_ttks[1],muon_mass,kinChi,kinNdf,muon_sigma));

KinematicParticleVertexFitter kFitter;
RefCountedKinematicTree mumuVertexFitTree;
mumuVertexFitTree = kFitter.fit(muonParticles);

if (mumuVertexFitTree->isValid())
{
mumuVertexFitTree->movePointerToTheTop();
RefCountedKinematicVertex mumu_KV = mumuVertexFitTree->currentDecayVertex();
if (mumu_KV->vertexIsValid())
{
RefCountedKinematicParticle mumu_vFit = mumuVertexFitTree->currentParticle();
refittedMass = mumu_vFit->currentState().mass();

mumuVtxCL = TMath::Prob((double)mumu_KV->chiSquared(),int(rint(mumu_KV->degreesOfFreedom())));
}
}

mumucand.addUserFloat("refittedMass", refittedMass);
mumucand.addUserFloat("mumuVtxCL", mumuVtxCL);

// ---- MC Truth, if enabled ----
// if (addMCTruth_) {
//   reco::GenParticleRef genMu1 = m1.genParticleRef();
//   reco::GenParticleRef genMu2 = m2.genParticleRef();
//   if (genMu1.isNonnull() && genMu2.isNonnull()) {
//     if (genMu1->numberOfMothers()>0 && genMu2->numberOfMothers()>0){
//       reco::GenParticleRef mom1 = genMu1->motherRef();
//       reco::GenParticleRef mom2 = genMu2->motherRef();
//       if (mom1.isNonnull() && (mom1 == mom2)) {
//         mumucand.setGenParticleRef(mom1); // set
//         mumucand.embedGenParticle();      // and embed
//         std::pair<int, float> MCinfo = findJpsiMCInfo(mom1);
//         mumucand.addUserInt("momPDGId",MCinfo.first);
//         mumucand.addUserFloat("ppdlTrue",MCinfo.second);
//       } else {
//         mumucand.addUserInt("momPDGId",0);
//         mumucand.addUserFloat("ppdlTrue",-99.);
//       }
//     } else {
//       edm::Handle<reco::GenParticleCollection> theGenParticles;
//       edm::EDGetTokenT<reco::GenParticleCollection> genCands_ = consumes<reco::GenParticleCollection>((edm::InputTag)"genParticles");
//       iEvent.getByToken(genCands_, theGenParticles);
//       if (theGenParticles.isValid()){
//         for(size_t iGenParticle=0; iGenParticle<theGenParticles->size();++iGenParticle) {
//           const Candidate & genCand = (*theGenParticles)[iGenParticle];
//           if (genCand.pdgId()==443 || genCand.pdgId()==100443 ||
//           genCand.pdgId()==553 || genCand.pdgId()==100553 || genCand.pdgId()==200553) {
//             reco::GenParticleRef mom1(theGenParticles,iGenParticle);
//             mumucand.setGenParticleRef(mom1);
//             mumucand.embedGenParticle();
//             std::pair<int, float> MCinfo = findJpsiMCInfo(mom1);
//             mumucand.addUserInt("momPDGId",MCinfo.first);
//             mumucand.addUserFloat("ppdlTrue",MCinfo.second);
//           }
//         }
//       } else {
//         mumucand.addUserInt("momPDGId",0);
//         mumucand.addUserFloat("ppdlTrue",-99.);
//       }
//     }
//   } else {
//     mumucand.addUserInt("momPDGId",0);
//     mumucand.addUserFloat("ppdlTrue",-99.);
//   }
// }
//std::coutug  11" << std::endl;

// ---- Push back output ----
if(muonFilters[i]>0 && muonFilters[j]>0)
mumucand.addUserInt("isTriggerMatched",int(true));
else
mumucand.addUserInt("isTriggerMatched",int(false));

oniaOutput->push_back(mumucand);
}
}
*/
std::sort(oniaOutput->begin(),oniaOutput->end(),vPComparator_);
//std::cout << "MuMu candidates count : " << oniaOutput->size() << std::endl;
iEvent.put(std::move(oniaOutput));

}



// ------------ method called once each job just before starting event loop  ------------
void
TrackProducerPAT::beginJob()
{
}

// ------------ method called once each job just after ending the event loop  ------------
void
TrackProducerPAT::endJob() {
}

//define this as a plug-in
DEFINE_FWK_MODULE(TrackProducerPAT);