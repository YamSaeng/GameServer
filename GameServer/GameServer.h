#pragma once
#pragma comment(lib,"winmm")
#include "NetworkLib.h"
#include "CommonProtocol.h"
#include "GameServerInfo.h"
#include "MemoryPoolTLS.h"
#include "Heap.h"

class CGameServerMessage;
class CMap;
class CDay;

class CGameServer : public CNetworkLib
{
private:
	// 로그인 서버와 통신할 소켓
	SOCKET _LoginServerSock;
	// 유저 데이터베이스 쓰레드
	HANDLE _UserDataBaseThread;
	// 월드 데이터베이스 쓰레드
	HANDLE _WorldDataBaseThread;
	// 클라이언트 종료시 정보저장 쓰레드
	HANDLE _ClientLeaveSaveThread;
	// 타이머잡 쓰레드
	HANDLE _TimerJobThread;
	// 로직 쓰레드
	HANDLE _LogicThread;

	// 타이머잡 쓰레드 깨우기 이벤트
	HANDLE _TimerThreadWakeEvent;

	// WorkerThread 종료용 변수
	bool _NetworkThreadEnd;
	// UpdateThrad 종료용 변수
	bool _UpdateThreadEnd;
	// User DataBaseThread 종료용 변수
	bool _UserDataBaseThreadEnd;
	// World DataBaseThread 종료용 변수
	bool _WorldDataBaseThreadEnd;
	// ClientLeaveSaveThread 종료용 변수
	bool _ClientLeaveSaveDBThreadEnd;

	// TimerJobThread 종료용 변수
	bool _TimerJobThreadEnd;
	// LogicThread 종료용 변수
	bool _LogicThreadEnd;

	// TimerJobThread 전용 Lock
	SRWLOCK _TimerJobLock;

	// 하루 관리
	CDay* _Day;

	//----------------------------------------------------------
	// Update 쓰레드
	//----------------------------------------------------------
	static unsigned __stdcall UpdateThreadProc(void* Argument);
	//----------------------------------------------------------
	// 유저 데이터베이스 쓰레드 ( 유저 데이터 베이스 작업 처리 )
	//----------------------------------------------------------
	static unsigned __stdcall UserDataBaseThreadProc(void* Argument);
	//----------------------------------------------------------
	// 월드 데이터베이스 쓰레드 ( 월드 데이터 베이스 작업 처리 )
	//----------------------------------------------------------
	static unsigned __stdcall WorldDataBaseThreadProc(void* Argument);
	//-------------------------------------------------------------
	// 클라이언트 접속 종료시 Player의 정보를 DB에 저장할 쓰레드
	//-------------------------------------------------------------
	static unsigned __stdcall ClientLeaveThreadProc(void* Argument);

	//----------------------------------------------------------
	// 타이머 잡 쓰레드 ( 타이머 잡 처리 )
	//----------------------------------------------------------
	static unsigned __stdcall TimerJobThreadProc(void* Argument);
	//--------------------------------------------------------
	// 로직처리 쓰레드 
	//--------------------------------------------------------
	static unsigned __stdcall LogicThreadProc(void* Argument);

	//---------------------------------
	// 캐릭터 생성 시 기본 셋팅
	//---------------------------------
	void PlayerDefaultSetting(int64& AccountId, st_GameObjectInfo& NewCharacterInfo, int8& CharacterCreateSlotIndex);

	//------------------------------------
	// 클라 접속 기본 정보 셋팅
	//------------------------------------
	void CreateNewClient(int64 SessionId);
	//------------------------------------
	// 클라 삭제 
	//------------------------------------
	void DeleteClient(st_Session* Session);

	//-------------------------------------------------------
	// 로그인 서버에 패킷 전송
	//-------------------------------------------------------
	void SendPacketToLoginServer(CGameServerMessage* Message);
	//--------------------------------------------------
	// 네트워크 패킷 처리
	//--------------------------------------------------
	void PacketProc(int64 SessionID, CMessage* Message);

	//----------------------------------------------------------------
	// 네트워크 패킷처리 함수
	//----------------------------------------------------------------

	//----------------------------------------------------------
	// 로그인 요청 처리
	//----------------------------------------------------------
	void PacketProcReqLogin(int64 SessionID, CMessage* Message);
	//-------------------------------------------------------------------
	// 캐릭터 생성 요청 처리
	//-------------------------------------------------------------------
	void PacketProcReqCreateCharacter(int64 SessionID, CMessage* Message);
	//-------------------------------------------------------------
	// 게임 입장 요청 처리
	//-------------------------------------------------------------
	void PacketProcReqEnterGame(int64 SessionID, CMessage* Message);
	//-------------------------------------------------------------
	// 캐릭터 정보 요청 처리
	//-------------------------------------------------------------
	void PacketProcReqCharacterInfo(int64 SessionID, CMessage* Message);

	
	//---------------------------------------------------------
	// 이동 요청 처리
	//---------------------------------------------------------
	void PacketProcReqMove(int64 SessionID, CMessage* Message);
	//------------------------------------------------------------
	// 이동 멈춤 처리
	//------------------------------------------------------------
	void PacketProcReqMoveStop(int64 SessionID, CMessage* Message);
	//------------------------------------------------------------
	// 바라보는 방향 처리
	//------------------------------------------------------------
	void PacketProcReqLookAtDirection(int64 SessionID, CMessage* Message);
	//---------------------------------------------------------------
	// 기술 처리 요청 처리
	//---------------------------------------------------------------
	void PacketProcReqSkillProcess(int64 SessionID, CMessage* Message);	
	//---------------------------------------------------------
	// 기술 시전 요청 취소 처리
	//---------------------------------------------------------
	void PacketProcReqSkillCastingCancel(int64 SessionId, CMessage* Message);
	//----------------------------------------------------------------------------
	// 채집 요청 처리
	//----------------------------------------------------------------------------
	void PacketProcReqGathering(int64 SessionID, CMessage* Message);
	//---------------------------------------------------------
	// 채집 요청 취소 처리
	//---------------------------------------------------------
	void PacketProcReqGatheringCancel(int64 SessionID, CMessage* Message);
	//----------------------------------------------------------------------------
	// 왼쪽 마우스 클릭 위치 오브젝트 정보 요청 처리
	//----------------------------------------------------------------------------
	void PacketProcReqLeftMouseObjectInfo(int64 SessionId, CMessage* Message);	
	//----------------------------------------------------------------------------
	// 왼쪽 마우스 클릭 UI 오브젝트 정보 요청 처리
	//----------------------------------------------------------------------------
	void PacketProcReqLeftMouseUIObjectInfo(int64 SessionID, CMessage* Message);
	//----------------------------------------------------------------------------
	// 왼쪽 마우스 드래그 오브젝트 선택
	//----------------------------------------------------------------------------
	void PacketProcReqLeftMouseDragObjectsSelect(int64 SessionID, CMessage* Message);
	//----------------------------------------------------------------------------
	// 오른쪽 마우스 클릭 위치 오브젝트 정보 요청 처리
	//----------------------------------------------------------------------------
	void PacketProcReqRightMouseObjectInfo(int64 SessionId, CMessage* Message);	
	//--------------------------------------------------------------------------
	// 제작대 선택 풀림 요청 처리
	//--------------------------------------------------------------------------
	void PacketProcReqCraftingTableNonSelect(int64 SessionID, CMessage* Message);	
	//--------------------------------------------------------------------
	// 채팅 메세지 요청 처리
	//--------------------------------------------------------------------
	void PacketProcReqChattingMessage(int64 SessionId, CMessage* Message);
	//------------------------------------------------------------
	// 아이템 가방 선택 요청 처리
	//------------------------------------------------------------
	void PacketProcReqItemSelect(int64 SessionId, CMessage* Message);
	//------------------------------------------------------------
	// 아이템 놓기 요청 처리
	//------------------------------------------------------------
	void PacketProcReqItemPlace(int64 SessionId, CMessage* Message);
	//------------------------------------------------------------
	// 아이템 회전 요청 처리
	//------------------------------------------------------------
	void PacketProcReqItemRotate(int64 SessionID, CMessage* Message);
	//-----------------------------------------------------------------------------
	// 스킬 특성 선택 요청 처리
	//-----------------------------------------------------------------------------
	void PacketProcReqSelectSkillCharacteristic(int64 SessionID, CMessage* Message);
	//---------------------------------------------------------------
	// 스킬 배우기 요청 처리
	//---------------------------------------------------------------
	void PacketProcReqLearnSkill(int64 SessionID, CMessage* Message);
	//------------------------------------------------------------------
	// 퀵슬롯 저장 요청 처리
	//------------------------------------------------------------------
	void PacketProcReqQuickSlotSave(int64 SessionId, CMessage* Message);
	//------------------------------------------------------------------
	// 퀵슬롯 스왑 요청 처리
	//------------------------------------------------------------------
	void PacketProcReqQuickSlotSwap(int64 SessionId, CMessage* Message);
	//------------------------------------------------------------------
	// 퀵슬롯 초기화 요청 처리
	//------------------------------------------------------------------
	void PacketProcReqQuickSlotInit(int64 SessionId, CMessage* Message);
	//------------------------------------------------------------------
	// 제작템 제작 요청 처리
	//------------------------------------------------------------------
	void PacketProcReqCraftingConfirm(int64 SessionId, CMessage* Message);
	//------------------------------------------------------------------
	// 아이템 사용 요청 처리
	//------------------------------------------------------------------
	void PacketProcReqItemUse(int64 SessionId, CMessage* Message);
	//------------------------------------------------------------------
	// 장비 아이템 해제 요청 처리
	//------------------------------------------------------------------
	void PacketProcReqOffEquipment(int64 SessionID, CMessage* Message);	
	//------------------------------------------------------------------
	// 씨앗 심기 요청 처리
	//------------------------------------------------------------------
	void PacketProcReqSeedFarming(int64 SessionID, CMessage* Message);
	//------------------------------------------------------------------
	// 작물 성장 단계 확인 요청 처리
	//------------------------------------------------------------------
	void PacketProcReqPlantGrowthCheck(int64 SessionID, CMessage* Message);
	//------------------------------------------------------------------
	// 그룹 초대 요청 처리
	//------------------------------------------------------------------
	void PacketProcReqPartyInvite(int64 SessionID, CMessage* Message);
	//------------------------------------------------------------------
	// 그룹 초대 수락 요청 처리
	//------------------------------------------------------------------
	void PacketProcReqPartyAccept(int64 SessionID, CMessage* Message);
	//------------------------------------------------------------------
	// 그룹 초대 거절 요청 처리
	//------------------------------------------------------------------
	void PacketProcReqPartyReject(int64 SessionID, CMessage* Message);
	//------------------------------------------------------------------
	// 그룹 탈퇴 요청 처리
	//------------------------------------------------------------------
	void PacketProcReqPartyQuit(int64 SessionID, CMessage* Message);
	//------------------------------------------------------------------
	// 그룹 추방 요청 처리
	//------------------------------------------------------------------
	void PacketProcReqPartyBanish(int64 SessionID, CMessage* Message);
	//------------------------------------------------------------------
	// 메뉴 요청 처리
	//------------------------------------------------------------------
	void PacketProcReqMenu(int64 SessionID, CMessage* Message);
	//------------------------------------------------------------------
	// 메뉴 요청 처리
	//------------------------------------------------------------------
	void PacketProcReqInteraction(int64 SessionID, CMessage* Message);
	//------------------------------------------------------------------
	// 아이템 줍기 요청 처리
	//------------------------------------------------------------------
	void PacketProcReqItemLooting(int64 SessionId, CMessage* Message);
	//------------------------------------------------------------------
	// 아이템 버리기 요청 처리
	//------------------------------------------------------------------
	void PacketProcReqItemDrop(int64 SessionID, CMessage* Message);
	//--------------------------------------------------------------------------
	// 제작대 재료 아이템 넣기 요청 처리
	//---------------------------------------------------------------------------
	void PacketProcReqCraftingTableItemAdd(int64 SessionID, CMessage* Message);
	//----------------------------------------------------------------------------
	// 제작대 재료 아이템 빼기 요청 처리
	//----------------------------------------------------------------------------
	void PacketProcReqCraftingTableMaterialItemSubtract(int64 SessionID, CMessage* Message);
	//----------------------------------------------------------------------------
	// 제작대 완성 아이템 빼기 요청 처리
	//----------------------------------------------------------------------------
	void PacketProcReqCraftingTableCompleteItemSubtract(int64 SessionID, CMessage* Message);
	//--------------------------------------------------------------------------
	// 제작대 제작 요청 처리
	//--------------------------------------------------------------------------
	void PacketProcReqCraftingTableCraftingStart(int64 SessionID, CMessage* Message);
	//--------------------------------------------------------------------------
	// 제작대 제작 멈춤 처리
	//--------------------------------------------------------------------------
	void PacketProcReqCraftingTableCraftingStop(int64 SessionID, CMessage* Message);
	//--------------------------------------------------------
	// 퐁 패킷 처리
	//--------------------------------------------------------
	void PacketProcReqPong(int64 SessionID, CMessage* Message);

	//-----------------------------------------------------------------------------------------------
	// DB 요청 처리 함수
	//-----------------------------------------------------------------------------------------------

	//-------------------------------------------------------------------
	// AccountID가 AccountDB에 있는지 체크
	//-------------------------------------------------------------------
	void PacketProcReqDBAccountCheck(CMessage* Message);
	//-------------------------------------------------------------------------------
	// 생성요청한 캐릭터가 DB에 있는지 확인 후 캐릭터 생성
	//-------------------------------------------------------------------------------
	void PacketProcReqDBCreateCharacterNameCheck(CMessage* Message);		
	//-----------------------------------------------------------------------
	// 게임 서버 접속시 캐릭터 정보를 DB에서 가져와서 클라에 전송
	//-----------------------------------------------------------------------
	void PacketProcReqDBCharacterInfoSend(CMessage* Message);
	//--------------------------------------------------------------------
	// 접속 종료시 플레이어 정보 DB에 기록
	//--------------------------------------------------------------------
	void PacketProcReqDBLeavePlayerInfoSave(CGameServerMessage* Message);

	//--------------------------------------------------------------------
	// 타일 정보 DB에서 가져오기
	//--------------------------------------------------------------------
	void WorldMapTileInfoLoad();

	//------------------------------------------------------------------
	// 타이머 잡 요청 처리 함수
	//------------------------------------------------------------------	
	
	//----------------------------------------------------------------
	// 핑 처리
	//----------------------------------------------------------------
	void PacketProcTimerPing(int64 SessionId);

	//--------------------------------------
	// 게임오브젝트 잡 생성 함수
	//--------------------------------------		
	
	//------------------------------------------------------------------------------
	// 플레이어 채널 퇴장 잡 생성 함수
	//------------------------------------------------------------------------------
	st_GameObjectJob* MakeGameObjectJobLeaveChannelPlayer(CGameObject* LeavePlayerObject, int32* PlayerIndexes);
	st_GameObjectJob* MakeGameObjectJobMove(float DirectionX, float DirectionY, float PositionX, float PositionY, int8 GameObjectState);
	st_GameObjectJob* MakeGameObjectJobMoveStop(float PositionX, float PositionY, int8 GameObjectState);			
	st_GameObjectJob* MakeGameObjectJobLookAtDirection(float DirectionX, float DirectionY);
	//-------------------------------------------------
	// 기술 특성 선택 잡 생성 함수
	//-------------------------------------------------
	st_GameObjectJob* MakeGameObjectJobSelectSkillCharacteristic(int8 SelectChracteristicType);		
	//------------------------------------------------------------------------------------------------------------------------------------
	// 기술 배우기 잡 생성 함수 
	//------------------------------------------------------------------------------------------------------------------------------------
	st_GameObjectJob* MakeGameObjectJobSkillLearn(bool IsSkillLearn, int8 LearnSkillCharacteristicType, int16 LearnSkillType);
	//-------------------------------------------------
	// 기술 처리 잡 생성 함수
	//-------------------------------------------------
	st_GameObjectJob* MakeGameObjectJobSkillProcess(int8 QuickSlotBarIndex, int8 QuickSlotBarSlotIndex, int8 SkillCharacteristicType, int16 SkillType, float SkillDirectionX, float SkillDirectionY);	
	//------------------------------------------------
	// 시전 취소 잡 생성 함수
	//------------------------------------------------
	st_GameObjectJob* MakeGameObjectJobSkillCastingCancel();	
	//------------------------------------------------
	// 채집 시작 잡 생성 함수
	//------------------------------------------------
	st_GameObjectJob* MakeGameObjectJobGatheringStart(CGameObject* GatheringObject);
	//---------------------------------------------------
	// 채집 취소 잡 생성 함수
	//---------------------------------------------------
	st_GameObjectJob* MakeGameObjectJobGatheringCancel();	
	
	//-----------------------------------------------------------------------------------
	// 아이템 버리기 잡 생성 함수
	//-----------------------------------------------------------------------------------
	st_GameObjectJob* MakeGameObjectJobItemDrop(int16 DropItemType, int32 DropItemCount);		
	//---------------------------------------------------------
	// 제작대 제작 시작 잡 생성 함수
	//---------------------------------------------------------
	st_GameObjectJob* MakeGameObjectJobCraftingTableStart(CGameObject* CraftingStartObject, en_SmallItemCategory CraftingCompleteItemType, int16 CraftingCount);
	//---------------------------------------------------------------------------------------------------
	// 제작대 아이템 재료 넣기 잡 생성 함수
	//---------------------------------------------------------------------------------------------------
	st_GameObjectJob* MakeGameObjectJobCraftingTableItemAdd(CGameObject* CraftingTableItemAddObject, int16 AddItemSmallCategory, int16 AddItemCount);
	//---------------------------------------------------------------------------------------------------
	// 제작대 아이템 재료 빼기 잡 생성 함수
	//---------------------------------------------------------------------------------------------------
	st_GameObjectJob* MakeGameObjectJobCraftingTableMaterialItemSubtract(CGameObject* CraftingTableItemSubtractObject, int16 SubtractItemSmallCategory, int16 SubtractItemCount);
	//---------------------------------------------------------------------------------------------------
	// 제작대 완성 아이템 빼기 잡 생성 함수
	//---------------------------------------------------------------------------------------------------
	st_GameObjectJob* MakeGameObjectJobCraftingTableCompleteItemSubtract(CGameObject* CraftingTableItemSubtractObject, int16 SubtractItemSmallCategory, int16 SubtractItemCount);


	//---------------------------------------------------------
	// 제작대 제작 멈춤 잡 생성 함수
	//---------------------------------------------------------
	st_GameObjectJob* MakeGameObjectJobCraftingTableCancel(CGameObject* CraftingStopObject);

	//--------------------------------------
	// 패킷조합 함수		
	//--------------------------------------

	//--------------------------------------
	// 클라이언트 접속 응답 패킷 조합
	//--------------------------------------
	CGameServerMessage* MakePacketResClientConnected();
	//---------------------------------------------------------------------------------------
	// 로그인 요청 응답 패킷 조합
	//---------------------------------------------------------------------------------------
	CGameServerMessage* MakePacketResLogin(bool& Status, int8& PlayerCount, int32* MyPlayerIndexes);	
	//---------------------------------------------------------------------------------------
	// 유저가 보유중인 캐릭터 정보 보내주기 패킷 조합
	//---------------------------------------------------------------------------------------
	CGameServerMessage* MakePacketMyCharacterInfos(int8& CharacterCount, int32* MyPlayerIndexes);
	//--------------------------------------------------------------------------------------------------
	// 캐릭터 생성 요청 응답 패킷 조합
	//--------------------------------------------------------------------------------------------------
	CGameServerMessage* MakePacketResCreateCharacter(bool IsSuccess, st_GameObjectInfo& CreateCharacterObjectInfo);				
	//---------------------------------------------------------------------------------------------
	// 게임서버 가방 아이템 선택 요청 응답 패킷 조합
	//---------------------------------------------------------------------------------------------
	CGameServerMessage* MakePacketResSelectItem(int64 AccountId, int64 ObjectId, CItem* SelectItem);
	//---------------------------------------------------------------------------------------------
	// 게임서버 아이템 회전 요청 응답 패킷 조합
	//---------------------------------------------------------------------------------------------
	CGameServerMessage* MakePacketResItemRotate(int64 AccountID, int64 PlayerID);
	//---------------------------------------------------------------------------------------------
	// 게임서버 가방 아이템 놓기 요청 응답 패킷 조합
	//---------------------------------------------------------------------------------------------
	CGameServerMessage* MakePacketResPlaceItem(int64 AccountId, int64 ObjectId, CItem* PlaceItem, CItem* OverlapItem);	
	//-----------------------------------------------------------------------------------------
	// 게임서버 가방 아이템 사용 요청 응답 패킷 조합
	//-----------------------------------------------------------------------------------------
	CGameServerMessage* MakePacketInventoryItemUse(int64 PlayerId, st_ItemInfo& UseItemInfo);	
	//-----------------------------------------------------------------------------------------
	// 게임서버 퀵슬롯 등록 요청 응답 패킷 조합
	//-----------------------------------------------------------------------------------------
	CGameServerMessage* MakePacketResQuickSlotBarSlotSave(st_QuickSlotBarSlotInfo QuickSlotBarSlotInfo);
	//-----------------------------------------------------------------------------------------
	// 게임서버 퀵슬롯 생성 패킷 조합
	//-----------------------------------------------------------------------------------------
	CGameServerMessage* MakePacketQuickSlotCreate(int8 QuickSlotBarSize, int8 QuickSlotBarSlotSize, vector<st_QuickSlotBarSlotInfo> QuickslotBarSlotInfos);
	//-----------------------------------------------------------------------------------------
	// 게임서버 퀵슬롯 스왑 요청 응답 패킷 조합 
	//-----------------------------------------------------------------------------------------
	CGameServerMessage* MakePacketResQuickSlotSwap(st_QuickSlotBarSlotInfo SwapAQuickSlotInfo, st_QuickSlotBarSlotInfo SwapBQuickSlotInfo);	
	//-----------------------------------------------------------------------------------------
	// 게임서버 제작템 목록 패킷 조합
	//-----------------------------------------------------------------------------------------
	CGameServerMessage* MakePacketCraftingList(int64 AccountId, int64 PlayerId, vector<st_CraftingItemCategory> CraftingItemList);			
	
	//-------------------------------------------------
	// 게임서버 핑 패킷 조합
	//-------------------------------------------------
	CGameServerMessage* MakePacketPing();		
public:
	CItem* NewItemCrate(st_ItemInfo& NewItemInfo);

	st_GameObjectJob* MakeGameObjectJobObjectDeSpawnObjectChannel(CGameObject* DeSpawnChannelObject);

	//-------------------------------------------------------------------------------
	// 플레이어 채널 입장 잡 생성 함수
	//-------------------------------------------------------------------------------
	st_GameObjectJob* MakeGameObjectJobPlayerEnterChannel(CGameObject* EnterChannelObject);
	//-------------------------------------------------------------------------------
	// 일반 오브젝트 채널 입장 잡 생성 함수
	//-------------------------------------------------------------------------------
	st_GameObjectJob* MakeGameObjectJobObjectEnterChannel(CGameObject* EnterChannelObject);
	//-------------------------------------------------------------------------------
	// 채널에서 일반 오브젝트 찾기 잡 생성 함수
	//-------------------------------------------------------------------------------
	st_GameObjectJob* MakeGameObjectJobFindObjectChannel(CGameObject* ReqPlayer, int64& FindObjectID, int16& FindObjectType);
	//-----------------------------------------------------------------------------------------------------------------------------
	// 게임서버 제작대 선택풀림 요청 응답 패킷 조합
	//-----------------------------------------------------------------------------------------------------------------------------
	CGameServerMessage* MakePacketResCraftingTableNonSelect(int64 CraftingTableObjectID, en_GameObjectType CraftingTableObjectType);
	//-------------------------------------------------------------------------------
	// 채널에서 제작대 제작 아이템 선택 잡 생성 함수
	//-------------------------------------------------------------------------------
	st_GameObjectJob* MakeGameObjectJobFindCraftingTableSelectItem(CGameObject* ReqPlayer, int64& FindCraftingTableObjectID, int16& FindCraftingTableObjectType, int16& LeftMouseItemCategory);
	//-------------------------------------------------------------------------------
	// 채널에서 제작대 제작 아이템 선택 잡 생성 함수
	//-------------------------------------------------------------------------------
	st_GameObjectJob* MakeGameObjectJobRightMouseObjectInfo(CGameObject* ReqPlayer, int64& FindObjectID, int16& FindObjectType);
	//-------------------------------------------------------------------------------
	// 채널에서 마우스 드래그 다중 오브젝트 선택 잡 생성 함수
	//-------------------------------------------------------------------------------
	st_GameObjectJob* MakeGameObjectLeftMouseDragObjectsSelect(int64 TargetID, float MouseDragStartPositionX, float MouseDragStartPositionY, float MouseDragEndPositionX, float MouseDragEndPositionY);
	//-------------------------------------------------------------------------------
	// 플레이어 제외 오브젝트 채널 퇴장 잡 생성 함수
	//-------------------------------------------------------------------------------
	st_GameObjectJob* MakeGameObjectJobLeaveChannel(CGameObject* LeaveChannelObject);	
	//-------------------------------------------------------------------------------
	// 데미지 처리 잡 생성 함수
	//-------------------------------------------------------------------------------
	st_GameObjectJob* MakeGameObjectDamage(int64& AttackerID, en_GameObjectType AttackerType, en_SkillKinds SkillKind,  int32& SkillMinDamage, int32& SkillMaxDamage, bool IsBackAttack);	
	//-------------------------------------------------------------------------------
	// 기술 체력 회복 잡 생성 함수
	//-------------------------------------------------------------------------------
	st_GameObjectJob* MakeGameObjectJobHPHeal(CGameObject* Healer, bool IsCritical, int32 HealPoint, en_SkillType SkillType);
	//-------------------------------------------------------------------------------
	// 아이템 체력 회복 잡 생성 함수
	//-------------------------------------------------------------------------------
	st_GameObjectJob* MakeGameObjectJobItemHPHeal(en_SmallItemCategory HealItemCategory);
	//-------------------------------------------------------------------------------
	// 마력 회복 잡 생성 함수
	//-------------------------------------------------------------------------------
	st_GameObjectJob* MakeGameObjectJobMPHeal(CGameObject* Healer, int32 MPHealPoint);
	//-------------------------------------------------------------------------------
	// 장비 아이템 착용 잡 생성 함수
	//-------------------------------------------------------------------------------
	st_GameObjectJob* MakeGameObjectJobOnEquipment(CItem* EquipmentItem);
	//-------------------------------------------------------------------------------
	// 장비 아이템 착용해제 잡 생성 함수
	//-------------------------------------------------------------------------------
	st_GameObjectJob* MakeGameObjectJobOffEquipment(int8& EquipmentParts);
	//-------------------------------------------------------------------------------
	// 씨앗 심기 잡 생성 함수
	//-------------------------------------------------------------------------------
	st_GameObjectJob* MakeGameObjectJobSeedFarming(CGameObject* ReqSeedFarmingObject, int16 SeedItemCategory);
	//-------------------------------------------------------------------------------
	// 작물 성장 단계 확인 잡 생성 함수
	//-------------------------------------------------------------------------------
	st_GameObjectJob* MakeGameObjectJobPlantGrowthCheck(CGameObject* ReqPlantGrowthCheckObject, int64 PlantObjectID, int16 PlantObjectType);

	//------------------------------------------------------
	// 제작대 선택 잡 생성 함수
	//------------------------------------------------------
	st_GameObjectJob* MakeGameObjectJobCraftingTableSelect(CGameObject* CraftingTableObject, CGameObject* OwnerObject);	
	//---------------------------------------------------------
	// 제작대 선택 풀림 잡 생성 함수
	//---------------------------------------------------------
	st_GameObjectJob* MakeGameObjectJobCraftingTableNonSelect(CGameObject* CraftingTableObject, int64& CraftingTableObjectID, int16& CraftingTableObjectType);

	//-------------------------------------------------------------
	// 아이템 가방 저장 잡 생성 함수
	//-------------------------------------------------------------
	st_GameObjectJob* MakeGameObjectJobItemSave(CGameObject* Item);
	//-------------------------------------------------------------
	// 그룹 초대 잡 생성 함수
	//-------------------------------------------------------------
	st_GameObjectJob* MakeGameObjectJobPartyInvite(CGameObject* ReqPartyPlayer, int64 InvitePlayerID);
	//-------------------------------------------------------------
	// 그룹 초대 수락 잡 생성 함수
	//-------------------------------------------------------------
	st_GameObjectJob* MakeGameObjectJobPartyAccept(int64 PartyReqPlayerID, int64 PartyAcceptPlayerID);
	//-------------------------------------------------------------
	// 그룹 초대 거절 잡 생성 함수
	//-------------------------------------------------------------
	st_GameObjectJob* MakeGameObjectJobPartyReject(int64 PartyRejectPlayerID, int64 ReqPartyInvitePlayerID);
	//-------------------------------------------------------------
	// 그룹 탈퇴 잡 생성 함수
	//-------------------------------------------------------------
	st_GameObjectJob* MakeGameObjectJobPartyQuit(int64 PartyQuitPlayerID);
	//-------------------------------------------------------------
	// 그룹 추방 잡 생성 함수
	//-------------------------------------------------------------
	st_GameObjectJob* MakeGameObjectJobPartyBanish(CGameObject* ReqPartyBanishPlayer, int64 PartyBanishPlayerID);
	//-------------------------------------------------------------
	// 메뉴 선택 잡 생성 함수
	//-------------------------------------------------------------
	st_GameObjectJob* MakeGameObjectMenu(int8 MenuType, CGameObject* MenuReqGameObject);
	//-------------------------------------------------------------
	// 상호작용 잡 생성 함수
	//-------------------------------------------------------------
	st_GameObjectJob* MakeGameObjectJobInteraction(int16 InteractionType, int16 ObjectType, int64 ObjectID);
	

	//-------------------------------------------------------------
	// 게임서버 입장 요청 응답 패킷 조합
	//-------------------------------------------------------------
	CGameServerMessage* MakePacketResEnterGame(bool EnterGameSuccess, st_GameObjectInfo* ObjectInfo, Vector2Int* SpawnPosition, vector<st_TileInfo>* AroundTileInfos);
	//---------------------------------------------------------------------------------------------------------------------------------------------------------------------
	// 게임서버 일반 데미지 응답 패킷 조합 ( 데미지 가한 대상 ID, 데미지 입은 대상 ID, 데미지를 입힌 기술 타입, 데미지 양, 크리티컬 여부)
	//---------------------------------------------------------------------------------------------------------------------------------------------------------------------
	CGameServerMessage* MakePacketResDamage(int64 ObjectID, int64 TargetID, int8 SkillKind, en_ResourceName EffectType, int32 Damage, bool IsCritical);
	//----------------------------------------------------------------------------------------
	// 게임서버 채집 데미지 응답 패킷 조합
	//----------------------------------------------------------------------------------------
	CGameServerMessage* MakePacketResGatheringDamage(int64 TargetID);
	//-----------------------------------------------------------------------------------------
	// 게임서버 공격요청 응답 패킷 조합
	//-----------------------------------------------------------------------------------------
	CGameServerMessage* MakePacketResAttack(int64 ObjectID);
	//-----------------------------------------------------------------------------------------
	// 게임서버 기술 시전 시작 패킷 조합
	//-----------------------------------------------------------------------------------------
	CGameServerMessage* MakePacketSkillCastingStart(int64 ObjectId, bool SpellStart, en_SkillType SkillType = en_SkillType::SKILL_TYPE_NONE, float SpellTime = 0.0f);
	//-----------------------------------------------------------------------------------------
	// 게임서버 채집요청 응답 패킷 조합
	//-----------------------------------------------------------------------------------------
	CGameServerMessage* MakePacketResGathering(int64 ObjectID, bool GatheringStart, wstring GatheringName);
	//-------------------------------------------------------------------------------------------------------------------------
	// 게임서버 오른쪽 마우스 오브젝트 정보 요청 응답 패킷 조합
	//-------------------------------------------------------------------------------------------------------------------------
	CGameServerMessage* MakePacketResRightMousePositionObjectInfo(int64 ReqPlayerID, int64 FindObjectID, en_GameObjectType FindObjectType);
	//-------------------------------------------------------------------------------------------------------------------------
	// 게임서버 왼쪽 마우스 오브젝트 정보 요청 응답 패킷 조합
	//-------------------------------------------------------------------------------------------------------------------------
	CGameServerMessage* MakePacketResLeftMousePositionObjectInfo(int64 AccountId, int64 PreviousChoiceObjectId, int64 FindObjectId,
		map<en_SkillType, CSkill*> BufSkillInfo, map<en_SkillType, CSkill*> DeBufSkillInfo);
	//-------------------------------------------------------------------------------------------------------------------------
	// 게임서버 왼쪽 마우스 드래그 다중 오브젝트 선택 응답 패킷 조합
	//-------------------------------------------------------------------------------------------------------------------------
	CGameServerMessage* MakePacketResLeftMouseDragObjectsSelect(vector<int64> DragObjects);
	//-----------------------------------------------------------------------------------------------
	// 게임서버 제작대 제작 아이템 선택 응답 패킷 조합
	//-----------------------------------------------------------------------------------------------
	CGameServerMessage* MakePacketResCraftingTableCompleteItemSelect(int64 CraftingTableObjectID, en_SmallItemCategory SelectCompleteType, map<en_SmallItemCategory, CItem*> MaterialItems);
	//-----------------------------------------------------------------------------------------
	// 게임서버 애니메이션 출력 패킷 조합
	//-----------------------------------------------------------------------------------------
	CGameServerMessage* MakePacketResAnimationPlay(int64 ObjectId, en_GameObjectType ObjectType, en_AnimationType AnimationType);
	//-----------------------------------------------------------------------------------------
	// 게임서버 오브젝트 스탯 변경 패킷 조합
	//-----------------------------------------------------------------------------------------
	CGameServerMessage* MakePacketResChangeObjectStat(int64 ObjectId, st_StatInfo ChangeObjectStatInfo);
	//-----------------------------------------------------------------------------------------
	// 게임서버 오브젝트 상태 변경 패킷 조합
	//-----------------------------------------------------------------------------------------
	CGameServerMessage* MakePacketResChangeObjectState(int64 ObjectId, en_CreatureState ObjectState);	
	//-----------------------------------------------------------------------------------------
	// 게임서버 바라보는 방향 패킷 조합
	//-----------------------------------------------------------------------------------------
	CGameServerMessage* MakePacketResFaceDirection(int64 ObjectID, float FaceDirectionX, float FaceDirectionY);
	//-----------------------------------------------------------------------------------------
	// 게임서버 이동 요청 응답 패킷 조합
	//-----------------------------------------------------------------------------------------
	CGameServerMessage* MakePacketResMove(int64& ObjectID, Vector2& LookAtDirection, Vector2& MoveDirection, Vector2& Position, en_CreatureState State, int64 TargetID = -1);
	//------------------------------------------------------------------------------------------------------
	// 게임서버 이동 멈춤 요청 응답 패킷 조합
	//------------------------------------------------------------------------------------------------------
	CGameServerMessage* MakePacketResMoveStop(int64 ObjectId, float StopPositionX, float StopPositionY);	
	//------------------------------------------------------------------------------------------------------
	// 게임서버 캐릭터 공격 상태 전환 패킷 조합
	//------------------------------------------------------------------------------------------------------
	CGameServerMessage* MakePacketResToAttack(int64 ObjectID, int64 TargetID, float StopPositionX, float StopPositionY, en_CreatureState State);
	//-----------------------------------------------------------------------------------------
	// 게임서버 아이템 움직임 시작 패킷 조합
	//-----------------------------------------------------------------------------------------
	CGameServerMessage* MakePacketItemMove(st_GameObjectInfo ItemMoveObjectInfo);
	//-----------------------------------------------------------------------------------------
	// 게임서버 사각 충돌체 월드 소환 패킷 조합
	//-----------------------------------------------------------------------------------------
	CGameServerMessage* MakePacketRectCollisionSpawn(CRectCollision* RectCollision);
	//-----------------------------------------------------------------------------------------
	// 게임서버 오브젝트 스폰 패킷 조합 ( 단일 대상 )
	//-----------------------------------------------------------------------------------------
	CGameServerMessage* MakePacketResObjectSpawn(CGameObject* SpawnObject);
	//-----------------------------------------------------------------------------------------
	// 게임서버 오브젝트 스폰 패킷 조합 ( 복수 대상 )
	//-----------------------------------------------------------------------------------------
	CGameServerMessage* MakePacketResObjectSpawn(int32 ObjectInfosCount, vector<CGameObject*> ObjectInfos);
	//-------------------------------------------------------------------
	// 게임서버 오브젝트 디스폰 패킷 조합 ( 단일 대상 )
	//-------------------------------------------------------------------
	CGameServerMessage* MakePacketResObjectDeSpawn(int64 DeSpawnObjectID);
	//-----------------------------------------------------------------------------------------
	// 게임서버 오브젝트 디스폰 패킷 조합 ( 복수 대상 )
	//-----------------------------------------------------------------------------------------
	CGameServerMessage* MakePacketResObjectDeSpawn(int32 DeSpawnObjectCount, vector<CGameObject*> DeSpawnObjects);
	//-----------------------------------------------------------------------------------------
	// 게임서버 오브젝트 죽음 패킷 조합
	//-----------------------------------------------------------------------------------------
	CGameServerMessage* MakePacketObjectDie(int64 DieObjectId, en_CreatureState DieObjectState);
	//-----------------------------------------------------------------------------------------
	// 게임서버 메세지 ( 채팅, 시스템 ) 요청 패킷 조합
	//-----------------------------------------------------------------------------------------
	CGameServerMessage* MakePacketResChattingBoxMessage(en_MessageType MessageType, st_Color Color, wstring ChattingMessage);
	//-----------------------------------------------------------------------------------------
	// 게임서버 데미지 메세지 전송 패킷 조합
	//-----------------------------------------------------------------------------------------
	CGameServerMessage* MakePacketResDamageChattingBoxMessage(en_MessageType MessageType,
		wstring AttackerName, wstring TargetName, en_SkillType DamageSkilltype, int32 Damage);
	//-----------------------------------------------------------------------------------------
	// 게임서버 오브젝트 위치 싱크 맞추기 패킷 조합
	//-----------------------------------------------------------------------------------------
	CGameServerMessage* MakePacketResSyncPosition(int64 TargetObjectId, st_PositionInfo SyncPosition);
	//-----------------------------------------------------------------------------------------
	// 게임서버 퀵슬롯 초기화 패킷 조합
	//-----------------------------------------------------------------------------------------
	CGameServerMessage* MakePacketResQuickSlotInit(int8 QuickSlotBarIndex, int8 QuickSlotBarSlotIndex);
	//------------------------------------------------------------------------------------------------------------
	// 게임서버 스킬 특성 선택 응답 패킷 조합
	//------------------------------------------------------------------------------------------------------------
	CGameServerMessage* MakePacketResSelectSkillCharacteristic(bool IsSuccess, int8 SkillCharacteristicType, vector<CSkill*> PassiveSkills, vector<CSkill*> ActiveSkills);
	//-----------------------------------------------------------------------------------------
	// 게임서버 스킬 저장 패킷 조합
	//-----------------------------------------------------------------------------------------
	CGameServerMessage* MakePacketResSkillToSkillBox(int64 TargetObjectId, st_SkillInfo* SkillInfo);
	//-------------------------------------------------------------------
	// 게임서버 스킬 배우기 응답 패킷 조합
	//-------------------------------------------------------------------
	CGameServerMessage* MakePacketResSkillLearn(bool IsSkillLearn, vector<en_SkillType> LearnSkillTypes, int8 SkillMaxPoint, int8 SkillPoint);		
	//---------------------------------------------------------------------------------
	// 게임서버 강화효과, 약화효과 패킷 조합
	//---------------------------------------------------------------------------------
	CGameServerMessage* MakePacketBufDeBuf(int64 TargetObjectId, bool BufDeBuf, st_SkillInfo* SkillInfo);
	//---------------------------------------------------------------------------------
	// 게임서버 강화효과, 약화효과 끄기 패킷 조합
	//---------------------------------------------------------------------------------
	CGameServerMessage* MakePacketBufDeBufOff(int64 TargetObjectId, bool BufDeBuf, en_SkillType OffSkillType);
	//--------------------------------------------------------------------------------------------------------
	// 게임서버 연속기 스킬 켜기 패킷 조합
	//--------------------------------------------------------------------------------------------------------
	CGameServerMessage* MakePacketComboSkillOn(vector<st_QuickSlotBarPosition> ComboSkillQuickSlotPositions, en_SkillType ComboSkilltype);
	//--------------------------------------------------------------------------------------------------------
	// 게임서버 연속기 스킬 끄기 패킷 조합
	//--------------------------------------------------------------------------------------------------------
	CGameServerMessage* MakePacketComboSkillOff(vector<st_QuickSlotBarPosition> ComboSkillQuickSlotPositions, en_SkillType OffComboSkillType, en_SkillType RollBackSkillType);
	//--------------------------------------------------------------------------------------------------------
	// 게임서버 연속기 스킬 끄기 패킷 조합
	//--------------------------------------------------------------------------------------------------------
	CGameServerMessage* MakePacketComboSkillOff(en_SkillType OffSkillType, vector<st_QuickSlotOffInfo> ComboSkillQuickSlotPositions);
	//-------------------------------------------------------------------
	// 연속기 기술 생성
	//-------------------------------------------------------------------
	st_GameObjectJob* MakeGameObjectJobComboSkillCreate(en_SkillCharacteristic PreviousSkillCharacteristic, en_SkillType PreviousSkillType, en_SkillCharacteristic ComboSkillCharacteristic, en_SkillType ComboSkillType, int8 QuickSlotBarIndex, int8 QuickSlotBarSlotIndex);
	//-----------------------------------------------------------------------------------------
	// 게임서버 경험치 패킷 조합
	//-----------------------------------------------------------------------------------------
	CGameServerMessage* MakePacketExperience(int64 GainExp, int64 CurrentExp, int64 RequireExp, int64 TotalExp);
	//-----------------------------------------------------------------------------------------
	// 게임서버 스킬 취소 패킷 조합
	//-----------------------------------------------------------------------------------------
	CGameServerMessage* MakePacketSkillCastingCancel(int64 PlayerId);
	//-----------------------------------------------------------------------------------------
	// 게임서버 채집 취소 패킷 조합
	//-----------------------------------------------------------------------------------------
	CGameServerMessage* MakePacketGatheringCancel(int64 ObjectID);
	//-----------------------------------------------------------------------------------------
	// 게임서버 재사용대기시간 출력 패킷 조합
	//-----------------------------------------------------------------------------------------
	CGameServerMessage* MakePacketCoolTime(vector<st_QuickSlotBarPosition> QuickSlotBarPositions, float SkillCoolTimeSpeed, CSkill* QuickSlotSkill = nullptr, int32 CoolTime = 0);
	//-----------------------------------------------------------------------------------------
	// 게임서버 스킬 에러 메세지 생성 패킷 조합
	//-----------------------------------------------------------------------------------------
	CGameServerMessage* MakePacketSkillError(en_GlobalMessageType PersonalMessageType, const WCHAR* SkillName = nullptr, int16 SkillDistance = 0);
	//-----------------------------------------------------------------------------------------
	// 게임서버 일반 에러 메세지 생성 패킷 조합
	//-----------------------------------------------------------------------------------------
	CGameServerMessage* MakePacketCommonError(en_GlobalMessageType PersonalMessageType, const WCHAR* Name = nullptr);
	//------------------------------------------------------------
	// 게임 서버 상태이상 적용 패킷 조합
	//------------------------------------------------------------
	CGameServerMessage* MakePacketStatusAbnormal(int64 TargetId, float PositionX, float PositionY, st_SkillInfo* SatusAbnormalSkillInfo, bool SetStatusAbnormal, int64 StatusAbnormal);
	//-----------------------------------------------------------------------------------------
	// 게임서버 가방 아이템 업데이트
	//-----------------------------------------------------------------------------------------
	CGameServerMessage* MakePacketInventoryItemUpdate(int64 PlayerId, st_ItemInfo UpdateItemInfo);
	//-------------------------------------------------------------------------------------------------------------------------
	// 게임서버 아이템 가방 저장 요청 응답 패킷 조합
	//-------------------------------------------------------------------------------------------------------------------------
	CGameServerMessage* MakePacketResItemToInventory(int64 TargetObjectId, st_ItemInfo InventoryItem, bool IsExist, int16 ItemEach, bool ItemGainPrint = true);
	//------------------------------------------------------------------------------------------------------------------------------------------------------
	// 게임서버 가방 돈 저장 요청 응답 패킷 조합
	//------------------------------------------------------------------------------------------------------------------------------------------------------
	CGameServerMessage* MakePacketResMoneyToInventory(int64 TargetObjectID, int64 Coin, st_ItemInfo ItemInfo, int16 ItemEach);
	//-----------------------------------------------------------------------------------------------
	// 게임서버 제작대 재료 아이템 목록 패킷 조합
	//-----------------------------------------------------------------------------------------------
	CGameServerMessage* MakePacketResCraftingTableMaterialItemList(int64 CraftingTableObjectID, en_GameObjectType CraftingTableObjectType, en_SmallItemCategory SelectCompleteItemType, map<en_SmallItemCategory, CItem*> MaterialItems);
	//-----------------------------------------------------------------------------------------------
	// 게임서버 제작대 아이템 넣기 응답 패킷 조합
	//-----------------------------------------------------------------------------------------------
	CGameServerMessage* MakePacketResCraftingTableInput(int64 CraftingTableObjectID, map<en_SmallItemCategory, CItem*> MaterialItems);
	//-----------------------------------------------------------------------------------------------
	// 게임서버 제작대 제작 시작 응답 패킷 조합
	//-----------------------------------------------------------------------------------------------
	CGameServerMessage* MakePacketResCraftingStart(int64 CraftingTableObjectID, st_ItemInfo CraftingItemInfo);
	//-----------------------------------------------------------------------------------------------
	// 게임서버 제작대 제작 멈춤 응답 패킷 조합
	//-----------------------------------------------------------------------------------------------
	CGameServerMessage* MakePacketResCraftingStop(int64 CraftingTableObjectID, st_ItemInfo CraftingStopItemInfo);
	//-----------------------------------------------------------------------------------------------
	// 게임서버 제작대 제작 남은 시간 패킷 조합
	//-----------------------------------------------------------------------------------------------
	CGameServerMessage* MakePacketResCraftingTableCraftRemainTime(int64 CraftingTableObjectID, st_ItemInfo CraftingItemInfo);
	//-----------------------------------------------------------------------------------------------
	// 게임서버 제작대 제작 완료 아이템 목록 패킷 조합
	//-----------------------------------------------------------------------------------------------
	CGameServerMessage* MakePacketResCraftingTableCompleteItemList(int64 CraftingTableObjectID, en_GameObjectType CraftingTableObjectType, map<en_SmallItemCategory, CItem*> CompleteItems);
	//-----------------------------------------------------------------------------------------
	// 게임서버 장비 착용 응답 패킷 조합
	//-----------------------------------------------------------------------------------------
	CGameServerMessage* MakePacketOnEquipment(int64 PlayerId, st_ItemInfo& EquipmentItemInfo);
	//-----------------------------------------------------------------------------------------
	// 게임서버 장비 해제 응답 패킷 조합
	//-----------------------------------------------------------------------------------------
	CGameServerMessage* MakePacketOffEquipment(int64 PlayerID, en_EquipmentParts OffEquipmentParts);
	//-----------------------------------------------------------------------------------------
	// 게임서버 씨앗 심기 응답 패킷 조합
	//-----------------------------------------------------------------------------------------
	CGameServerMessage* MakePacketSeedFarming(st_ItemInfo SeedItem, int64 SeedObjectID);
	//-----------------------------------------------------------------------------------------
	// 게임서버 작물 성장 단계 확인 요청 응답 패킷 조합
	//-----------------------------------------------------------------------------------------
	CGameServerMessage* MakePacketPlantGrowthStep(int64 PlantObjectID, int8 PlantGrowthStep, float PlantGrowthRatio);
	//-----------------------------------------------------------------------------------------
	// 게임서버 그룹 초대 응답 패킷 조합
	//-----------------------------------------------------------------------------------------
	CGameServerMessage* MakePacketResPartyInvite(int64 ReqPartyPlayerObjectID, wstring ReqPartyPlayerName);
	//-----------------------------------------------------------------------------------------
	// 게임서버 그룹 초대 수락 응답 패킷 조합
	//-----------------------------------------------------------------------------------------
	CGameServerMessage* MakePacketResPartyAccept(vector<CPlayer*> PartyPlayerInfos);
	//-----------------------------------------------------------------------------------------
	// 게임서버 그룹 거절 응답 패킷 조합
	//-----------------------------------------------------------------------------------------
	CGameServerMessage* MakePacketResPartyReject(wstring PartyInviteRejectName);	
	//-----------------------------------------------------------------------------------------
	// 게임서버 그룹 탈퇴 응답 패킷 조합
	//-----------------------------------------------------------------------------------------
	CGameServerMessage* MakePacketResPartyQuit(bool IsAllQuit, int64 QuitPartyPlayerID);
	//-----------------------------------------------------------------------------------------
	// 게임서버 그룹 추방 응답 패킷 조합
	//-----------------------------------------------------------------------------------------
	CGameServerMessage* MakePacketResPartyBanish(int64 BanishPlayerID);
	//-----------------------------------------------------------------------------------------
	// 게임서버 그룹장 위임 응답 패킷 조합
	//-----------------------------------------------------------------------------------------
	CGameServerMessage* MakePacketResPartyLeaderMandate(int64 PerviousPartyLeaderObjectID, int64 NewPartyLeaderObjectID);
	//-----------------------------------------------------------------------------------------
	// 게임서버 메뉴 응답 패킷 조합
	//-----------------------------------------------------------------------------------------
	CGameServerMessage* MakePacketResMenu(int8 MenuType);
	//-----------------------------------------------------------------------------------------
	// 게임서버 루팅 상호작용 응답 패킷 조합
	//-----------------------------------------------------------------------------------------
	CGameServerMessage* MakePacketResInteractionRooting(int64 RootingObjectID, en_InteractionType InteractionType, vector<st_ItemInfo> RootingItems, int16 Coin);
	//-----------------------------------------------------------------------------------------
	// 게임서버 그룹장 위임 응답 패킷 조합
	//-----------------------------------------------------------------------------------------
	CGameServerMessage* MakePacketResRayCasting(int64 ObjectID, vector<st_RayCatingPosition>& RayCastingPositions);
	//-----------------------------------------------------------------------------------------
	// 게임서버 타일 정보 응답 패킷 조합
	//-----------------------------------------------------------------------------------------
	CGameServerMessage* MakePacketResTileInfo(vector<st_TileInfo> Tiles);

	//---------------------------------------------------
	// 게임서버 요청 실패 응답 패킷 조합
	//---------------------------------------------------
	CGameServerMessage* MakePacketReqCancel(en_GAME_SERVER_PACKET_TYPE PacketType);

	//---------------------------------------------------
	// 로그인 서버 로그아웃 요청 패킷 조합
	//---------------------------------------------------
	CGameServerMessage* MakePacketLoginServerLogOut(int64 AccountID);
	//---------------------------------------------------
	// 로그인 서버 로그인 상태 변경 패킷 조합
	//---------------------------------------------------
	CGameServerMessage* MakePacketLoginServerLoginStateChange(int64 AccountID, en_LoginState ChangeLoginState);
public:
	//-----------------------------------------------
	// Job 메모리풀
	//-----------------------------------------------
	CMemoryPoolTLS<st_GameServerJob>* _GameServerJobMemoryPool;

	//------------------------------------
	// TimerJob 메모리풀
	//------------------------------------
	CMemoryPoolTLS<st_TimerJob>* _TimerJobMemoryPool;

	//------------------------------------
	// Job 큐
	//------------------------------------
	CLockFreeQue<st_GameServerJob*> _GameServerUserDBThreadMessageQue;
	CLockFreeQue<st_GameServerJob*> _GameServerWorldDBThreadMessageQue;

	CLockFreeQue<st_GameServerJob*> _GameServerClientLeaveDBThreadMessageQue;

	//--------------------------------------
	// TimerJob 우선순위 큐
	//--------------------------------------
	CHeap<int64, st_TimerJob*>* _TimerHeapJob;

	int64 _LogicThreadFPS;
	// 네트워크 쓰레드 활성화된 횟수
	int64 _NetworkThreadWakeCount;
	// 네트워크 쓰레드 TPS
	int64 _NetworkThreadTPS;
	
	// UpdateThread 쓰레드 깨우기 이벤트
	HANDLE _UpdateThreadWakeEvent;
	// User DB 쓰레드 깨우기 이벤트
	HANDLE _UserDataBaseWakeEvent;
	// World DB 쓰레드 깨우기 이벤트
	HANDLE _WorldDataBaseWakeEvent;
	// ClientLeaveDB 쓰레드 깨우기 이벤트
	HANDLE _ClientLeaveDBThreadWakeEvent;
	// DB 쓰레드 활성화된 횟수
	int64 _UserDBThreadWakeCount;
	// DB 쓰레드 TPS
	int64 _UserDBThreadTPS;
	// 클라 정보 저장 쓰레드 TPS
	int64 _LeaveDBThreadTPS;

	// 타이머 잡 쓰레드 활성화된 횟수
	int64 _TimerJobThreadWakeCount;
	// 타이머 잡 쓰레드 TPS
	int64 _TimerJobThreadTPS;

	CGameServer();
	~CGameServer();

	//------------------------------------------
	// 게임 서버 시작
	//------------------------------------------
	void GameServerStart(const WCHAR* OpenIP, int32 Port);

	//------------------------------------------------------
	// 로그인 서버와 연결
	//------------------------------------------------------
	void LoginServerConnect(const WCHAR* ConnectIP, int32 Port);

	//--------------------------------------------------
	// 새로운 클라이언트 접속 
	//--------------------------------------------------
	virtual void OnClientJoin(int64 SessionID) override;
	//--------------------------------------------------------------
	// 네트워크 패킷 처리
	//--------------------------------------------------------------
	virtual void OnRecv(int64 SessionID, CMessage* Packet) override;
	//--------------------------------------------------------------
	// 클라이언트 떠남
	//--------------------------------------------------------------
	virtual void OnClientLeave(st_Session* LeaveSession) override;
	virtual bool OnConnectionRequest(const wchar_t ClientIP, int32 Port) override;

	//--------------------------------------------------------------
	// 시야범위 기준으로 패킷 전송
	//--------------------------------------------------------------
	void SendPacketFieldOfView(vector<st_FieldOfViewInfo> FieldOfViewObject, CMessage* Message);
	//--------------------------------------------------------------
	// 오브젝트를 기준으로 시야뷰 안에 있는 플레이어 대상으로 패킷 전송
	//--------------------------------------------------------------
	void SendPacketFieldOfView(CGameObject* Object, CMessage* Message);	

	//-------------------------------------------
	// 핑 타이머 잡 생성
	//-------------------------------------------
	void PingTimerCreate(st_Session* PingSession);
};
