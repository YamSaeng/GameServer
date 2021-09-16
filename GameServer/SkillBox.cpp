#include "pch.h"
#include "SkillBox.h"

CSkillBox::CSkillBox()
{
}

CSkillBox::~CSkillBox()
{
    for (auto SkillInfoIterator : _Skills)
    {
        st_SkillInfo* Skilinfo = SkillInfoIterator.second;
        delete Skilinfo;
    }    
}

void CSkillBox::Init()
{

}

void CSkillBox::AddSkill(st_SkillInfo SkillInfo)
{
    st_SkillInfo* NewSkillInfo = new st_SkillInfo();
    *NewSkillInfo = SkillInfo;

    _Skills.insert(pair<byte, st_SkillInfo*>(SkillInfo._SlotIndex, NewSkillInfo));     
}

st_SkillInfo* CSkillBox::Get(int8 SlotIndex)
{
    return nullptr;
}
