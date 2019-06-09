/**
 * TcpSession.cpp
 * zhangyl 2017.03.09
 **/
#include "TcpSession.h"
#include "../base/AsyncLog.h"
#include "../base/Singleton.h"
#include "../net/ProtocolStream.h"
#include "../zlib1.2.11/ZlibUtil.h"
#include "ChatServer.h"
#include "Msg.h"

TcpSession::TcpSession(const std::weak_ptr<TcpConnection>& tmpconn) : tmpConn_(tmpconn)
{
    
}

TcpSession::~TcpSession()
{
    
}

void TcpSession::Send(int32_t cmd, int32_t seq, const std::string& data)
{
    Send(cmd, seq, data.c_str(), data.length());
}

void TcpSession::Send(int32_t cmd, int32_t seq, const char* data, int32_t dataLength)
{
    std::string outbuf;
    net::BinaryWriteStream writeStream(&outbuf);
    writeStream.WriteInt32(cmd);
    writeStream.WriteInt32(seq);
    writeStream.WriteCString(data, dataLength);
    writeStream.Flush();

    SendPackage(outbuf.c_str(), outbuf.length());
}

void TcpSession::Send(const std::string& p)
{
    SendPackage(p.c_str(), p.length());
}

void TcpSession::Send(const char* p, int32_t length)
{
    SendPackage(p, length);
}

void TcpSession::SendPackage(const char* p, int32_t length)
{   
    string srcbuf(p, length);
    string destbuf;
    if (!ZlibUtil::CompressBuf(srcbuf, destbuf))
    {
        LOGE("compress buf error");
        return;
    }
 
    string strPackageData;
    msg header;
    header.compressflag = 1;
    header.compresssize = destbuf.length();
    header.originsize = length;
    if (Singleton<ChatServer>::Instance().IsLogPackageBinaryEnabled())
    {
        LOGI("Send data, header length: %d, body length: %d", sizeof(header), destbuf.length());
    }
    
    //����һ����ͷ
    strPackageData.append((const char*)&header, sizeof(header));
    strPackageData.append(destbuf);

    //TODO: ��ЩSession��connection�������������Ҫ�ú�����һ��
    if (tmpConn_.expired())
    {
        //FIXME: ��������������Ҫ�Ų�
        LOGE("Tcp connection is destroyed , but why TcpSession is still alive ?");
        return;
    }

    std::shared_ptr<TcpConnection> conn = tmpConn_.lock();
    if (conn)
    {
        if (Singleton<ChatServer>::Instance().IsLogPackageBinaryEnabled())
        {
            size_t length = strPackageData.length();
            LOGI("Send data, package length: %d", length);
            //LOG_DEBUG_BIN((unsigned char*)strPackageData.c_str(), length);
        }
        
        conn->send(strPackageData);
    }
}