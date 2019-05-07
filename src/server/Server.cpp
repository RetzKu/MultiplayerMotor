#include "Server.h"

using namespace RakNet;
using namespace std;

#define PORT 60000

Server::Server(string IP, int Port)
{
	string title = "Raknet-Server";
	SetConsoleTitle(title.c_str());
	this->IP = IP;
	this->Port = Port;
	Connections = new UserDatabase();
	ServerStart();

	InitBulletWorld();
}

void Server::InitBulletWorld()
{
	collisionConfiguration =	new btDefaultCollisionConfiguration();
	dispatcher =				new btCollisionDispatcher(collisionConfiguration);
	overlappingPairCache =		new btDbvtBroadphase();
	solver =					new  btSequentialImpulseConstraintSolver;
	dynamicsWorld =				new btDiscreteDynamicsWorld(dispatcher, overlappingPairCache, solver, collisionConfiguration);

	dynamicsWorld->setGravity(btVector3(physics_gravity.x,physics_gravity.y,physics_gravity.z));

	AddCube(ObjectType::Floor, { 0.0f,0.0f,0.0f }, { 0.0f,0.0f,0.0f });
	AddCube(ObjectType::Floor, { -20.0f,0.0f,0.0f }, { 0.0f,0.0f,0.0f });
	AddCube(ObjectType::Floor, { 0.0f,0.0f,-20.0f }, { 0.0f,0.0f,0.0f });
	AddCube(ObjectType::Floor, { 20.0f,0.0f,0.0f }, { 0.0f,0.0f,0.0f });
	AddCube(ObjectType::Floor, { 0.0f,0.0f,20.0f }, { 0.0f,0.0f,0.0f });

	for (int x = 0; x < 3; x++) //tehd��n cubet mappiin
	{
		for (int y = 0; y < 3; y++) //tehd��n cubet mappiin
		{
			for (int z = 0; z < 3; z++) //tehd��n cubet mappiin
			{
				AddCube(ObjectType::FreeSimulation, { (float)x ,(float)y + 50.0f,(float)z }, { 0.0f,0.0f,0.0f });
			}
		}
	}
}

void Server::RemoveSmallCube(int id)
{
	for (int i = 0; i < cubes.size(); i++)
	{
		if (smallCubesActive[i].id == id)
		{
			smallCubesActive[i].id = (int)smallCubesActive.size() + 1;
			smallCubesActive[i].rb->clearForces();
			smallCubesActive[i].rb->forceActivationState(WANTS_DEACTIVATION);
			smallCubesInactive.push_back(smallCubesActive[i]);
			smallCubesActive.erase(smallCubesActive.begin() + i);

		}
	}
}

void Server::AddCube(ObjectType type, vec3 pos, vec3 rot)
{
	if (type == ObjectType::Player)
	{
		btTransform t;
		t.setIdentity();
		t.setOrigin({ pos.x,pos.y,pos.z });

		btVector3 defaultIntertia(0, 0, 0);
		btBoxShape* box = new btBoxShape({ player_scale.x, player_scale.y, player_scale.z });
		box->calculateLocalInertia(free_mass, defaultIntertia);

		btMotionState* boxmotion = new btDefaultMotionState(t);
		btRigidBody::btRigidBodyConstructionInfo boxInfo(free_mass, boxmotion, box, defaultIntertia);
		btRigidBody* newBox = new btRigidBody(boxInfo);

		dynamicsWorld->addRigidBody(newBox);
		Players.push_back(Cube(Players.size()+Floors.size()+smallCubesActive.size() + 1, type, newBox));
		Score.push_back(0);
	}
	if (type == ObjectType::Floor)
	{
		btTransform t;
		t.setIdentity();
		t.setOrigin({ pos.x,pos.y,pos.z });

		btVector3 defaultIntertia(0, 0, 0);
		btBoxShape* box = new btBoxShape({ floor_scale.x, floor_scale.y, floor_scale.z });
		box->calculateLocalInertia(0.0f, defaultIntertia);

		btMotionState* boxmotion = new btDefaultMotionState(t);
		btRigidBody::btRigidBodyConstructionInfo boxInfo(0.0f, boxmotion, box, defaultIntertia);
		btRigidBody* newBox = new btRigidBody(boxInfo);


		newBox->setFriction(floor_friction);
		dynamicsWorld->addRigidBody(newBox);
		Floors.push_back(Cube(Players.size()+Floors.size()+smallCubesActive.size() + 1, type, newBox));
		
	}
	if (type == ObjectType::FreeSimulation)
	{
		if (smallCubesInactive.size() == 0) //Create new rigidbody for new cube
		{
			btTransform t;
			t.setIdentity();
			t.setOrigin({pos.x,pos.y,pos.z});

			btVector3 defaultIntertia(0, 0, 0);
			btBoxShape* box = new btBoxShape({free_scale.x, free_scale.y, free_scale.z});
			box->calculateLocalInertia(free_mass, defaultIntertia);

			btMotionState* boxmotion = new btDefaultMotionState(t);
			btRigidBody::btRigidBodyConstructionInfo boxInfo(free_mass, boxmotion, box, defaultIntertia);
			btRigidBody* newBox = new btRigidBody(boxInfo);

			//newBox->setFriction(0.1f);
			dynamicsWorld->addRigidBody(newBox);

			smallCubesActive.push_back(Cube(Players.size()+Floors.size()+smallCubesActive.size() + 1, type, newBox));
		}
		else
		{
			smallCubesActive.push_back(smallCubesInactive[0]); //copy from first inactive to active ones
			smallCubesActive[smallCubesActive.size() - 1].rb->forceActivationState(ACTIVE_TAG);
			smallCubesInactive.erase(smallCubesInactive.begin()); 
		}
	}
}

void Server::AddPlayerCube(std::string name)
{
	btVector3 defaultIntertia(0, 0, 0);

	btBoxShape* box = new btBoxShape({ 2,2,2 });
	box->calculateLocalInertia(1.0f, defaultIntertia);

	btTransform t;
	t.setIdentity();
	t.setOrigin({0,5,0});

	btMotionState* boxmotion = new btDefaultMotionState(t);
	btRigidBody::btRigidBodyConstructionInfo boxInfo(20.0f, boxmotion, box, defaultIntertia);
	btRigidBody* newBox = new btRigidBody(boxInfo);
	newBox->setFriction(0.1f);
	//players.push_back(newBox);
	slots.push_back(name);
	dynamicsWorld->addRigidBody(newBox);
}

void Server::RemovePlayerCube(std::string name)
{
	for (int i = 0; i < Players.size(); i++)
	{
		if (slots[i] == name)
		{
			slots.erase(slots.begin() + i);
			Players.erase(Players.begin() + i);
		}
	}
}

void Server::ServerStart()
{
	Peer = RakPeerInterface::GetInstance();

	SD = new SocketDescriptor(PORT, 0);
	/*Creating Instance for server and starting up basic setup*/
	Peer->Startup(MaxConnections, SD, 1);
	Peer->SetMaximumIncomingConnections(MaxConnections);
	CONSOLE("Starting server at port " << Port);

	Delta120 = chrono::system_clock::now();
	TimeInterval = (int)((1.0 / 30) * 1000);

	running = true;
}

void Server::ServerStop()
{
	running = false;
	RakPeerInterface::DestroyInstance(Peer);
	delete Connections;
}

void Server::ServerUpdate()
{
	auto Delta = chrono::duration_cast<chrono::milliseconds>(chrono::system_clock::now() - Delta120);
	/*Loads packet from peer*/
	if ((float)Delta.count() > TimeInterval)
	{
		Delta120 += chrono::milliseconds((int)TimeInterval);

		//SendCubeInfo();
		WriteBulk();
		SendPlayerInfo();
		dynamicsWorld->stepSimulation(1.0 / 30.0,8);

		for (Packet = Peer->Receive(); Packet; Packet = Peer->Receive())
		{
			CheckPacket(*Packet);
			Peer->DeallocatePacket(Packet);
		}
	}
}

void Server::CheckPacket(const RakNet::Packet& P)
{
	switch (P.data[0])
	{
	case ID_NEW_INCOMING_CONNECTION:
		Connections->ConnectUser(Packet);
		CONSOLE(Packet->guid.ToString() << " Connected to server");
		break;
	case USERNAME_FOR_GUID:
		Result = Connections->RegisterGuid(Packet);
		SendResponse(Packet->systemAddress, LOGIN_ACCEPTED);
		AddCube(ObjectType::Player, { 3,3,0 }, { 0,0,0 }); //We create cube for player
		slots.push_back(Packet->guid.ToString()); //we save its guid.
		CONSOLE(Packet->guid.ToString() << " gave an username " << Result);
		break;
	case ID_CONNECTION_LOST:
		Connections->RemoveUser(Packet);
		RemovePlayerCube(Packet->guid.ToString());
		CONSOLE(Packet->guid.ToString() << " Connection lost");
		break;
	case PLAYER_COORD:
		ReadPlayerCoord(Packet);
		break;
	case PLAYER_INPUT:
		ReadPlayerInput(Packet);
		break;
	case PLAYER_STATE:
		ReadPlayerState(Packet);
		break;
	case PLAYER_LOOK_DIR:
		ReadPlayerLookDir(Packet);
		break;
	}
}

void Server::BroadcastVar(CustomMessages Var, RakNet::Packet Packet)
{
	RakNet::BitStream bs(Packet.data,Packet.length,false);
	Peer->Send(&bs, MEDIUM_PRIORITY, RELIABLE_ORDERED, 0, Packet.systemAddress, true, 0);
}

void Server::ReadPlayerCoord(RakNet::Packet* packet)
{
	RakNet::BitStream bs(packet->data,packet->length,false);
	bs.IgnoreBytes(sizeof(RakNet::MessageID));

	int x;
	int y;

	bs.Read(x);
	bs.Read(y);
	std::string user = Connections->FindUsername(packet->guid.ToString());
	if (user != "NONE")
	{
		CONSOLE("Received " << x << ", " << y << " From: " << user);
	}
}

void Server::ReadPlayerInput(RakNet::Packet* packet)
{
	RakNet::BitStream bs(packet->data, packet->length, false);
	bs.IgnoreBytes(sizeof(RakNet::MessageID));

	int w = 0;
	int a = 0;
	int s = 0;
	int d = 0;
	bs.Read(w);
	bs.Read(a);
	bs.Read(s);
	bs.Read(d);
	//if (players.size() != 0)
	//{
	//	//std::string target = Connections->FindUsername(packet->guid.ToString());
	//	for (int i = 0; i < players.size(); i++)
	//	{
	//		if (slots[i] == packet->guid.ToString())
	//		{
	//			players[i]->activate(true);
	//			players[i]->applyCentralForce({ (float)((a-d)*20),0,(float)((w-s)*20) });
	//		}
	//	}
	//}
	//TODO(mika) handlaa inputit;
	//printf("w: %i, a: %i, s: %i, d: %i, from: %s\n",w,a,s,d,packet->guid.ToString());
}

void Server::SendResponse(RakNet::SystemAddress sys, CustomMessages responseID)
{
	RakNet::BitStream bs;
	bs.Write((MessageID)responseID);
	Peer->Send(&bs, MEDIUM_PRIORITY, RELIABLE_ORDERED, 0, sys, false, 0);
}

bool Server::AskForVariable(CustomMessages var, INT64 guid)
{
	RakNet::BitStream bs;
	RakNet::RakNetGUID rakguid;
	rakguid.FromString(to_string(guid).c_str());

	bs.Write((RakNet::MessageID)var);
	Peer->Send(&bs, MEDIUM_PRIORITY, RELIABLE_ORDERED, 0, rakguid, false, 0);
	return true;
}

bool Server::AskForVariable(CustomMessages var, string username)
{
	string guid = Connections->FindGuid(username);
	if (guid == "NONE")
	{
		return false;
	}
	INT64 guidint = stoll(guid);
	return AskForVariable(var, guidint);
}

void Server::RequestFromAll(CustomMessages Requested)
{
	RakNet::BitStream bs;
	bs.Write((RakNet::MessageID)Requested);
	Peer->Send(&bs, HIGH_PRIORITY, RELIABLE_ORDERED, 0, UNASSIGNED_SYSTEM_ADDRESS, true, 0);
}

void Server::SendSlotID(RakNet::SystemAddress addr, int id)
{
	RakNet::BitStream bs;
	bs.Write((MessageID)PLAYER_SLOT);
	bs.Write(id);
	Peer->Send(&bs, MEDIUM_PRIORITY, RELIABLE_ORDERED, 0, addr, false, 0);
}

void Server::SendCubeInfo()
{
	RakNet::BitStream bs;
	btTransform trans;
	bs.Write((RakNet::MessageID)CUBE_INFO);
	int size = cubes.size();
	bs.Write(size);
	for (int i = 0; i < cubes.size(); i++)
	{
		cubes[i]->getMotionState()->getWorldTransform(trans);
		bs.Write(trans.getOrigin());
		bs.Write(trans.getRotation());
	}
	Peer->Send(&bs, HIGH_PRIORITY, RELIABLE_ORDERED, 0, UNASSIGNED_SYSTEM_ADDRESS, true, 0);
}

void Server::ReadPlayerState(RakNet::Packet* packet)
{
	RakNet::BitStream bs(packet->data, packet->length, false);
	bs.IgnoreBytes(sizeof(RakNet::MessageID));

	vec3 lookDir;
	inputType input;
	bs.Read(lookDir);
	bs.Read(input);
	//printf("%i\n", input);
	UpdatePlayerCube(packet->guid.ToString(),input,lookDir);
	//extract input
	//extract lookdir
	//Handle input

}

void Server::UpdatePlayerCube(std::string guid, inputType input, vec3 lookDir)
{
	//W = 22, A = 0, S = 18, D = 3
	//Check Input
	for (int i = 0; i < Players.size(); i++)
	{
		if (slots[i] == guid)
		{
			Players[i].rb->activate(true);
			vec3 cross = cross_product(lookDir, { 0,1,0 });
			cross = normalized(cross);

			lookDir = lookDir * MovementSpeedMultiplier;
			cross = cross * MovementSpeedMultiplier;

			if ((input & (1 << 22)) != 0)
			{
				//printf("w\n");
				Players[i].rb->applyCentralForce({lookDir.x,0,lookDir.z});
			}
			if ((input & (1 << 18)) != 0)
			{
				//printf("s\n");
				Players[i].rb->applyCentralForce({-lookDir.x,0,-lookDir.z});
			}
			if ((input & (1 << 0)) != 0)
			{
				//printf("a\n");
				Players[i].rb->applyCentralForce({-cross.x,0,-cross.z});
			}
			if ((input & (1 << 3)) != 0)
			{
				//printf("d\n");
				Players[i].rb->applyCentralForce({cross.x,0,cross.z});
			}
			if ((input & (1 << 4)) != 0)
			{
				//pelaajan kohdalle. laatikko //forcellea katsomis suuntaan;
			}
		}
	}
}

void Server::WriteBulk()
{
	RakNet::BitStream bs;
	btTransform trans;

	bs.Write((RakNet::MessageID)READ_BULK);
	//adding cubeinfo data to packet;
	packetID++;
	if (packetID > 99999) packetID = 0;
	bs.Write(packetID);
	
	if (smallCubesActive.size() != 0 || Floors.size() != 0 || Players.size() != 0)
	{
		bs.Write((RakNet::MessageID)CUBE_INFO);
		
		int size = smallCubesActive.size() + Floors.size() + Players.size();
		bs.Write(size);

		for (int i = 0; i < Floors.size(); i++)
		{
			bs.Write(Floors[i].id);
			bs.Write(Floors[i].type);

			Floors[i].rb->getMotionState()->getWorldTransform(trans);
			bs.Write(trans.getOrigin());
			bs.Write(trans.getRotation());
			//bs.Write(Floors[i].rb->getLinearVelocity());
			//bs.Write(Floors[i].rb->getAngularVelocity());
		}
		for (int i = 0; i < smallCubesActive.size(); i++)
		{
			bs.Write(smallCubesActive[i].id);
			bs.Write(smallCubesActive[i].type);

			smallCubesActive[i].rb->getMotionState()->getWorldTransform(trans);
			bs.Write(trans.getOrigin());
			bs.Write(trans.getRotation());
			//bs.Write(smallCubesActive[i].rb->getLinearVelocity());
			//bs.Write(smallCubesActive[i].rb->getAngularVelocity());
		}
		for (int i = 0; i < Players.size(); i++)
		{
			bs.Write(Players[i].id);
			bs.Write(Players[i].type);

			Players[i].rb->getMotionState()->getWorldTransform(trans);
			bs.Write(trans.getOrigin());
			bs.Write(trans.getRotation());
		}
	}

	Peer->Send(&bs, HIGH_PRIORITY, RELIABLE_ORDERED, 0, UNASSIGNED_SYSTEM_ADDRESS, true, 0);
}

void Server::SendPlayerInfo()
{
	RakNet::BitStream bs;

	bs.Write((MessageID)PLAYER_INFO);
	int size = Players.size();
	bs.Write(size); //how many players we got
	string name;
	int score = 0;
	int id = 0;

	for (int i = 0; i < Players.size(); i++)
	{
		name = Connections->FindUsername(slots[i]);
		char name2[12];
		strcpy(name2, name.c_str());
		id = Players[i].id;
		bs.Write(name2,12);
		bs.Write((score+1));
		bs.Write(id);
		bs.Write((i+1)); //TODO_MIKA: tee pelaaja slotit toimiviksi;
	}
	if (size != 0)
	{
		Peer->Send(&bs, HIGH_PRIORITY, RELIABLE_ORDERED, 0, UNASSIGNED_SYSTEM_ADDRESS, true, 0);
	}
}

void Server::ReadPlayerLookDir(RakNet::Packet* packet)
{
	RakNet::BitStream bs(packet->data, packet->length, false);
	bs.IgnoreBytes(sizeof(RakNet::MessageID));

	for (int i = 0; i < slots.size(); i++)
	{
		if(slots[i] == packet->guid.ToString())
		{

		}
	}
}

void Server::SendSmallCubeInfo()
{
	RakNet::BitStream bs;
	btTransform trans;
	bs.Write((RakNet::MessageID)CUBE_INFO);
	int size = smallCubesActive.size();
	bs.Write(size);

	for (int i = 0; i < size; i++)
	{
		bs.Write(smallCubesActive[i].id);
		bs.Write(smallCubesActive[i].type);

		smallCubesActive[i].rb->getMotionState()->getWorldTransform(trans);
		bs.Write(trans.getOrigin());
		bs.Write(trans.getRotation());
	}

	//for (int i = 0; i < cubes.size(); i++)
	//{
	//	cubes[i]->getMotionState()->getWorldTransform(trans);
	//	bs.Write(trans.getOrigin());
	//	bs.Write(trans.getRotation());
	//}
	Peer->Send(&bs, HIGH_PRIORITY, RELIABLE_ORDERED, 0, UNASSIGNED_SYSTEM_ADDRESS, true, 0);


}
