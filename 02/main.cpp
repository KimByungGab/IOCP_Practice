#include "NetworkCore.h"

int main()
{
	NetworkCore networkCore;

	// 소켓 초기화
	networkCore.InitSocket();

	// 소켓과 서버 주소 연결 후 등록
	networkCore.BindAndListen();

	// 서버 시작
	networkCore.StartServer();

	cout << "종료: quit" << endl;
	while (true)
	{
		string input;
		getline(cin, input);

		if (input == "quit")
			break;
	}

	networkCore.DestoryThread();

	return 0;
}
