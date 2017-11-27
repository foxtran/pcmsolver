/*
 * PCMSolver, an API for the Polarizable Continuum Model
 * Copyright (C) 2017 Roberto Di Remigio, Luca Frediani and collaborators.
 *
 * This file is part of PCMSolver.
 *
 * PCMSolver is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * PCMSolver is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with PCMSolver.  If not, see <http://www.gnu.org/licenses/>.
 *
 * For information on the complete list of contributors to the
 * PCMSolver API, see: <http://pcmsolver.readthedocs.io/>
 */

#pragma once

#include <iosfwd>

#include "Config.hpp"

#include <Eigen/Core>

/*! \file SphericalDiffuse.hpp */

namespace pcm {
namespace cavity {
class Element;
} // namespace cavity

namespace dielectric_profile {
class OneLayerTanh;
class MembraneTanh;
} // namespace dielectric_profile
} // namespace pcm

#include "GreenData.hpp"
#include "GreensFunction.hpp"
#include "InterfacesImpl.hpp"

namespace pcm {
namespace green {
/*! \class SphericalDiffuse
 *  \brief Green's function for a diffuse interface with spherical symmetry
 *  \author Hui Cao, Ville Weijo, Luca Frediani and Roberto Di Remigio
 *  \date 2010-2015
 *  \tparam ProfilePolicy functional form of the diffuse layer
 *
 *  The origin of the dielectric sphere can be changed by means of the constructor.
 *  The solution of the differential equation defining the Green's function is
 *  **always** performed assuming that the dielectric sphere is centered in the
 * origin of the
 *  coordinate system. Whenever the public methods are invoked to "sample" the
 * Green's function
 *  at a pair of points, a translation of the sampling points is performed first.
 */
template <typename ProfilePolicy = dielectric_profile::OneLayerTanh>
class SphericalDiffuse __final : public GreensFunction<Stencil, ProfilePolicy> {
public:
  /*! Constructor for a one-layer interface
   * \param[in] e1 left-side dielectric constant
   * \param[in] e2 right-side dielectric constant
   * \param[in] w width of the interface layer
   * \param[in] c center of the diffuse layer
   * \param[in] o center of the sphere
   * \param[in] l maximum value of angular momentum
   * \note For SFINAE to work correctly, this constructor **must be** in the header
   * file.
   */
  template <typename U = ProfilePolicy,
            typename = pcm::IsNotSame<U, dielectric_profile::MembraneTanh> >
  SphericalDiffuse(double e1,
                   double e2,
                   double w,
                   double c,
                   const Eigen::Vector3d & o,
                   int l)
      : GreensFunction<Stencil, ProfilePolicy>(ProfilePolicy(e1, e2, w, c)),
        origin_(o),
        maxLGreen_(l),
        maxLC_(2 * l) {
    initSphericalDiffuse();
  }
  /*! Constructor for a two-layer interface (membrane)
   * \param[in] e1 left-side dielectric constant
   * \param[in] e2 middle portion dielectric constant
   * \param[in] e3 right-side dielectric constant
   * \param[in] w12 width of the first interface layer
   * \param[in] w23 width of the second interface layer
   * \param[in] c12 center of the first diffuse layer
   * \param[in] c23 center of the second diffuse layer
   * \param[in] o center of the sphere
   * \param[in] l maximum value of angular momentum
   * \note For SFINAE to work correctly, this constructor **must be** in the header
   * file.
   */
  template <typename U = ProfilePolicy,
            typename = pcm::IsSame<U, dielectric_profile::MembraneTanh> >
  SphericalDiffuse(double e1,
                   double e2,
                   double e3,
                   double w12,
                   double w23,
                   double c12,
                   double c23,
                   const Eigen::Vector3d & o,
                   int l)
      : GreensFunction<Stencil, ProfilePolicy>(
            ProfilePolicy(e1, e2, e3, w12, w23, c12, c23)),
        origin_(o),
        maxLGreen_(l),
        maxLC_(2 * l) {
    initSphericalDiffuse();
  }
  virtual ~SphericalDiffuse() {}

  friend std::ostream & operator<<(std::ostream & os, SphericalDiffuse & gf) {
    return gf.printObject(os);
  }
  /*! \brief Returns Coulomb singularity separation coefficient
   *  \param[in] source location of the source charge
   *  \param[in] probe location of the probe charge
   */
  double coefficientCoulomb(const Eigen::Vector3d & source,
                            const Eigen::Vector3d & probe) const;
  /*! \brief Returns singular part of the Green's function
   *  \param[in] source location of the source charge
   *  \param[in] probe location of the probe charge
   */
  double Coulomb(const Eigen::Vector3d & source,
                 const Eigen::Vector3d & probe) const;
  /*! \brief Returns non-singular part of the Green's function (image potential)
   *  \param[in] source location of the source charge
   *  \param[in] probe location of the probe charge
   */
  double imagePotential(const Eigen::Vector3d & source,
                        const Eigen::Vector3d & probe) const;
  /*! Returns value of the directional derivative of the
   *  Coulomb singularity separation coefficient for the pair of points p1, p2:
   *  \f$ \nabla_{\mathbf{p_2}}G(\mathbf{p}_1, \mathbf{p}_2)\cdot
   *\mathbf{n}_{\mathbf{p}_2}\f$
   *  Notice that this method returns the directional derivative with respect
   *  to the probe point, thus assuming that the direction is relative to that
   *point.
   *
   *  \param[in] direction the direction
   *  \param[in]        p1 first point
   *  \param[in]        p2 second point
   */
  double coefficientCoulombDerivative(const Eigen::Vector3d & direction,
                                      const Eigen::Vector3d & p1,
                                      const Eigen::Vector3d & p2) const;
  /*! Returns value of the directional derivative of the
   *  singular part of the Greens's function for the pair of points p1, p2:
   *  \f$ \nabla_{\mathbf{p_2}}G(\mathbf{p}_1, \mathbf{p}_2)\cdot
   *\mathbf{n}_{\mathbf{p}_2}\f$
   *  Notice that this method returns the directional derivative with respect
   *  to the probe point, thus assuming that the direction is relative to that
   *point.
   *
   *  \param[in] direction the direction
   *  \param[in]        p1 first point
   *  \param[in]        p2 second point
   */
  double CoulombDerivative(const Eigen::Vector3d & direction,
                           const Eigen::Vector3d & p1,
                           const Eigen::Vector3d & p2) const;
  /*! Returns value of the directional derivative of the
   *  non-singular part (image potential) of the Greens's function for the pair of
   *points p1, p2:
   *  \f$ \nabla_{\mathbf{p_2}}G(\mathbf{p}_1, \mathbf{p}_2)\cdot
   *\mathbf{n}_{\mathbf{p}_2}\f$
   *  Notice that this method returns the directional derivative with respect
   *  to the probe point, thus assuming that the direction is relative to that
   *point.
   *
   *  \param[in] direction the direction
   *  \param[in]        p1 first point
   *  \param[in]        p2 second point
   */
  double imagePotentialDerivative(const Eigen::Vector3d & direction,
                                  const Eigen::Vector3d & p1,
                                  const Eigen::Vector3d & p2) const;
  /*! Handle to the dielectric profile evaluation */
  pcm::tuple<double, double> epsilon(const Eigen::Vector3d & point) const;
  void toFile(const std::string & prefix = "");
  EIGEN_MAKE_ALIGNED_OPERATOR_NEW
private:
  /*! Evaluates the Green's function given a pair of points
   *  \param[in] sp the source point
   *  \param[in] pp the probe point
   *
   *  \note This takes care of the origin shift
   */
  virtual Stencil operator()(Stencil * sp, Stencil * pp) const __override;
  /*! Returns value of the kernel of the \f$\mathcal{D}\f$ integral operator for
   * the
   * pair of points p1, p2:
   *  \f$ [\boldsymbol{\varepsilon}\nabla_{\mathbf{p_2}}G(\mathbf{p}_1,
   * \mathbf{p}_2)]\cdot \mathbf{n}_{\mathbf{p}_2}\f$
   *  To obtain the kernel of the \f$\mathcal{D}^\dagger\f$ operator call this
   * methods with \f$\mathbf{p}_1\f$
   *  and \f$\mathbf{p}_2\f$ exchanged and with \f$\mathbf{n}_{\mathbf{p}_2} =
   * \mathbf{n}_{\mathbf{p}_1}\f$
   *  \param[in] direction the direction
   *  \param[in]        p1 first point
   *  \param[in]        p2 second point
   */
  virtual double kernelD_impl(const Eigen::Vector3d & direction,
                              const Eigen::Vector3d & p1,
                              const Eigen::Vector3d & p2) const __override;

  virtual KernelS exportKernelS_impl() const __override;
  virtual KernelD exportKernelD_impl() const __override;
  virtual DerivativeProbe exportDerivativeProbe_impl() const __override;

  virtual double singleLayer_impl(const Element & e, double factor) const __override;
  virtual double doubleLayer_impl(const Element & e, double factor) const __override;

  virtual std::ostream & printObject(std::ostream & os) __override;
  /*! This calculates all the components needed to evaluate the Green's function */
  void initSphericalDiffuse();

  /*! Center of the dielectric sphere */
  Eigen::Vector3d origin_;

  /*! @{ Parameters and functions for the calculation of the Green's function,
   * including Coulomb singularity */
  /*! Maximum angular momentum in the __final summation over Legendre polynomials
   * to
   * obtain G */
  int maxLGreen_;
  /*! \brief First independent radial solution, used to build Green's function.
   *  \note The vector has dimension maxLGreen_ and has r^l behavior
   */
  std::vector<RadialFunction<detail::StateType, detail::LnTransformedRadial, Zeta> >
      zeta_;
  /*! \brief Second independent radial solution, used to build Green's function.
   *  \note The vector has dimension maxLGreen_  and has r^(-l-1) behavior
   */
  std::vector<RadialFunction<detail::StateType, detail::LnTransformedRadial, Omega> >
      omega_;
  /*! \brief Returns L-th component of the radial part of the Green's function
   *  \param[in] L  angular momentum
   *  \param[in] sp source point
   *  \param[in] pp probe point
   *  \param[in] Cr12 Coulomb singularity separation coefficient
   *  \note This function shifts the given source and probe points by the location
   * of
   * the
   *  dielectric sphere.
   */
  double imagePotentialComponent_impl(int L,
                                      const Eigen::Vector3d & sp,
                                      const Eigen::Vector3d & pp,
                                      double Cr12) const;
  /*! @}*/

  /*! @{ Parameters and functions for the calculation of the Coulomb singularity
   * separation coefficient */
  /*! Maximum angular momentum to obtain C(r, r'), needed to separate the Coulomb
   * singularity */
  int maxLC_; // = 2 * maxLGreen_;
  /*! \brief First independent radial solution, used to build coefficient.
   *  \note This is needed to separate the Coulomb singularity and has r^l behavior
   */
  RadialFunction<detail::StateType, detail::LnTransformedRadial, Zeta> zetaC_;
  /*! \brief Second independent radial solution, used to build coefficient.
   *  \note This is needed to separate the Coulomb singularity and has r^(-l-1)
   * behavior
   */
  RadialFunction<detail::StateType, detail::LnTransformedRadial, Omega> omegaC_;
  /*! \brief Returns coefficient for the separation of the Coulomb singularity
   *  \param[in] sp first point
   *  \param[in] pp second point
   *  \note This function shifts the given source and probe points by the location
   * of
   * the
   *  dielectric sphere.
   */
  double coefficient_impl(const Eigen::Vector3d & sp,
                          const Eigen::Vector3d & pp) const;
  /*! @}*/
};

template <typename ProfilePolicy>
IGreensFunction * createSphericalDiffuse(const GreenData & data) {
  return new SphericalDiffuse<ProfilePolicy>(
      data.epsilon1, data.epsilon2, data.width, data.center, data.origin, data.maxL);
}
} // namespace green
} // namespace pcm
