/* --------------------------------------------------
   Copyright (C): OpenGATE Collaboration
   This software is distributed under the terms
   of the GNU Lesser General  Public Licence (LGPL)
   See LICENSE.md for further details
   -------------------------------------------------- */

#include "G4SDManager.hh"
#include "G4MultiFunctionalDetector.hh"
#include "GateTestActor.h"

GateTestActor::GateTestActor() : G4VPrimitiveScorer("bidon") {
    std::cout << "GateTestActor Constructor" << std::endl;
    num_step = 0;
    mfd = NULL;
}

GateTestActor::~GateTestActor() {
    std::cout << "GateTestActor Destructor" << std::endl;
    //delete mfd;
}

G4bool GateTestActor::ProcessHits(G4Step *step,
                                  G4TouchableHistory *touchableHistory) {
    //std::cout << "GateTestActor::ProcessHits " << std::endl;
    num_step++;
    return true;
}

void GateTestActor::RegisterSD(G4LogicalVolume * logical_volume) {
    std::cout << "GateTestActor::registerSD" << std::endl;
    mfd = new G4MultiFunctionalDetector("bidon_msd");
    // do not always create check if exist
    // auto pointer
    G4SDManager::GetSDMpointer()->AddNewDetector(mfd);
    logical_volume->SetSensitiveDetector(mfd);
    mfd->RegisterPrimitive(this);
    std::cout << "GateTestActor::registerSD DONE" << std::endl;
}

void GateTestActor::PrintDebug() {
    std::cout << "num step " << num_step << std::endl;
}