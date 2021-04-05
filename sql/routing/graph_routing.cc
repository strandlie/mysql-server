//
// Created by Håkon Strandlie on 13/11/2020.
//

#include "graph_routing.h"
#include <vector>

class Vertex;

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


String Graph_router::produceString(std::vector<std::pair<Vertex, double>> pred_list) {

}