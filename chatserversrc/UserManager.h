/** 
 *  �������е��û���Ϣ����ʼ��Ϣ�����ݿ��м���, UserManager.h
 *  zhangyl 2017.03.15
 **/

#pragma once
#include <stdint.h>
#include <string>
#include <list>
#include <mutex>
#include <set>
#include <atomic>

using namespace std;

#define GROUPID_BOUBDARY   0x0FFFFFFF 

#define DEFAULT_TEAMNAME  "My Friends"

enum FRIEND_OPERATION
{
    FRIEND_OPERATION_ADD,
    FRIEND_OPERATION_DELETE
};

struct FriendInfo
{
    int32_t friendid;
    string  markname;
    string  teamname;
};

//�û�����Ⱥ
struct User
{
    int32_t        userid;      //0x0FFFFFFF������Ⱥ�ţ���������ͨ�û�
    string         username;    //Ⱥ�˻���usernameҲ��Ⱥ��userid���ַ�����ʽ
    string         password;
    string         nickname;    //Ⱥ�˺�ΪȺ����
    int32_t        facetype;
    string         customface;
    string         customfacefmt;//�Զ���ͷ���ʽ
    int32_t        gender;
    int32_t        birthday;
    string         signature;
    string         address;
    string         phonenumber;
    string         mail;
    /*
    �����û����ѷ�����Ϣ������Ⱥ�˻���Ϊ�գ�����:
    [{"teamname": "�ҵĺ���"}, {"teamname": "�ҵ�ͬ��"}, {"teamname": "��ҵ�ͻ�"}]
    */
    string             teaminfo;       //������ͨ�û���Ϊ������Ϣ������Ⱥ����Ϊ��
    int32_t            ownerid;        //����Ⱥ�˺ţ�ΪȺ��userid
    list<FriendInfo>   friends;        
};

class UserManager final
{
public:
    UserManager();
    ~UserManager();

    bool Init(const char* dbServer, const char* dbUserName, const char* dbPassword, const char* dbName);

    UserManager(const UserManager& rhs) = delete;
    UserManager& operator=(const UserManager& rhs) = delete;

    bool AddUser(User& u);
    bool MakeFriendRelationshipInDB(int32_t smallUserid, int32_t greaterUserid);
    bool ReleaseFriendRelationshipInDBAndMemory(int32_t smallUserid, int32_t greaterUserid);
    bool UpdateUserRelationshipInMemory(int32_t userid, int32_t target, FRIEND_OPERATION operation);
    bool AddFriendToUser(int32_t userid, int32_t friendid);
    bool DeleteFriendToUser(int32_t userid, int32_t friendid);

    bool IsFriend(int32_t userid, int32_t friendid);
    
    //TODO: ���඼�Ǹ����û���Ϣ�Ľӿڣ��������Կ���ͳһ����
    bool UpdateUserInfoInDb(int32_t userid, const User& newuserinfo);
    bool ModifyUserPassword(int32_t userid, const std::string& newpassword);
    //���ڴ�����ݿ��е�ĳ���û��ķ�����Ϣ�ĳ��µ�newteaminfo
    bool UpdateUserTeamInfoInDbAndMemory(int32_t userid, const std::string& newteaminfo);
    bool DeleteTeam(int32_t userid, const std::string& deletedteamname);
    bool ModifyTeamName(int32_t userid, const std::string& newteamname, const std::string& oldteamname);
    
    //�����û����ѱ�ע��
    bool UpdateMarknameInDb(int32_t userid, int32_t friendid, const std::string& newmarkname);
    //�ƶ���������������
    bool MoveFriendToOtherTeam(int32_t userid, int32_t friendid, const std::string& newteamname);

    bool AddGroup(const char* groupname, int32_t ownerid, int32_t& groupid);

    //������Ϣ���
    bool SaveChatMsgToDb(int32_t senderid, int32_t targetid, const std::string& chatmsg);

    //TODO: ���û�Խ��Խ�࣬������Խ��Խ���ʱ�����ϵ�еĺ���Ч�ʸ���
    bool GetUserInfoByUsername(const std::string& username, User& u);
    bool GetUserInfoByUserId(int32_t userid, User& u);
    bool GetUserInfoByUserId(int32_t userid, User*& u);
    bool GetFriendInfoByUserId(int32_t userid, std::list<User>& friends);
    //��ȡ���ѵı�ע��
    bool GetFriendMarknameByUserId(int32_t userid1, int32_t friendid, std::string& markname);
    bool GetTeamInfoByUserId(int32_t userid, std::string& teaminfo);

private:
    bool LoadUsersFromDb();
    bool LoadRelationshipFromDb(int32_t userid, std::list<FriendInfo>& r);

private:
    std::atomic_int     m_baseUserId{ 0 };        //m_baseUserId, ȡ���ݿ�����userid���ֵ�������û�����������ϵ���
    std::atomic<int>    m_baseGroupId{0x0FFFFFFF};
    list<User>          m_allCachedUsers;
    mutex               m_mutex;

    string              m_strDbServer;
    string              m_strDbUserName;
    string              m_strDbPassword;
    string              m_strDbName;
};