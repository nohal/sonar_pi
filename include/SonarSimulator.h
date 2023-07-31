#ifndef _SONARSIM_H_
#define _SONARSIM_H_

#include <wx/wxprec.h>
#ifndef WX_PRECOMP
    #include <wx/wx.h>
#endif

#define LOG(s)             std::cout << s << std::endl;
#define DEFAULT_PORT       60005
#define DEFAULT_ADDRESS    "239.192.0.5"

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
class MainFrame;

class SonarSimulator : public wxApp {
    public:
        SonarSimulator();
        virtual bool OnInit();
        wxSlider* m_slider; 
        wxCheckBox* cbox;
        long m_total_bytes;
    private:
        MainFrame* frame;
        wxTextCtrl* m_tc_ip_address;
        wxTextCtrl* m_tc_ip_port;
        void sleep_ms(int milliseconds);
        void OnIPTyped(wxCommandEvent& event);
        void OnBtnSet(wxCommandEvent& event);
        
};

class MainFrame: public wxFrame {
public:
    MainFrame(const wxString& title, const wxPoint& pos, const wxSize& size, SonarSimulator* ss);
    int      m_port;
    wxString m_group;
private:
    void fill_sonar_data(uint8_t *data);
    int fd;
    SonarSimulator* m_ss;
    void OnTimer(wxTimerEvent& WXUNUSED(event));
    wxTimer m_timer;
    int addrlen;
    struct sockaddr_in addr;
    void OnExit(wxCommandEvent& event);
    wxDECLARE_EVENT_TABLE();
};

enum {
    ID_Hello = 1
};

#endif
