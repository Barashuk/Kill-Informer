#include "plugin.h"
#include "imgui.h"
using namespace plugin;

class KillInformer {
public:
    static void OnRelease() {

    }
    static void OnInit() {

    }   
    static void OnDraw() {

    }
    static void OnReset() {

    }
    static void OnLost() {

    }
    KillInformer() {
        Events::d3dLostEvent += OnLost;
        Events::d3dResetEvent += OnReset;
        Events::drawingEvent += OnDraw;
        Events::shutdownRwEvent += OnRelease;
    }
} killInformer;
