//
// Created by Håkon Strandlie on 13/11/2020.
//

#ifndef MYSQL_GRAPH_ROUTING_H
#define MYSQL_GRAPH_ROUTING_H

#endif  // MYSQL_GRAPH_ROUTING_H

#include <boost/graph/graph_traits.hpp>
#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/dijkstra_shortest_paths.hpp>
#include <boost/property_map/property_map.hpp>
#include "my_dbug.h"

namespace b = boost;
class Graph_router {
 private:
  typedef b::adjacency_list<b::listS, b::vecS, b::directedS, b::no_property, b::property<b::edge_weight_t, int> > Graph;
  typedef b::graph_traits<Graph>::vertex_descriptor Vertex;
  typedef b::property_map<Graph, b::vertex_index_t>::type IndexMap;
  typedef std::pair<int, int> Edge;
  enum { A, B, C, D, E, N };

  const char* name = "ABCDE";
  const Edge edges[9] = { Edge(A, C),
                          Edge(B, B), Edge(B, D), Edge(B, E),
                          Edge(C, B), Edge(C, D),
                          Edge(D, E),
                          Edge(E, A), Edge(E, B) };

  Graph G;

 public:
  Graph_router() : G() {
    const int num_edges = sizeof(edges) / sizeof(edges[0]);
    for (int i = 0; i < num_edges; ++i) {
      DBUG_LOG("Routing", "Adding edge from " << name[edges[i].first] << " to " << name[edges[i].second]);
      add_edge(edges[i].first, edges[i].second, G);
    }
  }

  void printVertices();

};
