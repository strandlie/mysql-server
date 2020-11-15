//
// Created by Håkon Strandlie on 13/11/2020.
//

#ifndef MYSQL_GRAPH_ROUTING_H
#define MYSQL_GRAPH_ROUTING_H

#endif  // MYSQL_GRAPH_ROUTING_H

#include <include/my_sys.h>
#include <include/mysql/components/services/mysql_runtime_error_service.h>
#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/dijkstra_shortest_paths.hpp>
#include <boost/graph/graph_traits.hpp>
#include <boost/property_map/property_map.hpp>
#include "my_dbug.h"
#include "mysqld_error.h"
#include "sql_string.h"

namespace b = boost;
class Graph_router {
 private:
  /*
   * Typedefs
   */
  typedef b::adjacency_list<b::listS, b::vecS, b::directedS, b::no_property, b::property<b::edge_weight_t, int> > Graph;
  typedef b::property_map<Graph, b::vertex_index_t>::type IndexMap;
  typedef std::pair<int, int> Edge;
  typedef b::property<b::edge_weight_t, int> EdgeWeightProperty;

  /*
   * Utility
   */
  IndexMap index = get(b::vertex_index, G);

  /*
   * Training materials
   */
  enum { A, B, C, D, E, N };


  const char* name = "ABCDE";
  const Edge edges[9] = { Edge(A, C),
                          Edge(B, B), Edge(B, D), Edge(B, E),
                          Edge(C, B), Edge(C, D),
                          Edge(D, E),
                          Edge(E, A), Edge(E, B) };

  Graph G;

 public:
  typedef b::graph_traits<Graph>::vertex_descriptor Vertex;
  std::vector<double> distances;
  std::vector<Vertex> predecessors;
  Vertex currentSource;
  Graph_router(std::vector<Edge> edges, std::vector<double> weights) : G() {
    if (edges.size() != weights.size()) {
      my_error(ER_WRONG_PARAMETERS_TO_PROCEDURE, MYF(0), "Graph_router");
    }
    for (std::string::size_type i = 0; i < edges.size(); ++i) {
      DBUG_LOG("Routing", "Adding edge from " << edges[i].first << " to " << edges[i].second);
      add_edge(edges[i].first, edges[i].second, EdgeWeightProperty (weights[i]), G);
    }
    edges.clear();
    weights.clear();

    predecessors = std::vector<Vertex>(num_vertices(G), b::graph_traits<Graph>::null_vertex());
    distances = std::vector<double>(num_vertices(G));
  }

  void printVertices();
  Vertex *getSource(int id);
  void executeDijkstra(Vertex source);
  void getDistances(String *str);
  void getPredecessors(String *str);

};
