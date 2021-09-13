#include "pch.h"
#include "Player.h"
#include "ObjectManager.h"

CPlayer::CPlayer()
{
	_GameObjectInfo.ObjectType = en_GameObjectType::MELEE_PLAYER;	
}

CPlayer::CPlayer(st_GameObjectInfo _PlayerInfo)
{
	_GameObjectInfo = _PlayerInfo;		
	_GameObjectInfo.ObjectType = en_GameObjectType::MELEE_PLAYER;		
}

CPlayer::~CPlayer()
{
}

void CPlayer::Update()
{	
	switch (_GameObjectInfo.ObjectPositionInfo.State)
	{
	case en_CreatureState::ATTACK:		
		UpdateAttack();
		break;
	case en_CreatureState::SPELL:
		UpdateSpell();
		break;
	default:
		break;
	}
}

void CPlayer::OnDamaged(CGameObject* Attacker, int32 Damage)
{
	//CGameObject::OnDamaged(Attacker, Damage);

}

void CPlayer::OnDead(CGameObject* Killer)
{

}

void CPlayer::SetAttackMeleeType(en_AttackType AttackType, vector<CGameObject*> Targets)
{
	wstring AttackMeleeSystemString;
	wstring AttackTypeString;
	wstring AttackDamageString;

	wchar_t AttackTypeMessage[64] = L"0";
	wchar_t AttackDamageMessage[64] = L"0";

	_Targets.clear();

	_Targets = Targets;

	_AttackType = AttackType;	

	st_Vector2Int SpellCellPosition;
	CMessage* ResSyncPosition = nullptr;

	_AttackTick = GetTickCount64() + 500;
	_GameObjectInfo.ObjectPositionInfo.State = en_CreatureState::ATTACK;

	BroadCastPacket(en_PACKET_TYPE::en_PACKET_S2C_OBJECT_STATE_CHANGE);
		
	if (_Targets.size() >= 1)
	{
		switch (_AttackType)
		{
		case en_AttackType::MELEE_PLAYER_NORMAL_ATTACK:
		case en_AttackType::MAGIC_PLAYER_NORMAL_ATTACK:
			wsprintf(AttackTypeMessage, L"%s�� �Ϲݰ����� ����� %s���� ", _GameObjectInfo.ObjectName.c_str(), _Targets[0]->_GameObjectInfo.ObjectName.c_str());
			AttackTypeString = AttackTypeMessage;
			break;
		case en_AttackType::MELEE_PLAYER_CHOHONE_ATTACK:
			wsprintf(AttackTypeMessage, L"%s�� ��ȥ�񹫸� ����� %s���� ", _GameObjectInfo.ObjectName.c_str(), _Targets[0]->_GameObjectInfo.ObjectName.c_str());
			AttackTypeString = AttackTypeMessage;
			// �� ���� ��ġ�� ���Ѵ�.
			SpellCellPosition = GetFrontCellPosition(_GameObjectInfo.ObjectPositionInfo.MoveDir, 1);
			// Ÿ���� �� ���� ��ġ�� �̵���Ų��.
			_Channel->_Map->ApplyMove(_Targets[0], SpellCellPosition);

			// Ÿ���� ��ġ�� �����ϱ� ���� SyncPosition ��Ŷ�� ������.
			ResSyncPosition = G_ObjectManager->GameServer->MakePacketResSyncPosition(Targets[0]->_GameObjectInfo.ObjectId, Targets[0]->_GameObjectInfo.ObjectPositionInfo);
			G_ObjectManager->GameServer->SendPacketAroundSector(_Targets[0]->GetCellPosition(), ResSyncPosition);
			ResSyncPosition->Free();
			break;
		case en_AttackType::MELEE_PLAYER_SHAEHONE_ATTACK:
			wsprintf(AttackTypeMessage, L"%s�� ��ȥ�񹫸� ����� %s���� ", _GameObjectInfo.ObjectName.c_str(), _Targets[0]->_GameObjectInfo.ObjectName.c_str());
			AttackTypeString = AttackTypeMessage;

			// �� ���� 3ĭ ��ġ�� ���Ѵ�.
			SpellCellPosition = GetFrontCellPosition(_GameObjectInfo.ObjectPositionInfo.MoveDir, 3);
			_Channel->_Map->ApplyMove(this, SpellCellPosition);

			ResSyncPosition = G_ObjectManager->GameServer->MakePacketResSyncPosition(_GameObjectInfo.ObjectId, _GameObjectInfo.ObjectPositionInfo);
			G_ObjectManager->GameServer->SendPacketAroundSector(GetCellPosition(), ResSyncPosition);
			ResSyncPosition->Free();
			break;		
		case en_AttackType::MELEE_PLAYER_AROUND_ATTACK:				
			break;
		default:
			CRASH("�÷��̾� ���� Ÿ�� ���� Error");
			break;
		}

		for (CGameObject* Target : _Targets)
		{
			random_device Seed;
			default_random_engine Eng(Seed());	

			float CriticalPoint = _GameObjectInfo.ObjectStatInfo.CriticalPoint / 1000.0f;			
			bernoulli_distribution CriticalCheck(CriticalPoint);
			bool IsCritical = CriticalCheck(Eng);					
			
			mt19937 Gen(Seed());
			uniform_int_distribution<int> DamageChoiceRandom(_GameObjectInfo.ObjectStatInfo.MinAttackDamage, _GameObjectInfo.ObjectStatInfo.MaxAttackDamage);
			int32 ChoiceDamage = DamageChoiceRandom(Gen);
			int32 FinalDamage = IsCritical ? ChoiceDamage * 2 : ChoiceDamage;

			Target->OnDamaged(this, FinalDamage);

			wsprintf(AttackDamageMessage, L"%d�� �������� ����ϴ�.", FinalDamage);
			AttackDamageString = AttackDamageMessage;			

			AttackMeleeSystemString = IsCritical ? L"ġ��Ÿ! " + AttackTypeString + AttackDamageString : AttackTypeString + AttackDamageString;

			st_Color MessageColor(255, 192, 203);

			CMessage* ResAttackMeleeSystemMessagePacket = G_ObjectManager->GameServer->MakePacketResChattingMessage(_GameObjectInfo.ObjectId, en_MessageType::SYSTEM, IsCritical ? st_Color::Red() : st_Color::White(), AttackMeleeSystemString);
			G_ObjectManager->GameServer->SendPacketAroundSector(GetCellPosition(), ResAttackMeleeSystemMessagePacket);
			ResAttackMeleeSystemMessagePacket->Free();

			CMessage* ResMyAttackOtherPacket = G_ObjectManager->GameServer->MakePacketResAttack(_GameObjectInfo.ObjectId, Target->_GameObjectInfo.ObjectId, AttackType, FinalDamage, IsCritical);
			G_ObjectManager->GameServer->SendPacketAroundSector(GetCellPosition(), ResMyAttackOtherPacket);
			ResMyAttackOtherPacket->Free();

			CMessage* ResChangeHPPacket = G_ObjectManager->GameServer->MakePacketResChangeHP(Target->_GameObjectInfo.ObjectId, Target->_GameObjectInfo.ObjectStatInfo.HP, Targets[0]->_GameObjectInfo.ObjectStatInfo.MaxHP);
			G_ObjectManager->GameServer->SendPacketAroundSector(_Targets[0]->GetCellPosition(), ResChangeHPPacket);
			ResChangeHPPacket->Free();
		}
	}		
}

void CPlayer::SetAttackMagicType(en_AttackType AttackType, vector<CGameObject*> Targets)
{
	_Targets.clear();

	_Targets = Targets;

	_AttackType = AttackType;	

	if (_Targets.size() >= 1)
	{		
		_GameObjectInfo.ObjectPositionInfo.State = en_CreatureState::SPELL;

		// Magic �غ� ������� ���
		BroadCastPacket(en_PACKET_S2C_OBJECT_STATE_CHANGE);
		// CastringBar ���
		BroadCastPacket(en_PACKET_S2C_MAGIC_ATTACK);

		switch (_AttackType)
		{
		case en_AttackType::MAGIC_PLAYER_FIRE_ATTACK:			
			_AttackTick = GetTickCount64() + 1500;
			break;
		default:
			break;
		}
	}
	else if (_Targets.size() == 0)
	{
		
	}
}

void CPlayer::UpdateAttack()
{
	// ������ AttackTick�ð��� �ɰ��
	if (_AttackTick < GetTickCount64())
	{		
		switch (_AttackType)
		{
		case en_AttackType::MELEE_PLAYER_NORMAL_ATTACK:			
		case en_AttackType::MELEE_PLAYER_CHOHONE_ATTACK:			
		case en_AttackType::MELEE_PLAYER_SHAEHONE_ATTACK:			
		case en_AttackType::MELEE_PLAYER_AROUND_ATTACK:
		case en_AttackType::MAGIC_PLAYER_NORMAL_ATTACK:
			// Idle ���·� �ٲٰ� ���� ���� �÷��̾�鿡�� �˷��ش�.
			_GameObjectInfo.ObjectPositionInfo.State = en_CreatureState::IDLE;
			BroadCastPacket(en_PACKET_S2C_OBJECT_STATE_CHANGE);
			break;		
		default:
			break;
		}
		
	}
}

void CPlayer::UpdateSpell()
{
	// ���� ���� ��û�ÿ��� ���� ƽ�� AttackTick���� Ŀ���� �Ǹ� �������� �Ѵ�
	if (_AttackTick < GetTickCount64())
	{
		wstring AttackMagicSystemString;
		wstring AttackTypeString;
		wstring AttackDamageString;

		wchar_t AttackTypeMessage[64] = L"0";
		wchar_t AttackDamageMessage[64] = L"0";

		// ũ��Ƽ�� �Ǵ� �غ�
		random_device RD;
		mt19937 Gen(RD());

		uniform_int_distribution<int> CriticalPointCreate(0, 100);

		bool IsCritical = false;

		int16 CriticalPoint = CriticalPointCreate(Gen);

		// ũ��Ƽ�� ����
		// �� ĳ������ ũ��Ƽ�� ����Ʈ���� ���� ������ ũ��Ƽ�÷� �Ǵ��Ѵ�.				
		if (CriticalPoint < _GameObjectInfo.ObjectStatInfo.CriticalPoint)
		{
			IsCritical = true;
		}

		uniform_int_distribution<int> DamageChoiceRandom(_GameObjectInfo.ObjectStatInfo.MinAttackDamage, _GameObjectInfo.ObjectStatInfo.MaxAttackDamage);
		int32 ChoiceDamage = DamageChoiceRandom(Gen);
		int32 FinalDamage = IsCritical ? ChoiceDamage * 2 : ChoiceDamage;
		
		// ������ ó��
		_Targets[0]->OnDamaged(this, FinalDamage);

		// ���� ����
		CMessage* ResAttackMagicPacket = G_ObjectManager->GameServer->MakePacketResAttack(
			_GameObjectInfo.ObjectId,
			_Targets[0]->_GameObjectInfo.ObjectId,
			_AttackType,
			FinalDamage,
			true);
		G_ObjectManager->GameServer->SendPacketAroundSector(_Targets[0]->GetCellPosition(), ResAttackMagicPacket);
		ResAttackMagicPacket->Free();				

		// ���� �޼��� ����
		switch (_AttackType)
		{
		case en_AttackType::MAGIC_PLAYER_FIRE_ATTACK:	
			wsprintf(AttackTypeMessage, L"%s�� ȭ�������� ����� %s�� ", _GameObjectInfo.ObjectName.c_str(), _Targets[0]->_GameObjectInfo.ObjectName.c_str());
			AttackTypeString = AttackTypeMessage;			
			
			_GameObjectInfo.ObjectPositionInfo.State = en_CreatureState::IDLE;			
			BroadCastPacket(en_PACKET_TYPE::en_PACKET_S2C_OBJECT_STATE_CHANGE);
			break;
		}				

		wsprintf(AttackDamageMessage, L"%d�� �������� ����ϴ�.", FinalDamage);
		AttackDamageString = AttackDamageMessage;

		AttackMagicSystemString = AttackTypeMessage + AttackDamageString;

		// �޼��� ����
		CMessage* ResAttackMagicSystemMessagePacket = G_ObjectManager->GameServer->MakePacketResChattingMessage(_GameObjectInfo.ObjectId, en_MessageType::SYSTEM, st_Color::Red(), AttackMagicSystemString);
		G_ObjectManager->GameServer->SendPacketAroundSector(GetCellPosition(), ResAttackMagicSystemMessagePacket);
		ResAttackMagicSystemMessagePacket->Free();	
		
		// HP ���� ����
		CMessage* ResChangeHPPacket = G_ObjectManager->GameServer->MakePacketResChangeHP(_Targets[0]->_GameObjectInfo.ObjectId, _Targets[0]->_GameObjectInfo.ObjectStatInfo.HP, _Targets[0]->_GameObjectInfo.ObjectStatInfo.MaxHP);
		G_ObjectManager->GameServer->SendPacketAroundSector(GetCellPosition(), ResChangeHPPacket);
		ResChangeHPPacket->Free();
	}
}
