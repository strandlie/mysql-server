//
// Created by Håkon Strandlie on 13/11/2020.
//

#include "graph_routing.h"
#include <boost/geometry.hpp>
#include <boost/graph/astar_search.hpp>
#include <boost/graph/dijkstra_shortest_paths.hpp>
#include <vector>
#include "routing_stats.h"

class Vertex;

Graph_router::Graph_router(std::vector<Edge> edges, std::vector<double> weights,
                           Edge source_and_target)
    : G() {
  if (edges.size() != weights.size()) {
    my_error(ER_WRONG_PARAMETERS_TO_PROCEDURE, MYF(0), "Graph_router");
    return;
  }

  // Prune edges that are too far away that was not pruned away in
  // Item_sum_route::add
  typedef boost::geometry::cs::geographic<boost::geometry::radian> Wgs84Coords;
  typedef boost::geometry::model::point<double, 2, Wgs84Coords> GeographicPoint;

  double mid_long = (std::get<1>(source_and_target).first +
                     std::get<2>(source_and_target).first) /
                    2;
  double mid_lati = (std::get<1>(source_and_target).second +
                     std::get<2>(source_and_target).second) /
                    2;

  GeographicPoint route_mid_point = GeographicPoint(mid_long, mid_lati);
  GeographicPoint route_end_point =
      GeographicPoint(std::get<2>(source_and_target).first,
                      std::get<2>(source_and_target).second);
  std::vector<Edge> prunedEdges;
  std::vector<double> prunedWeights;
  double radius =
      boost::geometry::distance(route_end_point, route_mid_point) * 2;
  for (auto i = 0; i < edges.size(); i++) {
    if (boost::geometry::distance(route_mid_point,
                                  GeographicPoint(std::get<1>(edges[i]).first,
                                                  std::get<1>(edges[i]).second)) <
            radius ||
        boost::geometry::distance(route_mid_point,
                                  GeographicPoint(std::get<2>(edges[i]).first,
                                                  std::get<2>(edges[i]).second)) <
            radius) {
      prunedEdges.push_back(edges[i]);
      prunedWeights.push_back(weights[i]);
    }
  }

  edges.clear();
  weights.clear();

  for (std::string::size_type i = 0; i < prunedEdges.size(); ++i) {
    add_edge(std::get<0>(prunedEdges[i]).first, std::get<0>(prunedEdges[i]).second,
             EdgeWeightProperty(prunedWeights[i]), G);
  }
  RoutingStats::numEdgesAdded += prunedEdges.size();
  prunedEdges.clear();
  prunedWeights.clear();

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

  for (v_iter vi = vertices(G).first; vi != vertices(G).second; ++vi) {
    if (*vi == (std::string::size_type)id) {
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
  dijkstra_shortest_paths(
      G, source,
      b::distance_map(&distances[0]).predecessor_map(&predecessors[0]));
  currentSource = source;  // Only add the new source after successful dijkstra
}

/**
 * Get the predecessors for a specific vertex
 * @param id The id of the vertex we want to find the predecessors to
 * @return A vector of the predecessors, with the source node at the
 * end, and the target node at the beginning
 */
std::vector<Graph_router::Vertex> Graph_router::getPredecessorsTo(
    unsigned long id) {
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
        if (parent == graph_traits<Graph>::null_vertex()) {
          // DBUG_LOG("Routing", "parent(" << current << ") = no parent");
        } else {
          // DBUG_LOG("Routing", "parent(" << current << ") = " << parent);
          vec.push_back(parent);
        }
      } while (parent != current &&
               parent != graph_traits<Graph>::null_vertex());
    }
  }
  return vec;
}

std::vector<std::pair<Graph_router::Vertex, double>>
Graph_router::getDistances() {
  std::vector<std::pair<Graph_router::Vertex, double>> vec;
  if (distances.empty()) {
    return vec;
  }

  // DBUG_LOG("Routing", "Distances from start vertex: " << currentSource);
  b::graph_traits<Graph>::vertex_iterator vi;
  for (vi = vertices(G).first; vi != vertices(G).second; ++vi) {
    // DBUG_LOG("Routing", "distance(" << index(*vi) << ") = " <<
    // distances[*vi]);
    vec.emplace_back(index(*vi), distances[*vi]);
  }
  return vec;
}

/**
 * Only find the distance to a specific vertex
 * @param id The id of the vertex we want the distance to
 */
std::pair<Graph_router::Vertex, double> Graph_router::getDistancesTo(
    unsigned long id) {
  using namespace boost;
  if (distances.empty()) {
    return std::pair<Graph_router::Vertex, double>{Graph_router::null_vertex(),
                                                   0};
  }

  typedef graph_traits<Graph>::vertex_iterator vertex_iter;
  std::pair<vertex_iter, vertex_iter> vp = vertices(G);
  for (; vp.first != vp.second; ++vp.first) {
    if (*vp.first == id) {
      // DBUG_LOG("Routing", "distance(" << *vp.first << ") = " <<
      // distances[*vp.first]);
      return std::pair<Graph_router::Vertex, double>(*vp.first,
                                                     distances[*vp.first]);
    }
  }
  return std::pair<Graph_router::Vertex, double>{Graph_router::null_vertex(),
                                                 0};
}

String *Graph_router::produceDistanceString(
    std::vector<std::pair<Vertex, double>> dist_map) {}

void Graph_router::producePredString(String *str, std::vector<Vertex> preds,
                                     long tgt_node_id) {
  std::vector<Graph_router::Vertex>::reverse_iterator rit = preds.rbegin();
  str->append("Source: \n");
  for (; rit != preds.rend(); ++rit) {
    str->append("\t|--> ");
    str->append_longlong(*rit);
    str->append("\n");
  }
  str->append("Target: ");
  str->append_longlong(tgt_node_id);
  str->append("\n");

  str->append("Num swaps: ");
  str->append_longlong(RoutingStats::numSwaps);
  str->append("\n");
  str->append("Num hits: ");
  str->append_longlong(RoutingStats::numBufferHits);
  str->append("\n");
  str->append("Num bytes read: ");
  str->append_longlong(RoutingStats::numBytesRead);
  str->append("\n");
  str->append("Num bytes written: ");
  str->append_longlong(RoutingStats::numBytesWritten);
  str->append("\n");
  str->append("Num edges in graph: ");
  str->append_longlong(RoutingStats::numRowsInGraph);
  str->append("\n");
  str->append("Num edges added: ");
  str->append_longlong(RoutingStats::numEdgesAdded);
  str->append("\n");
}