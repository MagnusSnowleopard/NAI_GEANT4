#include "RunAction.hh"

#include "G4AnalysisManager.hh"
#include "G4GenericMessenger.hh"
#include "G4Run.hh"
#include "G4SystemOfUnits.hh"

namespace NaI
{

RunAction::RunAction()
{
  ConfigureMessenger();

  auto analysisManager = G4AnalysisManager::Instance();
  G4cout << "Using " << analysisManager->GetType() << " analysis manager" << G4endl;

  analysisManager->SetVerboseLevel(1);
  analysisManager->SetFileName("NaI.root");

  const G4int nbins = 16384;
  const G4double xmin = 0.;
  const G4double xmax = 16384.;

  analysisManager->CreateH1("EdepTotal_keV", "Total energy deposited in NaI per event", nbins, xmin, xmax);
  analysisManager->CreateH1("EdepGammaOrigin_keV",
                            "Energy deposited in NaI by gamma-origin track lineage", nbins, xmin,
                            xmax);
  analysisManager->CreateH1("EdepNeutronOrigin_keV",
                            "Energy deposited in NaI by neutron-origin track lineage", nbins, xmin,
                            xmax);

  analysisManager->CreateH1("EdepGammaOnly_keV", "Total deposited energy for gamma-only primaries",
                            nbins, xmin, xmax);
  analysisManager->CreateH1("EdepNeutronOnly_keV",
                            "Total deposited energy for neutron-only primaries", nbins, xmin, xmax);
  analysisManager->CreateH1("EdepFullAmBe_keV", "Total deposited energy for full AmBe primaries",
                            nbins, xmin, xmax);
  analysisManager->CreateH1("EdepGammaOriginSmeared_keV",
                            "Gamma-origin deposited energy/event with detector-resolution smearing",
                            nbins, xmin, xmax);
  analysisManager->CreateH1("EdepGammaOnlySmeared_keV",
                            "Total deposited energy for gamma-only primaries with smearing", nbins,
                            xmin, xmax);
  analysisManager->CreateH1("EdepFullAmBeSmeared_keV",
                            "Total deposited energy for full AmBe primaries with smearing when gamma is present",
                            nbins, xmin, xmax);
}

RunAction::~RunAction()
{
  delete fMessenger;
}

void RunAction::ConfigureMessenger()
{
  fMessenger = new G4GenericMessenger(this, "/NaI/analysis/", "Analysis controls");
  auto& centerCmd = fMessenger->DeclarePropertyWithUnit(
      "peakCenter", "keV", fPeakCenter,
      "Center of experimental 4.438 MeV peak window used for event counting");
  centerCmd.SetStates(G4State_PreInit, G4State_Idle);

  auto& widthCmd = fMessenger->DeclarePropertyWithUnit(
      "peakHalfWidth", "keV", fPeakHalfWidth,
      "Half-width of experimental peak window (use ~1-2 sigma)");
  widthCmd.SetStates(G4State_PreInit, G4State_Idle);

  auto& resEnableCmd = fMessenger->DeclareProperty(
      "applyResolutionSmearing", fApplyResolutionSmearing,
      "Enable/disable Gaussian energy smearing for gamma-related deposited-energy spectra.");
  resEnableCmd.SetStates(G4State_PreInit, G4State_Idle);

  auto& resRefEnergyCmd = fMessenger->DeclarePropertyWithUnit(
      "resolutionRefEnergy", "keV", fResolutionRefEnergy,
      "Reference energy where resolution FWHM fraction is specified.");
  resRefEnergyCmd.SetStates(G4State_PreInit, G4State_Idle);

  auto& resRefFwhmCmd = fMessenger->DeclareProperty(
      "resolutionRefFwhmFraction", fResolutionRefFwhmFraction,
      "Detector resolution at reference energy as FWHM/E (e.g. 0.055 for 5.5%).");
  resRefFwhmCmd.SetStates(G4State_PreInit, G4State_Idle);
}

void RunAction::BeginOfRunAction(const G4Run*)
{
  fEdep = 0.;
  fGammaOnlyWindowCounts = 0;
  fNeutronOnlyWindowCounts = 0;
  fFullAmBeWindowCounts = 0;

  auto analysisManager = G4AnalysisManager::Instance();
  analysisManager->OpenFile();
}

void RunAction::CountWindowEvent(G4bool gammaOnlyPrimary, G4bool neutronOnlyPrimary,
                                 G4bool fullAmBePrimary)
{
  if (gammaOnlyPrimary) {
    ++fGammaOnlyWindowCounts;
  }
  if (neutronOnlyPrimary) {
    ++fNeutronOnlyWindowCounts;
  }
  if (fullAmBePrimary) {
    ++fFullAmBeWindowCounts;
  }
}

void RunAction::EndOfRunAction(const G4Run* run)
{
  auto analysisManager = G4AnalysisManager::Instance();

  analysisManager->Write();
  analysisManager->CloseFile();

  G4cout << "\n=== 4.438 MeV peak-window summary ===\n"
         << " Run events: " << run->GetNumberOfEvent() << "\n"
         << " Window center (keV): " << GetPeakCenterKeV() << "\n"
         << " Window half-width (keV): " << GetPeakHalfWidthKeV() << "\n"
         << " Gamma-only events in window: " << fGammaOnlyWindowCounts << "\n"
         << " Neutron-only events in window: " << fNeutronOnlyWindowCounts << "\n"
         << " Full-AmBe events in window: " << fFullAmBeWindowCounts << G4endl;
}

}  // namespace NaI
