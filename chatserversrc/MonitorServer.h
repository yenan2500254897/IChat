/**
 * ��ط������࣬MonitorServer.h
 * zhangyl 2018.03.09
 */
#ifndef __MONITOR_SERVER_H__
#define __MONITOR_SERVER_H__
#include "MonitorSession.h"
#include <memory>
#include <mutex>
#include <list>

using namespace net;

//class MonitorSession;

class MonitorServer final
{
public:
    MonitorServer() = default;
    ~MonitorServer() = default;
    MonitorServer(const MonitorServer& rhs) = delete;
    MonitorServer& operator =(const MonitorServer& rhs) = delete;

public:
    bool Init(const char* ip, short port, EventLoop* loop, const char* token);
    void Uninit();

    bool IsMonitorTokenValid(const char* token);

    //�����ӵ������û����ӶϿ���������Ҫͨ��conn->connected()���жϣ�һ��ֻ����loop�������
    void OnConnection(std::shared_ptr<TcpConnection> conn);
    //���ӶϿ�
    void OnClose(const std::shared_ptr<TcpConnection>& conn);

private:
    std::shared_ptr<TcpServer>                     m_server;
    std::list<std::shared_ptr<MonitorSession>>     m_sessions;
    std::mutex                                     m_sessionMutex;      //���߳�֮�䱣��m_sessions
    std::string                                    m_token;             //�鿴ĳЩ����������Ҫ��token
};

#endif //!__MONITOR_SERVER_H__