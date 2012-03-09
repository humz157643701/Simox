/**
* This file is part of Simox.
*
* Simox is free software; you can redistribute it and/or modify
* it under the terms of the GNU Lesser General Public License as
* published by the Free Software Foundation; either version 2 of
* the License, or (at your option) any later version.
*
* Simox is distributed in the hope that it will be useful, but
* WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU Lesser General Public License for more details.
*
* You should have received a copy of the GNU Lesser General Public License
* along with this program. If not, see <http://www.gnu.org/licenses/>.
*
* @package    VirtualRobot
* @author     Nikolaus Vahrenkamp
* @copyright  2012 Nikolaus Vahrenkamp
*             GNU Lesser General Public License
*
*/
#ifndef _VirtualRobot_VoxelTree6D_h_
#define _VirtualRobot_VoxelTree6D_h_

#include "../VirtualRobotImportExport.h"
#include "../VirtualRobotException.h"
#include "VoxelTree6DElement.hpp"

#include <string>
#include <vector>

namespace VirtualRobot 
{

template <typename T>
class VoxelTree6D
{
public:

	/*!
	*/
	VoxelTree6D(float minExtend[6], float maxExtend[6], float discretizationTransl, float discretizationRot)
	{
		memcpy (&(this->minExtend[0]),&(minExtend[0]),sizeof(float)*6);
		memcpy (&(this->maxExtend[0]),&(maxExtend[0]),sizeof(float)*6);
		if (discretizationTransl<=0.0f || discretizationRot<=0.0f)
		{
			THROW_VR_EXCEPTION ("INVALID parameters");
		}
		float size[6];
		for (int i=0;i<6;i++)
		{
			size[i] = maxExtend[i] - minExtend[i];
		}

		float maxSize = 0;
		for (int i=0;i<3;i++)
		{
			if (size[i] > maxSize)
				maxSize = size[i];
		}
		int steps = (int)(maxSize / discretizationTransl + 0.5f);
		float maxSize2 = 0;
		for (int i=3;i<6;i++)
		{
			if (size[i] > maxSize2)
				maxSize2 = size[i];
		}
		int steps2 = (int)(maxSize2 / discretizationRot + 0.5f);
		if (steps2 > steps)
			steps = steps2;
		maxLevels = steps;
		THROW_VR_EXCEPTION_IF (steps<=0,"Invalid parameters...");
		root = new VoxelTree6DElement<T>(minExtend,size,0,maxLevels);

	}

	virtual ~VoxelTree6D()
	{
		delete root;
	}

	/*!
		Store entry to this voxel grid.
		Creates a leaf if necessary. Existing entries are silently overwritten.
		A copy of e is created.
	*/
	bool setEntry(float pos[6], const T &e)
	{
		return root->setEntry(pos,e);
	}

	/*!
		Returns entry at pos. If pos is outside the workspace representation or no data stored at pos, NULL is returned.
	*/
	T* getEntry(float pos[6])
	{
		return root->getEntry(pos);
	}


	/*!
		Check if the corresponding leaf is present in data structure.
		\Return False if p is outside the extends or if no data is stored for pos
	*/
	//bool isCovered(float pos[6]);

protected:
	float minExtend[6];
	float maxExtend[6];
	int maxLevels;

	VoxelTree6DElement<T> *root;

};



} // namespace

#endif // _VirtualRobot_VoxelTree6D_h_