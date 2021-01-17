/* --------------------------------------------------
   Copyright (C): OpenGATE Collaboration
   This software is distributed under the terms
   of the GNU Lesser General  Public Licence (LGPL)
   See LICENSE.md for further details
   -------------------------------------------------- */

#include <iostream>
#include <pybind11/stl.h>

#include "G4RunManager.hh"
#include "G4UImanager.hh"
#include "G4UIExecutive.hh"
#include "G4VisExecutive.hh"
#include "G4UIsession.hh"
#include "GamSourceManager.h"
#include "GamDictHelpers.h"

/* There will be one SourceManager per thread */

GamSourceManager::GamSourceManager() {
    fStartNewRun = true;
    fNextRunId = 0;
}


void GamSourceManager::Initialize(TimeIntervals simulation_times, py::dict &options) {
    fSimulationTimes = simulation_times;
    fStartNewRun = true;
    fNextRunId = 0;
    fOptions = options;
    DD(fOptions);
    fVisualizationFlag = DictBool(fOptions, "g4_visualisation_flag");
    DDD(fVisualizationFlag);
}

void GamSourceManager::AddSource(GamVSource *source) {
    fSources.push_back(source);
}


// Temporary: later option will be used to control the verbosity
class UIsessionSilent : public G4UIsession {
public:

    virtual G4int ReceiveG4cout(const G4String & /*coutString*/) { return 0; }

    virtual G4int ReceiveG4cerr(const G4String & /*cerrString*/) { return 0; }
};


void GamSourceManager::StartMainThread() {
    // Create the main macro command
    std::ostringstream oss;
    oss << "/run/beamOn " << INT32_MAX;
    std::string run = oss.str();
    DD(run);
    DD(fOptions.size());
    //auto a = py::bool_(fOptions["g4_visualisation_flag"]);
    //DD(a);
    // Loop on run
    for (size_t run_id = 0; run_id < fSimulationTimes.size(); run_id++) {
        StartRun(run_id);
        DDD("Before BeamOn");
        InitializeVisualization();
        // G4RunManager::GetRunManager()->BeamOn(INT32_MAX);
        DDD("before apply command beam on");
        auto uim = G4UImanager::GetUIpointer();
        // uim->SetVerboseLevel(0);
        uim->ApplyCommand(run);
        DDD("After BeamOn");
    }
    DDD("End Start MainThread");

}

void GamSourceManager::StartRun(int run_id) {
    // set the current time interval
    fCurrentTimeInterval = fSimulationTimes[run_id];
    // set the current time
    fCurrentSimulationTime = fCurrentTimeInterval.first;
    // Prepare the run for all source
    for (auto source:fSources) {
        source->PrepareNextRun();
    }
    // Check next time
    PrepareNextSource();
    if (fNextActiveSource == NULL) return;
    fStartNewRun = false;
}

void GamSourceManager::PrepareNextSource() {
    fNextActiveSource = NULL;
    double min_time = fCurrentTimeInterval.first;
    double max_time = fCurrentTimeInterval.second;
    // Ask all sources their next time, keep the closest one
    for (auto source:fSources) {
        auto t = source->PrepareNextTime(fCurrentSimulationTime);
        if ((t >= min_time) && (t < max_time)) {
            max_time = t;
            fNextActiveSource = source;
            fNextSimulationTime = t;
        }
    }
    // If no next time in the current interval, active source is NULL
}

void GamSourceManager::CheckForNextRun() {
    // FIXME Check active source NULL ?
    if (fNextActiveSource == NULL) {
        G4RunManager::GetRunManager()->AbortRun(true);
        fStartNewRun = true;
        fNextRunId++;
        if (fNextRunId == fSimulationTimes.size()) {
            // Sometimes, the source must clean some data in its own thread, not by the master thread
            // (for example with a G4SingleParticleSource object)
            // The CleanThread method is used for that.
            for (auto source:fSources) {
                source->CleanInThread();
            }
            // FIXME --> Add here actor SimulationStopInThread
        }
    }
}

void GamSourceManager::GeneratePrimaries(G4Event *event) {
    // Needed to initialize a new Run (all threads)
    if (fStartNewRun) StartRun(fNextRunId);

    // update the current time
    fCurrentSimulationTime = fNextSimulationTime;

    // shoot particle
    fNextActiveSource->GeneratePrimaries(event, fCurrentSimulationTime);

    // prepare the next source
    PrepareNextSource();

    // check if this is not the end of the run
    CheckForNextRun();
}

void GamSourceManager::InitializeVisualization() {
    if (!fVisualizationFlag) return;
    DDD("before session start");
    char *argv[1];
    //uiex = new G4UIExecutive(1, argv);
    viex = new G4VisExecutive("all");
    //G4RunManager::GetRunManager()->SetVerboseLevel(0);
    /*
     quiet,         // Nothing is printed.
     startup,       // Startup and endup messages are printed...
     errors,        // ...and errors...
     warnings,      // ...and warnings...
     confirmations, // ...and confirming messages...
     parameters,    // ...and parameters of scenes and views...
     all            // ...and everything available.
     */
    viex->Initialise();
    auto uim = G4UImanager::GetUIpointer();
    DDD(uim->GetVerboseLevel());
    DDD("before apply command");
    uim->ApplyCommand("/vis/open OGLIQt");
    uim->ApplyCommand("/control/verbose 2");
    DDD("after vis open");
    uim->ApplyCommand("/vis/drawVolume");
    uim->ApplyCommand("/vis/viewer/flush"); //# not sure needed
    uim->ApplyCommand("/tracking/storeTrajectory 1");
    uim->ApplyCommand("/vis/scene/add/trajectories");
    uim->ApplyCommand("/vis/scene/endOfEventAction accumulate");
    uim->SetCoutDestination(new UIsessionSilent);
    DD("end init visu");
}

void GamSourceManager::StartVisualization() {
    if (!fVisualizationFlag) return;
    DDD("After run beam on");
    //uiex->SessionStart();
    DDD("after session start");

    // FIXME VISUALIZATION
    //self.simulation.g4_apply_command("/run/beamOn {self.max_int}")
    //        self.simulation.g4_ui_executive.SessionStart()
    //self.g4_ui = g4.G4UImanager.GetUIpointer()
    //        self.g4_ui.ApplyCommand(command)

    /*
    auto ui = G4UImanager::GetUIpointer();
    DD("before beam on");
    ui->ApplyCommand("/run/beamOn 2147483647");
    DD("ici");
     */
    /*
    //self.simulation.g4_ui_executive.SessionStart()
    char *argv[1];
    DD("la");
    auto uie = new G4UIExecutive(1, argv);
    DD("before start")
    uie->SessionStart();
    DD("after start")
    //G4RunManager::GetRunManager()->BeamOn(INT32_MAX);
     */

}
