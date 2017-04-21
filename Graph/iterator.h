#ifndef ITERATOR_H
#define ITERATOR_H

#include "iterator"

template<class iter>
class base {
    typedef typename iter::value_type value_type;
protected:
    value_type& ref(iter &it){
        return  const_cast<value_type&>(*it);
    }
    const value_type& ref(iter const  &it) const {
        return *it;
    }
};

template<class iter, class vertex_iterator>
class const_edge_policy {
public:
    typedef typename iter::value_type::value_type    value_type;
protected:
    typedef typename iter::value_type                value_type_from_iter;
    typedef typename iter::difference_type           difference_type;
    typedef          value_type&                     reference ;
    typedef          value_type*                     pointer ;

    virtual const value_type_from_iter& dereference(iter const & it) const = 0;
    iter iter_;
    static bool const flag_const_value = true;
public:public:
    const_edge_policy() = default;
    const_edge_policy(iter const &it) : iter_(it) {}

    const value_type* operator->() const {
        return &dereference(iter_).data();
    }

    const value_type& operator*()  const {
        return dereference(iter_).data();
    }

    vertex_iterator from() const {
        return  dereference (iter_).from();
    }

    vertex_iterator to() const {
        return dereference (iter_).to();
    }
};


template<class iter, class vertex_iterator>
class edge_policy {
public:
    typedef typename iter::value_type::value_type    value_type; // edge_data
protected:
    typedef typename iter::value_type                value_type_from_iter; //edge
    typedef typename iter::difference_type           difference_type;
    typedef          value_type&                     reference ;
    typedef          value_type*                     pointer ;

    virtual const value_type_from_iter& dereference(iter const & it) const = 0;
    virtual       value_type_from_iter& dereference(iter       & it)       = 0;

    iter iter_;
    static bool const flag_const_value = false;
public:public:
    edge_policy() = default;
    edge_policy(iter const &it) : iter_(it) {}

    value_type& operator*() {
        return dereference (iter_).data();
    }

    value_type* operator->() {
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
    typedef typename iter::value_type           value_type_from_iter;
    typedef          value_type&                reference ;
    typedef          value_type*                pointer;
    virtual const value_type& dereference(iter const & it) const = 0;
    static bool const flag_const_value = true;
    iter iter_;
public:
    vertex_policy() = default;
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
    bool operator() (type &) {
        return true;
    }
};

template<class iter, class base_policy, class policy, class filter =
         filter_true<iter>>
struct iterator : public base_policy, policy {
    typedef typename base_policy::value_type        value_type;
    typedef typename base_policy::value_type_from_iter value_type_from_iter;
    typedef typename base_policy::reference         reference ;
    typedef typename base_policy::pointer           pointer;
    typedef          std::ptrdiff_t                 difference_type;
    typedef          std::forward_iterator_tag      iterator_category;

     using predicate = std::function<bool(iter)>;

    using base_policy::iter_;
    using base_policy::flag_const_value;
    using policy::ref;

    iterator() = default;
    iterator(iter const &it, predicate func = filter()) : base_policy(it),
        filter_(func) {end_ = iter(); }
    iterator(iter const &it, iter const &end, predicate func = filter()) :
        base_policy(it), filter_(func), end_(end) { }

    iterator(iterator const &it) :base_policy(it.iter_) {
        iter_ = it.iter_;
        filter_ = it.filter_;
        end_ = it.end_;
    }

    template<class iterator_type>
    iterator(iterator_type const &other,
                  typename std::enable_if<!iterator_type::flag_const_value>::type* = 0) {
        iter_ = other.iter_;
    }

    bool operator==(iterator const &it) const {
        return iter_ == it.iter_;
    }

    bool operator!=(iterator const &it) const {
        return iter_ != it.iter_;
    }

    iterator& operator++() {
        iter_++;
        while (iter_ != end_ && !filter_(iter_)) iter_++;
        return *this;
    }

    iterator operator++(int) {
        iterator old = *this;
        iter_++;
        while (iter_ != end_ && !filter_(iter_)) iter_++;
        return old;
    }

    const value_type_from_iter& dereference(iter const & it) const {
        return ref(it);
    }

    value_type_from_iter& dereference(iter & it) {
        return ref(it);
    }

private:
    predicate filter_;
    iter end_;
};

template<class iter, class vertex_iterator>
class filter_policy {
public:
    using value_type = typename iter::value_type;
protected:
    using value_type_from_iter = value_type;
    using reference = const value_type&;
    using pointer = const value_type*;

    virtual const value_type& dereference(iter const& it) const = 0;
    iter iter_;
    static const bool flag_const_value = true;

public:
    filter_policy() = default;
    filter_policy(iter const& it) : iter_(it) {}
    filter_policy(filter_policy const & policy) {
        iter_ = policy.iter_;
    }

    const value_type& operator*() const {
        return dereference(iter_);
    }
    const value_type* operator->() const {
        return &dereference(iter_);
    }

    vertex_iterator from() const {
        return iter_.from();
    }

    vertex_iterator to() const {
        return iter_.to();
    }
};

#endif // ITERATOR_H
