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
#include "sonar_pi.h"

#include <wx/aui/framemanager.h>
#include <wx/aui/dockart.h>

#include <algorithm>
#include <stdlib.h>
#include <time.h>
#include "SonarPane.h"
#include "SonarDisplayWindow.h"

#define ID_DETACH 1
#define ID_PREFS  2
#define ID_SHOW_BAR 3
#define ID_HIDE_BAR  4

PLUGIN_BEGIN_NAMESPACE
enum { TIMER_ID = 1 };

wxDEFINE_EVENT(NEW_DATA_TYPE, wxCommandEvent);
wxDEFINE_EVENT(NEW_DEPTH_TYPE, wxCommandEvent);


BEGIN_EVENT_TABLE(SonarDisplayWindow, wxPanel)
    //EVT_BUTTON(wxID_CLOSE, SonarDisplayWindow::OnClose)
    EVT_CLOSE(SonarDisplayWindow::OnClose)
    EVT_TIMER(TIMER_ID, SonarDisplayWindow::OnTimer)
    EVT_TOGGLEBUTTON(wxID_ANY, SonarDisplayWindow::OnBtnMonochrome)
    EVT_CONTEXT_MENU(SonarDisplayWindow::OnContextMenu)
    EVT_MENU(wxID_ANY, SonarDisplayWindow::OnContextMenuSelect)
    EVT_LEFT_DOWN(SonarDisplayWindow::OnLeftClick)
END_EVENT_TABLE()

    SonarDisplayWindow::SonarDisplayWindow(wxWindow* w_parent, const wxString& title, sonar_pi* sp, wxAuiManager* aui_mgr)
    : wxPanel(w_parent, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL, wxEmptyString) {
    m_pi            = sp;
    m_aui_mgr       = aui_mgr;
    m_menu_docked   = nullptr;
    m_menu_floating = nullptr;
    m_depth_avail   = false;
    m_frequency     = m_pi->m_sonar_frequency;
    m_display_mode  = MULTICOLOR;
    
    
    m_sizer = new wxBoxSizer(wxVERTICAL);
    t_label1 = new wxStaticText(this, wxID_ANY, wxT("0.00kn (0.0m)"));

    // Control Panel
    m_st_slider = new wxStaticText(this, wxID_ANY, wxT("Freq. (kHz):"));
    m_slider = new wxSlider(this, wxID_ANY, 150, 100, 250, wxDefaultPosition, wxDefaultSize, wxSL_LABELS);
    m_slider->SetTick(10);
    m_slider->SetTickFreq(10);
    
    m_cp_sizer = new wxBoxSizer(wxHORIZONTAL);
    //m_btnOCLOSE = new wxButton(this, wxID_CLOSE, wxT("close"));

    m_tgl_monochrome = new wxToggleButton(this, wxID_ANY, wxT("Monochrome"));

    m_cp_sizer->Add(m_st_slider, 0, wxALIGN_LEFT | wxALIGN_CENTER_VERTICAL | wxLEFT | wxBOTTOM | wxTOP, 5);
    m_cp_sizer->Add(m_slider, 1, wxALIGN_CENTER_VERTICAL | wxBOTTOM | wxTOP, 5);
    m_cp_sizer->Add(m_tgl_monochrome, 0, wxALIGN_RIGHT | wxALIGN_CENTER_VERTICAL | wxRIGHT | wxBOTTOM | wxTOP, 5);

    m_sizer->Add(t_label1, 0, wxALIGN_CENTRE_VERTICAL | wxEXPAND);
    dummy = new wxPanel(this);
    wxStaticText* lbl_dummy = new wxStaticText(dummy, wxID_ANY, wxT("OpenGL is not enabled. No sonar data will be displayed. Depth information may be availabe."));
    wxBoxSizer* szr = new wxBoxSizer(wxVERTICAL);
    szr->Add(lbl_dummy, 0, wxEXPAND | wxALIGN_CENTER_HORIZONTAL);
    dummy->SetSizer(szr);
    m_sizer->Add(dummy, 1, wxEXPAND | wxALIGN_CENTER_HORIZONTAL);
    
    m_sizer->Add(m_cp_sizer, 0, wxEXPAND | wxALIGN_RIGHT | wxALIGN_BOTTOM);
    SetSizer(m_sizer);

    //DimeWindow(this);
    Fit();
    Layout();

    wxString name = wxString::Format(wxT("SONAR v%d.%d.%d-%s"), PLUGIN_VERSION_MAJOR, PLUGIN_VERSION_MINOR, PLUGIN_VERSION_PATCH, PLUGIN_VERSION_TWEAK);
    
    // Let the AUI manager know our panel
    wxAuiPaneInfo pane_info = wxAuiPaneInfo()
                        .Name(name)
                        .Caption(name)
                        .CaptionVisible(true)
                        .Movable(true)
                        .TopDockable(false)
                        .BottomDockable(true)
                        .LeftDockable(false)
                        .RightDockable(true)
                        .CloseButton(true)
                        .Centre()
                        .MinSize(wxSize(140, 180))
                        .FloatingSize(wxSize(420, 240))
                        .Float()
#ifndef __WXMAC__
                        .Hide()
#endif
                        .Dockable(true);
    
    m_aui_mgr->AddPane(this, pane_info);
    m_aui_mgr->Update();


    // Instantiate whether or not OpenGL available, so it can take the data upon data receiver start.
    sopa = new SonarPane(this);

    // one second timer for NMEA message sending
    m_timer = new wxTimer(this, TIMER_ID);

    m_aui_mgr->Bind(wxEVT_AUI_PANE_CLOSE, &SonarDisplayWindow::FrameClosed, this);

    // We have to attach it here after initialization (and not EVT table)
    Bind(wxEVT_SIZE, &SonarDisplayWindow::OnSize, this);

    m_slider->Bind(wxEVT_SCROLL_CHANGED, &SonarDisplayWindow::OnSliderRelease, this);
    
}


void SonarDisplayWindow::OnSliderRelease(wxScrollEvent& event) {
    StopDataReceiver();
    m_frequency = event.GetInt();
    StartDataReceiver(false);
}
void SonarDisplayWindow::FrameClosed(wxAuiManagerEvent& event) {
    m_pi->closing();
}

void SonarDisplayWindow::OnLeftClick(wxMouseEvent& event) {
        wxAuiPaneInfo &p = m_aui_mgr->GetPane(this);

    if (p.IsDocked()) {
        if (!m_menu_docked) {
            m_menu_docked = new wxMenu;
            m_menu_docked->Append(ID_DETACH, wxT("Undock"));
            
            
            m_menu_docked->Append(ID_HIDE_BAR, wxT("Hide Bar"));
        
            m_menu_docked->Append(ID_SHOW_BAR, wxT("Show Bar"));
            m_menu_docked->AppendSeparator();
            m_menu_docked->Append(ID_PREFS, wxT("Preferences"));
            
        }
        PopupMenu(m_menu_docked);
    } else {
        if (!m_menu_floating) {
            m_menu_floating = new wxMenu;
            
            
            m_menu_floating->Append(ID_HIDE_BAR, wxT("Hide Bar"));
        
            m_menu_floating->Append(ID_SHOW_BAR, wxT("Show Bar"));
            m_menu_floating->AppendSeparator();
            m_menu_floating->Append(ID_PREFS, wxT("Preferences"));
        }
        PopupMenu(m_menu_floating);
    }

}

void SonarDisplayWindow::OnContextMenu(wxContextMenuEvent& event) {

    wxAuiPaneInfo &p = m_aui_mgr->GetPane(this);

    if (p.IsDocked()) {
        if (!m_menu_docked) {
            m_menu_docked = new wxMenu;
            m_menu_docked->Append(ID_DETACH, wxT("Undock"));
            
            
            m_menu_docked->Append(ID_HIDE_BAR, wxT("Hide Bar"));
        
            m_menu_docked->Append(ID_SHOW_BAR, wxT("Show Bar"));
            m_menu_docked->AppendSeparator();
            m_menu_docked->Append(ID_PREFS, wxT("Preferences"));
           
        }
        PopupMenu(m_menu_docked);
    } else {
        if (!m_menu_floating) {
            m_menu_floating = new wxMenu;
            
            
            
                m_menu_floating->Append(ID_HIDE_BAR, wxT("Hide Bar"));
            
                m_menu_floating->Append(ID_SHOW_BAR, wxT("Show Bar"));
                m_menu_floating->AppendSeparator();
                m_menu_floating->Append(ID_PREFS, wxT("Preferences"));
            
        }
        PopupMenu(m_menu_floating);
    }

   
}
void SonarDisplayWindow::OnContextMenuSelect(wxCommandEvent& event) {

    wxAuiPaneInfo &p = m_aui_mgr->GetPane(this);
    switch(event.GetId()) {
        case ID_DETACH:
            p.Float().CaptionVisible(true);
            m_aui_mgr->Update();
            break;
        case ID_PREFS:
            m_pi->ShowPreferencesDialog(this);
            break;
        case ID_SHOW_BAR:
            m_cp_sizer->Show(true);
            Layout();
            break;
        case ID_HIDE_BAR:
            m_cp_sizer->Show(false);
            Layout();
            break;
    }
}
void SonarDisplayWindow::ShowFrame() {

    if (m_pi->IsOpenGLEnabled()) {    
        GetSizer()->Replace(dummy, sopa);
    }
    wxAuiPaneInfo &p = m_aui_mgr->GetPane(this);
    

    if (p.IsDocked()) {
        p.CaptionVisible(false);
    } else {
        p.CaptionVisible(true);
    }

    p.Show();
    m_aui_mgr->Update();
    
    Layout();
    
}

void SonarDisplayWindow::HideFrame() {
    wxAuiPaneInfo &p = m_aui_mgr->GetPane(this);
    p.Hide();
    m_aui_mgr->Update();
    if (m_pi->IsOpenGLEnabled()) {    
        GetSizer()->Replace(sopa, dummy);
    }
}

void SonarDisplayWindow::OnSize(wxSizeEvent& event) {

    wxAuiPaneInfo &p = m_aui_mgr->GetPane(this);

    int width = event.GetSize().x;

    bool is_docked = p.IsDocked();
    if (is_docked != m_docked) {
        m_docked = is_docked;
        p.CloseButton(!is_docked);
        p.CaptionVisible(!is_docked);
        p.Show();
    }

    if (width < 400) {
        m_st_slider->Hide();
    } 
    if (width < 300) {
        m_slider->Hide();
    }
    if (width >= 400) {
        m_st_slider->Show();
    }
    if (width >= 300) {
        m_slider->Show();
    }
    
    event.Skip();
}

void SonarDisplayWindow::OnBtnMonochrome(wxCommandEvent& event) {
    if (((wxToggleButton*)event.GetEventObject())->GetValue()) {
        m_display_mode = MONOCHROME;
    } else {
        m_display_mode = MULTICOLOR;
    }
}

SonarDisplayWindow::~SonarDisplayWindow() {
    m_aui_mgr->Unbind(wxEVT_AUI_PANE_CLOSE, &SonarDisplayWindow::FrameClosed, this);
    m_aui_mgr->DetachPane(this);
    delete m_timer;
    delete sopa;
}

bool SonarDisplayWindow::StartDataReceiver(bool clear) {
    Bind(NEW_DATA_TYPE, &SonarDisplayWindow::NewSonarData, this);
    Bind(NEW_DEPTH_TYPE, &SonarDisplayWindow::NewDepthData, this);
    
    if (clear) {
        index           = BUF_X;
        m_depth_value   = 0;
        m_depth         = 0;

        sopa->ResetDataBuffer();    // clear the screen
    }
    // startup a data provider
    if (m_pi->m_data_interface == MODE_NET) {
        m_data_receiver = new UDPDataReceiver(this, m_pi);
    } else if (m_pi->m_data_interface == MODE_SERIAL) {
        m_data_receiver = new SerialDataReceiver(this);
    } else {
        return false;
    }

    m_data_receiver->SetFrequency(m_frequency);

    // let the receiver setup itself
    m_data_receiver->Startup();

    // fire of the thread
    if (m_data_receiver->Run() != wxTHREAD_NO_ERROR) {
        wxLogMessage("Can't create the thread!");
        delete m_data_receiver;
        m_data_receiver = nullptr;
        return false;
    }

    // one second timer for NMEA message sending
    m_timer->Start(1000);

    return true;
}

void SonarDisplayWindow::StopDataReceiver() {
    m_timer->Stop();
    wxCriticalSectionLocker enter(m_pThreadCS);
    m_data_receiver->Shutdown();
    if (m_data_receiver) {
        if (m_data_receiver->Delete() != wxTHREAD_NO_ERROR)
            LOG("Can't delete the thread!");
        delete m_data_receiver;
        while (1) {

            wxCriticalSectionLocker enter(m_pThreadCS);
            if (!m_data_receiver) {
                break;
            } else {
                // wait for thread completion
                wxThread::This()->Sleep(1);
            }
        }
    }
    Unbind(NEW_DATA_TYPE, &SonarDisplayWindow::NewSonarData, this);
    Unbind(NEW_DEPTH_TYPE, &SonarDisplayWindow::NewDepthData, this);
}

void SonarDisplayWindow::SetSpeed(double kn) {
    m_speed = kn;
    wxString str;
    str.Printf(_T("%.2fkn (%.1fm)\n"), kn, kn2ms(kn) * (BUF_X / 10.0));
    t_label1->SetLabelText(str);
}

void SonarDisplayWindow::OnClose(wxCloseEvent& event) {
    HideFrame();
    m_pi->closing();
}

/*void SonarDisplayWindow::OnClose(wxCommandEvent& event) {
    wxCloseEvent CloseEvent;
    CloseEvent.SetEventType(wxEVT_CLOSE_WINDOW);
    wxPostEvent(this, CloseEvent);
}*/

void SonarDisplayWindow::NewDepthData(wxCommandEvent& evt) {
    m_depth_value = evt.GetInt();
    m_depth = m_depth_value / 100.0;

    if (m_depth_avail) {
        m_pi->SendNMEABuffer(m_depth);
        m_depth_avail = false;
    }
}
void SonarDisplayWindow::NewSonarData(wxCommandEvent& evt) {
  
    msgbuf = m_data_receiver->GetData();
    if (m_depth_value == 0) return;
    if (index) {
        index--;
        memcpy(sopa->data_buffer[index], msgbuf, sizeof(sopa->data_buffer[index]));
        m_depths[index] = m_depth_value;
    } else {
        for (int i = BUF_X - 1; i > 0; i--) {
            memcpy(sopa->data_buffer[i], sopa->data_buffer[i - 1], DATA_WIDTH);
        }
        memcpy(sopa->data_buffer[0], msgbuf, sizeof(sopa->data_buffer[0]));
        std::rotate(std::begin(m_depths), std::end(m_depths) - 1, std::end(m_depths));
        m_depths[0] = m_depth_value;
    }

    int t = 0;
    for (int i = 0; i < BUF_X; i++) {
        t = std::max(t, (int)m_depths[i]);
    }
    m_depth_max = t;

    sopa->Refresh(false);
}

int SonarDisplayWindow::GetDisplayMode() {
    return m_display_mode;
}

void SonarDisplayWindow::SetDisplayMode(int _display_mode) {
    m_display_mode = _display_mode;
    m_tgl_monochrome->SetValue(m_display_mode == MONOCHROME);
}

uint8_t SonarDisplayWindow::GetFrequency() {
    return m_frequency;
}

void SonarDisplayWindow::SetFrequency(uint8_t frequency) {
    m_slider->SetValue(frequency);
    m_frequency = frequency;
}

PLUGIN_END_NAMESPACE