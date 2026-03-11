#include "detector.hh"
#include "G4Step.hh"
#include "G4ThreeVector.hh"
#include "G4SystemOfUnits.hh"
#include "G4Run.hh"           

#include <fstream>
#include <iomanip>

MySensitiveDetector::MySensitiveDetector(G4String name, G4int nx, G4int ny)
: G4VSensitiveDetector(name), fNx(nx), fNy(ny)
{
    fImage.resize(fNx, std::vector<G4double>(fNy, 0.0));
}

MySensitiveDetector::~MySensitiveDetector() 
{
    std::ofstream outFile("../result/xray_image.dat", std::ios::trunc);  // writes in working directory

    for (int iy = 0; iy < fNy; iy++) {
        for (int ix = 0; ix < fNx; ix++) {
            outFile << fImage[ix][iy];
            if (ix < fNx - 1) outFile << " ";
        }
        outFile << "\n";
    }

    outFile.close();
}

void MySensitiveDetector::Initialize(G4HCofThisEvent*)
{}

G4bool MySensitiveDetector::ProcessHits(G4Step *aStep, G4TouchableHistory *)
{
    // Get energy deposited in this step
    G4double edep = aStep->GetTotalEnergyDeposit();
    if (edep <= 0.) return false;  

    const G4VTouchable *touchable = aStep->GetPreStepPoint()->GetTouchable();
    G4int copyNo = touchable->GetCopyNumber();

    // Convert flat copyNo back to (i,j)
    G4int ix = copyNo % fNx;
    G4int iy = copyNo / fNx;

    if (ix >= 0 && ix < fNx && iy >= 0 && iy < fNy) {
        fImage[ix][iy] += edep;  // accumulate deposited energy
    }

    return true;
}

void MySensitiveDetector::EndOfEvent(G4HCofThisEvent*)
{}

void MySensitiveDetector::EndOfRun(const G4Run*)
{
    std::ofstream outFile("../xray_image.dat", std::ios::trunc);

    for (int iy = 0; iy < fNy; iy++) {
        for (int ix = 0; ix < fNx; ix++) {
            outFile << fImage[ix][iy];
            if (ix < fNx - 1) outFile << " ";
        }
        outFile << "\n";
    }
    outFile.close();

    // Reset for next run
    for (int ix = 0; ix < fNx; ix++) {
        std::fill(fImage[ix].begin(), fImage[ix].end(), 0.0);
    }

    G4cout << ">>> Wrote xray_image.dat" << G4endl;
}
