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
	// �α��� ������ ����� ����
	SOCKET _LoginServerSock;
	// ���� �����ͺ��̽� ������
	HANDLE _UserDataBaseThread;
	// ���� �����ͺ��̽� ������
	HANDLE _WorldDataBaseThread;
	// Ŭ���̾�Ʈ ����� �������� ������
	HANDLE _ClientLeaveSaveThread;
	// Ÿ�̸��� ������
	HANDLE _TimerJobThread;
	// ���� ������
	HANDLE _LogicThread;

	// Ÿ�̸��� ������ ����� �̺�Ʈ
	HANDLE _TimerThreadWakeEvent;

	// WorkerThread ����� ����
	bool _NetworkThreadEnd;
	// UpdateThrad ����� ����
	bool _UpdateThreadEnd;
	// User DataBaseThread ����� ����
	bool _UserDataBaseThreadEnd;
	// World DataBaseThread ����� ����
	bool _WorldDataBaseThreadEnd;
	// ClientLeaveSaveThread ����� ����
	bool _ClientLeaveSaveDBThreadEnd;

	// TimerJobThread ����� ����
	bool _TimerJobThreadEnd;
	// LogicThread ����� ����
	bool _LogicThreadEnd;

	// TimerJobThread ���� Lock
	SRWLOCK _TimerJobLock;

	// �Ϸ� ����
	CDay* _Day;

	//----------------------------------------------------------
	// Update ������
	//----------------------------------------------------------
	static unsigned __stdcall UpdateThreadProc(void* Argument);
	//----------------------------------------------------------
	// ���� �����ͺ��̽� ������ ( ���� ������ ���̽� �۾� ó�� )
	//----------------------------------------------------------
	static unsigned __stdcall UserDataBaseThreadProc(void* Argument);
	//----------------------------------------------------------
	// ���� �����ͺ��̽� ������ ( ���� ������ ���̽� �۾� ó�� )
	//----------------------------------------------------------
	static unsigned __stdcall WorldDataBaseThreadProc(void* Argument);
	//-------------------------------------------------------------
	// Ŭ���̾�Ʈ ���� ����� Player�� ������ DB�� ������ ������
	//-------------------------------------------------------------
	static unsigned __stdcall ClientLeaveThreadProc(void* Argument);

	//----------------------------------------------------------
	// Ÿ�̸� �� ������ ( Ÿ�̸� �� ó�� )
	//----------------------------------------------------------
	static unsigned __stdcall TimerJobThreadProc(void* Argument);
	//--------------------------------------------------------
	// ����ó�� ������ 
	//--------------------------------------------------------
	static unsigned __stdcall LogicThreadProc(void* Argument);

	//---------------------------------
	// ĳ���� ���� �� �⺻ ����
	//---------------------------------
	void PlayerDefaultSetting(int64& AccountId, st_GameObjectInfo& NewCharacterInfo, int8& CharacterCreateSlotIndex);

	//------------------------------------
	// Ŭ�� ���� �⺻ ���� ����
	//------------------------------------
	void CreateNewClient(int64 SessionId);
	//------------------------------------
	// Ŭ�� ���� 
	//------------------------------------
	void DeleteClient(st_Session* Session);

	//-------------------------------------------------------
	// �α��� ������ ��Ŷ ����
	//-------------------------------------------------------
	void SendPacketToLoginServer(CGameServerMessage* Message);
	//--------------------------------------------------
	// ��Ʈ��ũ ��Ŷ ó��
	//--------------------------------------------------
	void PacketProc(int64 SessionID, CMessage* Message);

	//----------------------------------------------------------------
	// ��Ʈ��ũ ��Ŷó�� �Լ�
	//----------------------------------------------------------------

	//----------------------------------------------------------
	// �α��� ��û ó��
	//----------------------------------------------------------
	void PacketProcReqLogin(int64 SessionID, CMessage* Message);
	//-------------------------------------------------------------------
	// ĳ���� ���� ��û ó��
	//-------------------------------------------------------------------
	void PacketProcReqCreateCharacter(int64 SessionID, CMessage* Message);
	//-------------------------------------------------------------
	// ���� ���� ��û ó��
	//-------------------------------------------------------------
	void PacketProcReqEnterGame(int64 SessionID, CMessage* Message);
	//-------------------------------------------------------------
	// ĳ���� ���� ��û ó��
	//-------------------------------------------------------------
	void PacketProcReqCharacterInfo(int64 SessionID, CMessage* Message);

	//---------------------------------------------------------
	// �̵� ��û ó��
	//---------------------------------------------------------
	void PacketProcReqMove(int64 SessionID, CMessage* Message);
	//------------------------------------------------------------
	// �̵� ���� ó��
	//------------------------------------------------------------
	void PacketProcReqMoveStop(int64 SessionID, CMessage* Message);
	//---------------------------------------------------------------
	// ���� ��û ó��
	//---------------------------------------------------------------
	void PacketProcReqMelee(int64 SessionID, CMessage* Message);
	//---------------------------------------------------------
	// ���� ��û ó��
	//---------------------------------------------------------
	void PacketProcReqMagic(int64 SessionId, CMessage* Message);
	//---------------------------------------------------------
	// ���� ��û ��� ó��
	//---------------------------------------------------------
	void PacketProcReqMagicCancel(int64 SessionId, CMessage* Message);
	//----------------------------------------------------------------------------
	// ä�� ��û ó��
	//----------------------------------------------------------------------------
	void PacketProcReqGathering(int64 SessionID, CMessage* Message);
	//---------------------------------------------------------
	// ä�� ��û ��� ó��
	//---------------------------------------------------------
	void PacketProcReqGatheringCancel(int64 SessionID, CMessage* Message);
	//----------------------------------------------------------------------------
	// ���� ���콺 Ŭ�� ��ġ ������Ʈ ���� ��û ó��
	//----------------------------------------------------------------------------
	void PacketProcReqLeftMouseObjectInfo(int64 SessionId, CMessage* Message);	
	//----------------------------------------------------------------------------
	// ���� ���콺 Ŭ�� UI ������Ʈ ���� ��û ó��
	//----------------------------------------------------------------------------
	void PacketProcReqLeftMouseUIObjectInfo(int64 SessionID, CMessage* Message);
	//----------------------------------------------------------------------------
	// ������ ���콺 Ŭ�� ��ġ ������Ʈ ���� ��û ó��
	//----------------------------------------------------------------------------
	void PacketProcReqRightMouseObjectInfo(int64 SessionId, CMessage* Message);	
	//--------------------------------------------------------------------------
	// ���۴� ���� Ǯ�� ��û ó��
	//--------------------------------------------------------------------------
	void PacketProcReqCraftingTableNonSelect(int64 SessionID, CMessage* Message);	
	//--------------------------------------------------------------------
	// ä�� �޼��� ��û ó��
	//--------------------------------------------------------------------
	void PacketProcReqChattingMessage(int64 SessionId, CMessage* Message);
	//------------------------------------------------------------
	// ������ ���� ���� ��û ó��
	//------------------------------------------------------------
	void PacketProcReqItemSelect(int64 SessionId, CMessage* Message);
	//------------------------------------------------------------
	// ������ ���� ��û ó��
	//------------------------------------------------------------
	void PacketProcReqItemPlace(int64 SessionId, CMessage* Message);
	//------------------------------------------------------------
	// ������ ȸ�� ��û ó��
	//------------------------------------------------------------
	void PacketProcReqItemRotate(int64 SessionID, CMessage* Message);
	//-----------------------------------------------------------------------------
	// ��ų Ư�� ���� ��û ó��
	//-----------------------------------------------------------------------------
	void PacketProcReqSelectSkillCharacteristic(int64 SessionID, CMessage* Message);
	//---------------------------------------------------------------
	// ��ų ���� ��û ó��
	//---------------------------------------------------------------
	void PacketProcReqLearnSkill(int64 SessionID, CMessage* Message);
	//------------------------------------------------------------------
	// ������ ���� ��û ó��
	//------------------------------------------------------------------
	void PacketProcReqQuickSlotSave(int64 SessionId, CMessage* Message);
	//------------------------------------------------------------------
	// ������ ���� ��û ó��
	//------------------------------------------------------------------
	void PacketProcReqQuickSlotSwap(int64 SessionId, CMessage* Message);
	//------------------------------------------------------------------
	// ������ �ʱ�ȭ ��û ó��
	//------------------------------------------------------------------
	void PacketProcReqQuickSlotInit(int64 SessionId, CMessage* Message);
	//------------------------------------------------------------------
	// ������ ���� ��û ó��
	//------------------------------------------------------------------
	void PacketProcReqCraftingConfirm(int64 SessionId, CMessage* Message);
	//------------------------------------------------------------------
	// ������ ��� ��û ó��
	//------------------------------------------------------------------
	void PacketProcReqItemUse(int64 SessionId, CMessage* Message);
	//------------------------------------------------------------------
	// ��� ������ ���� ��û ó��
	//------------------------------------------------------------------
	void PacketProcReqOffEquipment(int64 SessionID, CMessage* Message);
	//------------------------------------------------------------------
	// ������ ���� UI ��û ó��
	//------------------------------------------------------------------
	void PacketProcReqUIMenuTileBuy(int64 SessionID, CMessage* Message);
	//------------------------------------------------------------------
	// ������ ���� ��û ó��
	//------------------------------------------------------------------
	void PacketProcReqTileBuy(int64 SessionID, CMessage* Message);
	//------------------------------------------------------------------
	// ���� �ɱ� ��û ó��
	//------------------------------------------------------------------
	void PacketProcReqSeedFarming(int64 SessionID, CMessage* Message);
	//------------------------------------------------------------------
	// �۹� ���� �ܰ� Ȯ�� ��û ó��
	//------------------------------------------------------------------
	void PacketProcReqPlantGrowthCheck(int64 SessionID, CMessage* Message);
	//------------------------------------------------------------------
	// ������ �ݱ� ��û ó��
	//------------------------------------------------------------------
	void PacketProcReqItemLooting(int64 SessionId, CMessage* Message);
	//------------------------------------------------------------------
	// ������ ������ ��û ó��
	//------------------------------------------------------------------
	void PacketProcReqItemDrop(int64 SessionID, CMessage* Message);
	//--------------------------------------------------------------------------
	// ���۴� ��� ������ �ֱ� ��û ó��
	//---------------------------------------------------------------------------
	void PacketProcReqCraftingTableItemAdd(int64 SessionID, CMessage* Message);
	//----------------------------------------------------------------------------
	// ���۴� ��� ������ ���� ��û ó��
	//----------------------------------------------------------------------------
	void PacketProcReqCraftingTableMaterialItemSubtract(int64 SessionID, CMessage* Message);
	//----------------------------------------------------------------------------
	// ���۴� �ϼ� ������ ���� ��û ó��
	//----------------------------------------------------------------------------
	void PacketProcReqCraftingTableCompleteItemSubtract(int64 SessionID, CMessage* Message);
	//--------------------------------------------------------------------------
	// ���۴� ���� ��û ó��
	//--------------------------------------------------------------------------
	void PacketProcReqCraftingTableCraftingStart(int64 SessionID, CMessage* Message);
	//--------------------------------------------------------------------------
	// ���۴� ���� ���� ó��
	//--------------------------------------------------------------------------
	void PacketProcReqCraftingTableCraftingStop(int64 SessionID, CMessage* Message);
	//--------------------------------------------------------
	// �� ��Ŷ ó��
	//--------------------------------------------------------
	void PacketProcReqPong(int64 SessionID, CMessage* Message);

	//-----------------------------------------------------------------------------------------------
	// DB ��û ó�� �Լ�
	//-----------------------------------------------------------------------------------------------

	//-------------------------------------------------------------------
	// AccountID�� AccountDB�� �ִ��� üũ
	//-------------------------------------------------------------------
	void PacketProcReqDBAccountCheck(CMessage* Message);
	//-------------------------------------------------------------------------------
	// ������û�� ĳ���Ͱ� DB�� �ִ��� Ȯ�� �� ĳ���� ����
	//-------------------------------------------------------------------------------
	void PacketProcReqDBCreateCharacterNameCheck(CMessage* Message);		
	//-----------------------------------------------------------------------
	// ���� ���� ���ӽ� ĳ���� ������ DB���� �����ͼ� Ŭ�� ����
	//-----------------------------------------------------------------------
	void PacketProcReqDBCharacterInfoSend(CMessage* Message);
	//--------------------------------------------------------------------
	// ���� ����� �÷��̾� ���� DB�� ���
	//--------------------------------------------------------------------
	void PacketProcReqDBLeavePlayerInfoSave(CGameServerMessage* Message);

	//------------------------------------------------------------------
	// Ÿ�̸� �� ��û ó�� �Լ�
	//------------------------------------------------------------------	
	
	//----------------------------------------------------------------
	// �� ó��
	//----------------------------------------------------------------
	void PacketProcTimerPing(int64 SessionId);

	//--------------------------------------
	// ���ӿ�����Ʈ �� ���� �Լ�
	//--------------------------------------		
	
	//------------------------------------------------------------------------------
	// �÷��̾� ä�� ���� �� ���� �Լ�
	//------------------------------------------------------------------------------
	st_GameObjectJob* MakeGameObjectJobLeaveChannelPlayer(CGameObject* LeavePlayerObject, int32* PlayerIndexes);
	//-------------------------------------------------
	// �Ϲ� ���� �ѱ� �� ���� �Լ�
	//-------------------------------------------------
	st_GameObjectJob* MakeGameObjectJobDefaultAttack();		
	//-------------------------------------------------
	// ��ų Ư�� ���� �� ���� �Լ�
	//-------------------------------------------------
	st_GameObjectJob* MakeGameObjectJobSelectSkillCharacteristic(int8 SelectCharacteristicIndex, int8 SelectChracteristicType);		
	//------------------------------------------------------------------------------------------------------------------------------------
	// ��ų ���� �� ���� �Լ� 
	//------------------------------------------------------------------------------------------------------------------------------------
	st_GameObjectJob* MakeGameObjectJobSkillLearn(bool IsSkillLearn, int8 LearnSkillCharacterIndex, int8 LearnSkillCharacteristicType, int16 LearnSkillType);
	//-------------------------------------------------
	// ���� ��� ó�� �� ���� �Լ�
	//-------------------------------------------------
	st_GameObjectJob* MakeGameObjectJobMeleeAttack(int8 MeleeCharacteristicType, int16 MeleeSkillType);
	//------------------------------------------------
	// ���� ���� �� ���� �Լ�
	//------------------------------------------------
	st_GameObjectJob* MakeGameObjectJobSpellStart(int8 SpellCharacteristicType, int16 StartSpellSkilltype);
	//------------------------------------------------
	// ���� ���� ��� �� ���� �Լ�
	//------------------------------------------------
	st_GameObjectJob* MakeGameObjectJobSpellCancel();
	//------------------------------------------------
	// ä�� ���� �� ���� �Լ�
	//------------------------------------------------
	st_GameObjectJob* MakeGameObjectJobGatheringStart(CGameObject* GatheringObject);
	//---------------------------------------------------
	// ä�� ��� �� ���� �Լ�
	//---------------------------------------------------
	st_GameObjectJob* MakeGameObjectJobGatheringCancel();	
	
	//-----------------------------------------------------------------------------------
	// ������ ������ �� ���� �Լ�
	//-----------------------------------------------------------------------------------
	st_GameObjectJob* MakeGameObjectJobItemDrop(int16 DropItemType, int32 DropItemCount);		
	//---------------------------------------------------------
	// ���۴� ���� ���� �� ���� �Լ�
	//---------------------------------------------------------
	st_GameObjectJob* MakeGameObjectJobCraftingTableStart(CGameObject* CraftingStartObject, en_SmallItemCategory CraftingCompleteItemType, int16 CraftingCount);
	//---------------------------------------------------------------------------------------------------
	// ���۴� ������ ��� �ֱ� �� ���� �Լ�
	//---------------------------------------------------------------------------------------------------
	st_GameObjectJob* MakeGameObjectJobCraftingTableItemAdd(CGameObject* CraftingTableItemAddObject, int16 AddItemSmallCategory, int16 AddItemCount);
	//---------------------------------------------------------------------------------------------------
	// ���۴� ������ ��� ���� �� ���� �Լ�
	//---------------------------------------------------------------------------------------------------
	st_GameObjectJob* MakeGameObjectJobCraftingTableMaterialItemSubtract(CGameObject* CraftingTableItemSubtractObject, int16 SubtractItemSmallCategory, int16 SubtractItemCount);
	//---------------------------------------------------------------------------------------------------
	// ���۴� �ϼ� ������ ���� �� ���� �Լ�
	//---------------------------------------------------------------------------------------------------
	st_GameObjectJob* MakeGameObjectJobCraftingTableCompleteItemSubtract(CGameObject* CraftingTableItemSubtractObject, int16 SubtractItemSmallCategory, int16 SubtractItemCount);


	//---------------------------------------------------------
	// ���۴� ���� ���� �� ���� �Լ�
	//---------------------------------------------------------
	st_GameObjectJob* MakeGameObjectJobCraftingTableCancel(CGameObject* CraftingStopObject);

	//--------------------------------------
	// ��Ŷ���� �Լ�		
	//--------------------------------------

	//--------------------------------------
	// Ŭ���̾�Ʈ ���� ���� ��Ŷ ����
	//--------------------------------------
	CGameServerMessage* MakePacketResClientConnected();
	//---------------------------------------------------------------------------------------
	// �α��� ��û ���� ��Ŷ ����
	//---------------------------------------------------------------------------------------
	CGameServerMessage* MakePacketResLogin(bool& Status, int8& PlayerCount, int32* MyPlayerIndexes);
	//--------------------------------------------------------------------------------------------------
	// ĳ���� ���� ��û ���� ��Ŷ ����
	//--------------------------------------------------------------------------------------------------
	CGameServerMessage* MakePacketResCreateCharacter(bool IsSuccess, st_GameObjectInfo& CreateCharacterObjectInfo);				
	//---------------------------------------------------------------------------------------------
	// ���Ӽ��� ���� ������ ���� ��û ���� ��Ŷ ����
	//---------------------------------------------------------------------------------------------
	CGameServerMessage* MakePacketResSelectItem(int64 AccountId, int64 ObjectId, CItem* SelectItem);
	//---------------------------------------------------------------------------------------------
	// ���Ӽ��� ������ ȸ�� ��û ���� ��Ŷ ����
	//---------------------------------------------------------------------------------------------
	CGameServerMessage* MakePacketResItemRotate(int64 AccountID, int64 PlayerID);
	//---------------------------------------------------------------------------------------------
	// ���Ӽ��� ���� ������ ���� ��û ���� ��Ŷ ����
	//---------------------------------------------------------------------------------------------
	CGameServerMessage* MakePacketResPlaceItem(int64 AccountId, int64 ObjectId, CItem* PlaceItem, CItem* OverlapItem);	
	//-----------------------------------------------------------------------------------------
	// ���Ӽ��� ���� ������ ��� ��û ���� ��Ŷ ����
	//-----------------------------------------------------------------------------------------
	CGameServerMessage* MakePacketInventoryItemUse(int64 PlayerId, st_ItemInfo& UseItemInfo);	
	//-----------------------------------------------------------------------------------------
	// ���Ӽ��� ������ ��� ��û ���� ��Ŷ ����
	//-----------------------------------------------------------------------------------------
	CGameServerMessage* MakePacketResQuickSlotBarSlotSave(st_QuickSlotBarSlotInfo QuickSlotBarSlotInfo);
	//-----------------------------------------------------------------------------------------
	// ���Ӽ��� ������ ���� ��Ŷ ����
	//-----------------------------------------------------------------------------------------
	CGameServerMessage* MakePacketQuickSlotCreate(int8 QuickSlotBarSize, int8 QuickSlotBarSlotSize, vector<st_QuickSlotBarSlotInfo> QuickslotBarSlotInfos);
	//-----------------------------------------------------------------------------------------
	// ���Ӽ��� ������ ���� ��û ���� ��Ŷ ���� 
	//-----------------------------------------------------------------------------------------
	CGameServerMessage* MakePacketResQuickSlotSwap(st_QuickSlotBarSlotInfo SwapAQuickSlotInfo, st_QuickSlotBarSlotInfo SwapBQuickSlotInfo);	
	//-----------------------------------------------------------------------------------------
	// ���Ӽ��� ������ ��� ��Ŷ ����
	//-----------------------------------------------------------------------------------------
	CGameServerMessage* MakePacketCraftingList(int64 AccountId, int64 PlayerId, vector<st_CraftingItemCategory> CraftingItemList);		
	//----------------------------------------------------------------------------------------------	
	// ���Ӽ��� �޴� UI Ÿ�� ���� ���� ��Ŷ ����
	//----------------------------------------------------------------------------------------------
	CGameServerMessage* MakePacketResMenuTileBuy(vector<st_TileMapInfo> AroundMapTile);
	//-----------------------------------------------------------------------------------------
	// ���Ӽ��� Ÿ�� ���� ���� ��Ŷ ����
	//-----------------------------------------------------------------------------------------
	CGameServerMessage* MakePacketResTileBuy(st_TileMapInfo TileMapInfo);
	
	//-------------------------------------------------
	// ���Ӽ��� �� ��Ŷ ����
	//-------------------------------------------------
	CGameServerMessage* MakePacketPing();		
public:
	CItem* NewItemCrate(st_ItemInfo& NewItemInfo);

	st_GameObjectJob* MakeGameObjectJobObjectDeSpawnObjectChannel(CGameObject* DeSpawnChannelObject);

	//-------------------------------------------------------------------------------
	// �÷��̾� ä�� ���� �� ���� �Լ�
	//-------------------------------------------------------------------------------
	st_GameObjectJob* MakeGameObjectJobPlayerEnterChannel(CGameObject* EnterChannelObject);
	//-------------------------------------------------------------------------------
	// �Ϲ� ������Ʈ ä�� ���� �� ���� �Լ�
	//-------------------------------------------------------------------------------
	st_GameObjectJob* MakeGameObjectJobObjectEnterChannel(CGameObject* EnterChannelObject);
	//-------------------------------------------------------------------------------
	// ä�ο��� �Ϲ� ������Ʈ ã�� �� ���� �Լ�
	//-------------------------------------------------------------------------------
	st_GameObjectJob* MakeGameObjectJobFindObjectChannel(CGameObject* ReqPlayer, int64& FindObjectID, int16& FindObjectType);
	//-----------------------------------------------------------------------------------------------------------------------------
	// ���Ӽ��� ���۴� ����Ǯ�� ��û ���� ��Ŷ ����
	//-----------------------------------------------------------------------------------------------------------------------------
	CGameServerMessage* MakePacketResCraftingTableNonSelect(int64 CraftingTableObjectID, en_GameObjectType CraftingTableObjectType);
	//-------------------------------------------------------------------------------
	// ä�ο��� ���۴� ���� ������ ���� �� ���� �Լ�
	//-------------------------------------------------------------------------------
	st_GameObjectJob* MakeGameObjectJobFindCraftingTableSelectItem(CGameObject* ReqPlayer, int64& FindCraftingTableObjectID, int16& FindCraftingTableObjectType, int16& LeftMouseItemCategory);
	//-------------------------------------------------------------------------------
	// ä�ο��� ���۴� ���� ������ ���� �� ���� �Լ�
	//-------------------------------------------------------------------------------
	st_GameObjectJob* MakeGameObjectJobRightMouseObjectInfo(CGameObject* ReqPlayer, int64& FindObjectID, int16& FindObjectType);
	//-------------------------------------------------------------------------------
	// �÷��̾� ���� ������Ʈ ä�� ���� �� ���� �Լ�
	//-------------------------------------------------------------------------------
	st_GameObjectJob* MakeGameObjectJobLeaveChannel(CGameObject* LeaveChannelObject);
	//--------------------------------------------------------------------------------------------------------------------------
	// ���ӱ� ���� �ѱ� �� ���� �Լ�
	//--------------------------------------------------------------------------------------------------------------------------
	st_GameObjectJob* MakeGameObjectJobComboSkillCreate(CSkill* ComboSkill);
	//------------------------------------------------
	// ���ӱ� ���� ���� �� ���� �Լ�
	//------------------------------------------------
	st_GameObjectJob* MakeGameObjectJobComboSkillOff();
	//-------------------------------------------------------------------------------
	// ������ ó�� �� ���� �Լ�
	//-------------------------------------------------------------------------------
	st_GameObjectJob* MakeGameObjectDamage(CGameObject* Attacker, bool IsCritical, int32 Damage, en_SkillType SkillType);
	//-------------------------------------------------------------------------------
	// ��� ü�� ȸ�� �� ���� �Լ�
	//-------------------------------------------------------------------------------
	st_GameObjectJob* MakeGameObjectJobHPHeal(CGameObject* Healer, bool IsCritical, int32 HealPoint, en_SkillType SkillType);
	//-------------------------------------------------------------------------------
	// ������ ü�� ȸ�� �� ���� �Լ�
	//-------------------------------------------------------------------------------
	st_GameObjectJob* MakeGameObjectJobItemHPHeal(en_SmallItemCategory HealItemCategory);
	//-------------------------------------------------------------------------------
	// ���� ȸ�� �� ���� �Լ�
	//-------------------------------------------------------------------------------
	st_GameObjectJob* MakeGameObjectJobMPHeal(CGameObject* Healer, int32 MPHealPoint);
	//-------------------------------------------------------------------------------
	// ��� ������ ���� �� ���� �Լ�
	//-------------------------------------------------------------------------------
	st_GameObjectJob* MakeGameObjectJobOnEquipment(CItem* EquipmentItem);
	//-------------------------------------------------------------------------------
	// ��� ������ �������� �� ���� �Լ�
	//-------------------------------------------------------------------------------
	st_GameObjectJob* MakeGameObjectJobOffEquipment(int8& EquipmentParts);
	//-------------------------------------------------------------------------------
	// ���� �ɱ� �� ���� �Լ�
	//-------------------------------------------------------------------------------
	st_GameObjectJob* MakeGameObjectJobSeedFarming(CGameObject* ReqSeedFarmingObject, int16 SeedItemCategory);
	//-------------------------------------------------------------------------------
	// �۹� ���� �ܰ� Ȯ�� �� ���� �Լ�
	//-------------------------------------------------------------------------------
	st_GameObjectJob* MakeGameObjectJobPlantGrowthCheck(CGameObject* ReqPlantGrowthCheckObject, int64 PlantObjectID, int16 PlantObjectType);

	//------------------------------------------------------
	// ���۴� ���� �� ���� �Լ�
	//------------------------------------------------------
	st_GameObjectJob* MakeGameObjectJobCraftingTableSelect(CGameObject* CraftingTableObject, CGameObject* OwnerObject);	
	//---------------------------------------------------------
	// ���۴� ���� Ǯ�� �� ���� �Լ�
	//---------------------------------------------------------
	st_GameObjectJob* MakeGameObjectJobCraftingTableNonSelect(CGameObject* CraftingTableObject, int64& CraftingTableObjectID, int16& CraftingTableObjectType);

	//-------------------------------------------------------------
	// ������ ���� ���� �� ���� �Լ�
	//-------------------------------------------------------------
	st_GameObjectJob* MakeGameObjectJobItemSave(CGameObject* Item);

	//-------------------------------------------------------------
	// ���Ӽ��� ���� ��û ���� ��Ŷ ����
	//-------------------------------------------------------------
	CGameServerMessage* MakePacketResEnterGame(bool EnterGameSuccess, st_GameObjectInfo* ObjectInfo, st_Vector2Int* SpawnPosition);
	//----------------------------------------------------------------------------------------
	// ���Ӽ��� �Ϲ� ������ ���� ��Ŷ ����
	//----------------------------------------------------------------------------------------
	CGameServerMessage* MakePacketResDamage(int64 ObjectID, int64 TargetID, en_SkillType SkillType, int32 Damage, bool IsCritical);
	//----------------------------------------------------------------------------------------
	// ���Ӽ��� ä�� ������ ���� ��Ŷ ����
	//----------------------------------------------------------------------------------------
	CGameServerMessage* MakePacketResGatheringDamage(int64 TargetID);
	//-----------------------------------------------------------------------------------------
	// ���Ӽ��� ���ݿ�û ���� ��Ŷ ����
	//-----------------------------------------------------------------------------------------
	CGameServerMessage* MakePacketResAttack(int64 PlayerDBId, int64 TargetId, en_SkillType SkillType, int32 Damage, bool IsCritical);
	//-----------------------------------------------------------------------------------------
	// ���Ӽ��� ������û ���� ��Ŷ ����
	//-----------------------------------------------------------------------------------------
	CGameServerMessage* MakePacketResMagic(int64 ObjectId, bool SpellStart, en_SkillType SkillType = en_SkillType::SKILL_TYPE_NONE, float SpellTime = 0.0f);
	//-----------------------------------------------------------------------------------------
	// ���Ӽ��� ä����û ���� ��Ŷ ����
	//-----------------------------------------------------------------------------------------
	CGameServerMessage* MakePacketResGathering(int64 ObjectID, bool GatheringStart, wstring GatheringName);
	//-------------------------------------------------------------------------------------------------------------------------
	// ���Ӽ��� ������ ���콺 ������Ʈ ���� ��û ���� ��Ŷ ����
	//-------------------------------------------------------------------------------------------------------------------------
	CGameServerMessage* MakePacketResRightMousePositionObjectInfo(int64 ReqPlayerID, int64 FindObjectID, en_GameObjectType FindObjectType);
	//-------------------------------------------------------------------------------------------------------------------------
	// ���Ӽ��� ���� ���콺 ������Ʈ ���� ��û ���� ��Ŷ ����
	//-------------------------------------------------------------------------------------------------------------------------
	CGameServerMessage* MakePacketResLeftMousePositionObjectInfo(int64 AccountId, int64 PreviousChoiceObjectId, int64 FindObjectId,
		map<en_SkillType, CSkill*> BufSkillInfo, map<en_SkillType, CSkill*> DeBufSkillInfo);
	//-----------------------------------------------------------------------------------------------
	// ���Ӽ��� ���۴� ���� ������ ���� ���� ��Ŷ ����
	//-----------------------------------------------------------------------------------------------
	CGameServerMessage* MakePacketResCraftingTableCompleteItemSelect(int64 CraftingTableObjectID, en_SmallItemCategory SelectCompleteType, map<en_SmallItemCategory, CItem*> MaterialItems);
	//-----------------------------------------------------------------------------------------
	// ���Ӽ��� �ִϸ��̼� ��� ��Ŷ ����
	//-----------------------------------------------------------------------------------------
	CGameServerMessage* MakePacketResAnimationPlay(int64 ObjectId, en_MoveDir Dir, wstring AnimationName);
	//-----------------------------------------------------------------------------------------
	// ���Ӽ��� ������Ʈ ���� ���� ��Ŷ ����
	//-----------------------------------------------------------------------------------------
	CGameServerMessage* MakePacketResChangeObjectStat(int64 ObjectId, st_StatInfo ChangeObjectStatInfo);
	//-----------------------------------------------------------------------------------------
	// ���Ӽ��� ������Ʈ ���� ���� ��Ŷ ����
	//-----------------------------------------------------------------------------------------
	CGameServerMessage* MakePacketResChangeObjectState(int64 ObjectId, en_MoveDir Direction, en_GameObjectType ObjectType, en_CreatureState ObjectState);
	//-----------------------------------------------------------------------------------------
	// ���Ӽ��� ���� ������Ʈ ���� ���� ��Ŷ ���� 
	//-----------------------------------------------------------------------------------------
	CGameServerMessage* MakePacketResChangeMonsterObjectState(int64 ObjectId, en_MoveDir Direction, en_GameObjectType ObjectType, en_CreatureState ObjectState, en_MonsterState MonsterState);
	//-----------------------------------------------------------------------------------------
	// ���Ӽ��� �̵� ��û ���� ��Ŷ ����
	//-----------------------------------------------------------------------------------------
	CGameServerMessage* MakePacketResMove(int64 ObjectId, bool CanMove, st_PositionInfo PositionInfo);
	//-----------------------------------------------------------------------------------------
	// ���Ӽ��� ���� �̵� ��Ŷ ����
	//-----------------------------------------------------------------------------------------
	CGameServerMessage* MakePacketResMonsterMove(int64 ObjectId, en_GameObjectType ObjectType, bool CanMove, st_PositionInfo PositionInfo, en_MonsterState MonsterState);
	//------------------------------------------------------------------------------------------------------
	// ���Ӽ��� �̵� ���� ��û ���� ��Ŷ ����
	//------------------------------------------------------------------------------------------------------
	CGameServerMessage* MakePacketResMoveStop(int64 ObjectId, st_PositionInfo PositionInto);
	//-----------------------------------------------------------------------------------------
	// ���Ӽ��� ���� ��Ŷ ����
	//-----------------------------------------------------------------------------------------
	CGameServerMessage* MakePacketPatrol(int64 ObjectId, en_GameObjectType ObjectType, bool CanMove, st_PositionInfo PositionInfo, en_MonsterState MonsterState);
	//-----------------------------------------------------------------------------------------
	// ���Ӽ��� ������ ������ ���� ��Ŷ ����
	//-----------------------------------------------------------------------------------------
	CGameServerMessage* MakePacketItemMove(st_GameObjectInfo ItemMoveObjectInfo);
	//-----------------------------------------------------------------------------------------
	// ���Ӽ��� ������Ʈ ���� ��Ŷ ���� ( ���� ��� )
	//-----------------------------------------------------------------------------------------
	CGameServerMessage* MakePacketResObjectSpawn(CGameObject* SpawnObject);
	//-----------------------------------------------------------------------------------------
	// ���Ӽ��� ������Ʈ ���� ��Ŷ ���� ( ���� ��� )
	//-----------------------------------------------------------------------------------------
	CGameServerMessage* MakePacketResObjectSpawn(int32 ObjectInfosCount, vector<CGameObject*> ObjectInfos);
	//-------------------------------------------------------------------
	// ���Ӽ��� ������Ʈ ���� ��Ŷ ���� ( ���� ��� )
	//-------------------------------------------------------------------
	CGameServerMessage* MakePacketResObjectDeSpawn(int64 DeSpawnObjectID);
	//-----------------------------------------------------------------------------------------
	// ���Ӽ��� ������Ʈ ���� ��Ŷ ���� ( ���� ��� )
	//-----------------------------------------------------------------------------------------
	CGameServerMessage* MakePacketResObjectDeSpawn(int32 DeSpawnObjectCount, vector<CGameObject*> DeSpawnObjects);
	//-----------------------------------------------------------------------------------------
	// ���Ӽ��� ������Ʈ ���� ��Ŷ ����
	//-----------------------------------------------------------------------------------------
	CGameServerMessage* MakePacketObjectDie(int64 DieObjectId);
	//-----------------------------------------------------------------------------------------
	// ���Ӽ��� �޼��� ( ä��, �ý��� ) ��û ��Ŷ ����
	//-----------------------------------------------------------------------------------------
	CGameServerMessage* MakePacketResChattingBoxMessage(int64 PlayerDBId, en_MessageType MessageType, st_Color Color, wstring ChattingMessage);
	//-----------------------------------------------------------------------------------------
	// ���Ӽ��� ������Ʈ ��ġ ��ũ ���߱� ��Ŷ ����
	//-----------------------------------------------------------------------------------------
	CGameServerMessage* MakePacketResSyncPosition(int64 TargetObjectId, st_PositionInfo SyncPosition);
	//-----------------------------------------------------------------------------------------
	// ���Ӽ��� ������ �ʱ�ȭ ��Ŷ ����
	//-----------------------------------------------------------------------------------------
	CGameServerMessage* MakePacketResQuickSlotInit(int8 QuickSlotBarIndex, int8 QuickSlotBarSlotIndex);
	//------------------------------------------------------------------------------------------------------------
	// ���Ӽ��� ��ų Ư�� ���� ���� ��Ŷ ����
	//------------------------------------------------------------------------------------------------------------
	CGameServerMessage* MakePacketResSelectSkillCharacteristic(bool IsSuccess, int8 SkillCharacteristicIndex, int8 SkillCharacteristicType, vector<CSkill*> PassiveSkills, vector<CSkill*> ActiveSkills);
	//-----------------------------------------------------------------------------------------
	// ���Ӽ��� ��ų ���� ��Ŷ ����
	//-----------------------------------------------------------------------------------------
	CGameServerMessage* MakePacketResSkillToSkillBox(int64 TargetObjectId, st_SkillInfo* SkillInfo);
	//-------------------------------------------------------------------
	// ���Ӽ��� ��ų ���� ���� ��Ŷ ����
	//-------------------------------------------------------------------
	CGameServerMessage* MakePacketResSkillLearn(bool IsSkillLearn, en_SkillType LearnSkillType, int8 SkillMaxPoint, int8 SkillPoint);
	//-----------------------------------------------------------------------------------------
	// ���Ӽ��� ����Ʈ ��� ��Ŷ ����
	//-----------------------------------------------------------------------------------------
	CGameServerMessage* MakePacketEffect(int64 TargetObjectId, en_EffectType EffectType, float PrintEffectTime);
	//---------------------------------------------------------------------------------
	// ���Ӽ��� ��ȭȿ��, ��ȭȿ�� ��Ŷ ����
	//---------------------------------------------------------------------------------
	CGameServerMessage* MakePacketBufDeBuf(int64 TargetObjectId, bool BufDeBuf, st_SkillInfo* SkillInfo);
	//---------------------------------------------------------------------------------
	// ���Ӽ��� ��ȭȿ��, ��ȭȿ�� ���� ��Ŷ ����
	//---------------------------------------------------------------------------------
	CGameServerMessage* MakePacketBufDeBufOff(int64 TargetObjectId, bool BufDeBuf, en_SkillType OffSkillType);
	//--------------------------------------------------------------------------------------------------------
	// ���Ӽ��� ���ӱ� ��ų �ѱ� ��Ŷ ����
	//--------------------------------------------------------------------------------------------------------
	CGameServerMessage* MakePacketComboSkillOn(vector<st_Vector2Int> ComboSkillQuickSlotPositions, st_SkillInfo ComboSkillInfo);
	//--------------------------------------------------------------------------------------------------------
	// ���Ӽ��� ���ӱ� ��ų ���� ��Ŷ ����
	//--------------------------------------------------------------------------------------------------------
	CGameServerMessage* MakePacketComboSkillOff(vector<st_Vector2Int> ComboSkillQuickSlotPositions, st_SkillInfo ComboSkillInfo, en_SkillType OffComboSkillType);
	//-----------------------------------------------------------------------------------------
	// ���Ӽ��� ����ġ ��Ŷ ����
	//-----------------------------------------------------------------------------------------
	CGameServerMessage* MakePacketExperience(int64 AccountId, int64 PlayerId, int64 GainExp, int64 CurrentExp, int64 RequireExp, int64 TotalExp);
	//-----------------------------------------------------------------------------------------
	// ���Ӽ��� ��ų ��� ��Ŷ ����
	//-----------------------------------------------------------------------------------------
	CGameServerMessage* MakePacketMagicCancel(int64 PlayerId);
	//-----------------------------------------------------------------------------------------
	// ���Ӽ��� ä�� ��� ��Ŷ ����
	//-----------------------------------------------------------------------------------------
	CGameServerMessage* MakePacketGatheringCancel(int64 ObjectID);
	//-----------------------------------------------------------------------------------------
	// ���Ӽ��� ������ð� ��� ��Ŷ ����
	//-----------------------------------------------------------------------------------------
	CGameServerMessage* MakePacketCoolTime(int8 QuickSlotBarIndex, int8 QuickSlotBarSlotIndex, float SkillCoolTimeSpeed, CSkill* QuickSlotSkill = nullptr, int32 CoolTime = 0);
	//-----------------------------------------------------------------------------------------
	// ���Ӽ��� ��ų ���� �޼��� ���� ��Ŷ ����
	//-----------------------------------------------------------------------------------------
	CGameServerMessage* MakePacketSkillError(en_PersonalMessageType PersonalMessageType, const WCHAR* SkillName = nullptr, int16 SkillDistance = 0);
	//-----------------------------------------------------------------------------------------
	// ���Ӽ��� �Ϲ� ���� �޼��� ���� ��Ŷ ����
	//-----------------------------------------------------------------------------------------
	CGameServerMessage* MakePacketCommonError(en_PersonalMessageType PersonalMessageType, const WCHAR* Name = nullptr);
	//------------------------------------------------------------
	// ���� ���� �����̻� ���� ��Ŷ ����
	//------------------------------------------------------------
	CGameServerMessage* MakePacketStatusAbnormal(int64 TargetId, en_GameObjectType ObjectType, en_MoveDir Dir, en_SkillType SkillType, bool SetStatusAbnormal, int8 StatusAbnormal);
	//-----------------------------------------------------------------------------------------
	// ���Ӽ��� ���� ������ ������Ʈ
	//-----------------------------------------------------------------------------------------
	CGameServerMessage* MakePacketInventoryItemUpdate(int64 PlayerId, st_ItemInfo UpdateItemInfo);
	//-------------------------------------------------------------------------------------------------------------------------
	// ���Ӽ��� ������ ���� ���� ��û ���� ��Ŷ ����
	//-------------------------------------------------------------------------------------------------------------------------
	CGameServerMessage* MakePacketResItemToInventory(int64 TargetObjectId, st_ItemInfo InventoryItem, bool IsExist, int16 ItemEach, bool ItemGainPrint = true);
	//------------------------------------------------------------------------------------------------------------------------------------------------------
	// ���Ӽ��� ���� �� ���� ��û ���� ��Ŷ ����
	//------------------------------------------------------------------------------------------------------------------------------------------------------
	CGameServerMessage* MakePacketResMoneyToInventory(int64 TargetObjectID, int64 GoldCoinCount, int16 SliverCoinCount, int16 BronzeCoinCount, st_ItemInfo ItemInfo, int16 ItemEach);
	//-----------------------------------------------------------------------------------------------
	// ���Ӽ��� ���۴� ��� ������ ��� ��Ŷ ����
	//-----------------------------------------------------------------------------------------------
	CGameServerMessage* MakePacketResCraftingTableMaterialItemList(int64 CraftingTableObjectID, en_GameObjectType CraftingTableObjectType, en_SmallItemCategory SelectCompleteItemType, map<en_SmallItemCategory, CItem*> MaterialItems);
	//-----------------------------------------------------------------------------------------------
	// ���Ӽ��� ���۴� ������ �ֱ� ���� ��Ŷ ����
	//-----------------------------------------------------------------------------------------------
	CGameServerMessage* MakePacketResCraftingTableInput(int64 CraftingTableObjectID, map<en_SmallItemCategory, CItem*> MaterialItems);
	//-----------------------------------------------------------------------------------------------
	// ���Ӽ��� ���۴� ���� ���� ���� ��Ŷ ����
	//-----------------------------------------------------------------------------------------------
	CGameServerMessage* MakePacketResCraftingStart(int64 CraftingTableObjectID, st_ItemInfo CraftingItemInfo);
	//-----------------------------------------------------------------------------------------------
	// ���Ӽ��� ���۴� ���� ���� ���� ��Ŷ ����
	//-----------------------------------------------------------------------------------------------
	CGameServerMessage* MakePacketResCraftingStop(int64 CraftingTableObjectID, st_ItemInfo CraftingStopItemInfo);
	//-----------------------------------------------------------------------------------------------
	// ���Ӽ��� ���۴� ���� ���� �ð� ��Ŷ ����
	//-----------------------------------------------------------------------------------------------
	CGameServerMessage* MakePacketResCraftingTableCraftRemainTime(int64 CraftingTableObjectID, st_ItemInfo CraftingItemInfo);
	//-----------------------------------------------------------------------------------------------
	// ���Ӽ��� ���۴� ���� �Ϸ� ������ ��� ��Ŷ ����
	//-----------------------------------------------------------------------------------------------
	CGameServerMessage* MakePacketResCraftingTableCompleteItemList(int64 CraftingTableObjectID, en_GameObjectType CraftingTableObjectType, map<en_SmallItemCategory, CItem*> CompleteItems);
	//-----------------------------------------------------------------------------------------
	// ���Ӽ��� ��� ���� ���� ��Ŷ ����
	//-----------------------------------------------------------------------------------------
	CGameServerMessage* MakePacketOnEquipment(int64 PlayerId, st_ItemInfo& EquipmentItemInfo);
	//-----------------------------------------------------------------------------------------
	// ���Ӽ��� ��� ���� ���� ��Ŷ ����
	//-----------------------------------------------------------------------------------------
	CGameServerMessage* MakePacketOffEquipment(int64 PlayerID, en_EquipmentParts OffEquipmentParts);
	//-----------------------------------------------------------------------------------------
	// ���Ӽ��� ���� �ɱ� ���� ��Ŷ ����
	//-----------------------------------------------------------------------------------------
	CGameServerMessage* MakePacketSeedFarming(st_ItemInfo SeedItem, int64 SeedObjectID);
	//-----------------------------------------------------------------------------------------
	// ���Ӽ��� �۹� ���� �ܰ� Ȯ�� ��û ���� ��Ŷ ����
	//-----------------------------------------------------------------------------------------
	CGameServerMessage* MakePacketPlantGrowthStep(int64 PlantObjectID, int8 PlantGrowthStep, float PlantGrowthRatio);
	//---------------------------------------------------
	// ���Ӽ��� ��û ���� ���� ��Ŷ ����
	//---------------------------------------------------
	CGameServerMessage* MakePacketReqCancel(en_GAME_SERVER_PACKET_TYPE PacketType);

	//---------------------------------------------------
	// �α��� ���� �α׾ƿ� ��û ��Ŷ ����
	//---------------------------------------------------
	CGameServerMessage* MakePacketLoginServerLogOut(int64 AccountID);
	//---------------------------------------------------
	// �α��� ���� �α��� ���� ���� ��Ŷ ����
	//---------------------------------------------------
	CGameServerMessage* MakePacketLoginServerLoginStateChange(int64 AccountID, en_LoginState ChangeLoginState);
public:
	//-----------------------------------------------
	// Job �޸�Ǯ
	//-----------------------------------------------
	CMemoryPoolTLS<st_GameServerJob>* _GameServerJobMemoryPool;

	//------------------------------------
	// TimerJob �޸�Ǯ
	//------------------------------------
	CMemoryPoolTLS<st_TimerJob>* _TimerJobMemoryPool;

	//------------------------------------
	// Job ť
	//------------------------------------
	CLockFreeQue<st_GameServerJob*> _GameServerUserDBThreadMessageQue;
	CLockFreeQue<st_GameServerJob*> _GameServerWorldDBThreadMessageQue;

	CLockFreeQue<st_GameServerJob*> _GameServerClientLeaveDBThreadMessageQue;

	//--------------------------------------
	// TimerJob �켱���� ť
	//--------------------------------------
	CHeap<int64, st_TimerJob*>* _TimerHeapJob;

	int64 _LogicThreadFPS;
	// ��Ʈ��ũ ������ Ȱ��ȭ�� Ƚ��
	int64 _NetworkThreadWakeCount;
	// ��Ʈ��ũ ������ TPS
	int64 _NetworkThreadTPS;
	
	// UpdateThread ������ ����� �̺�Ʈ
	HANDLE _UpdateThreadWakeEvent;
	// User DB ������ ����� �̺�Ʈ
	HANDLE _UserDataBaseWakeEvent;
	// World DB ������ ����� �̺�Ʈ
	HANDLE _WorldDataBaseWakeEvent;
	// ClientLeaveDB ������ ����� �̺�Ʈ
	HANDLE _ClientLeaveDBThreadWakeEvent;
	// DB ������ Ȱ��ȭ�� Ƚ��
	int64 _UserDBThreadWakeCount;
	// DB ������ TPS
	int64 _UserDBThreadTPS;
	// Ŭ�� ���� ���� ������ TPS
	int64 _LeaveDBThreadTPS;

	// Ÿ�̸� �� ������ Ȱ��ȭ�� Ƚ��
	int64 _TimerJobThreadWakeCount;
	// Ÿ�̸� �� ������ TPS
	int64 _TimerJobThreadTPS;

	CGameServer();
	~CGameServer();

	//------------------------------------------
	// ���� ���� ����
	//------------------------------------------
	void GameServerStart(const WCHAR* OpenIP, int32 Port);

	//------------------------------------------------------
	// �α��� ������ ����
	//------------------------------------------------------
	void LoginServerConnect(const WCHAR* ConnectIP, int32 Port);

	//--------------------------------------------------
	// ���ο� Ŭ���̾�Ʈ ���� 
	//--------------------------------------------------
	virtual void OnClientJoin(int64 SessionID) override;
	//--------------------------------------------------------------
	// ��Ʈ��ũ ��Ŷ ó��
	//--------------------------------------------------------------
	virtual void OnRecv(int64 SessionID, CMessage* Packet) override;
	//--------------------------------------------------------------
	// Ŭ���̾�Ʈ ����
	//--------------------------------------------------------------
	virtual void OnClientLeave(st_Session* LeaveSession) override;
	virtual bool OnConnectionRequest(const wchar_t ClientIP, int32 Port) override;

	//--------------------------------------------------------------
	// �þ߹��� �������� ��Ŷ ����
	//--------------------------------------------------------------
	void SendPacketFieldOfView(vector<st_FieldOfViewInfo> FieldOfViewObject, CMessage* Message);
	//--------------------------------------------------------------
	// ������Ʈ�� �������� �þߺ� �ȿ� �ִ� �÷��̾� ������� ��Ŷ ����
	//--------------------------------------------------------------
	void SendPacketFieldOfView(CGameObject* Object, CMessage* Message);	

	//-------------------------------------------
	// �� Ÿ�̸� �� ����
	//-------------------------------------------
	void PingTimerCreate(st_Session* PingSession);
};
