#include <raknet/MessageIdentifiers.h>
#include <vector>

enum CustomMessages
{
	CHAT_MESSAGE = ID_USER_PACKET_ENUM + 1,
	USERNAME_FOR_GUID,
	PLAYER_COORD,
	MOUSE_COORD,
	LOGIN_FAILED,
	LOGIN_ACCEPTED,
	USERNAME,
	PLAYER_COORD_UPDATE,
	PONG,
	PING,
	PLAYER_INPUT,
	BALL_UPDATE,
	CUBE_INFO,
	READ_BULK,
	WRITE_BULK,
	PLAYER_INFO,
	FLOOR,
	PLAYER_SLOT,
	PLAYER_LOOK_DIR,
	PLAYER_STATE,
	SERVER_DELTA,
	UNKNOWN,
};

enum Type
{
	INT_TYPE,
	STRING_TYPE,
	FLOAT_TYPE,
};

struct MessageType
{
	MessageType(Type type, CustomMessages MessageID) { this->type = type; this->MessageID = MessageID; }
	MessageType() {};
	Type type;
	CustomMessages MessageID;
};

template <typename T>
struct Var
{
	Type type;
	std::vector<T*> Values;
	CustomMessages MessageID;
};