/******************************************************************************
 *
 * Project:  OpenCPN
 * Purpose:  Sonar Plugin
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

#include <chrono>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <stdio.h>

#include <regex> 
#include "sonar_pi.h"


#include <wx/filename.h>
#include <wx/log.h>
#include "wx/wxprec.h"
#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif  // precompiled headers

PLUGIN_BEGIN_NAMESPACE

// the class factories, used to create and destroy instances of the PlugIn

extern "C" DECL_EXP opencpn_plugin* create_pi(void* ppimgr) {
    return static_cast<opencpn_plugin*>(new sonar_pi(ppimgr));
}

extern "C" DECL_EXP void destroy_pi(opencpn_plugin* pi) { delete pi; }

sonar_pi::sonar_pi(void* ppimgr) : opencpn_plugin_118(ppimgr), m_color_scheme(PI_GLOBAL_COLOR_SCHEME_RGB), m_shown(false) {

    m_opengl_mode = OPENGL_UNKOWN;

    if (!wxDirExists(GetDataDir())) {
        wxFileName::Mkdir(GetDataDir(), wxS_DIR_DEFAULT, wxPATH_MKDIR_FULL);
    }

}

sonar_pi::~sonar_pi() { }

/**
 * Is called after the main GUI has been built. This enables us to setup our GUI objects (AUI).
*/
void sonar_pi::UpdateAuiStatus() {
    wxAuiPaneInfo &pane =  m_aui_mgr->GetPane(sonarDisplayWindow);

    m_shown = pane.IsShown();

    if (m_shown) {
        sonarDisplayWindow->ShowFrame();
    } else {
        sonarDisplayWindow->HideFrame();
    }
    SetToolbarItemState(pi_id, m_shown);
}

/**
 * 
*/
void sonar_pi::closing() {
    m_shown = false;
    SetToolbarItemState(pi_id, m_shown);
}

int sonar_pi::Init() {
    m_pconfig = GetOCPNConfigObject();
    LoadConfig();
    m_aui_mgr = GetFrameAuiManager();
    
    _svg_dashboardsk = GetDataDir() + "sonar_pi.svg";
    _svg_dashboardsk_rollover = GetDataDir() + "sonar_pi.svg";
    _svg_dashboardsk_toggled = GetDataDir() + "sonar_pi.svg";
    AddLocaleCatalog(_T("opencpn-sonar_pi"));

    pi_id = InsertPlugInToolSVG(_T( "Sonar" ), _svg_dashboardsk, _svg_dashboardsk_rollover, _svg_dashboardsk_toggled, wxITEM_CHECK, _("Sonar"), _T( "" ),
            nullptr, -1, 0, this);
   
    sonarDisplayWindow = new SonarDisplayWindow(GetOCPNCanvasWindow(), _T("Sonar Display"), this, m_aui_mgr);
    sonarDisplayWindow->SetDisplayMode(m_display_mode);
    sonarDisplayWindow->SetFrequency(m_sonar_frequency);

    
    sonarDisplayWindow->StartDataReceiver(true);


    return WANTS_NMEA_SENTENCES | WANTS_TOOLBAR_CALLBACK | INSTALLS_TOOLBAR_TOOL | WANTS_PREFERENCES | WANTS_DYNAMIC_OPENGL_OVERLAY_CALLBACK | WANTS_OPENGL_OVERLAY_CALLBACK     \
        | WANTS_OVERLAY_CALLBACK | USES_AUI_MANAGER;
}

//TODO: Not really using this. Refactor the GL detection!
bool sonar_pi::RenderGLOverlayMultiCanvas(wxGLContext *pcontext, PlugIn_ViewPort *vp, int canvasIndex) {
    m_opencpn_gl_context = pcontext;
    SetOpenGLMode(OPENGL_ON);
    return true;
}

wxGLContext* sonar_pi::GetChartOpenGLContext() {
    return m_opencpn_gl_context;
}


void sonar_pi::SetOpenGLMode(OpenGLMode mode) {
    if (m_opengl_mode != mode) {
        m_opengl_mode = mode;
        if (mode == OPENGL_ON) {
            LOG("OpenGL mode set on");
        } else if (mode == OPENGL_OFF) {
            LOG("OpenGL mode set off");
        } else if (mode == OPENGL_UNKOWN) {
            LOG("OpenGL mode set unknown");
        }
    }
}
bool sonar_pi::DeInit() {
    m_display_mode      = sonarDisplayWindow->GetDisplayMode();
    m_sonar_frequency   = sonarDisplayWindow->GetFrequency();

    sonarDisplayWindow->StopDataReceiver();
    sonarDisplayWindow->Close();
    
    delete sonarDisplayWindow;
    sonarDisplayWindow = NULL;
    
    closing();
    
    RemovePlugInTool( pi_id );
    
    SaveConfig();
    
    return true;
}

int sonar_pi::GetAPIVersionMajor() { return API_VERSION_MAJOR; }

int sonar_pi::GetAPIVersionMinor() { return API_VERSION_MINOR; }

int sonar_pi::GetPlugInVersionMajor() { return PLUGIN_VERSION_MAJOR; }

int sonar_pi::GetPlugInVersionMinor() { return PLUGIN_VERSION_MINOR; }

wxBitmap* sonar_pi::GetPlugInBitmap() { return &m_logo; }
wxString sonar_pi::GetCommonName() { return _("Sonar"); }

wxString sonar_pi::GetShortDescription() {
    return _("Sonar Display PlugIn for OpenCPN");
}

wxString sonar_pi::GetLongDescription() {
    return _("Simple Sonar Display.\n");
}

int sonar_pi::GetToolbarToolCount() { return 1; }

void sonar_pi::OnToolbarToolCallback(int id) {
    wxAuiPaneInfo &pane =  m_aui_mgr->GetPane(sonarDisplayWindow);
   
    m_shown = pane.IsShown();
    if (m_shown) {
        sonarDisplayWindow->HideFrame();
        m_shown = false;
    } else {
        sonarDisplayWindow->ShowFrame();
        m_shown = true;
    }
    
}

void sonar_pi::ShowPreferencesDialog(wxWindow* _parent) {

    PreferencesWindow *preferencesWindow = new PreferencesWindow(_parent, wxString::Format(wxT("Sonar Preferences v%d.%d.%d-%s"), PLUGIN_VERSION_MAJOR, PLUGIN_VERSION_MINOR, PLUGIN_VERSION_PATCH, PLUGIN_VERSION_TWEAK));
    wxFont *pFont = OCPNGetFont(_T("Dialog"), 0);
    preferencesWindow->SetFont(*pFont);
    preferencesWindow->SetSerialPortDefinition(m_serial_port);

    preferencesWindow->SetIPAddressDefinition(m_ip_address);
    preferencesWindow->SetDataInterfaceDefinition(m_data_interface);
    preferencesWindow->SetIPPortDefinition(m_ip_port);
    preferencesWindow->SetBackgroundColour(m_background_colour);
    preferencesWindow->SetSerialSendMulticast(m_serial_send_multicast);
    preferencesWindow->SetFrequency(sonarDisplayWindow->GetFrequency());
    preferencesWindow->SetSensorOffst(m_sensor_offset);
    preferencesWindow->SetNMEAID(m_nmea_id);
    preferencesWindow->SetNMEAType(m_nmea_type);
    preferencesWindow->SetNMEAEnable(m_nmea_enable);

    preferencesWindow->Fit();
    preferencesWindow->Layout();
    
    if (preferencesWindow->ShowModal() == wxID_OK) {

        sonarDisplayWindow->StopDataReceiver();

        m_serial_port           = preferencesWindow->GetSerialPortDefinition();
        m_ip_address            = preferencesWindow->GetIPAddressDefinition();
        m_data_interface        = preferencesWindow->GetDataInterfaceDefinition();
        m_ip_port               = preferencesWindow->GetIPPortDefinition();
        m_background_colour     = preferencesWindow->GetBackgroundColour();
        m_serial_send_multicast = preferencesWindow->GetSerialSendMulticast();
        m_sonar_frequency       = preferencesWindow->GetFrequency();
        sonarDisplayWindow      ->SetFrequency(m_sonar_frequency);
        m_sensor_offset         = preferencesWindow->GetSensorOffset();
        m_nmea_type             = preferencesWindow->GetNMEAType();
        m_nmea_id               = preferencesWindow->GetNMEAID();
        m_nmea_enable           = preferencesWindow->GetNMEAEnable();

        SaveConfig();
        
        sonarDisplayWindow->StartDataReceiver(true);
    } else {
        wxLogMessage(wxT("Nothing to save here"));
    }
}


void sonar_pi::SetColorScheme(PI_ColorScheme cs) {
    m_color_scheme = cs;
    // TODO, for now we have no windows
}



void sonar_pi::SetNMEASentence(wxString& sentence) {
    std::string stc = sentence.ToStdString();

    stc.erase(std::remove(stc.begin(), stc.end(), '\n'), stc.cend());
    stc.erase(std::remove(stc.begin(), stc.end(), '\r'), stc.cend());
// $--RMC,hhmmss.ss,A,llll.ll,a,yyyyy.yy,a,x.x,x.x,xxxx,x.x,a*hh<CR><LF>
    regex regexp("^\\$[A-Z]{2}RMC,[0-9.]+,.,[0-9.]+,[NS],[0-9.]+,[EW],([0-9.]+),.*");
    smatch m;
    if (regex_match(stc, m, regexp)) {
        sonarDisplayWindow->SetSpeed(stod(m[1]));
    }

}
void sonar_pi::SendNMEABuffer(float depth) {
    if (!m_nmea_enable) return;

    uint8_t crc = 0;
    wxString sentence;

    switch(m_nmea_type) {
        case ID_DBT:
            sentence = wxString::Format(_T("%sDBT,%.1f,f,%.1f,M,0.0,F"), m_nmea_id, m2ft(depth), depth);
            break;
        case ID_DPT:
            sentence = wxString::Format(_T("%sDPT,%0.1f,%.1f"), m_nmea_id, depth, m_sensor_offset);
            break;
        default:
            return;
    }

    for ( wxString::const_iterator it = sentence.begin(); it != sentence.end(); ++it ) {
        crc ^= static_cast<uint8_t>(*it);
    }
    
    sentence = wxString::Format(_T("$%s*%02X\r\n"), sentence, crc);
    
    PushNMEABuffer(sentence);
}

wxString sonar_pi::GetDataDir()
{
    return GetPluginDataDir("sonar_pi") + wxFileName::GetPathSeparator() + "data"
        + wxFileName::GetPathSeparator();
}

wxBitmap sonar_pi::GetBitmapFromSVG(
    const wxString& filename, const wxCoord w, const wxCoord h)
{
    return GetBitmapFromSVGFile(GetDataDir() + filename, w, h);
}


bool sonar_pi::LoadConfig(void) {
  wxFileConfig *p_conf = (wxFileConfig *)m_pconfig;

  if (p_conf) {
    p_conf->SetPath(_T( "/PlugIns/Sonar" ));
    p_conf->Read(_T( "serial_port" ), &m_serial_port);

    p_conf->Read(_T( "serial_baud" ), &m_serial_baud, DEFAULT_SERIAL_BAUD);

    if (p_conf->Exists(_T( "ip_address" ))) {
        p_conf->Read(_T( "ip_address" ), &m_ip_address);
    }
    p_conf->Read(_T( "data_interface" ), &m_data_interface, MODE_SERIAL);
    
    if (p_conf->Exists(_T( "ip_port" ))) {
        p_conf->Read(_T( "ip_port" ), &m_ip_port);
    }
    if (p_conf->Exists(_T( "background_colour" ))) {
        wxString t;
        p_conf->Read(_T( "background_colour" ), &t);
        m_background_colour.Set(t);
    } else {
        m_background_colour = *wxBLACK;
    }
    p_conf->Read(_T( "serial_send_multicast" ), &m_serial_send_multicast, false);
    p_conf->Read(_T( "sonar_frequency" ), &m_sonar_frequency, DEFAULT_FREQUENCY);
    p_conf->Read(_T( "sensor_offset" ), &m_sensor_offset, 0.0);
    p_conf->Read(_T( "display_mode" ), &m_display_mode, MULTICOLOR);
    p_conf->Read(_T( "nmea_type" ), &m_nmea_type, ID_DBT);
    p_conf->Read(_T( "nmea_id" ), &m_nmea_id, ID_NMEA);
    p_conf->Read(_T( "nmea_enable" ), &m_nmea_enable, false);

    
    return true;
  } else
    return false;
}

bool sonar_pi::SaveConfig(void) {
    wxFileConfig *p_conf = (wxFileConfig *)m_pconfig;

    if (p_conf) {
        p_conf->SetPath(_T ( "/PlugIns/Sonar" ));
        p_conf->Write(_T ( "serial_port" ), m_serial_port);
        p_conf->Write(_T ( "serial_baud" ), m_serial_baud);
        p_conf->Write(_T ( "ip_address" ), m_ip_address);
        p_conf->Write(_T ( "data_interface" ), m_data_interface);
        p_conf->Write(_T ( "ip_port" ), m_ip_port);
        p_conf->Write(_T ( "background_colour" ), m_background_colour.GetAsString(wxC2S_HTML_SYNTAX));
        p_conf->Write(_T ( "serial_send_multicast" ), m_serial_send_multicast);
        p_conf->Write(_T ( "sonar_frequency" ), m_sonar_frequency);
        p_conf->Write(_T ( "display_mode" ), m_display_mode);
        p_conf->Write(_T ( "nmea_type" ), m_nmea_type);
        p_conf->Write(_T ( "nmea_id" ), m_nmea_id);
        p_conf->Write(_T ( "nmea_enable" ), m_nmea_enable);


        return true;
    } else
        return false;
}
PLUGIN_END_NAMESPACE
