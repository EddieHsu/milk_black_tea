
#include "stdafx.h"
#include "socket.h"



static bool m_Init_Flag=false;  
  
bool Socket::Initialize()  
{  
       if (!m_Init_Flag)  
       {  
           WSAData wsa_data;   
            if (WSAStartup(0x202,&wsa_data) != 0) return false; // ��l�ƥ��� //  
           m_Init_Flag = true;  
       }    
       return true;
}  

void Socket::Terminate()  
{   
    if (m_Init_Flag)  
    {  
        WSACleanup();  
        m_Init_Flag = false;  
    }    
}

Socket::Socket(void)  
//�����G�غcSocket����  
{  
    m_Socket = INVALID_SOCKET;  
} 

Socket::~Socket()  
//�����G�ѺcSocket����  
{  
    Close();  
}  

bool Socket::IsLocalHost(const char* hostname)  
//�����G�ˬd�O�_��localhost�I�s  
//��J�Ghostname = Server��}  
//�Ǧ^�G�O�_��localhost�I�s  
{  
   if (hostname == NULL) return true;  
   if (*hostname == 0) return true;  
   if (stricmp(hostname,"localhost") == 0) return true;  
   if (strcmp(hostname,"127.0.0.1") == 0) return true;  
   return false;  
}  

bool Socket::IsOpened(void)  
//�����G�˴�Socket�O�_�w�}��  
//�Ǧ^�G�˴����G  
{  
   if (m_Socket == INVALID_SOCKET) return false;  
   return true;  
} 

bool Socket::Open(const char* hostname, int port)  
//�����G�}�һPServer���s�u  
//��J�Ghostname,port = Server��}�P�q�T��  
//�Ǧ^�G���ѶǦ^false  
{  
   Close();  
   if (!Initialize()) return false;  
   struct sockaddr_in sock_addr;  
   // �ѥXsocket address //  
   if (IsLocalHost(hostname)) hostname = "127.0.0.1";  
   sock_addr.sin_family = AF_INET; // ARPA�����зǨ�w
   sock_addr.sin_port = htons(port);  
   struct hostent *hostinfo = gethostbyname(hostname);  
   if (hostinfo == NULL) return false;  
   sock_addr.sin_addr = *(struct in_addr *) hostinfo->h_addr;  
   // �إ�socket //  
   try  
   {  
     m_Socket = socket(AF_INET,SOCK_STREAM,0);  
   }  
   catch(...)  
   {  
     m_Socket = INVALID_SOCKET;  
     return false;  
   }  
   if (m_Socket == INVALID_SOCKET) return false;  
   // �}�l�s�u //  
   try  
   {  
     if (connect(m_Socket,(struct sockaddr*)&sock_addr,sizeof(sock_addr)) >= 0) return true;  
   }  
   catch(...)  
   {  
   }  
   // ���B�i�H�[�J�@�ǿ��~�B�z... //  
   Close();  
   return false;  
}  

void Socket::Close(void)  
//�����G�����PServer���s�u  
{  
   if (!IsOpened()) return;  
   try  
   {  
     shutdown(m_Socket,SD_SEND);  
   }  
   catch(...)  
   {  
   }  
   try  
   {  
     closesocket(m_Socket);  
   }  
   catch(...)  
   {  
   }  
   m_Socket = INVALID_SOCKET;  
} 

bool Socket::Listen(int port)  
//�����G��ť�Y��Port  
//��J�Gport = ��ťPort  
//�Ǧ^�G���ѶǦ^false  
{  
    Close();  
    if (!Initialize()) return false;  
    struct sockaddr_in sock_addr;  
    sock_addr.sin_family = AF_INET;  
    sock_addr.sin_addr.s_addr = INADDR_ANY;  
    sock_addr.sin_port = htons(port);  
    // �إ�socket //  
    try  
    {  
        m_Socket = socket(AF_INET,SOCK_STREAM,0);  
    }  
    catch(...)  
    {  
        m_Socket = INVALID_SOCKET;  
        return false;  
    }  
    if (m_Socket == INVALID_SOCKET) return false;  
    // Bind socket //  
    int on = 1;  
    setsockopt(m_Socket,SOL_SOCKET,SO_REUSEADDR,(char*)&on,sizeof(on));  
    int rc;  
    try  
    {  
        rc = bind(m_Socket,(struct sockaddr*)&sock_addr,sizeof(sock_addr));  
    }  
    catch(...)  
    {  
        rc = SOCKET_ERROR;  
    }  
    if (rc == SOCKET_ERROR)   
    {  
        Close();  
        return false;  
    }  
    // Listen socket //  
    try  
    {  
        rc = listen(m_Socket,SOMAXCONN);  
    }  
    catch(...)  
    {  
        rc = SOCKET_ERROR;  
    }  
    if (rc == SOCKET_ERROR)   
    {  
        Close();  
        return false;  
   }  
    return true;  
} 

bool Socket::Accept(SOCKET &socket)  
//�����G���ݱ����s�u  
//��X�G�s�usocket  
//�Ǧ^�G���ѶǦ^false  
{  
    socket = INVALID_SOCKET;  
    if (!IsOpened()) return false;  
    struct sockaddr_in from;  

    int fromlen = sizeof(from);  
	

    try  
    {  
        socket = accept(m_Socket,(struct sockaddr*)&from,&fromlen);  
    }  
    catch(...)  
    {  
        socket = INVALID_SOCKET;  
        return false;  
    }  
    return true;  
}  

void Socket::SetSocket(SOCKET socket)  
//�����G�]�w�s�u��socket  
//��J�Gsocket = �s�u��socket  
{  
   Close();  
   m_Socket = socket;  
} 

/* 
* �ѩ�read��ƥ��������観�e�Ӹ�Ʈɤ~�|��^, �]���b�I�sread�e�̦n�������O�_����ƶi��,  
* �H�K�i��timeout�B�z. �o�̧ڥu�H"��"��timeout�����, �ݭn��ӷL���ɶ�, �Цۦ�ק�. 
*/  
bool Socket::WaitInputData(int seconds)  
//�����G���ݹ��e�Ӹ��  
//��J�Gseconds = ���ݬ��  
//�Ǧ^�G�S����ƶǦ^false  
{  
    if (!IsOpened())   
    {  
        ("\t[WaitInputData] Socket Not Open Yet!\n");  
        return false;  
    }  
    // �]�wdescriptor sets //  
    fd_set socket_set;  
    FD_ZERO(&socket_set);  
    FD_SET((unsigned int)m_Socket,&socket_set);  
    // �]�wtimeout�ɶ� //  
    struct timeval timeout;  
    timeout.tv_sec = seconds;  
    timeout.tv_usec = 0;  
    // �����O�_����� //  
    try  
    {  
       if (select(FD_SETSIZE,&socket_set,NULL,NULL,&timeout) <= 0)   
        {  
            printf("\t[WaitInputData] Timeout!\n");  
            return false;  
        }  
    }  
    catch(...)  
    {  
        printf("\t[WaitInputData] Exception!\n");  
        return false;  
    }  
    return true;  
}  

bool Socket::Read(void* data, long len, long &ret_len)  
//�����GŪ�����  
//��J�Gdata, len = ��ƽw�İϻP�j�p  
//��X�Gdata = Ū�������, ret_len = ���Ū������Ƥj�p�A0����w�_�u  
//�Ǧ^�G���ѶǦ^false  
//�Ƶ��G����Ʒ|�@�����즳Ū����Ʃε����s�u�ɤ~�Ǧ^  
{  
    ret_len = 0;  
    if (!IsOpened()) return true;  
    try  
    {  
        ret_len = recv(m_Socket,(char*)data,len,0);  
    }  
    catch(...)  
    {  
        ret_len = SOCKET_ERROR;  
    }  
    if (ret_len < 0)  
    {  
        ret_len = 0;  
        return false;  
    }  
    return true;  
}

bool Socket::Write(const void* data, long len)  
//�����G�e�X���  
//��J�Gdata, len = ��ƽw�İϻP�j�p  
//�Ǧ^�G���ѶǦ^false  
{  
    if (!IsOpened()) return false;  
    if (len <= 0) return true;  
    int write_len;  
    try  
    {  
        write_len = send(m_Socket,(const char*)data,len,0);  
    }  
    catch(...)  
    {  
        write_len = SOCKET_ERROR;  
    }  
    if (write_len != len) return false;  
    return true;  
}  
  
bool Socket::GetHostIP(const char* hostname, int &ip1, int &ip2, int &ip3, int &ip4)  
//�����G���o���whost��ip  
//��J�Ghostname = host��}  
//��X�Gip1-4 = ip��}  
//�Ǧ^�G���ѶǦ^false  
{  
    if (IsLocalHost(hostname))  
    {  
        // �����X��ڪ�hostname //  
        struct hostent *hostinfo = gethostbyname("localhost");  
        if (hostinfo == NULL) return false;  
        hostname = hostinfo->h_name;  
    }  
    struct hostent* hostinfo = gethostbyname(hostname);  
    if (hostinfo == NULL) return false;  
    char* addr = hostinfo->h_addr_list[0];  
    ip1 = (unsigned char) addr[0];  
    ip2 = (unsigned char) addr[1];  
    ip3 = (unsigned char) addr[2];  
    ip4 = (unsigned char) addr[3];  
    return true;  
}  
 
/* 
* Nagle Algorithm���Բӻ���, �аѦ�MSDN "Nagle Algorithm"�@��, �o�Ӻt��k�D�n�O�קK�L 
* �h�s�����e�X���, �N��������A�@���e�X. ���D���s�Ĳv�B�D�@���ʸ�ưe�X���q�T�{�� 
* �Ө� (�ҦpTTY, telnet��), �o�Ӻt��k�i�H�j�q���C��������ƶǿ�q. ���Y�O�w�]�p�n�@�� 
* �ʫʥ]���q�T�n��Ө�, �o�Ӻt��k�Ϧӷ|�Y���v�T�Ĳv.  
* �Ӱ���Nagle Algorithm, �o�]�O����ƪ��D�n�ت�. 
* ��h�i�H�Ѧ� :  
*/  
bool Socket::SetNoDelay(void)  
//�����G�]�w������ǰe (����Nagle Algorithm)  
//�Ǧ^�G�]�w���ѶǦ^false  
{  
    if (!IsOpened()) return false;  
    int on = 1;  
    if (setsockopt(m_Socket,IPPROTO_TCP,TCP_NODELAY,(char*)&on,sizeof(on)) != 0) return false;  
    return true;  
}
/*
void Socket::thread( void )
{  
	
	char buf[128] = {0};
	char buf_[128]= {0};
	long test;

	while(!IsOpened())
	{
		Open("192.168.100.2",3742);
	}
	
	while(true)
	{

		int i = 0 ;
		RemoteCommandStruct cmd;

		cmd.comand_test_item.vendor = ATHEROS ; 
		cmd.comand_test_item.module = WCBN4500AB ;
		cmd.comand_test_item.band = SINGLE_BAND ;
		cmd.comand_test_item.chain = CHAIN_1T_1R ;


		cmd.command_header.packet_type = PACKET_TYPE_TEST_ITEM ;

		Write((char*)&cmd, sizeof(cmd) );
		Sleep(5000);

		if(i == 3)
		{
			break;
		}
	}
}
	*/
