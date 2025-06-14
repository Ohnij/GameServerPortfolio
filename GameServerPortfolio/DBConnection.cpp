#include "stdafx.h"
#include "DBConnection.h"


#include "jhnet.pb.h"
#include "PacketManager.h"
#include "Util.h"

#include "DBRequest_CheckAccountLogin.h"
#include "DBRequest_GetCharacterList.h"
#include "DBRequest_CreateCharacter.h"

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
    SQLDisconnect(_h_dbc);
    SQLFreeHandle(SQL_HANDLE_DBC, _h_dbc);
    SQLFreeHandle(SQL_HANDLE_ENV, _h_env);
    return true;
}

bool DBConnection::TestQuery()
{
    SQLHSTMT stmt;

    // 쿼리 실행을 위한 문장 핸들 생성
    if (SQL_ERROR == SQLAllocHandle(SQL_HANDLE_STMT, _h_dbc, &stmt))
    {
        PrintErrorMessage(SQL_HANDLE_STMT, stmt);
        //생성 못했으니 Free 해줄필요 없음.
        return false;
    }

    // SQL문 실행
    SQLWCHAR sql[] = L"SELECT * FROM accountDB.dbo.account";
    //SQLWCHAR sql[] = L"SELECT * FROM GameDB.dbo.testtable; ";

    SQLRETURN ret = SQLExecDirectW(stmt, sql, SQL_NTS);
    if (ret == SQL_ERROR)
    {
        PrintErrorMessage(SQL_HANDLE_STMT, stmt);
        SQLFreeHandle(SQL_HANDLE_STMT, stmt);
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

        while (SQLFetch(stmt) == SQL_SUCCESS) {
            //SQLGetData(_h_stmt, 1, SQL_C_WCHAR, name, 50, NULL);
            //SQLGetData(_h_stmt, 2, SQL_C_LONG, &accuid, sizeof(age), NULL);
            SQLGetData(stmt, 1, SQL_C_SBIGINT, &accuid, sizeof(accuid), NULL);
            SQLGetData(stmt, 2, SQL_C_CHAR, id, sizeof(id), NULL);
            SQLGetData(stmt, 3, SQL_C_CHAR, pw, sizeof(pw), NULL);

            // 결과 출력
            std::cout << "inidata_uid:"<< accuid << " / id:" << id << " / pw:" << pw << "\n";
        }

    }

    SQLFreeHandle(SQL_HANDLE_STMT, stmt);
    return true;
}

bool DBConnection::TestQuery2()
{
    SQLHSTMT stmt; 
    // 쿼리 실행을 위한 문장 핸들 생성
    if (SQL_ERROR == SQLAllocHandle(SQL_HANDLE_STMT, _h_dbc, &stmt))
    {
        PrintErrorMessage(SQL_HANDLE_STMT, stmt);
        //생성 못했으니 Free 해줄필요 없음.
        return false;
    }

    // SQL문 실행
    //SQLWCHAR sql[] = L"SELECT * FROM account";
    SQLWCHAR sql[] = L"SELECT * FROM GameDB.dbo.testtable; ";

    SQLRETURN ret = SQLExecDirectW(stmt, sql, SQL_NTS);
    if (ret == SQL_ERROR)
    {
        PrintErrorMessage(SQL_HANDLE_STMT, stmt);
        SQLFreeHandle(SQL_HANDLE_STMT, stmt);
        return false;
    }

    // 결과 가져오기
    if (ret == SQL_SUCCESS || ret == SQL_SUCCESS_WITH_INFO) {
        SQLVARCHAR test[50] = {};


        while (SQLFetch(stmt) == SQL_SUCCESS) {
            //SQLGetData(_h_stmt, 1, SQL_C_WCHAR, name, 50, NULL);
            //SQLGetData(_h_stmt, 2, SQL_C_LONG, &accuid, sizeof(age), NULL);
            SQLGetData(stmt, 1, SQL_C_CHAR, test, sizeof(test), NULL);

            // 결과 출력
            std::cout << "test:" << test << "\n";
        }

    }
    SQLFreeHandle(SQL_HANDLE_STMT, stmt);
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


void DBConnection::StoredProcedure_CheckAccountLogin(DBRequest_CheckAccountLogin* request)
{
    SQLHSTMT stmt = nullptr;
    SQLRETURN ret;

    int out_uid = -1;
    int out_result_code = 2;

    ret = SQLAllocHandle(SQL_HANDLE_STMT, _h_dbc, &stmt);
    if (ret != SQL_SUCCESS && ret != SQL_SUCCESS_WITH_INFO)
    {
        PrintErrorMessage(SQL_HANDLE_STMT, stmt);
        //생성 못했으니 Free 해줄필요 없음.
        return;
    }

    const char* sql = "{CALL accountdb.dbo.sp_CheckAccountLogin(?, ?, ?, ?)}";
    ret = SQLPrepareA(stmt, (SQLCHAR*)sql, SQL_NTS);
    if (ret != SQL_SUCCESS && ret != SQL_SUCCESS_WITH_INFO)
    {
        PrintErrorMessage(SQL_HANDLE_STMT, stmt);
        SQLFreeHandle(SQL_HANDLE_STMT, stmt);
        return;
    }
    //SQLBindParameter(stmt, 1, SQL_PARAM_OUTPUT, SQL_C_LONG, SQL_INTEGER, sizeof(ret), 0 & ret, 0, 0);
    //SQLBindParameter(stmt, 2, SQL_PARAM_INPUT, SQL_C_LONG, SQL_INTEGER, sizeof(prm), 0 & prm, 0, 0);

    SQLBindParameter(stmt, 1, SQL_PARAM_INPUT, SQL_C_CHAR, SQL_VARCHAR, 20, 0, (SQLPOINTER)request->m_id.c_str(), 0, 0);
    SQLBindParameter(stmt, 2, SQL_PARAM_INPUT, SQL_C_CHAR, SQL_VARCHAR, 30, 0, (SQLPOINTER)request->m_pw.c_str(), 0, 0);
    SQLBindParameter(stmt, 3, SQL_PARAM_OUTPUT, SQL_C_SLONG, SQL_INTEGER, sizeof(out_uid), 0, &out_uid, 0, 0);
    SQLBindParameter(stmt, 4, SQL_PARAM_OUTPUT, SQL_C_SLONG, SQL_INTEGER, sizeof(out_result_code), 0, &out_result_code, 0, 0);

    ret = SQLExecute(stmt);


    if (ret != SQL_SUCCESS && ret != SQL_SUCCESS_WITH_INFO)
    {
        PrintErrorMessage(SQL_HANDLE_STMT, stmt);
        SQLFreeHandle(SQL_HANDLE_STMT, stmt);
        //로그등..
        return;
    }

    SQLFreeHandle(SQL_HANDLE_STMT, stmt);

    //push 처리결과

    //처리로 이관할것

	//auto client = request->_client.lock();
	//if (!client)
	//{
	//	std::cerr << "StoredProcedure_CheckAccountLogin :: client null \n";
	//	return;
	//}

 //   jhnet::SCP_Login send_packet;
	//send_packet.set_account_uid(out_uid);
	//send_packet.set_login_ok(false);
	//if (out_result_code == 0)
	//{
	//	send_packet.set_login_ok(true);
	//	client->Login(out_uid);
	//	std::cerr << "로그인성공 \n";
	//}
	//else
	//{
	//	std::cerr << "로그인실패 \n";
	//}
	//PacketManager::Instance().Send(client, send_packet, jhnet::PacketId::S2C_LOGIN);

}

void DBConnection::StoredProcedure_GetCharacterList(DBRequest_GetCharacterList* request)
{
    jhnet::SCP_CharList send_packet;
    {
        SQLHSTMT stmt = nullptr;
        SQLRETURN ret;

        ret = SQLAllocHandle(SQL_HANDLE_STMT, _h_dbc, &stmt);
        if (ret != SQL_SUCCESS && ret != SQL_SUCCESS_WITH_INFO)
        {
            PrintErrorMessage(SQL_HANDLE_STMT, stmt);
            //생성 못했으니 Free 해줄필요 없음.
            return;
        }

        const char* sql = "{CALL GameDB.dbo.sp_GetCharacterList(?)}";
        ret = SQLPrepareA(stmt, (SQLCHAR*)sql, SQL_NTS);
        if (ret != SQL_SUCCESS && ret != SQL_SUCCESS_WITH_INFO)
        {
            PrintErrorMessage(SQL_HANDLE_STMT, stmt);
            SQLFreeHandle(SQL_HANDLE_STMT, stmt);
            return;
        }
        SQLBindParameter(stmt, 1, SQL_PARAM_INPUT, SQL_C_SLONG, SQL_INTEGER, sizeof(request->m_iAccountID), 0, &request->m_iAccountID, 0, 0);
        ret = SQLExecute(stmt);
        if (ret != SQL_SUCCESS && ret != SQL_SUCCESS_WITH_INFO)
        {
            PrintErrorMessage(SQL_HANDLE_STMT, stmt);
            SQLFreeHandle(SQL_HANDLE_STMT, stmt);
            //로그등..
            return;
        }


        //character_uid, nickname, char_level, job_code
        int character_uid;
        wchar_t nickname[20];
        int char_level;
        int job_code;

        SQLBindCol(stmt, 1, SQL_C_SLONG, &character_uid, 0, nullptr);
        SQLBindCol(stmt, 2, SQL_C_WCHAR, &nickname, sizeof(nickname), nullptr);
        SQLBindCol(stmt, 3, SQL_C_SLONG, &char_level, 0, nullptr);
        SQLBindCol(stmt, 4, SQL_C_SLONG, &job_code, 0, nullptr);

        //결과 행 반복 fetch
        while ((ret = SQLFetch(stmt)) == SQL_SUCCESS) {
            auto insert_character = send_packet.add_characters();
            insert_character->set_character_uid(character_uid);
            insert_character->set_nickname(WStringToUtf8(nickname));
            insert_character->set_level(char_level);
            insert_character->set_job_code(job_code);
        }
        SQLFreeHandle(SQL_HANDLE_STMT, stmt);
    }


    //push 처리결과

    //처리로 이관할것

    //auto client = request->_client.lock();
    //if (!client)
    //{
    //    std::cerr << "StoredProcedure_CheckAccountLogin :: client null \n";
    //    return;
    //}

    //PacketManager::Instance().Send(client, send_packet, jhnet::PacketId::S2C_CHAR_LIST);
}

void DBConnection::StoredProcedure_CreateCharacter(DBRequest_CreateCharacter* request)
{
    int out_char_uid = -1;
    int out_result_code = 2;
    // 0: 성공, 1: 닉네임 중복, 2: 기타 오류
    {
        SQLHSTMT stmt = nullptr;
        SQLRETURN ret;

        ret = SQLAllocHandle(SQL_HANDLE_STMT, _h_dbc, &stmt);
        if (ret != SQL_SUCCESS && ret != SQL_SUCCESS_WITH_INFO)
        {
            PrintErrorMessage(SQL_HANDLE_STMT, stmt);
            //생성 못했으니 Free 해줄필요 없음.
            return;
        }

        const char* sql = "{CALL accountdb.dbo.sp_ReserveCharacterSlot(?, ?, ?, ?)}";
        ret = SQLPrepareA(stmt, (SQLCHAR*)sql, SQL_NTS);
        if (ret != SQL_SUCCESS && ret != SQL_SUCCESS_WITH_INFO)
        {
            PrintErrorMessage(SQL_HANDLE_STMT, stmt);
            SQLFreeHandle(SQL_HANDLE_STMT, stmt);
            return;
        }
        SQLBindParameter(stmt, 1, SQL_PARAM_INPUT, SQL_C_WCHAR, SQL_WVARCHAR, 20, 0, (SQLPOINTER)request->m_wsNickname.c_str(), 0, 0);
        SQLBindParameter(stmt, 2, SQL_PARAM_INPUT, SQL_C_SLONG, SQL_INTEGER, sizeof(request->m_iAccountID), 0, &request->m_iAccountID, 0, 0);
        SQLBindParameter(stmt, 3, SQL_PARAM_OUTPUT, SQL_C_SLONG, SQL_INTEGER, sizeof(out_char_uid), 0, &out_char_uid, 0, 0);
        SQLBindParameter(stmt, 4, SQL_PARAM_OUTPUT, SQL_C_SLONG, SQL_INTEGER, sizeof(out_result_code), 0, &out_result_code, 0, 0);

        ret = SQLExecute(stmt);

        if (ret != SQL_SUCCESS && ret != SQL_SUCCESS_WITH_INFO)
        {
            PrintErrorMessage(SQL_HANDLE_STMT, stmt);
            SQLFreeHandle(SQL_HANDLE_STMT, stmt);
            //로그등..
            return;
        }
        SQLFreeHandle(SQL_HANDLE_STMT, stmt);
    }


    //push 처리결과

    //처리로 이관할것

    //auto client = request->_client.lock();
    //if (!client)
    //{
    //    std::cerr << "StoredProcedure_CheckAccountLogin :: client null \n";
    //    return;
    //}

    //jhnet::SCP_CreateChar send_packet;
    //send_packet.set_create_ok(false);
    //if (out_result_code != 0)
    //{
    //    (out_result_code == 1)? send_packet.set_error_message("중복된 닉네임") : send_packet.set_error_message("생성실패");
    //    PacketManager::Instance().Send(client, send_packet, jhnet::PacketId::S2C_CREATE_CHAR);
    //    return;
    //}


    {
        SQLHSTMT stmt = nullptr;
        SQLRETURN ret;

        ret = SQLAllocHandle(SQL_HANDLE_STMT, _h_dbc, &stmt);
        if (ret != SQL_SUCCESS && ret != SQL_SUCCESS_WITH_INFO)
        {
            PrintErrorMessage(SQL_HANDLE_STMT, stmt);
            //생성 못했으니 Free 해줄필요 없음.
            return;
        }


        //@character_uid INT,
        //@account_uid INT,
        //    @nickname NVARCHAR(20),
        //  @job_code tinyint,
        //    @ResultCode INT OUTPUT

        const char* sql = "{CALL GameDB.dbo.sp_CreateCharacter(?, ?, ?, ?, ?)}";
        ret = SQLPrepareA(stmt, (SQLCHAR*)sql, SQL_NTS);
        if (ret != SQL_SUCCESS && ret != SQL_SUCCESS_WITH_INFO)
        {
            PrintErrorMessage(SQL_HANDLE_STMT, stmt);
            SQLFreeHandle(SQL_HANDLE_STMT, stmt);
            return;
        }
        SQLBindParameter(stmt, 1, SQL_PARAM_INPUT, SQL_C_SLONG, SQL_INTEGER, sizeof(out_char_uid), 0, &out_char_uid, 0, 0);
        SQLBindParameter(stmt, 2, SQL_PARAM_INPUT, SQL_C_SLONG, SQL_INTEGER, sizeof(request->m_iAccountID), 0, &request->m_iAccountID, 0, 0);
        SQLBindParameter(stmt, 3, SQL_PARAM_INPUT, SQL_C_WCHAR, SQL_WVARCHAR, 20, 0, (SQLPOINTER)request->m_wsNickname.c_str(), 0, 0);
        SQLBindParameter(stmt, 4, SQL_PARAM_INPUT, SQL_C_TINYINT, SQL_TINYINT, sizeof(request->m_iJobCode), 0, &request->m_iJobCode, 0, 0);
        SQLBindParameter(stmt, 5, SQL_PARAM_OUTPUT, SQL_C_SLONG, SQL_INTEGER, sizeof(out_result_code), 0, &out_result_code, 0, 0);

        ret = SQLExecute(stmt);

        if (ret != SQL_SUCCESS && ret != SQL_SUCCESS_WITH_INFO)
        {
            PrintErrorMessage(SQL_HANDLE_STMT, stmt);
            SQLFreeHandle(SQL_HANDLE_STMT, stmt);
            //로그등..
            return;
        }
        SQLFreeHandle(SQL_HANDLE_STMT, stmt);
    }

    //push 처리결과

    //처리로 이관할것

    //if (out_result_code != 0)
    //{
    //    send_packet.set_error_message("캐릭터생성중 에러가 발생했습니다");
    //    PacketManager::Instance().Send(client, send_packet, jhnet::PacketId::S2C_CREATE_CHAR);
    //    return;
    //}


    //send_packet.set_create_ok(true);
    //PacketManager::Instance().Send(client, send_packet, jhnet::PacketId::S2C_LOGIN);

}

