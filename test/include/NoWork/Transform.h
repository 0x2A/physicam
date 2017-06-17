
#pragma once

#include "nowork/Common.h"
#include "nowork/Quaternion.h"


#define RadToDeg 57.2957795130823f
#define DegToRad 0.01745329251994f

class Transform
{

public:
	Transform();
	virtual void Translate(float x, float y, float z, bool world = false);
	virtual void Translate(const glm::vec3 &dir, bool world = false);
	virtual void Transformate(const glm::mat4& mat, bool world = false);
	virtual void SetPosition(const glm::vec3 &pos);
	virtual void SetYPosition(const float &pos);
	virtual void Rotate(float x, float y, float z, bool world = false);
	virtual void Rotate(const glm::vec3 &dir, bool world = false);
	virtual void Rotate(Quaternion &q, bool world = false);
	virtual void Rotate(glm::quat &q, bool world = false);
	virtual void ResetRotation();
	virtual void Scale(const glm::vec3 &scale);
	virtual void Scale(float x, float y, float z);
	virtual void SetScale(float x, float y, float z);
	virtual void SetScale(const glm::vec3 &scale);
	virtual glm::vec3 GetScale();

	static glm::vec4 ScreenSpaceToWorldSpacePoint(glm::ivec2 ssPpoint, glm::mat4 ViewProjectionMatrix, glm::ivec2 viewportSize);


	void Reset();

	virtual glm::vec3 WorldPosition();
	glm::vec3 GetRotation();
	void LookAt(glm::vec3 &targetPosition);
	
	virtual glm::vec3 Up();
	virtual glm::vec3 Right();
	virtual glm::vec3 Forward();
	
	glm::mat4 operator*(glm::mat4 const&);
	glm::mat4 operator*(Transform &);
	
	virtual glm::mat4 GetModelMatrix();

	//overrides the model matrix (useful for custom matrix calculations)
	void SetModelMatrix(glm::mat4&);

protected:

	glm::mat4 m_Transform = glm::mat4(), m_Scale;
	std::vector<Transform*> m_ChildTransforms;
		
};

class ViewTransform : public Transform
{

public:
	ViewTransform();
	virtual void Translate(float x, float y, float z, bool world = false) override;
	virtual void Translate(const glm::vec3 &dir, bool world = false) override;
	virtual void Transformate(const glm::mat4& mat, bool world = false) override;
	virtual void Rotate(float x, float y, float z, bool world = false) override;
	virtual void Rotate(const glm::vec3 &dir, bool world = false) override;

	virtual glm::mat4 GetModelMatrix() override;
	glm::mat4 GetViewMatrix();

	virtual glm::vec3 ViewUp();
	virtual glm::vec3 ViewRight();
	virtual glm::vec3 ViewForward();
};

inline ViewTransform::ViewTransform()
{

}

inline void ViewTransform::Translate(float x, float y, float z, bool world /*= false*/)
{
	Translate(glm::vec3(x, y, z));
}

inline void ViewTransform::Translate(const glm::vec3 &dir, bool world /*= false*/)
{
	glm::mat4 transMatrix = glm::translate(-dir);
	if (world)
		m_Transform = m_Transform * transMatrix;
	else
		m_Transform = transMatrix * m_Transform;
}

inline void ViewTransform::Transformate(const glm::mat4& mat, bool world /*= false*/)
{
	if (world)
		m_Transform = m_Transform * mat;
	else
		m_Transform = mat * m_Transform;
}

inline void ViewTransform::Rotate(float x, float y, float z, bool world /*= false*/)
{
	Rotate(glm::vec3(x, y, z));
}

inline void ViewTransform::Rotate(const glm::vec3 &dir, bool world /*= false*/)
{
	glm::mat4 rotMatrix = glm::rotate(-dir.x, glm::vec3(1, 0, 0));
	rotMatrix *= glm::rotate(-dir.y, glm::vec3(0, 1, 0));
	rotMatrix *= glm::rotate(-dir.z, glm::vec3(0, 0, 1));

	if (world)
		m_Transform = m_Transform * rotMatrix;
	else
		m_Transform = rotMatrix * m_Transform;
}

inline glm::mat4 ViewTransform::GetModelMatrix()
{
	return glm::inverse(m_Transform * m_Scale);
}

inline glm::mat4 ViewTransform::GetViewMatrix()
{
	return m_Transform * m_Scale;
}

inline glm::vec3 ViewTransform::ViewUp()
{
	glm::vec4 up = GetViewMatrix() * glm::vec4(0, 1, 0, 0);
	return glm::vec3(up);
}

inline glm::vec3 ViewTransform::ViewRight()
{
	glm::vec4 r = GetViewMatrix() * glm::vec4(1, 0, 0, 0);
	return glm::vec3(r);
}

inline glm::vec3 ViewTransform::ViewForward()
{
	glm::vec4 r = GetViewMatrix() * glm::vec4(0, 0, 1, 0);
	return glm::vec3(r);
}


inline Transform::Transform()
{}

inline void Transform::SetPosition(const glm::vec3 &pos)
{
	//glm::vec4 oPos = m_Transform * glm::vec4(0,0,0,1);
	//m_Transform *= glm::translate(glm::vec3(-oPos));
	m_Transform = glm::translate(glm::vec3(pos));
}

inline void Transform::SetYPosition(const float &pos)
{
	glm::vec4 oPos = m_Transform * glm::vec4(0,0,0,1);
	oPos.x = oPos.z = 0;
	m_Transform *= glm::translate(glm::vec3(-oPos));
	m_Transform *= glm::translate(glm::vec3(0,pos,0));
}

inline void Transform::Reset()
{
	m_Transform = glm::mat4();
}

inline void Transform::Translate(float x, float y, float z, bool world)
{
	glm::mat4 transMatrix = glm::translate(glm::vec3(x,y,z));
	if(world)
		m_Transform = transMatrix * m_Transform;
	else
		m_Transform = m_Transform * transMatrix;
}

inline void Transform::Translate(const glm::vec3 &dir, bool world)
{
	glm::mat4 transMatrix = glm::translate(dir);
	if(world)
		m_Transform = transMatrix * m_Transform;
	else
		m_Transform = m_Transform * transMatrix;
}

inline void Transform::Rotate(float x, float y, float z, bool world)
{
	glm::mat4 rotMatrix = glm::rotate( x, glm::vec3(1,0,0));
	rotMatrix *= glm::rotate( y, glm::vec3(0,1,0));
	rotMatrix *= glm::rotate( z, glm::vec3(0,0,1));

	if(world)
		m_Transform = rotMatrix * m_Transform;
	else
		m_Transform = m_Transform * rotMatrix;
			
}

inline void Transform::Rotate(const glm::vec3 &rot,bool world)
{
	glm::mat4 rotMatrix = glm::rotate( rot.x, glm::vec3(1,0,0));
	rotMatrix *= glm::rotate( rot.y, glm::vec3(0,1,0));
	rotMatrix *= glm::rotate( rot.z, glm::vec3(0,0,1));

	if(world)	
		m_Transform = rotMatrix * m_Transform;
	else
		m_Transform = m_Transform * rotMatrix;
}

inline void Transform::Rotate(Quaternion& q, bool world)
{
	glm::mat4 rotMatrix = q;
	if (world)
		m_Transform = rotMatrix * m_Transform;
	else
		m_Transform = m_Transform * rotMatrix;
}
inline void Transform::Rotate(glm::quat& q, bool world)
{
	glm::mat4 rotMatrix = glm::toMat4(q);
	if (world)
		m_Transform = rotMatrix * m_Transform;
	else
		m_Transform = m_Transform * rotMatrix;
}

inline void Transform::ResetRotation()
{
	glm::mat4 rot(m_Transform[0][0], m_Transform[0][1], m_Transform[0][2], 0,
		m_Transform[1][0], m_Transform[1][1], m_Transform[1][2], 0.f,
		m_Transform[2][0], m_Transform[2][1], m_Transform[2][2], 0.f,
				0.f, 0.f, 0.f, 1.f);
	rot = glm::inverse(rot);

	m_Transform *= rot;
}


inline void Transform::Scale(const glm::vec3 &scale)
{
	m_Scale *= glm::scale(scale);
}

inline void Transform::Scale(float x, float y, float z)
{
	m_Scale *= glm::scale(glm::vec3(x,y,z));
}

inline  glm::vec3 Transform::GetScale()
{
	return glm::vec3(m_Scale * glm::vec4(1,1,1,0));
}

inline glm::vec3 Transform::WorldPosition()
{
	return glm::vec3(GetModelMatrix() * glm::vec4(0, 0, 0, 1));;
}

inline glm::vec3 Transform::Forward() 
{
	glm::vec4 forward = GetModelMatrix() * glm::vec4(0, 0, -1, 0);
	return glm::vec3(forward);
}

inline glm::vec3 Transform::Right() 
{
	glm::vec4 right = GetModelMatrix() * glm::vec4(1, 0, 0, 0);
	return glm::vec3(right);
}

inline glm::vec3 Transform::Up()
{
	glm::vec4 up = GetModelMatrix() * glm::vec4(0, 1, 0, 0);
	return glm::vec3(up);
}

inline glm::vec3 Transform::GetRotation()
{
	float thetaX, thetaY, thetaZ;
	//Decomposing rotation matrix, upper left part from transform matrix
	if(m_Transform[0][0] == 1.0f)
	{
		thetaY = atan2f(m_Transform[0][2],m_Transform[2][3]);
		thetaX = thetaZ = 0;
	}
	else if (m_Transform[0][0] == -1.0f)
	{
		thetaY = atan2f(m_Transform[0][2],m_Transform[2][3]);
		thetaX = thetaZ = 0;
	}
	else
	{
		thetaY = atan2(-m_Transform[2][0],m_Transform[0][0]);
		thetaZ = asin(m_Transform[1][0]);
		thetaX = atan2(-m_Transform[1][2],m_Transform[1][1]);
	}

	thetaX *= RadToDeg;
	thetaY *= RadToDeg;
	thetaZ *= RadToDeg;
	if(thetaX < 0.0f) thetaX += 360.0f;
	if(thetaY < 0.0f) thetaY += 360.0f;
	if(thetaZ < 0.0f) thetaZ += 360.0f;
	return glm::vec3(thetaX,thetaY,thetaZ);
}

inline glm::mat4 Transform::GetModelMatrix()
{
	return m_Transform * m_Scale;
}

inline void Transform::SetModelMatrix(glm::mat4& mat)
{
	m_Transform = mat;
}

inline void Transform::LookAt(glm::vec3& targetPosition)
{
	m_Transform = glm::lookAt(WorldPosition(),targetPosition,glm::vec3(0,1,0));
}

inline glm::mat4 Transform::operator*(glm::mat4 const& m)
{
	return GetModelMatrix() * m;
}

inline glm::mat4 Transform::operator*(Transform &m)
{
	return GetModelMatrix() * m.GetModelMatrix();
}


inline void Transform::Transformate(const glm::mat4& mat, bool world /*= false*/)
{
	if (world)
		m_Transform = mat * m_Transform;
	else
		m_Transform = m_Transform * mat;
}

inline void Transform::SetScale(float x, float y, float z)
{
	m_Scale = glm::scale(x, y, z);
}

inline void Transform::SetScale(const glm::vec3 &scale)
{
	m_Scale = glm::scale(scale);
}

inline glm::vec4 Transform::ScreenSpaceToWorldSpacePoint(glm::ivec2 ssPpoint, glm::mat4 ViewProjectionMatrix, glm::ivec2 viewportSize)
{
	glm::mat4 vpInverse = ViewProjectionMatrix._inverse();

	glm::vec4 out;
	float winZ = 1.0;

	out[0] = (2.0f*(ssPpoint.x / (float)viewportSize.x)) - 1.0f;
	out[1] = 1.0f - (2.0f*(ssPpoint.y / (float)viewportSize.y));
	out[2] = 2.0* winZ - 1.0;
	out[3] = 1.0;
	out = out * vpInverse;

	out.w = 1.0f / out.w;
	out.x *= out.w;
	out.y *= out.w;
	out.z *= out.w;

	return out;
}
