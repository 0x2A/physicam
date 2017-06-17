#pragma once

#include "nowork/Common.h"
#include "nowork/Transform.h"


class SceneObject
{
public:

	NOWORK_API virtual Transform* GetTransform() { return &m_Transform; }

protected:


	Transform m_Transform;
};