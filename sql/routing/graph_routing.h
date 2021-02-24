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
#include "sql/malloc_allocator.h"
#include "sql_string.h"
#include "routing_container.h"




/**
 * CUSTOM ALLOCATOR
 */
namespace boost {
struct vecS_profiled {};
template <typename ValueType>
struct container_gen<vecS_profiled, ValueType> {
  typedef routing::RVector<ValueType, Routing_allocator<ValueType>> type;
};

template <>
struct parallel_edge_traits<vecS_profiled> {
  typedef allow_parallel_edge_tag type;
};

namespace detail {
template <>
struct is_random_access<vecS_profiled> {
  enum { value = true };
  typedef mpl::true_ type;
};
}
}


/**
 * ROUTING IMPLEMENTATION
 */
namespace b = boost;
class Graph_router {
 private:
  /*
   * Typedefs
   */
  typedef b::adjacency_list<b::vecS_profiled, b::vecS_profiled, b::undirectedS,
                            b::no_property, b::property<b::edge_weight_t, double>>
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
  std::vector<double, Routing_allocator<double>> distances;
  std::vector<Vertex, Routing_allocator<Vertex>> predecessors;
  Vertex currentSource;
  Graph_router(std::vector<Edge> edges, std::vector<double> weights) : G() {
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

    predecessors = std::vector<Vertex, Routing_allocator<Vertex>>(
        num_vertices(G), b::graph_traits<Graph>::null_vertex());
    distances = std::vector<double, Routing_allocator<double>>(num_vertices(G));
  }

  Vertex getSource(int id);
  void executeDijkstra(Vertex source);
  void getDistances(String *str);
  void getDistancesTo(int id, String *str);
  void getPredecessorsTo(int id, String *str);
};
