#include <iostream>
#include "assert.h"
#include "vector"
#include "algorithm"

#include <cassert>
#include <type_traits>
#include <iterator>
#include <iostream>
#include <vector>
#include <functional>


#include "graph.h"
#include "filtered_graph.h"
#include "path_finding.h"
using namespace std;

template<class T>
using is_const_ref = std::is_const<std::remove_reference_t<T>>;

template<class T>
void avoid_unused_typedef()
{}

template<class Iterator>
void check_iterator_concept(Iterator it, bool is_const)
{
    static_assert(std::is_same<std::forward_iterator_tag,
                  typename std::iterator_traits<Iterator>::iterator_category>::value, "kek");
    assert(is_const_ref<decltype((*it))>::value == is_const);

    using a = typename std::iterator_traits<Iterator>::value_type;
    using b = typename std::iterator_traits<Iterator>::reference;
    using c = typename std::iterator_traits<Iterator>::pointer;
    using d = typename std::iterator_traits<Iterator>::difference_type;
    avoid_unused_typedef<a>();
    avoid_unused_typedef<b>();
    avoid_unused_typedef<c>();
    avoid_unused_typedef<d>();

    Iterator tmp;
    tmp = it;
    assert(tmp == it);
    assert(tmp == it++);

    Iterator tmp2;
    using std::swap;
    swap(tmp2, it);
    assert(tmp != it);
    swap(tmp2, it);

    assert(++tmp == it);
    assert(++tmp != it);


    (void)*it;
    (void)it.operator->();
}

template<class Iterator, class ConstIterator>
void check_iterator_conversion()
{
    Iterator it;
//    ConstIterator cit = it;
//    (void)cit;
}

using simple_graph_t    = au::graph<int, int>;

simple_graph_t make_simple_graph()
{
    au::graph<int, int> g;
    g.add_vertex(1);
    g.add_vertex(2);
    g.add_vertex(3);
    g.add_vertex(4);

    auto v1 = g.find_vertex(1);
    auto v2 = g.find_vertex(2);
    auto v3 = g.find_vertex(3);
    auto v4 = g.find_vertex(4);

    g.add_edge(v1, v2, 1);
    g.add_edge(v1, v3, 2);
    g.add_edge(v2, v3, 3);

    g.add_edge(v4, v1, 2);
    g.add_edge(v4, v2, 2);
    g.add_edge(v4, v3, 10);

    return g;
}

void check_graph_concept()
{
    auto g = make_simple_graph();
    auto const &cg = g;

    check_iterator_concept(g.vertex_begin(), true);
    check_iterator_concept(cg.vertex_begin(), true);
    check_iterator_concept(g.edge_begin(g.find_vertex(1)), false);
    check_iterator_concept(cg.edge_begin(cg.find_vertex(1)), true);

    check_iterator_conversion<decltype(g)::vertex_iterator,
            decltype(g)::vertex_const_iterator>();
    check_iterator_conversion<decltype(g)::edge_iterator,
            decltype(g)::edge_const_iterator>();

    static_assert(std::is_same<decltype(g)::vertex_data, int>::value, "str");
    static_assert(std::is_same<decltype(g)::edge_data, int>::value, "str");

    decltype(g)::vertex_iterator       v1 = g.edge_begin(g.find_vertex(1)).from();
    decltype(g)::vertex_const_iterator cv1 = cg.edge_begin(cg.find_vertex(1)).from();

    decltype(g)::vertex_iterator       v2 = g.edge_begin(g.find_vertex(1)).to();
    decltype(g)::vertex_const_iterator cv2 = cg.edge_begin(cg.find_vertex(1)).to();

    assert(*v1 == 1  && *cv1 == 1);
    assert(*v2 == 2  || *v2 == 3);
    assert(*cv2 == 2 || *cv2 == 3);
}

void check_removing()
{
    auto g = make_simple_graph();

    auto v1 = g.find_vertex(1);
    auto v2 = g.find_vertex(2);
    auto v3 = g.find_vertex(3);

    g.remove_edge(g.find_edge(v2, v3));
    assert(g.edge_begin(v2) == g.edge_end(v2));

    g.remove_edge(g.find_edge(v1, v2));
    assert(g.edge_begin(v1) != g.edge_end(v1));

    g.remove_vertex(v3);
    v1 = g.find_vertex(1);
    assert(g.edge_begin(v1) == g.edge_end(v1));
}

void check_edge_iterator()
{
    const int max_id = 10000;

    auto g = make_simple_graph();
    for (int i = 4; i < max_id; ++i)
        g.add_vertex(i);

    auto e_it = g.find_edge(g.find_vertex(1), g.find_vertex(2));

    assert(*(e_it.from()) == 1);
    assert(*e_it.to()   == 2);
}

using simple_filtered_t = au::filtered_graph<simple_graph_t,
        std::function<bool(int)>, std::function<bool(int)>>;

simple_filtered_t make_simple_filtered(simple_graph_t const &g)
{
    auto vertex_predicate = [](int vertex_data)
    {
        return vertex_data != 3;
    };

    auto edge_predicate = [](int edge_data)
    {
        return edge_data != 1;
    };

    return simple_filtered_t(g, vertex_predicate, edge_predicate);
}

void check_filtered_graph()
{
    auto g  = make_simple_graph();
    auto fg = make_simple_filtered(g);

    auto v1 = fg.find_vertex(1);
    auto v2 = fg.find_vertex(2);
    auto v3 = fg.find_vertex(3);
    auto v4 = fg.find_vertex(4);

    assert(v1 != fg.vertex_end());
    assert(v2 != fg.vertex_end());
    assert(v3 == fg.vertex_end());
    assert(v4 != fg.vertex_end());

    check_iterator_concept(fg.vertex_begin(), true);
//    check_iterator_concept(fg.edge_begin(v4), true);

    using std::distance;
//    assert(distance(fg.edge_begin(v1), fg.edge_end(v1)) == 0);
//    assert(distance(fg.edge_begin(v2), fg.edge_end(v2)) == 0);
//    assert(distance(fg.edge_begin(v4), fg.edge_end(v4)) == 2);

//    assert(fg.find_edge(v1, v2) == fg.edge_end(v1));
//    assert(*fg.find_edge(v4, v2) == 2);
}

template<class Graph>
std::vector<int> collect_vertex_path(Graph const &g, int vertex_from, int vertex_to)
{
    auto it_from = g.find_vertex(vertex_from);
    auto it_to   = g.find_vertex(vertex_to);

    std::vector<int> res;
    auto edge_visitor = [&res](typename Graph::edge_const_iterator e_it)
    {
        res.push_back(*e_it.from());
    };

    auto len_functor = [](int value)
    {
        return static_cast<double>(value);
    };

    bool path_found = au::find_shortest_path(g, it_from, it_to, len_functor, edge_visitor);

    assert(path_found != res.empty() || it_from == it_to);
    if (path_found)
        res.push_back(*it_to);

    return res;
}

void check_shortest_path()
{
    auto g  = make_simple_graph();
    auto fg = make_simple_filtered(g);

    assert((collect_vertex_path(g, 1, 2) == std::vector<int>{1, 2}));
    assert((collect_vertex_path(g, 1, 3) == std::vector<int>{1, 3}));
    assert((collect_vertex_path(g, 1, 4) == std::vector<int>{}));

    assert((collect_vertex_path(g, 4, 1) == std::vector<int>{4, 1}));
    assert((collect_vertex_path(g, 4, 3) == std::vector<int>{4, 1, 2}));
    assert((collect_vertex_path(g, 4, 4) == std::vector<int>{4}));

    assert((collect_vertex_path(fg, 4, 1) == std::vector<int>{4, 1}));
    assert((collect_vertex_path(fg, 4, 3) == std::vector<int>{}));
    assert((collect_vertex_path(fg, 4, 4) == std::vector<int>{4}));
}


void test_add() {
    au::graph<size_t, size_t> g;

    auto vertex_1 = g.add_vertex (1);
    auto vertex_2 = g.add_vertex (2);
    auto vertex_3 = g.add_vertex (3);

    auto edge_1_2 = g.add_edge (vertex_1, vertex_2, 12);
    auto edge_1_3 = g.add_edge (vertex_1, vertex_3, 13);
    auto edge_2_3 = g.add_edge (vertex_2, vertex_3, 23);

    cout<<*edge_1_2<<" " <<*edge_1_3<<" "<<*edge_2_3<<endl;

    auto vertex_begin = g.vertex_begin ();
    auto vertex_end = g.vertex_end ();

    auto it_vertex = vertex_begin;
    for (size_t i = 3; it_vertex!= vertex_end; i--, it_vertex++ ) {
        std::cout<<*it_vertex<<std::endl;
        assert( i == *it_vertex);
    }

    auto edge_begin = g.edge_begin (vertex_1);
    auto edge_end = g.edge_end (vertex_1);


    std::vector<size_t> edge = {13, 12};
    auto it_edge = edge_begin;
    for (size_t i = 0; i< edge.size (); i++, it_edge++) {
        cout<<edge[i] << "==" <<*it_edge<<" with data "<<*it_edge <<std::endl;
        assert (edge[i] == *it_edge);
    }
    assert(edge_end == it_edge);
}

void test_remove_edge() {
    au::graph<int, int> g;

    auto vertex_1 = g.add_vertex (1);
    auto vertex_2 = g.add_vertex (2);
    auto vertex_3 = g.add_vertex (3);

    auto edge_1_2 = g.add_edge (vertex_1, vertex_2, 12);
    auto edge_1_3 = g.add_edge (vertex_1, vertex_3, 13);
    auto edge_2_3 = g.add_edge (vertex_2, vertex_3, 23);

    cout<<*edge_1_2 <<" " <<*edge_1_3<<" "<<*edge_2_3<<endl;

    g.remove_edge (edge_1_2);

    auto edge_begin = g.edge_begin (vertex_2);
    auto edge_end = g.edge_end (vertex_2);

    std::vector<int> edge = {23};
    for (size_t i = 0; i< edge.size ();i++, edge_begin++) {
        assert (edge[i] ==  *edge_begin);
    }

    edge = {13};
    edge_begin = g.edge_begin (vertex_1);
    edge_end = g.edge_end (vertex_1);
    for (size_t i = 0; i< edge.size ();i++) {
        assert (edge[i] == *edge_begin);
        edge_begin++;
    }
    assert(edge_begin == edge_end);

    edge = {23};
    edge_begin = g.edge_begin (vertex_2);
    edge_end = g.edge_end (vertex_2);
    for (size_t i = 0; i< edge.size ();i++, edge_begin++) {
        assert (edge[i] == *edge_begin);
    }

    g.remove_edge (edge_1_3);
    edge_begin = g.edge_begin (vertex_1);
    edge_end = g.edge_end (vertex_1);
    assert(edge_begin == edge_end);

}

void test_simple() {
    au::graph<int, int> g;
    auto v1 = g.add_vertex(0);
    v1 = g.find_vertex(0);
    auto v2 = g.add_vertex(1);
    g.add_edge(v1, v2, 0);
    auto e1 = g.find_edge(v1, v2);
    assert(e1 != g.edge_end(v1));
    g.remove_edge(e1);
    e1 = g.find_edge(v1, v2);
    assert(e1 == g.edge_end(v1));


    au::graph<std::string, int> gh;
    std::unordered_set<std::string> strings = {"aaa", "bbb"};
    for (auto it : strings) {
        gh.add_vertex(it);
    }
    std::unordered_set<std::string> results;

    for (auto iter = gh.vertex_begin(); iter != gh.vertex_end(); ++iter) {
        results.emplace(*iter);
    }
    assert(strings == results);
}

void test_find_vertex() {
    au::graph<std::string, int> gh;
    std::unordered_set<std::string> strings = {"aaa", "bbb"};

    for (auto it : strings) {
        gh.add_vertex(it);
    }

    auto it = gh.find_vertex("aaa");

    assert(*it == "aaa");
}

void const_test() {
    au::graph<std::string, int> gh;
    std::unordered_set<std::string> strings = {"aaa", "bbb"};
    for (auto it : strings) {
        gh.add_vertex(it);
    }
    au::graph<std::string, int> const cp = gh;
    std::unordered_set<std::string> actuals;
    for (auto iter = cp.vertex_begin(); iter != cp.vertex_end(); ++iter) {
        actuals.emplace(*iter);

    }
    assert(strings == actuals);
}

void test_find_edge() {
    au::graph<int, int> gh;

    auto from = gh.add_vertex(1);
    auto to = gh.add_vertex(2);

    gh.add_edge(from, to, 10);
    auto it = gh.find_edge(from, to);
    assert( *it.from() == 1);
    assert( *it.to() == 2);
}

struct func{
    bool operator()(int const& t) const {
        return t>=10;
    }
};

struct func_edges {
    bool operator()(au::graph<int, int>::edge const& t) const {
        return t.data () < 10;
    }
};

void test_filter_graph_vertex() {

    au::graph<int, int> gh;

    size_t size = 20;
    std::vector<int> vertices;
    for (size_t i =0; i< size; i++) {
        vertices.push_back (i);
    }
    std::unordered_set<int> filtered;
    for (size_t i = 10; i< size; i++) {
        filtered.insert(i);
    }
    std::unordered_set<int> actuals;
    for (size_t i = 0; i < vertices.size(); ++i) {
        gh.add_vertex(vertices[i]);
    }

    au::filtered_graph<au::graph<int, int>, func, func_edges> fl(gh);

    for(auto it = fl.vertex_begin(); it != fl.vertex_end(); ++it) {
        actuals.insert(*it);
    }

    assert(filtered == actuals);

}

void test_filter_graph_edge() {
    au::graph<int, int> gh;

    int from = 0;
    int size = 4;
    std::vector<int> tos;
    for (int i=0; i< size; i++) {
        tos.push_back (i);
    }

    gh.add_vertex(from);
    for (size_t i = 0; i < tos.size(); ++i) {
        gh.add_vertex(tos[i]);
    }
    auto from_it = gh.find_vertex(from);
    std::vector<int> edge_data;
    for (size_t i = 0; i < tos.size(); ++i) {
        auto to_it = gh.find_vertex(tos[i]);
        auto data = i*2;
        gh.add_edge(from_it, to_it, data);
        edge_data.push_back (data);
    }
    au::filtered_graph<au::graph<int, int>, func, func_edges> fl(gh);
    auto from_it_fl = fl.find_vertex(from);

    std::vector<int> res;
    for(auto it = fl.edge_begin(from_it_fl);
        it != fl.edge_end(from_it_fl); ++it) {
        cout << *it << endl;
        res.push_back ( (*it));
    }
    std::sort(res.begin (), res.end ());
    assert(res == edge_data);
}

void test_const_iterator() {
    au::graph<int, int> gh;
    std::unordered_set<int> ints = {1,2,3};
    for (auto it : ints) {
        gh.add_vertex(it);
    }
    au::graph<int, int> const cp = gh;
    std::unordered_set<int> actuals;
    for (auto iter = cp.vertex_begin(); iter != cp.vertex_end(); ++iter) {
        actuals.emplace(*iter);
    }
    assert( ints == actuals);
}


void test_short_path() {
    au::graph<int, int> graph;
    auto v1 = graph.add_vertex(1);
    auto v2 = graph.add_vertex(2);
    auto v3 = graph.add_vertex(3);
    auto v4 = graph.add_vertex(4);
    auto v5 = graph.add_vertex(5);
    graph.add_edge(v1, v2, 10);
    graph.add_edge(v2, v3, 10);
    graph.add_edge(v3, v4, 10);
    graph.add_edge(v4, v5, 10);
    auto len_function = [](const auto& w) {
        return w;
    };

    size_t sum = 0;
    auto visitor = [&sum](const auto& e) {
        sum += e.data();
    };

    au::graph<int, int>::vertex_const_iterator const_v1 = v1;
    au::graph<int, int>::vertex_const_iterator const_v2 = v2;
    au::graph<int, int>::vertex_const_iterator const_v3 = v3;
    au::graph<int, int>::vertex_const_iterator const_v4 = v4;
    au::graph<int, int>::vertex_const_iterator const_v5 = v5;
    assert(au::find_shortest_path(graph, const_v1, const_v5, len_function, visitor));
    assert(sum == 40);

    sum = 0;
    assert(au::find_shortest_path(graph, const_v1, const_v4, len_function, visitor));
    assert(sum == 30);

    sum = 0;
    assert(au::find_shortest_path(graph, const_v1, const_v3, len_function, visitor));
    assert(sum == 20);

    sum = 0;
    assert(au::find_shortest_path(graph, const_v1, const_v2, len_function, visitor));
    assert(sum == 10);

    sum = 0;
    assert(au::find_shortest_path(graph, const_v2, const_v5, len_function, visitor));
    assert(sum == 30);

    assert(!au::find_shortest_path(graph, const_v5, const_v1, len_function, visitor));
    assert(!au::find_shortest_path(graph, const_v4, const_v1, len_function, visitor));
    assert(!au::find_shortest_path(graph, const_v3, const_v1, len_function, visitor));
    assert(!au::find_shortest_path(graph, const_v2, const_v1, len_function, visitor));
    assert(!au::find_shortest_path(graph, const_v1, const_v1, len_function, visitor));

    assert(!au::find_shortest_path(graph, const_v3, const_v2, len_function, visitor));
    assert(!au::find_shortest_path(graph, const_v4, const_v2, len_function, visitor));
    assert(!au::find_shortest_path(graph, const_v5, const_v2, len_function, visitor));

    assert(!au::find_shortest_path(graph, const_v4, const_v3, len_function, visitor));
    assert(!au::find_shortest_path(graph, const_v5, const_v3, len_function, visitor));

    assert(!au::find_shortest_path(graph, const_v5, const_v4, len_function, visitor));
}

void test_reverse_edge() {
    au::graph<int, int> graph;
    auto v1 = graph.add_vertex(1);
    auto v2 = graph.add_vertex(2);
    auto v3 = graph.add_vertex(3);
    graph.add_edge(v1, v2, 10);
    graph.add_edge(v2, v1, 10);
    graph.add_edge(v2, v3, 10);
    graph.add_edge(v3, v2, 10);
    graph.add_edge(v3, v1, 10);

    graph.remove_vertex (v1);

    au::graph<int, int>::vertex_const_iterator const_v2 = v2;
    au::graph<int, int>::vertex_const_iterator const_v3 = v3;

    auto edge_begin_v2 = graph.edge_begin (const_v2);
    auto edge_end_v2 = graph.edge_end (const_v2);
    assert(*edge_begin_v2.from () == 2);
    assert(*edge_begin_v2.to() == 3);
    assert(*edge_begin_v2 == 10);
    edge_begin_v2++;
    assert(edge_begin_v2 == edge_end_v2);


    auto edge_begin_v3 = graph.edge_begin (const_v3);
    auto edge_end_v3 = graph.edge_end (const_v3);
    assert(*edge_begin_v3.from () == 3);
    assert(*edge_begin_v3.to() ==2);
    assert(*edge_begin_v3== 10);
    edge_begin_v3++;
    assert(edge_begin_v3 == edge_end_v3);

}

int main() {

    test_add ();
    test_const_iterator();
    test_remove_edge ();
    test_simple ();
    test_find_vertex ();
    const_test ();
    test_find_edge ();

    test_reverse_edge ();

    test_filter_graph_vertex ();
    test_filter_graph_edge ();

    test_short_path();


    check_graph_concept();
    check_removing();
    check_edge_iterator();
    check_filtered_graph();
    return 0;

    return 0;
}
