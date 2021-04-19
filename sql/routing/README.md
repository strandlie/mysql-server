## One slight modification
For dynamic serialization / deserialization to work, a small change is needed to the Boost Graph Library. 

* In the Boost Library find the following file: `graph/detail/adjacency_list.hpp`
* Comment out `protected:` on line 401, so that it looks like this: `   //protected:`
