//
// Created by Håkon Strandlie on 06/05/2021.
//

#ifndef MYSQL_ROUTING_STATS_H
#define MYSQL_ROUTING_STATS_H

class RoutingStats {
 public:
  static inline unsigned long long numSwaps = 0;
  static inline unsigned long long numBytesRead = 0;
  static inline unsigned long long numBytesWritten = 0;
  static inline unsigned long long numRowsInGraph = 0;
  static inline unsigned long long numEdgesAdded = 0;
  static inline unsigned long long numBufferHits = 0;

  static void reset() {
    RoutingStats::numSwaps = 0;
    RoutingStats::numBytesRead = 0;
    RoutingStats::numBytesWritten = 0;
    RoutingStats::numRowsInGraph = 0;
    RoutingStats::numEdgesAdded = 0;
  }
};

#endif  // MYSQL_ROUTING_STATS_H
