#pragma once


//--------------------------------------------------------------------------------------------------
#include "Engine/Core/Vertex_PCUTBN.hpp"
#include "Engine/Core/Vertex_PCU.hpp"
#include "Engine/Math/Mat44.hpp"


//--------------------------------------------------------------------------------------------------
#include <vector>
#include <string>


//--------------------------------------------------------------------------------------------------
class OBJLoader
{
private:
	static void ParseAndRemoveNewLineAndCarraigeReturnFromFile(std::string const& fileName, Strings& out_delimitedList);
public:
	static void LoadOBJFileByName(std::string const& fileName, std::vector<Vertex_PCU>& out_verts, std::vector<unsigned int>& out_indexes, Mat44 const& transformFixUpMat);
	static void LoadOBJFileByName(std::string const& fileName, std::vector<Vertex_PCUTBN>& out_verts, std::vector<unsigned int>& out_indexes, Mat44 const& transformFixUpMat);
};