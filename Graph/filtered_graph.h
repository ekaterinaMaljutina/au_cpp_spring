#ifndef FILTERED_GRAPH_H
#define FILTERED_GRAPH_H

#include "graph.h"

namespace au {

template<class graph, typename vertex_filter,typename edge_filter>
class filtered_graph {
public:

    typedef typename graph::vertex_data                  vertex_data;
    typedef typename graph::edge_data                    edge_data;
    typedef          std::function<bool(vertex_data)>    vertex_function;
    typedef          std::function<bool(edge_data)>      edge_function;
    typedef typename graph::vertex_const_iterator        vertex_const_iterator_graph;
    typedef typename graph::edge_const_iterator          edge_const_iterator_type;

    class vertex_filter_function {
    public:
        vertex_filter_function(vertex_function const &filter) : filter_(filter) { }

        bool operator()(vertex_const_iterator_graph const & iter) const {
            return filter_(*iter);
        }

    private:
        vertex_function filter_;
    };

    class edge_filter_function {
    public:
        edge_filter_function(edge_function const &filter,
                             vertex_function const &filter_vertex)
            : filter_(filter), filter_vertex_(filter_vertex) { }

        bool operator()(edge_const_iterator_type const & iter) const {
            return filter_(*iter) && filter_vertex_(*iter.from ())
                    && filter_vertex_(*iter.to());
        }

    private:
        edge_function filter_;
        vertex_function filter_vertex_;
    };

    using vertex_iterator = iterator<vertex_const_iterator_graph ,
                                    vertex_policy<vertex_const_iterator_graph >,
                                    base<vertex_const_iterator_graph>>;
    using edge_iterator = iterator<edge_const_iterator_type,
                                    filter_policy<edge_const_iterator_type,
                                            vertex_const_iterator_graph>,
                                    base<edge_const_iterator_type>>;
    using edge_const_iterator = edge_iterator;

    using vertex_const_iterator = vertex_iterator;

    filtered_graph(graph const& g) : graph_(g) {}

    filtered_graph(graph const& g, vertex_filter const &vertex_filter_,
                   edge_filter const & edge_filter_) :
        graph_(g), vertex_filter_(vertex_filter_),
        edge_filter_(edge_filter_) { }

    vertex_iterator find_vertex(vertex_data const& data) const {
        if (!vertex_filter_(data)) {
            return vertex_iterator(graph_.vertex_end (), graph_.vertex_end (),
                                   vertex_filter_function(vertex_filter_));
        }
        return vertex_iterator(graph_.find_vertex (data),
                               graph_.vertex_end (),
                               vertex_filter_function(vertex_filter_));
    }
    edge_iterator find_edge (vertex_iterator const &from,
                             vertex_iterator const &to) const {
        auto vertex_end = vertex_iterator(graph_.vertex_end ());
        if (from == vertex_end || to == vertex_end ) {
            return edge_iterator();
        }
        auto from_iter = graph_.find_vertex (*from);
        auto to_iter = graph_.find_vertex (*to);
        auto iter = graph_.find_edge (from_iter, to_iter);
        if (vertex_filter_(*iter.from()) && vertex_filter_(*iter.to())
                && edge_filter_(*iter)) {
            return edge_iterator(iter, graph_.edge_end (graph_.find_vertex (*from)),
                             edge_filter_function( edge_filter_,vertex_filter_));
        }
        return edge_iterator(graph_.edge_end (graph_.find_vertex (*from)),
                             graph_.edge_end (graph_.find_vertex (*from)),
                             edge_filter_function( edge_filter_,vertex_filter_));
    }
    vertex_iterator vertex_begin() const {
        if (graph_.vertex_begin () == graph_.vertex_end ()) {
            return vertex_iterator(graph_.vertex_end (),
                                   graph_.vertex_end (),
                                   vertex_filter_function( vertex_filter_));
        }
        vertex_iterator iter (graph_.vertex_begin (), graph_.vertex_end (),
                              vertex_filter_function( vertex_filter_));
        if (!vertex_filter_(*iter)) iter++;
        return iter;

    }
    vertex_iterator vertex_end () const {
        return vertex_iterator(graph_.vertex_end (),
                               graph_.vertex_end (),
                               vertex_filter_function(vertex_filter_));
    }
    edge_iterator edge_begin(vertex_iterator const &from) const {
        if (from == vertex_iterator(graph_.vertex_end ())) {
            return edge_iterator();
        }
        auto from_iter = graph_.find_vertex (*from);
        if (graph_.edge_begin (from_iter) == graph_.edge_end (from_iter)) {
            return edge_iterator(graph_.edge_end  (from_iter),
                                 graph_.edge_end (from_iter),
                                 edge_filter_function(edge_filter_,vertex_filter_));
        }

        edge_iterator iter(graph_.edge_begin (from_iter),
                             graph_.edge_end (from_iter),
                             edge_filter_function(edge_filter_,vertex_filter_));
        if (!vertex_filter_(*iter.from()) || !vertex_filter_(*iter.to())
                || !edge_filter_(*iter)) iter++;
        return iter;
    }
    edge_iterator edge_end (vertex_iterator const &from) const {
        auto from_iter = graph_.find_vertex (*from);
        if (from_iter == graph_.vertex_end ()) {
            return edge_iterator();
        }
        return edge_iterator(graph_.edge_end (from_iter),
                             graph_.edge_end (from_iter),
                             edge_filter_function(edge_filter_, vertex_filter_));
    }
private:
    graph const& graph_;
    vertex_filter vertex_filter_;
    edge_filter edge_filter_;
}; // class filtere_graph

} // namespace au
#endif // FILTERED_GRAPH_H
