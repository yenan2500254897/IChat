/**
 *  �������������࣬IMServer.cpp
 *  zhangyl 2017.03.09
 **/
#include "ChatServer.h"

#include "../net/InetAddress.h"
#include "../base/AsyncLog.h"
#include "../base/Singleton.h"
#include "ChatSession.h"
#include "UserManager.h"

ChatServer::ChatServer()
{
    m_logPackageBinary = false;
}

bool ChatServer::Init(const char* ip, short port, EventLoop* loop)
{   
    InetAddress addr(ip, port);
    m_server.reset(new TcpServer(loop, addr, "FLAMINGO-SERVER", TcpServer::kReusePort));
    m_server->setConnectionCallback(std::bind(&ChatServer::OnConnection, this, std::placeholders::_1));
    //��������
    m_server->start(6);

    return true;
}

void ChatServer::Uninit()
{
    if (m_server)
        m_server->stop();
}

void ChatServer::EnableLogPackageBinary(bool enable)
{
    m_logPackageBinary = enable;
}

bool ChatServer::IsLogPackageBinaryEnabled()
{
    return m_logPackageBinary;
}

void ChatServer::OnConnection(std::shared_ptr<TcpConnection> conn)
{
    if (conn->connected())
    {
        LOGD("client connected: %s", conn->peerAddress().toIpPort().c_str());
        ++m_sessionId;
        std::shared_ptr<ChatSession> spSession(new ChatSession(conn, m_sessionId));
        conn->setMessageCallback(std::bind(&ChatSession::OnRead, spSession.get(), std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));       

        std::lock_guard<std::mutex> guard(m_sessionMutex);
        m_sessions.push_back(spSession);
    }
    else
    {
        OnClose(conn);
    }
}

void ChatServer::OnClose(const std::shared_ptr<TcpConnection>& conn)
{
    //�Ƿ����û�����
    //bool bUserOffline = false;
    UserManager& userManager = Singleton<UserManager>::Instance();

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
            //��Session����֮ǰ�������ߵ���ЧSession������Ϊ���������ߣ��Ÿ������������������Ϣ
            if ((*iter)->IsSessionValid())
            { 
                //���������ߺ��ѣ��������������������Ϣ
                std::list<User> friends;
                int32_t offlineUserId = (*iter)->GetUserId();
                userManager.GetFriendInfoByUserId(offlineUserId, friends);
                for (const auto& iter2 : friends)
                {
                    for (auto& iter3 : m_sessions)
                    {
                        //�ú����Ƿ����ߣ����߻����session��
                        if (iter2.userid == iter3->GetUserId())
                        {
                            iter3->SendUserStatusChangeMsg(offlineUserId, 2);

                            LOGI("SendUserStatusChangeMsg to user(userid=%d): user go offline, offline userid = %d", iter3->GetUserId(), offlineUserId);
                        }
                    }
                }
            }
            else
            {
                LOGI("Session is invalid, userid=%d", (*iter)->GetUserId());
            }
            
            //ͣ����Session�ĵ��߼��
            //(*iter)->DisableHeartbaetCheck();
            //�û�����
            m_sessions.erase(iter);
            //bUserOffline = true;
            LOGI("client disconnected: %s", conn->peerAddress().toIpPort().c_str());
            break;
        }
    }

    LOGI("current online user count: %d", (int)m_sessions.size());
}

void ChatServer::GetSessions(std::list<std::shared_ptr<ChatSession>>& sessions)
{
    std::lock_guard<std::mutex> guard(m_sessionMutex);
    sessions = m_sessions;
}

bool ChatServer::GetSessionByUserIdAndClientType(std::shared_ptr<ChatSession>& session, int32_t userid, int32_t clientType)
{
    std::lock_guard<std::mutex> guard(m_sessionMutex);
    std::shared_ptr<ChatSession> tmpSession;
    for (const auto& iter : m_sessions)
    {
        tmpSession = iter;
        if (iter->GetUserId() == userid && iter->GetClientType() == clientType)
        {
            session = tmpSession;
            return true;
        }
    }

    return false;
}

bool ChatServer::GetSessionsByUserId(std::list<std::shared_ptr<ChatSession>>& sessions, int32_t userid)
{
    std::lock_guard<std::mutex> guard(m_sessionMutex);
    std::shared_ptr<ChatSession> tmpSession;
    for (const auto& iter : m_sessions)
    {
        tmpSession = iter;
        if (iter->GetUserId() == userid)
        {
            sessions.push_back(tmpSession);
            return true;
        }
    }

    return false;
}

int32_t ChatServer::GetUserStatusByUserId(int32_t userid)
{
    std::lock_guard<std::mutex> guard(m_sessionMutex);
    for (const auto& iter : m_sessions)
    {
        if (iter->GetUserId() == userid)
        {
            return iter->GetUserStatus();
        }
    }

    return 0;
}

int32_t ChatServer::GetUserClientTypeByUserId(int32_t userid)
{
    std::lock_guard<std::mutex> guard(m_sessionMutex);
    bool bMobileOnline = false;
    int clientType = CLIENT_TYPE_UNKOWN;
    for (const auto& iter : m_sessions)
    {
        if (iter->GetUserId() == userid)
        {   
            clientType = iter->GetUserClientType();
            //��������ֱ�ӷ��ص�������״̬
            if (clientType == CLIENT_TYPE_PC)
                return clientType;
            else if (clientType == CLIENT_TYPE_ANDROID || clientType == CLIENT_TYPE_IOS)
                bMobileOnline = true;
        }
    }

    //ֻ���ֻ����߲ŷ����ֻ�����״̬
    if (bMobileOnline)
        return clientType;

    return CLIENT_TYPE_UNKOWN;
}