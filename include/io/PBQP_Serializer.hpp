#ifndef IO_PBQP_SERIALIZER_HPP_
#define IO_PBQP_SERIALIZER_HPP_

#include <cstdio>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <iterator>
#include <map>
#include <memory>
#include <string>
#include <vector>

#include <nlohmann/json.hpp>

#include "graph/PBQPGraph.hpp"
#include "io/TypeSerializers.hpp"

namespace pbqppapa {

template <typename T>
class TypeSerializer;
template <typename T>
class PBQPGraph;
template <typename T>
class InfinityWrapper;

/**
 * Saves PBQP instances to JSON files and loads them from them
 */
template <typename T>
class PBQP_Serializer {

public:
	/**
	 * Saves a PBQP instance to the given path. No checks are done on how valid the path is
	 * Optionally additional debug information can written out, which is useful for humans sometimes
	 * This debug information is ignored when reading PBQP instances from files
	 */
	static void saveToFile(const std::string& path, PBQPGraph<T>* graph, bool debug = false) {
		std::ofstream out(path);
		nlohmann::json json = graphToJson(graph, debug);
		out << json << std::endl;
		out.close();
	}

	/**
	 * Loads a PBQP instances from a json file. No checks are done whether the given path is valid or if
	 * it contains a proper JSON PBQP. If not, this will explode
	 */
	static PBQPGraph<T>* loadFromFile(const std::string& path) {
		std::ifstream in(path);
		nlohmann::json json;
		in >> json;
		in.close();
		return jsonToGraph(json);
	}

	/**
	 * Gets a nice printable version of a matrix. Useful for debugging
	 */
	static std::string matrixToString(const Matrix<T>& matrix, const std::string& separator = " ") {
		std::string output;
		for (unsigned short row = 0; row < matrix.getRowCount(); row++) {
			for (unsigned short column = 0; column < matrix.getColumnCount(); column++) {
				output += serializeElement<T>(matrix.get(row, column));
				output += separator;
			}
			output += '\n';
		}
		return output;
	}

private:
	static PBQPGraph<T>* jsonToGraph(const nlohmann::json& json) {
		nlohmann::json metaJson = json["meta"];
		std::string parsedType = metaJson["type"];
		if (parsedType != getTypeName<T>()) {
			std::cout << "Invalid type loading, expected type: " << getTypeName<T>() << ", but got type: " << parsedType
					  << "\nAttempting to load anyway...\n";
		}
		auto graph = std::make_unique<PBQPGraph<T>>();
		std::map<unsigned int, PBQPNode<T>*> nodeByIndex;
		nlohmann::json nodeJson = json["nodes"];
		for (const nlohmann::json& singleNodeJson : nodeJson) {
			unsigned int index = singleNodeJson["index"];
			Vector<T> vek = parseVector(singleNodeJson["cost"]);
			auto node = std::make_unique<PBQPNode<T>>(index, vek);
			graph->addNode(node.get());
			nodeByIndex.insert({index, node.get()});
			node.release();
		}
		nlohmann::json edgeJson = json["edges"];
		for (const nlohmann::json& singleEdgeJson : edgeJson) {
			Matrix<T> mat = parseMatrix(singleEdgeJson);
			PBQPNode<T>* source = nodeByIndex.find(singleEdgeJson["source"])->second;
			PBQPNode<T>* target = nodeByIndex.find(singleEdgeJson["target"])->second;
			graph->addEdge(source, target, mat);
		}
		if (json.find("peo") != json.end()) {
			const nlohmann::json& peoJson = json["peo"];
			std::vector<PBQPNode<T>*> peoVector;
			peoVector.reserve(peoJson.size());
			for (unsigned int i = 0; i < peoJson.size(); i++) {
				peoVector.push_back(nodeByIndex.find(peoJson[i])->second);
			}
			graph->setPEO(peoVector);
		}
		return graph.release();
	}

	static Vector<T> parseVector(const nlohmann::json& json) {
		Vector<T> vek = Vector<T>(static_cast<unsigned short>(json.size()));
		for (unsigned short i = 0; i < json.size(); i++) {
			vek.get(i) = deserializeElement<T>(json[i]);
		}
		return vek;
	}

	static Matrix<T> parseMatrix(const nlohmann::json& json) {
		unsigned short rows = json["rows"];
		unsigned short columns = json["columns"];
		nlohmann::json valueJson = json["cost"];
		Matrix<T> mat = Matrix<T>(rows, columns);
		for (unsigned int i = 0; i < valueJson.size(); i++) {
			mat.getRaw(i) = deserializeElement<T>(valueJson[i]);
		}
		return mat;
	}

	static nlohmann::json graphToJson(PBQPGraph<T>* graph, bool debug) {
		nlohmann::json json;
		json["meta"] = serializeMeta(graph);
		nlohmann::json nodeJsons = nlohmann::json::array();
		for (auto iter = graph->getNodeBegin(); iter != graph->getNodeEnd(); ++iter) {
			nlohmann::json nodeJson;
			PBQPNode<T>* node = *iter;
			nodeJson["index"] = node->getIndex();
			nlohmann::json costVector = nlohmann::json::array();
			for (unsigned short i = 0; i < node->getVectorDegree(); i++) {
				costVector.push_back(serializeElement<T>(node->getVector().get(i)));
			}
			nodeJson["cost"] = costVector;
			if (debug) {
				nodeJson["degree"] = node->getDegree();
				nlohmann::json neighborVector = nlohmann::json::array();
				for (PBQPNode<T>* neighbor : node->getAdjacentNodes()) {
					neighborVector.push_back(std::to_string(neighbor->getIndex()));
				}
				nodeJson["neighbours"] = neighborVector;
			}
			nodeJsons.push_back(nodeJson);
		}
		json["nodes"] = nodeJsons;
		nlohmann::json edgeJsons = nlohmann::json::array();
		for (auto iter = graph->getEdgeBegin(); iter != graph->getEdgeEnd(); ++iter) {
			PBQPEdge<T>* edge = *iter;
			nlohmann::json edgeJson;
			edgeJson["source"] = edge->getSource()->getIndex();
			edgeJson["target"] = edge->getTarget()->getIndex();
			edgeJson["columns"] = edge->getMatrix().getColumnCount();
			edgeJson["rows"] = edge->getMatrix().getRowCount();
			nlohmann::json matrixValues;
			unsigned int length = edge->getMatrix().getElementCount();
			for (unsigned int i = 0; i < length; i++) {
				matrixValues.push_back(serializeElement<T>(edge->getMatrix().getRaw(i)));
			}
			edgeJson["cost"] = matrixValues;
			edgeJsons.push_back(edgeJson);
		}
		json["edges"] = edgeJsons;
		if (graph->getPEO().size() > 0) {
			json["peo"] = serializePEO(graph->getPEO());
		}
		return json;
	}

	static nlohmann::json serializePEO(const std::vector<PBQPNode<T>*>& peo) {
		nlohmann::json peoVector = nlohmann::json::array();
		for (auto peoIter = peo.begin(); peoIter != peo.end(); ++peoIter) {
			if (!(*peoIter)->isDeleted()) {
				peoVector.push_back((*peoIter)->getIndex());
			}
		}
		return peoVector;
	}

	static nlohmann::json serializeMeta(const PBQPGraph<T>* graph) {
		nlohmann::json json;
		json["version"] = 1;
		json["nodeCount"] = graph->getNodeCount();
		json["edgeCount"] = graph->getEdgeCount();
		json["type"] = getTypeName<T>();
		return json;
	}
};

} // namespace pbqppapa

#endif /* IO_PBQP_SERIALIZER_HPP_ */
