#ifndef _SONAR_DISPLAY_WINDOW_H_
#define _SONAR_DISPLAY_WINDOW_H_

#include "pi_common.h"

#ifdef __WXMAC__
#include <GLUT/glut.h>
#else
#include <GL/glut.h>
#endif
#include <wx/tglbtn.h>


#include "UDPDataReceiver.h"
#include "SerialDataReceiver.h"

#define BUF_X 600
#define DATA_WIDTH 50
#define DATA_SIZE BUF_X * (DATA_WIDTH+1)
#define ID_DEPTH 201
#define ID_SONAR 202

#define ID_DUMMY_PANEL 1

PLUGIN_BEGIN_NAMESPACE

// how to declare a custom event. this can go in a header
DECLARE_EVENT_TYPE(wxEVT_MY_EVENT, -1)

wxDECLARE_EVENT(NEW_DATA_TYPE, wxCommandEvent);
wxDECLARE_EVENT(NEW_DEPTH_TYPE, wxCommandEvent);

class SonarPane;
class sonar_pi;

class SonarDisplayWindow : public wxPanel {
    public:
        SonarDisplayWindow(wxWindow* w_parent, const wxString& title, sonar_pi* sp, wxAuiManager* aui_mgr);
        ~SonarDisplayWindow();
        
        sonar_pi* m_pi;

        wxStaticText *t_label1;
        uint16_t index;
        float m_depth;
        char* msgbuf;
        uint16_t m_depth_max;
        uint16_t m_depths[BUF_X] = {{0}}; 
        int m_display_mode;

        bool StartDataReceiver(bool clear);
        void StopDataReceiver();
        void SetSpeed(double kn);
        void OnTimer(wxTimerEvent& WXUNUSED(event)) { m_depth_avail = true; }
        void ShowFrame();
        void HideFrame();
        void FrameClosed(wxAuiManagerEvent& event);
        int GetDisplayMode();
        void SetDisplayMode(int display_mode);
        void SetFrequency(uint8_t frequency);
        uint8_t GetFrequency();

   private:
        SonarPane *sopa;
        struct timespec begin, end;
        bool m_depth_avail;
        wxUint8 m_frequency;
        wxStaticText* m_st_slider;
        int m_depth_value;
        wxAuiManager* m_aui_mgr;
        wxSlider* m_slider;
        wxTimer* m_timer;
        //wxButton *m_btnOCLOSE;
        wxMenu* m_menu_docked;
        wxMenu* m_menu_floating;
        wxPanel* dummy;
        wxToggleButton* m_tgl_monochrome;
        double m_speed;
        bool m_docked;
        wxBoxSizer* m_cp_sizer;
        wxBoxSizer* m_sizer;

        void OnContextMenu(wxContextMenuEvent& event);
        void OnContextMenuSelect(wxCommandEvent& event);
        void OnLeftClick(wxMouseEvent& event);
        void OnSliderRelease(wxScrollEvent& event);
        void OnClose(wxCloseEvent& event);
        //void OnClose(wxCommandEvent& event);
        void OnSize( wxSizeEvent& event );
        void OnBtnMonochrome(wxCommandEvent& event);        
        void NewDepthData(wxCommandEvent& evt);
        void NewSonarData(wxCommandEvent& evt);
        //uint8_t t_data_buffer[BUF_X][50];
    protected:

        wxCriticalSection m_pThreadCS;
        IDataReceiver *m_data_receiver;
        friend class UDPDataReceiver;
        friend class SerialDataReceiver;
    
    DECLARE_EVENT_TABLE()
};


PLUGIN_END_NAMESPACE
#endif