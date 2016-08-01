// Copyright (C) 2016 George Makrydakis <george@irrequietus.eu>
// Licensed under MPLv2 (https://www.mozilla.org/en-US/MPL/2.0/)

#include <list>
#include <type_traits>
#include <cstdio>
#include <algorithm>

// The purpose of this code is purely educational, so that the relations between
// fundamental operations in functional programming constructs become clear to
// the reader versed in C++; it can be written in many different ways but since
// I was utterly bored I just picked one at random. This code requires C++14
// because of std::result_of_t, generic lambdas, though can be adapted to C++11
// but we are already heading towards to C++17. A std::list will be used for
// the examples and the purpose here is to see how far one can get  with the
// complexity of the constructs involved as well as whether certain laws are
// respected. Of course, it is all about the Monads.

// NOTE: Just compile with -std=c++14

/*
 * From Saunders Mac Lane's "Categories for the Working Mathematician", 1971:
 * 
 * "All told, a monad in X is just a monoid in the category of endofunctors of
 *  X, with product × replaced by composition of endofunctors and unit set by
 *  the identity endofunctor."
 * 
 * C++'s design is quite unhelpful in dealing with such constructs but it is
 * worth a try to understand what Saunders is talking about in coding terms.
 * 
 */

/* Step 1: define "unit" as *unary* operation for a std::list<X>; it represents
           the "identity endofunctor", essentially the constructor for a list.
*/
template<typename X> 
std::list<X> unit(X const & x)
{ return std::list<X>{x}; }

/* Step 2: define "prod" operation for a std::list<X>; if empty, returns empty,
           otherwise the idiomatic way of shifting around items in a list is
           deployed through a lambda (but could also be without it). Actually,
           "prod" is the infamous "bind" and beware that unlike "unit", it is
           a **binary** operation. Notice that "prod" is dedicated to std::list
           **endofunctor** composition; notice the **recursion** involved.
*/
template<typename F, typename X>
std::result_of_t<F(X)> prod(F f, std::list<X> x) {
    return (x.empty())
        ? std::result_of_t<F(X)>{}
        : [&]() { std::result_of_t<F(X)> y{ f(x.front()) };
                  x.pop_front();
                  y.splice(y.end(), prod(f,x));
                  return y;
                } ();
}

/* Step 4: "join" (or "flatten") can be defined in terms of prod. */
template<typename X>
std::list<X> join(std::list<std::list<X>> const& x) {
    return prod([](auto y) { return y; }, x);
}

/* Step 5: "fmap" can be defined in terms of prod, unit */
template<typename F, typename X>
std::list<X> fmap(F f, std::list<X> const & x) {
    return prod([=](auto y) { return unit(f(y)); }, x);
}

/* Step 6: "foldl" because it is quite easy to do anyway */
template<typename F, typename X, typename Y>
Y foldl(F f, std::list<X> const & m, Y y) {
    for(auto && i : m)
        y = f(y, i);
    return y;
}

int main () {
    
    /* Task 1: Let's create an integer sequence in a std::list<int64_t>
     
       The choice of int64_t is deliberate; if you wish to change the number
       of the items in the list to extreme sizes, you can have integer
       overflows in some of the operations performed for demonstrative
       purposes on the more complicated constructs in composition.
     
     */
    std::list<int64_t> ls(100);
    std::iota(ls.begin(), ls.end(), 0);
    
    /* Task 2: Complicating our life because we want to.
      
       Let's define two lambdas returning a std::list<int64_t> monad after
       a specific function has been applied to them. We choose addition
       and multiplication; a triple of notes:
       
        (a) the two operations have different identity elements
            (zero vs one) and their composition is NOT associative!
        (b) function application "returns" a list for every int64_teger
            used with such functions, because "unit" is deployed,
            providing a sort of "wrap".
        (c) The two functions are not directly composable and this
            is deliberate: they "generate", "construct", "return"
            a "wrap" through the use of "unit". That result of the
            "unit" is to allow their composition through the "prod"
            function.
    */
    
    auto f = [](int64_t x) { return unit(x * x); }; // "endofunctor"
    auto g = [](int64_t x) { return unit(x + x); }; // "endofunctor"
    
    /* Task 3: Given of how we have defined "unit" and "prod" let's see if
     *         the triple (std::list, unit, prod) forms a monad; the three
     *         laws of left/right identity and associativity must be proven
     *         as valid. Kleisli triples rock your world...
     */
    
    /* law1: left identity */
    auto law1 = [=](auto x) {
        return prod(f, unit(x)) == f(x);
    };
    
    /* law2: right identity; notice that we have to specify the proper overload
             and C++ has some particular rules about how to do that.
    */
    auto law2 = [=](auto x) {
        typedef std::list<int64_t>(*U)(int64_t const &);
        return prod(static_cast<U>(&unit), unit(x)) == std::list<int64_t>{x};
    };
    
    /* law3: associativity of product operation; notice "prod" is essentially
             associative and "unit" is its identity.
    */
    auto law3 = [=](auto x) {
        return prod(f, prod(g, unit(x)))
            == prod([=](auto x) { return prod(f,g(x)); }, unit(x));
    };
    
    auto laws_check = [=](auto s, auto x) {
        return s && law1(x) && law2(x) && law3(x);
    };
    
    if(foldl(laws_check, ls, true)) {
        printf("\nleft identity, right identity, associativity laws valid.\n");
        printf("... so, it is a monad after all!\n");
        printf("... so, we can now start playing and pay the consequences!\n");
    }
    
    /* Task 4: Let's run a couple of summations through folding and test these
               constructs, will they work properly? Everything is fair game
               from this point on so I am improvising.
     */
        
    // Some known formulas for assistive purposes like summation, element count.
    auto sum = [](auto x, auto y) { return x + y; };
    auto dif = [](auto x, auto y) { return x - y; };
    auto sqr = [](auto x) { return x * x; };
    auto sn1 = [](auto x) { return x*(x+1)/2; };
    auto sn2 = [](auto x) { return x*(x+1)*(2*x+1)/6; };
    

    
    // the sum of the doubles is: foldl(sum, prod(g, ls), int64_t{})
    printf("Sum of doubles of integer sequence 0,1,2,3,...,%d test: %s\n"
          , ls.size() - 1
          , foldl(sum, prod(g, ls), int64_t{}) == 2*sn1(ls.size()-1)
                ? "true" : "false" );
    
    // The sum of squares is: foldl(sum, prod(f, ls), int64_t{})
    printf("Sum of squares of integer sequence 0,1,2,3,...,%d test: %s\n"
          , ls.size() - 1
          , foldl(sum, prod(f, ls), int64_t{}) == sn2(ls.size()-1)
                ? "true" : "false" );
    
    // We can do even more complicated constructs, let's try it out for the
    // sake of demonstrative purposes; there is a rather peculiar relation
    // between the sum of squares and the square of the sum of our integer
    // sequence (see http://math.stackexchange.com/a/439238):
    //
    //   n * Σ(χ^2) - (Σ(χ))^2 = 1/2 * ΣiΣj((χ(i) - x(j))^2)
    //
    // Let's try and verify that computationally...
        
    auto par = [](auto x, auto y) { return [=](auto z) { return x(z,y); }; };
    auto dot = [](auto x, auto y) { return [=](auto z) { return x(y(z)); }; };
    
    auto dx_sqr
        = [&](auto x, auto l) { return fmap(dot(sqr,par(dif,x)), l); };
    
    auto sigma_squares =
        [=](auto & x) { return foldl(sum,fmap(sqr,x),int64_t{}); };
    
    auto sigma_sqr =
        [=](auto & x) { return sqr(foldl(sum,x, int64_t{})); };
        
    auto sigma_dx2 =
        [=](auto & x) { return foldl(sum,prod(par(dx_sqr,x),x), int64_t{}); };
    
    printf( "Sum of squares vs square of sums (provided no overflow): %s\n\n"
          , (ls.size() * sigma_squares(ls) - sigma_sqr(ls) == sigma_dx2(ls) / 2)
                ? "true" : "false (you overflowed it!)" );
  
    return {};
}
