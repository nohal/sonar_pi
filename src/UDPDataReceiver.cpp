

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <iostream>

#include "sonar_pi.h"

#include "SonarDisplayWindow.h"
#include "UDPDataReceiver.h"
#include <signal.h>

PLUGIN_BEGIN_NAMESPACE

UDPDataReceiver::UDPDataReceiver(SonarDisplayWindow* app, sonar_pi* sp) : wxThread(wxTHREAD_JOINABLE)  { 
    m_app       = app;
    m_pi        = sp;
    m_has_addr  = false;
}   

UDPDataReceiver::~UDPDataReceiver() {
    wxCriticalSectionLocker enter(m_app->m_pThreadCS);
    m_app->m_data_receiver = nullptr;
    m_app = nullptr;
};

void UDPDataReceiver::Startup() {

    //TODO: implement only if a secure way can be found
    /*SendCommand(MODE_OFF);
    SendCommand(m_frequency);
    SendCommand(MODE_SONAR)*/;

#if defined(WIN32) || defined(_WIN32) || defined(__WIN32__)
    //
    // Initialize Windows Socket API with given VERSION.
    //
    WSADATA wsaData;
    if (WSAStartup(0x0101, &wsaData)) {
        perror("WSAStartup");
        //return (wxThread::ExitCode)1;  
    }
#endif

    // create what looks like an ordinary UDP socket
    //
    fd = socket(AF_INET, SOCK_DGRAM, 0);
    
    if (fd < 0) {
        perror("socket");
        // TODO: erro handling
    }

    // allow multiple sockets to use the same PORT number
    //
    u_int yes = 1;
    if (
        setsockopt(
            fd, SOL_SOCKET, SO_REUSEADDR, (char*) &yes, sizeof(yes)
        ) < 0
    ){
       perror("Reusing ADDR failed");
       // TODO: erro handling
    }

    // set up destination address
    //
    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_ANY); // differs from sender
    addr.sin_port = htons(m_pi->m_ip_port);

    // bind to receive address
    //
    if (::bind(fd, (struct sockaddr*) &addr, sizeof(addr)) < 0) {
        perror("bind");
        // TODO: erro handling
    }

    // use setsockopt() to request that the kernel join a multicast group
    //
    struct ip_mreq mreq;
    mreq.imr_multiaddr.s_addr = inet_addr(const_cast<char*>((const char*)m_pi->m_ip_address.ToUTF8()));
    mreq.imr_interface.s_addr = htonl(INADDR_ANY);
    if (
        setsockopt(
            fd, IPPROTO_IP, IP_ADD_MEMBERSHIP, (char*) &mreq, sizeof(mreq)
        ) < 0
    ){
        perror("setsockopt");
        // TODO: error handling
    }
    

}

void UDPDataReceiver::SetFrequency(wxUint8 frequency) {
    if (frequency < 100 || frequency > 250) return;
    m_frequency = frequency;
}

char* UDPDataReceiver::GetData() {
    return buf;
}

void UDPDataReceiver::parse_message(uint8_t* msgbuf) {
    if(msgbuf[0]== ID_DEPTH) {
        wxCommandEvent *evt = new wxCommandEvent(NEW_DEPTH_TYPE);
        evt->SetInt(msgbuf[1] * 100 + msgbuf[2]);
        wxQueueEvent(m_app, evt);
    } else if(msgbuf[0]== ID_SONAR) {
        memcpy(buf, msgbuf+1, sizeof(buf));
        wxCommandEvent *evt = new wxCommandEvent(NEW_DATA_TYPE);
        wxQueueEvent(m_app, evt);       
    }
}

void UDPDataReceiver::SendCommand(char cmd) {
    if (!m_has_addr) return;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_addr = m_remote_ip_address;
    addr.sin_port = htons(m_pi->m_ip_port + 1);

    int _fd = socket(AF_INET, SOCK_STREAM, 0);
    
    
    sockaddr *sa = (struct sockaddr *) &addr;
    int addrlen = sizeof(addr);

    int status = connect(_fd, sa, addrlen);
    if (status == 0) {
        char _b[1]= {cmd};

        send(_fd, _b, 1, 0
        );
        
        shutdown(_fd, SHUT_WR);
    }
    CLOSE(_fd);
}

wxThread::ExitCode UDPDataReceiver::Entry() {
    uint8_t msgbuf[MSGBUFSIZE];
    struct sockaddr_in from;
    socklen_t len = sizeof(from);

    while (!TestDestroy()) {
        int nbytes = recvfrom(
            fd,
            reinterpret_cast<char*>(msgbuf),
            MSGBUFSIZE,
            0,
            (struct sockaddr*)&from,
            &len
        );
        if (nbytes == -1) {
            continue;
        }
        if (nbytes < 0) {
            perror("recvfrom");
            return (wxThread::ExitCode)0;  
        }

        if (nbytes > 0) {

            if (msgbuf[0] == ID_SONAR || msgbuf[0] == ID_DEPTH) {
                m_remote_ip_address = from.sin_addr;
                m_has_addr = true;
                
                /*SendCommand(MODE_OFF);
                SendCommand(m_frequency);
                SendCommand(MODE_SONAR);*/
                
                parse_message(msgbuf);
            }


        
        }
     }

#if defined(WIN32) || defined(_WIN32) || defined(__WIN32__)
    WSACleanup();
#endif


    return (wxThread::ExitCode)0;     // success;
}

void UDPDataReceiver::Shutdown() {
    shutdown(fd, SHUT_RDWR);
    CLOSE(fd);
}

PLUGIN_END_NAMESPACE
