// This file is part of Eigen, a lightweight C++ template library
// for linear algebra.
//
// Copyright (C) 2009-2010 Gael Guennebaud <gael.guennebaud@inria.fr>
//
// Eigen is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public
// License as published by the Free Software Foundation; either
// version 3 of the License, or (at your option) any later version.
//
// Alternatively, you can redistribute it and/or
// modify it under the terms of the GNU General Public License as
// published by the Free Software Foundation; either version 2 of
// the License, or (at your option) any later version.
//
// Eigen is distributed in the hope that it will be useful, but WITHOUT ANY
// WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
// FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License or the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public
// License and a copy of the GNU General Public License along with
// Eigen. If not, see <http://www.gnu.org/licenses/>.

#ifndef EIGEN_BLASUTIL_H
#define EIGEN_BLASUTIL_H

// This file contains many lightweight helper classes used to
// implement and control fast level 2 and level 3 BLAS-like routines.

// forward declarations
template<typename Scalar, typename Index, int mr, int nr, typename Conj>
struct ei_gebp_kernel;

template<typename Scalar, typename Index, int nr, int StorageOrder, bool PanelMode=false>
struct ei_gemm_pack_rhs;

template<typename Scalar, typename Index, int mr, int StorageOrder, bool Conjugate = false, bool PanelMode = false>
struct ei_gemm_pack_lhs;

template<
  typename Scalar, typename Index,
  int LhsStorageOrder, bool ConjugateLhs,
  int RhsStorageOrder, bool ConjugateRhs,
  int ResStorageOrder>
struct ei_general_matrix_matrix_product;

template<bool ConjugateLhs, bool ConjugateRhs, typename Scalar, typename Index, typename RhsType>
static void ei_cache_friendly_product_colmajor_times_vector(
  Index size, const Scalar* lhs, Index lhsStride, const RhsType& rhs, Scalar* res, Scalar alpha);

template<bool ConjugateLhs, bool ConjugateRhs, typename Scalar, typename Index, typename ResType>
static void ei_cache_friendly_product_rowmajor_times_vector(
  const Scalar* lhs, Index lhsStride, const Scalar* rhs, Index rhsSize, ResType& res, Scalar alpha);

// Provides scalar/packet-wise product and product with accumulation
// with optional conjugation of the arguments.
template<bool ConjLhs, bool ConjRhs> struct ei_conj_helper;

template<> struct ei_conj_helper<false,false>
{
  template<typename T>
  EIGEN_STRONG_INLINE T pmadd(const T& x, const T& y, const T& c) const { return  ei_pmadd(x,y,c); }
  template<typename T>
  EIGEN_STRONG_INLINE T pmul(const T& x, const T& y) const { return  ei_pmul(x,y); }
};

template<> struct ei_conj_helper<false,true>
{
  template<typename T> std::complex<T>
  pmadd(const std::complex<T>& x, const std::complex<T>& y, const std::complex<T>& c) const
  { return c + pmul(x,y); }

  template<typename T> std::complex<T> pmul(const std::complex<T>& x, const std::complex<T>& y) const
  { return std::complex<T>(ei_real(x)*ei_real(y) + ei_imag(x)*ei_imag(y), ei_imag(x)*ei_real(y) - ei_real(x)*ei_imag(y)); }
};

template<> struct ei_conj_helper<true,false>
{
  template<typename T> std::complex<T>
  pmadd(const std::complex<T>& x, const std::complex<T>& y, const std::complex<T>& c) const
  { return c + pmul(x,y); }

  template<typename T> std::complex<T> pmul(const std::complex<T>& x, const std::complex<T>& y) const
  { return std::complex<T>(ei_real(x)*ei_real(y) + ei_imag(x)*ei_imag(y), ei_real(x)*ei_imag(y) - ei_imag(x)*ei_real(y)); }
};

template<> struct ei_conj_helper<true,true>
{
  template<typename T> std::complex<T>
  pmadd(const std::complex<T>& x, const std::complex<T>& y, const std::complex<T>& c) const
  { return c + pmul(x,y); }

  template<typename T> std::complex<T> pmul(const std::complex<T>& x, const std::complex<T>& y) const
  { return std::complex<T>(ei_real(x)*ei_real(y) - ei_imag(x)*ei_imag(y), - ei_real(x)*ei_imag(y) - ei_imag(x)*ei_real(y)); }
};

// Lightweight helper class to access matrix coefficients.
// Yes, this is somehow redundant with Map<>, but this version is much much lighter,
// and so I hope better compilation performance (time and code quality).
template<typename Scalar, typename Index, int StorageOrder>
class ei_blas_data_mapper
{
  public:
    ei_blas_data_mapper(Scalar* data, Index stride) : m_data(data), m_stride(stride) {}
    EIGEN_STRONG_INLINE Scalar& operator()(Index i, Index j)
    { return m_data[StorageOrder==RowMajor ? j + i*m_stride : i + j*m_stride]; }
  protected:
    Scalar* EIGEN_RESTRICT m_data;
    Index m_stride;
};

// lightweight helper class to access matrix coefficients (const version)
template<typename Scalar, typename Index, int StorageOrder>
class ei_const_blas_data_mapper
{
  public:
    ei_const_blas_data_mapper(const Scalar* data, Index stride) : m_data(data), m_stride(stride) {}
    EIGEN_STRONG_INLINE const Scalar& operator()(Index i, Index j) const
    { return m_data[StorageOrder==RowMajor ? j + i*m_stride : i + j*m_stride]; }
  protected:
    const Scalar* EIGEN_RESTRICT m_data;
    Index m_stride;
};

// Defines various constant controlling register blocking for matrix-matrix algorithms.
template<typename Scalar>
struct ei_product_blocking_traits
{
  typedef typename ei_packet_traits<Scalar>::type PacketType;
  enum {
    PacketSize = sizeof(PacketType)/sizeof(Scalar),
    NumberOfRegisters = EIGEN_ARCH_DEFAULT_NUMBER_OF_REGISTERS,

    // register block size along the N direction (must be either 2 or 4)
    nr = NumberOfRegisters/4,

    // register block size along the M direction (currently, this one cannot be modified)
    mr = 2 * PacketSize
  };
};

/* Helper class to analyze the factors of a Product expression.
 * In particular it allows to pop out operator-, scalar multiples,
 * and conjugate */
template<typename XprType> struct ei_blas_traits
{
  typedef typename ei_traits<XprType>::Scalar Scalar;
  typedef const XprType& ExtractType;
  typedef XprType _ExtractType;
  enum {
    IsComplex = NumTraits<Scalar>::IsComplex,
    IsTransposed = false,
    NeedToConjugate = false,
    HasUsableDirectAccess = (    (int(XprType::Flags)&DirectAccessBit)
                     && (  /* Uncomment this when the low-level matrix-vector product functions support strided vectors
                           bool(XprType::IsVectorAtCompileTime)
                         ||  */
                           int(ei_inner_stride_at_compile_time<XprType>::ret) == 1)
                   ) ?  1 : 0
  };
  typedef typename ei_meta_if<bool(HasUsableDirectAccess),
    ExtractType,
    typename _ExtractType::PlainObject
    >::ret DirectLinearAccessType;
  static inline ExtractType extract(const XprType& x) { return x; }
  static inline Scalar extractScalarFactor(const XprType&) { return Scalar(1); }
};

// pop conjugate
template<typename Scalar, typename NestedXpr>
struct ei_blas_traits<CwiseUnaryOp<ei_scalar_conjugate_op<Scalar>, NestedXpr> >
 : ei_blas_traits<NestedXpr>
{
  typedef ei_blas_traits<NestedXpr> Base;
  typedef CwiseUnaryOp<ei_scalar_conjugate_op<Scalar>, NestedXpr> XprType;
  typedef typename Base::ExtractType ExtractType;

  enum {
    IsComplex = NumTraits<Scalar>::IsComplex,
    NeedToConjugate = Base::NeedToConjugate ? 0 : IsComplex
  };
  static inline ExtractType extract(const XprType& x) { return Base::extract(x.nestedExpression()); }
  static inline Scalar extractScalarFactor(const XprType& x) { return ei_conj(Base::extractScalarFactor(x.nestedExpression())); }
};

// pop scalar multiple
template<typename Scalar, typename NestedXpr>
struct ei_blas_traits<CwiseUnaryOp<ei_scalar_multiple_op<Scalar>, NestedXpr> >
 : ei_blas_traits<NestedXpr>
{
  typedef ei_blas_traits<NestedXpr> Base;
  typedef CwiseUnaryOp<ei_scalar_multiple_op<Scalar>, NestedXpr> XprType;
  typedef typename Base::ExtractType ExtractType;
  static inline ExtractType extract(const XprType& x) { return Base::extract(x.nestedExpression()); }
  static inline Scalar extractScalarFactor(const XprType& x)
  { return x.functor().m_other * Base::extractScalarFactor(x.nestedExpression()); }
};

// pop opposite
template<typename Scalar, typename NestedXpr>
struct ei_blas_traits<CwiseUnaryOp<ei_scalar_opposite_op<Scalar>, NestedXpr> >
 : ei_blas_traits<NestedXpr>
{
  typedef ei_blas_traits<NestedXpr> Base;
  typedef CwiseUnaryOp<ei_scalar_opposite_op<Scalar>, NestedXpr> XprType;
  typedef typename Base::ExtractType ExtractType;
  static inline ExtractType extract(const XprType& x) { return Base::extract(x.nestedExpression()); }
  static inline Scalar extractScalarFactor(const XprType& x)
  { return - Base::extractScalarFactor(x.nestedExpression()); }
};

// pop/push transpose
template<typename NestedXpr>
struct ei_blas_traits<Transpose<NestedXpr> >
 : ei_blas_traits<NestedXpr>
{
  typedef typename NestedXpr::Scalar Scalar;
  typedef ei_blas_traits<NestedXpr> Base;
  typedef Transpose<NestedXpr> XprType;
  typedef Transpose<typename Base::_ExtractType>  ExtractType;
  typedef Transpose<typename Base::_ExtractType> _ExtractType;
  typedef typename ei_meta_if<bool(Base::HasUsableDirectAccess),
    ExtractType,
    typename ExtractType::PlainObject
    >::ret DirectLinearAccessType;
  enum {
    IsTransposed = Base::IsTransposed ? 0 : 1
  };
  static inline const ExtractType extract(const XprType& x) { return Base::extract(x.nestedExpression()); }
  static inline Scalar extractScalarFactor(const XprType& x) { return Base::extractScalarFactor(x.nestedExpression()); }
};

template<typename T, bool HasUsableDirectAccess=ei_blas_traits<T>::HasUsableDirectAccess>
struct ei_extract_data_selector {
  static const typename T::Scalar* run(const T& m)
  {
    return &ei_blas_traits<T>::extract(m).const_cast_derived().coeffRef(0,0); // FIXME this should be .data()
  }
};

template<typename T>
struct ei_extract_data_selector<T,false> {
  static typename T::Scalar* run(const T&) { return 0; }
};

template<typename T> const typename T::Scalar* ei_extract_data(const T& m)
{
  return ei_extract_data_selector<T>::run(m);
}

#endif // EIGEN_BLASUTIL_H
