/******************************************************************************
 *
 * Project:  OpenCPN
 * Purpose:  Sonar Plugin (Simulator)
 * Author:   Nicholas John Koch
 *
 ******************************************************************************
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, version 3, or (at your option) any later
 * version of the license.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 ******************************************************************************/
#include <wx/wxprec.h>
#ifndef WX_PRECOMP
    #include <wx/wx.h>
#endif
    #include <sys/types.h>
#if defined(WIN32) || defined(_WIN32) || defined(__WIN32__)
    #include <Winsock2.h> // before Windows.h, else Winsock 1 conflict
    #include <Ws2tcpip.h> // needed for ip_mreq definition for multicast
    #include <Windows.h>
    #include <stdint.h>
#else
    #include <sys/socket.h>
    #include <netinet/in.h>
    #include <arpa/inet.h>
    #define IP_ROUTER_ALERT    5        
    #define IP_PKTOPTIONS      9
    #define IP_PMTUDISC        10        
    #define IP_MTU_DISCOVER    10       
    #define IP_RECVERR         11       


    /* IP_MTU_DISCOVER arguments.  */
    #define IP_PMTUDISC_DONT   0        /* Never send DF frames.  */
    #define IP_PMTUDISC_WANT   1        /* Use per route hints.  */
    #define IP_PMTUDISC_DO     2        /* Always DF.  */
    #define IP_PMTUDISC_PROBE  3        /* Ignore dst pmtu.  */
    #include <unistd.h> // for sleep()
#endif   

#ifdef __unix__
    #define CLOSE(x)    close(x);
#else
    #define CLOSE(x)    _close(x);
#endif

#include <wx/regex.h>

#include "SonarSimulator.h"


uint8_t sonar_data[51];        // Array holding the simulated rssi data
uint8_t depth_data[3];         // depth (two bytes)
#define ID_DEPTH 201        // ID as described by EchoZilla dev docs
#define ID_SONAR 202        // ID as described by EchoZilla dev docs
#define ID_IP_ADDRESS   1
#define ID_IP_PORT      2
#define MAX_DEPTH       5000.0 // cm

wxBEGIN_EVENT_TABLE(MainFrame, wxFrame)
    EVT_MENU(wxID_EXIT,  MainFrame::OnExit)
    EVT_TIMER(wxID_ANY, MainFrame::OnTimer)
wxEND_EVENT_TABLE()

wxIMPLEMENT_APP(SonarSimulator);

void MainFrame::fill_sonar_data(uint8_t *data) {
    for (uint8_t i = 0; i < 50; i++) {
        // Random data?
        if (m_ss->cbox->GetValue()) {
            data[i+1] = rand() % 201;
        } else {
            // Linear data
            data[i+1] = 200 - i * 4;
        }
    }
}
void SonarSimulator::sleep_ms(int milliseconds) { // cross-platform sleep function
#if defined(WIN32) || defined(_WIN32) || defined(__WIN32__)
    Sleep(milliseconds);
#elif _POSIX_C_SOURCE >= 199309L
    struct timespec ts;
    ts.tv_sec = milliseconds / 1000;
    ts.tv_nsec = (milliseconds % 1000) * 1000000;
    nanosleep(&ts, NULL);
#else
    if (milliseconds >= 1000)
      sleep(milliseconds / 1000);
    usleep((milliseconds % 1000) * 1000);
#endif
}
/**
 * Constructor
*/
SonarSimulator::SonarSimulator() {
    // Init variables
    m_total_bytes = 0;
}

/**
 * WX Init
*/
bool SonarSimulator::OnInit() {

    frame = new MainFrame( "Sonar Data Simulator", wxDefaultPosition, wxSize(-1, 320), this );

    // Main layouter
    wxBoxSizer* sizer = new wxBoxSizer(wxVERTICAL);
    // Slider for depth simulation
    m_slider = new wxSlider(frame, wxID_ANY, 0, 0, MAX_DEPTH, wxDefaultPosition, wxDefaultSize, wxSL_LABELS);
    // random data
    cbox = new wxCheckBox(frame, wxID_ANY, wxT("Random Data"));


    sizer->Add(m_slider, 0, wxEXPAND | wxALL, 10);
    sizer->Add(cbox, 0, wxEXPAND | wxALL, 10);

    wxBoxSizer* ip_sizer = new wxBoxSizer(wxHORIZONTAL);

    // UDP multicast IP address
    m_tc_ip_address = new wxTextCtrl(frame, ID_IP_ADDRESS, wxT(DEFAULT_ADDRESS), wxDefaultPosition, wxDefaultSize, wxTE_CENTRE,  IPAddressValidator());
    m_tc_ip_address->SetMaxLength(15);
    ip_sizer->Add(m_tc_ip_address, 0, wxEXPAND | wxALL, 10);

    // UDP multicats IP port
    m_tc_ip_port = new wxTextCtrl(frame, ID_IP_PORT, wxString::Format(wxT("%i"), DEFAULT_PORT), wxDefaultPosition, wxDefaultSize, wxTE_CENTRE, IntegerValidator());
    m_tc_ip_port->SetMaxLength(5);
    ip_sizer->Add(m_tc_ip_port, 0, wxEXPAND | wxALL, 10);

    wxButton* btn_set = new wxButton(frame, wxID_ANY, wxT("set it"));
    ip_sizer->Add(btn_set, 0, wxEXPAND | wxALL, 10);

    sizer->Add(ip_sizer, 0, wxEXPAND);

    frame->SetSizer(sizer);
    frame->Centre();
    frame->Show( true );

    

    m_tc_ip_address ->Bind(wxEVT_TEXT, &SonarSimulator::OnIPTyped, this);
    m_tc_ip_port    ->Bind(wxEVT_TEXT, &SonarSimulator::OnIPTyped, this);
    btn_set         ->Bind(wxEVT_BUTTON, &SonarSimulator::OnBtnSet, this);

    return true;
}

/**
 * Set button pressed
*/
void SonarSimulator::OnBtnSet(wxCommandEvent& event) {
    
    m_tc_ip_port->GetValue().ToInt(&frame->m_port);
    frame->m_group = m_tc_ip_address->GetValue();
    
    event.Skip();
}

/**
 * Validation helper for IP address and port input field
*/
void SonarSimulator::OnIPTyped(wxCommandEvent& event) {
    wxRegEx* re;        // we use regex here to check the entry
    
    switch (event.GetId()) {
        case ID_IP_ADDRESS:
            re = new wxRegEx("[0-9.]+");    // digits and dots allowed
            break;
        case ID_IP_PORT:
            re = new wxRegEx("[0-9]+");     // only digits allowed
            break;
        default:
            return;

    }

    wxTextCtrl* b   = (wxTextCtrl*)event.GetEventObject();
    wxValidator* v  = b->GetValidator();
    bool success    = v->Validate(b);       // check validity
    wxString test   = b->GetValue();
    
    long cp = b->GetInsertionPoint();       // get cursor position
    
    wxString result = wxEmptyString;        // holds the clean string

    size_t start;
    size_t length;
    
    while (re->Matches(test)) {
        wxString s = re->GetMatch(test);
        result.Append(s);
        re->GetMatch(&start, &length, 0);
        test = test.Mid(start + length);
    }

    b->ChangeValue(result);                 // set maybe changed value
    
    bool retest = v->Validate(b);           // revalidate

    // adjust cursor position
    if (retest && !success) {
        b->SetInsertionPoint(cp - 1);
    } else if (!retest && !success) {
        b->SetInsertionPoint(cp);
    }

    event.Skip();
}

/**
 * GUI window
*/
MainFrame::MainFrame(const wxString& title, const wxPoint& pos, const wxSize& size, SonarSimulator* ss)
        : wxFrame(NULL, wxID_ANY, title, pos, size), m_timer(this) {
    
    m_ss    = ss;
    m_port  = DEFAULT_PORT;
    m_group = DEFAULT_ADDRESS;
    CreateStatusBar();
    SetStatusText( "Boring Status here ..." );
    m_group = DEFAULT_ADDRESS; 
    
#if defined(WIN32) || defined(_WIN32) || defined(__WIN32__)
    WORD wVersionRequested = MAKEWORD(1,1);
    WSADATA wsaData;
    WSAStartup(wVersionRequested, &wsaData);
#endif
 

    fd = socket(AF_INET, SOCK_DGRAM, 0);
    if (fd < 0) {
        perror("socket");
        return ;
    }

    char val = IP_PMTUDISC_DO;
    setsockopt(fd, IPPROTO_IP, IP_MTU_DISCOVER, &val, sizeof(val));

    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(m_port);
    
    addrlen = sizeof(addr);
    
    sonar_data[0] = ID_SONAR;
    depth_data[0] = ID_DEPTH;

    m_timer.Start(100); // sending @ 10 Hz
 
}

/**
 * Bye, bye
*/
void MainFrame::OnExit(wxCommandEvent& event) {
#if defined(WIN32) || defined(_WIN32) || defined(__WIN32__)
    WSACleanup();
#endif
    CLOSE(true);
}

/**
 * Timer func
*/
void MainFrame::OnTimer(wxTimerEvent& WXUNUSED(event)) {
    int val = m_ss->m_slider->GetValue();
    depth_data[1] = val/100;
    depth_data[2] = val - (depth_data[1] * 100) ;

    if (!val) {
        return;
    }

    // prepare dummy data
    fill_sonar_data(sonar_data);

    addr.sin_addr.s_addr = inet_addr(const_cast<char*>((const char*)m_group.ToUTF8()));
    addr.sin_port = htons(m_port);
    int nbytes = sendto(
        fd,
        (const char*)sonar_data,
        51,
        0,
        (struct sockaddr*) &addr,
        addrlen
    );
    
    if (nbytes < 0) {
        perror("sendto");
        return;
    } else {
        m_ss->m_total_bytes += nbytes;
    }
    nbytes = sendto(
        fd,
        (const char*)depth_data,
        3,
        0,
        (struct sockaddr*) &addr,
        addrlen
    );
    if (nbytes < 0) {
        perror("sendto");
        return;
    } else {
        m_ss->m_total_bytes += nbytes;
    }
   
    // Update status to reflect activities
    SetStatusText(wxString::Format(wxT("Bytes sent: %.2fKB"), (float)m_ss->m_total_bytes / 1024.0));
}

/**
 * Validator for IP address entries
*/
bool IPAddressValidator::Validate(wxWindow *WXUNUSED(parent)) {
    wxTextCtrl* tc = (wxTextCtrl*)GetWindow();
    wxString val = tc->GetValue();

    wxRegEx r(wxT("^(25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)\\.(25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)\\.(25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)\\.(25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)$"));

    if (r.Matches(val)) {
        tc->SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_WINDOW));
        tc->Refresh();
        return true;
    } else {
        tc->SetBackgroundColour("pink");
        tc->Refresh();
        return false;
    }

    return true;
}
bool IPAddressValidator::TransferToWindow() {
    return true;
}

bool IPAddressValidator::TransferFromWindow() {
    return true;
}

/**
 * Port number validator
*/
bool IntegerValidator::Validate(wxWindow *WXUNUSED(parent)) {
    wxTextCtrl* tc = (wxTextCtrl*)GetWindow();
    wxString val = tc->GetValue();

    wxRegEx r(wxT("^[0-9]{4,5}$"));

    if (r.Matches(val)) {
        int n;
        val.ToInt(&n);
        if (n < 1024) {
            tc->SetBackgroundColour("pink");
        } else {
            tc->SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_WINDOW));
        }

        
        tc->Refresh();
        return true;
    } else {
        tc->SetBackgroundColour("pink");
        tc->Refresh();
        return false;
    }

    return true;
}
bool IntegerValidator::TransferToWindow() {
    return true;
}

bool IntegerValidator::TransferFromWindow() {
    return true;
}