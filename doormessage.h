#ifndef DOORMESSAGE
#define DOORMESSAGE


typedef unsigned char BYTE;
typedef unsigned short WORD;
typedef unsigned int DWORD;

enum MessageType
{
    SEARCH=0x1994,
    SETIP=0x1996,
    OPEN=0x1940,
};

#pragma pack(1)

typedef struct DoorMessage
{

}DoorMessage;

#pragma pack()

char* CreateDoorMessage(MessageType type,const char* controllerID=0x00000000 )
{
    char message[64];
    memset(message,);
}

#endif // DOORMESSAGE
