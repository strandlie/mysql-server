//
// Created by Håkon Strandlie on 13/11/2020.
//

#include "graph_routing.h"

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

