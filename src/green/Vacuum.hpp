/* pcmsolver_copyright_start */
/*
 *     PCMSolver, an API for the Polarizable Continuum Model
 *     Copyright (C) 2013 Roberto Di Remigio, Luca Frediani and contributors
 *
 *     This file is part of PCMSolver.
 *
 *     PCMSolver is free software: you can redistribute it and/or modify
 *     it under the terms of the GNU Lesser General Public License as published by
 *     the Free Software Foundation, either version 3 of the License, or
 *     (at your option) any later version.
 *
 *     PCMSolver is distributed in the hope that it will be useful,
 *     but WITHOUT ANY WARRANTY; without even the implied warranty of
 *     MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *     GNU Lesser General Public License for more details.
 *
 *     You should have received a copy of the GNU Lesser General Public License
 *     along with PCMSolver.  If not, see <http://www.gnu.org/licenses/>.
 *
 *     For information on the complete list of contributors to the
 *     PCMSolver API, see: <http://pcmsolver.github.io/pcmsolver-doc>
 */
/* pcmsolver_copyright_end */

#ifndef VACUUM_HPP
#define VACUUM_HPP

#include <cmath>
#include <iosfwd>
#include <string>

#include "Config.hpp"

#include <Eigen/Dense>
#include "taylor.hpp"

class Element;

#include "DerivativeTypes.hpp"
#include "ForIdGreen.hpp"
#include "GreenData.hpp"
#include "GreensFunction.hpp"
#include "GreensFunctionFactory.hpp"

/*! \file Vacuum.hpp
 *  \class Vacuum
 *  \brief Green's function for vacuum.
 *  \author Luca Frediani and Roberto Di Remigio
 *  \date 2012-2014
 *  \tparam DerivativeTraits evaluation strategy for the function and its derivatives
 *  \tparam IntegratorPolicy policy for the calculation of diagonal elements
 */

template <typename DerivativeTraits,
          typename IntegratorPolicy>
class Vacuum : public GreensFunction<DerivativeTraits, IntegratorPolicy, Uniform,
                                     Vacuum<DerivativeTraits, IntegratorPolicy> >
{
public:
    Vacuum() : GreensFunction<DerivativeTraits, IntegratorPolicy, Uniform,
                              Vacuum<DerivativeTraits, IntegratorPolicy> >()
    {
        this->profile_ = Uniform(1.0);
    }
    virtual ~Vacuum() {}
    /*! Returns value of the kernel of the \f$\mathcal{S}\f$ integral operator, i.e. the value of the
     *  Greens's function for the pair of points p1, p2: \f$ G(\mathbf{p}_1, \mathbf{p}_2)\f$
     *  \param[in] p1 first point
     *  \param[in] p2 second point
     */
    virtual double function(const Eigen::Vector3d & p1, const Eigen::Vector3d & p2) const
    {
        DerivativeTraits sp[3], pp[3], res;
        sp[0] = p1(0); sp[1] = p1(1); sp[2] = p1(2);
        pp[0] = p2(0); pp[1] = p2(1); pp[2] = p2(2);
        res = this->operator()(sp, pp);
        return res[0];
    }
    /*! Returns value of the kernel for the calculation of the \f$\mathcal{D}\f$ integral operator
     *  for the pair of points p1, p2:
     *  \f$ [\boldsymbol{\varepsilon}\nabla_{\mathbf{p_2}}G(\mathbf{p}_1, \mathbf{p}_2)]\cdot \mathbf{n}_{\mathbf{p}_2}\f$
     *  To obtain the kernel of the \f$\mathcal{D}^\dagger\f$ operator call this methods with \f$\mathbf{p}_1\f$
     *  and \f$\mathbf{p}_2\f$ exchanged and with \f$\mathbf{n}_{\mathbf{p}_2} = \mathbf{n}_{\mathbf{p}_1}\f$
     *  \param[in] direction the direction
     *  \param[in]        p1 first point
     *  \param[in]        p2 second point
     */
    virtual double derivative(const Eigen::Vector3d & direction,
                              const Eigen::Vector3d & p1, const Eigen::Vector3d & p2) const
    {
        return this->derivativeProbe(direction, p1, p2);
    }
    /*! Returns value of the directional derivative of the
     *  Greens's function for the pair of points p1, p2:
     *  \f$ \nabla_{\mathbf{p_1}}G(\mathbf{p}_1, \mathbf{p}_2)\cdot \mathbf{n}_{\mathbf{p}_1}\f$
     *  Notice that this method returns the directional derivative with respect
     *  to the source point.
     *  \param[in] normal_p1 the normal vector to p1
     *  \param[in]        p1 first point
     *  \param[in]        p2 second point
     */
    virtual double derivativeSource(const Eigen::Vector3d & normal_p1,
                            const Eigen::Vector3d & p1, const Eigen::Vector3d & p2) const
    {
        DerivativeTraits t1[3], t2[3], der;
        t1[0] = p1(0); t1[1] = p1(1); t1[2] = p1(2);
        t1[0][1] = normal_p1(0); t1[1][1] = normal_p1(1); t1[2][1] = normal_p1(2);
        t2[0] = p2(0); t2[1] = p2(1); t2[2] = p2(2);
        der = this->operator()(t1, t2);
        return der[1];
    }
    /*! Returns value of the directional derivative of the
     *  Greens's function for the pair of points p1, p2:
     *  \f$ \nabla_{\mathbf{p_2}}G(\mathbf{p}_1, \mathbf{p}_2)\cdot \mathbf{n}_{\mathbf{p}_2}\f$
     *  Notice that this method returns the directional derivative with respect
     *  to the probe point.
     *  \param[in] normal_p2 the normal vector to p2
     *  \param[in]        p1 first point
     *  \param[in]        p2 second point
     */
    virtual double derivativeProbe(const Eigen::Vector3d & normal_p2,
                                   const Eigen::Vector3d & p1, const Eigen::Vector3d & p2) const
    {
        DerivativeTraits t1[3], t2[3], der;
        t1[0] = p1(0); t1[1] = p1(1); t1[2] = p1(2);
        t2[0] = p2(0); t2[1] = p2(1); t2[2] = p2(2);
        t2[0][1] = normal_p2(0); t2[1][1] = normal_p2(1); t2[2][1] = normal_p2(2);
        der = this->operator()(t1, t2);
        return der[1];
    }

    /*! Calculates the diagonal elements of the S operator: \f$ S_{ii} \f$
     *  \param[in] e i-th finite element
     */
    virtual double diagonalS(const Element & e) const
    {
            return this->diagonal_.computeS(*this, e);
    }
    /*! Calculates the diagonal elements of the D operator: \f$ D_{ii} \f$
     *  \param[in] e i-th finite element
     */
    virtual double diagonalD(const Element & e) const
    {
            return this->diagonal_.computeD(*this, e);
    }

    friend std::ostream & operator<<(std::ostream & os, Vacuum & gf) {
        return gf.printObject(os);
    }
private:
    /*! Evaluates the Green's function given a pair of points
     *  \param[in] sp the source point
     *  \param[in] pp the probe point
     */
    virtual DerivativeTraits operator()(DerivativeTraits * sp, DerivativeTraits * pp) const
    {
        DerivativeTraits res;
        res = 1.0/sqrt((sp[0]-pp[0])*(sp[0]-pp[0])+
                       (sp[1]-pp[1])*(sp[1]-pp[1])+
                       (sp[2]-pp[2])*(sp[2]-pp[2]));
        return res;
    }
    virtual std::ostream & printObject(std::ostream & os)
    {
        os << "Green's function type: vacuum";
        return os;
    }
};

/*! \file Vacuum.hpp
 *  \class Vacuum
 *  \brief Green's function for vacuum.
 *  \author Luca Frediani and Roberto Di Remigio
 *  \date 2012-2014
 *  \tparam IntegratorPolicy policy for the calculation of diagonal elements
 *
 *  Explicit specialization
 */

template <typename IntegratorPolicy>
class Vacuum<Numerical, IntegratorPolicy> : public GreensFunction<Numerical, IntegratorPolicy, Uniform,
                                     Vacuum<Numerical, IntegratorPolicy> >
{
public:
    Vacuum() : GreensFunction<Numerical, IntegratorPolicy, Uniform,
                              Vacuum<Numerical, IntegratorPolicy> >()
    {
        this->profile_ = Uniform(1.0);
    }
    virtual ~Vacuum() {}
    /*! Returns value of the kernel of the \f$\mathcal{S}\f$ integral operator, i.e. the value of the
     *  Greens's function for the pair of points p1, p2: \f$ G(\mathbf{p}_1, \mathbf{p}_2)\f$
     *  \param[in] p1 first point
     *  \param[in] p2 second point
     */
    virtual double function(const Eigen::Vector3d & p1, const Eigen::Vector3d & p2) const
    {
        Numerical sp[3], pp[3], res;
        sp[0] = p1(0); sp[1] = p1(1); sp[2] = p1(2);
        pp[0] = p2(0); pp[1] = p2(1); pp[2] = p2(2);
        res = this->operator()(sp, pp);
        return res;
    }
    /*! Returns value of the kernel for the calculation of the \f$\mathcal{D}\f$ integral operator
     *  for the pair of points p1, p2:
     *  \f$ [\boldsymbol{\varepsilon}\nabla_{\mathbf{p_2}}G(\mathbf{p}_1, \mathbf{p}_2)]\cdot \mathbf{n}_{\mathbf{p}_2}\f$
     *  To obtain the kernel of the \f$\mathcal{D}^\dagger\f$ operator call this methods with \f$\mathbf{p}_1\f$
     *  and \f$\mathbf{p}_2\f$ exchanged and with \f$\mathbf{n}_{\mathbf{p}_2} = \mathbf{n}_{\mathbf{p}_1}\f$
     *  \param[in] direction the direction
     *  \param[in]        p1 first point
     *  \param[in]        p2 second point
     */
    virtual double derivative(const Eigen::Vector3d & direction,
                              const Eigen::Vector3d & p1, const Eigen::Vector3d & p2) const
    {
        return this->derivativeProbe(direction, p1, p2);
    }
    /*! Returns value of the directional derivative of the
     *  Greens's function for the pair of points p1, p2:
     *  \f$ \nabla_{\mathbf{p_1}}G(\mathbf{p}_1, \mathbf{p}_2)\cdot \mathbf{n}_{\mathbf{p}_1}\f$
     *  Notice that this method returns the directional derivative with respect
     *  to the source point.
     *  \param[in] normal_p1 the normal vector to p1
     *  \param[in]        p1 first point
     *  \param[in]        p2 second point
     */
    virtual double derivativeSource(const Eigen::Vector3d & normal_p1,
                            const Eigen::Vector3d & p1, const Eigen::Vector3d & p2) const
    {
        using namespace std::placeholders;
        return threePointStencil(std::bind(&Vacuum<Numerical, IntegratorPolicy>::function, this, _1, _2),
                                p1, p2, normal_p1, this->delta_);
    }
    /*! Returns value of the directional derivative of the
     *  Greens's function for the pair of points p1, p2:
     *  \f$ \nabla_{\mathbf{p_2}}G(\mathbf{p}_1, \mathbf{p}_2)\cdot \mathbf{n}_{\mathbf{p}_2}\f$
     *  Notice that this method returns the directional derivative with respect
     *  to the probe point.
     *  \param[in] normal_p2 the normal vector to p2
     *  \param[in]        p1 first point
     *  \param[in]        p2 second point
     */
    virtual double derivativeProbe(const Eigen::Vector3d & normal_p2,
                                   const Eigen::Vector3d & p1, const Eigen::Vector3d & p2) const
    {
        using namespace std::placeholders;
        return threePointStencil(std::bind(&Vacuum<Numerical, IntegratorPolicy>::function, this, _1, _2),
                                p2, p1, normal_p2, this->delta_);
    }

    /*! Calculates the diagonal elements of the S operator: \f$ S_{ii} \f$
     *  \param[in] e i-th finite element
     */
    virtual double diagonalS(const Element & e) const
    {
            return this->diagonal_.computeS(*this, e);
    }
    /*! Calculates the diagonal elements of the D operator: \f$ D_{ii} \f$
     *  \param[in] e i-th finite element
     */
    virtual double diagonalD(const Element & e) const
    {
            return this->diagonal_.computeD(*this, e);
    }

    friend std::ostream & operator<<(std::ostream & os, Vacuum & gf) {
        return gf.printObject(os);
    }
private:
    /*! Evaluates the Green's function given a pair of points
     *  \param[in] sp the source point
     *  \param[in] pp the probe point
     */
    virtual Numerical operator()(Numerical * sp, Numerical * pp) const
    {
        Numerical res;
        res = 1.0/sqrt((sp[0]-pp[0])*(sp[0]-pp[0])+
                       (sp[1]-pp[1])*(sp[1]-pp[1])+
                       (sp[2]-pp[2])*(sp[2]-pp[2]));
        return res;
    }
    virtual std::ostream & printObject(std::ostream & os)
    {
        os << "Green's function type: vacuum";
        return os;
    }
};

/*
namespace
{
    struct buildVacuum {
        template <typename DerivativeType>
        IGreensFunction * operator()(const greenData & _data) {
            DiagonalIntegrator * integrator =
		    DiagonalIntegratorFactory::TheDiagonalIntegratorFactory().createDiagonalIntegrator(_data.integratorType);
            return new Vacuum<DerivativeType>(integrator);
        }
    };

    IGreensFunction * createVacuum(const greenData & _data)
    {
        buildVacuum build;
        return for_id<derivative_types>(build, _data, _data.how);
    }
    const std::string VACUUM("VACUUM");
    const bool registeredVacuum =
        GreensFunctionFactory::TheGreensFunctionFactory().registerGreensFunction(
            VACUUM, createVacuum);
}
*/

#endif // VACUUM_HPP
