#include "NetworkServer.h"

int main()
{
	NetworkServer server;

	// 소켓 초기화
	server.InitSocket();

	// 소켓과 서버 주소 연결 후 등록
	server.BindAndListen();

	// 서버 시작
	server.Run();

	cout << "종료: quit" << endl;
	while (true)
	{
		string input;
		getline(cin, input);

		if (input == "quit")
			break;
	}

	server.End();

	return 0;
}