//
// Created by Håkon Strandlie on 13/11/2020.
//

#include "graph_routing.h"

class Vertex;
void Graph_router::printVertices() {

  // Vector for storing distance property
  std::vector<int> d(num_vertices(G));

  // Get the first vertex
  Vertex s = *(vertices(G).first);
  DBUG_LOG("Routing", "Source: " << name[s]);

  // Invoke variant 2 of Dijkstra's algorithm
  dijkstra_shortest_paths(G, s, b::distance_map(&d[0]));

  // Get the property map for vertex indices
  typedef b::property_map<Graph, b::vertex_index_t>::type IndexMap;
  IndexMap index = get(b::vertex_index, G);

  DBUG_LOG("Routing", "Distances from start vertex: " << name[index(s)]);
  b::graph_traits<Graph>::vertex_iterator vi;
  for(vi = vertices(G).first; vi != vertices(G).second; ++vi) {
    DBUG_LOG("Routing", "distance(" << name[index(*vi)] << ") = " << d[*vi]);
  }

  using std::vector;

  vector<Vertex> p(num_vertices(G), b::graph_traits<Graph>::null_vertex()); // The predecessor array
  dijkstra_shortest_paths(G, s, b::distance_map(&d[0]).predecessor_map(&p[0]));

  DBUG_LOG("Routing", "Parents in the tree of shortest paths: ");
  for(vi = vertices(G).first; vi != vertices(G).second; ++vi) {
    DBUG_LOG("Routing", "parent(" << name[*vi]);
    if(p[*vi] == b::graph_traits<Graph>::null_vertex()) {
      DBUG_LOG("Routing", ") = no parent");
    } else {
      DBUG_LOG("Routing", ") = " << name[p[*vi]]);
    }
  }
}

Graph_router::Vertex *Graph_router::getSource(int id) {
  using namespace boost;
  graph_traits<Graph>::vertex_iterator vi;
  vi = vertices(G).first;
  Graph_router::Vertex s;
  for( ; vi != vertices(G).second; ++vi) {
    s = *vi;
    if (index(s) == (std::string::size_type) id) {
      return &s;
    }
  }
  return nullptr;
}

void Graph_router::executeDijkstra(Vertex source) {
  dijkstra_shortest_paths(G, source, b::distance_map(&distances[0]).predecessor_map(&predecessors[0]));
  currentSource = source; // Only add the new source after successful dijkstra
}

/**
 * Create a pretty-formatted string for predecessors
 * @param str The address to write the result string to
 */
void Graph_router::getPredecessors(String *str) {
  if (predecessors.size() == 0) {
    memcpy(str, nullptr, 1);
    return;
  }

  using namespace boost;

  // TODO: Write to the string, not just debug-print
  graph_traits<Graph>::vertex_iterator vi;
  DBUG_LOG("Routing", "Parents in the tree of shortest paths: ");
  for(vi = vertices(G).first; vi != vertices(G).second; ++vi) {
    DBUG_LOG("Routing", "parent(" << *vi);
    if(predecessors[*vi] == b::graph_traits<Graph>::null_vertex()) {
      DBUG_LOG("Routing", ") = no parent");
    } else {
      DBUG_LOG("Routing", ") = " << predecessors[*vi]);
    }
  }
}

/**
 * Create a pretty-formatted string for distances
 * @param str The address to write the result string to
 */
void Graph_router::getDistances(String *str) {
  if (distances.size() == 0) {
    memcpy(str, nullptr, 1);
    return;
  }

  // TODO: Write to the string, not just debug-print
  DBUG_LOG("Routing", "Distances from start vertex: " << index(currentSource));
  b::graph_traits<Graph>::vertex_iterator vi;
  for(vi = vertices(G).first; vi != vertices(G).second; ++vi) {
    DBUG_LOG("Routing", "distance(" << index(*vi) << ") = " << distances[*vi]);
  }
}