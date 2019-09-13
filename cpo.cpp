#include<type_traits>
#include<algorithm>
#include<iostream>
#include<vector>
#include<cstddef>

namespace lib {
  namespace detail {
    struct no_adl_tag { };

    //create an ambiguity with std::swap
    template <typename T>
    no_adl_tag swap(T& a, T& b) = delete;

    //if there is a user defined overload
    template <typename T>
    decltype(swap(std::declval<T>(), std::declval<T>())) try_to_swap(int);

    //fallback
    template <typename T>
    no_adl_tag try_to_swap(long);

    template <typename T>
    inline constexpr bool is_adl_swapable =
      !std::is_same_v<decltype(detail::try_to_swap<T>(42)) , no_adl_tag>;

    struct cpo_swap {
      //Find any user defined overloads
      template<typename T>
      constexpr std::enable_if_t<detail::is_adl_swapable<T>>
      operator()(T&& t1, T&& t2) const {
        std::cout << "indirection\n";
        return swap(std::forward<T>(t1),
                    std::forward<T>(t2));
      }

      //Otherwise fallback to the generic case
      template<typename T>
      constexpr std::enable_if_t<!detail::is_adl_swapable<T> &&
				 std::is_move_constructible_v<T> &&
				 std::is_move_assignable_v<T>>
      operator()(T&& t1, T&& t2) const {
        //fallback for std::swap
        std::cout << "fallback std::swap\n";
        return std::swap(std::forward<T>(t1), std::forward<T>(t2));
      } 
    };
  } 
  //swap is a CPO. Inline constexpr allows multiple definitions of swap
  //in different translation units.
  inline constexpr auto swap = detail::cpo_swap();
}

namespace user {

  struct MyType {};
  
  void swap(MyType& t1, MyType& t2) {
    std::cout << "I am a CPO\n";
  } 

  void f() {
    MyType t1, t2;
    //using lib::swap;
    //auto v = std::vector<MyType<std::vector<int>>>(3);
    //auto v2 = std::vector<int>(3);
    using lib::swap;
    swap(t1, t2);
    //op(begin(v), end(v), begin(o), end(o), swap);
  }
}

int main() {
  user::f();
}
