#pragma comment(lib, "ws2_32.lib")
#include <winsock2.h>
#include <iostream>
#include <string>
#include <list>
#include <nlohmann/json.hpp>
#pragma warning(disable : 4996)

SOCKET Connections[100];
int Counter = 0;

struct  RESPONSE
{
	bool success = true;
	std::string data = "";
};

struct REQUEST
{
	int type = 0;
	std::string data = "";
};

class DB
{
public:
	static std::list<int> get_user_chats_id(int user_id)
	{
		/*
		возвращает все id чатов, к которым есть доступ данного пользователя

		использует файл user_to_chat.txt
		*/
	}
	static std::list<int> get_chat_users_id(int chat_id)
	{
		/*
		возвращает все id пользователей, у которых есть достум к чату

		использует файл chat_[id чата].txt
		*/
	}
	static void add_new_user(int user_id, std::string passwd, std::string name) {}
	static void add_user_to_chat(int chat_id, int user_id) {}
	static void add_message(int chat_id, int user_id, std::string message) {}
	static std::string get_all_messages(int chat_id) {}
	static std::string get_user_passwd(int user_id) {}
};

class ChatController
{
public:
	static RESPONSE send_message_command_handler(REQUEST command) {
		/*
		принимает сообщение пользователя и обрабатывает его
		*/
		std::string data = command.data;

	}
};

class AuthoriseController
{
public:
	static RESPONSE login(std::string data);
	static RESPONSE logout(std::string data);
};

RESPONSE handler(REQUEST message)
{
	RESPONSE answer = RESPONSE();
	switch (message.type){
	case 1:
		answer.data = "понян обработал 1";
		std::cout << "Была запрошена команда 1" << std::endl;
		break;
	case 3:
		answer.data = "your command is 3, your data is: " + message.data;
		std::cout << "Была запрошена команда 3" << std::endl;
		break;
	default:
		answer.success = false;
		answer.data = "не понял команду";
	}
	return answer;
}

std::string RESPONSE_to_JSON(RESPONSE response) {
	nlohmann::json j{};
	j["success"] = response.success;
	j["data"] = response.data;
	std::cout << j;
	return j.dump();
}

void send_message(int index, RESPONSE response)
{
	std::string text = RESPONSE_to_JSON(response);
	text = response.data;
	int text_size = text.length();
	send(Connections[index], (char *)&text_size, sizeof(int), NULL);
	send(Connections[index], text.c_str(), text_size, NULL);
}

void Start_listen(int index)
{
	int msg_size;
	while (true)
	{
		recv(Connections[index], (char *)&msg_size, sizeof(int), NULL);
		char * serialized_request = new char[msg_size + 1];

		serialized_request[msg_size] = '\0';
		recv(Connections[index], serialized_request, msg_size, NULL);

		std::cout << "принято от клиента: " << serialized_request << std::endl;

		REQUEST request = REQUEST();
		nlohmann::json json = nlohmann::json::parse(serialized_request);
		request.type = json["type"].get<int>();
		request.data = json["data"].get<std::string>();

		RESPONSE response = handler(request);
		send_message(index, response);

		delete[] serialized_request;
	}
}

int main(int argc, char *argv[])
{
	// WSAStartup
	WSAData wsaData;
	setlocale(LC_ALL, "Russian");
	WORD DLLVersion = MAKEWORD(2, 1);
	if (WSAStartup(DLLVersion, &wsaData) != 0)
	{
		std::cout << "Error" << std::endl;
		exit(1);
	}

	SOCKADDR_IN addr;
	int sizeofaddr = sizeof(addr);
	addr.sin_addr.s_addr = inet_addr("127.0.0.1");
	addr.sin_port = htons(1111);
	addr.sin_family = AF_INET;

	SOCKET sListen = socket(AF_INET, SOCK_STREAM, NULL);
	bind(sListen, (SOCKADDR *)&addr, sizeof(addr));
	listen(sListen, SOMAXCONN);

	SOCKET newConnection;
	for (int i = 0; i < 100; i++)
	{
		newConnection = accept(sListen, (SOCKADDR *)&addr, &sizeofaddr);

		if (newConnection == 0)
		{
			std::cout << "Error #2\n";
		}
		else
		{
			std::cout << "Client Connected!\n";
			Connections[i] = newConnection;
			Counter++;
			CreateThread(NULL, NULL, (LPTHREAD_START_ROUTINE)Start_listen, (LPVOID)(i), NULL, NULL);
		}
	}

	system("pause");
	return 0;
}