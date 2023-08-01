#include "SonarPane.h"

#include "pi_common.h"
#include "sonar_pi.h"
#include <wx/glcanvas.h>

#ifdef __WXMAC__
#include <GLUT/glut.h>
#else
#include <GL/glut.h>
#endif


PLUGIN_BEGIN_NAMESPACE

static int attribs[] = { WX_GL_RGBA, WX_GL_DOUBLEBUFFER, WX_GL_DEPTH_SIZE, 0, WX_GL_SAMPLE_BUFFERS, 1, 0 };

SonarPane::SonarPane(SonarDisplayWindow* parent)
    : wxGLCanvas(parent, wxID_ANY, attribs, wxDefaultPosition, wxDefaultSize, wxFULL_REPAINT_ON_RESIZE, _T("")) {
        
    _x = this->GetSize().GetWidth();
    _y = this->GetSize().GetHeight();
    m_parent = parent;
    
    if (m_parent->m_pi->IsOpenGLEnabled()) {
        m_context = new wxGLContext(this);
        Bind(wxEVT_PAINT, &SonarPane::Render, this);
        Bind(wxEVT_SIZE, &SonarPane::OnResize, this);
    } else {
        m_context = nullptr;
        Hide();
    }
}

void SonarPane::ResetDataBuffer() {
    memset(data_buffer, 0, BUF_X * DATA_WIDTH * sizeof (uint8_t));
}

SonarPane::~SonarPane() {
    if (m_context) {
        Unbind(wxEVT_PAINT, &SonarPane::Render, this);
        Unbind(wxEVT_SIZE, &SonarPane::OnResize, this);
        delete m_context;
    }
}

void SonarPane::OnResize(wxSizeEvent& event) {
    Refresh(false);
    _x = event.GetSize().GetX();
    _y = event.GetSize().GetY();
}

void SonarPane::Render(wxPaintEvent& event) {
    if (!m_parent->m_pi->IsOpenGLEnabled()) return;

    // This is required even though dc is not used otherwise.
    wxPaintDC dc(this);
    
    SetCurrent(*m_context);
    GLfloat zoom = 1;
    GLfloat fac1;

    if (m_parent->m_depth < 1) {
        fac1 = 1 / 1.5;
    } else {
        fac1 = m_parent->m_depth / (m_parent->m_depth * 1.5);
    }

    GLfloat fac2 = 1 - fac1;

    GLfloat _h = (2.0 / DATA_WIDTH) * fac2;



    glPushMatrix();
    glPushAttrib(GL_ALL_ATTRIB_BITS);
    
    glClearColor((GLfloat)(m_parent->m_pi->m_background_colour.GetRed()/256.0),
    (GLfloat)(m_parent->m_pi->m_background_colour.GetGreen()/256.0),
    (GLfloat)(m_parent->m_pi->m_background_colour.GetBlue()/256.0), 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glViewport(0, 0, _x, _y);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glMatrixMode(GL_MODELVIEW);
    

    for (uint16_t i = 0; i < BUF_X; i++) {
        for (uint8_t j = 0; j < DATA_WIDTH; j++) {

            if (m_parent->m_display_mode == MONOCHROME) {

                GLfloat f = data_buffer[i][j] / 200.0;
                glColor4f((GLfloat)1.0f, (GLfloat)1.0f, (GLfloat)1.0f, f);

            } else if (m_parent->m_display_mode == MULTICOLOR) {
                
                if (data_buffer[i][j] < 67) {
                    //glColor3f(1.0f * (data_buffer[i][j] / 66.0), 0.0f, 0.0f);
                    glColor4f((GLfloat)1.0f, (GLfloat)0.0f, (GLfloat)0.0f, (GLfloat)(1.0f * (data_buffer[i][j] / 66.0)));
                } else if (data_buffer[i][j] < 134) {
                    //glColor3f(0.0f, 1.0f * ((data_buffer[i][j] - 67) / 66.0), 0.0f);
                    glColor4f((GLfloat)0.0f, (GLfloat)1.0f, (GLfloat)0.0f, (GLfloat)(1.0f * ((data_buffer[i][j] - 67) / 66.0)));
                } else if (data_buffer[i][j] <= 200) {
                    //glColor3f(0.0f, 0.0f, 1.0f * ((data_buffer[i][j] - 133) / 67.0));
                    glColor4f((GLfloat)0.0f, (GLfloat)0.0f, (GLfloat)1.0f, (GLfloat)(1.0f * ((data_buffer[i][j] - 133) / 67.0)));
                      
                }
            }
            if (m_parent->m_depth_max > 0)
                zoom = (float)m_parent->m_depths[i]
                    / (float)m_parent->m_depth_max;

            GLfloat h1 = ((j * _h - fac2) * -1) - fac1;
            GLfloat h2 = ((j * _h + _h - fac2) * -1) - fac1;

            h1 *= zoom;
            h2 *= zoom;

            GLfloat w1 = i * _w - 1;
            GLfloat w2 = i * _w + _w - 1;

            glRectf(w1, h1, w2, h2);
        }
    }
    
    glPopAttrib();
    glPopMatrix();

    SwapBuffers();
}

PLUGIN_END_NAMESPACE