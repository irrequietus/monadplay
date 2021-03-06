# monadplay

Back in 1971 (that is several decades ago), a man named *Saunders Mac Lane* wrote a book called *"[Categories for the Working Mathematician](https://en.wikipedia.org/wiki/Categories_for_the_Working_Mathematician)"*. In one of the pages, it is written:

> All told, a monad in X is just a monoid in the category of endofunctors of X, with product × replaced by composition of endofunctors and unit set by the identity endofunctor. - [Saunders Mac Lane](https://en.wikipedia.org/wiki/Saunders_Mac_Lane)

This simple file uses `std::list` and defines the two operations as free functions `unit` and `prod` in such a way as to respect this exact definition, verifies the three monadic laws allowing the infamous Kleisli triple (known as Monad) to emerge naturally. Compile it with `-std=c++14` or whatever else is your (at least) C++14 compiler enabling mode. Comments help you navigate through the implementation as a concept more than anything else (it is really simple). The following is a comment-free snippet.

```c++
    auto f = [](int64_t x) { return unit(x * x); };
    auto g = [](int64_t x) { return unit(x + x); };
    
    auto law1 = [=](auto x) {
        return prod(f, unit(x)) == f(x);
    };
    
    auto law2 = [=](auto x) {
        typedef std::list<int64_t>(*U)(int64_t const &);
        return prod(static_cast<U>(&unit), unit(x)) == std::list<int64_t>{x};
    };
    
    auto law3 = [=](auto x) {
        return prod(f, prod(g, unit(x)))
            == prod([=](auto x) { return prod(f,g(x)); }, unit(x));
    };
```
The reason I wrote this is because `C++` is a language that makes it difficult for such constructs (as *monads*) to emerge and be used naturally for a variety of reasons related to its design; yet, there are simple ways that these may come about, despite the mental acrobatics one must do to deploy them.
