/*
 * Copyright (c) 2015, Georgia Tech Research Corporation
 * All rights reserved.
 *
 * Author(s): Michael X. Grey <mxgrey@gatech.edu>
 *
 * Georgia Tech Graphics Lab and Humanoid Robotics Lab
 *
 * Directed by Prof. C. Karen Liu and Prof. Mike Stilman
 * <karenliu@cc.gatech.edu> <mstilman@cc.gatech.edu>
 *
 * This file is provided under the following "BSD-style" License:
 *   Redistribution and use in source and binary forms, with or
 *   without modification, are permitted provided that the following
 *   conditions are met:
 *   * Redistributions of source code must retain the above copyright
 *     notice, this list of conditions and the following disclaimer.
 *   * Redistributions in binary form must reproduce the above
 *     copyright notice, this list of conditions and the following
 *     disclaimer in the documentation and/or other materials provided
 *     with the distribution.
 *   THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND
 *   CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES,
 *   INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 *   MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 *   DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR
 *   CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 *   SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 *   LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF
 *   USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED
 *   AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 *   LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
 *   ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 *   POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef DART_DYNAMICS_JACOBIANENTITY_H_
#define DART_DYNAMICS_JACOBIANENTITY_H_

#include <memory>

#include "dart/dynamics/Frame.h"

namespace dart {
namespace dynamics {

class Skeleton;
class DegreeOfFreedom;
class InverseKinematics;

/// The JacobianEntity class serves as a common interface for BodyNodes and
/// EndEffectors to both be used as references for IK modules. This is a pure
/// abstract class.
class JacobianEntity : public virtual Frame
{
public:

  /// This constructor only exists as a formality, because Entity and Frame
  /// require it. Since this class is pure abstract and virutally inherits
  /// Frame, the most derived class needs to call the constructor of Entity and
  /// Frame anyway.
  JacobianEntity();

  /// Virtual destructor
  virtual ~JacobianEntity();

  /// Get a pointer to an IK module for this Entity. If _createIfNull is true,
  /// then the IK module will be generated if one does not already exist.
  InverseKinematics* getIK(bool _createIfNull = true);

  /// Get a pointer to an IK module for this Entity. Because this is a const
  /// function, a new IK module will NOT be created, even if one does not
  /// already exist.
  const InverseKinematics* getIK() const;

  /// Create a new IK module for this Entity. If an IK module already exists in
  /// this Entity, it will be destroyed and replaced by a brand new one.
  InverseKinematics* createIK();

  /// Wipe away the IK module for this Entity, leaving it as a nullptr.
  void clearIK();

  //----------------------------------------------------------------------------
  /// \{ \name Structural Properties
  //----------------------------------------------------------------------------

  /// Return the Skeleton this JacobianEntity is attached to
  virtual std::shared_ptr<Skeleton> getSkeleton() = 0;

  /// Return the Skeleton this JacobianEntity is attached to
  virtual std::shared_ptr<const Skeleton> getSkeleton() const = 0;

  /// The number of the generalized coordinates which affect this JacobianEntity
  virtual size_t getNumDependentGenCoords() const = 0;

  /// Indices of the generalized coordinates which affect this JacobianEntity
  virtual const std::vector<size_t>& getDependentGenCoordIndices() const = 0;

  /// Same as getNumDependentGenCoords()
  virtual size_t getNumDependentDofs() const = 0;

  /// Returns a DegreeOfFreedom vector containing the dofs that form a Chain
  /// leading up to this JacobianEntity from the root of the Skeleton.
  virtual const std::vector<const DegreeOfFreedom*> getChainDofs() const = 0;

  /// \}

  //----------------------------------------------------------------------------
  /// \{ \name Jacobian Functions
  //----------------------------------------------------------------------------

  /// Return the generalized Jacobian targeting the origin of this Entity. The
  /// Jacobian is expressed in the Frame of this Entity
  virtual const math::Jacobian& getJacobian() const = 0;

  /// A version of getJacobian() that lets you specify a coordinate Frame to
  /// express the Jacobian in.
  virtual math::Jacobian getJacobian(const Frame* _inCoordinatesOf) const = 0;

  /// Return the generalized Jacobian targeting an offset within the Frame of
  /// this Entity.
  virtual math::Jacobian getJacobian(const Eigen::Vector3d& _offset) const = 0;

  /// A version of getJacobian(const Eigen::Vector3d&) that lets you specify a
  /// coordinate Frame to express the Jacobian in.
  virtual math::Jacobian getJacobian(const Eigen::Vector3d& _offset,
                                     const Frame* _inCoordinatesOf) const = 0;

  /// Return the generalized Jacobian targeting the origin of this Entity. The
  /// Jacobian is expressed in the World Frame.
  virtual const math::Jacobian& getWorldJacobian() const = 0;

  /// Return the generalized Jacobian targeting an offset in this Entity. The
  /// _offset is expected in coordinates of this BodyNode Frame. The Jacobian is
  /// expressed in the World Frame.
  virtual math::Jacobian getWorldJacobian(
      const Eigen::Vector3d& _offset) const = 0;

  /// Return the linear Jacobian targeting the origin of this BodyNode. You can
  /// specify a coordinate Frame to express the Jacobian in.
  virtual math::LinearJacobian getLinearJacobian(
      const Frame* _inCoordinatesOf = Frame::World()) const = 0;

  /// Return the generalized Jacobian targeting an offset within the Frame of
  /// this BodyNode.
  virtual math::LinearJacobian getLinearJacobian(
      const Eigen::Vector3d& _offset,
      const Frame* _inCoordinatesOf = Frame::World()) const = 0;

  /// Return the angular Jacobian targeting the origin of this BodyNode. You can
  /// specify a coordinate Frame to express the Jacobian in.
  virtual math::AngularJacobian getAngularJacobian(
      const Frame* _inCoordinatesOf = Frame::World()) const = 0;

  /// Return the spatial time derivative of the generalized Jacobian targeting
  /// the origin of this BodyNode. The Jacobian is expressed in this BodyNode's
  /// coordinate Frame.
  ///
  /// NOTE: Since this is a spatial time derivative, it should be used with
  /// spatial vectors. If you are using classical linear and angular
  /// acceleration vectors, then use getJacobianClassicDeriv(),
  /// getLinearJacobianDeriv(), or getAngularJacobianDeriv() instead.
  virtual const math::Jacobian& getJacobianSpatialDeriv() const = 0;

  /// A version of getJacobianSpatialDeriv() that can return the Jacobian in
  /// coordinates of any Frame.
  ///
  /// NOTE: This Jacobian Derivative is only for use with spatial vectors. If
  /// you are using classical linear and angular vectors, then use
  /// getJacobianClassicDeriv(), getLinearJacobianDeriv(), or
  /// getAngularJacobianDeriv() instead.
  virtual math::Jacobian getJacobianSpatialDeriv(
      const Frame* _inCoordinatesOf) const = 0;

  /// Return the spatial time derivative of the generalized Jacobian targeting
  /// an offset in the Frame of this BodyNode. The Jacobian is expressed in
  /// this BodyNode's coordinate Frame.
  ///
  /// NOTE: This Jacobian Derivative is only for use with spatial vectors. If
  /// you are using classic linear and angular vectors, then use
  /// getJacobianClassicDeriv(), getLinearJacobianDeriv(), or
  /// getAngularJacobianDeriv() instead.
  ///
  /// \sa getJacobianSpatialDeriv()
  virtual math::Jacobian getJacobianSpatialDeriv(
      const Eigen::Vector3d& _offset) const = 0;

  /// A version of getJacobianSpatialDeriv(const Eigen::Vector3d&) that allows
  /// an arbitrary coordinate Frame to be specified.
  virtual math::Jacobian getJacobianSpatialDeriv(
      const Eigen::Vector3d& _offset,
      const Frame* _inCoordinatesOf) const = 0;

  /// Return the classical time derivative of the generalized Jacobian targeting
  /// the origin of this BodyNode. The Jacobian is expressed in the World
  /// coordinate Frame.
  ///
  /// NOTE: Since this is a classical time derivative, it should be used with
  /// classical linear and angular vectors. If you are using spatial vectors,
  /// use getJacobianSpatialDeriv() instead.
  virtual const math::Jacobian& getJacobianClassicDeriv() const = 0;

  /// A version of getJacobianClassicDeriv() that can return the Jacobian in
  /// coordinates of any Frame.
  ///
  /// NOTE: Since this is a classical time derivative, it should be used with
  /// classical linear and angular vectors. If you are using spatial vectors,
  /// use getJacobianSpatialDeriv() instead.
  virtual math::Jacobian getJacobianClassicDeriv(
      const Frame* _inCoordinatesOf) const = 0;

  /// A version of getJacobianClassicDeriv() that can compute the Jacobian for
  /// an offset within the Frame of this BodyNode. The offset must be expressed
  /// in the coordinates of this BodyNode Frame.
  ///
  /// NOTE: Since this is a classical time derivative, it should be used with
  /// classical linear and angular vectors. If you are using spatial vectors,
  /// use getJacobianSpatialDeriv() instead.
  virtual math::Jacobian getJacobianClassicDeriv(
      const Eigen::Vector3d& _offset,
      const Frame* _inCoordinatesOf = Frame::World()) const = 0;

  /// Return the linear Jacobian (classical) time derivative, in terms of any
  /// coordinate Frame.
  ///
  /// NOTE: Since this is a classical time derivative, it should be used with
  /// classical linear vectors. If you are using spatial vectors, use
  /// getJacobianSpatialDeriv() instead.
  virtual math::LinearJacobian getLinearJacobianDeriv(
      const Frame* _inCoordinatesOf = Frame::World()) const = 0;

  /// A version of getLinearJacobianDeriv() that can compute the Jacobian for
  /// an offset within the Frame of this BodyNode. The offset must be expressed
  /// in coordinates of this BodyNode Frame.
  ///
  /// NOTE: Since this is a classical time derivative, it should be used with
  /// classical linear vectors. If you are using spatial vectors, use
  /// getJacobianSpatialDeriv() instead.
  virtual math::LinearJacobian getLinearJacobianDeriv(
      const Eigen::Vector3d& _offset,
      const Frame* _inCoordinatesOf = Frame::World()) const = 0;

  /// Return the angular Jacobian time derivative, in terms of any coordinate
  /// Frame.
  virtual math::AngularJacobian getAngularJacobianDeriv(
      const Frame* _inCoordinatesOf = Frame::World()) const = 0;

  /// \}

protected:

  /// Inverse kinematics module which gets lazily created upon request
  mutable std::unique_ptr<InverseKinematics> mIK;

};

} // namespace dart
} // namespace dynamics

#endif // DART_DYNAMICS_JACOBIANENTITY_H_
