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
	// �̱����� ����� ���Ͽ� ���������, �̵������� �Ұ���
	DBConnection(const DBConnection&) = delete;
	DBConnection& operator=(const DBConnection&) = delete;

	static DBConnection* GetInstance();		// �̱��� Ŭ����ȭ�ϱ� ���Ͽ� staticȭ
	void SetTable(const char* tableName) { mTableName = const_cast<char*>(tableName); }
	int InsertData(map<string, string> data);
	vector<map<string, string>> SelectData(vector<string> columns, string whereStr = "", string orderStr = "", string limitStr = "");
	int UpdateData(unordered_map<string, string> updateData, string whereStr = "");
	int DeleteData(string whereStr = "");
	int Query(string query);
	~DBConnection();
	
private:
	DBConnection();	// �̱��� Ŭ������ ���Ͽ� �������� ������� ���Բ� private���� ����
	MYSQL mMysql;				// mysql ����ü
	MYSQL* mConn = nullptr;		// DB Ŀ�ؼ� ��ü
	static DBConnection* mInstance;		// �̱��� �ν��Ͻ�

	string mTableName = "";		// ���̺� �̸�

	// DB ����
	// ���߿� .env ���丮�� ������ �ϳ� ���� �ϸ� �����ϸ� �� �� ����.
	// ��� ������������ properties�� �����ϰ� Node.js������ .env�� ȯ�溯���� �����߾��µ�
	// C++������ ���� �̷��� �ϴ� �� ������ �� ����.
	const char* mServer = "localhost";
	const char* mUser = "test";
	const char* mPW = "1111";
	const char* mDB = "cpp_test";
	const int mPort = 3306;
};