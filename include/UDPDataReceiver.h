#ifndef _UDP_DATA_RECEIVER_H_
#define _UDP_DATA_RECEIVER_H_

#include "IDataReceiver.h"
#ifdef _WIN32
    #include <Winsock2.h> // before Windows.h, else Winsock 1 conflict
    #include <Ws2tcpip.h> // needed for ip_mreq definition for multicast
    #include <Windows.h>
    #define SHUT_RD     0x00
    #define SHUT_WR     0x01
    #define SHUT_RDWR   0x02
#else
    #include <sys/types.h>
    #include <sys/socket.h>
    #include <netinet/in.h>
    #include <arpa/inet.h>
    #include <time.h>
#endif

#if defined (__unix__) || (defined (__APPLE__) && defined (__MACH__))
    #define CLOSE(x)    close(x);
#else
    #define CLOSE(x)    _close(x);
#endif

#define LOG(str)    std::cout << str << std::endl << std::flush;
#define MSGBUFSIZE 256


PLUGIN_BEGIN_NAMESPACE
class SonarDisplayWindow;
class sonar_pi;
class UDPDataReceiver : public IDataReceiver {

    public:
        UDPDataReceiver(SonarDisplayWindow* app, sonar_pi* sp);
        virtual ~UDPDataReceiver();

        virtual ExitCode Entry();
        void Shutdown();
        void Startup();
        char* GetData();
        void SetFrequency(wxUint8 frequency);

    private:
        SonarDisplayWindow* m_app;

        sonar_pi* m_pi;
        in_addr m_remote_ip_address;
        bool m_has_addr;
        struct sockaddr_in addr;
        int fd;
        char buf[50];
        bool m_running;
        uint8_t m_frequency;
        
        void SendCommand(char cmd);
        void parse_message(uint8_t* msgbuf);
};

PLUGIN_END_NAMESPACE

#endif