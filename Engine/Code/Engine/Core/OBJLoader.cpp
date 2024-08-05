#include "Engine/Core/Time.hpp"
#include "Engine/Core/FileUtils.hpp"
#include "Engine/Core/OBJLoader.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Core/VertexUtils.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"


//--------------------------------------------------------------------------------------------------
void OBJLoader::LoadOBJFileByName(std::string const& fileName, std::vector<Vertex_PCU>& out_verts, std::vector<unsigned int>& out_indexes, Mat44 const& transformFixUpMat)
{
	if (!DoesFileExist(fileName))
	{
		return;
	}

	Strings newLineAndCarriageReturnDelimitedList;
	double timeBeforeParsingOBJFile = GetCurrentTimeSeconds();
	ParseAndRemoveNewLineAndCarraigeReturnFromFile(fileName, newLineAndCarriageReturnDelimitedList);
	double timeAfterParsingOBJFile					= GetCurrentTimeSeconds();
	double timeTakenToParseAndLoadFileIntoMemory	= timeAfterParsingOBJFile - timeBeforeParsingOBJFile;
	// Populate the vertex PNCU
	std::vector<Vec3> vertexPositionList;
	std::vector<Vec3> vertexNormalList;
	std::vector<Vec2> vertexTexCoordList;
	
	int numOfFaces		= 0;
	int numOfTriangles	= 0;

	out_verts.reserve(newLineAndCarriageReturnDelimitedList.size());

	double timeBeforeCreatingVertexesAndIndexes = GetCurrentTimeSeconds();
	for (int stringIndex = 0; stringIndex < (int)newLineAndCarriageReturnDelimitedList.size(); ++stringIndex)
	{
		std::string currentString	=	newLineAndCarriageReturnDelimitedList[stringIndex];
		if (currentString.size() < 2) continue;
		unsigned char firstChar		=	currentString[0];
		unsigned char secondChar	=	currentString[1];
		if (firstChar == '#') continue;

		if (firstChar == 'v' && secondChar == ' ')
		{
			Strings spaceDelimitedList = SplitStringOnDelimiter(currentString, ' ');
			int spaceDelimitedListSize = (int)spaceDelimitedList.size();
			if (spaceDelimitedListSize > 3)
			{
				float x = (float)atof(spaceDelimitedList[size_t(spaceDelimitedListSize) - 3].c_str());
				float y = (float)atof(spaceDelimitedList[size_t(spaceDelimitedListSize) - 2].c_str());
				float z = (float)atof(spaceDelimitedList[size_t(spaceDelimitedListSize) - 1].c_str());
				vertexPositionList.emplace_back(x, y, z);
			}
			continue;
		}

		if (firstChar == 'v' && secondChar == 'n')
		{
			Strings spaceDelimitedList = SplitStringOnDelimiter(currentString, ' ');
			int spaceDelimitedListSize = (int)spaceDelimitedList.size();
			if (spaceDelimitedListSize > 3)
			{
				float x = (float)atof(spaceDelimitedList[size_t(spaceDelimitedListSize) - 3].c_str());
				float y = (float)atof(spaceDelimitedList[size_t(spaceDelimitedListSize) - 2].c_str());
				float z = (float)atof(spaceDelimitedList[size_t(spaceDelimitedListSize) - 1].c_str());
				vertexNormalList.emplace_back(x, y, z);
			}
			continue;
		}
		
		if (firstChar == 'v' && secondChar == 't')
		{
			Strings spaceDelimitedList = SplitStringOnDelimiter(currentString, ' ');
			int spaceDelimitedListSize = (int)spaceDelimitedList.size();
			if (spaceDelimitedListSize > 3)
			{
				float u = (float)atof(spaceDelimitedList[size_t(spaceDelimitedListSize) - 3].c_str());
				float v = (float)atof(spaceDelimitedList[size_t(spaceDelimitedListSize) - 2].c_str());
				vertexTexCoordList.emplace_back(u, v);
			}
			else if (spaceDelimitedListSize > 2)
			{
				float u = (float)atof(spaceDelimitedList[size_t(spaceDelimitedListSize) - 2].c_str());
				float v = (float)atof(spaceDelimitedList[size_t(spaceDelimitedListSize) - 1].c_str());
				vertexTexCoordList.emplace_back(u, v);
			}
			continue;
		}
		
		if (firstChar == 'f' && secondChar == ' ')
		{
			Strings spaceDelimitedList	=	SplitStringOnDelimiter(currentString, ' ');
			int spaceDelimitedListSize	=	(int)spaceDelimitedList.size(); 
			// unsigned int indexCount		=	0;
			std::vector<Vec3> reorderedPosList;
			std::vector<Vec3> reorderedNormalList;
			std::vector<Vec2> reorderedUVList;

			for (int subStringIndex	= 1; subStringIndex < spaceDelimitedListSize; ++subStringIndex)
			{
				std::string currentSubString		=	spaceDelimitedList[size_t(subStringIndex)];
				if (currentSubString == "") continue;
				Strings forwardSlashDelimitedList	=	SplitStringOnDelimiter(currentSubString, '/');
				std::string subString1				=	forwardSlashDelimitedList[0];
				
				Vec3 pos;
				int indexIntoPositionList = atoi(subString1.c_str()) - 1;
				pos = vertexPositionList[indexIntoPositionList];
				reorderedPosList.emplace_back(pos);
				++numOfFaces;
				Vec3 normal;
				Vec2 uv;
				if (forwardSlashDelimitedList.size() > 1)
				{
					std::string subString2 = forwardSlashDelimitedList[1];
					std::string subString3 = forwardSlashDelimitedList[2];

					if (subString2 != "")
					{
						int indexIntoTextureCoordList	=		atoi(subString2.c_str()) - 1;
						uv								=		vertexTexCoordList[indexIntoTextureCoordList];
						reorderedUVList.emplace_back(uv);
					}
					if (subString3 != "")
					{
						int indexIntoNormalList		=	atoi(subString3.c_str()) - 1;
						normal						=	vertexNormalList[indexIntoNormalList];
						reorderedNormalList.emplace_back(normal);
					}
				}
			}
			if (reorderedNormalList.size() == 0 && reorderedUVList.size() == 0)
			{
				// #ToDo: Rename
				Vec3 const& vert1	=	reorderedPosList[size_t(0)];
				size_t nextIndex	=	1;
				while (nextIndex < reorderedPosList.size() - 1)
				{
					Vec3 const& vert2	=	reorderedPosList[nextIndex];
					Vec3 const& vert3	=	reorderedPosList[nextIndex + 1];

					Vec3 dispFromVert1To2 = vert2 - vert1;
					Vec3 dispFromVert2To3 = vert3 - vert2;
					Vec3 vertexNormal	  = CrossProduct3D(dispFromVert1To2, dispFromVert2To3);
					vertexNormal.Normalize();

					Vec3 tangents;
					Vec3 binormals;

					int vertIndex = (int)out_verts.size();
					out_verts.emplace_back(reorderedPosList[size_t(0)],		Rgba8::WHITE, Vec2());
					out_verts.emplace_back(reorderedPosList[nextIndex],		Rgba8::WHITE, Vec2());
					out_verts.emplace_back(reorderedPosList[nextIndex + 1], Rgba8::WHITE, Vec2());

					out_indexes.emplace_back((unsigned int)vertIndex);
					out_indexes.emplace_back((unsigned int)vertIndex + 1);
					out_indexes.emplace_back((unsigned int)vertIndex + 2);
					
					++numOfTriangles;
					nextIndex += 1;
				}
			}
			else if (reorderedNormalList.size() != 0)
			{
				if (reorderedUVList.size() == 0)
				{
					Vec3 const& vert1	= reorderedPosList[size_t(0)];

					size_t nextIndex = 1;
					while (nextIndex < reorderedPosList.size() - 1)
					{
						Vec3 const& vert2 = reorderedPosList[nextIndex];
						Vec3 const& vert3 = reorderedPosList[nextIndex + 1];

						int vertIndex = (int)out_verts.size();
						out_verts.emplace_back(vert1, Rgba8::WHITE, Vec2());
						out_verts.emplace_back(vert2, Rgba8::WHITE, Vec2());
						out_verts.emplace_back(vert3, Rgba8::WHITE, Vec2());

						out_indexes.emplace_back((unsigned int)vertIndex);
						out_indexes.emplace_back((unsigned int)vertIndex + 1);
						out_indexes.emplace_back((unsigned int)vertIndex + 2);

						++numOfTriangles;
						nextIndex += 1;
					}
				}
				else
				{
					Vec3 const& vert1	= reorderedPosList[size_t(0)];
					Vec2 const& uv1		= reorderedUVList[size_t(0)];

					size_t nextIndex = 1;
					while (nextIndex < reorderedPosList.size() - 1)
					{
						Vec3 const& vert2 = reorderedPosList[nextIndex];
						Vec3 const& vert3 = reorderedPosList[nextIndex + 1];

						Vec2 const& uv2 = reorderedUVList[nextIndex];
						Vec2 const& uv3 = reorderedUVList[nextIndex + 1];

						int vertIndex = (int)out_verts.size();
						out_verts.emplace_back(vert1, Rgba8::WHITE, uv1);
						out_verts.emplace_back(vert2, Rgba8::WHITE, uv2);
						out_verts.emplace_back(vert3, Rgba8::WHITE, uv3);

						out_indexes.emplace_back((unsigned int)vertIndex);
						out_indexes.emplace_back((unsigned int)vertIndex + 1);
						out_indexes.emplace_back((unsigned int)vertIndex + 2);

						++numOfTriangles;
						nextIndex += 1;
					}
				}
			}
		}
	}

	if (out_verts.size() == 0)
	{
		for (int vertexIndex = 0; vertexIndex < (int)vertexPositionList.size(); vertexIndex += 3)
		{
			Vec3 const& vert1 = vertexPositionList[size_t(vertexIndex)];
			Vec3 const& vert2 = vertexPositionList[size_t(vertexIndex) + 1];
			Vec3 const& vert3 = vertexPositionList[size_t(vertexIndex) + 2];
		
			Vec3 dispFromVert1To2 = vert2 - vert1;
			Vec3 dispFromVert2To3 = vert3 - vert2;
			Vec3 vertexNormal = CrossProduct3D(dispFromVert1To2, dispFromVert2To3);
			vertexNormal.Normalize();

			Vec3 tangent;
			Vec3 binormal;

			out_verts.emplace_back(vertexPositionList[size_t(vertexIndex)],		Rgba8::WHITE, Vec2());
			out_verts.emplace_back(vertexPositionList[size_t(vertexIndex) + 1], Rgba8::WHITE, Vec2());
			out_verts.emplace_back(vertexPositionList[size_t(vertexIndex) + 2], Rgba8::WHITE, Vec2());

			out_indexes.emplace_back((unsigned int)vertexIndex);
			out_indexes.emplace_back((unsigned int)vertexIndex + 1);
			out_indexes.emplace_back((unsigned int)vertexIndex + 2);

			++numOfTriangles;
		}
	}
	double timeAfterCreatingVertexesAndIndexes = GetCurrentTimeSeconds();
	double timeTakenToCreateVertexesAndIndexes = timeAfterCreatingVertexesAndIndexes - timeBeforeCreatingVertexesAndIndexes;
	TransformVertexArray3D(out_verts, transformFixUpMat);

	DebuggerPrintf("\n--------------------------------------------------------------------------------------------------");
	DebuggerPrintf("\nLoaded .obj file %s", fileName.c_str());
	DebuggerPrintf("\n[file data]    vertexes: %d    texture coordinates: %d    normals: %d    faces: %d    triangles: %d", (int)vertexPositionList.size(), (int)vertexTexCoordList.size(), (int)vertexNormalList.size(), numOfFaces, numOfTriangles);
	DebuggerPrintf("\n[loaded mesh]  vertexes: %d    indexes: %d", (int)out_verts.size(), (int)out_indexes.size());
	DebuggerPrintf("\n[time]         parse: %f    create: %f", timeTakenToParseAndLoadFileIntoMemory, timeTakenToCreateVertexesAndIndexes);
	DebuggerPrintf("\n--------------------------------------------------------------------------------------------------\n\n");
}


//--------------------------------------------------------------------------------------------------
void OBJLoader::LoadOBJFileByName(std::string const& fileName, std::vector<Vertex_PCUTBN>& out_verts, std::vector<unsigned int>& out_indexes, Mat44 const& transformFixUpMat)
{	
	if (!DoesFileExist(fileName))
	{
		return;
	}

	Strings newLineAndCarriageReturnDelimitedList;
	double timeBeforeParsingOBJFile = GetCurrentTimeSeconds();
	ParseAndRemoveNewLineAndCarraigeReturnFromFile(fileName, newLineAndCarriageReturnDelimitedList);
	double timeAfterParsingOBJFile					= GetCurrentTimeSeconds();
	double timeTakenToParseAndLoadFileIntoMemory	= timeAfterParsingOBJFile - timeBeforeParsingOBJFile;
	// Populate the vertex PNCU
	std::vector<Vec3> vertexPositionList;
	std::vector<Vec3> vertexNormalList;
	std::vector<Vec2> vertexTexCoordList;
	
	int numOfFaces		= 0;
	int numOfTriangles	= 0;

	out_verts.reserve(newLineAndCarriageReturnDelimitedList.size());

	double timeBeforeCreatingVertexesAndIndexes = GetCurrentTimeSeconds();
	for (int stringIndex = 0; stringIndex < (int)newLineAndCarriageReturnDelimitedList.size(); ++stringIndex)
	{
		std::string currentString	=	newLineAndCarriageReturnDelimitedList[stringIndex];
		if (currentString.size() < 2) continue;
		// Strings spaceDelimitedList	=	SplitStringOnDelimiter(currentString, ' ');
		unsigned char firstChar		=	currentString[0];
		unsigned char secondChar	=	currentString[1];
		if (firstChar == '#') continue;

		if (firstChar == 'v' && secondChar == ' ')
		{
			Strings spaceDelimitedList = SplitStringOnDelimiter(currentString, ' ');
			int spaceDelimitedListSize = (int)spaceDelimitedList.size();
			if (spaceDelimitedListSize > 3)
			{
				float x = (float)atof(spaceDelimitedList[size_t(spaceDelimitedListSize) - 3].c_str());
				float y = (float)atof(spaceDelimitedList[size_t(spaceDelimitedListSize) - 2].c_str());
				float z = (float)atof(spaceDelimitedList[size_t(spaceDelimitedListSize) - 1].c_str());
				vertexPositionList.emplace_back(x, y, z);
			}
			continue;
		}

		if (firstChar == 'v' && secondChar == 'n')
		{
			Strings spaceDelimitedList = SplitStringOnDelimiter(currentString, ' ');
			int spaceDelimitedListSize = (int)spaceDelimitedList.size();
			if (spaceDelimitedListSize > 3)
			{
				float x = (float)atof(spaceDelimitedList[size_t(spaceDelimitedListSize) - 3].c_str());
				float y = (float)atof(spaceDelimitedList[size_t(spaceDelimitedListSize) - 2].c_str());
				float z = (float)atof(spaceDelimitedList[size_t(spaceDelimitedListSize) - 1].c_str());
				vertexNormalList.emplace_back(x, y, z);
			}
			continue;
		}
		
		if (firstChar == 'v' && secondChar == 't')
		{
			Strings spaceDelimitedList = SplitStringOnDelimiter(currentString, ' ');
			int spaceDelimitedListSize = (int)spaceDelimitedList.size();
			if (spaceDelimitedListSize > 3)
			{
				float u = (float)atof(spaceDelimitedList[size_t(spaceDelimitedListSize) - 3].c_str());
				float v = (float)atof(spaceDelimitedList[size_t(spaceDelimitedListSize) - 2].c_str());
				vertexTexCoordList.emplace_back(u, v);
			}
			else if (spaceDelimitedListSize > 2)
			{
				float u = (float)atof(spaceDelimitedList[size_t(spaceDelimitedListSize) - 2].c_str());
				float v = (float)atof(spaceDelimitedList[size_t(spaceDelimitedListSize) - 1].c_str());
				vertexTexCoordList.emplace_back(u, v);
			}
			continue;
		}
		
		if (firstChar == 'f' && secondChar == ' ')
		{
			Strings spaceDelimitedList	=	SplitStringOnDelimiter(currentString, ' ');
			int spaceDelimitedListSize	=	(int)spaceDelimitedList.size(); 
			// unsigned int indexCount		=	0;
			std::vector<Vec3> reorderedPosList;
			std::vector<Vec3> reorderedNormalList;
			std::vector<Vec2> reorderedUVList;

			for (int subStringIndex	= 1; subStringIndex < spaceDelimitedListSize; ++subStringIndex)
			{
				std::string currentSubString		=	spaceDelimitedList[size_t(subStringIndex)];
				if (currentSubString == "") continue;
				Strings forwardSlashDelimitedList	=	SplitStringOnDelimiter(currentSubString, '/');
				std::string subString1				=	forwardSlashDelimitedList[0];
				
				Vec3 pos;
				int indexIntoPositionList = atoi(subString1.c_str()) - 1;
				pos = vertexPositionList[indexIntoPositionList];
				reorderedPosList.emplace_back(pos);
				++numOfFaces;
				Vec3 normal;
				Vec2 uv;
				if (forwardSlashDelimitedList.size() > 1)
				{
					std::string subString2 = forwardSlashDelimitedList[1];
					std::string subString3 = forwardSlashDelimitedList[2];

					if (subString2 != "")
					{
						int indexIntoTextureCoordList =		atoi(subString2.c_str()) - 1;
						uv							  =		vertexTexCoordList[indexIntoTextureCoordList];
						reorderedUVList.emplace_back(uv);
					}
					if (subString3 != "")
					{
						int indexIntoNormalList	=	atoi(subString3.c_str()) - 1;
						normal					=	vertexNormalList[indexIntoNormalList];
						reorderedNormalList.emplace_back(normal);
					}
				}
				// out_verts.emplace_back(pos, normal, Rgba8::WHITE, uv);
				// out_indexes.emplace_back(indexCount);
				// ++indexCount;
			}
			if (reorderedNormalList.size() == 0 && reorderedUVList.size() == 0)
			{
				// #ToDo: Rename
				Vec3 const& vert1	= reorderedPosList[size_t(0)];
				size_t nextIndex = 1;
				// for (int vertexIndex = 0; vertexIndex < (int)reorderedPosList.size(); vertexIndex += 3)
				while (nextIndex < reorderedPosList.size() - 1)
				{
					Vec3 const& vert2	= reorderedPosList[nextIndex];
					Vec3 const& vert3	= reorderedPosList[nextIndex + 1];

					Vec3 dispFromVert1To2	= vert2 - vert1;
					Vec3 dispFromVert2To3	= vert3 - vert2;
					Vec3 vertexNormal		= CrossProduct3D(dispFromVert1To2, dispFromVert2To3);
					vertexNormal.Normalize();

					Vec3 tangents;
					Vec3 binormals;

					int vertIndex = (int)out_verts.size();
					out_verts.emplace_back(reorderedPosList[size_t(0)],		Rgba8::WHITE, Vec2(), tangents, binormals, vertexNormal);
					out_verts.emplace_back(reorderedPosList[nextIndex],		Rgba8::WHITE, Vec2(), tangents, binormals, vertexNormal);
					out_verts.emplace_back(reorderedPosList[nextIndex + 1], Rgba8::WHITE, Vec2(), tangents, binormals, vertexNormal);

					out_indexes.emplace_back((unsigned int)vertIndex);
					out_indexes.emplace_back((unsigned int)vertIndex + 1);
					out_indexes.emplace_back((unsigned int)vertIndex + 2);
					
					++numOfTriangles;
					nextIndex += 1;
				}
			}
			else if (reorderedNormalList.size() != 0)
			{
				if (reorderedUVList.size() == 0)
				{
					Vec3 const& vert1	= reorderedPosList[size_t(0)];
					Vec3 const& normal1 = reorderedNormalList[size_t(0)];

					size_t nextIndex = 1;
					while (nextIndex < reorderedPosList.size() - 1)
					{
						Vec3 const& vert2 = reorderedPosList[nextIndex];
						Vec3 const& vert3 = reorderedPosList[nextIndex + 1];

						Vec3 const& normal2 = reorderedNormalList[nextIndex];
						Vec3 const& normal3 = reorderedNormalList[nextIndex + 1];

						Vec3 binormal;
						Vec3 tangent;

						int vertIndex = (int)out_verts.size();
						out_verts.emplace_back(vert1, Rgba8::WHITE, Vec2(), tangent, binormal, normal1);
						out_verts.emplace_back(vert2, Rgba8::WHITE, Vec2(), tangent, binormal, normal2);
						out_verts.emplace_back(vert3, Rgba8::WHITE, Vec2(), tangent, binormal, normal3);

						out_indexes.emplace_back((unsigned int)vertIndex);
						out_indexes.emplace_back((unsigned int)vertIndex + 1);
						out_indexes.emplace_back((unsigned int)vertIndex + 2);

						++numOfTriangles;
						nextIndex += 1;
					}
					//for (int vertexIndex = 0; vertexIndex < (int)reorderedPosList.size(); ++vertexIndex)
					//{
					//	int vertIndex = int(out_verts.size());
					//	out_verts.emplace_back(reorderedPosList[size_t(vertexIndex)], reorderedNormalList[size_t(vertexIndex)], Rgba8::WHITE);
					//	out_indexes.emplace_back((unsigned int)vertIndex);
					//}
				}
				else
				{
					Vec3 const& vert1	= reorderedPosList[size_t(0)];
					Vec3 const& normal1 = reorderedNormalList[size_t(0)];
					Vec2 const& uv1		= reorderedUVList[size_t(0)];

					size_t nextIndex = 1;
					while (nextIndex < reorderedPosList.size() - 1)
					{
						Vec3 const& vert2 = reorderedPosList[nextIndex];
						Vec3 const& vert3 = reorderedPosList[nextIndex + 1];

						Vec3 const& normal2 = reorderedNormalList[nextIndex];
						Vec3 const& normal3 = reorderedNormalList[nextIndex + 1];

						Vec2 const& uv2 = reorderedUVList[nextIndex];
						Vec2 const& uv3 = reorderedUVList[nextIndex + 1];

						Vec3 tangent;
						Vec3 binormal;

						int vertIndex = (int)out_verts.size();
						out_verts.emplace_back(vert1, Rgba8::WHITE, uv1, tangent, binormal, normal1);
						out_verts.emplace_back(vert2, Rgba8::WHITE, uv2, tangent, binormal, normal2);
						out_verts.emplace_back(vert3, Rgba8::WHITE, uv3, tangent, binormal, normal3);

						out_indexes.emplace_back((unsigned int)vertIndex);
						out_indexes.emplace_back((unsigned int)vertIndex + 1);
						out_indexes.emplace_back((unsigned int)vertIndex + 2);

						++numOfTriangles;
						nextIndex += 1;
					}
					// for (int vertexIndex = 0; vertexIndex < (int)reorderedPosList.size(); ++vertexIndex)
					// {
					// 	int vertIndex = int(out_verts.size());
					// 	out_verts.emplace_back(reorderedPosList[size_t(vertexIndex)], reorderedNormalList[size_t(vertexIndex)], Rgba8::WHITE, reorderedUVList[size_t(vertexIndex)]);
					// 	out_indexes.emplace_back((unsigned int)vertIndex);
					// 
					// }
				}
			}
		}

		/// if (firstChar == 'v')
		/// {
		/// 	if (secondChar == ' ')
		/// 	{
		/// 		Strings spaceDelimitedList	=	SplitStringOnDelimiter(currentString, ' ');
		/// 		int spaceDelimitedListSize	=	(int)spaceDelimitedList.size();
		/// 		if (spaceDelimitedListSize > 3)
		/// 		{
		/// 			std::string test1 =	spaceDelimitedList[size_t(spaceDelimitedListSize) - 3];
		/// 			
		/// 			float x		=   (float)atof(spaceDelimitedList[size_t(spaceDelimitedListSize) - 3].c_str());
		/// 			float y		=	(float)atof(spaceDelimitedList[size_t(spaceDelimitedListSize) - 2].c_str());
		/// 			float z		=	(float)atof(spaceDelimitedList[size_t(spaceDelimitedListSize) - 1].c_str());
		/// 			vertexPositionList.emplace_back(x, y, z);
		/// 		}
		/// 	}
		/// 	else if (secondChar == 't')
		/// 	{
		/// 		// Texture Coords
		/// 	}
		/// 	else if (secondChar == 'n')
		/// 	{
		/// 		// Vertex Normal
		/// 	}
		/// }
		/// else if (firstChar == 'f')
		/// {
		/// 	// Face elements
		/// }
	}

	if (out_verts.size() == 0)
	{
		for (int vertexIndex = 0; vertexIndex < (int)vertexPositionList.size(); vertexIndex += 3)
		{
			Vec3 const& vert1 = vertexPositionList[size_t(vertexIndex)];
			Vec3 const& vert2 = vertexPositionList[size_t(vertexIndex) + 1];
			Vec3 const& vert3 = vertexPositionList[size_t(vertexIndex) + 2];
		
			Vec3 dispFromVert1To2 = vert2 - vert1;
			Vec3 dispFromVert2To3 = vert3 - vert2;
			Vec3 vertexNormal = CrossProduct3D(dispFromVert1To2, dispFromVert2To3);
			vertexNormal.Normalize();

			Vec3 tangent;
			Vec3 binormal;

			out_verts.emplace_back(vertexPositionList[size_t(vertexIndex)],		Rgba8::WHITE, Vec2(), tangent, binormal, vertexNormal);
			out_verts.emplace_back(vertexPositionList[size_t(vertexIndex) + 1], Rgba8::WHITE, Vec2(), tangent, binormal, vertexNormal);
			out_verts.emplace_back(vertexPositionList[size_t(vertexIndex) + 2], Rgba8::WHITE, Vec2(), tangent, binormal, vertexNormal);

			out_indexes.emplace_back((unsigned int)vertexIndex);
			out_indexes.emplace_back((unsigned int)vertexIndex + 1);
			out_indexes.emplace_back((unsigned int)vertexIndex + 2);

			++numOfTriangles;
		}
	}
	double timeAfterCreatingVertexesAndIndexes = GetCurrentTimeSeconds();
	double timeTakenToCreateVertexesAndIndexes = timeAfterCreatingVertexesAndIndexes - timeBeforeCreatingVertexesAndIndexes;
	TransformVertexArray3D(out_verts, transformFixUpMat);

	DebuggerPrintf("\n--------------------------------------------------------------------------------------------------");
	DebuggerPrintf("\nLoaded .obj file %s", fileName.c_str());
	DebuggerPrintf("\n[file data]    vertexes: %d    texture coordinates: %d    normals: %d    faces: %d    triangles: %d", (int)vertexPositionList.size(), (int)vertexTexCoordList.size(), (int)vertexNormalList.size(), numOfFaces, numOfTriangles);
	DebuggerPrintf("\n[loaded mesh]  vertexes: %d    indexes: %d", (int)out_verts.size(), (int)out_indexes.size());
	DebuggerPrintf("\n[time]         parse: %f    create: %f", timeTakenToParseAndLoadFileIntoMemory, timeTakenToCreateVertexesAndIndexes);
	DebuggerPrintf("\n--------------------------------------------------------------------------------------------------\n\n");

	CalculateTangentSpaceVectors(out_verts, out_indexes);

	// if (faceList.size() == 0)
	// {
	// 	if (vertexNormalList.size() == 0)
	// 	{
	// 		for (int vertexIndex = 0; vertexIndex < (int)vertexPositionList.size(); vertexIndex += 3)
	// 		{
	// 			Vec3 const& vert1 = vertexPositionList[size_t(vertexIndex)];
	// 			Vec3 const& vert2 = vertexPositionList[size_t(vertexIndex) + 1];
	// 			Vec3 const& vert3 = vertexPositionList[size_t(vertexIndex) + 2];
	// 
	// 			Vec3 dispFromVert1To2 = vert2 - vert1;
	// 			Vec3 dispFromVert2To3 = vert3 - vert2;
	// 			Vec3 vertexNormal = CrossProduct3D(dispFromVert1To2, dispFromVert2To3);
	// 			vertexNormal.Normalize();
	// 			vertexNormalList.emplace_back(vertexNormal);
	// 			vertexNormalList.emplace_back(vertexNormal);
	// 			vertexNormalList.emplace_back(vertexNormal);
	// 		}
	// 	}
	// 
	// 	if (vertexTexCoordList.size() == 0)
	// 	{
	// 		for (int vertexIndex = 0; vertexIndex < (int)vertexPositionList.size(); vertexIndex += 3)
	// 		{
	// 			Vec2 defaultTexCoord;
	// 			vertexTexCoordList.emplace_back(defaultTexCoord);
	// 			vertexTexCoordList.emplace_back(defaultTexCoord);
	// 			vertexTexCoordList.emplace_back(defaultTexCoord);
	// 
	// 			// #Temp: figure out where to put this and how to use it
	// 			out_indexes.emplace_back((unsigned int)(vertexIndex));
	// 			out_indexes.emplace_back((unsigned int)(vertexIndex) + 1);
	// 			out_indexes.emplace_back((unsigned int)(vertexIndex) + 2);
	// 		}
	// 	}
	// 
	// 	for (int vertexIndex = 0; vertexIndex < (int)vertexPositionList.size(); ++vertexIndex)
	// 	{
	// 		out_verts.emplace_back(vertexPositionList[size_t(vertexIndex)], vertexNormalList[size_t(vertexIndex)], Rgba8::WHITE);
	// 	}
	// }
}


//--------------------------------------------------------------------------------------------------
void OBJLoader::ParseAndRemoveNewLineAndCarraigeReturnFromFile(std::string const& fileName, Strings& out_delimitedList)
{
	// Read file contents
	std::string objFileContents;
	FileReadToString(objFileContents, fileName);

	// Split strings on newline and carriage return characters (\n and \r respectively)
	Strings newLineDelimitedList = SplitStringOnDelimiter(objFileContents, '\r');
	// #ToDo: Should this be one for loop where I split on \r and parse the text?
	std::string newLineDelimitedFileContents = "";
	for (int stringIndex = 0; stringIndex < (int)newLineDelimitedList.size(); ++stringIndex)
	{
		std::string const& currentString = newLineDelimitedList[stringIndex];
		newLineDelimitedFileContents += currentString;
	}
	out_delimitedList = SplitStringOnDelimiter(newLineDelimitedFileContents, '\n');
}
