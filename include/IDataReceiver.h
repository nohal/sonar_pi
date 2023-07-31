#ifndef _IDATARECEIVER_H_
#define _IDATARECEIVER_H_

#include "pi_common.h"

#include "wx/wxprec.h"
#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif
#define LOG(str)    std::cout << str << std::endl << std::flush;

#define DEFAULT_FREQUENCY  150         // means 150kHz
#define MODE_OFF    0
#define MODE_SONAR  1
#define MODE_DEPTH  2

PLUGIN_BEGIN_NAMESPACE

/**
 * Interface for defining data receiver (e.g. IP or serial)
*/
class IDataReceiver: public virtual wxThread {

    public:
        virtual void Startup() = 0;
        virtual void Shutdown() = 0;
        virtual char* GetData() = 0;
        virtual void SetFrequency(wxUint8 frequency) = 0;
};

PLUGIN_END_NAMESPACE

#endif