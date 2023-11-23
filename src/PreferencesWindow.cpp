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
#include "PreferencesWindow.h"
#include "serialib.h"
#include <wx/statline.h>

#define LOG(str)    std::cout << str << std::endl << std::flush;
#define ID_IP_ADDRESS   1
#define ID_IP_PORT      2
#define ID_TYPE         3

PLUGIN_BEGIN_NAMESPACE

BEGIN_EVENT_TABLE(PreferencesWindow, wxDialog)
EVT_COLOURPICKER_CHANGED(wxID_ANY, PreferencesWindow::OnSelectColour)
END_EVENT_TABLE()


PreferencesWindow::PreferencesWindow(wxWindow* w_parent, const wxString& title) : wxDialog(w_parent, wxID_ANY, title, wxDefaultPosition, wxDefaultSize) {
    m_data_interface = MODE_SERIAL;
    re = new wxRegEx("[a-zA-Z]+");
 
    serialib serial;
    
    for (int i = 1; i < 99; i++) {
        char device_name[64];
        // Prepare the port name (Windows)
        #if defined (_WIN32) || defined( _WIN64)
            sprintf (device_name,"\\\\.\\COM%d",i);
        #endif

        // Prepare the port name (Linux)
        #ifdef __linux__
            //TODO: find a more elegant solution. Remember, the device path can be entered manually later.
            sprintf (device_name,"/dev/ttyS%d",i-1);
        #endif

        // try to connect to the device
        if (serial.openDevice(device_name, DEFAULT_SERIAL_BAUD) == 1) {
            m_arrItems.Add(device_name);
            
            // Close the device before testing the next port
            serial.closeDevice();
        }
    }
    // just a workaround helper
    wxTextCtrl* tc_t = new wxTextCtrl(this, wxID_ANY, _T(ID_NMEA), wxDefaultPosition, wxDefaultSize, 0);

    m_sz_main      = new wxBoxSizer(wxVERTICAL);

    // Top Row
    sz_top_row      = new wxBoxSizer(wxVERTICAL);
    wxArrayString *selection = new wxArrayString;
    selection->Add("Serial Interface");
    selection->Add("Network (UDP)");
    m_rb_receiver  = new wxRadioBox(this, wxID_ANY, _T("Data Receiver Channel"), wxPoint(0, 0), wxDefaultSize, *selection, 1, wxRA_SPECIFY_COLS); 
    
  
    sz_top_row->Add(m_rb_receiver, 0, wxEXPAND | wxLEFT | wxTOP | wxRIGHT, 10);
    m_sz_main->Add(sz_top_row, 0, wxEXPAND);
    
    // Details Panel
    wxFont boldFont(14, wxFONTFAMILY_TELETYPE, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_BOLD);

    // *********************************************
    // * Serial                                    *
    // *********************************************
    m_pnl_details_serial  = new wxPanel(this);
    wxBoxSizer* s1      = new wxBoxSizer(wxVERTICAL);
    wxStaticText* t_label1 = new wxStaticText(m_pnl_details_serial, -1, wxT("Serial Connection Details"));
    t_label1->SetFont(boldFont);
    s1->Add(t_label1, 0, wxTOP | wxBOTTOM, 5);
    wxBoxSizer* sz_ser_port      = new wxBoxSizer(wxHORIZONTAL);
    wxStaticText* st_ser_port = new wxStaticText(m_pnl_details_serial, -1, wxT("Serial Port:"));
    sz_ser_port->Add(st_ser_port, 0, wxALIGN_CENTER_VERTICAL);
    m_box = new wxComboBox(m_pnl_details_serial, wxID_ANY, _T(""), wxDefaultPosition, wxDefaultSize, m_arrItems, wxTE_PROCESS_ENTER | wxTE_PROCESS_TAB);
    sz_ser_port->Add(m_box, 1);
    // Provide Multicast
    //wxBoxSizer* sz_serial_multicast     = new wxBoxSizer(wxHORIZONTAL);
    m_cb_provide_multicast    = new wxCheckBox(m_pnl_details_serial, wxID_ANY, _T("Send as Multicast over Network"));
    
    s1->Add(sz_ser_port, 0, wxEXPAND);
    s1->Add(m_cb_provide_multicast);
    m_pnl_details_serial->SetSizer(s1);

    // *********************************************
    // * Network                                   *
    // *********************************************
    m_pnl_details_net     = new wxPanel(this);
    wxStaticText* t_label2 = new wxStaticText(m_pnl_details_net, -1, wxT("Network Connection Details"));
    t_label2->SetFont(boldFont);
    wxBoxSizer* sz_net      = new wxBoxSizer(wxVERTICAL);
    sz_net->Add(t_label2, 0, wxTOP | wxBOTTOM, 5);
    m_pnl_details_net->SetSizer(sz_net);

    // Net Address
    wxBoxSizer* sz_net_address      = new wxBoxSizer(wxHORIZONTAL);
    wxStaticText* t_label3 = new wxStaticText(m_pnl_details_net, -1, wxT("Multicast UDP IPv4 Address:"));
    wxSize sz_ipa = tc_t->GetSizeFromTextSize(tc_t->GetTextExtent("999.999.999.999").x);
    m_tc_ip_address = new wxTextCtrl(m_pnl_details_net, ID_IP_ADDRESS, m_ip_address, wxDefaultPosition, sz_ipa, wxTE_CENTRE, IPAddressValidator());
    m_tc_ip_address->SetMaxLength(15);
    sz_net_address->Add(t_label3, 0, wxALIGN_LEFT | wxALIGN_CENTER_VERTICAL);
    sz_net_address->Add(m_tc_ip_address, 0, wxALIGN_CENTER_VERTICAL);
    // Net Address Port
    wxStaticText* t_label4 = new wxStaticText(m_pnl_details_net, -1, wxT("Listening Port:"));
    wxSize sz_ipp = tc_t->GetSizeFromTextSize(tc_t->GetTextExtent("99999").x);
    m_tc_ip_port = new wxTextCtrl(m_pnl_details_net, ID_IP_PORT, wxEmptyString, wxDefaultPosition, sz_ipp, wxTE_CENTRE, IntegerValidator());
    m_tc_ip_port->SetMaxLength(5);
    sz_net_address->Add(t_label4, 0, wxALIGN_LEFT | wxALIGN_CENTER_VERTICAL | wxLEFT, 5);
    sz_net_address->Add(m_tc_ip_port, 0, wxALIGN_CENTER_VERTICAL);
    sz_net->Add(sz_net_address);


    m_pnl_details_serial->SetMinSize(wxSize(-1, 110));
    m_pnl_details_net->SetMinSize(wxSize(-1, 110));
    
    m_sz_main->Add(m_pnl_details_serial, 1, wxEXPAND | wxALL, 10);
    m_sz_main->Add(m_pnl_details_net, 1, wxEXPAND | wxALL, 10);
    
    
    m_pnl_details_serial->Show();
    m_pnl_details_net->Hide();

    m_sz_main->Add(new wxStaticLine(this), 0, wxEXPAND | wxALL, 10);

    // *********************************************
    // * NMEA                                      *
    // *********************************************
    wxBoxSizer* sz_nmea      = new wxBoxSizer(wxHORIZONTAL);
    wxStaticText* st_nmea = new wxStaticText(this, -1, _T("NMEA Settings"));
    st_nmea->SetFont(boldFont);
    m_sz_main->Add(st_nmea, 0, wxEXPAND | wxALIGN_LEFT | wxLEFT | wxRIGHT, 10);


    wxStaticText* st_nmea1 = new wxStaticText(this, wxID_ANY, "ID:");
    sz_nmea->Add(st_nmea1, 1, wxALIGN_CENTER_VERTICAL | wxLEFT, 10);
    
    tc_t->Hide();
    wxSize sz = tc_t->GetSizeFromTextSize(tc_t->GetTextExtent("WW").x);
    m_tc_id = new wxTextCtrl(this, ID_TYPE, ID_NMEA, wxDefaultPosition, sz, wxTE_CENTRE, IDValidator());
    m_tc_id->SetMaxLength(2);
    

    sz_nmea->Add(m_tc_id, 0, wxALIGN_CENTER_VERTICAL | wxLEFT, 5);

    wxStaticText* st_nmea2 = new wxStaticText(this, wxID_ANY, "Type:");
    sz_nmea->Add(st_nmea2, 1, wxALIGN_CENTER_VERTICAL | wxLEFT, 5);
    wxArrayString  arrItems;
    arrItems.Add(ID_DBT_STR);
    arrItems.Add(ID_DPT_STR);
    m_cb_nmea = new wxComboBox(this, wxID_ANY, _T(ID_DBT_STR), wxDefaultPosition, wxDefaultSize, arrItems, wxCB_READONLY);
    sz_nmea->Add(m_cb_nmea, 1, wxALIGN_CENTER_VERTICAL);

    // offset
    m_st_offset = new wxStaticText(this, wxID_ANY, "Offset:");
    wxSize sz_o = tc_t->GetSizeFromTextSize(tc_t->GetTextExtent("-99.9").x);
    m_tc_offset = new wxTextCtrl(this, wxID_ANY, "0.0", wxDefaultPosition, sz_o, wxTE_CENTRE);
    m_tc_offset->SetMaxLength(5);
    m_st_offset->Hide();
    m_tc_offset->Hide();
    sz_nmea->Add(m_st_offset , 0, wxALIGN_CENTER_VERTICAL | wxLEFT, 5);
    sz_nmea->Add(m_tc_offset , 0, wxALIGN_CENTER_VERTICAL | wxLEFT, 5);
    tc_t->Destroy();
    tc_t = nullptr;

    m_st_result = new wxStaticText(this, wxID_ANY, _T("$-----"));
    
    sz_nmea->Add(m_st_result , 5, wxALIGN_CENTER_HORIZONTAL | wxALIGN_CENTER_VERTICAL | wxLEFT | wxRIGHT, 5);
    
    m_sz_main->Add(sz_nmea, 0, wxEXPAND);
    m_cb_nmea_enable = new wxCheckBox(this, wxID_ANY, "Enable NMEA sentence provisioning");
    m_sz_main->Add(m_cb_nmea_enable, 0, wxLEFT | wxEXPAND | wxRIGHT, 5);

    m_sz_main->Add(new wxStaticLine(this), 0, wxEXPAND | wxALL, 10);

    // *********************************************
    // * Miscell                                   *
    // *********************************************
    wxStaticText* st_miscell_head = new wxStaticText(this, -1, wxT("Sensor Setting"));
    st_miscell_head->SetFont(boldFont);
    m_sz_main->Add(st_miscell_head, 0, wxEXPAND | wxALIGN_LEFT | wxLEFT | wxRIGHT, 10);
    wxBoxSizer* sz_miscell      = new wxBoxSizer(wxHORIZONTAL);
    wxStaticText* st_slider = new wxStaticText(this, wxID_ANY, wxT("Freq. (kHz):"));
    m_slider = new wxSlider(this, wxID_ANY, 150, 100, 250, wxDefaultPosition, wxDefaultSize, wxSL_LABELS);
    m_slider->SetTick(10);
    m_slider->SetTickFreq(10);
    sz_miscell->Add(st_slider, 0, wxALIGN_CENTER_VERTICAL);
    sz_miscell->Add(m_slider, 1, wxALIGN_CENTER_VERTICAL);
    m_sz_main->Add(sz_miscell, 0, wxEXPAND | wxALIGN_LEFT | wxLEFT | wxRIGHT, 10);
    m_sz_main->Add(new wxStaticLine(this), 0, wxEXPAND | wxALL, 10);
    
    // *********************************************
    // * Styles                                    *
    // *********************************************
    wxStaticText* st_styles_head = new wxStaticText(this, -1, wxT("Styles"));
    st_styles_head->SetFont(boldFont);
    m_sz_main->Add(st_styles_head, 0, wxEXPAND | wxALIGN_LEFT | wxLEFT | wxRIGHT, 10);
    wxBoxSizer* sz_styles      = new wxBoxSizer(wxVERTICAL);
    // Color
    wxBoxSizer* sz_styles_color      = new wxBoxSizer(wxHORIZONTAL);
    wxStaticText* t_label5 = new wxStaticText(this, -1, wxT("Background (Water) Colour:"));

    m_cpc = new wxColourPickerCtrl(this, wxID_ANY, *wxBLUE, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T(""));
    sz_styles_color->Add(t_label5, 3, wxALIGN_LEFT | wxALIGN_CENTER_VERTICAL);
    sz_styles_color->Add(m_cpc, 1);
    
    sz_styles->Add(sz_styles_color, 0, wxEXPAND);
    
    m_sz_main->Add(sz_styles, 0, wxEXPAND | wxLEFT | wxRIGHT, 10);
    
    m_sz_main->Add(new wxStaticLine(this), 0, wxEXPAND | wxALL, 10);


    // Window Buttons
    wxBoxSizer *sz_main_buttons = new wxBoxSizer(wxHORIZONTAL);
    wxButton *ok = new wxButton(this, wxID_OK, wxT("Ok"));
    wxButton *cancel = new wxButton(this, wxID_CANCEL, wxT("Cancel"));
    sz_main_buttons->Add(ok);
    sz_main_buttons->Add(cancel);
    m_sz_main->Add(sz_main_buttons, 0, wxALIGN_RIGHT | wxRIGHT | wxBOTTOM, 10);


    SetSizer(m_sz_main);

    //Fit();
    Centre();

    Bind(wxEVT_RADIOBOX, &PreferencesWindow::OnRadioBoxChange, this);
    m_tc_ip_address ->Bind(wxEVT_TEXT, &PreferencesWindow::OnIPTyped, this);
    m_tc_ip_address ->Bind(wxEVT_KILL_FOCUS, &PreferencesWindow::OnTextFocusLost, this);
    m_tc_ip_port    ->Bind(wxEVT_TEXT, &PreferencesWindow::OnIPTyped, this);
    m_tc_ip_port    ->Bind(wxEVT_KILL_FOCUS, &PreferencesWindow::OnTextFocusLost, this);
    m_tc_id         ->Bind(wxEVT_TEXT, &PreferencesWindow::OnIDTextTyped, this);
    m_tc_id         ->Bind(wxEVT_KILL_FOCUS, &PreferencesWindow::OnTextFocusLost, this);
    m_cb_nmea       ->Bind(wxEVT_COMBOBOX, &PreferencesWindow::OnDxTSelect, this);
}

void PreferencesWindow::OnTextFocusLost(wxFocusEvent& event) {
    wxTextCtrl* tc  = (wxTextCtrl*)event.GetEventObject();
    wxValidator* v  = tc->GetValidator();
    bool success    = v->Validate(tc);
    int id          = event.GetId();

    if (!success) {
        switch (id) {
            case ID_TYPE:
                tc->ChangeValue(ID_NMEA);
                break;
            case ID_IP_ADDRESS:
                tc->ChangeValue(m_ip_address);
                break;
            case ID_IP_PORT:
                tc->ChangeValue(wxString::Format(wxT("%i"), m_ip_port));
                break;

        }
        
        tc->SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_WINDOW));
        tc->Refresh();
    }
}

void PreferencesWindow::OnDxTSelect(wxCommandEvent& event) {
    if (m_cb_nmea->GetValue() == ID_DPT_STR) {
        m_st_offset->Show();
        m_tc_offset->Show();
    } else {
        m_st_offset->Hide();
        m_tc_offset->Hide();
    }
    m_nmea_type = m_cb_nmea->GetSelection();
    Layout();
    UpdateNMEASentence();
}

void PreferencesWindow::OnIDTextTyped(wxCommandEvent& event) {
    wxTextCtrl* b   = (wxTextCtrl*)event.GetEventObject();
    wxValidator* v  = b->GetValidator();
    bool success    = v->Validate(b);
    wxString test   = b->GetValue();
    long cp         = b->GetInsertionPoint();
    
    wxString result;
    size_t sta;
    size_t le;
    while (re->Matches(test)) {
        wxString s = re->GetMatch(test);
        result.Append(s);
        re->GetMatch(&sta, &le, 0);
        test = test.Mid(sta + le);
    }
    m_nmea_id = result.Upper();
    b->ChangeValue(m_nmea_id);
    if(v->Validate(b) && !success) {
        b->SetInsertionPoint(cp - 1);
    }

    if (success) {
        UpdateNMEASentence();
    }
    
}

/**
 * 
*/
void PreferencesWindow::OnIPTyped(wxCommandEvent& event) {
    wxRegEx* re;        
    
    switch (event.GetId()) {
        case ID_IP_ADDRESS:
            re = new wxRegEx("[0-9.]+");        // digits and dots allowed
            break;
        case ID_IP_PORT:
            re = new wxRegEx("[0-9]+");         // only digits allowed
            break;
        default:
            return;

    }

    wxTextCtrl* b   = (wxTextCtrl*)event.GetEventObject();
    wxValidator* v  = b->GetValidator();
    bool success    = v->Validate(b);           // check validity
    wxString test   = b->GetValue();
    
    long cp         = b->GetInsertionPoint();   // get cursor position
    
    wxString result = wxEmptyString;            // holds the clean string

    size_t start;
    size_t length;
    
    while (re->Matches(test)) {
        wxString s = re->GetMatch(test);
        result.Append(s);
        re->GetMatch(&start, &length, 0);
        test = test.Mid(start + length);
    }

    b->ChangeValue(result);                     // set maybe changed value
    
    bool retest = v->Validate(b);               // revalidate

    // adjust cursor position
    if (retest && !success) {
        b->SetInsertionPoint(cp - 1);
    } else if (!retest && !success) {
        b->SetInsertionPoint(cp);
    }

    event.Skip();
}

void PreferencesWindow::UpdateNMEASentence() {
    if (m_cb_nmea->GetValue() == ID_DPT_STR) {
        double dval;
        m_tc_offset->GetValue().ToDouble(&dval);
        m_st_result->SetLabelText(wxString::Format(_T(" = $%s%s,%.1f,%.1f"), m_tc_id->GetValue(), m_cb_nmea->GetValue(), 0.0, dval));
    } else {
        m_st_result->SetLabelText(wxString::Format(_T(" = $%s%s,%.1f,f,%.1f,M,%.1f,F"), m_tc_id->GetValue(), m_cb_nmea->GetValue(), 0.0, 0.0, 0.0));
    }
}


PreferencesWindow::~PreferencesWindow() {
    Unbind(wxEVT_RADIOBOX, &PreferencesWindow::OnRadioBoxChange, this);
    m_tc_ip_address ->Unbind(wxEVT_TEXT, &PreferencesWindow::OnIPTyped, this);
    m_tc_ip_address ->Unbind(wxEVT_KILL_FOCUS, &PreferencesWindow::OnTextFocusLost, this);
    m_tc_ip_port    ->Unbind(wxEVT_TEXT, &PreferencesWindow::OnIPTyped, this);
    m_tc_ip_port    ->Unbind(wxEVT_KILL_FOCUS, &PreferencesWindow::OnTextFocusLost, this);
    m_tc_id         ->Unbind(wxEVT_TEXT, &PreferencesWindow::OnIDTextTyped, this);
    m_tc_id         ->Unbind(wxEVT_KILL_FOCUS, &PreferencesWindow::OnTextFocusLost, this);
    m_cb_nmea       ->Unbind(wxEVT_COMBOBOX, &PreferencesWindow::OnDxTSelect, this);
}

void PreferencesWindow::OnRadioBoxChange(wxCommandEvent& event) {
    wxRadioBox* rb = (wxRadioBox*)event.GetEventObject();
    SwitchDataInterfacePanel(rb->GetSelection());
}

void PreferencesWindow::OnSelectColour(wxColourPickerEvent& event) {

    m_background_colour = event.GetColour();
}

void PreferencesWindow::SwitchDataInterfacePanel(uint8_t n) {

    switch (n) {
        case 0:
            m_data_interface = MODE_SERIAL;
            m_pnl_details_net->Hide();
            //m_pnl_details = m_pnl_details_serial;
            m_pnl_details_serial->Show();
            m_sz_main->Layout();
            break;
        case 1:
            m_data_interface = MODE_NET;
            m_pnl_details_serial->Hide();
            //m_pnl_details = m_pnl_details_net;
            m_pnl_details_net->Show();
            m_sz_main->Layout();
            break;
    }
}

wxString PreferencesWindow::GetSerialPortDefinition() {
    return m_box->GetValue();
}
void PreferencesWindow::SetSerialPortDefinition(wxString serial_port) {
    
    uint8_t i = 0;
    bool found = false;
    for (wxString item : m_arrItems) {
        if (!serial_port.compare(item)) {
            m_box->SetSelection(i);
            found = true;
            break;
        }
        i++;
    }
    if (!found) {
        m_box->SetValue(serial_port);
    }

}

wxString PreferencesWindow::GetIPAddressDefinition() {
    wxString s = m_tc_ip_address->GetValue();
    s.Replace(" ", "");
    return s;
}

void PreferencesWindow::SetIPAddressDefinition(wxString ip_address) {
    m_ip_address = ip_address;
    m_tc_ip_address->SetValue(m_ip_address);
}

wxString PreferencesWindow::GetDataInterfaceDefinition() {
    return m_data_interface;
}

void PreferencesWindow::SetDataInterfaceDefinition(wxString data_interface) {
    m_data_interface = data_interface;
    if (data_interface == MODE_NET) {
        m_rb_receiver->SetSelection(1);
        SwitchDataInterfacePanel(1);
    } else {
        m_rb_receiver->SetSelection(0);
        SwitchDataInterfacePanel(0);
    }
}

uint16_t PreferencesWindow::GetIPPortDefinition() {
   int val;
#if (wxCHECK_VERSION(3, 1, 6))
   m_tc_ip_port->GetValue().ToInt(&val);
#else
   val = wxAtoi(m_tc_ip_port->GetValue());
#endif
   return val;
}

void PreferencesWindow::SetIPPortDefinition(uint16_t ip_port) {
    m_ip_port = ip_port;
    m_tc_ip_port->SetValue(wxString::Format(wxT("%i"), m_ip_port));
}

wxColour PreferencesWindow::GetBackgroundColour() {
    return m_background_colour;
}

void PreferencesWindow::SetBackgroundColour(wxColour background_colour) {
    m_background_colour = background_colour;
    m_cpc->SetColour(m_background_colour);
}

bool PreferencesWindow::GetSerialSendMulticast() {
    return m_cb_provide_multicast->GetValue();
}

void PreferencesWindow::SetSerialSendMulticast(bool serial_send_multicast) {
    m_cb_provide_multicast->SetValue(serial_send_multicast);
}

uint8_t PreferencesWindow::GetFrequency() {
    return m_slider->GetValue();
}

void PreferencesWindow::SetFrequency(uint8_t frequency) {
    m_slider->SetValue(frequency);
}

float PreferencesWindow::GetSensorOffset() {
    return m_sensor_offset;
}

void PreferencesWindow::SetSensorOffst(float offset) {
    m_sensor_offset = offset;
    m_tc_offset->SetValue(wxString::Format(wxT("%.1f"), m_sensor_offset));
}
wxString PreferencesWindow::GetNMEAID() {
    return m_nmea_id;
}

void PreferencesWindow::SetNMEAID(wxString id) {
    m_nmea_id = id;
    m_tc_id->SetValue(m_nmea_id);
}

int PreferencesWindow::GetNMEAType() {
    return m_nmea_type;
}

void PreferencesWindow::SetNMEAType(int type) {
    m_nmea_type = type;
    
    switch(m_nmea_type) {
        case ID_DPT:
            m_st_offset->Show();
            m_tc_offset->Show();
            break;
        case ID_DBT:
            m_st_offset->Hide();
            m_tc_offset->Hide();
            break;
        default:
            return;
    }
    m_cb_nmea->SetSelection(m_nmea_type);
    Layout();
    UpdateNMEASentence();
}


bool PreferencesWindow::GetNMEAEnable() {
    return m_cb_nmea_enable->GetValue();
}

void PreferencesWindow::SetNMEAEnable(bool enabled) {
    return m_cb_nmea_enable->SetValue(enabled);
}


bool IDValidator::Validate(wxWindow *WXUNUSED(parent)) {
    wxTextCtrl* tc = (wxTextCtrl*)GetWindow();
    wxString val = tc->GetValue();

    wxRegEx r(_T("[a-zA-Z]{2}"));

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
bool IDValidator::TransferToWindow() {
    return true;
}

bool IDValidator::TransferFromWindow() {
    return true;
}

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

bool IntegerValidator::Validate(wxWindow *WXUNUSED(parent)) {
    wxTextCtrl* tc = (wxTextCtrl*)GetWindow();
    wxString val = tc->GetValue();

    wxRegEx r(wxT("^[0-9]{1,5}$"));

    if (r.Matches(val)) {
        int n;
#if (wxCHECK_VERSION(3, 1, 6))
        val.ToInt(&n);
#else
	n = wxAtoi(val);
#endif
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
PLUGIN_END_NAMESPACE
