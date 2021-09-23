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
	// 지정한 AttackTick시간이 될경우
	if (_AttackTick < GetTickCount64())
	{		
  		switch (_SkillType)
		{
		case en_SkillType::SKILL_KNIGHT_NORMAL:
		case en_SkillType::SKILL_KNIGHT_CHOHONE:
		case en_SkillType::SKILL_KNIGHT_SHAEHONE:
		case en_SkillType::SKILL_KNIGHT_SMASH_WAVE:
		case en_SkillType::SKILL_SHAMAN_NORMAL:
			// Idle 상태로 바꾸고 주위 섹터 플레이어들에게 알려준다.
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
	// 마법 공격 요청시에는 현재 틱이 AttackTick보다 커지게 되면 공격판정 한다
	if (_AttackTick < GetTickCount64())
	{
		wstring MagicSystemString;

		wchar_t SpellMessage[64] = L"0";

		// 크리티컬 판단 준비
		random_device RD;
		mt19937 Gen(RD());

		uniform_int_distribution<int> CriticalPointCreate(0, 100);

		bool IsCritical = false;

		int16 CriticalPoint = CriticalPointCreate(Gen);
		// 크리티컬 판정
		// 내 캐릭터의 크리티컬 포인트보다 값이 작으면 크리티컬로 판단한다.				
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
								   
			// 데미지 처리
			_Target->OnDamaged(this, FinalDamage);		

			wsprintf(SpellMessage, L"%s가 불꽃작살을 사용해 %s에게 %d의 데미지를 줬습니다.", _GameObjectInfo.ObjectName.c_str(), _Target->_GameObjectInfo.ObjectName.c_str(), FinalDamage);

			MagicSystemString = SpellMessage;
		}
			break;
		case en_SkillType::SKILL_SHAMAN_HEALING_LIGHT:
		{
			FinalDamage = 100;
			_Target->OnHeal(this, FinalDamage);

			wsprintf(SpellMessage, L"%s가 치유의빛을 사용해 %s를 %d만큼 회복했습니다.", _GameObjectInfo.ObjectName.c_str(), _Target->_GameObjectInfo.ObjectName.c_str(), 10);
			MagicSystemString = SpellMessage;
		}
			break;
		case en_SkillType::SKILL_SHAMAN_HEALING_WIND:
		{
			FinalDamage = 200;
			_Target->OnHeal(this, FinalDamage);
			
			wsprintf(SpellMessage, L"%s가 치유의바람을 사용해 %s를 %d만큼 회복했습니다.", _GameObjectInfo.ObjectName.c_str(), _Target->_GameObjectInfo.ObjectName.c_str(), 10);
			MagicSystemString = SpellMessage;
		}
		break;
		default:
			break;
		}					

		// 공격 응답
		CMessage* ResAttackMagicPacket = G_ObjectManager->GameServer->MakePacketResAttack(
			_GameObjectInfo.ObjectId,
			_Target->_GameObjectInfo.ObjectId,
			_SkillType,
			FinalDamage,
			false);
		G_ObjectManager->GameServer->SendPacketAroundSector(_Target->GetCellPosition(), ResAttackMagicPacket);
		ResAttackMagicPacket->Free();

		// Idle로 상태 변경 후 주위섹터에 전송
		_GameObjectInfo.ObjectPositionInfo.State = en_CreatureState::IDLE;
		BroadCastPacket(en_PACKET_TYPE::en_PACKET_S2C_OBJECT_STATE_CHANGE);

		// 시스템 메세지 전송
		CMessage* ResAttackMagicSystemMessagePacket = G_ObjectManager->GameServer->MakePacketResChattingMessage(_GameObjectInfo.ObjectId, en_MessageType::SYSTEM, st_Color::White(), MagicSystemString);
		G_ObjectManager->GameServer->SendPacketAroundSector(GetCellPosition(), ResAttackMagicSystemMessagePacket);
		ResAttackMagicSystemMessagePacket->Free();	
		
		// HP 변경 전송
		CMessage* ResChangeHPPacket = G_ObjectManager->GameServer->MakePacketResChangeHP(_Target->_GameObjectInfo.ObjectId, _Target->_GameObjectInfo.ObjectStatInfo.HP, _Target->_GameObjectInfo.ObjectStatInfo.MaxHP);
		G_ObjectManager->GameServer->SendPacketAroundSector(GetCellPosition(), ResChangeHPPacket);
		ResChangeHPPacket->Free();	

		// 스펠창 끝
		CMessage* ResMagicPacket = G_ObjectManager->GameServer->MakePacketResMagic(_GameObjectInfo.ObjectId, false);
		G_ObjectManager->GameServer->SendPacketAroundSector(GetCellPosition(), ResMagicPacket);
		ResMagicPacket->Free();
	}
}
