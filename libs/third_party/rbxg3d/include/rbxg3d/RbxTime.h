#ifndef RBX_TIME_H
#define RBX_TIME_H
#include "g3d/G3DGameUnits.h"

namespace RBX
{
	class RbxTime
	{
	public:
		static G3D::RealTime getTick();

	private:
		static G3D::RealTime m_startTime;
	};
}

#endif