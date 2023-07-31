#ifndef _SERIALDATARECEIVER_H_
#define _SERIALDATARECEIVER_H_

#include "pi_common.h"
#include "IDataReceiver.h"
#include "SonarDisplayWindow.h"
#include "serialib.h"

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

        void parse_message(char* buf);

    protected:
        void SendCommand(uint8_t cmd);
};



PLUGIN_END_NAMESPACE

#endif