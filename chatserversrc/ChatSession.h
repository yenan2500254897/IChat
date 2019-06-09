/** 
 * ChatSession.h
 * zhangyl, 2017.03.10
 **/

#pragma once
#include "../net/Buffer.h"
#include "../net/TimerId.h"
#include "TcpSession.h"
using namespace net;

struct OnlineUserInfo
{
    int32_t     userid;
    std::string username;
    std::string nickname;
    std::string password;
    int32_t     clienttype;     //�ͻ�������, 0δ֪, pc=1, android/ios=2
    int32_t     status;         //����״̬ 0���� 1���� 2æµ 3�뿪 4����
};

/**
 * ����Ự��
 */
class ChatSession : public TcpSession
{
public:
    ChatSession(const std::shared_ptr<TcpConnection>& conn, int sessionid);
    virtual ~ChatSession();

    ChatSession(const ChatSession& rhs) = delete;
    ChatSession& operator =(const ChatSession& rhs) = delete;

    //�����ݿɶ�, �ᱻ�������loop����
    void OnRead(const std::shared_ptr<TcpConnection>& conn, Buffer* pBuffer, Timestamp receivTime);   
    
    int32_t GetSessionId()
    {
        return m_id;
    }

    int32_t GetUserId()
    {
        return m_userinfo.userid;
    }

    std::string GetUsername()
    {
        return m_userinfo.username;
    }

    std::string GetNickname()
    {
        return m_userinfo.nickname;
    }

    std::string GetPassword()
    {
        return m_userinfo.password;
    }

    int32_t GetClientType()
    {
        return m_userinfo.clienttype;
    }

    int32_t GetUserStatus()
    {
        return m_userinfo.status;
    }

    int32_t GetUserClientType()
    {
        return m_userinfo.clienttype;
    }

    /**
     *@param type ȡֵ�� 1 �û����ߣ� 2 �û����ߣ� 3 �����ǳơ�ͷ��ǩ������Ϣ����
     */
    void SendUserStatusChangeMsg(int32_t userid, int type, int status = 0);

    //��SessionʧЧ�����ڱ������ߵ��û���session
    void MakeSessionInvalid();
    bool IsSessionValid();

    void EnableHearbeatCheck();
    void DisableHeartbeatCheck();

    //��������������ָ��ʱ���ڣ�������30�룩δ�յ����ݰ����������Ͽ��ڿͻ��˵�����
    void CheckHeartbeat(const std::shared_ptr<TcpConnection>& conn);

private:
    bool Process(const std::shared_ptr<TcpConnection>& conn, const char* inbuf, size_t buflength);
    
    void OnHeartbeatResponse(const std::shared_ptr<TcpConnection>& conn);
    void OnRegisterResponse(const std::string& data, const std::shared_ptr<TcpConnection>& conn);
    void OnLoginResponse(const std::string& data, const std::shared_ptr<TcpConnection>& conn);
    void OnGetFriendListResponse(const std::shared_ptr<TcpConnection>& conn);
    void OnFindUserResponse(const std::string& data, const std::shared_ptr<TcpConnection>& conn);
    void OnChangeUserStatusResponse(const std::string& data, const std::shared_ptr<TcpConnection>& conn);
    void OnOperateFriendResponse(const std::string& data, const std::shared_ptr<TcpConnection>& conn);
    void OnAddGroupResponse(int32_t groupId, const std::shared_ptr<TcpConnection>& conn);
    void OnUpdateUserInfoResponse(const std::string& data, const std::shared_ptr<TcpConnection>& conn);
    void OnModifyPasswordResponse(const std::string& data, const std::shared_ptr<TcpConnection>& conn);
    void OnCreateGroupResponse(const std::string& data, const std::shared_ptr<TcpConnection>& conn);
    void OnGetGroupMembersResponse(const std::string& data, const std::shared_ptr<TcpConnection>& conn);
    void OnChatResponse(int32_t targetid, const std::string& data, const std::shared_ptr<TcpConnection>& conn);
    void OnMultiChatResponse(const std::string& targets, const std::string& data, const std::shared_ptr<TcpConnection>& conn);
    void OnScreenshotResponse(int32_t targetid, const std::string& bmpHeader, const std::string& bmpData, const std::shared_ptr<TcpConnection>& conn);
    void OnUpdateTeamInfoResponse(int32_t operationType, const std::string& newTeamName, const std::string& oldTeamName, const std::shared_ptr<TcpConnection>& con);
    void OnModifyMarknameResponse(int32_t friendid, const std::string& newmarkname, const std::shared_ptr<TcpConnection>& conn);
    void OnMoveFriendToOtherTeamResponse(int32_t friendid, const std::string& newteamname, const std::string& oldteamname, const std::shared_ptr<TcpConnection>& conn);

    void DeleteFriend(const std::shared_ptr<TcpConnection>& conn, int32_t friendid);

    //�����û�������Ϣ��װӦ����ͻ��˵ĺ����б���Ϣ
    void MakeUpFriendListInfo(std::string& friendinfo, const std::shared_ptr<TcpConnection>& conn);

    //��������Ϣ�ı���ʱ��ĳɷ�����ʱ�䣬�޸ĳɹ�����true,ʧ�ܷ���false��
    bool ModifyChatMsgLocalTimeToServerTime(const std::string& chatInputJson, std::string& chatOutputJson);

private:
    int32_t           m_id;                 //session id
    OnlineUserInfo    m_userinfo;
    int32_t           m_seq;                //��ǰSession���ݰ����к�
    bool              m_isLogin;            //��ǰSession��Ӧ���û��Ƿ��Ѿ���¼
    time_t            m_lastPackageTime;    //��һ���շ�����ʱ��
    TimerId           m_checkOnlineTimerId; //����Ƿ����ߵĶ�ʱ��id
};