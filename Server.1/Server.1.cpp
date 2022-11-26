#pragma comment(lib, "ws2_32.lib")
#include <winsock2.h>
#include <iostream>
#include <string>
#include <list>
#pragma warning(disable: 4996)

SOCKET Connections[100];
int Counter = 0;

struct MESSAGE
{
	bool success = true;
	std::string type;
	std::string data = "";
};

MESSAGE handler(MESSAGE message) {
	MESSAGE answer = MESSAGE();
	if (message.type == "01") {
		answer.data = "понян обработал 1";
		std::cout << "Была запрошена команда 1)" << std::endl;
	}
	else if (message.type == "02") {
		answer.data = "понян обработал 2";
		std::cout << "Была запрошена команда 2)" << std::endl;
	}
	else if (message.type == "03") {
		answer.data = "понян обработал 3, а еще ты ввел данные: "+message.data;
		std::cout << "Была запрошена команда 3)" << std::endl;
	}
	else if (message.type == "04") {
		answer.data = "понян обработал 4";
		std::cout << "Была запрошена команда 4)" << std::endl;
	}
	else {
		answer.success = false;
		answer.data = "не понял";
	}
	return answer;
}

void send_message(int index, MESSAGE message) {
	std::string text = message.data;
	int text_size = text.length();
	send(Connections[index], (char*)&text_size, sizeof(int), NULL);
	send(Connections[index], text.c_str(), text_size, NULL);
}

void Start_listen(int index) {
	int msg_size;
	while (true) {
		recv(Connections[index], (char*)&msg_size, sizeof(int), NULL);
		char* msg = new char[msg_size + 1];
		
		msg[msg_size] = '\0';
		recv(Connections[index], msg, msg_size, NULL);

		// первые 2 символа будут являться номером команды. все что позже - данные
		MESSAGE message = MESSAGE();
		std::string msg_str = msg;
		message.type = msg_str.substr(0, 2);
		message.data = msg_str.substr(2);

		MESSAGE answer = handler(message);
		send_message(index, answer);

		delete[] msg;
	}
}

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

class AuthoriseController
{
public:
	static MESSAGE login(std::string data);
	static MESSAGE logout(std::string data);
};
int main(int argc, char* argv[]) {
	//WSAStartup
	WSAData wsaData;
	setlocale(LC_ALL, "Russian");
	WORD DLLVersion = MAKEWORD(2, 1);
	if (WSAStartup(DLLVersion, &wsaData) != 0) {
		std::cout << "Error" << std::endl;
		exit(1);
	}

	SOCKADDR_IN addr;
	int sizeofaddr = sizeof(addr);
	addr.sin_addr.s_addr = inet_addr("127.0.0.1");
	addr.sin_port = htons(1111);
	addr.sin_family = AF_INET;

	SOCKET sListen = socket(AF_INET, SOCK_STREAM, NULL);
	bind(sListen, (SOCKADDR*)&addr, sizeof(addr));
	listen(sListen, SOMAXCONN);

	SOCKET newConnection;
	for (int i = 0; i < 100; i++) {
		newConnection = accept(sListen, (SOCKADDR*)&addr, &sizeofaddr);

		if (newConnection == 0) {
			std::cout << "Error #2\n";
		}
		else {
			std::cout << "Client Connected!\n";
			Connections[i] = newConnection;
			Counter++;
			CreateThread(NULL, NULL, (LPTHREAD_START_ROUTINE)Start_listen, (LPVOID)(i), NULL, NULL);
		}
	}


	system("pause");
	return 0;
}