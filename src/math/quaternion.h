/*********************************************************************
 *           Copyright (C) 2016 mwl4 - All rights reserved           *
 *********************************************************************
 * File       : quaternion.h
 * Project    : ConverterPIX
 * Developers : Michal Wojtowicz (mwl450@gmail.com)
 			  : Piotr Krupa (piotrkrupa06@gmail.com)
 *********************************************************************/

#pragma once

#include <prerequisites.h>
#include "vector.h"

#include <glm/gtc/quaternion.hpp>

#pragma pack(push, 1)

namespace prism
{
	class quat_t
	{
	public:
		float m_w;
		float m_x;
		float m_y;
		float m_z;

	public:
		quat_t()
		{
			m_w = 1.f;
			m_x = m_y = m_z = 0.f;
		}
		quat_t(float w, float x, float y, float z)
		{
			m_w = w;
			m_x = x;
			m_y = y;
			m_z = z;
		}	
		void set(const float3& axis, float angle)
		{
			const float half_angle = angle * 0.5f;
			m_x = axis[0] * sinf(half_angle);
			m_y = axis[1] * sinf(half_angle);
			m_z = axis[2] * sinf(half_angle);
			m_w = cosf(half_angle);
		}
		float length() const
		{
			return m_x * m_x + m_y * m_y + m_z * m_z;
		}
		float sq_length() const
		{
			return sqrtf(length());
		}
		void normalize()
		{
			const float len = sq_length();
			m_x /= len;
			m_y /= len;
			m_z /= len;
		}
		quat_t normalized() const
		{
			quat_t result(*this);
			result.normalize();
			return result;
		}
	};	ENSURE_SIZE(quat_t, 16);

	static glm::quat glm_cast(const quat_t &quat)
	{
		return glm::quat(quat.m_w, quat.m_x, quat.m_y, quat.m_z);
	}

	static std::string to_string(const prism::quat_t &quat)
	{
		return fmt::sprintf(
			FLT_FT "  " FLT_FT "  " FLT_FT "  " FLT_FT,
			flh(quat.m_w), flh(quat.m_x), flh(quat.m_y), flh(quat.m_z)
		);
	}
} // namespace prism

typedef prism::quat_t Quaternion;

#pragma pack(pop)

/* eof */
