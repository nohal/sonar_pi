#ifndef _SONAR_PANE_H_
#define _SONAR_PANE_H_

#include "pi_common.h"

#include <GL/glu.h>
#include <wx/glcanvas.h>

#include "SonarDisplayWindow.h"

PLUGIN_BEGIN_NAMESPACE

class SonarPane : public wxGLCanvas {

    public:
        SonarPane(SonarDisplayWindow* parent);
        virtual ~SonarPane();
        
        uint8_t data_buffer[BUF_X][DATA_WIDTH] ; 

        void ResetDataBuffer();

    private:
        SonarDisplayWindow* m_parent;

        float _w = 2.0 / BUF_X;
        int _x;
        int _y;
        wxGLContext*	m_context;
        struct timespec begin, end;
                
        void OnResize(wxSizeEvent& event);
        void Render(wxPaintEvent& WXUNUSED(event));

};

PLUGIN_END_NAMESPACE

#endif 
