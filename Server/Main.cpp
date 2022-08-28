#include <wx/wx.h>
#include <wx/fileconf.h>
#define _D_CORE_DLL
#define _D_NEWTON_DLL
#define _D_COLLISION_DLL
#include <ndNewton.h>
#include <KNet.hpp>

namespace WorldEngine
{
    namespace
    {
        ndWorld* _ndWorld;

        std::string CurrentMap;
    }

    ndWorld* GetPhysicsWorld()
    {
        return _ndWorld;
    }
}

#include "NetCode.hpp"

#include "Import_GLTF.hpp"
#include "SceneGraph.hpp"

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

    std::chrono::time_point<std::chrono::steady_clock> startFrame = std::chrono::high_resolution_clock::now();
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

public:
    void OnTimer(wxTimerEvent& event);

    MyFrame(const wxString& title)
        : wxFrame(NULL, wxID_ANY, title)
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

        // create a status bar just for fun (by default with 1 pane only)
        CreateStatusBar(2);
        SetStatusText("Server Application");

        m_log = new wxTextCtrl( this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxTE_MULTILINE | wxTE_READONLY);
        wxLog::SetActiveTarget(new wxLogTextCtrl(m_log));

        conf = new wxFileConfig(wxEmptyString, wxEmptyString, wxGetCwd() + "/settings.ini");
        conf->SetPath("/net");
        std::string ini_IP(conf->Read("ip", "127.0.0.1").c_str());
        const long ini_Send_Port = conf->ReadLong("send_port", 8000);
        const long ini_Recv_Port = conf->ReadLong("recv_port", 8001);
        const long ini_Tickrate = conf->ReadLong("tickrate", 60);
        wxLogMessage("[Load settings.ini]");
        wxLogMessage("\tIP: %s", ini_IP);
        wxLogMessage("\tSend Port: %ld", ini_Send_Port);
        wxLogMessage("\tRecv Port: %ld", ini_Recv_Port);
        wxLogMessage("\tTickrate: %ld\n", ini_Tickrate);
        conf->SetPath("/game");
        WorldEngine::CurrentMap = conf->Read("map", "./media/models/newMap.gltf").ToStdString();

        //
        //	Physics Initialization
        WorldEngine::_ndWorld = new ndWorld();
        WorldEngine::_ndWorld->SetThreadCount(std::thread::hardware_concurrency() - 2);
        WorldEngine::_ndWorld->SetSubSteps(3);
        WorldEngine::_ndWorld->SetSolverIterations(2);
        wxLogMessage("Physics Initialized");

        //
        //  World Initialization
        WorldEngine::SceneGraph::Initialize(WorldEngine::CurrentMap.c_str());

        //
        //  Initialize NetCode
        WorldEngine::NetCode::Initialize(ini_IP.c_str(), ini_Send_Port, ini_Recv_Port);

        TickTimer = new wxTimer();
        TickTimer->Bind(wxEVT_TIMER, &MyFrame::OnTimer, this);
        TickTimer->Start(1000/ini_Tickrate);
    }

    ~MyFrame()
    {
        wxLogMessage("[Shutting Down]");

        //
        //  Deinitialize NetCode
        WorldEngine::NetCode::Deinitialize();

        //
        //  Delete the Physics World
        delete WorldEngine::_ndWorld;
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
    startFrame = std::chrono::high_resolution_clock::now();
    PushFrameDelta(deltaFrame);
    //==============================
    //
    // 
    //  NetCode Tick
    WorldEngine::NetCode::Tick();

    // 
    //  Update the world
    if (deltaFrame > 0.0f)
    {
        WorldEngine::_ndWorld->Update(deltaFrame);
    }
    //
    //
    //==============================
    deltaFrame = std::chrono::duration<float, std::milli>(std::chrono::high_resolution_clock::now() - startFrame).count() / 1000.f;
    SetStatusText("Tick Time " + wxString(std::to_string(GetDeltaFrames() * 1000)) + "ms");
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