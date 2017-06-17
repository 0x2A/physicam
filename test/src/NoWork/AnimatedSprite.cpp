#include "nowork/AnimatedSprite.h"


AnimatedSprite::AnimatedSprite()
{
	m_Paused = false;
	m_Stopped = true;
	m_CurrentAnimation = -1;
}


void AnimatedSprite::AddAnimation(SpriteAnimationPtr animation)
{
	m_Animations.push_back(animation);
}

void AnimatedSprite::AddAnimation(const std::vector<SpriteAnimationPtr>& animation)
{
	for (auto animation : animation)
	{
		m_Animations.push_back(animation);
	}
}

AnimatedSpritePtr AnimatedSprite::Create()
{
	AnimatedSpritePtr sprite = AnimatedSpritePtr(new AnimatedSprite);
	return sprite;
}

void AnimatedSprite::SelectAnimation(unsigned int index)
{
	if (index >= m_Animations.size()) //if index out of range
		return; //ignore selection

	m_CurrentAnimation = index;
	m_Animations[index]->Reset();
}

void AnimatedSprite::Play()
{
	if (m_CurrentAnimation <= -1)
		return;

	m_Paused = false;
	m_Stopped = false;
}

void AnimatedSprite::Play(unsigned int index)
{
	m_CurrentAnimation = index;
	m_Paused = false;
	m_Stopped = false;
}

void AnimatedSprite::Pause()
{
	m_Paused = true;
}

void AnimatedSprite::Stop()
{
	m_Stopped = true;
	if (m_CurrentAnimation > -1)
		m_Animations[m_CurrentAnimation]->Reset();
}

void AnimatedSprite::Render()
{
	if (m_CurrentAnimation <= -1)
		return;

	m_Animations[m_CurrentAnimation]->Render(m_Transform.GetModelMatrix());
}

void AnimatedSprite::Update(double deltaTime)
{
	if (m_Paused || m_Stopped || m_CurrentAnimation <= -1)
		return;

	m_Animations[m_CurrentAnimation]->Update(deltaTime);
}

SpriteAnimationPtr AnimatedSprite::GetAnimation(unsigned int index)
{
	if (index >= m_Animations.size()) //if index out of range
		return nullptr; //return null

	return m_Animations[index];
}

SpriteAnimationPtr AnimatedSprite::GetCurrentAnimation()
{
	if (m_CurrentAnimation <= -1)
		return nullptr;

	return m_Animations[m_CurrentAnimation];
}

void AnimatedSprite::Loop(bool b)
{
	if (m_CurrentAnimation <= -1)
		return;

	m_Animations[m_CurrentAnimation]->Loop(b);
}

void AnimatedSprite::Loop(unsigned int index, bool b)
{
	auto anim = GetAnimation(index);
	if (!anim) return;
	anim->Loop(b);
}
