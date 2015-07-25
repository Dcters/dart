/*
 * Copyright (c) 2013-2015, Georgia Tech Research Corporation
 * All rights reserved.
 *
 * Author(s): Jeongseok Lee <jslee02@gmail.com>
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

#ifndef DART_DYNAMICS_FREEJOINT_H_
#define DART_DYNAMICS_FREEJOINT_H_

#include <string>

#include <Eigen/Dense>

#include "dart/dynamics/MultiDofJoint.h"

namespace dart {
namespace dynamics {

/// class FreeJoint
class FreeJoint : public MultiDofJoint<6>
{
public:

  friend class Skeleton;

  struct Properties : MultiDofJoint<6>::Properties
  {
    Properties(const MultiDofJoint<6>::Properties& _properties =
                                                MultiDofJoint<6>::Properties());

    virtual ~Properties() = default;
  };

  /// Destructor
  virtual ~FreeJoint();

  /// Get the Properties of this FreeJoint
  Properties getFreeJointProperties() const;

  // Documentation inherited
  virtual const std::string& getType() const override;

  /// Get joint type for this class
  static const std::string& getStaticType();

  // Documentation inherited
  virtual bool isCyclic(size_t _index) const override;

  /// Convert a transform into a 6D vector that can be used to set the positions
  /// of a FreeJoint. The positions returned by this function will result in a
  /// relative transform of
  /// getTransformFromParentBodyNode() * _tf * getTransformFromChildBodyNode().inverse()
  /// between the parent BodyNode and the child BodyNode frames when applied to
  /// a FreeJoint.
  static Eigen::Vector6d convertToPositions(const Eigen::Isometry3d& _tf);

  /// Convert a FreeJoint-style 6D vector into a transform
  static Eigen::Isometry3d convertToTransform(const Eigen::Vector6d& _positions);

  /// Set the transform, spatial velocity, and spatial acceleration of the child
  /// BodyNode relative to an arbitrary Frame. The reference frame can be
  /// arbitrarily specified.
  ///
  /// If you want to set more than one kind of Cartetian coordinates (e.g.,
  /// transform and spatial velocity) at the same time, you should call
  /// corresponding setters in a certain order (transform -> velocity ->
  /// acceleration), If you don't velocity or acceleration can be corrupted by
  /// transform or velocity. This function calls the corresponding setters in
  /// the right order so that all the desired Cartetian coordinates are properly
  /// set.
  ///
  /// Pass nullptr for "newTransform", "newSpatialVelocity", or
  /// "newSpatialAcceleration" if you don't want to set them.
  ///
  /// \param[in] newTransform Desired transform of the child BodyNode.
  /// \param[in] withRespectTo The relative Frame of "newTransform".
  /// \param[in] newSpatialVelocity Desired spatial velocity of the child
  /// BodyNode.
  /// \param[in] velrelativeTo The relative frame of "newSpatialVelocity".
  /// \param[in] velinCoordinatesOf The reference frame of "newSpatialVelocity".
  /// \param[in] newSpatialAcceleration Desired spatial acceleration of the
  /// child BodyNode.
  /// \param[in] accrelativeTo The relative frame of "newSpatialAcceleration".
  /// \param[in] accinCoordinatesOf The reference frame of
  /// "newSpatialAcceleration".
  void setRelativeSpatialMotion(
      const Eigen::Isometry3d* newTransform,
      const Frame* withRespectTo,
      const Eigen::Vector6d* newSpatialVelocity,
      const Frame* velRelativeTo,
      const Frame* velInCoordinatesOf,
      const Eigen::Vector6d* newSpatialAcceleration,
      const Frame* accRelativeTo,
      const Frame* accInCoordinatesOf);

  /// Set the transform of the child BodyNode relative to the parent BodyNode
  /// \param[in] newTransform Desired transform of the child BodyNode.
  void setRelativeTransform(const Eigen::Isometry3d& newTransform);

  /// Set the transform of the child BodyNode relative to an arbitrary Frame.
  /// \param[in] newTransform Desired transform of the child BodyNode.
  /// \param[in] withRespectTo The relative Frame of "newTransform".
  void setRelativeTransform(const Eigen::Isometry3d& newTransform,
                            const Frame* withRespectTo);

  /// Set the spatial velocity of the child BodyNode relative to the parent
  /// BodyNode.
  /// \param[in] newSpatialVelocity Desired spatial velocity of the child
  /// BodyNode. The reference frame of "newSpatialVelocity" is the child
  /// BodyNode.
  void setRelativeSpatialVelocity(const Eigen::Vector6d& newSpatialVelocity);

  /// Set the spatial velocity of the child BodyNode relative to the parent
  /// BodyNode.
  /// \param[in] newSpatialVelocity Desired spatial velocity of the child
  /// BodyNode.
  /// \param[in] inCoordinatesOf The reference frame of "newSpatialVelocity".
  void setRelativeSpatialVelocity(const Eigen::Vector6d& newSpatialVelocity,
                                  const Frame* inCoordinatesOf);

  /// Set the spatial velocity of the child BodyNode relative to an arbitrary
  /// Frame.
  /// \param[in] newSpatialVelocity Desired spatial velocity of the child
  /// BodyNode.
  /// \param[in] relativeTo The relative frame of "newSpatialVelocity".
  /// \param[in] inCoordinatesOf The reference frame of "newSpatialVelocity".
  void setRelativeSpatialVelocity(const Eigen::Vector6d& newSpatialVelocity,
                                  const Frame* relativeTo,
                                  const Frame* inCoordinatesOf);

  /// Set the spatial acceleration of the child BodyNode relative to the parent
  /// BodyNode.
  /// \param[in] newSpatialVelocity Desired spatial acceleration of the child
  /// BodyNode. The reference frame of "newSpatialAcceleration" is the child
  /// BodyNode.
  void setRelativeSpatialAcceleration(
      const Eigen::Vector6d& newSpatialAcceleration);

  /// Set the spatial acceleration of the child BodyNode relative to the parent
  /// BodyNode.
  /// \param[in] newSpatialAcceleration Desired spatial acceleration of the
  /// child BodyNode.
  /// \param[in] inCoordinatesOf The reference frame of
  /// "newSpatialAcceleration".
  void setRelativeSpatialAcceleration(
      const Eigen::Vector6d& newSpatialAcceleration,
      const Frame* inCoordinatesOf);

  /// Set the spatial acceleration of the child BodyNode relative to an
  /// arbitrary Frame.
  /// \param[in] newSpatialAcceleration Desired spatial acceleration of the
  /// child BodyNode.
  /// \param[in] relativeTo The relative frame of "newSpatialAcceleration".
  /// \param[in] inCoordinatesOf The reference frame of
  /// "newSpatialAcceleration".
  void setRelativeSpatialAcceleration(
      const Eigen::Vector6d& newSpatialAcceleration,
      const Frame* relativeTo,
      const Frame* inCoordinatesOf);

  // Documentation inherited
  Eigen::Matrix6d getLocalJacobianStatic(
      const Eigen::Vector6d& _positions) const override;

  // Documentation inherited
  Eigen::Vector6d getPositionDifferencesStatic(
      const Eigen::Vector6d& _q2, const Eigen::Vector6d& _q1) const override;

protected:

  /// Constructor called by Skeleton class
  FreeJoint(const Properties& _properties);

  // Documentation inherited
  virtual Joint* clone() const override;

  using MultiDofJoint::getLocalJacobianStatic;

  // Documentation inherited
  virtual void integratePositions(double _dt) override;

  // Documentation inherited
  virtual void updateDegreeOfFreedomNames() override;

  // Documentation inherited
  virtual void updateLocalTransform() const override;

  // Documentation inherited
  virtual void updateLocalJacobian(bool =true) const override;

  // Documentation inherited
  virtual void updateLocalJacobianTimeDeriv() const override;

protected:

  /// Access mQ, which is an auto-updating variable
  const Eigen::Isometry3d& getQ() const;

  /// Transformation matrix dependent on generalized coordinates
  ///
  /// Do not use directly! Use getQ() to access this
  mutable Eigen::Isometry3d mQ;

public:
  // To get byte-aligned Eigen vectors
  EIGEN_MAKE_ALIGNED_OPERATOR_NEW
};

}  // namespace dynamics
}  // namespace dart

#endif  // DART_DYNAMICS_FREEJOINT_H_
