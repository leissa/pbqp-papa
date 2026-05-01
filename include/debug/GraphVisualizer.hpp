#ifndef DEBUG_GRAPHVISUALIZER_HPP_
#define DEBUG_GRAPHVISUALIZER_HPP_

#include <map>
#include <memory>
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

inline std::unique_ptr<char[]> convertStringToC(const std::string& string) {
	auto writable = std::make_unique<char[]>(string.size() + 1);
	std::copy(string.begin(), string.end(), writable.get());
	writable[string.size()] = '\0'; // don't forget the terminating 0
	return writable;
}

template <typename T>
class GraphVisualizer {

public:
	static void dump(PBQPGraph<T>* graph, std::string path, bool showVectors = false) {
		initializeGVC_PBQP();
		auto graphName = convertStringToC("g");
		Agraph_t* graphVis = agopen(graphName.get(), Agdirected, 0);
		std::map<PBQPNode<T>*, Agnode_t*> nodeMapping;
		PBQP_Serializer<T> serial;
		auto nodeUrlAttr = convertStringToC("URL");
		auto nodeUrlValue = convertStringToC("URLVALUE");
		auto nodeTooltipAttr = convertStringToC("tooltip");
		auto nodeTooltipValue = convertStringToC("tooltipValue");
		agattr(graphVis, AGNODE, nodeUrlAttr.get(), nodeUrlValue.get());
		agattr(graphVis, AGNODE, nodeTooltipAttr.get(), nodeTooltipValue.get());
		for (auto iter = graph->getNodeBegin(); iter != graph->getNodeEnd(); ++iter) {
			PBQPNode<T>* node = *iter;
			auto name = convertStringToC(
					"N " + std::to_string(node->getIndex()) + "\n" + serial.matrixToString(node->getVector()));
			Agnode_t* nodeVis;
			if (showVectors) {
				nodeVis = agnode(graphVis, name.get(), 1);
			} else {
				auto shortName = convertStringToC(std::to_string(node->getIndex()));
				nodeVis = agnode(graphVis, shortName.get(), 1);
			}
			auto urlAttr = convertStringToC("URL");
			auto urlValue = convertStringToC("URLVALUE");
			auto tooltipAttr = convertStringToC("tooltip");
			agset(nodeVis, urlAttr.get(), urlValue.get());
			agset(nodeVis, tooltipAttr.get(), name.get());
			nodeMapping.insert({node, nodeVis});
		}
		if (showVectors) {
			auto edgeLabelAttr = convertStringToC("label");
			auto edgeLabelDefault = convertStringToC("a");
			agattr(graphVis, AGEDGE, edgeLabelAttr.get(), edgeLabelDefault.get());
		}
		auto edgeUrlAttr = convertStringToC("URL");
		auto edgeUrlValue = convertStringToC("URLVALUE");
		auto edgeTooltipAttr = convertStringToC("edgetooltip");
		auto edgeTooltipValue = convertStringToC("");
		auto penWidthAttr = convertStringToC("penwidth");
		auto penWidthValue = convertStringToC("1.0");
		agattr(graphVis, AGEDGE, edgeUrlAttr.get(), edgeUrlValue.get());
		agattr(graphVis, AGEDGE, edgeTooltipAttr.get(), edgeTooltipValue.get());
		agattr(graphVis, AGEDGE, penWidthAttr.get(), penWidthValue.get());
		for (auto iter = graph->getEdgeBegin(); iter != graph->getEdgeEnd(); ++iter) {
			PBQPEdge<T>* edge = *iter;
			Agnode_t* sourceVis = nodeMapping.find(edge->getSource())->second;
			Agnode_t* targetVis = nodeMapping.find(edge->getTarget())->second;
			auto name = convertStringToC(serial.matrixToString(edge->getMatrix()));
			Agedge_t* edgeVis = agedge(graphVis, sourceVis, targetVis, name.get(), 1);
			if (showVectors) {
				auto labelAttr = convertStringToC("label");
				agset(edgeVis, labelAttr.get(), name.get());
			}
			auto urlAttr = convertStringToC("URL");
			auto urlValue = convertStringToC("URLVALUE");
			auto tooltipAttr = convertStringToC("edgetooltip");
			auto localPenWidthAttr = convertStringToC("penwidth");
			auto localPenWidthValue = convertStringToC("1.0");
			agset(edgeVis, urlAttr.get(), urlValue.get());
			agset(edgeVis, tooltipAttr.get(), name.get());
			agattr(graphVis, AGEDGE, localPenWidthAttr.get(), localPenWidthValue.get());
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
