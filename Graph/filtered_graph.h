#ifndef FILTERED_GRAPH_H
#define FILTERED_GRAPH_H

#include "graph.h"

namespace au {

template<class graph, typename vertex_filter,typename edge_filter>
class filtered_graph {
public:

    using vertex_data = typename graph::vertex_data;
    using edge_data = typename graph::edge_data;

    using edge = typename graph::edge;

    using vertex_const_iterator_graph  = typename graph::vertex_const_iterator;
    using edge_const_iterator_type  = typename graph::edge_const_iterator;

    using vertex_iterator = iterator<vertex_const_iterator_graph,
                                    vertex_policy<vertex_const_iterator_graph>,
                                    base_vertex<vertex_const_iterator_graph>,
                                    vertex_filter>;
    using edge_iterator = iterator<edge_const_iterator_type,
                                const_edge_policy<edge_const_iterator_type,
                                            edge_data,
                                            vertex_const_iterator_graph>,
                                base_edge<edge_const_iterator_type>>; //,
//                                edge_filter>;
    using edge_const_iterator = edge_iterator;

    filtered_graph(graph const& g) : graph_(g) {}

    filtered_graph(graph const& g, vertex_filter const &vertex_filter_,
                   edge_filter const & edge_filter_) :
        graph_(g), vertex_filter_(vertex_filter_),
        edge_filter_(edge_filter_) {}

    vertex_iterator find_vertex(vertex_data const& data) const {
        return vertex_iterator(graph_.find_vertex (data), vertex_filter_);
    }
    edge_iterator find_edge (vertex_iterator const &from,
                             vertex_iterator const &to) const {
        auto from_iter = graph_.find_vertex (*from);
        auto to_iter = graph_.find_vertex (*to);
        auto iter = graph_.find_edge (from_iter, to_iter);
        return edge_iterator(iter);
    }
    vertex_iterator vertex_begin() const {
        return vertex_iterator(graph_.vertex_begin ());
    }
    vertex_iterator vertex_end () const {
        return vertex_iterator(graph_.vertex_end ());
    }
    edge_iterator edge_begin(vertex_iterator const &from) const {
        auto from_iter = graph_.find_vertex (*from);
        return edge_iterator(graph_.edge_begin (from_iter));
    }
    edge_iterator edge_end (vertex_iterator const &from) const {
        auto from_iter = graph_.find_vertex (*from);
        return edge_iterator(graph_.edge_end (from_iter));
    }
private:
    graph const& graph_;
    vertex_filter vertex_filter_;
    edge_filter edge_filter_;
}; // class filtere_graph

} // namespace au
#endif // FILTERED_GRAPH_H
