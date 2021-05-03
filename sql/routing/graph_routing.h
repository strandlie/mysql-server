//
// Created by Håkon Strandlie on 13/11/2020.
//

#ifndef MYSQL_GRAPH_ROUTING_H
#define MYSQL_GRAPH_ROUTING_H

#endif  // MYSQL_GRAPH_ROUTING_H

#include <include/my_sys.h>
#include <include/mysql/components/services/mysql_runtime_error_service.h>
#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/graph_traits.hpp>
#include <boost/property_map/property_map.hpp>
#include "my_dbug.h"
#include "mysqld_error.h"
#include "rvector.h"
//#include "sql/malloc_allocator.h"
#include "sql_string.h"

/**
 * ROUTING IMPLEMENTATION
 */
namespace b = boost;
class Graph_router {
 private:
  /*
   * Typedefs
   */
  typedef b::adjacency_list<
      b::vecS, b::vecS_profiled, b::undirectedS, b::no_property,
      b::property<b::edge_weight_t, double>, b::no_property, b::vecS>//, b::no_property, b::vecS_profiled>
      Graph;
  typedef b::property_map<Graph, b::vertex_index_t>::type IndexMap;
  typedef std::pair<long, long> Edge;
  typedef b::property<b::edge_weight_t, double> EdgeWeightProperty;

  /*
   * Utility
   */
  IndexMap index = get(b::vertex_index, G);

  Graph G;

 public:
  typedef b::graph_traits<Graph>::vertex_descriptor Vertex;
  // std::vector<double, Routing_allocator<double>> distances;
  std::vector<double> distances;
  // std::vector<Vertex, Routing_allocator<Vertex>> predecessors;
  std::vector<Vertex> predecessors;
  Vertex currentSource;
  Graph_router(std::vector<Edge> edges, std::vector<double> weights);

  Graph_router();

  static Vertex null_vertex() { return b::graph_traits<Graph>::null_vertex(); }
  Vertex getSource(unsigned long id);
  void executeDijkstra(Vertex source);
  std::vector<std::pair<Vertex, double>> getDistances();
  std::pair<Vertex, double> getDistancesTo(unsigned long id);
  std::vector<Vertex> getPredecessorsTo(unsigned long id);
  String *produceDistanceString(std::vector<std::pair<Vertex, double>>);
  void producePredString(String*, std::vector<Vertex>, long);
};
