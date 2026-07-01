#include "CInterface.h"

#include "graph/PBQPEdge.hpp"
#include "graph/PBQPGraph.hpp"
#include "graph/PBQPNode.hpp"
#include "graph/PBQPSolution.hpp"
#include "graph/Vector.hpp"
#include "io/PBQP_Serializer.hpp"
#include "math/InfinityWrapper.hpp"
#include "solve/StepByStepSolver.hpp"
#if PBQP_USE_GUROBI
#include "math/GurobiConverter.hpp"
#endif

#include <algorithm>
#include <cstdio>
#include <ctime>
#include <iostream>
#include <memory>
#include <vector>

namespace pbqppapa {

#define CINTERFACEIMPL(TYPENAME, SHORTNAME)                                                                            \
	struct pbqp_##SHORTNAME##_parsing {                                                                                \
		std::unique_ptr<PBQPGraph<InfinityWrapper<TYPENAME>>> graph;                                                   \
		std::vector<PBQPNode<InfinityWrapper<TYPENAME>>*> nodes;                                                       \
		std::vector<PBQPNode<InfinityWrapper<TYPENAME>>*> peo;                                                         \
		size_t maximumIndex = 0;                                                                                       \
	};                                                                                                                 \
	extern "C" void pbqp_##SHORTNAME##_addNode(                                                                        \
			struct pbqp_##SHORTNAME##_parsing* pbqpparsing, TYPENAME* data, uint16_t length, size_t index) {           \
		std::vector<InfinityWrapper<TYPENAME>> infData(length);                                                        \
		for (uint16_t i = 0; i < length; i++) {                                                                        \
			infData[i] = InfinityWrapper<TYPENAME>(data[i]);                                                           \
		}                                                                                                              \
		Vector<InfinityWrapper<TYPENAME>> vek(length, infData.data());                                                 \
		auto node = std::make_unique<PBQPNode<InfinityWrapper<TYPENAME>>>(index, vek);                                 \
		pbqpparsing->graph->addNode(node.get());                                                                       \
		pbqpparsing->nodes.at(index) = node.get();                                                                     \
		node.release();                                                                                                \
	}                                                                                                                  \
	extern "C" void pbqp_##SHORTNAME##_addEdge(                                                                        \
			struct pbqp_##SHORTNAME##_parsing* pbqpparsing, size_t sourceIndex, size_t targetIndex, TYPENAME* data) {  \
		PBQPNode<InfinityWrapper<TYPENAME>>* source = pbqpparsing->nodes.at(sourceIndex);                              \
		PBQPNode<InfinityWrapper<TYPENAME>>* target = pbqpparsing->nodes.at(targetIndex);                              \
		const size_t length = source->getVectorDegree() * target->getVectorDegree();                                   \
		std::vector<InfinityWrapper<TYPENAME>> infData(length);                                                        \
		for (size_t i = 0; i < length; i++) {                                                                          \
			infData[i] = InfinityWrapper<TYPENAME>(data[i]);                                                           \
		}                                                                                                              \
		Matrix<InfinityWrapper<TYPENAME>> mat(source->getVectorDegree(), target->getVectorDegree(), infData.data());   \
		pbqpparsing->graph->addEdge(source, target, mat);                                                              \
	}                                                                                                                  \
	struct pbqp_##SHORTNAME##_solution* pbqp_##SHORTNAME##_convertSolution(                                            \
			struct pbqp_##SHORTNAME##_parsing* pbqpparsing, PBQPSolution<InfinityWrapper<TYPENAME>>* cppSol) {         \
		auto cSol = std::make_unique<pbqp_##SHORTNAME##_solution>();                                                   \
		auto selections = std::make_unique<uint16_t[]>(pbqpparsing->maximumIndex);                                     \
		for (size_t i = 0; i < pbqpparsing->maximumIndex; i++) {                                                       \
			if (!pbqpparsing->nodes.at(i)) {                                                                           \
				continue;                                                                                              \
			}                                                                                                          \
			selections[i] = cppSol->getSolution(i);                                                                    \
		}                                                                                                              \
		cSol->selections = selections.release();                                                                       \
		cSol->length = pbqpparsing->maximumIndex;                                                                      \
		return cSol.release();                                                                                         \
	}                                                                                                                  \
	extern "C" struct pbqp_##SHORTNAME##_solution* pbqp_##SHORTNAME##_solve(                                           \
			struct pbqp_##SHORTNAME##_parsing* pbqpparsing) {                                                          \
		if (pbqpparsing->graph->getNodeCount() == 0) {                                                                 \
			auto emptySolution = std::make_unique<pbqp_##SHORTNAME##_solution>();                                      \
			emptySolution->selections = nullptr;                                                                       \
			emptySolution->length = 0;                                                                                 \
			return emptySolution.release();                                                                            \
		}                                                                                                              \
		StepByStepSolver<TYPENAME> solver(pbqpparsing->graph.get());                                                   \
		auto cppLevelSol = std::unique_ptr<PBQPSolution<InfinityWrapper<TYPENAME>>>(solver.solveFully());              \
		pbqp_##SHORTNAME##_solution* cLevelSol = pbqp_##SHORTNAME##_convertSolution(pbqpparsing, cppLevelSol.get());   \
		return cLevelSol;                                                                                              \
	}                                                                                                                  \
	extern "C" struct pbqp_##SHORTNAME##_parsing* pbqp_##SHORTNAME##_createInstance(size_t maximumIndex) {             \
		auto result = std::make_unique<pbqp_##SHORTNAME##_parsing>();                                                  \
		result->graph = std::make_unique<PBQPGraph<InfinityWrapper<TYPENAME>>>();                                      \
		result->nodes.resize(maximumIndex);                                                                            \
		result->maximumIndex = maximumIndex;                                                                           \
		return result.release();                                                                                       \
	}                                                                                                                  \
	extern "C" void pbqp_##SHORTNAME##_addToPEO(struct pbqp_##SHORTNAME##_parsing* pbqpparsing, size_t index) {        \
		pbqpparsing->peo.push_back(pbqpparsing->nodes.at(index));                                                      \
	}                                                                                                                  \
	extern "C" void pbqp_##SHORTNAME##_dump(struct pbqp_##SHORTNAME##_parsing* pbqpparsing, char* path) {              \
		if (pbqpparsing->graph->getNodeCount() == 0) {                                                                 \
			std::cout << "Graph requested to dump to flat file was empty, did not dump";                               \
			return;                                                                                                    \
		}                                                                                                              \
		if (pbqpparsing->graph->getEdgeCount() == 0) {                                                                 \
			std::cout << "NO EDGES";                                                                                   \
			return;                                                                                                    \
		}                                                                                                              \
		std::string stringPath;                                                                                        \
		if (!path) {                                                                                                   \
			stringPath = std::to_string(std::chrono::system_clock::now().time_since_epoch().count()) + ".json";        \
		} else {                                                                                                       \
			stringPath = std::string(path);                                                                            \
		}                                                                                                              \
		std::cout << "Dumping to " << stringPath << "\n";                                                              \
		pbqpparsing->graph->setPEO(pbqpparsing->peo);                                                                  \
		PBQP_Serializer<InfinityWrapper<TYPENAME>>::saveToFile(stringPath, pbqpparsing->graph.get());                  \
	}                                                                                                                  \
	extern "C" void pbqp_##SHORTNAME##_free(struct pbqp_##SHORTNAME##_parsing* pbqpparsing) {                          \
		delete pbqpparsing;                                                                                            \
	}                                                                                                                  \
	extern "C" void pbqp_##SHORTNAME##_finishPEO(struct pbqp_##SHORTNAME##_parsing* pbqpparsing) {                     \
		std::reverse(pbqpparsing->peo.begin(), pbqpparsing->peo.end());                                                \
	}                                                                                                                  \
	extern "C" TYPENAME pbqp_##SHORTNAME##_getNodeVektorValue(                                                         \
			struct pbqp_##SHORTNAME##_parsing* pbqpparsing, size_t nodeIndex, uint16_t entryIndex) {                   \
		PBQPNode<InfinityWrapper<TYPENAME>>* node = pbqpparsing->nodes.at(nodeIndex);                                  \
		return node->getVector().get(entryIndex).getValue();                                                           \
	}                                                                                                                  \
	extern "C" size_t pbqp_##SHORTNAME##_getAdjacentEdgeCount(                                                         \
			struct pbqp_##SHORTNAME##_parsing* pbqpparsing, size_t nodeIndex) {                                        \
		PBQPNode<InfinityWrapper<TYPENAME>>* node = pbqpparsing->nodes.at(nodeIndex);                                  \
		return node->getDegree();                                                                                      \
	}                                                                                                                  \
	extern "C" uint16_t pbqp_##SHORTNAME##_getVectorLength(                                                            \
			struct pbqp_##SHORTNAME##_parsing* pbqpparsing, size_t nodeIndex) {                                        \
		PBQPNode<InfinityWrapper<TYPENAME>>* node = pbqpparsing->nodes.at(nodeIndex);                                  \
		return node->getVectorDegree();                                                                                \
	}                                                                                                                  \
	extern "C" TYPENAME pbqp_##SHORTNAME##_getMatrixEntry(struct pbqp_##SHORTNAME##_parsing* pbqpparsing,              \
			size_t nodeIndex, size_t adjacencyIndex, uint16_t row, uint16_t column) {                                  \
		PBQPNode<InfinityWrapper<TYPENAME>>* node = pbqpparsing->nodes.at(nodeIndex);                                  \
		return node->getAdjacentEdges().at(adjacencyIndex)->getMatrix().get(row, column).getValue();                   \
	}                                                                                                                  \
	extern "C" size_t pbqp_##SHORTNAME##_getAdjacentNodeIndex(                                                         \
			struct pbqp_##SHORTNAME##_parsing* pbqpparsing, size_t nodeIndex, size_t adjacencyIndex) {                 \
		PBQPNode<InfinityWrapper<TYPENAME>>* node = pbqpparsing->nodes.at(nodeIndex);                                  \
		return node->getAdjacentNodes().at(adjacencyIndex)->getIndex();                                                \
	}

#if PBQP_USE_GUROBI
#define CGUROBI(TYPENAME, SHORTNAME)                                                                                   \
	extern "C" struct pbqp_##SHORTNAME##_solution* pbqp_##SHORTNAME##_solveGurobi(                                     \
			struct pbqp_##SHORTNAME##_parsing* pbqpparsing) {                                                          \
		if (pbqpparsing->graph->getNodeCount() == 0) {                                                                 \
			auto emptySolution = std::make_unique<pbqp_##SHORTNAME##_solution>();                                      \
			emptySolution->selections = nullptr;                                                                       \
			emptySolution->length = 0;                                                                                 \
			return emptySolution.release();                                                                            \
		}                                                                                                              \
		GurobiConverter<TYPENAME> gurobi(pbqpparsing->graph.get());                                                    \
		auto cppLevelSol = std::unique_ptr<PBQPSolution<InfinityWrapper<TYPENAME>>>(gurobi.solveGurobiQuadratic());    \
		pbqp_##SHORTNAME##_solution* cLevelSol = pbqp_##SHORTNAME##_convertSolution(pbqpparsing, cppLevelSol.get());   \
		return cLevelSol;                                                                                              \
	}
#endif

} // namespace pbqppapa

using namespace pbqppapa;

CINTERFACEIMPL(uint32_t, uint)

CINTERFACEIMPL(uint16_t, ushort)

CINTERFACEIMPL(uint64_t, ulong)

#if PBQP_USE_GUROBI
CGUROBI(uint32_t, uint)
CGUROBI(uint16_t, ushort)
CGUROBI(uint64_t, ulong)
#endif
