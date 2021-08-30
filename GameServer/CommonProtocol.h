#pragma once

enum en_PACKET_TYPE
{
	////////////////////////////////////////////////////////
	//
	//	Client & Server Protocol
	//
	////////////////////////////////////////////////////////

	//------------------------------------------------------
	// Game Server
	//------------------------------------------------------
	en_PACKET_CS_GAME_SERVER = 0,
	//------------------------------------------------------------
	// 게임서버 섹터 이동 요청
	//
	//	{
	//		WORD	Type
	//
	//		INT64	AccountNo
	//		WORD	SectorX
	//		WORD	SectorY
	//	}
	//
	//------------------------------------------------------------
	en_PACKET_CS_GAME_REQ_SECTOR_MOVE,

	//------------------------------------------------------------
	// 게임서버 섹터 이동 결과
	//
	//	{
	//		WORD	Type
	//
	//		INT64	AccountNo
	//		WORD	SectorX
	//		WORD	SectorY
	//	}
	//
	//------------------------------------------------------------
	en_PACKET_CS_GAME_RES_SECTOR_MOVE,

	//------------------------------------------------------------
	// 게임서버 채팅보내기 요청
	//
	//	{
	//		WORD	Type
	//
	//		INT64	AccountNo
	//		WORD	MessageLen
	//		WCHAR	Message[MessageLen / 2]		// null 미포함
	//	}
	//
	//------------------------------------------------------------
	en_PACKET_CS_GAME_REQ_MESSAGE,

	//------------------------------------------------------------
	// 게임서버 채팅보내기 응답  (다른 클라가 보낸 채팅도 이걸로 받음)
	//
	//	{
	//		WORD	Type
	//
	//		INT64	AccountNo
	//		WCHAR	ID[20]						// null 포함
	//		WCHAR	Nickname[20]				// null 포함
	//		
	//		WORD	MessageLen
	//		WCHAR	Message[MessageLen / 2]		// null 미포함
	//	}
	//
	//------------------------------------------------------------
	en_PACKET_CS_GAME_RES_MESSAGE,

	//------------------------------------------------------------
	// 하트비트
	//
	//	{
	//		WORD		Type
	//	}
	//
	//
	// 클라이언트는 이를 30초마다 보내줌.
	// 서버는 40초 이상동안 메시지 수신이 없는 클라이언트를 강제로 끊어줘야 함.
	//------------------------------------------------------------	
	en_PACKET_CS_GAME_REQ_HEARTBEAT,


	//-----------------------------------------------------------
	// 게임서버 클라이언트 접속 완료 응답
	// {
	// 	   short Type
	// }
	//-----------------------------------------------------------
	en_PACKET_S2C_GAME_CLIENT_CONNECTED,

	//------------------------------------------------------------
	// 게임서버 로그인 요청
	//	{
    //		WORD	Type
	//
	//		int 	AccountNo
	//		WCHAR	ClientID[20];		
	//		int     TokenKey
	//	}
	//
	//------------------------------------------------------------
	en_PACKET_C2S_GAME_REQ_LOGIN,

	//------------------------------------------------------------
	// 게임서버 로그인 응답
	//	{
	//		WORD	Type
	//
	//		bool	Status				// 0:실패	1:성공	
	//	}
	//------------------------------------------------------------
	en_PACKET_S2C_GAME_RES_LOGIN,
	
	//------------------------------------------------------------
	// 게임서버 캐릭터 생성 요청
	// int32 CharacterNameLen
	// WCHAR CharacterName
	//------------------------------------------------------------
	en_PACKET_C2S_GAME_CREATE_CHARACTER,
	
	//------------------------------------------------------------
	// 게임서버 캐릭터 생성 요청 응답
	// int32 PlayerDBId
	// bool IsSuccess
	// wstring PlayerName
	//------------------------------------------------------------
	en_PACKET_S2C_GAME_CREATE_CHARACTER,

	//------------------------------------------------------------
	// 게임서버 캐릭터 입장
	// int64 AccoountId
	// int8 EnterGameCharacterNameLen
	// wstring EnterGameCharacterName
	//------------------------------------------------------------
	en_PACKET_C2S_GAME_ENTER,
	
	//------------------------------------------------------------
	// 게임서버 캐릭터 입장 요청 응답
	// int64 AccountId
	// int32 PlaterDBId
	// wstring EnterCharaceterName
	// st_GameObjectInfo ObjectInfo
	//------------------------------------------------------------
	en_PACKET_S2C_GAME_ENTER,
	
	//------------------------------------------------------------
	// 게임서버 캐릭터 움직이기 요청
	// int64 AccountId
	// int32 PlayerDBId
	// int8 Dir
	//------------------------------------------------------------
	en_PACKET_C2S_MOVE,
	
	//------------------------------------------------------------
	// 게임서버 캐릭터 움직이기 요청
	// int64 AccountId
	// int32 PlayerDBId
	// bool CanGo
	// st_PositionInfo PositionInfo
	//------------------------------------------------------------
	en_PACKET_S2C_MOVE,	
	
	//------------------------------------------------------------
	// 게임서버 캐릭터 공격 요청
	// int64 AccountId
	// int32 PlayerDBId
	// int8 Dir
	// en_AttackRange RangeAttack;
	// int8 RangeDistance;
	//------------------------------------------------------------	
	en_PACKET_C2S_ATTACK,
	
	//------------------------------------------------------------
	// 게임서버 캐릭터 공격 요청 응답
	// int64 AccountId
	// int32 PlayerDBId
	// int8 Dir
	//------------------------------------------------------------	
	en_PACKET_S2C_ATTACK,
	
	//------------------------------------------------------------
	// 게임서버 캐릭터 스폰
	// int64 AccountId
	// int32 PlayerDBId
	// wstring SpawnObjectName
	// st_GameObjectInfo GameObjectInfo
	//------------------------------------------------------------	
	en_PACKET_S2C_SPAWN,
	
	//------------------------------------------------------------
	// 게임서버 캐릭터 디스폰
	// int64 AccountId
	// int32 PlayerDBId
	//------------------------------------------------------------
	en_PACKET_S2C_DESPAWN,
	
	//------------------------------------------------------------
	// 게임서버 HP 변경
	// int64 AccountId
	// int32 PlayerDBId
	// int32 CurrentHP
	// int32 MaxHP
	//------------------------------------------------------------
	en_PACKET_S2C_CHANGE_HP,
	
	//------------------------------------------------------------
	// 게임서버 마우스 위치 캐릭터 정보 요청	
	// int64 AccountId
	// int32 PlayerDBId
	// int32 X
	// int32 Y
	//------------------------------------------------------------
	en_PACKET_C2S_MOUSE_POSITION_OBJECT_INFO,
	
	//------------------------------------------------------------
	// 게임서버 마우스 위치 캐릭터 정보 요청 응답
	// int64 AccountId
	// int32 PlayerDBId
	// st_GameObjectInfo ObjectInfo
	//------------------------------------------------------------
	en_PACKET_S2C_MOUSE_POSITION_OBJECT_INFO,
	
	//------------------------------------------------------------
	// 게임서버 오브젝트 상태 변경 요청
	// int64 AccountId
	// int64 ObjectId
	// en_CreatureState ObjectState
	//------------------------------------------------------------
	en_PACKET_C2S_OBJECT_STATE_CHANGE,

	//------------------------------------------------------------
	// 게임서버 오브젝트 상태 변경 요청 응답
	// int32 ObjectId
	// en_CreatureState ObjectState
	//------------------------------------------------------------
	en_PACKET_S2C_OBJECT_STATE_CHANGE,

	//------------------------------------------------------------
	// 게임서버 오브젝트 죽음 응답
	// int32 ObjectId
	//------------------------------------------------------------
	en_PACKET_S2C_DIE,

	//------------------------------------------------------------
    // 게임서버 채팅 메세지 요청
    // int32 ObjectId
    // string Message
    //------------------------------------------------------------
	en_PACKET_C2S_MESSAGE,

	//------------------------------------------------------------
	// 게임서버 채팅 메세지 요청
	// int32 ObjectId
	// string Message
	//------------------------------------------------------------
	en_PACKET_S2C_MESSAGE,
	
	//------------------------------------------------------------
	// 게임서버 위치 동기화 요청
	// int32 ObjectId
	// st_Position Position
	//------------------------------------------------------------
	en_PACKET_C2S_SYNC_POSITION,

	//------------------------------------------------------------
	// 게임서버 위치 동기화 요청 응답
	// int32 ObjectId
	// st_Position Position
	//------------------------------------------------------------
	en_PACKET_S2C_SYNC_POSITION,


	//------------------------------------------------------------
	// 게임서버 아이템 인벤토리 저장 요청
	// int64 AccountId
	// int64 ObjectId
	// int8 ObjectType
	// int32 ObjectPositionX
	// int32 ObjectPositionY
	// int64 TargetObjectId
	//------------------------------------------------------------
	en_PACKET_C2S_ITEM_TO_INVENTORY,
    
	//------------------------------------------------------------
	// 게임서버 아이템 인벤토리 저장 요청 응답
	// int64 TargetObjectId
	// st_ItemInfo ItemInfo
	//------------------------------------------------------------
	en_PACKET_S2C_ITEM_TO_INVENTORY,

	//------------------------------------------------------------
	// 게임서버 아이템 인벤토리 저장 요청 응답
	// int64 AccountId
	// int64 ObjectId
	// int64 GoldCount
	// int8 SliverCount
	// int8 BronzeCount
	//------------------------------------------------------------
	en_PACKET_S2C_GOLD_SAVE
};