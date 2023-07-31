#ifndef _PREFERENCES_WINDOW_H_
#define _PREFERENCES_WINDOW_H_

#include "pi_common.h"
#include <wx/regex.h>
#include <wx/clrpicker.h>

PLUGIN_BEGIN_NAMESPACE

class IDValidator: public wxValidator {
    public:
        virtual bool Validate(wxWindow* parent) override;
        virtual wxObject* Clone() const override { return new IDValidator(*this); }
        // Called to transfer data to the window
        virtual bool TransferToWindow() override;

        // Called to transfer data from the window
        virtual bool TransferFromWindow() override;
};

class IPAddressValidator: public wxValidator {
    public:
        virtual bool Validate(wxWindow* parent) override;
        virtual wxObject* Clone() const override { return new IPAddressValidator(*this); }
        // Called to transfer data to the window
        virtual bool TransferToWindow() override;

        // Called to transfer data from the window
        virtual bool TransferFromWindow() override;
};

class IntegerValidator: public wxValidator {
    public:
        virtual bool Validate(wxWindow* parent) override;
        virtual wxObject* Clone() const override { return new IntegerValidator(*this); }
        // Called to transfer data to the window
        virtual bool TransferToWindow() override;

        // Called to transfer data from the window
        virtual bool TransferFromWindow() override;
};

class PreferencesWindow : public wxDialog {

    public:
        PreferencesWindow(wxWindow* w_parent, const wxString& title);
        ~PreferencesWindow();

        wxColour GetBackgroundColour();
        wxString GetDataInterfaceDefinition();
        uint8_t  GetFrequency();
        wxString GetIPAddressDefinition();
        uint16_t GetIPPortDefinition();
        bool     GetNMEAEnable();
        wxString GetNMEAID();
        int      GetNMEAType();
        float    GetSensorOffset();
        wxString GetSerialPortDefinition();
        bool     GetSerialSendMulticast();

        void SetSerialPortDefinition(wxString serial_port);
        void SetSerialSendMulticast(bool serial_send_multicast);
        void SetIPAddressDefinition(wxString ip_address);
        void SetDataInterfaceDefinition(wxString data_interface);
        void SetIPPortDefinition(uint16_t ip_port);
        void SetBackgroundColour(wxColour background_colour);
        void SetFrequency(uint8_t frequency);
        void SetSensorOffst(float offset);
        void SetNMEAID(wxString id);
        void SetNMEAType(int type);
        void SetNMEAEnable(bool enbaled);

    private:
        bool        m_serial_send_multicast;
        wxString    m_serial_port;
        wxString    m_ip_address;
        wxString    m_data_interface;
        uint16_t    m_ip_port;
        wxColour    m_background_colour;
        float       m_sensor_offset;
        wxString    m_nmea_id;
        int         m_nmea_type;
        bool        m_nmea_enabled;

        wxTextCtrl *tc1;
        wxRadioBox* m_rb_receiver;
        wxBoxSizer* m_sz_main;
        wxPanel*    m_pnl_details;
        wxPanel*    m_pnl_details_serial;
        wxPanel*    m_pnl_details_net;
        wxBoxSizer* sz_top_row;
        wxTextCtrl* m_tc_ip_address;
        wxTextCtrl* m_tc_ip_port;
        wxSlider*   m_slider;
        wxCheckBox* m_cb_nmea_enable;

        wxCheckBox*     m_cb_provide_multicast;
        wxArrayString   m_arrItems;
        wxComboBox*     m_box;
        wxColourPickerCtrl* m_cpc;
        wxComboBox*     m_cb_nmea;
        wxStaticText*   m_st_result;
        wxTextCtrl*     m_tc_id;
        wxTextCtrl*     m_tc_offset;
        wxStaticText*   m_st_offset;
        wxRegEx*        re;

        
        void OnIDTextTyped(wxCommandEvent& event);
        void OnIPTyped(wxCommandEvent& event);
        void OnButtonOkClick(wxCommandEvent& event);
        void OnRadioBoxChange(wxCommandEvent& event);
        void OnSelectColour(wxColourPickerEvent& event);
        void SwitchDataInterfacePanel(uint8_t n);
        void OnDxTSelect(wxCommandEvent& event);
        void OnTextFocusLost(wxFocusEvent& event);
        void UpdateNMEASentence();
        
        DECLARE_EVENT_TABLE();
};
PLUGIN_END_NAMESPACE
#endif