#include "SerialDataReceiver.h"
#include "sonar_pi.h"
#ifdef _WIN32
    #include <Winsock2.h> // before Windows.h, else Winsock 1 conflict
    #include <Ws2tcpip.h> // needed for ip_mreq definition for multicast
    #include <Windows.h>
#else
    #include <sys/socket.h>
    #include <arpa/inet.h>
#endif

PLUGIN_BEGIN_NAMESPACE

#define OPENING_SUCCESS                  1
#define DEVICE_NOT_FOUND                -1
#define ERROR_WHILE_OPENING             -2
#define ERROR_WHILE_GETTING_PARAMETERS  -3
#define BAUDS_NOT_RECOGNIZED            -4
#define ERROR_WRITING_PARAMETERS        -5
#define ERROR_WHILE_WRITING_TIMEOUT     -6
#define DATABITS_NOT_RECOGNIZED         -7
#define STOPBITS_NOT_RECOGNIZED         -8
#define PARITY_NOT_RECOGNIZED           -9

SerialDataReceiver::SerialDataReceiver(SonarDisplayWindow* app) : wxThread(wxTHREAD_JOINABLE) {
    m_app       = app;
    m_frequency = DEFAULT_FREQUENCY;
}

void SerialDataReceiver::Startup() {
    int error_opening = serial.openDevice(m_app->m_pi->m_serial_port, m_app->m_pi->m_serial_baud);
    m_running = error_opening == 1;

    if (m_running) {
        SendCommand(MODE_OFF);
        SendCommand(m_frequency);
        SendCommand(MODE_SONAR);
    } else {
        switch(error_opening) {
            case DEVICE_NOT_FOUND:
                wxLogMessage(wxString::Format("Port %s not found.", m_app->m_pi->m_serial_port));
                break;
            case ERROR_WHILE_OPENING:
                wxLogMessage(wxString::Format("Port %s could not be opened with baud %d.", m_app->m_pi->m_serial_port, m_app->m_pi->m_serial_baud));
                break;
        }
        wxLogMessage(_T("Will not receive any sonar data nor providing depth information to the system."));
    }

    // enable multicast transmission
    if (m_running && m_app->m_pi->m_serial_send_multicast) {
        
            m_fd = socket(AF_INET, SOCK_DGRAM, 0);
            if (m_fd < 0) {
                perror("socket");
                // TODO: better exception handling
            } else {

                char val = IP_PMTUDISC_DO;
                setsockopt(m_fd, IPPROTO_IP, IP_MTU_DISCOVER, &val, sizeof(val));

                memset(&addr, 0, sizeof(addr));
                addr.sin_family = AF_INET;
                addr.sin_addr.s_addr = inet_addr(const_cast<char*>((const char*)m_app->m_pi->m_ip_address.ToUTF8()));
                addr.sin_port = htons(m_app->m_pi->m_ip_port);
                
                addrlen = sizeof(addr);
            }
    }
}


SerialDataReceiver::~SerialDataReceiver() {
    wxCriticalSectionLocker enter(m_app->m_pThreadCS);
    m_app->m_data_receiver = nullptr;
    m_app = nullptr;
};

void SerialDataReceiver::SendCommand(uint8_t cmd) {
        serial.writeBytes(&cmd, 1);
}


void SerialDataReceiver::Shutdown() {

    if (serial.isDeviceOpen()) {
        SendCommand(MODE_OFF);

        serial.closeDevice();
    }
    shutdown(m_fd, SHUT_RDWR);
    _close(m_fd);

    m_running = false;

}

void SerialDataReceiver::SetFrequency(uint8_t frequency) {
    if (frequency < 100 || frequency > 250) return;
    m_frequency = frequency;
}

char* SerialDataReceiver::GetData() {
    return buf;
}

void SerialDataReceiver::parse_message(char* msgbuf) {
    
    if(msgbuf[0]== ID_DEPTH) {
        
        wxCommandEvent *evt = new wxCommandEvent(NEW_DEPTH_TYPE);
        
        evt->SetInt(msgbuf[1] *100 + msgbuf[2]);
        wxQueueEvent(m_app, evt);

        if(m_app->m_pi->m_serial_send_multicast) {
            sendto(
                m_fd,
                msgbuf,
                3,
                0,
                (struct sockaddr *) &addr,
                addrlen
            );
        }


    } else if(msgbuf[0]== ID_SONAR) {
        
        wxCommandEvent *evt = new wxCommandEvent(NEW_DATA_TYPE);
    
        memcpy(buf, msgbuf+1, sizeof(buf));
        
        wxQueueEvent(m_app, evt);
        
        if(m_app->m_pi->m_serial_send_multicast) {
            sendto(
                m_fd,
                msgbuf,
                51,
                0,
                (struct sockaddr *) &addr,
                addrlen
            );
        }
    }
}

wxThread::ExitCode SerialDataReceiver::Entry() {
       
    char buf[51];

    while (!TestDestroy() && m_running) {
        

        if (serial.available()) {

            serial.readBytes(buf, 1);
            

            if (buf[0] == ID_DEPTH) {
                serial.readBytes(buf + 1, 2);
                parse_message(buf);
            } else if (buf[0] == ID_SONAR) {
                serial.readBytes(buf + 1, 50);
                parse_message(buf);
            }
        }

    }
#ifdef _WIN32
    //WSACleanup();
#endif

    wxLogMessage(_T("Serial Data Receiver Receiption Thread ended."));

    return (wxThread::ExitCode)0;     // success;
}

PLUGIN_END_NAMESPACE