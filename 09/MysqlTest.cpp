#include <iostream>

#include <mysql.h>
#pragma comment(lib, "libmySQL.lib")
#include <sql.h>
#include <sqlext.h>

#include "DBConnection.h"

using namespace std;

int main()
{
    /*
    * 예시 테이블 정보
    * 테이블명: test_table
    * 컬럼: ID(Bigint(20), Primary_key, NotNull, Auto_increment), name(varchar(45)), phone(varchar(45))
    */
    DBConnection* conn = DBConnection::GetInstance();

    // insert
    map<string, string> insertData;
    insertData.insert(make_pair("name", "testName3"));
    insertData.insert(make_pair("phone", "010-0000-0003"));

    conn->SetTable("test_table");
    conn->InsertData(insertData);

    // select
    vector<string> columns;
    columns.push_back("ID");
    columns.push_back("name");
    columns.push_back("phone");

    conn->SetTable("test_table");
    auto result = conn->SelectData(columns);

    for (int i = 0; i < result.size(); i++)
    {
        cout << result[i]["ID"] << "/" << result[i]["name"] << "/" << result[i]["phone"] << endl;
    }

    // update
    unordered_map<string, string> updateInfo;
    updateInfo.insert(make_pair("name", "testName1"));
    updateInfo.insert(make_pair("phone", "010-0000-0001"));
    
    conn->SetTable("test_table");
    conn->UpdateData(updateInfo, "ID = 1");

    // delete
    conn->SetTable("test_table");
    conn->DeleteData("ID = 3");

    return 0;
}