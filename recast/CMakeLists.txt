CMAKE_MINIMUM_REQUIRED(VERSION 2.8)

SET(recast_navigation_sources
	debug_utils/src/DebugDraw.cpp
	debug_utils/src/DetourDebugDraw.cpp
	debug_utils/src/RecastDebugDraw.cpp
	debug_utils/src/RecastDump.cpp
	detour/src/DetourAlloc.cpp
	detour/src/DetourCommon.cpp
	detour/src/DetourNavMesh.cpp
	detour/src/DetourNavMeshBuilder.cpp
	detour/src/DetourNavMeshQuery.cpp
	detour/src/DetourNode.cpp
	detour_tile_cache/src/DetourTileCache.cpp
	detour_tile_cache/src/DetourTileCacheBuilder.cpp
	recast/src/Recast.cpp
	recast/src/RecastAlloc.cpp
	recast/src/RecastArea.cpp
	recast/src/RecastContour.cpp
	recast/src/RecastFilter.cpp
	recast/src/RecastLayers.cpp
	recast/src/RecastMesh.cpp
	recast/src/RecastMeshDetail.cpp
	recast/src/RecastRasterization.cpp
	recast/src/RecastRegion.cpp
)

SET(recast_navigation_headers
	debug_utils/include/DebugDraw.h
	debug_utils/include/DetourDebugDraw.h
	debug_utils/include/RecastDebugDraw.h
	debug_utils/include/RecastDump.h
	detour/include/DetourAlloc.h
	detour/include/DetourAssert.h
	detour/include/DetourCommon.h
	detour/include/DetourMath.h
	detour/include/DetourNavMesh.h
	detour/include/DetourNavMeshBuilder.h
	detour/include/DetourNavMeshQuery.h
	detour/include/DetourNode.h
	detour/include/DetourStatus.h
	detour_tile_cache/include/DetourTileCache.h
	detour_tile_cache/include/DetourTileCacheBuilder.h
	recast/include/Recast.h
	recast/include/RecastAlloc.h
	recast/include/RecastAssert.h
)

SOURCE_GROUP(DebugUtils FILES
	debug_utils/src/DebugDraw.cpp
	debug_utils/src/DetourDebugDraw.cpp
	debug_utils/src/RecastDebugDraw.cpp
	debug_utils/src/RecastDump.cpp
	debug_utils/include/DebugDraw.h
	debug_utils/include/DetourDebugDraw.h
	debug_utils/include/RecastDebugDraw.h
	debug_utils/include/RecastDump.h
)

SOURCE_GROUP(Detour FILES
	detour/src/DetourAlloc.cpp
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

SOURCE_GROUP(DetourTileCache FILES
	detour_tile_cache/src/DetourTileCache.cpp
	detour_tile_cache/src/DetourTileCacheBuilder.cpp
	detour_tile_cache/include/DetourTileCache.h
	detour_tile_cache/include/DetourTileCacheBuilder.h
)

SOURCE_GROUP(Recast FILES
	recast/src/Recast.cpp
	recast/src/RecastAlloc.cpp
	recast/src/RecastArea.cpp
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