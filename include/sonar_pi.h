#ifndef _SONARPI_H_
#define _SONARPI_H_

#include "config.h"
#include "ocpn_plugin.h"
#include "pi_common.h"

#include "wx/wxprec.h"
#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif

#include <wx/fileconf.h>
#include "PreferencesWindow.h"
#include "SonarDisplayWindow.h"

//TODO: Needs cleanup

PLUGIN_BEGIN_NAMESPACE

//----------------------------------------------------------------------------------------------------------
//    The PlugIn Class Definition
//----------------------------------------------------------------------------------------------------------

enum OpenGLMode { OPENGL_UNKOWN, OPENGL_OFF, OPENGL_ON };

class sonar_pi : public opencpn_plugin_118 {
private:

    wxFileConfig *m_pconfig;
    bool LoadConfig(void);
    bool SaveConfig(void);
    wxAuiManager* m_aui_mgr;
    /// Color scheme used by the plugin
    int m_color_scheme;
    /// Bitmap representation of the logo of the plugin
    wxBitmap m_logo;
    /// Path to the configuration file
    wxString m_config_file;

    bool m_shown;

    //SerialHelper* sh;
    wxString _svg_dashboardsk;
    wxString _svg_dashboardsk_rollover;
    wxString _svg_dashboardsk_toggled;
    int pi_id;
    void SetOpenGLMode(OpenGLMode mode);
    OpenGLMode m_opengl_mode;


    wxGLContext* m_opencpn_gl_context = 0;
    int m_display_mode;
    int m_nmea_type;
    wxString m_nmea_id;
    bool m_nmea_enable;
public:
    bool m_serial_send_multicast;

    wxColour m_background_colour = *wxBLACK;

    wxString m_serial_port;
    int m_serial_baud;
    wxString m_ip_address;
    wxString m_data_interface;
    int m_ip_port;
    bool m_serial_port_open;
    int m_sonar_frequency;
    float m_sensor_offset;

    SonarDisplayWindow *sonarDisplayWindow;
    wxGLContext* GetChartOpenGLContext();
    void SendNMEABuffer(float depth);
    void closing();
    bool RenderGLOverlayMultiCanvas(wxGLContext *pcontext, PlugIn_ViewPort *vp, int canvasIndex);
    bool IsOpenGLEnabled() { return true; }// m_opengl_mode == OPENGL_ON; }
    /// Constructor
    ///
    /// \param ppimgr Pointer to the plugin manager
    explicit sonar_pi(void* ppimgr);

    /// Destructor
    ~sonar_pi() override;
    void UpdateAuiStatus() override;
    //    The required PlugIn Methods
    /// Initialize the plugin
    ///
    /// \return
    int Init() override;

    /// Deinitialize the plugin
    ///
    /// \return
    bool DeInit() override;

    /// Get major version of the plugin API the plugin requires
    ///
    /// \return Major version of the API
    int GetAPIVersionMajor() override;

    /// Get minor version of the plugin API the plugin requires
    ///
    /// \return Minor version of the API
    int GetAPIVersionMinor() override;

    /// Get major version of the plugin
    ///
    /// \return MAjor version of the plugin
    int GetPlugInVersionMajor() override;

    /// Get minor version of the plugin
    ///
    /// \return Minor version of the plugin
    int GetPlugInVersionMinor() override;

    /// Get bitmap icon of the plugin logo
    ///
    /// \return pointer to the bitmap containing the logo
    wxBitmap* GetPlugInBitmap() override;

    /// Get the name of the plugin
    ///
    /// \return Name of the plugin
    wxString GetCommonName() override;

    /// Get short description of the plugin
    /// The description should be a short single line text that fits the list
    /// view in the OpenCPN plugin manager tab of the Toolbox
    ///
    /// \return Short description of the plugin
    wxString GetShortDescription() override;

    /// Get long description of the plugin
    ///
    /// \return Longer text describing the plugin displayed on the plugin detail
    /// tile
    ///         in the OpenCPN plugin manager tab of the Toolbox once the plugin
    ///         is selected.
    wxString GetLongDescription() override;

    void ShowPreferencesDialog(wxWindow* parent) override;

    /// Set color scheme the plugin should use
    /// Invoked when the core application color scheme is changed
    ///
    /// \param cs Color scheme
    void SetColorScheme(PI_ColorScheme cs) override;

    /// Callback delivering NMEA messages from the core application
    ///
    /// \param sentence The NMEA 0183 message
    void SetNMEASentence(wxString& sentence) override;

    

    /// Get Path to the plaugin data
    ///
    /// \return Path to the plugin data including the trailing separator
    wxString GetDataDir();

    /// Get bitmap from SVG file
    ///
    /// \param filename Path to the SVG file
    /// \param w Width of the requested bitmap
    /// \param h Height of the requested bitmal
    /// \return Generated bitmap
    wxBitmap GetBitmapFromSVG(
        const wxString& filename, const wxCoord w, const wxCoord h);


int GetToolbarToolCount();
void OnToolbarToolCallback(int id);
};

PLUGIN_END_NAMESPACE

#endif //_SONARPI_H_
