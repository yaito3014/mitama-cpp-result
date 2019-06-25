#ifndef MITAMA_RESULT_IMPL_HPP
#define MITAMA_RESULT_IMPL_HPP

#include <result/detail/meta.hpp>
#include <result/traits/impl_traits.hpp>
#include <result/traits/Deref.hpp>
#include <result/detail/dangling.hpp>
#include <optional>
#include <functional>
#include <boost/optional.hpp>
#include <boost/optional/optional_io.hpp>
#include <boost/utility/in_place_factory.hpp>

namespace mitama {

template <class, class = void>
class unwrap_or_default_friend_injector
{
public:
  void unwrap_or_default() const = delete;
};

template <mutability _mu, class T, class E>
class unwrap_or_default_friend_injector<basic_result<_mu, T, E>,
                                        std::enable_if_t<std::disjunction_v<std::is_default_constructible<T>, std::is_aggregate<T>>>>
{
public:
  T unwrap_or_default() const
  {
    if constexpr (std::is_aggregate_v<T>){
      return static_cast<basic_result<_mu, T, E> const *>(this)->is_ok()
        ? static_cast<basic_result<_mu, T, E> const *>(this)->unwrap()
        : T{};
    }
    else {
      return static_cast<basic_result<_mu, T, E> const *>(this)->is_ok()
        ? static_cast<basic_result<_mu, T, E> const *>(this)->unwrap()
        : T();
    }
  }
};

template <class, class = void>
class transpose_friend_injector
{
public:
  void transpose() const = delete;
};

/// @brief
///   impl<_mutability, T, E> basic_result<_mutability, Option<T>, E>
template <mutability _mutability, class T, class E>
class transpose_friend_injector<basic_result<_mutability, T, E>,
                                std::enable_if_t<meta::is_optional<std::decay_t<T>>::value>>
{
  using optional_type = meta::remove_cvr_t<typename meta::repack<T>::template type<basic_result<_mutability, typename meta::remove_cvr_t<T>::value_type, E>>>;
public:
  optional_type transpose() const &
  {
    if constexpr (meta::is_boost_optional<meta::remove_cvr_t<T>>::value) {
      if (static_cast<basic_result<_mutability, T, E> const *>(this)->is_ok()) {
        if (auto const& opt = boost::get<success<T>>(static_cast<basic_result<_mutability, T, E> const*>(this)->storage_).x; opt) {
          return optional_type(boost::in_place(mitama::in_place_ok, opt.value()));
        }
        else {
          return boost::none;
        }
      }
      else {
          return optional_type(boost::in_place(mitama::in_place_err, boost::get<failure<E>>(static_cast<basic_result<_mutability, T, E> const*>(this)->storage_).x));
      }
    }
    else {
      if (static_cast<basic_result<_mutability, T, E> const *>(this)->is_ok()) {
        if (auto const& opt = std::get<success<T>>(static_cast<basic_result<_mutability, T, E> const*>(this)->storage_).x; opt) {
          return optional_type(std::in_place, mitama::in_place_ok, opt.value());
        }
        else {
          return std::nullopt;
        }
      }
      else {
          return optional_type(std::in_place, mitama::in_place_err, std::get<failure<E>>(static_cast<basic_result<_mutability, T, E> const*>(this)->storage_).x);
      }
    }
  }
};


template <class, class = void>
class indirect_friend_injector
{
public:
  void indirect() const = delete;
  void indirect_ok() const = delete;
  void indirect_err() const = delete;
};

/// @brief
/// impl<_mutability, T, E> basic_result<_mutability, T, E>
/// where
///   T: Deref
///   E: Deref
template <mutability _mutability, class T, class E>
class indirect_friend_injector<basic_result<_mutability, T, E>, 
                            std::enable_if_t<
                              std::conjunction_v<
                                traits::is_dereferencable<T>,
                                traits::is_dereferencable<E>
                            >>>
{
  using indirect_ok_result = basic_result<_mutability, std::remove_reference_t<typename traits::Deref<T>::Target>&, std::remove_reference_t<E>&>;
  using indirect_err_result = basic_result<_mutability, std::remove_reference_t<T>&, std::remove_reference_t<typename traits::Deref<E>::Target>&>;
  using indirect_result = basic_result<_mutability, std::remove_reference_t<typename traits::Deref<T>::Target>&, std::remove_reference_t<typename traits::Deref<E>::Target>&>;
  using const_indirect_ok_result = basic_result<_mutability, meta::remove_cvr_t<typename traits::Deref<T>::Target> const&, meta::remove_cvr_t<E> const&>;
  using const_indirect_err_result = basic_result<_mutability, meta::remove_cvr_t<T> const&, meta::remove_cvr_t<typename traits::Deref<E>::Target> const&>;
  using const_indirect_result = basic_result<_mutability, meta::remove_cvr_t<typename traits::Deref<T>::Target> const&, meta::remove_cvr_t<typename traits::Deref<E>::Target> const&>;
  using dangling_indirect_ok_result = basic_result<_mutability, dangling<std::reference_wrapper<std::remove_reference_t<typename traits::Deref<T>::Target>>>, dangling<std::reference_wrapper<std::remove_reference_t<E>>>>;
  using dangling_indirect_err_result = basic_result<_mutability, dangling<std::remove_reference_t<T>&>, dangling<std::reference_wrapper<std::remove_reference_t<typename traits::Deref<E>::Target>>>>;
  using dangling_indirect_result = basic_result<_mutability, dangling<std::reference_wrapper<std::remove_reference_t<typename traits::Deref<T>::Target>>>, dangling<std::reference_wrapper<std::remove_reference_t<typename traits::Deref<E>::Target>>>>;
public:

  constexpr auto indirect_ok() & -> indirect_ok_result {
    if ( static_cast<basic_result<_mutability, T, E> const *>(this)->is_ok() ) {
      return indirect_ok_result{in_place_ok, *boost::get<success<T>>(static_cast<basic_result<_mutability, T, E>*>(this)->storage_).x};
    }
    else {
      return indirect_ok_result{in_place_err, boost::get<failure<E>>(static_cast<basic_result<_mutability, T, E>*>(this)->storage_).x};
    }
  }
  constexpr auto indirect_err() & -> indirect_err_result {
    if ( static_cast<basic_result<_mutability, T, E> const *>(this)->is_ok() ) {
      return indirect_err_result{in_place_ok, boost::get<success<T>>(static_cast<basic_result<_mutability, T, E>*>(this)->storage_).x};
    }
    else {
      return indirect_err_result{in_place_err, *boost::get<failure<E>>(static_cast<basic_result<_mutability, T, E>*>(this)->storage_).x};
    }
  }
  constexpr auto indirect() & -> indirect_result {
    if ( static_cast<basic_result<_mutability, T, E> const *>(this)->is_ok() ) {
      return indirect_result{in_place_ok, *boost::get<success<T>>(static_cast<basic_result<_mutability, T, E>*>(this)->storage_).x};
    }
    else {
      return indirect_result{in_place_err, *boost::get<failure<E>>(static_cast<basic_result<_mutability, T, E>*>(this)->storage_).x};
    }
  }



  constexpr auto indirect_ok() const & -> const_indirect_ok_result {
    if ( static_cast<basic_result<_mutability, T, E> const *>(this)->is_ok() ) {
      return const_indirect_ok_result{in_place_ok, *boost::get<success<T>>(static_cast<basic_result<_mutability, T, E>*>(this)->storage_).x};
    }
    else {
      return const_indirect_ok_result{in_place_err, boost::get<failure<E>>(static_cast<basic_result<_mutability, T, E>*>(this)->storage_).x};
    }
  }
  constexpr auto indirect_err() const & -> const_indirect_err_result {
    if ( static_cast<basic_result<_mutability, T, E> const *>(this)->is_ok() ) {
      return const_indirect_err_result{in_place_ok, boost::get<success<T>>(static_cast<basic_result<_mutability, T, E>*>(this)->storage_).x};
    }
    else {
      return const_indirect_err_result{in_place_err, *boost::get<failure<E>>(static_cast<basic_result<_mutability, T, E>*>(this)->storage_).x};
    }
  }
  constexpr auto indirect() const & -> const_indirect_result {
    if ( static_cast<basic_result<_mutability, T, E> const *>(this)->is_ok() ) {
      return const_indirect_result{in_place_ok, *boost::get<success<T>>(static_cast<basic_result<_mutability, T, E>*>(this)->storage_).x};
    }
    else {
      return const_indirect_result{in_place_err, *boost::get<failure<E>>(static_cast<basic_result<_mutability, T, E>*>(this)->storage_).x};
    }
  }



  constexpr auto indirect_ok() && -> dangling_indirect_ok_result {
    if ( static_cast<basic_result<_mutability, T, E> const *>(this)->is_ok() ) {
      return dangling_indirect_ok_result{in_place_ok, std::ref(*boost::get<success<T>>(static_cast<basic_result<_mutability, T, E>*>(this)->storage_).x)};
    }
    else {
      return dangling_indirect_ok_result{in_place_err, boost::get<failure<E>>(static_cast<basic_result<_mutability, T, E>*>(this)->storage_).x};
    }
  }
  constexpr auto indirect_err() && -> dangling_indirect_err_result {
    if ( static_cast<basic_result<_mutability, T, E> const *>(this)->is_ok() ) {
      return dangling_indirect_err_result{in_place_ok, boost::get<success<T>>(static_cast<basic_result<_mutability, T, E>*>(this)->storage_).x};
    }
    else {
      return dangling_indirect_err_result{in_place_err, std::ref(*boost::get<failure<E>>(static_cast<basic_result<_mutability, T, E>*>(this)->storage_).x)};
    }
  }
  constexpr auto indirect() && -> dangling_indirect_result {
    if ( static_cast<basic_result<_mutability, T, E> const *>(this)->is_ok() ) {
      return dangling_indirect_result{in_place_ok, std::ref(*boost::get<success<T>>(static_cast<basic_result<_mutability, T, E>*>(this)->storage_).x)};
    }
    else {
      return dangling_indirect_result{in_place_err, std::ref(*boost::get<failure<E>>(static_cast<basic_result<_mutability, T, E>*>(this)->storage_).x)};
    }
  }

  constexpr void indirect_ok() const && = delete;
  constexpr void indirect_err() const && = delete;
  constexpr void indirect() const && = delete;

};

/// @brief
/// impl<_mutability, T, E> basic_result<_mutability, T, E>
/// where
///   T: Deref
template <mutability _mutability, class T, class E>
class indirect_friend_injector<basic_result<_mutability, T, E>, 
                            std::enable_if_t<
                              std::conjunction_v<
                                traits::is_dereferencable<T>,
                                std::negation<traits::is_dereferencable<E>>
                            >>>
{
  using indirect_ok_result = basic_result<_mutability, std::remove_reference_t<typename traits::Deref<T>::Target>&, std::remove_reference_t<E>&>;
  using const_indirect_ok_result = basic_result<_mutability, meta::remove_cvr_t<typename traits::Deref<T>::Target> const&, meta::remove_cvr_t<E> const&>;
  using dangling_indirect_ok_result = basic_result<_mutability, dangling<std::reference_wrapper<std::remove_reference_t<typename traits::Deref<T>::Target>>>, dangling<std::reference_wrapper<std::remove_reference_t<E>>>>;
public:
  constexpr auto indirect_ok() & -> indirect_ok_result {
    if ( static_cast<basic_result<_mutability, T, E> const *>(this)->is_ok() ) {
      return indirect_ok_result{in_place_ok, *boost::get<success<T>>(static_cast<basic_result<_mutability, T, E>*>(this)->storage_).x};
    }
    else {
      return indirect_ok_result{in_place_err, boost::get<failure<E>>(static_cast<basic_result<_mutability, T, E>*>(this)->storage_).x};
    }
  }
  constexpr void indirect_err() & = delete;
  constexpr void indirect() & = delete;



  constexpr auto indirect_ok() const & -> const_indirect_ok_result {
    if ( static_cast<basic_result<_mutability, T, E> const *>(this)->is_ok() ) {
      return const_indirect_ok_result{in_place_ok, *boost::get<success<T>>(static_cast<basic_result<_mutability, T, E>*>(this)->storage_).x};
    }
    else {
      return const_indirect_ok_result{in_place_err, boost::get<failure<E>>(static_cast<basic_result<_mutability, T, E>*>(this)->storage_).x};
    }
  }
  constexpr void indirect_err() const & = delete;
  constexpr void indirect() const & = delete;



  constexpr auto indirect_ok() && -> dangling_indirect_ok_result {
    if ( static_cast<basic_result<_mutability, T, E> const *>(this)->is_ok() ) {
      return dangling_indirect_ok_result{in_place_ok, std::ref(*boost::get<success<T>>(static_cast<basic_result<_mutability, T, E>*>(this)->storage_).x)};
    }
    else {
      return dangling_indirect_ok_result{in_place_err, boost::get<failure<E>>(static_cast<basic_result<_mutability, T, E>*>(this)->storage_).x};
    }
  }
  constexpr void indirect_err() && = delete;
  constexpr void indirect() && = delete;

  constexpr void indirect_ok() const && = delete;
  constexpr void indirect_err() const && = delete;
  constexpr void indirect() const && = delete;

};

/// @brief
/// impl<_mutability, T, E> basic_result<_mutability, T, E>
/// where
///   E: Deref
template <mutability _mutability, class T, class E>
class indirect_friend_injector<basic_result<_mutability, T, E>, 
                            std::enable_if_t<
                              std::conjunction_v<
                                std::negation<traits::is_dereferencable<T>>,
                                traits::is_dereferencable<E>
                            >>>
{
  using indirect_err_result = basic_result<_mutability, std::remove_reference_t<T>&, std::remove_reference_t<typename traits::Deref<E>::Target>&>;
  using const_indirect_err_result = basic_result<_mutability, meta::remove_cvr_t<T> const&, meta::remove_cvr_t<typename traits::Deref<E>::Target> const&>;
  using dangling_indirect_err_result = basic_result<_mutability, dangling<std::remove_reference_t<T>&>, dangling<std::remove_reference_t<typename traits::Deref<E>::Target>&>>;
public:
  constexpr void indirect_ok() & = delete;
  constexpr auto indirect_err() & -> indirect_err_result {
    if ( static_cast<basic_result<_mutability, T, E> const *>(this)->is_ok() ) {
      return indirect_err_result{in_place_ok, boost::get<success<T>>(static_cast<basic_result<_mutability, T, E>*>(this)->storage_).x};
    }
    else {
      return indirect_err_result{in_place_err, *boost::get<failure<E>>(static_cast<basic_result<_mutability, T, E>*>(this)->storage_).x};
    }
  }
  constexpr void indirect() & = delete;



  constexpr void indirect_ok() const & = delete;
  constexpr auto indirect_err() const & -> const_indirect_err_result {
    if ( static_cast<basic_result<_mutability, T, E> const *>(this)->is_ok() ) {
      return const_indirect_err_result{in_place_ok, boost::get<success<T>>(static_cast<basic_result<_mutability, T, E>*>(this)->storage_).x};
    }
    else {
      return const_indirect_err_result{in_place_err, *boost::get<failure<E>>(static_cast<basic_result<_mutability, T, E>*>(this)->storage_).x};
    }
  }
  constexpr void indirect() const & = delete;



  constexpr void indirect_ok() && = delete;
  constexpr auto indirect_err() && -> dangling_indirect_err_result {
    if ( static_cast<basic_result<_mutability, T, E> const *>(this)->is_ok() ) {
      return dangling_indirect_err_result{in_place_ok, boost::get<success<T>>(static_cast<basic_result<_mutability, T, E>*>(this)->storage_).x};
    }
    else {
      return dangling_indirect_err_result{in_place_err, std::ref(*boost::get<failure<E>>(static_cast<basic_result<_mutability, T, E>*>(this)->storage_).x)};
    }
  }
  constexpr void indirect() && = delete;

  constexpr void indirect_ok() const && = delete;
  constexpr void indirect_err() const && = delete;
  constexpr void indirect() const && = delete;

};


} // !namespace mitama
#endif
