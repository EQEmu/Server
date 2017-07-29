CMAKE_MINIMUM_REQUIRED(VERSION 2.8)

SET(recast_navigation_sources
	detour/src/DetourAlloc.cpp
	detour/src/DetourAssert.cpp
	detour/src/DetourCommon.cpp
	detour/src/DetourNavMesh.cpp
	detour/src/DetourNavMeshBuilder.cpp
	detour/src/DetourNavMeshQuery.cpp
	detour/src/DetourNode.cpp
	recast/src/Recast.cpp
	recast/src/RecastAlloc.cpp
	recast/src/RecastArea.cpp
	recast/src/RecastAssert.cpp
	recast/src/RecastContour.cpp
	recast/src/RecastFilter.cpp
	recast/src/RecastLayers.cpp
	recast/src/RecastMesh.cpp
	recast/src/RecastMeshDetail.cpp
	recast/src/RecastRasterization.cpp
	recast/src/RecastRegion.cpp
)

SET(recast_navigation_headers
	detour/include/DetourAlloc.h
	detour/include/DetourAssert.h
	detour/include/DetourCommon.h
	detour/include/DetourMath.h
	detour/include/DetourNavMesh.h
	detour/include/DetourNavMeshBuilder.h
	detour/include/DetourNavMeshQuery.h
	detour/include/DetourNode.h
	detour/include/DetourStatus.h
	recast/include/Recast.h
	recast/include/RecastAlloc.h
	recast/include/RecastAssert.h
)

SOURCE_GROUP(Detour FILES
	detour/src/DetourAlloc.cpp
	detour/src/DetourAssert.cpp
	detour/src/DetourCommon.cpp
	detour/src/DetourNavMesh.cpp
	detour/src/DetourNavMeshBuilder.cpp
	detour/src/DetourNavMeshQuery.cpp
	detour/src/DetourNode.cpp
	detour/include/DetourAlloc.h
	detour/include/DetourAssert.h
	detour/include/DetourCommon.h
	detour/include/DetourMath.h
	detour/include/DetourNavMesh.h
	detour/include/DetourNavMeshBuilder.h
	detour/include/DetourNavMeshQuery.h
	detour/include/DetourNode.h
	detour/include/DetourStatus.h
)

SOURCE_GROUP(Recast FILES
	recast/src/Recast.cpp
	recast/src/RecastAlloc.cpp
	recast/src/RecastArea.cpp
	recast/src/RecastAssert.cpp
	recast/src/RecastContour.cpp
	recast/src/RecastFilter.cpp
	recast/src/RecastLayers.cpp
	recast/src/RecastMesh.cpp
	recast/src/RecastMeshDetail.cpp
	recast/src/RecastRasterization.cpp
	recast/src/RecastRegion.cpp
	recast/include/Recast.h
	recast/include/RecastAlloc.h
	recast/include/RecastAssert.h
)

ADD_LIBRARY(recast_navigation ${recast_navigation_sources} ${recast_navigation_headers})

SET(LIBRARY_OUTPUT_PATH ${PROJECT_BINARY_DIR}/lib)