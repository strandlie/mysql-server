//
// Created by Håkon Strandlie on 13/11/2020.
//

#include "graph_routing.h"
#include <vector>
#include <boost/graph/dijkstra_shortest_paths.hpp>

class Vertex;

Graph_router::Graph_router(std::vector<Edge> edges, std::vector<double> weights): G() {
  if (edges.size() != weights.size()) {
    my_error(ER_WRONG_PARAMETERS_TO_PROCEDURE, MYF(0), "Graph_router");
    return;
  }
  for (std::string::size_type i = 0; i < edges.size(); ++i) {
    add_edge(edges[i].first, edges[i].second, EdgeWeightProperty(weights[i]),
             G);
  }
  edges.clear();
  weights.clear();

  /*predecessors = std::vector<Vertex, Routing_allocator<Vertex>>(
      num_vertices(G), b::graph_traits<Graph>::null_vertex());
  distances = std::vector<double, Routing_allocator<double>>(num_vertices(G));
  */
  predecessors = std::vector<Vertex>(num_vertices(G), null_vertex());
  distances = std::vector<double>(num_vertices(G));
}

Graph_router::Graph_router() : G() {
  predecessors = std::vector<Vertex>(num_vertices(G), null_vertex());
  distances = std::vector<double>(num_vertices(G));
}

Graph_router::Vertex Graph_router::getSource(unsigned long id) {
  using namespace boost;
  typedef graph_traits<Graph>::vertex_iterator v_iter;

  for(v_iter vi = vertices(G).first ; vi != vertices(G).second; ++vi) {
    if (*vi == (std::string::size_type) id) {
      return *vi;
    }
  }
  DBUG_LOG("Routing", "Did not find the node");
  return Graph_router::null_vertex();
}


/**
 * Supply the source and execute Dijkstra's single source - all destinations
 * @param source The source to start from
 */
void Graph_router::executeDijkstra(Vertex source) {
  dijkstra_shortest_paths(G, source, b::distance_map(&distances[0]).predecessor_map(&predecessors[0]));
  currentSource = source; // Only add the new source after successful dijkstra
}


/**
 * Get the predecessors for a specific vertex
 * @param id The id of the vertex we want to find the predecessors to
 * @return A vector of the predecessors, with the source node at the
 * end, and the target node at the beginning
 */
std::vector<Graph_router::Vertex> Graph_router::getPredecessorsTo(unsigned long id) {
  std::vector<Graph_router::Vertex> vec;
  if (predecessors.empty()) {
    return vec;
  }

  using namespace boost;

  typedef graph_traits<Graph>::vertex_iterator vertex_iter;
  std::pair<vertex_iter, vertex_iter> vp;
  for (vp = vertices(G); vp.first != vp.second; ++vp.first) {

    Vertex v = *vp.first;
    if (v == id) {
      Vertex parent = v;
      Vertex current;
      do {
        current = parent;
        parent = predecessors[current];
        if(parent == graph_traits<Graph>::null_vertex()) {
          DBUG_LOG("Routing", "parent(" << current << ") = no parent");
        } else {
          DBUG_LOG("Routing", "parent(" << current << ") = " << parent);
          vec.push_back(parent);
        }
      } while(parent != current && parent != graph_traits<Graph>::null_vertex());
    }
  }
  return vec;

}

std::vector<std::pair<Graph_router::Vertex, double>> Graph_router::getDistances() {
  std::vector<std::pair<Graph_router::Vertex, double>> vec;
  if(distances.empty()) {
    return vec;
  }

  DBUG_LOG("Routing", "Distances from start vertex: " << currentSource);
  b::graph_traits<Graph>::vertex_iterator vi;
  for(vi = vertices(G).first; vi != vertices(G).second; ++vi) {
    DBUG_LOG("Routing", "distance(" << index(*vi) << ") = " << distances[*vi]);
    vec.emplace_back(index(*vi), distances[*vi]);
  }
  return vec;
}

/**
 * Only find the distance to a specific vertex
 * @param id The id of the vertex we want the distance to
 */
std::pair<Graph_router::Vertex, double> Graph_router::getDistancesTo(unsigned long id) {
  using namespace boost;
  if (distances.empty()) {
    return std::pair<Graph_router::Vertex, double>{ Graph_router::null_vertex(), 0 };
  }

  typedef graph_traits<Graph>::vertex_iterator vertex_iter;
  std::pair<vertex_iter, vertex_iter> vp = vertices(G);
  for ( ; vp.first != vp.second; ++vp.first) {
    if (*vp.first == id) {
      DBUG_LOG("Routing", "distance(" << *vp.first << ") = " << distances[*vp.first]);
      return std::pair<Graph_router::Vertex, double>(*vp.first, distances[*vp.first]);
    }
  }
  return std::pair<Graph_router::Vertex, double>{ Graph_router::null_vertex(), 0 };
}


String *Graph_router::produceDistanceString(std::vector<std::pair<Vertex, double>> dist_map) {

}

String *Graph_router::producePredString(std::vector<Vertex> preds, long tgt_node_id) {

  String *value = new (current_thd->mem_root)
      String("", 5, &my_charset_utf8mb4_general_ci);
  std::vector<Graph_router::Vertex>::reverse_iterator rit = preds.rbegin();
  value->append("Source: \n");
  for(; rit != preds.rend(); ++rit) {
    value->append("\t|--> ");
    value->append_longlong(*rit);
    value->append("\n");
  }
  value->append("Target: ");
  value->append_longlong(tgt_node_id);
  value->append("\n");
  return value;
}