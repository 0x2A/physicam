#pragma once


#include "nowork/Common.h"

class Quaternion
{
public:
	NOWORK_API Quaternion();
	NOWORK_API Quaternion(double a, double b, double c, double d);
	NOWORK_API Quaternion(Quaternion const& q);
	NOWORK_API virtual ~Quaternion();
	NOWORK_API void initQuat(double alpha, double x, double y, double z);
	NOWORK_API double magnitude();
	NOWORK_API Quaternion operator*=(Quaternion const& q);
	NOWORK_API Quaternion operator*(Quaternion const& q);
	NOWORK_API glm::mat4 operator*(glm::mat4 const& q);
	NOWORK_API operator glm::mat4();
	NOWORK_API void normalize();
	NOWORK_API Quaternion conjugate();
	NOWORK_API double a() const { return ma; };
	NOWORK_API double b() const { return mb; };
	NOWORK_API double c() const { return mc; };
	NOWORK_API double d() const { return md; };
	NOWORK_API static glm::mat4 Rotate(double alpha, double x, double y, double z);
	
private: 
	double ma, mb, mc, md;
	
};
