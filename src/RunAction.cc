#include "RunAction.hh"

#include "G4AnalysisManager.hh"
#include "G4Run.hh"

#include <cmath>
#include <cstdlib>
#include <string>

namespace NaI
{

RunAction::RunAction()
{
  auto* analysisManager = G4AnalysisManager::Instance();
  analysisManager->SetVerboseLevel(1);

  std::string outputName = "NaI";
  if (const char* outputEnv = std::getenv("NAI_OUTPUT_BASENAME")) {
    if (outputEnv[0] != '\0') {
      outputName = outputEnv;
    }
  }
  else if (const char* modeEnv = std::getenv("NAI_SOURCE_MODE")) {
    outputName += "_" + std::string(modeEnv);
  }
  analysisManager->SetFileName(outputName);

  if (const char* centerEnv = std::getenv("NAI_PEAK_CENTER_KEV")) {
    const auto value = std::atof(centerEnv);
    if (value > 0.) {
      fPeakCenterKeV = value;
    }
  }

  if (const char* widthEnv = std::getenv("NAI_PEAK_HALF_WIDTH_KEV")) {
    const auto value = std::atof(widthEnv);
    if (value > 0.) {
      fPeakHalfWidthKeV = value;
    }
  }

  constexpr G4int nbins = 8192;
  constexpr G4double xmin = 0.;
  constexpr G4double xmax = 16384.;

  analysisManager->CreateH1("EdepTotal", "Total event energy in NaI (keV)", nbins, xmin, xmax);
  analysisManager->CreateH1("EdepGammaLineage", "Energy in NaI from primary-gamma lineage (keV)", nbins, xmin, xmax);
  analysisManager->CreateH1("EdepNeutronLineage", "Energy in NaI from primary-neutron lineage (keV)", nbins, xmin, xmax);
}

void RunAction::BeginOfRunAction(const G4Run*)
{
  auto* analysisManager = G4AnalysisManager::Instance();
  fEventsGammaInWindow = 0;
  fEventsNeutronInWindow = 0;
  analysisManager->OpenFile();
}

void RunAction::ScorePeakWindow(G4double gammaEdepKeV, G4double neutronEdepKeV)
{
  if (std::abs(gammaEdepKeV - fPeakCenterKeV) <= fPeakHalfWidthKeV) {
    ++fEventsGammaInWindow;
  }
  if (std::abs(neutronEdepKeV - fPeakCenterKeV) <= fPeakHalfWidthKeV) {
    ++fEventsNeutronInWindow;
  }
}

void RunAction::EndOfRunAction(const G4Run*)
{
  auto* analysisManager = G4AnalysisManager::Instance();

  G4cout << "\n=== 4.438 MeV window summary ===\n"
         << "Window center (keV): " << fPeakCenterKeV << "\n"
         << "Window half-width (keV): " << fPeakHalfWidthKeV << "\n"
         << "Gamma-lineage events in window: " << fEventsGammaInWindow << "\n"
         << "Neutron-lineage events in window: " << fEventsNeutronInWindow << "\n";

  if ((fEventsGammaInWindow + fEventsNeutronInWindow) > 0) {
    const auto fn = static_cast<G4double>(fEventsNeutronInWindow)
      / static_cast<G4double>(fEventsGammaInWindow + fEventsNeutronInWindow);
    G4cout << "Estimated neutron-induced fraction fn: " << fn << "\n";
  }

  analysisManager->Write();
  analysisManager->CloseFile();
}

}  // namespace NaI
