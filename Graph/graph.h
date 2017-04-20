#ifndef GRAPH_NEW_H
#define GRAPH_NEW_H

#include "unordered_set"
#include "unordered_map"
#include "vector"
#include "iterator.h"
#include "memory"

namespace au {

template<class vertex_type, class edge_type>
class graph {
public:
    struct hash_edge;

    typedef vertex_type vertex_data;
    typedef edge_type edge_data;

    typedef std::unordered_set<vertex_data> vertexies;

    using vertex_iterator_type = typename vertexies::const_iterator;
    using vertex_const_iterator_type = typename vertexies::const_iterator;

    using vertex_const_iterator = iterator<vertex_const_iterator_type,
                                vertex_policy<vertex_const_iterator_type>,
                                base_vertex<vertex_const_iterator_type>>;

    using vertex_iterator = vertex_const_iterator;

    struct edge {
        typedef edge_data value_type;

        edge () = default;

        edge(vertex_data const& from, vertex_data const& to) :
            from_(from), to_(to) {
        }

        edge(vertex_data const& from, vertex_data const& to,
             edge_data const& edges, std::shared_ptr<vertexies> const &vert):
                vertexies_prt(vert),
                from_(from), to_(to), data_(edges) { }

        bool operator == (edge const & e) const {
            return from_ == e.from_ && to_ == e.to_;
        }

        vertex_const_iterator from () const {
            return vertex_const_iterator(vertexies_prt->find (from_));
        }

        vertex_data from_data() const {
            return from_;
        }

        vertex_const_iterator to() const {
            return vertex_const_iterator(vertexies_prt->find (to_));
        }

        vertex_data to_data() const {
            return to_;
        }

        value_type& data() {
            return data_;
        }

        const value_type& data() const {
            return data_;
        }

        friend std::ostream& operator<<(std::ostream& os, edge const& edge) {
            os << "from: " <<edge.from ()<<" to: " <<edge.to ()
               << " ;";
            return os;
        }
    private:
        std::shared_ptr<vertexies> vertexies_prt;
        vertex_data from_;
        vertex_data to_;
        edge_data  data_;

    };

    struct hash_edge {
        size_t operator() (edge const &edge) const {
            return std::hash<vertex_data>()(edge.from_data ()) ^
                   std::hash<vertex_data>()(edge.to_data ());
        }
    };



    typedef std::unordered_set<edge, hash_edge> edge_set;
    typedef std::unordered_map<vertex_data, edge_set> edges;

    using edge_iterator_type = typename edge_set::iterator;
    using edge_const_iterator_type = typename edge_set::const_iterator;

    using edge_iterator = iterator<edge_iterator_type,
                                edge_policy<edge_iterator_type,
                                            edge_data,
                                            vertex_iterator>,
                                base_edge<edge_iterator_type>>;
    using edge_const_iterator = iterator<edge_const_iterator_type,
                                const_edge_policy<edge_const_iterator_type,
                                            edge_data,
                                            vertex_const_iterator>,
                                base_edge<edge_const_iterator_type>>;


    graph() {
        vertexies_ = std::make_shared<vertexies>();
    }

    vertex_iterator add_vertex(vertex_data const &data) {
        auto pair_iter = vertexies_->insert (data);
        if (pair_iter.second){
            return vertex_iterator(pair_iter.first);
        }
        return vertex_iterator();
    }

    edge_iterator add_edge (vertex_iterator const &from,
                            vertex_iterator const &to,
                            edge_data const& data) {
        return edge_iterator(edges_[*from].insert(
                { *from, *to, data, vertexies_ }).first);
    }

    void remove_vertex(vertex_iterator const  &iter) {
        auto value = *iter;
        std::unordered_map<vertex_data,edge> save;
        for (auto edges_begin = edges_.begin (); edges_begin != edges_.end ();
             edges_begin++) {
            for (auto item = edges_begin->second.begin();
                 item != edges_begin->second.end(); item++) {
                if(item->to_data() == value ||
                        item->from_data() == value) {
                    save.insert ({edges_begin->first, *item});
                }
            }
        }

        for (auto & item : save) {
            edges_[item.first].erase(item.second);
        }

        auto it = vertexies_->find (value) ;
        if ( it != vertexies_->end ()) {
            vertexies_->erase (it);
        }
    }

    void remove_edge(edge_iterator const &iter) {
        auto from = *iter.from ();
        edges_[from].erase({ from, *iter.to()}) ;
    }

    vertex_iterator find_vertex(vertex_data const &data) {
        return vertex_iterator(vertexies_->find (data));
    }

    edge_iterator find_edge(vertex_iterator const &from,
                            vertex_iterator const &to) {
        auto from_iter = edges_.find (*from);
        if (from_iter == edges_.end ()) {
            return edge_iterator();
        }
        auto iter_set = edges_.at (*from);
        auto iter =  iter_set.find({*from, *to});
        return edge_iterator(iter);
    }

    vertex_const_iterator find_vertex(vertex_data const & data) const {
        return vertex_const_iterator(vertexies_->find (data));
    }

    edge_const_iterator find_edge(vertex_iterator const  &from,
                                  vertex_iterator const  &to) const {
        auto from_iter = edges_.find (*from);
        if (from_iter == edges_.end ()) {
            return edge_const_iterator();
        }
        const auto iter_set = edges_.at (*from);
        auto iter =  iter_set.find({*from, *to});
        return edge_const_iterator(iter);
    }

    vertex_iterator vertex_begin() {
        return vertex_iterator(vertexies_->begin ());
    }

    vertex_iterator vertex_end() {
        return vertex_iterator(vertexies_->end ());
    }

    vertex_const_iterator vertex_begin() const {
        return vertex_const_iterator(vertexies_->begin ());
    }

    vertex_const_iterator vertex_end() const {
        return vertex_const_iterator(vertexies_->end ());
    }

    edge_iterator edge_begin(vertex_iterator const &from) {
        if (edges_.find (*from) != edges_.end ()) {
            return edge_iterator(edges_[*from].begin());
        }
        return edge_iterator();
    }

    edge_iterator edge_end(vertex_iterator const &from) {
        if (edges_.find (*from) != edges_.end ()) {
            return edge_iterator(edges_[*from].end());
        }
        return edge_iterator();
    }

    edge_const_iterator edge_begin(vertex_iterator const &from) const {
        if (edges_.find (*from) != edges_.cend()) {
            return edge_const_iterator(edges_.at (*from).cbegin());
        }
        return edge_const_iterator();
    }

    edge_const_iterator edge_end(vertex_iterator const  &from) const {
        if (edges_.find (*from) != edges_.cend()) {
            return edge_const_iterator(edges_.at (*from).cend());
        }
        return edge_const_iterator();
    }

private:
    std::shared_ptr<vertexies> vertexies_;
    edges edges_;

}; // class graph
}
// namespace au

#endif // GRAPH_NEW_H
