#include <windows.h>
#include <sql.h>
#include "DBConnection.h"
#include <iostream>

DBConnection::DBConnection()
{
    
}

DBConnection::~DBConnection()
{
    Release();
}

bool DBConnection::Init()
{

    // ODBC 환경 핸들 생성
    if (SQL_ERROR == SQLAllocHandle(SQL_HANDLE_ENV, SQL_NULL_HANDLE, &_h_env))
    {
        PrintErrorMessage(SQL_HANDLE_ENV, _h_env);
        return false;
    }

    // ODBC 버전 설정
    if(SQL_ERROR == SQLSetEnvAttr(_h_env, SQL_ATTR_ODBC_VERSION, (void*)SQL_OV_ODBC3, 0))
    {
        PrintErrorMessage(SQL_HANDLE_ENV, _h_env);
        return false;
    }

    // DB 연결 핸들 생성
    if(SQL_ERROR == SQLAllocHandle(SQL_HANDLE_DBC, _h_env, &_h_dbc))
    {
        PrintErrorMessage(SQL_HANDLE_DBC, _h_dbc);
        return false;
    }




    wchar_t inidata_uid[100] = {};
    wchar_t inidata_pwd[100] = {};
    wchar_t inidata_server[100] = {};
    wchar_t inidata_dsn[100] = {};
    wchar_t inidata_database[100] = {};

    if (GetFileAttributesW(L"./db.config.ini") == INVALID_FILE_ATTRIBUTES) 
    {
        std::cout << "db.config.ini not founded\n";
        return false;
    }

    //ini 파일에서 ID/PW 읽기
    GetPrivateProfileStringW(L"database", L"uid", L"", inidata_uid, 100, L"./db.config.ini");
    GetPrivateProfileStringW(L"database", L"pwd", L"", inidata_pwd, 100, L"./db.config.ini");
    GetPrivateProfileStringW(L"database", L"server", L"", inidata_server, 100, L"./db.config.ini");
    GetPrivateProfileStringW(L"database", L"dsn", L"", inidata_dsn, 100, L"./db.config.ini");
    GetPrivateProfileStringW(L"database", L"database", L"", inidata_database, 100, L"./db.config.ini");

	// 연결 문자열 생성
	SQLWCHAR connectionString[512] = {};
    swprintf_s(connectionString, 512,
        L"DRIVER={SQL Server};SERVER=%s;DATABASE=%s;UID=%s;PWD=%s;",
        inidata_server, inidata_database, inidata_uid, inidata_pwd);

    //ODBC DSN으로 연결
    SQLWCHAR connStr[512] = {};
    swprintf_s(connStr, 512, L"DSN=%s;UID=%s;PWD=%s;", inidata_dsn, inidata_uid, inidata_pwd);

    //if (SQL_ERROR == SQLDriverConnectW(_h_dbc, NULL, connectionString, SQL_NTS, NULL, 0, NULL, SQL_DRIVER_NOPROMPT))
    if (SQL_ERROR == SQLDriverConnectW(_h_dbc, NULL, connStr, SQL_NTS, NULL, 0, NULL, SQL_DRIVER_NOPROMPT))
    {
        PrintErrorMessage(SQL_HANDLE_DBC, _h_dbc);
        return false;
    }

	return true;
}

bool DBConnection::Release()
{
    // 핸들 해제
    SQLFreeHandle(SQL_HANDLE_STMT, _h_stmt);
    SQLDisconnect(_h_dbc);
    SQLFreeHandle(SQL_HANDLE_DBC, _h_dbc);
    SQLFreeHandle(SQL_HANDLE_ENV, _h_env);
    return true;
}

bool DBConnection::TestQuery()
{
    // 쿼리 실행을 위한 문장 핸들 생성
    if (SQL_ERROR == SQLAllocHandle(SQL_HANDLE_STMT, _h_dbc, &_h_stmt))
    {
        PrintErrorMessage(SQL_HANDLE_STMT, _h_stmt);
        return false;
    }

    // SQL문 실행
    SQLWCHAR sql[] = L"SELECT * FROM accountDB.dbo.account";
    //SQLWCHAR sql[] = L"SELECT * FROM GameDB.dbo.testtable; ";

    SQLRETURN ret = SQLExecDirectW(_h_stmt, sql, SQL_NTS);
    if (ret == SQL_ERROR)
    {
        PrintErrorMessage(SQL_HANDLE_STMT, _h_stmt);
        return false;
    }

    // 결과 가져오기
    if (ret == SQL_SUCCESS || ret == SQL_SUCCESS_WITH_INFO) {
        //SQLWCHAR name[50];
        //SQLINTEGER age;
        SQLBIGINT accuid;   //SQL_C_SBIGINT /sign    SQL_C_UBIGINT /unsign
        SQLVARCHAR id[20] = {};
        SQLVARCHAR pw[30] = {};

		//SQL                       ODBC CTYPE
        //CHAR(n) / VARCHAR(n)	    SQL_C_CHAR
		//NCHAR(n) / NVARCHAR(n)    SQL_C_WCHAR
        //TINYINT	                SQL_C_STINYINT / SQL_C_UTINYINT
		//SMALLINT	                SQL_C_SSHORT / SQL_C_USHORT
		//INT	                    SQL_C_SLONG / SQL_C_ULONG /SQL_C_LONG
		//BIGINT                    SQL_C_SBIGINT / SQL_C_UBIGINT
		//FLOAT (8바이트)	        SQL_C_DOUBLE
		//REAL (4바이트)	            SQL_C_FLOAT
		//DATETIME, SMALLDATETIME   SQL_C_TYPE_TIMESTAMP
        //UNIQUEIDENTIFIER(16바이트) SQL_C_GUID

        while (SQLFetch(_h_stmt) == SQL_SUCCESS) {
            //SQLGetData(_h_stmt, 1, SQL_C_WCHAR, name, 50, NULL);
            //SQLGetData(_h_stmt, 2, SQL_C_LONG, &accuid, sizeof(age), NULL);
            SQLGetData(_h_stmt, 1, SQL_C_SBIGINT, &accuid, sizeof(accuid), NULL);
            SQLGetData(_h_stmt, 2, SQL_C_CHAR, id, sizeof(id), NULL);
            SQLGetData(_h_stmt, 3, SQL_C_CHAR, pw, sizeof(pw), NULL);

            // 결과 출력
            std::cout << "inidata_uid:"<< accuid << " / id:" << id << " / pw:" << pw << "\n";
        }

    }
    return true;
}

bool DBConnection::TestQuery2()
{
    // 쿼리 실행을 위한 문장 핸들 생성
    if (SQL_ERROR == SQLAllocHandle(SQL_HANDLE_STMT, _h_dbc, &_h_stmt))
    {
        PrintErrorMessage(SQL_HANDLE_STMT, _h_stmt);
        return false;
    }

    // SQL문 실행
    //SQLWCHAR sql[] = L"SELECT * FROM account";
    SQLWCHAR sql[] = L"SELECT * FROM GameDB.dbo.testtable; ";

    SQLRETURN ret = SQLExecDirectW(_h_stmt, sql, SQL_NTS);
    if (ret == SQL_ERROR)
    {
        PrintErrorMessage(SQL_HANDLE_STMT, _h_stmt);
        return false;
    }

    // 결과 가져오기
    if (ret == SQL_SUCCESS || ret == SQL_SUCCESS_WITH_INFO) {
        SQLVARCHAR test[50] = {};


        while (SQLFetch(_h_stmt) == SQL_SUCCESS) {
            //SQLGetData(_h_stmt, 1, SQL_C_WCHAR, name, 50, NULL);
            //SQLGetData(_h_stmt, 2, SQL_C_LONG, &accuid, sizeof(age), NULL);
            SQLGetData(_h_stmt, 1, SQL_C_CHAR, test, sizeof(test), NULL);

            // 결과 출력
            std::cout << "test:" << test << "\n";
        }

    }
    return true;
}

void DBConnection::PrintErrorMessage(SQLSMALLINT handleType, SQLHANDLE handle)
{
    SQLWCHAR sqlState[6];
    SQLINTEGER nativeError;
    SQLWCHAR messageText[256];
    SQLSMALLINT textLength;
    SQLSMALLINT recNumber = 1;

    while (true)
    {
        SQLRETURN ret = SQLGetDiagRecW(
            handleType,
            handle,
            recNumber,
            sqlState,
            &nativeError,
            messageText,
            sizeof(messageText) / sizeof(SQLWCHAR),
            &textLength
        );

        if (ret == SQL_NO_DATA)
            break;
        if (ret != SQL_SUCCESS)
            break;

        char sqlStateA[6] = {};
        char messageTextA[512] = {};
        messageText[textLength] = '\0';
        WideCharToMultiByte(CP_ACP, 0, sqlState, -1, sqlStateA, sizeof(sqlStateA), NULL, NULL);
        WideCharToMultiByte(CP_ACP, 0, messageText, -1, messageTextA, sizeof(messageTextA), NULL, NULL);

        // ANSI 스타일 출력
        printf("[ODBC ERROR] SQLSTATE: %s\n", sqlStateA);
        printf("Native Error: %d\n", nativeError);
        printf("Message: %s\n\n", messageTextA);

        recNumber++;
    }
}

