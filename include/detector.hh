#ifndef DETECTOR_HH
#define DETECTOR_HH

#include "G4VSensitiveDetector.hh"
#include "G4Run.hh"

#include <vector>

class MySensitiveDetector : public G4VSensitiveDetector
{
public:
    MySensitiveDetector(G4String name, G4int nx=50, G4int ny=50);
    ~MySensitiveDetector();  

    virtual void Initialize(G4HCofThisEvent* hce) override;
    virtual G4bool ProcessHits(G4Step*, G4TouchableHistory*) override;
    virtual void EndOfEvent(G4HCofThisEvent* hce) override;
    virtual void EndOfRun(const G4Run* run);  

    G4int GetNx() const { return fNx; }
    G4int GetNy() const { return fNy; }
    G4double GetPixel(G4int ix, G4int iy) const { return fImage[ix][iy]; }

    // Set the output file path written by EndOfRun (default: ../xray_image.dat)
    void SetOutputFile(const G4String& path) { fOutputFile = path; }

private:
    G4int fNx, fNy;
    std::vector<std::vector<G4double>> fImage;  // deposited energy per pixel
    G4String fOutputFile = "../results/raw/xray_image.dat";
};

#endif
