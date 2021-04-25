// Test!.cpp
//

#include <wx/sizer.h>
#include <wx/wx.h>
#include <wx/timer.h>
#include <wx/event.h>
#include <wx/sound.h>
#include <vector>
#include <thread>
#include <random>
#include <tuple>
#include <fstream>
#include <Windows.h>
#include <string>

wxDEFINE_EVENT(COLLISION, wxCommandEvent);
wxDEFINE_EVENT(PAUSE, wxCommandEvent);
wxDEFINE_EVENT(UNPAUSE, wxCommandEvent);
wxDEFINE_EVENT(RESET, wxCommandEvent);

const int popSize = 50;
const int startDis = 45;

class Genome
{
public:

    //150, 110, 80, 0.005, 0
    int longJump = 500;
    int medJump = 500;
    int shortJump = 500;
    double adjustI = 0;
    double speedAdjust = 0;
};

class population
{
public:

    Genome mine;
    int fitness = 0;
    bool collided = false;
    bool increase = false;
    double gravity = 0;
    int a = 0;

    std::tuple<int, int, int, int> location;

    void adjust()
    {
        mine.speedAdjust += mine.adjustI;
    }
};


class BasicDrawPane;

class RenderTimer : public wxTimer
{
    BasicDrawPane* pane;
public:
    RenderTimer(BasicDrawPane* pane);
    void Notify();
    void start();
};


class BasicDrawPane : public wxWindow
{

public:
    BasicDrawPane(wxFrame* parent);

    void paintEvent(wxPaintEvent& evt);

    std::tuple<int, int, int, int> render(wxDC& dc);
    std::tuple<int, int, int, int> render2(wxDC& dc);
    std::tuple<int, int, int, int> renderE(wxDC& dc);
    std::tuple<int, int, int, int> renderEE(wxDC& dc);

    void renderScore(wxDC& dc);
    std::tuple<int, int, int, int> renderDino(wxDC& dc, population& Dino);
    void TriggerPaint(wxKeyEvent& event);
    void GayTrigger(wxKeyEvent& event);
    void AI(population& D, std::pair<int, int> dis);
    void reset(wxCommandEvent& evt);
    void dinoCount(wxDC& dc);

    bool increase = false;
    int a = 0;
    double gravity = 0;
    int aa = false, bb = false, cc = false, dd = false;
    int frames1 = 0, frames2 = 0, framesRan = 0, framesRan2 = 0;
    int wait1 = 0, wait2 = 0, wait3 = 0;
    double waitTime1 = startDis;
    double x_speed = -5;
    int score = 0, checkBit = 1;
    bool paused = false;
    int dinosLeft = popSize;

    std::ofstream outFile;

    double x, x1, x2, x3;

    int index = -1;

    wxCoord w = 40, h = 65;

    wxImage Front, Back, Cactus, smallCactus, wideCactus, WidestCactus;
    bool leg = false;
    const int switchTime = 8;
    int whenSwitch = switchTime;

    DECLARE_EVENT_TABLE()
};

std::vector<population> Play;

class MyFrame;

class MyApp : public wxApp
{
    bool OnInit();

    MyFrame* frame;
public:

};


RenderTimer::RenderTimer(BasicDrawPane* pane) : wxTimer()
{
    RenderTimer::pane = pane;
}

void RenderTimer::Notify()
{
    pane->Refresh();
}

void RenderTimer::start()
{
    wxTimer::Start(10);
}

IMPLEMENT_APP(MyApp)

class MyFrame : public wxFrame
{
    RenderTimer* timer;
    BasicDrawPane* drawPane;

public:
    MyFrame() : wxFrame((wxFrame*)NULL, -1, wxT("Dino Game"), wxPoint(50, 50), wxSize(1080, 550))
    {
        wxBoxSizer* sizer = new wxBoxSizer(wxHORIZONTAL);
        drawPane = new BasicDrawPane(this);
        sizer->Add(drawPane, 1, wxEXPAND);
        SetSizer(sizer);

        timer = new RenderTimer(drawPane);
        Show();
        timer->start();
    }
    ~MyFrame()
    {
        delete timer;
    }
    void onClose(wxCloseEvent& evt)
    {
        timer->Stop();
        evt.Skip();
    }

    void stopTimer(wxCommandEvent& evt)
    {
        timer->Stop();
        if (false)
        {
            wxMessageDialog dia(nullptr, "You've Lost!", "Fail", wxOK | wxCENTRE);
            dia.ShowModal();
            std::exit(0);
        }
        evt.Skip();
    }
    
    void pause(wxCommandEvent& evt)
    {
        timer->Stop();
    }

    void unPause(wxCommandEvent& evt)
    {
        timer->start();
    }

    DECLARE_EVENT_TABLE()
};


BEGIN_EVENT_TABLE(MyFrame, wxFrame)
    EVT_CLOSE(MyFrame::onClose)
    EVT_COMMAND(wxID_ANY, COLLISION, MyFrame::stopTimer)
    EVT_COMMAND(wxID_ANY, PAUSE, MyFrame::pause)
    EVT_COMMAND(wxID_ANY, UNPAUSE, MyFrame::unPause)
END_EVENT_TABLE()

bool MyApp::OnInit()
{
    ::wxInitAllImageHandlers();

    frame = new MyFrame();
    frame->Show();

    return true;
}


BEGIN_EVENT_TABLE(BasicDrawPane, wxWindow)
    EVT_PAINT(BasicDrawPane::paintEvent)
    EVT_KEY_DOWN(BasicDrawPane::TriggerPaint)
    EVT_KEY_UP(BasicDrawPane::GayTrigger)
    EVT_COMMAND(wxID_ANY, RESET, BasicDrawPane::reset)
END_EVENT_TABLE()



BasicDrawPane::BasicDrawPane(wxFrame* parent) :
    wxWindow(parent, wxID_ANY)
{
    Front.LoadFile("FrontLeg.png", wxBITMAP_TYPE_PNG);
    Front.Rescale(w + 15, h);
    Back.LoadFile("BackLeg.png", wxBITMAP_TYPE_PNG);
    Back.Rescale(w + 15, h);
    Cactus.LoadFile("Cactus.png", wxBITMAP_TYPE_PNG);
    Cactus.Rescale(38, 60);
    smallCactus.LoadFile("smallCactus.png", wxBITMAP_TYPE_PNG);
    smallCactus.Rescale(38, 48);
    wideCactus.LoadFile("WideCactus.png", wxBITMAP_TYPE_PNG);
    wideCactus.Rescale(55, 60);
    WidestCactus.LoadFile("WidestCactus.png", wxBITMAP_TYPE_PNG);
    WidestCactus.Rescale(68, 60);

    outFile.open("scores.txt");

    Play.resize(popSize);

    std::random_device rd;
    std::mt19937 mersenne(rd());

    std::uniform_int_distribution<int> first(-80, 80);
    std::uniform_real_distribution<double> second(0.001, 0.015);

    std::uniform_int_distribution<int> ran(0, 1);

    for (int i = 0; i < Play.size(); i++)
    {
        if (i != Play.size() - 1)
        {
            Play[i].mine.longJump += first(mersenne);
            Play[i].mine.medJump += first(mersenne);
            Play[i].mine.shortJump += first(mersenne);
            Play[i].mine.adjustI = second(mersenne);
        }
    }

    wxSize sz = GetClientSize();

    x = sz.x;
    x1 = sz.x - 40;
    x2 = sz.x;
    x3 = sz.x;
}

bool collision(std::tuple<int, int, int, int> c, std::vector<std::tuple<int, int, int, int>> cac)
{
    for (int i = 0; i < cac.size(); i++)
    {
        if (((std::get<0>(cac[i]) > std::get<0>(c) && std::get<0>(cac[i]) <= std::get<0>(c) + std::get<2>(c)) || std::get<0>(cac[i]) == std::get<0>(c)) &&
            ((std::get<1>(cac[i]) > std::get<1>(c) && std::get<1>(cac[i]) <= std::get<1>(c) + std::get<3>(c)) || std::get<1>(cac[i]) == std::get<1>(c)))
            return true;

        else if ((std::get<0>(cac[i]) < std::get<0>(c) && std::get<0>(cac[i]) + std::get<2>(cac[i]) >= std::get<0>(c)) &&
            ((std::get<1>(cac[i]) < std::get<1>(c) && std::get<1>(cac[i]) + std::get<3>(cac[i]) >= std::get<1>(c)) || std::get<1>(cac[i]) == std::get<1>(c)))
            return true;
    }

    return false;
}


void BasicDrawPane::paintEvent(wxPaintEvent& evt)
{
    wxPaintDC dc(this);

    std::random_device rd;
    std::mt19937 mersenne(rd());
    std::uniform_int_distribution<int> die(0, 12);

    //           x,   y,   w,   h
    std::tuple<int, int, int, int> a, b, ran, ran2;

    renderScore(dc);
    dinoCount(dc);
    score++;

    if (die(mersenne) % 4 == 0 && frames1 == 0 && wait1 == 0)
    {
        aa = true;
        wait1 = 1;
    }
    if (die(mersenne) % 6 == 0 && frames2 == 0 && wait1 == 0)
    {
        bb = true;
        wait1 = 1;
    }
    if (die(mersenne) % 3 == 0 && framesRan == 0 && wait1 == 0)
    {
        cc = true;
        wait1 = 1;
    }
    if (die(mersenne) % 5 == 0 && framesRan2 == 0 && wait1 == 0)
    {
        dd = true;
        wait1 = 1;
    }

    std::vector<std::tuple<int, int, int, int>> cac;

    if (aa)
    {
        a = render(dc);
        cac.push_back(a);
    }
    if (bb)
    {
        b = render2(dc);
        cac.push_back(b);
    }
    if (cc)
    {
        ran = renderE(dc);
        cac.push_back(ran);
    }
    if (dd)
    {
        ran2 = renderEE(dc);
        cac.push_back(ran2);
    }

    if (wait1 > 0)
        wait1++;

    if (wait1 > waitTime1)
    {
        wait1 = 0;
    }

    for (int i = 0; i < Play.size(); i++)
    {
        if (!Play[i].collided)
            Play[i].location = renderDino(dc, Play[i]);
    }

    int alive = 0;

    for (int i = 0; i < Play.size(); i++)
    {
        if (!Play[i].collided)
            Play[i].collided = collision(Play[i].location, cac);
        else
            Play[i].fitness = score;

        if (!Play[i].collided)
        {
            alive++;
            index = i;
        }
    }

    dinosLeft = alive;

    if (alive == 0)
    {
        outFile << Play[index].fitness << '\n';
        wxCommandEvent event(RESET);
        wxPostEvent(this, event);
        return;
    }

    int dino = std::get<0>(Play[index].location);
    std::pair<int, int> distance = std::make_pair(-1, -1);

    for (auto& i : cac)
    {
        if ((std::get<0>(i) - dino < distance.first && std::get<0>(i) - dino >= 0) || distance.first == -1)
        {
            distance.first = std::get<0>(i) - dino;
            distance.second = std::get<2>(i);
        }
    }

    for (int i = 0; i < Play.size(); i++)
    {
        if(!Play[i].collided)
            AI(Play[i], distance);
    }

    if (x_speed > -45)
    {
        x_speed -= 0.004;// 0.0020;
        waitTime1 -= .05;
    }
}

std::tuple<int, int, int, int> BasicDrawPane::render(wxDC& dc)
{
    wxCoord w = 15, h = 45;
    wxSize sz = GetClientSize();

    x += x_speed;

    frames1++;

    if (x < -w)
    {
        x = sz.x;
        frames1 = 0;
        aa = false;
    }


    dc.SetPen(*wxBLACK_PEN);
    dc.SetBrush(*wxBLACK_BRUSH);
    int y = wxMax(0, sz.y - (h + 0));
    dc.DrawBitmap(Cactus, x - 8, y-28, false);

  //  wxRect recToDraw(x, y, w, h);
  //  dc.DrawRectangle(recToDraw);

    return std::make_tuple(x, y, w, h);
}

void BasicDrawPane::TriggerPaint(wxKeyEvent& event)
{
  /*  if (event.GetKeyCode() == WXK_SPACE)
    {
        if (a == 0 || a <= 16)
        {
            increase = true;
            wxSound::Play(wxT("DinoJump.wav"), wxSOUND_ASYNC);
        }
    }*/
    if (event.GetKeyCode() == WXK_ESCAPE)
    {
        if (!paused)
        {
            paused = true;
            wxCommandEvent event(PAUSE);
            wxPostEvent(this, event);
        }
        else
        {
            paused = false;
            wxCommandEvent event(UNPAUSE);
            wxPostEvent(this, event);
        }
    }
    if (event.GetKeyCode() == WXK_DOWN)
    {
        h = 35;
    }
}

std::tuple<int, int, int, int> BasicDrawPane::renderDino(wxDC& dc, population& Dino)
{
    wxSize sz = GetClientSize();

    dc.SetPen(*wxBLACK_PEN);
    dc.SetBrush(*wxBLACK_BRUSH);
 
    int x = wxMax(0, (sz.x - w) / 11);
    int y = wxMax(0, (sz.y - (h + 20) - Dino.a));

    int max = 17;

    if (Dino.increase && Dino.gravity < max)
    {
        Dino.a += (max - int(Dino.gravity));
        Dino.gravity += 1.15;
    }
    else if (Dino.a > 0)
    {
        Dino.a -= 8;
        Dino.increase = false;
        Dino.gravity = 0;
    }
    else Dino.a = 0;

    if (leg)
    {
        dc.DrawBitmap(Front, x, y, false);
        whenSwitch--;
        if (whenSwitch == 0)
        {
            leg = !leg;
            whenSwitch = switchTime;
        }
    }
    else
    {
        dc.DrawBitmap(Back, x, y, false);
        whenSwitch--;
        if (whenSwitch == 0)
        {
            leg = !leg;
            whenSwitch = switchTime;
        }
    }

    return std::make_tuple(x, y, w, h);
}

std::tuple<int, int, int, int> BasicDrawPane::render2(wxDC& dc)
{
    wxCoord w = 15, h = 35;
    wxSize sz = GetClientSize();

    x1 += x_speed;

    frames2++;

    if (x1 < -w)
    {
        x1 = sz.x;
        frames2 = 0;
        bb = false;
    }


    dc.SetPen(*wxBLACK_PEN);
    dc.SetBrush(*wxBLACK_BRUSH);
    int y = wxMax(0, sz.y - (h + 20));

    dc.DrawBitmap(smallCactus, x1 - 8, y - 6, false);

  //  wxRect recToDraw(x1, y, w, h);
  //  dc.DrawRectangle(recToDraw);

    return std::make_tuple(x1, y, w, h);
}

void BasicDrawPane::GayTrigger(wxKeyEvent& event)
{
    if (event.GetKeyCode() == WXK_DOWN)
        if (h < 65)
            h = 65;
}


std::tuple<int, int, int, int> BasicDrawPane::renderE(wxDC& dc)
{
    wxCoord w = 35, h = 45;
    wxSize sz = GetClientSize();

    x2 += x_speed;

    framesRan++;

    if (x2 < -w)
    {
        x2 = sz.x;
        framesRan = 0;
        cc = false;
    }


    dc.SetPen(*wxBLACK_PEN);
    dc.SetBrush(*wxBLACK_BRUSH);
    int y = wxMax(0, sz.y - (h + 0));
    dc.DrawBitmap(wideCactus, x2 - 8, y - 28, false);

  //  wxRect recToDraw(x2, y, w, h);
  //  dc.DrawRectangle(recToDraw);

    return std::make_tuple(x2, y, w, h);
}

std::tuple<int, int, int, int> BasicDrawPane::renderEE(wxDC& dc)
{
    wxCoord w = 45, h = 45;
    wxSize sz = GetClientSize();

    x3 += x_speed;

    framesRan2++;

    if (x3 < -w)
    {
        x3 = sz.x;
        framesRan2 = 0;
        dd = false;
    }


    dc.SetPen(*wxBLACK_PEN);
    dc.SetBrush(*wxBLACK_BRUSH);
    int y = wxMax(0, sz.y - (h + 0));
    dc.DrawBitmap(WidestCactus, x3 - 8, y - 28, false);

  //  wxRect recToDraw(x3, y, w, h);
  //  dc.DrawRectangle(recToDraw);

    return std::make_tuple(x3, y, w, h);
}

void BasicDrawPane::renderScore(wxDC& dc)
{
    wxCoord w = 60, h = 20;
    wxSize sz = GetClientSize();

    double x = sz.x - 80;

    dc.SetPen(*wxBLACK_PEN);
    dc.SetBrush(*wxBLACK_BRUSH);
    int y = wxMax(0, sz.y - (h + 20));

    dc.DrawText(wxString("Score: " + std::to_string(score/12)) , wxPoint(x, sz.y - 480));

    if ((score / 14) == (checkBit*100))
    {
        checkBit++;
    }
}

void BasicDrawPane::dinoCount(wxDC& dc)
{
    wxCoord w = 60, h = 20;
    wxSize sz = GetClientSize();

    double x = sz.x - 220;

    dc.SetPen(*wxBLACK_PEN);
    dc.SetBrush(*wxBLACK_BRUSH);
    int y = wxMax(0, sz.y - (h + 20));

    dc.DrawText(wxString("Dinosaurs Left: " + std::to_string(dinosLeft)), wxPoint(x, sz.y - 480));
}

void BasicDrawPane::reset(wxCommandEvent& evt)
{
    increase = false;
    a = 0;
    gravity = 0;
    aa = false, bb = false, cc = false, dd = false;
    frames1 = 0, frames2 = 0, framesRan = 0, framesRan2 = 0;
    wait1 = 0, wait2 = 0, wait3 = 0, waitTime1 = startDis;
    x_speed = -5;
    score = 0, checkBit = 1;
    paused = false;
    dinosLeft = popSize;

    w = 40, h = 65;

    wxSize sz = GetClientSize();
    
    x = sz.x;
    x1 = sz.x - 40;
    x2 = sz.x;
    x3 = sz.x;

    leg = false;
    whenSwitch = switchTime;

    std::random_device rd;
    std::mt19937 mersenne(rd());

    std::uniform_int_distribution<int> first(-80, 80);
    std::uniform_real_distribution<double> second(0.001, 0.015);

    std::uniform_int_distribution<int> ran(0, 1);

    for (int i = 0; i < Play.size(); i++)
    {
        Play[i].fitness = 0;
        Play[i].collided = false;
        Play[i].increase = false;
        Play[i].gravity = 0;
        Play[i].a = 0;

        if (index != -1)
        {
            Play[i].mine.shortJump = Play[index].mine.shortJump;
            Play[i].mine.medJump = Play[index].mine.shortJump;
            Play[i].mine.longJump = Play[index].mine.longJump;
            Play[i].mine.adjustI = Play[index].mine.adjustI;
        }

        if (i != Play.size() - 1)
        {
            Play[i].mine.longJump += first(mersenne);
            Play[i].mine.medJump += first(mersenne);
            Play[i].mine.shortJump += first(mersenne);
            Play[i].mine.adjustI = second(mersenne);
        }
    }

    index = -1;
}


void BasicDrawPane::AI(population& D, std::pair<int, int> dis)
{
    if (dis.first >= 0 && (D.a == 0 || D.a <= 16))
    {
        if(dis.second > 35 && dis.first <= (D.mine.longJump + D.mine.speedAdjust))
            D.increase = true;
        else if(dis.second == 35 && dis.first <= (D.mine.longJump + D.mine.speedAdjust))
            D.increase = true;
        else if (dis.first <= (D.mine.shortJump + D.mine.speedAdjust))
            D.increase = true;

    //    wxSound::Play(wxT("DinoJump.wav"), wxSOUND_ASYNC);
    }
    D.adjust();
}



//Use this bottom code if you want to play the game yourself!


/*

// Test!.cpp
//

#include <wx/sizer.h>
#include <wx/wx.h>
#include <wx/timer.h>
#include <wx/event.h>
#include <wx/sound.h>
#include <vector>
#include <thread>
#include <random>
#include <tuple>
#include <fstream>
#include <Windows.h>
#include <string>

wxDEFINE_EVENT(COLLISION, wxCommandEvent);
wxDEFINE_EVENT(PAUSE, wxCommandEvent);
wxDEFINE_EVENT(UNPAUSE, wxCommandEvent);
wxDEFINE_EVENT(RESET, wxCommandEvent);

const int popSize = 50;
const int startDis = 45;

class Genome
{
public:

    //150, 110, 80, 0.005, 0
    int longJump = 500;
    int medJump = 500;
    int shortJump = 500;
    double adjustI = 0;
    double speedAdjust = 0;
};

class population
{
public:

    Genome mine;
    int fitness = 0;
    bool collided = false;
    bool increase = false;
    double gravity = 0;
    int a = 0;

    std::tuple<int, int, int, int> location;

    void adjust()
    {
        mine.speedAdjust += mine.adjustI;
    }
};


class BasicDrawPane;

class RenderTimer : public wxTimer
{
    BasicDrawPane* pane;
public:
    RenderTimer(BasicDrawPane* pane);
    void Notify();
    void start();
};


class BasicDrawPane : public wxWindow
{

public:
    BasicDrawPane(wxFrame* parent);

    void paintEvent(wxPaintEvent& evt);

    std::tuple<int, int, int, int> render(wxDC& dc);
    std::tuple<int, int, int, int> render2(wxDC& dc);
    std::tuple<int, int, int, int> renderE(wxDC& dc);
    std::tuple<int, int, int, int> renderEE(wxDC& dc);

    void renderScore(wxDC& dc);
    std::tuple<int, int, int, int> renderDino(wxDC& dc, population& Dino);
    void TriggerPaint(wxKeyEvent& event);
    void GayTrigger(wxKeyEvent& event);
   // void AI(population& D, std::pair<int, int> dis);
    void reset(wxCommandEvent& evt);
    void dinoCount(wxDC& dc);

    bool increase = false;
    int a = 0;
    double gravity = 0;
    int aa = false, bb = false, cc = false, dd = false;
    int frames1 = 0, frames2 = 0, framesRan = 0, framesRan2 = 0;
    int wait1 = 0, wait2 = 0, wait3 = 0;
    double waitTime1 = startDis;
    double x_speed = -5;
    int score = 0, checkBit = 1;
    bool paused = false;
    int dinosLeft = popSize;

    std::ofstream outFile;

    double x, x1, x2, x3;

    int index = -1;

    wxCoord w = 40, h = 65;

    wxImage Front, Back, Cactus, smallCactus, wideCactus, WidestCactus;
    bool leg = false;
    const int switchTime = 8;
    int whenSwitch = switchTime;

    DECLARE_EVENT_TABLE()
};

std::vector<population> Play;

class MyFrame;

class MyApp : public wxApp
{
    bool OnInit();

    MyFrame* frame;
public:

};


RenderTimer::RenderTimer(BasicDrawPane* pane) : wxTimer()
{
    RenderTimer::pane = pane;
}

void RenderTimer::Notify()
{
    pane->Refresh();
}

void RenderTimer::start()
{
    wxTimer::Start(10);
}

IMPLEMENT_APP(MyApp)

class MyFrame : public wxFrame
{
    RenderTimer* timer;
    BasicDrawPane* drawPane;

public:
    MyFrame() : wxFrame((wxFrame*)NULL, -1, wxT("Dino Game"), wxPoint(50, 50), wxSize(1080, 550))
    {
        wxBoxSizer* sizer = new wxBoxSizer(wxHORIZONTAL);
        drawPane = new BasicDrawPane(this);
        sizer->Add(drawPane, 1, wxEXPAND);
        SetSizer(sizer);

        timer = new RenderTimer(drawPane);
        Show();
        timer->start();
    }
    ~MyFrame()
    {
        delete timer;
    }
    void onClose(wxCloseEvent& evt)
    {
        timer->Stop();
        evt.Skip();
    }

    void stopTimer(wxCommandEvent& evt)
    {
        timer->Stop();
        if (false)
        {
            wxMessageDialog dia(nullptr, "You've Lost!", "Fail", wxOK | wxCENTRE);
            dia.ShowModal();
            std::exit(0);
        }
        evt.Skip();
    }

    void pause(wxCommandEvent& evt)
    {
        timer->Stop();
    }

    void unPause(wxCommandEvent& evt)
    {
        timer->start();
    }

    DECLARE_EVENT_TABLE()
};


BEGIN_EVENT_TABLE(MyFrame, wxFrame)
    EVT_CLOSE(MyFrame::onClose)
    EVT_COMMAND(wxID_ANY, COLLISION, MyFrame::stopTimer)
    EVT_COMMAND(wxID_ANY, PAUSE, MyFrame::pause)
    EVT_COMMAND(wxID_ANY, UNPAUSE, MyFrame::unPause)
END_EVENT_TABLE()

bool MyApp::OnInit()
{
    ::wxInitAllImageHandlers();

    frame = new MyFrame();
    frame->Show();

    return true;
}


BEGIN_EVENT_TABLE(BasicDrawPane, wxWindow)
    EVT_PAINT(BasicDrawPane::paintEvent)
    EVT_KEY_DOWN(BasicDrawPane::TriggerPaint)
    EVT_KEY_UP(BasicDrawPane::GayTrigger)
    EVT_COMMAND(wxID_ANY, RESET, BasicDrawPane::reset)
END_EVENT_TABLE()



BasicDrawPane::BasicDrawPane(wxFrame* parent) :
    wxWindow(parent, wxID_ANY)
{
    Front.LoadFile("FrontLeg.png", wxBITMAP_TYPE_PNG);
    Front.Rescale(w + 15, h);
    Back.LoadFile("BackLeg.png", wxBITMAP_TYPE_PNG);
    Back.Rescale(w + 15, h);
    Cactus.LoadFile("Cactus.png", wxBITMAP_TYPE_PNG);
    Cactus.Rescale(38, 60);
    smallCactus.LoadFile("smallCactus.png", wxBITMAP_TYPE_PNG);
    smallCactus.Rescale(38, 48);
    wideCactus.LoadFile("WideCactus.png", wxBITMAP_TYPE_PNG);
    wideCactus.Rescale(55, 60);
    WidestCactus.LoadFile("WidestCactus.png", wxBITMAP_TYPE_PNG);
    WidestCactus.Rescale(68, 60);

    outFile.open("scores.txt");

    //Play.resize(popSize);
    Play.resize(1);

    std::random_device rd;
    std::mt19937 mersenne(rd());

    std::uniform_int_distribution<int> first(-80, 80);
    std::uniform_real_distribution<double> second(0.001, 0.015);

    std::uniform_int_distribution<int> ran(0, 1);

    for (int i = 0; i < Play.size(); i++)
    {
        if (i != Play.size() - 1)
        {
            Play[i].mine.longJump += first(mersenne);
            Play[i].mine.medJump += first(mersenne);
            Play[i].mine.shortJump += first(mersenne);
            Play[i].mine.adjustI = second(mersenne);
        }
    }

    wxSize sz = GetClientSize();

    x = sz.x;
    x1 = sz.x - 40;
    x2 = sz.x;
    x3 = sz.x;
}

bool collision(std::tuple<int, int, int, int> c, std::vector<std::tuple<int, int, int, int>> cac)
{
    for (int i = 0; i < cac.size(); i++)
    {
        if (((std::get<0>(cac[i]) > std::get<0>(c) && std::get<0>(cac[i]) <= std::get<0>(c) + std::get<2>(c)) || std::get<0>(cac[i]) == std::get<0>(c)) &&
            ((std::get<1>(cac[i]) > std::get<1>(c) && std::get<1>(cac[i]) <= std::get<1>(c) + std::get<3>(c)) || std::get<1>(cac[i]) == std::get<1>(c)))
            return true;

        else if ((std::get<0>(cac[i]) < std::get<0>(c) && std::get<0>(cac[i]) + std::get<2>(cac[i]) >= std::get<0>(c)) &&
            ((std::get<1>(cac[i]) < std::get<1>(c) && std::get<1>(cac[i]) + std::get<3>(cac[i]) >= std::get<1>(c)) || std::get<1>(cac[i]) == std::get<1>(c)))
            return true;
    }

    return false;
}


void BasicDrawPane::paintEvent(wxPaintEvent& evt)
{
    wxPaintDC dc(this);

    std::random_device rd;
    std::mt19937 mersenne(rd());
    std::uniform_int_distribution<int> die(0, 12);

    //           x,   y,   w,   h
    std::tuple<int, int, int, int> a, b, ran, ran2;

    renderScore(dc);
    dinoCount(dc);
    score++;

    if (die(mersenne) % 4 == 0 && frames1 == 0 && wait1 == 0)
    {
        aa = true;
        wait1 = 1;
    }
    if (die(mersenne) % 6 == 0 && frames2 == 0 && wait1 == 0)
    {
        bb = true;
        wait1 = 1;
    }
    if (die(mersenne) % 3 == 0 && framesRan == 0 && wait1 == 0)
    {
        cc = true;
        wait1 = 1;
    }
    if (die(mersenne) % 5 == 0 && framesRan2 == 0 && wait1 == 0)
    {
        dd = true;
        wait1 = 1;
    }

    std::vector<std::tuple<int, int, int, int>> cac;

    if (aa)
    {
        a = render(dc);
        cac.push_back(a);
    }
    if (bb)
    {
        b = render2(dc);
        cac.push_back(b);
    }
    if (cc)
    {
        ran = renderE(dc);
        cac.push_back(ran);
    }
    if (dd)
    {
        ran2 = renderEE(dc);
        cac.push_back(ran2);
    }

    if (wait1 > 0)
        wait1++;

    if (wait1 > waitTime1)
    {
        wait1 = 0;
    }

    for (int i = 0; i < Play.size(); i++)
    {
        if (!Play[i].collided)
            Play[i].location = renderDino(dc, Play[i]);
    }

    int alive = 0;

    for (int i = 0; i < Play.size(); i++)
    {
        if (!Play[i].collided)
            Play[i].collided = collision(Play[i].location, cac);
        else
            Play[i].fitness = score;

        if (!Play[i].collided)
        {
            alive++;
            index = i;
        }
    }

    dinosLeft = alive;

    if (alive == 0)
    {
        outFile << Play[index].fitness << '\n';
        wxCommandEvent event(RESET);
        wxPostEvent(this, event);
        return;
    }

    int dino = std::get<0>(Play[index].location);
    std::pair<int, int> distance = std::make_pair(-1, -1);

    for (auto& i : cac)
    {
        if ((std::get<0>(i) - dino < distance.first && std::get<0>(i) - dino >= 0) || distance.first == -1)
        {
            distance.first = std::get<0>(i) - dino;
            distance.second = std::get<2>(i);
        }
    }

 /*   for (int i = 0; i < Play.size(); i++)
    {
        if(!Play[i].collided)
            AI(Play[i], distance);
    }*

if (x_speed > -45)
{
    x_speed -= 0.004;// 0.0020;
    waitTime1 -= .05;
}
}

std::tuple<int, int, int, int> BasicDrawPane::render(wxDC& dc)
{
    wxCoord w = 15, h = 45;
    wxSize sz = GetClientSize();

    x += x_speed;

    frames1++;

    if (x < -w)
    {
        x = sz.x;
        frames1 = 0;
        aa = false;
    }


    dc.SetPen(*wxBLACK_PEN);
    dc.SetBrush(*wxBLACK_BRUSH);
    int y = wxMax(0, sz.y - (h + 0));
    dc.DrawBitmap(Cactus, x - 8, y - 28, false);

    //  wxRect recToDraw(x, y, w, h);
    //  dc.DrawRectangle(recToDraw);

    return std::make_tuple(x, y, w, h);
}

void BasicDrawPane::TriggerPaint(wxKeyEvent& event)
{
    if (event.GetKeyCode() == WXK_SPACE)
    {
        if (a == 0 || a <= 16)
        {
            Play[0].increase = true;
            wxSound::Play(wxT("DinoJump.wav"), wxSOUND_ASYNC);
        }
    }
    if (event.GetKeyCode() == WXK_ESCAPE)
    {
        if (!paused)
        {
            paused = true;
            wxCommandEvent event(PAUSE);
            wxPostEvent(this, event);
        }
        else
        {
            paused = false;
            wxCommandEvent event(UNPAUSE);
            wxPostEvent(this, event);
        }
    }
    if (event.GetKeyCode() == WXK_DOWN)
    {
        h = 35;
    }
}

std::tuple<int, int, int, int> BasicDrawPane::renderDino(wxDC& dc, population& Dino)
{
    wxSize sz = GetClientSize();

    dc.SetPen(*wxBLACK_PEN);
    dc.SetBrush(*wxBLACK_BRUSH);

    int x = wxMax(0, (sz.x - w) / 11);
    int y = wxMax(0, (sz.y - (h + 20) - Dino.a));

    int max = 17;

    if (Dino.increase && Dino.gravity < max)
    {
        Dino.a += (max - int(Dino.gravity));
        Dino.gravity += 1.15;
    }
    else if (Dino.a > 0)
    {
        Dino.a -= 8;
        Dino.increase = false;
        Dino.gravity = 0;
    }
    else Dino.a = 0;

    if (leg)
    {
        dc.DrawBitmap(Front, x, y, false);
        whenSwitch--;
        if (whenSwitch == 0)
        {
            leg = !leg;
            whenSwitch = switchTime;
        }
    }
    else
    {
        dc.DrawBitmap(Back, x, y, false);
        whenSwitch--;
        if (whenSwitch == 0)
        {
            leg = !leg;
            whenSwitch = switchTime;
        }
    }

    return std::make_tuple(x, y, w, h);
}

std::tuple<int, int, int, int> BasicDrawPane::render2(wxDC& dc)
{
    wxCoord w = 15, h = 35;
    wxSize sz = GetClientSize();

    x1 += x_speed;

    frames2++;

    if (x1 < -w)
    {
        x1 = sz.x;
        frames2 = 0;
        bb = false;
    }


    dc.SetPen(*wxBLACK_PEN);
    dc.SetBrush(*wxBLACK_BRUSH);
    int y = wxMax(0, sz.y - (h + 20));

    dc.DrawBitmap(smallCactus, x1 - 8, y - 6, false);

    //  wxRect recToDraw(x1, y, w, h);
    //  dc.DrawRectangle(recToDraw);

    return std::make_tuple(x1, y, w, h);
}

void BasicDrawPane::GayTrigger(wxKeyEvent& event)
{
    if (event.GetKeyCode() == WXK_DOWN)
        if (h < 65)
            h = 65;
}


std::tuple<int, int, int, int> BasicDrawPane::renderE(wxDC& dc)
{
    wxCoord w = 35, h = 45;
    wxSize sz = GetClientSize();

    x2 += x_speed;

    framesRan++;

    if (x2 < -w)
    {
        x2 = sz.x;
        framesRan = 0;
        cc = false;
    }


    dc.SetPen(*wxBLACK_PEN);
    dc.SetBrush(*wxBLACK_BRUSH);
    int y = wxMax(0, sz.y - (h + 0));
    dc.DrawBitmap(wideCactus, x2 - 8, y - 28, false);

    //  wxRect recToDraw(x2, y, w, h);
    //  dc.DrawRectangle(recToDraw);

    return std::make_tuple(x2, y, w, h);
}

std::tuple<int, int, int, int> BasicDrawPane::renderEE(wxDC& dc)
{
    wxCoord w = 45, h = 45;
    wxSize sz = GetClientSize();

    x3 += x_speed;

    framesRan2++;

    if (x3 < -w)
    {
        x3 = sz.x;
        framesRan2 = 0;
        dd = false;
    }


    dc.SetPen(*wxBLACK_PEN);
    dc.SetBrush(*wxBLACK_BRUSH);
    int y = wxMax(0, sz.y - (h + 0));
    dc.DrawBitmap(WidestCactus, x3 - 8, y - 28, false);

    //  wxRect recToDraw(x3, y, w, h);
    //  dc.DrawRectangle(recToDraw);

    return std::make_tuple(x3, y, w, h);
}

void BasicDrawPane::renderScore(wxDC& dc)
{
    wxCoord w = 60, h = 20;
    wxSize sz = GetClientSize();

    double x = sz.x - 80;

    dc.SetPen(*wxBLACK_PEN);
    dc.SetBrush(*wxBLACK_BRUSH);
    int y = wxMax(0, sz.y - (h + 20));

    dc.DrawText(wxString("Score: " + std::to_string(score / 12)), wxPoint(x, sz.y - 480));

    if ((score / 14) == (checkBit * 100))
    {
        checkBit++;
    }
}

void BasicDrawPane::dinoCount(wxDC& dc)
{
    wxCoord w = 60, h = 20;
    wxSize sz = GetClientSize();

    double x = sz.x - 220;

    dc.SetPen(*wxBLACK_PEN);
    dc.SetBrush(*wxBLACK_BRUSH);
    int y = wxMax(0, sz.y - (h + 20));

    dc.DrawText(wxString("Dinosaurs Left: " + std::to_string(dinosLeft)), wxPoint(x, sz.y - 480));
}

void BasicDrawPane::reset(wxCommandEvent& evt)
{
    increase = false;
    a = 0;
    gravity = 0;
    aa = false, bb = false, cc = false, dd = false;
    frames1 = 0, frames2 = 0, framesRan = 0, framesRan2 = 0;
    wait1 = 0, wait2 = 0, wait3 = 0, waitTime1 = startDis;
    x_speed = -5;
    score = 0, checkBit = 1;
    paused = false;
    dinosLeft = popSize;

    w = 40, h = 65;

    wxSize sz = GetClientSize();

    x = sz.x;
    x1 = sz.x - 40;
    x2 = sz.x;
    x3 = sz.x;

    leg = false;
    whenSwitch = switchTime;

    std::random_device rd;
    std::mt19937 mersenne(rd());

    std::uniform_int_distribution<int> first(-80, 80);
    std::uniform_real_distribution<double> second(0.001, 0.015);

    std::uniform_int_distribution<int> ran(0, 1);

    for (int i = 0; i < Play.size(); i++)
    {
        Play[i].fitness = 0;
        Play[i].collided = false;
        Play[i].increase = false;
        Play[i].gravity = 0;
        Play[i].a = 0;

        if (index != -1)
        {
            Play[i].mine.shortJump = Play[index].mine.shortJump;
            Play[i].mine.medJump = Play[index].mine.shortJump;
            Play[i].mine.longJump = Play[index].mine.longJump;
            Play[i].mine.adjustI = Play[index].mine.adjustI;
        }

        if (i != Play.size() - 1)
        {
            Play[i].mine.longJump += first(mersenne);
            Play[i].mine.medJump += first(mersenne);
            Play[i].mine.shortJump += first(mersenne);
            Play[i].mine.adjustI = second(mersenne);
        }
    }

    index = -1;
}


/*void BasicDrawPane::AI(population& D, std::pair<int, int> dis)
{
    if (dis.first >= 0 && (D.a == 0 || D.a <= 16))
    {
        if(dis.second > 35 && dis.first <= (D.mine.longJump + D.mine.speedAdjust))
            D.increase = true;
        else if(dis.second == 35 && dis.first <= (D.mine.longJump + D.mine.speedAdjust))
            D.increase = true;
        else if (dis.first <= (D.mine.shortJump + D.mine.speedAdjust))
            D.increase = true;

    //    wxSound::Play(wxT("DinoJump.wav"), wxSOUND_ASYNC);
    }
    D.adjust();
}


*/