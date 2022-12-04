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
	std::string text = "";
};

struct REQUEST
{
	int user_id;
	int type;
	std::string text;
	int chat_id;
};

struct USER
{
	int id = -1;
};

struct MESSAGE {
	int user_id;
	int chat_id;
	std::string text;
};

class DB
{
private:
	static MESSAGE JSON_to_MESSAGE(std::string serialized_message) {
		MESSAGE request = MESSAGE();
		nlohmann::json json = nlohmann::json::parse(serialized_message);
		request.user_id = json["user_id"].get<int>();
		request.text = json["text"].get<std::string>();
		request.chat_id = json["chat_id"].get<int>();
		return request;
	}
	static std::string MESSAGE_to_JSON(MESSAGE message) {
		nlohmann::json j{};
		j["user_id"] = message.user_id;
		j["chat_id"] = message.chat_id;
		j["text"] = message.text;
		std::cout << j;
		return j.dump();
	}
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
	static void add_new_user(int user_id, std::string passwd, std::string name);
	static void add_user_to_chat(int chat_id, int user_id);
	static void add_message(int user_id, int chat_id, std::string text) 
	{
		MESSAGE message = MESSAGE{ user_id, chat_id, text };
		std::string record = DB::MESSAGE_to_JSON(message);
		/*
		* тут нужно открыть файл и записать строку record
		*/
	};
	static std::list<MESSAGE> get_all_messages(int chat_id) 
	{
		/*
		* прочесть файл построчно и к каждой применить DB::JSON_to_MESSAGE
		*/
	};
	static std::string get_user_passwd(int user_id);
};

class ChatController
{
public:
	static boolean have_user_assess_to_chat(int user_id, int chat_id);
	static boolean is_chat_exist(int chat_id);

	static RESPONSE send_message(int user_id, int chat_id, std::string text ) {
		/*
		принимает запрос пользователя и обрабатывает его
		может отправит, а может нет
		*/
		RESPONSE response = RESPONSE();
		if (not ChatController::is_chat_exist(chat_id)) {
			response.text = "chat not exist";
			response.success = false;
			return response;
		}
		if (not ChatController::have_user_assess_to_chat(user_id, chat_id)) {
				response.text = "you have not assess to this chat";
				response.success = false;
				return response;
		}
		DB::add_message(user_id, chat_id, text);
		return RESPONSE();
	}
};

class AuthoriseController
{
public:
	static RESPONSE login(std::string data);
	static RESPONSE logout(std::string data);
	static USER get_user_by_connection_index(int index) {
		USER user = USER();
		return user;
	};
};

RESPONSE request_handler(REQUEST message)
{
	RESPONSE response;
	switch (message.type){
	case 1:
	// send message
		std::cout << "Была запрошена команда 1 - отправка сообщения" << std::endl;
		ChatController::send_message(message.user_id, message.chat_id, message.text);
		break;
	default:
		response = RESPONSE();
		response.success = false;
		response.text = "не понял команду";
	}
	return response;
}

std::string RESPONSE_to_JSON(RESPONSE response) {
	nlohmann::json j{};
	j["success"] = response.success;
	j["text"] = response.text;
	std::cout << j;
	return j.dump();
}

REQUEST JSON_to_REQUEST(int connection_index, char* serialized_request) {
	REQUEST request = REQUEST();
	nlohmann::json json = nlohmann::json::parse(serialized_request);
	request.user_id = AuthoriseController::get_user_by_connection_index(connection_index).id;
	request.type = json["type"].get<int>();
	request.text = json["text"].get<std::string>();
	request.chat_id = json["chat_id"].get<int>();
	return request;
}

void send_message(int index, RESPONSE response)
{
	std::string text = RESPONSE_to_JSON(response);
	int text_size = text.length();
	send(Connections[index], (char *)&text_size, sizeof(int), NULL);
	send(Connections[index], text.c_str(), text_size, NULL);
}

void start_listen(int index)
{
	int msg_size;
	while (true)
	{
		recv(Connections[index], (char *)&msg_size, sizeof(int), NULL);
		char * serialized_request = new char[msg_size + 1];
		serialized_request[msg_size] = '\0';
		recv(Connections[index], serialized_request, msg_size, NULL);

		std::cout << "принято от клиента: " << serialized_request << std::endl;
				
		REQUEST request = JSON_to_REQUEST(index, serialized_request);
		RESPONSE response = request_handler(request);

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
			CreateThread(NULL, NULL, (LPTHREAD_START_ROUTINE)start_listen, (LPVOID)(i), NULL, NULL);
		}
	}

	system("pause");
	return 0;
}