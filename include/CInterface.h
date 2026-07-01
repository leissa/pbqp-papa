#pragma once

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
#include <map>
#include <vector>

#include "graph/PBQPGraph.hpp"
#include "math/InfinityWrapper.hpp"
#endif

#ifdef __cplusplus
extern "C" {
#endif

#define CINTERFACESIG(TYPENAME, SHORTNAME)                                                                             \
	struct pbqp_##SHORTNAME##_parsing;                                                                                 \
	struct pbqp_##SHORTNAME##_solution {                                                                               \
		uint16_t* selections;                                                                                          \
		size_t length;                                                                                                 \
	};                                                                                                                 \
	void pbqp_##SHORTNAME##_addNode(                                                                                   \
			struct pbqp_##SHORTNAME##_parsing*, TYPENAME* data, uint16_t length, size_t index);                        \
	void pbqp_##SHORTNAME##_addEdge(                                                                                   \
			struct pbqp_##SHORTNAME##_parsing*, size_t sourceIndex, size_t targetIndex, TYPENAME* data);               \
	struct pbqp_##SHORTNAME##_solution* pbqp_##SHORTNAME##_solve(struct pbqp_##SHORTNAME##_parsing*);                  \
	struct pbqp_##SHORTNAME##_parsing* pbqp_##SHORTNAME##_createInstance(size_t nodeAmount);                           \
	void pbqp_##SHORTNAME##_addToPEO(struct pbqp_##SHORTNAME##_parsing*, size_t index);                                \
	void pbqp_##SHORTNAME##_finishPEO(struct pbqp_##SHORTNAME##_parsing*);                                             \
	void pbqp_##SHORTNAME##_dump(struct pbqp_##SHORTNAME##_parsing*, char* path);                                      \
	void pbqp_##SHORTNAME##_free(struct pbqp_##SHORTNAME##_parsing*);                                                  \
	TYPENAME pbqp_##SHORTNAME##_getNodeVektorValue(                                                                    \
			struct pbqp_##SHORTNAME##_parsing* pbqpparsing, size_t nodeIndex, uint16_t entryIndex);                    \
	size_t pbqp_##SHORTNAME##_getAdjacentEdgeCount(struct pbqp_##SHORTNAME##_parsing* pbqpparsing, size_t nodeIndex);  \
	uint16_t pbqp_##SHORTNAME##_getVectorLength(struct pbqp_##SHORTNAME##_parsing* pbqpparsing, size_t nodeIndex);     \
	TYPENAME pbqp_##SHORTNAME##_getMatrixEntry(struct pbqp_##SHORTNAME##_parsing* pbqpparsing, size_t nodeIndex,       \
			size_t adjacencyIndex, uint16_t row, uint16_t column);                                                     \
	size_t pbqp_##SHORTNAME##_getAdjacentNodeIndex(                                                                    \
			struct pbqp_##SHORTNAME##_parsing* pbqpparsing, size_t nodeIndex, size_t adjacencyIndex);
// struct pbqp_ ## SHORTNAME ## _solution* pbqp_ ## SHORTNAME ## _solveGurobi(struct pbqp_ ## SHORTNAME ## _parsing*);

#if PBQP_USE_GUROBI
#define CGUROBIINTERFACE(TYPENAME, SHORTNAME)                                                                          \
	struct pbqp_##SHORTNAME##_solution* pbqp_##SHORTNAME##_solveGurobi(struct pbqp_##SHORTNAME##_parsing*);
#endif

CINTERFACESIG(uint32_t, uint)

CINTERFACESIG(uint16_t, ushort)

CINTERFACESIG(uint64_t, ulong)

#if PBQP_USE_GUROBI
CGUROBIINTERFACE(uint32_t, uint)
CGUROBIINTERFACE(uint16_t, ushort)
CGUROBIINTERFACE(uint64_t, ulong)
#endif

#ifdef __cplusplus
} // end extern "C"
#endif
