//@HEADER
// ************************************************************************
//
//                        Kokkos v. 4.0
//       Copyright (2022) National Technology & Engineering
//               Solutions of Sandia, LLC (NTESS).
//
// Under the terms of Contract DE-NA0003525 with NTESS,
// the U.S. Government retains certain rights in this software.
//
// Part of Kokkos, under the Apache License v2.0 with LLVM Exceptions.
// See https://kokkos.org/LICENSE for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//@HEADER

#ifndef KOKKOSBLAS1_UPDATE_HPP_
#define KOKKOSBLAS1_UPDATE_HPP_

#include <KokkosBlas1_update_spec.hpp>
#include <KokkosKernels_helpers.hpp>
#include <KokkosKernels_Error.hpp>

namespace KokkosBlas {

/// \brief Compute Z := alpha*X + beta*Y + gamma*Z.
///
/// \tparam XMV 1-D or 2-D Kokkos::View specialization.
/// \tparam YMV 1-D or 2-D Kokkos::View specialization.  It must have
///   the same rank as XMV.
/// \tparam ZMV 1-D or 2-D Kokkos::View specialization.  It must have
///   the same rank as XMV and YMV, and it must make sense to add up
///   the entries of XMV and YMV and assign them to the entries of
///   ZMV.
template <class XMV, class YMV, class ZMV>
void update(const typename XMV::non_const_value_type& alpha, const XMV& X,
            const typename YMV::non_const_value_type& beta, const YMV& Y,
            const typename ZMV::non_const_value_type& gamma, const ZMV& Z) {
  static_assert(Kokkos::is_view<XMV>::value,
                "KokkosBlas::update: "
                "X is not a Kokkos::View.");
  static_assert(Kokkos::is_view<YMV>::value,
                "KokkosBlas::update: "
                "Y is not a Kokkos::View.");
  static_assert(Kokkos::is_view<ZMV>::value,
                "KokkosBlas::update: "
                "Z is not a Kokkos::View.");
  static_assert(std::is_same<typename ZMV::value_type,
                             typename ZMV::non_const_value_type>::value,
                "KokkosBlas::update: Z is const.  "
                "It must be nonconst, because it is an output argument "
                "(we have to be able to write to its entries).");
  static_assert(int(ZMV::rank) == int(XMV::rank),
                "KokkosBlas::update: "
                "X and Z must have the same rank.");
  static_assert(int(ZMV::rank) == int(YMV::rank),
                "KokkosBlas::update: "
                "Y and Z must have the same rank.");
  static_assert(ZMV::rank == 1 || ZMV::rank == 2,
                "KokkosBlas::update: "
                "XMV, YMV, and ZMV must either have rank 1 or rank 2.");

  // Check compatibility of dimensions at run time.
  if (X.extent(0) != Y.extent(0) || X.extent(1) != Y.extent(1) ||
      X.extent(0) != Z.extent(0) || X.extent(1) != Z.extent(1)) {
    std::ostringstream os;
    os << "KokkosBlas::update (MV): Dimensions of X, Y, and Z do not match: "
       << "Z: " << Z.extent(0) << " x " << Z.extent(1) << ", X: " << X.extent(0)
       << " x " << X.extent(1) << ", Y: " << Y.extent(0) << " x "
       << Y.extent(1);
    KokkosKernels::Impl::throw_runtime_exception(os.str());
  }

  // Create unmanaged versions of the input Views.  XMV, YMV, and ZMV
  // may be rank 1 or rank 2, but they must all have the same rank.

  typedef Kokkos::View<
      typename std::conditional<XMV::rank == 1, typename XMV::const_value_type*,
                                typename XMV::const_value_type**>::type,
      typename KokkosKernels::Impl::GetUnifiedLayout<XMV>::array_layout,
      typename XMV::device_type, Kokkos::MemoryTraits<Kokkos::Unmanaged> >
      XMV_Internal;

  typedef Kokkos::View<
      typename std::conditional<YMV::rank == 1, typename YMV::const_value_type*,
                                typename YMV::const_value_type**>::type,
      typename KokkosKernels::Impl::GetUnifiedLayout<YMV>::array_layout,
      typename YMV::device_type, Kokkos::MemoryTraits<Kokkos::Unmanaged> >
      YMV_Internal;

  typedef Kokkos::View<
      typename std::conditional<ZMV::rank == 1,
                                typename ZMV::non_const_value_type*,
                                typename ZMV::non_const_value_type**>::type,
      typename KokkosKernels::Impl::GetUnifiedLayout<ZMV>::array_layout,
      typename ZMV::device_type, Kokkos::MemoryTraits<Kokkos::Unmanaged> >
      ZMV_Internal;

  XMV_Internal X_internal = X;
  YMV_Internal Y_internal = Y;
  ZMV_Internal Z_internal = Z;

#ifdef KOKKOSKERNELS_PRINT_DEMANGLED_TYPE_INFO
  using std::cerr;
  using std::endl;
  cerr << "KokkosBlas::update:" << endl
       << "  XMV_Internal: " << demangledTypeName(X_internal) << endl
       << "  YMV_Internal: " << demangledTypeName(Y_internal) << endl
       << "  ZMV_Internal: " << demangledTypeName(Z_internal) << endl
       << endl;
#endif  // KOKKOSKERNELS_PRINT_DEMANGLED_TYPE_INFO

  return Impl::Update<XMV_Internal, YMV_Internal, ZMV_Internal>::update(
      alpha, X_internal, beta, Y_internal, gamma, Z_internal);
}

}  // namespace KokkosBlas

#endif  // KOKKOSBLAS1_UPDATE_HPP_
