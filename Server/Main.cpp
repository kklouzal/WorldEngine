#include <KNet.hpp>
#include <wx/wx.h>
#include <wx/fileconf.h>
#include "btBulletDynamicsCommon.h"
#include "BulletCollision/NarrowPhaseCollision/btRaycastCallback.h"
#include "BulletCollision/CollisionDispatch/btCollisionDispatcherMt.h"
#include "BulletDynamics/Dynamics/btSimulationIslandManagerMt.h"
#include "BulletDynamics/Dynamics/btDiscreteDynamicsWorldMt.h"
#include "BulletDynamics/ConstraintSolver/btSequentialImpulseConstraintSolverMt.h"

namespace WorldEngine
{
    namespace
    {
        //
        //	Bullet Physics
        btDefaultCollisionConfiguration* collisionConfiguration;
        btCollisionDispatcherMt* dispatcher;
        btBroadphaseInterface* broadphase;
        btConstraintSolverPoolMt* solverPool;
        btDiscreteDynamicsWorld* dynamicsWorld;

        std::string CurrentMap;
        std::string DefaultPlayerModel;
        std::chrono::seconds ClientTimeout;
        std::string CurrentGamemode;
        long TickRate = 66;
    }

    btDiscreteDynamicsWorld* GetPhysicsWorld()
    {
        return dynamicsWorld;
    }
}

//
//  Forward Declarations
class Player;
#include "LuaScripting.hpp"
//
#include "NetCode.hpp"
#include "Import_GLTF.hpp"
#include "SceneGraph.hpp"

#include "NetCode.impl.hpp"
//
#include "LuaScripting.impl.hpp"

// Define a new application type, each program should derive a class from wxApp
class MyApp : public wxApp
{
public:
    virtual bool OnInit() wxOVERRIDE;
};


// IDs for the controls and the menu commands
enum
{
    // menu items
    Minimal_Quit = wxID_EXIT,
    Minimal_About = wxID_ABOUT
};

// Define a new frame type: this is going to be our main frame
class MyFrame : public wxFrame
{
    wxTimer* TickTimer;
    wxTextCtrl* m_log;
    wxFileConfig* conf;

    float phys_TickRate = 1.0f / WorldEngine::TickRate;
    std::chrono::time_point<std::chrono::steady_clock> Now = std::chrono::steady_clock::now();
    std::chrono::time_point<std::chrono::steady_clock> startFrame = std::chrono::steady_clock::now();
    float deltaPhys = 0.f;
    float deltaFrame = 0.f;
    std::deque<float> Frames;
    //
    //	Push a new frame time into the list
    inline void PushFrameDelta(const float F) {
        Frames.push_back(F);
        if (Frames.size() > 30) {
            Frames.pop_front();
        }
    }
    //
    //	Return the average frame time from the list
    inline const float GetDeltaFrames() const {
        float DF = 0;
        for (auto& F : Frames) {
            DF += F;
        }
        return DF / Frames.size();
    }
    //
    std::deque<float> Phys;
    //
    //	Push a new Phys time into the list
    inline void PushPhysDelta(const float F) {
        Phys.push_back(F);
        if (Phys.size() > 30) {
            Phys.pop_front();
        }
    }
    //
    //	Return the average Phys time from the list
    inline const float GetDeltaPhys() const {
        float DF = 0;
        for (auto& F : Phys) {
            DF += F;
        }
        return DF / Phys.size();
    }

public:
    void OnTimer(wxTimerEvent& event);

    MyFrame(const wxString& title)
        : wxFrame(NULL, wxID_ANY, title, wxDefaultPosition, wxSize(640, 480))
    {
        // set the frame icon
        SetIcon(wxICON(sample));

        // create a menu bar
        wxMenu* fileMenu = new wxMenu;

        // the "About" item should be in the help menu
        wxMenu* helpMenu = new wxMenu;
        helpMenu->Append(Minimal_About, "&About\tF1", "Show about dialog");

        fileMenu->Append(Minimal_Quit, "E&xit\tAlt-X", "Quit this program");

        // now append the freshly created menu to the menu bar...
        wxMenuBar* menuBar = new wxMenuBar();
        menuBar->Append(fileMenu, "&File");
        menuBar->Append(helpMenu, "&Help");

        // ... and attach this menu bar to the frame
        SetMenuBar(menuBar);

        CreateStatusBar(3);
        SetStatusText("Server Application");

        m_log = new wxTextCtrl( this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxTE_MULTILINE | wxTE_READONLY);
        wxLog::SetActiveTarget(new wxLogTextCtrl(m_log));
        conf = new wxFileConfig(wxEmptyString, wxEmptyString, wxGetCwd() + "/settings.ini");
        conf->SetPath("/net");
        std::string ini_IP(conf->Read("ip", "127.0.0.1").c_str());
        const long ini_Send_Port = conf->ReadLong("send_port", 8000);
        const long ini_Recv_Port = conf->ReadLong("recv_port", 8001);
        WorldEngine::TickRate = conf->ReadLong("tickrate", 60);
        phys_TickRate = 1.0f / WorldEngine::TickRate;
        WorldEngine::ClientTimeout = std::chrono::seconds(conf->ReadLong("client_timeout", 300));
        wxLogMessage("[Load settings.ini]\n");
        wxLogMessage("\t[NET]");
        wxLogMessage("\t\tIP: %s", ini_IP);
        wxLogMessage("\t\tSend Port: %ld", ini_Send_Port);
        wxLogMessage("\t\tRecv Port: %ld", ini_Recv_Port);
        wxLogMessage("\t\tTickrate: %ld", WorldEngine::TickRate);
        wxLogMessage("\t\tClient Timeout: %ld\n", WorldEngine::ClientTimeout.count());
        wxLogMessage("\t[GAME]");
        conf->SetPath("/game");
        WorldEngine::CurrentGamemode = conf->Read("gamemode", "test_gamemode").ToStdString();
        WorldEngine::CurrentMap = conf->Read("map", "./media/models/newMap.gltf").ToStdString();
        WorldEngine::DefaultPlayerModel = conf->Read("player", "./media/models/brickFrank.gltf").ToStdString();
        wxLogMessage("\t\tMap File: %s", WorldEngine::CurrentMap.c_str());
        wxLogMessage("\t\tPlayer Model: %s\n", WorldEngine::DefaultPlayerModel.c_str());


        //
        //	Physics Initialization
        //btSetTaskScheduler(btGetOpenMPTaskScheduler());
        //btSetTaskScheduler(btGetTBBTaskScheduler());
        //btSetTaskScheduler(btGetPPLTaskScheduler());
        btSetTaskScheduler(btCreateDefaultTaskScheduler());
        //
        btDefaultCollisionConstructionInfo cci;
        cci.m_defaultMaxPersistentManifoldPoolSize = 102400;
        cci.m_defaultMaxCollisionAlgorithmPoolSize = 102400;
        WorldEngine::collisionConfiguration = new btDefaultCollisionConfiguration(cci);
        WorldEngine::dispatcher = new btCollisionDispatcherMt(WorldEngine::collisionConfiguration, 40);
        WorldEngine::broadphase = new btDbvtBroadphase();
        //
        //	Solver Pool
        btConstraintSolver* solvers[BT_MAX_THREAD_COUNT];
        int maxThreadCount = BT_MAX_THREAD_COUNT;
        for (int i = 0; i < maxThreadCount; ++i)
        {
            solvers[i] = new btSequentialImpulseConstraintSolverMt();
        }
        WorldEngine::solverPool = new btConstraintSolverPoolMt(solvers, maxThreadCount);
        btSequentialImpulseConstraintSolverMt* solver = new btSequentialImpulseConstraintSolverMt();
        //
        //	Create Dynamics World
        WorldEngine::dynamicsWorld = new btDiscreteDynamicsWorldMt(WorldEngine::dispatcher, WorldEngine::broadphase, WorldEngine::solverPool, solver, WorldEngine::collisionConfiguration);
        //
        //	Set World Properties
        WorldEngine::dynamicsWorld->setGravity(btVector3(0, -10, 0));
        WorldEngine::dynamicsWorld->setForceUpdateAllAabbs(false);
        WorldEngine::dynamicsWorld->getSolverInfo().m_solverMode = SOLVER_SIMD |
            //SOLVER_USE_WARMSTARTING |
            //SOLVER_RANDMIZE_ORDER |
            // SOLVER_INTERLEAVE_CONTACT_AND_FRICTION_CONSTRAINTS |
            // SOLVER_USE_2_FRICTION_DIRECTIONS |
            SOLVER_ENABLE_FRICTION_DIRECTION_CACHING |
            SOLVER_CACHE_FRIENDLY |
            SOLVER_DISABLE_IMPLICIT_CONE_FRICTION |
            //SOLVER_DISABLE_VELOCITY_DEPENDENT_FRICTION_DIRECTION |
            0;

        WorldEngine::dynamicsWorld->getSolverInfo().m_numIterations = 5;
        btSequentialImpulseConstraintSolverMt::s_allowNestedParallelForLoops = true;
        wxLogMessage("Physics Initialized");

        //
        //  LUA Initialization
        WorldEngine::LUA::Initialize();

        //
        //  SceneGraph Initialization
        WorldEngine::SceneGraph::Initialize();

        //
        //  Create the World
        WorldEngine::SceneGraph::_World = new WorldSceneNode(WorldEngine::CurrentMap.c_str());

        //
        //  Initialize NetCode
        WorldEngine::NetCode::Initialize(ini_IP.c_str(), ini_Send_Port, ini_Recv_Port);

        TickTimer = new wxTimer();
        TickTimer->Bind(wxEVT_TIMER, &MyFrame::OnTimer, this);
        TickTimer->Start(1000/WorldEngine::TickRate);
    }

    ~MyFrame()
    {
        wxLogMessage("[Shutting Down]");

        //
        //  Deinitialize LUA
        WorldEngine::LUA::Deinitialize();

        //
        //  Deinitialize SceneGraph
        WorldEngine::SceneGraph::Deinitiaize();

        //
        //  Deinitialize NetCode
        WorldEngine::NetCode::Deinitialize();

        //
        //  Delete the Physics World
        delete WorldEngine::dynamicsWorld;
        delete WorldEngine::solverPool;
        delete WorldEngine::broadphase;
        delete WorldEngine::dispatcher;
        delete WorldEngine::collisionConfiguration;
    }

    // event handlers (these functions should _not_ be virtual)
    void OnQuit(wxCommandEvent& event)
    {
        Close(true);
    }

    void OnAbout(wxCommandEvent& event)
    {
        wxMessageBox(wxString::Format
        (
            "All your base are belong to us.\n"
            "\n"
            "%s\n%s",
            wxVERSION_STRING,
            wxGetOsDescription()
        ),
            "About WorldEngine Server",
            wxOK | wxICON_INFORMATION,
            this);
    }

private:
    // any class wishing to process wxWidgets events must use this macro
    wxDECLARE_EVENT_TABLE();
};

void MyFrame::OnTimer(wxTimerEvent&)
{
    Now = std::chrono::steady_clock::now();
    deltaFrame = std::chrono::duration<float, std::milli>(Now - startFrame).count() / 1000.f;
    startFrame = Now;
    PushFrameDelta(deltaFrame);
    SetStatusText("Tick Time " + std::to_string(GetDeltaFrames() * 1000) + "ms");
    //==============================
    //
    // 
    //
    //  NetCode Tick
    WorldEngine::NetCode::Tick(startFrame);
    //
    //  SceneNode Tick
    WorldEngine::SceneGraph::Tick(startFrame);
    // 
    //  Update the world
    if (deltaFrame > 0.0f)
    {
        Now = std::chrono::steady_clock::now();
        WorldEngine::dynamicsWorld->stepSimulation(deltaFrame, 5, phys_TickRate);
        deltaPhys = std::chrono::duration<float, std::milli>(std::chrono::steady_clock::now() - Now).count() / 1000.f;
        PushPhysDelta(deltaPhys);
        SetStatusText("Phys Time " + std::to_string(GetDeltaPhys() * 1000) + "ms", 1);
    }
    //
    //
    //==============================
    SetStatusText("Scene Nodes: " + std::to_string(WorldEngine::SceneGraph::SceneNodes.size()), 2);
}

// the event tables connect the wxWidgets events with the functions (event
// handlers) which process them. It can be also done at run-time, but for the
// simple menu events like this the static method is much simpler.
wxBEGIN_EVENT_TABLE(MyFrame, wxFrame)
EVT_MENU(Minimal_Quit, MyFrame::OnQuit)
EVT_MENU(Minimal_About, MyFrame::OnAbout)
wxEND_EVENT_TABLE()

// Create a new application object: this macro will allow wxWidgets to create
// the application object during program execution (it's better than using a
// static object for many reasons) and also implements the accessor function
// wxGetApp() which will return the reference of the right type (i.e. MyApp and
// not wxApp)
wxIMPLEMENT_APP(MyApp);

// 'Main program' equivalent: the program execution "starts" here
bool MyApp::OnInit()
{
    // call the base class initialization method, currently it only parses a
    // few common command-line options but it could be do more in the future
    if (!wxApp::OnInit())
        return false;

    // create the main application window
    MyFrame* frame = new MyFrame("WorldEngine Server");

    // and show it (the frames, unlike simple controls, are not shown when
    // created initially)
    frame->Show(true);

    // success: wxApp::OnRun() will be called which will enter the main message
    // loop and the application will run. If we returned false here, the
    // application would exit immediately.
    return true;
}