✅완료
❌미완료
➕추가작업
📌픽스작업
🚧필요작업
🔄진행중
# 250606
✅MSSQL Server / SSMS설치 및 ODBC연동
✅README작성 - 개발환경 버전 기록
➕DB테이블 최소기능버전 생성  + SP프로시저 제작
➕INI파싱으로 민감정보 깃허브 제외

# 250607
❌C++ SP프로시저 구조 제작
❌C# DummyClient 수동 조작으로 Character 생성 까지 만들어보기
➕Client Allocater 제작 Client(IOCP통신객체) -> GameClient(Game에서사용)상속 객체
* Function 이용, 상속구조 GameClient Allocater
* 생성시 GameClient 풀에서 가져와서 Client로 캐스팅
* 해제시 Client를 GameClient로 캐스팅 > 없으면 smart_ptr해제, 있으면 풀 반납
➕InitPacketHandler() >생성자로 이동하여 호출누락 방지. 
➕매니저 생성자는 Private로 설정하여 Instance외에 호출되지 않도록 수정
➕ObjectPool() -> Clear함수 추가
➕Iocp 시작시 Client allocater 매개변수 추가
DBManager제작
* DBConnection / Thread / DBRequest Queue관리 
➕프로시저호출 테스트완료

# 250608
✅C# DummyClient 수동 조작 버전 Flow
* 기본 Flow완성
🔄C# DummyClient 수동 조작으로 Character 생성 까지 만들어보기
* 로그인까지 완료
✅C++ Client수신 > DB > GameClient 캐싱 > Client송신 Flow 제작 및 테스트
➕C# DummyClient Command Reflection / Attribute C#문법 사용하여 함수매핑자동화
➕DummyClient 로직 수정
➕C++ PacketManager -> Packet생성자 매핑 지저분해서 함수제작
➕C++ PacketManager -> Packet핸들러 매핑 지저분해서 함수제작

# 250609
✅C# DummyClient 수동 조작으로 Character 생성 까지 만들어보기 

# 250610~250613
➕서버코어 부분 Library 로 옮기기
➕채팅서버 프로젝트 파서 테스트


# 250614
📌Game서버 리팩토링





# WorkList
🔄C# DummyClient 수동 조작 버전 Flow
📝GameClient 정보 캐싱 변수 +
📝GameMap(인스턴스) / Packet / DBResponse 구조 생각해보고 구조짜기.
📝Unity 클라이언트 제작
📝GameMap Data로드 및 Data파싱, class객체 생성
📝최소기능 Game Flow 성공 (연결-로그인-게임맵진입-행동)
📝Thread 관리 구조 
📝C++ SP프로시저 구조 제작