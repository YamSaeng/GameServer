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

void CPlayer::SetAttackMagicType(en_SkillType SkillType, vector<CGameObject*> Targets)
{
	_Targets.clear();

	_Targets = Targets;

	_SkillType = SkillType;

	if (_Targets.size() >= 1)
	{		
		_GameObjectInfo.ObjectPositionInfo.State = en_CreatureState::SPELL;

		// Magic 준비 모션으로 출력
		BroadCastPacket(en_PACKET_S2C_OBJECT_STATE_CHANGE);
		// CastringBar 출력
		BroadCastPacket(en_PACKET_S2C_MAGIC_ATTACK);

		switch (_SkillType)
		{
		case en_SkillType::SKILL_SHAMAN_FIRE:			
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
	// 지정한 AttackTick시간이 될경우
	if (_AttackTick < GetTickCount64())
	{		
		switch (_SkillType)
		{
		case en_SkillType::SKILL_KNIGHT_NORMAL:
		case en_SkillType::SKILL_KNIGHT_CHOHONE:
		case en_SkillType::SKILL_KNIGHT_SHAEHONE:
		case en_SkillType::SKILL_KNIGHT_AROUND_ONE_ATTACK:
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
		wstring AttackMagicSystemString;
		wstring AttackTypeString;
		wstring AttackDamageString;

		wchar_t AttackTypeMessage[64] = L"0";
		wchar_t AttackDamageMessage[64] = L"0";

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

		uniform_int_distribution<int> DamageChoiceRandom(_GameObjectInfo.ObjectStatInfo.MinAttackDamage, _GameObjectInfo.ObjectStatInfo.MaxAttackDamage);
		int32 ChoiceDamage = DamageChoiceRandom(Gen);
		int32 FinalDamage = IsCritical ? ChoiceDamage * 2 : ChoiceDamage;
		
		// 데미지 처리
		_Targets[0]->OnDamaged(this, FinalDamage);

		// 공격 응답
		CMessage* ResAttackMagicPacket = G_ObjectManager->GameServer->MakePacketResAttack(
			_GameObjectInfo.ObjectId,
			_Targets[0]->_GameObjectInfo.ObjectId,
			_SkillType,
			FinalDamage,
			true);
		G_ObjectManager->GameServer->SendPacketAroundSector(_Targets[0]->GetCellPosition(), ResAttackMagicPacket);
		ResAttackMagicPacket->Free();				

		// 공격 메세지 생성
		switch (_SkillType)
		{
		case en_SkillType::SKILL_SHAMAN_FIRE:
			wsprintf(AttackTypeMessage, L"%s가 화염공격을 사용해 %s를 ", _GameObjectInfo.ObjectName.c_str(), _Targets[0]->_GameObjectInfo.ObjectName.c_str());
			AttackTypeString = AttackTypeMessage;			
			
			_GameObjectInfo.ObjectPositionInfo.State = en_CreatureState::IDLE;			
			BroadCastPacket(en_PACKET_TYPE::en_PACKET_S2C_OBJECT_STATE_CHANGE);
			break;
		}				

		wsprintf(AttackDamageMessage, L"%d의 데미지를 줬습니다.", FinalDamage);
		AttackDamageString = AttackDamageMessage;

		AttackMagicSystemString = AttackTypeMessage + AttackDamageString;

		// 메세지 전송
		CMessage* ResAttackMagicSystemMessagePacket = G_ObjectManager->GameServer->MakePacketResChattingMessage(_GameObjectInfo.ObjectId, en_MessageType::SYSTEM, st_Color::Red(), AttackMagicSystemString);
		G_ObjectManager->GameServer->SendPacketAroundSector(GetCellPosition(), ResAttackMagicSystemMessagePacket);
		ResAttackMagicSystemMessagePacket->Free();	
		
		// HP 변경 전송
		CMessage* ResChangeHPPacket = G_ObjectManager->GameServer->MakePacketResChangeHP(_Targets[0]->_GameObjectInfo.ObjectId, _Targets[0]->_GameObjectInfo.ObjectStatInfo.HP, _Targets[0]->_GameObjectInfo.ObjectStatInfo.MaxHP);
		G_ObjectManager->GameServer->SendPacketAroundSector(GetCellPosition(), ResChangeHPPacket);
		ResChangeHPPacket->Free();
	}
}
