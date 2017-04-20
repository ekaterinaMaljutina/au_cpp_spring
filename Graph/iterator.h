#ifndef ITERATOR_H
#define ITERATOR_H

#include "iterator"

template<class iter>
class base_edge {
    typedef typename iter::value_type value_type;
protected:
    value_type& ref(iter &it){
        return  const_cast<value_type&>(*it);
    }

    const value_type& ref(iter const  &it) const {
        return *it;
    }

};

template<class iter>
class base_vertex {
    typedef typename iter::value_type value_type;
protected:
    const value_type& ref(iter const  &it) const {
        return *it;
    }
};

template<class iter, class edge_data, class vertex_iterator>
class const_edge_policy {
public:
    typedef typename iter::value_type               value_type;
protected:
    typedef typename iter::difference_type          difference_type;
    typedef          value_type&                     reference ;
    typedef          value_type*                     pointer ;

    virtual const value_type& dereference(iter const & it) const = 0;
    iter iter_;
    static bool const flag_const_value = true;
public:public:
    const_edge_policy() {}
    const_edge_policy(iter const &it) : iter_(it) {}

    const edge_data* operator->() const {
        return &dereference(iter_).data();
    }

    const edge_data& operator*()  const {
        return dereference (iter_).data();
    }

    vertex_iterator from() const {
        return  dereference (iter_).from();
    }

    vertex_iterator to() const {
        return dereference (iter_).to();
    }
};


template<class iter, class edge_data, class vertex_iterator>
class edge_policy {
public:
    typedef typename iter::value_type                value_type;
protected:
    typedef typename iter::difference_type           difference_type;
    typedef          value_type&                     reference ;
    typedef          value_type*                     pointer ;

    virtual const value_type& dereference(iter const & it) const = 0;
    virtual       value_type& dereference(iter       & it)       = 0;

    iter iter_;
    static bool const flag_const_value = false;
public:public:
    edge_policy() {}
    edge_policy(iter const &it) : iter_(it) {}

    edge_data& operator*() {
        return dereference (iter_).data();
    }

    edge_data* operator->() {
        return &dereference(iter_).data();
    }

    vertex_iterator from() const {
        return dereference (iter_).from();
    }

    vertex_iterator  to() const {
        return dereference (iter_).to();
    }
};

template<class iter>
class vertex_policy {
public:
    typedef typename iter::value_type           value_type;
protected:
    typedef          value_type&                reference ;
    typedef          value_type*                pointer;
    virtual const value_type& dereference(iter const & it) const = 0;
    static bool const flag_const_value = true;
    iter iter_;
public:
    vertex_policy() : iter_() {}
    vertex_policy(iter const &it) : iter_(it) {}

    const value_type& operator *() const {
        return dereference (iter_);
    }
    const value_type* operator ->() const {
        return &dereference(iter_);
    }
};

template<class type>
struct filter_true {
    bool operator() (type const &) const{
        return true;
    }
};

template<class iter, class base_policy, class policy, class filter =
         filter_true<typename base_policy::value_type>>
struct iterator : public base_policy, policy {
    typedef typename base_policy::value_type        value_type;
    typedef typename base_policy::reference         reference ;
    typedef typename base_policy::pointer           pointer;
    typedef          std::ptrdiff_t                 difference_type;
    typedef          std::forward_iterator_tag      iterator_category;
    typedef          std::function<bool(value_type)>      filter_function;

    using base_policy::iter_;
    using policy::ref;

    iterator( ) : base_policy() { }
    iterator(iter const &it, filter_function filter_ = filter())
        :  base_policy(it), filter_(filter_)  { }

    iterator(iterator const &it) {
        iter_ = it.iter_;
        filter_ = it.filter_;
    }

    template<class iterator_type>
    iterator(iterator_type const& other,
                  typename std::enable_if<!iterator_type::flag_const_value>::type) {
        iter_ = other.iter_;
    }


    bool operator==(iterator const &it) {
        return iter_ == it.iter_;
    }

    bool operator!=(iterator const &it) {
        return iter_ != it.iter_;
    }

    iterator& operator++() {
        while ( !filter_(*(++iter_)));
        return *this;
    }

    iterator operator++(int) {
        iterator old = *this;
        while ( !filter_(*(++iter_)));
        return old;
    }

    const value_type& dereference(iter const & it) const {
        return ref(it);
    }

    value_type& dereference(iter & it) {
        return ref(it);
    }

private:
    filter_function filter_;
};

#endif // ITERATOR_H
