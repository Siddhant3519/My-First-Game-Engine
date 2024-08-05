#pragma once
#include <vector>

//--------------------------------------------------------------------------------------------------
template<typename T_TypeOfThingPointedTo>
void ClearAndDeleteEverything(std::vector<T_TypeOfThingPointedTo*>& myVector)
{
	for (int index = 0; index < (int)myVector.size(); ++index)
	{
		delete myVector[index];
	}

	myVector.clear();
}

//--------------------------------------------------------------------------------------------------
template<typename T_TypeOfThingPointedTo>
void NullOutAnyReferencesToThisObject(std::vector<T_TypeOfThingPointedTo*>& myVector, T_TypeOfThingPointedTo* objectToStopPointingTo)
{
	for (int index = 0; index < (int)myVector.size(); ++index)
	{
		if (myVector[index] == objectToStopPointingTo)
		{
			myVector[index] = nullptr;
		}
	}
}