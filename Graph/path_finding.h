#ifndef SHORTED_PATH_H
#define SHORTED_PATH_H
#include "map"
#include "vector"
#include "algorithm"
#include <limits>


namespace au {

template<class graph, class edge_len, class path_visitor>
bool find_shortest_path(graph const& graph_,
                        typename graph::vertex_const_iterator from,
                        typename graph::vertex_const_iterator to,
                        edge_len && len_functor,
                        path_visitor&& visitor) {

    typedef typename graph::vertex_data vertex_data;
    typedef typename graph::edge edge;

    typedef std::map<vertex_data, double> distance;
    typedef std::map<vertex_data, edge> vertex_prev;
    typedef std::map<vertex_data, bool> visit;


    distance distance_;
    vertex_prev vertex_prev_;
    visit visit_;

    auto begin = graph_.vertex_begin();
    auto end = graph_.vertex_end();

    for (auto item = begin; item != end; item++) {
        visit_[*item] = false;
        distance_[*item] = std::numeric_limits<double>::infinity ();
    }
    distance_[*from] = .0;

    for (auto vertex = begin; vertex != end; vertex++) {
        // find min edge from not visited vertex
        auto current_vertex = end;
        for (auto item = begin; item != end; item++) {
            if (!visit_[*item] && (current_vertex == end  ||
                    distance_[*item] < distance_[*current_vertex])) {
                current_vertex = item;
            }
        }
        if (distance_[*current_vertex] == std::numeric_limits<double>::infinity()){
            break;
        }
        visit_[*current_vertex] = true;

        auto edge_begin_current_vertex  = graph_.edge_begin(current_vertex);
        auto edge_end_current_vertex = graph_.edge_end(current_vertex);

        for (auto current_edge = edge_begin_current_vertex;
             current_edge != edge_end_current_vertex; current_edge++) {
            if (distance_.find (*current_edge.from())
                    != distance_.end () &&
                    distance_.find (*current_edge.to()) != distance_.end ()  &&
                    distance_[*current_edge.from()] + len_functor(*current_edge) <
                                        distance_[*current_edge.to()] ) {
                distance_[*current_edge.to()] = distance_[*current_edge.from()] +
                                       len_functor(*current_edge) ;
                vertex_prev_[*current_edge.to()] = *current_edge;
            }
        }
    }

    std::vector<edge> full_path;
    auto  vertex = *to;
    for (; vertex != *from;) {
        if (vertex == vertex_prev_[vertex].from()) {
            break;
        }
        full_path.push_back (vertex_prev_[vertex]);
        vertex = vertex_prev_[vertex].from();
    }
    if (full_path.size () == 0 || (full_path.size () == 1 && full_path[0].from() == 0)) {
        return false;
    }
    std::reverse (full_path.begin(), full_path.end());
    for (const auto& edge : full_path) {
            visitor(edge);
    }

    return true;

}
} // namespace au

#endif // SHORTED_PATH_H
