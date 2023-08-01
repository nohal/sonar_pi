#ifndef _SERIALDATARECEIVER_H_
#define _SERIALDATARECEIVER_H_

#include "pi_common.h"
#include "IDataReceiver.h"
#include "SonarDisplayWindow.h"
#include "serialib.h"

#if defined (__unix__) || (defined (__APPLE__) && defined (__MACH__))
    #define CLOSE(x)    close(x);
#else
    #define CLOSE(x)    _close(x);
#endif

PLUGIN_BEGIN_NAMESPACE

class SerialDataReceiver: public IDataReceiver {

    public:
        SerialDataReceiver(SonarDisplayWindow* app);
        ~SerialDataReceiver();

        void Startup();
        void Shutdown();
        char* GetData();
        virtual ExitCode Entry();
        void SetFrequency(uint8_t frequency);

    private:
        SonarDisplayWindow* m_app;
        serialib serial;
        char buf[50];
        int m_fd;
        struct sockaddr_in addr;
        int addrlen;
        bool m_running;
        uint8_t m_frequency;

        void parse_message(uint8_t* buf);

    protected:
        void SendCommand(uint8_t cmd);
};



PLUGIN_END_NAMESPACE

#endif