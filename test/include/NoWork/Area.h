#pragma once
#include "nowork/Common.h"

template<typename T>
class Area
{
public:

	Area()
	{}

	Area(T posX1, T posY1, T posX2, T posY2) : m_Vals(posX1, posY1, posX2, posY2)
	{}

	Area(glm::detail::tvec4<T> vals) : m_Vals(vals)
	{}

	T Width()
	{
		return glm::abs(m_Vals.x - m_Vals.z);
	}
	T Height()
	{
		return glm::abs(m_Vals.y - m_Vals.w);
	}

	T X1()
	{
		return m_Vals.x;
	}
	
	T X2()
	{
		return m_Vals.z;
	}

	T Y1()
	{
		return m_Vals.y;
	}

	T Y2()
	{
		return m_Vals.w;
	}



private:
	glm::detail::tvec4<T> m_Vals;
};
