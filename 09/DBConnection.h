#pragma once

#include <iostream>
#include <vector>
#include <map>
#include <unordered_map>
#include <string>

#include <mysql.h>
#pragma comment(lib, "libmySQL.lib")

using namespace std;

class DBConnection
{
public:
	// 싱글톤을 만들기 위하여 복사생성자, 이동생성자 불가능
	DBConnection(const DBConnection&) = delete;
	DBConnection& operator=(const DBConnection&) = delete;

	static DBConnection* GetInstance();		// 싱글톤 클래스화하기 위하여 static화
	void SetTable(const char* tableName) { mTableName = const_cast<char*>(tableName); }
	int InsertData(map<string, string> data);
	vector<map<string, string>> SelectData(vector<string> columns, string whereStr = "", string orderStr = "", string limitStr = "");
	int UpdateData(unordered_map<string, string> updateData, string whereStr = "");
	int DeleteData(string whereStr = "");
	int Query(string query);
	~DBConnection();
	
private:
	DBConnection();	// 싱글톤 클래스를 위하여 가져가서 사용하지 말게끔 private으로 숨김
	MYSQL mMysql;				// mysql 구조체
	MYSQL* mConn = nullptr;		// DB 커넥션 객체
	static DBConnection* mInstance;		// 싱글턴 인스턴스

	string mTableName = "";		// 테이블 이름

	// DB 정보
	// 나중에 .env 디렉토리에 파일을 하나 만들어서 하면 관리하면 될 것 같다.
	// 사실 스프링에서는 properties에 관리하고 Node.js에서는 .env로 환경변수를 관리했었는데
	// C++에서는 따로 이렇다 하는 건 없었던 것 같다.
	const char* mServer = "localhost";
	const char* mUser = "test";
	const char* mPW = "1111";
	const char* mDB = "cpp_test";
	const int mPort = 3306;
};