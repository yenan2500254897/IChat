/**
 * ��ط������࣬MonitorServer.cpp
 * zhangyl 2018.03.09
 */
#include "../net/InetAddress.h"
#include "../base/AsyncLog.h"
#include "../base/Singleton.h"
#include "../net/TcpServer.h"
#include "../net/EventLoop.h"
#include "../net/EventLoopThread.h"
#include "../net/EventLoopThreadPool.h"
#include "MonitorSession.h"
#include "MonitorServer.h"

bool MonitorServer::Init(const char* ip, short port, EventLoop* loop, const char* token)
{
    m_token = token;
     
    InetAddress addr(ip, port);
    m_server.reset(new TcpServer(loop, addr, "ZYL-MYIMMONITORSERVER", TcpServer::kReusePort));
    m_server->setConnectionCallback(std::bind(&MonitorServer::OnConnection, this, std::placeholders::_1));
    //��������
    m_server->start(1);

    return true;
}

void MonitorServer::Uninit()
{
    if (m_server)
        m_server->stop();
}

//�����ӵ������û����ӶϿ���������Ҫͨ��conn->connected()���жϣ�һ��ֻ����loop�������
void MonitorServer::OnConnection(std::shared_ptr<TcpConnection> conn)
{
    if (conn->connected())
    {
        std::shared_ptr<MonitorSession> spSession(new MonitorSession(conn));
        conn->setMessageCallback(std::bind(&MonitorSession::OnRead, spSession.get(), std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));

        {
            std::lock_guard<std::mutex> guard(m_sessionMutex);
            m_sessions.push_back(spSession);
        }
        
        spSession->ShowHelp();
    }
    else
    {
        OnClose(conn);
    }
}

//���ӶϿ�
void MonitorServer::OnClose(const std::shared_ptr<TcpConnection>& conn)
{
    //TODO: �����Ĵ����߼�̫���ң���Ҫ�Ż�
    std::lock_guard<std::mutex> guard(m_sessionMutex);
    for (auto iter = m_sessions.begin(); iter != m_sessions.end(); ++iter)
    {
        if ((*iter)->GetConnectionPtr() == NULL)
        {
            LOGE("connection is NULL");
            break;
        }

        //ͨ���ȶ�connection�����ҵ���Ӧ��session
        if ((*iter)->GetConnectionPtr() == conn)
        {
            m_sessions.erase(iter);
            LOGI("monitor client disconnected: %s", conn->peerAddress().toIpPort().c_str());
            break;
        }
    }
}

bool MonitorServer::IsMonitorTokenValid(const char* token)
{
    if (token == NULL || token[0] == '\0')
        return false;

    return m_token == token;
}