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
	CGameObject::OnDamaged(Attacker, Damage);

}

void CPlayer::OnDead(CGameObject* Killer)
{

}

void CPlayer::UpdateAttack()
{
	// ������ AttackTick�ð��� �ɰ��
	if (_AttackTick < GetTickCount64())
	{		
  		switch (_SkillType)
		{
		case en_SkillType::SKILL_KNIGHT_NORMAL:
		case en_SkillType::SKILL_KNIGHT_CHOHONE:
		case en_SkillType::SKILL_KNIGHT_SHAEHONE:
		case en_SkillType::SKILL_KNIGHT_SMASH_WAVE:
		case en_SkillType::SKILL_SHAMAN_NORMAL:
			// Idle ���·� �ٲٰ� ���� ���� �÷��̾�鿡�� �˷��ش�.
			_GameObjectInfo.ObjectPositionInfo.State = en_CreatureState::IDLE;
			BroadCastPacket(en_PACKET_S2C_OBJECT_STATE_CHANGE);
			_SkillType = en_SkillType::SKILL_TYPE_NONE;
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
		wstring MagicSystemString;

		wchar_t SpellMessage[64] = L"0";

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

		int32 FinalDamage = 0;

		switch (_SkillType)
		{
		case en_SkillType::SKILL_SHAMNA_FLAME_HARPOON:
		{
			uniform_int_distribution<int> DamageChoiceRandom(_GameObjectInfo.ObjectStatInfo.MinAttackDamage, _GameObjectInfo.ObjectStatInfo.MaxAttackDamage);
			int32 ChoiceDamage = DamageChoiceRandom(Gen);
			FinalDamage = 40;// IsCritical ? ChoiceDamage * 2 : ChoiceDamage
								   
			// ������ ó��
			_Target->OnDamaged(this, FinalDamage);		

			wsprintf(SpellMessage, L"%s�� �Ҳ��ۻ��� ����� %s���� %d�� �������� ����ϴ�.", _GameObjectInfo.ObjectName.c_str(), _Target->_GameObjectInfo.ObjectName.c_str(), FinalDamage);

			MagicSystemString = SpellMessage;
		}
			break;
		case en_SkillType::SKILL_SHAMAN_HEALING_LIGHT:
		{
			FinalDamage = 100;
			_Target->OnHeal(this, FinalDamage);

			wsprintf(SpellMessage, L"%s�� ġ���Ǻ��� ����� %s�� %d��ŭ ȸ���߽��ϴ�.", _GameObjectInfo.ObjectName.c_str(), _Target->_GameObjectInfo.ObjectName.c_str(), 10);
			MagicSystemString = SpellMessage;
		}
			break;
		case en_SkillType::SKILL_SHAMAN_HEALING_WIND:
		{
			FinalDamage = 200;
			_Target->OnHeal(this, FinalDamage);
			
			wsprintf(SpellMessage, L"%s�� ġ���ǹٶ��� ����� %s�� %d��ŭ ȸ���߽��ϴ�.", _GameObjectInfo.ObjectName.c_str(), _Target->_GameObjectInfo.ObjectName.c_str(), 10);
			MagicSystemString = SpellMessage;
		}
		break;
		default:
			break;
		}					

		// ���� ����
		CMessage* ResAttackMagicPacket = G_ObjectManager->GameServer->MakePacketResAttack(
			_GameObjectInfo.ObjectId,
			_Target->_GameObjectInfo.ObjectId,
			_SkillType,
			FinalDamage,
			false);
		G_ObjectManager->GameServer->SendPacketAroundSector(_Target->GetCellPosition(), ResAttackMagicPacket);
		ResAttackMagicPacket->Free();

		// Idle�� ���� ���� �� �������Ϳ� ����
		_GameObjectInfo.ObjectPositionInfo.State = en_CreatureState::IDLE;
		BroadCastPacket(en_PACKET_TYPE::en_PACKET_S2C_OBJECT_STATE_CHANGE);

		// �ý��� �޼��� ����
		CMessage* ResAttackMagicSystemMessagePacket = G_ObjectManager->GameServer->MakePacketResChattingMessage(_GameObjectInfo.ObjectId, en_MessageType::SYSTEM, st_Color::White(), MagicSystemString);
		G_ObjectManager->GameServer->SendPacketAroundSector(GetCellPosition(), ResAttackMagicSystemMessagePacket);
		ResAttackMagicSystemMessagePacket->Free();	
		
		// HP ���� ����
		CMessage* ResChangeHPPacket = G_ObjectManager->GameServer->MakePacketResChangeHP(_Target->_GameObjectInfo.ObjectId, _Target->_GameObjectInfo.ObjectStatInfo.HP, _Target->_GameObjectInfo.ObjectStatInfo.MaxHP);
		G_ObjectManager->GameServer->SendPacketAroundSector(GetCellPosition(), ResChangeHPPacket);
		ResChangeHPPacket->Free();	

		// ����â ��
		CMessage* ResMagicPacket = G_ObjectManager->GameServer->MakePacketResMagic(_GameObjectInfo.ObjectId, false);
		G_ObjectManager->GameServer->SendPacketAroundSector(GetCellPosition(), ResMagicPacket);
		ResMagicPacket->Free();
	}
}
