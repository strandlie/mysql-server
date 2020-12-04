//
// Created by Håkon Strandlie on 13/11/2020.
//

#include "graph_routing.h"

class Vertex;

Graph_router::Vertex Graph_router::getSource(int id) {
  using namespace boost;
  graph_traits<Graph>::vertex_iterator vi;
  vi = vertices(G).first;
  Graph_router::Vertex s;
  for( ; vi != vertices(G).second; ++vi) {
    s = *vi;
    if (s == (std::string::size_type) id) {
      return s;
    }
  }
  DBUG_LOG("Routing", "Did not find the node");
  return -1;
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
 * @param str The string we (eventually) want to write the result to
 */
void Graph_router::getPredecessorsTo(int id, String *str) {
  if (predecessors.size() == 0) {
    memcpy(str, nullptr, 0);
    return;
  }

  using namespace boost;

  typedef graph_traits<Graph>::vertex_iterator vertex_iter;
  std::pair<vertex_iter, vertex_iter> vp;
  vp = vertices(G);
  for ( ; vp.first != vp.second; ++vp.first) {
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
        };
      } while(parent != current && parent != graph_traits<Graph>::null_vertex());
    }
  }

}

/**
 * Create a pretty-formatted string for all distances in the graph to the source
 * @param str The address to (eventually) write the result string to
 */
void Graph_router::getDistances(String *str) {
  if (distances.size() == 0) {
    memcpy(str, nullptr, 0);
    return;
  }

  // TODO: Write to the string, not just debug-print
  DBUG_LOG("Routing", "Distances from start vertex: " << currentSource);
  b::graph_traits<Graph>::vertex_iterator vi;
  for(vi = vertices(G).first; vi != vertices(G).second; ++vi) {
    DBUG_LOG("Routing", "distance(" << index(*vi) << ") = " << distances[*vi]);
  }
}

/**
 * Only find the distance to a specific vertex
 * @param id The id of the vertex we want the distance to
 * @param str A result string to (eventually) write the result to
 */
void Graph_router::getDistancesTo(int id, String* str) {
  if (distances.size() == 0) {
    memcpy(str, nullptr, 0);
    return;
  }

  using namespace boost;

  typedef graph_traits<Graph>::vertex_iterator vertex_iter;
  std::pair<vertex_iter, vertex_iter> vp = vertices(G);
  for ( ; vp.first != vp.second; ++vp.first) {
    if (*vp.first == id) {
      DBUG_LOG("Routing", "distance(" << *vp.first << ") = " << distances[*vp.first]);
    }
  }
}