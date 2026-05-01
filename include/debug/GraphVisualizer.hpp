#ifndef DEBUG_GRAPHVISUALIZER_HPP_
#define DEBUG_GRAPHVISUALIZER_HPP_

#include <map>
#include <stdexcept>

#include "io/PBQP_Serializer.hpp"

#if PBQP_USE_GVC
#include <gvc.h>
#include <gvcext.h>

namespace pbqppapa {

template <typename T>
class PBQPGraph;
template <typename T>
class PBQPNode;
template <typename T>
class PBQPEdge;

#if defined(GVDLL)
#define PBQP_GRAPHVIZ_IMPORT __declspec(dllimport)
#else
#define PBQP_GRAPHVIZ_IMPORT
#endif

extern "C" {
PBQP_GRAPHVIZ_IMPORT extern gvplugin_library_t gvplugin_dot_layout_LTX_library;
PBQP_GRAPHVIZ_IMPORT extern gvplugin_library_t gvplugin_core_LTX_library;
}

#undef PBQP_GRAPHVIZ_IMPORT

inline bool wasGvcInited = false;
inline GVC_t* gvcGlobalContext = nullptr;
inline lt_symlist_t gvcBuiltins[] = {
		{"gvplugin_dot_layout_LTX_library", &gvplugin_dot_layout_LTX_library},
		{"gvplugin_core_LTX_library", &gvplugin_core_LTX_library},
		{0, 0},
};

inline void initializeGVC_PBQP() {
	if (!wasGvcInited) {
		wasGvcInited = true;
		/* set up a graphviz context - but only once even for multiple graphs */
		if (!gvcGlobalContext) {
			gvcGlobalContext = gvContextPlugins(gvcBuiltins, false);
		}
	}
}

inline char* convertStringToC(const std::string& string) {
	char* writable = new char[string.size() + 1];
	std::copy(string.begin(), string.end(), writable);
	writable[string.size()] = '\0'; // don't forget the terminating 0
	return writable;
}

template <typename T>
class GraphVisualizer {

public:
	static void dump(PBQPGraph<T>* graph, std::string path, bool showVectors = false) {
		initializeGVC_PBQP();
		Agraph_t* graphVis = agopen(convertStringToC("g"), Agdirected, 0);
		std::map<PBQPNode<T>*, Agnode_t*> nodeMapping;
		PBQP_Serializer<T> serial;
		agattr(graphVis, AGNODE, convertStringToC("URL"), convertStringToC("URLVALUE"));
		agattr(graphVis, AGNODE, convertStringToC("tooltip"), convertStringToC("tooltipValue"));
		for (auto iter = graph->getNodeBegin(); iter != graph->getNodeEnd(); ++iter) {
			PBQPNode<T>* node = *iter;
			char* name = convertStringToC(
					"N " + std::to_string(node->getIndex()) + "\n" + serial.matrixToString(node->getVector()));
			Agnode_t* nodeVis;
			if (showVectors) {
				nodeVis = agnode(graphVis, name, 1);
			} else {
				nodeVis = agnode(graphVis, convertStringToC(std::to_string(node->getIndex())), 1);
			}
			agset(nodeVis, convertStringToC("URL"), convertStringToC("URLVALUE"));
			agset(nodeVis, convertStringToC("tooltip"), name);
			nodeMapping.insert({node, nodeVis});
		}
		if (showVectors) {
			agattr(graphVis, AGEDGE, convertStringToC("label"), convertStringToC("a"));
		}
		agattr(graphVis, AGEDGE, convertStringToC("URL"), convertStringToC("URLVALUE"));
		agattr(graphVis, AGEDGE, convertStringToC("edgetooltip"), convertStringToC(""));
		agattr(graphVis, AGEDGE, convertStringToC("penwidth"), convertStringToC("1.0"));
		for (auto iter = graph->getEdgeBegin(); iter != graph->getEdgeEnd(); ++iter) {
			PBQPEdge<T>* edge = *iter;
			Agnode_t* sourceVis = nodeMapping.find(edge->getSource())->second;
			Agnode_t* targetVis = nodeMapping.find(edge->getTarget())->second;
			char* name = convertStringToC(serial.matrixToString(edge->getMatrix()));
			Agedge_t* edgeVis = agedge(graphVis, sourceVis, targetVis, name, 1);
			if (showVectors) {
				agset(edgeVis, convertStringToC("label"), name);
			}
			agset(edgeVis, convertStringToC("URL"), convertStringToC("URLVALUE"));
			agset(edgeVis, convertStringToC("edgetooltip"), name);
			agattr(graphVis, AGEDGE, convertStringToC("penwidth"), convertStringToC("1.0"));
		}
		if (gvLayout(gvcGlobalContext, graphVis, "dot") != 0) {
			agclose(graphVis);
			throw std::runtime_error("Graphviz failed to compute a dot layout");
		}
		if (gvRenderFilename(gvcGlobalContext, graphVis, "svg", path.c_str()) != 0) {
			gvFreeLayout(gvcGlobalContext, graphVis);
			agclose(graphVis);
			throw std::runtime_error("Graphviz failed to render SVG output");
		}
		gvFreeLayout(gvcGlobalContext, graphVis);
		agclose(graphVis);
	}
};
} // namespace pbqppapa

#endif

#endif /* DEBUG_GRAPHVISUALIZER_HPP_ */
