/*
 * Copyright (c) 2014, Georgia Tech Research Corporation
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

#ifndef DART_DYNAMICS_SINGLEDOFJOINT_H_
#define DART_DYNAMICS_SINGLEDOFJOINT_H_

#include <string>

#include "dart/dynamics/Joint.h"

namespace dart {
namespace dynamics {

class BodyNode;
class Skeleton;

/// class SingleDofJoint
class SingleDofJoint : public Joint
{
public:
  /// Constructor
  SingleDofJoint(const std::string& _name);

  /// Destructor
  virtual ~SingleDofJoint();

  // Documentation inherited
  DEPRECATED(4.1) virtual size_t getDof() const;

  // Documentation inherited
  virtual size_t getNumDofs() const;

  // Documentation inherited
  virtual void setIndexInSkeleton(size_t _index, size_t _indexInSkeleton);

  // Documentation inherited
  virtual size_t getIndexInSkeleton(size_t _index) const;

  //----------------------------------------------------------------------------
  // Input/output
  //----------------------------------------------------------------------------

  // Documentation inherited
  virtual void setInput(size_t _index, double _input);

  // Documentation inherited
  virtual double getInput(size_t _index) const;

  // Documentation inherited
  virtual void setInputs(const Eigen::VectorXd& _inputs);

  // Documentation inherited
  virtual Eigen::VectorXd getInputs() const;

  // Documentation inherited
  virtual void resetInputs();

  //----------------------------------------------------------------------------
  // Position
  //----------------------------------------------------------------------------

  // TODO(JS): Not to use Eigen::VectorXd

  // Documentation inherited
  virtual void setPosition(size_t _index, double _position);

  // Documentation inherited
  virtual double getPosition(size_t _index) const;

  // Documentation inherited
  virtual void setPositions(const Eigen::VectorXd& _positions);

  // Documentation inherited
  virtual Eigen::VectorXd getPositions() const;

  // Documentation inherited
  virtual void resetPositions();

  // Documentation inherited
  virtual void setPositionLowerLimit(size_t _index, double _position);

  // Documentation inherited
  virtual double getPositionLowerLimit(size_t _index) const;

  // Documentation inherited
  virtual void setPositionUpperLimit(size_t _index, double _position);

  // Documentation inherited
  virtual double getPositionUpperLimit(size_t _index) const;

  //----------------------------------------------------------------------------
  // Velocity
  //----------------------------------------------------------------------------

  // Documentation inherited
  virtual void setVelocity(size_t _index, double _velocity);

  // Documentation inherited
  virtual double getVelocity(size_t _index) const;

  // Documentation inherited
  virtual void setVelocities(const Eigen::VectorXd& _velocities);

  // Documentation inherited
  virtual Eigen::VectorXd getVelocities() const;

  // Documentation inherited
  virtual void resetVelocities();

  // Documentation inherited
  virtual void setVelocityLowerLimit(size_t _index, double _velocity);

  // Documentation inherited
  virtual double getVelocityLowerLimit(size_t _index) const;

  // Documentation inherited
  virtual void setVelocityUpperLimit(size_t _index, double _velocity);

  // Documentation inherited
  virtual double getVelocityUpperLimit(size_t _index) const;

  //----------------------------------------------------------------------------
  // Acceleration
  //----------------------------------------------------------------------------

  // Documentation inherited
  virtual void setAcceleration(size_t _index, double _acceleration);

  // Documentation inherited
  virtual double getAcceleration(size_t _index) const;

  // Documentation inherited
  virtual void setAccelerations(const Eigen::VectorXd& _accelerations);

  // Documentation inherited
  virtual Eigen::VectorXd getAccelerations() const;

  // Documentation inherited
  virtual void resetAccelerations();

  // Documentation inherited
  virtual void setAccelerationLowerLimit(size_t _index, double _acceleration);

  // Documentation inherited
  virtual double getAccelerationLowerLimit(size_t _index) const;

  // Documentation inherited
  virtual void setAccelerationUpperLimit(size_t _index, double _acceleration);

  // Documentation inherited
  virtual double getAccelerationUpperLimit(size_t _index) const;

  //----------------------------------------------------------------------------
  // Force
  //----------------------------------------------------------------------------

  // Documentation inherited
  virtual void setForce(size_t _index, double _force);

  // Documentation inherited
  virtual double getForce(size_t _index);

  // Documentation inherited
  virtual void setForces(const Eigen::VectorXd& _forces);

  // Documentation inherited
  virtual Eigen::VectorXd getForces() const;

  // Documentation inherited
  virtual void resetForces();

  // Documentation inherited
  virtual void setForceLowerLimit(size_t _index, double _force);

  // Documentation inherited
  virtual double getForceLowerLimit(size_t _index) const;

  // Documentation inherited
  virtual void setForceUpperLimit(size_t _index, double _force);

  // Documentation inherited
  virtual double getForceUpperLimit(size_t _index) const;

  //----------------------------------------------------------------------------
  // Velocity change
  //----------------------------------------------------------------------------

  // Documentation inherited
  virtual void setVelocityChange(size_t _index, double _velocityChange);

  // Documentation inherited
  virtual double getVelocityChange(size_t _index) const;

  // Documentation inherited
  virtual void resetVelocityChanges();

  //----------------------------------------------------------------------------
  // Constraint impulse
  //----------------------------------------------------------------------------

  // Documentation inherited
  virtual void setConstraintImpulse(size_t _index, double _impulse);

  // Documentation inherited
  virtual double getConstraintImpulse(size_t _index) const;

  // Documentation inherited
  virtual void resetConstraintImpulses();

  //----------------------------------------------------------------------------
  // Integration
  //----------------------------------------------------------------------------

  // Documentation inherited
  virtual void integratePositions(double _dt);

  // Documentation inherited
  virtual void integrateVelocities(double _dt);

  //----------------------------------------------------------------------------
  // Spring and damper
  //----------------------------------------------------------------------------

  // Documentation inherited
  virtual void setSpringStiffness(size_t _index, double _k);

  // Documentation inherited
  virtual double getSpringStiffness(size_t _index) const;

  // Documentation inherited
  virtual void setRestPosition(size_t _index, double _q0);

  // Documentation inherited
  virtual double getRestPosition(size_t _index) const;

  // Documentation inherited
  virtual void setDampingCoefficient(size_t _index, double _d);

  // Documentation inherited
  virtual double getDampingCoefficient(size_t _index) const;

  //----------------------------------------------------------------------------

  /// Get potential energy
  virtual double getPotentialEnergy() const;

protected:
  //----------------------------------------------------------------------------
  /// \{ \name Recursive dynamics routines
  //----------------------------------------------------------------------------

  // Documentation inherited
  virtual const math::Jacobian getLocalJacobian() const override;

  // Documentation inherited
  virtual const math::Jacobian getLocalJacobianTimeDeriv() const override;

  // Documentation inherited
  virtual void addVelocityTo(Eigen::Vector6d& _vel) override;

  // Documentation inherited
  virtual void setPartialAccelerationTo(
      Eigen::Vector6d& _partialAcceleration,
      const Eigen::Vector6d& _childVelocity) override;

  // Documentation inherited
  virtual void addAccelerationTo(Eigen::Vector6d& _acc) override;

  // Documentation inherited
  virtual void addVelocityChangeTo(Eigen::Vector6d& _velocityChange) override;

  // Documentation inherited
  virtual void addChildArtInertiaFDTo(
      Eigen::Matrix6d& _parentArtInertia,
      const Eigen::Matrix6d& _childArtInertia) override;

  // Documentation inherited
  virtual void addChildArtInertiaHDTo(
      Eigen::Matrix6d& _parentArtInertia,
      const Eigen::Matrix6d& _childArtInertia) override;

  /// \brief addChildArtInertiaToHDTorqueType
  /// \param _parentArtInertia
  /// \param _childArtInertia
  void addChildArtInertiaToHDTorqueType(
      Eigen::Matrix6d& _parentArtInertia,
      const Eigen::Matrix6d& _childArtInertia);

  /// \brief addChildArtInertiaToHDAccelerationType
  /// \param _parentArtInertia
  /// \param _childArtInertia
  void addChildArtInertiaToHDAccelerationType(
      Eigen::Matrix6d& _parentArtInertia,
      const Eigen::Matrix6d& _childArtInertia);

  // Documentation inherited
  virtual void addChildArtInertiaImplicitFDTo(
      Eigen::Matrix6d& _parentArtInertia,
      const Eigen::Matrix6d& _childArtInertia) override;

  // Documentation inherited
  virtual void addChildArtInertiaImplicitHDTo(
      Eigen::Matrix6d& _parentArtInertia,
      const Eigen::Matrix6d& _childArtInertia) override;

  /// \brief addChildArtInertiaImplicitToHDTorqueType
  /// \param _parentArtInertia
  /// \param _childArtInertia
  void addChildArtInertiaImplicitToHDTorqueType(
      Eigen::Matrix6d& _parentArtInertia,
      const Eigen::Matrix6d& _childArtInertia);

  /// \brief addChildArtInertiaImplicitToHDAccelerationType
  /// \param _parentArtInertia
  /// \param _childArtInertia
  void addChildArtInertiaImplicitToHDAccelerationType(
      Eigen::Matrix6d& _parentArtInertia,
      const Eigen::Matrix6d& _childArtInertia);

  // Documentation inherited
  virtual void updateInvProjArtInertiaFD(
      const Eigen::Matrix6d& _artInertia) override;

  // Documentation inherited
  virtual void updateInvProjArtInertiaHD(
      const Eigen::Matrix6d& _artInertia) override;

  /// \brief updateInvProjArtInertiaHDTorqueType
  /// \param _artInertia
  void updateInvProjArtInertiaHDTorqueType(
      const Eigen::Matrix6d& _artInertia);

  /// \brief updateInvProjArtInertiaHDAccelerationType
  /// \param _artInertia
  void updateInvProjArtInertiaHDAccelerationType(
      const Eigen::Matrix6d& _artInertia);

  // Documentation inherited
  virtual void updateInvProjArtInertiaImplicitFD(
      const Eigen::Matrix6d& _artInertia,
      double _timeStep) override;

  // Documentation inherited
  virtual void updateInvProjArtInertiaImplicitHD(
      const Eigen::Matrix6d& _artInertia,
      double _timeStep) override;

  void updateInvProjArtInertiaImplicitHDTorqueType(
      const Eigen::Matrix6d& _artInertia,
      double _timeStep);

  void updateInvProjArtInertiaImplicitHDAccelerationType(
      const Eigen::Matrix6d& _artInertia,
      double _timeStep);

  // Documentation inherited
  virtual void addChildBiasForceFDTo(
      Eigen::Vector6d& _parentBiasForce,
      const Eigen::Matrix6d& _childArtInertia,
      const Eigen::Vector6d& _childBiasForce,
      const Eigen::Vector6d& _childPartialAcc) override;

  virtual void addChildBiasForceHDTo(
      Eigen::Vector6d& _parentBiasForce,
      const Eigen::Matrix6d& _childArtInertia,
      const Eigen::Vector6d& _childBiasForce,
      const Eigen::Vector6d& _childPartialAcc) override;

  void addChildBiasForceToHDTorqueType(
      Eigen::Vector6d& _parentBiasForce,
      const Eigen::Matrix6d& _childArtInertia,
      const Eigen::Vector6d& _childBiasForce,
      const Eigen::Vector6d& _childPartialAcc);

  void addChildBiasForceToHDAccelerationType(
      Eigen::Vector6d& _parentBiasForce,
      const Eigen::Matrix6d& _childArtInertia,
      const Eigen::Vector6d& _childBiasForce,
      const Eigen::Vector6d& _childPartialAcc);

  // Documentation inherited
  virtual void addChildBiasImpulseFDTo(
      Eigen::Vector6d& _parentBiasImpulse,
      const Eigen::Matrix6d& _childArtInertia,
      const Eigen::Vector6d& _childBiasImpulse) override;

  // Documentation inherited
  virtual void addChildBiasImpulseHDTo(
      Eigen::Vector6d& _parentBiasImpulse,
      const Eigen::Matrix6d& _childArtInertia,
      const Eigen::Vector6d& _childBiasImpulse) override;

  /// \brief
  void addChildBiasImpulseToHDTorqueType(
      Eigen::Vector6d& _parentBiasImpulse,
      const Eigen::Matrix6d& _childArtInertia,
      const Eigen::Vector6d& _childBiasImpulse);

  /// \brief
  void addChildBiasImpulseToHDAccelerationType(
      Eigen::Vector6d& _parentBiasImpulse,
      const Eigen::Matrix6d& _childArtInertia,
      const Eigen::Vector6d& _childBiasImpulse);

  // Documentation inherited
  virtual void updateTotalForceFD(const Eigen::Vector6d& _bodyForce,
                                  double _timeStep) override;

  // Documentation inherited
  virtual void updateTotalForceHD(const Eigen::Vector6d& _bodyForce,
                                  double _timeStep) override;

  /// \brief updateTotalForceHDTorqueType
  /// \param _bodyForce
  /// \param _timeStep
  void updateTotalForceHDTorqueType(const Eigen::Vector6d& _bodyForce,
                                     double _timeStep);

  /// \brief updateTotalForceHDAccelerationType
  /// \param _bodyForce
  /// \param _timeStep
  void updateTotalForceHDAccelerationType(const Eigen::Vector6d& _bodyForce,
                                           double _timeStep);

  // Documentation inherited
  virtual void updateTotalImpulseFD(
      const Eigen::Vector6d& _bodyImpulse) override;

  // Documentation inherited
  virtual void updateTotalImpulseHD(
      const Eigen::Vector6d& _bodyImpulse) override;

  /// \brief updateTotalImpulseHDTorqueType
  /// \param _bodyImpulse
  void updateTotalImpulseHDTorqueType(
      const Eigen::Vector6d& _bodyImpulse);

  /// \brief updateTotalImpulseHDAccelerationType
  /// \param _bodyImpulse
  void updateTotalImpulseHDAccelerationType(
      const Eigen::Vector6d& _bodyImpulse);

  // Documentation inherited
  virtual void resetTotalImpulses() override;

  // Documentation inherited
  virtual void updateAccelerationFD(
      const Eigen::Matrix6d& _artInertia,
      const Eigen::Vector6d& _spatialAcc) override;

  // Documentation inherited
  virtual void updateAccelerationHD(
      const Eigen::Matrix6d& _artInertia,
      const Eigen::Vector6d& _spatialAcc) override;

  /// \brief updateAccelerationHDTorqueType
  /// \param _artInertia
  /// \param _spatialAcc
  void updateAccelerationHDTorqueType(
      const Eigen::Matrix6d& _artInertia,
      const Eigen::Vector6d& _spatialAcc);

  /// \brief updateAccelerationHDAccelerationType
  /// \param _artInertia
  /// \param _spatialAcc
  void updateAccelerationHDAccelerationType(
      const Eigen::Matrix6d& _artInertia,
      const Eigen::Vector6d& _spatialAcc);

  // Documentation inherited
  virtual void updateVelocityChangeFD(
      const Eigen::Matrix6d& _artInertia,
      const Eigen::Vector6d& _velocityChange) override;

  // Documentation inherited
  virtual void updateVelocityChangeHD(
      const Eigen::Matrix6d& _artInertia,
      const Eigen::Vector6d& _velocityChange) override;

  /// \brief updateVelocityChangeHDTorqueType
  /// \param _artInertia
  /// \param _velocityChange
  void updateVelocityChangeHDTorqueType(
      const Eigen::Matrix6d& _artInertia,
      const Eigen::Vector6d& _velocityChange);

  /// \brief updateVelocityChangeHDAccelerationType
  /// \param _artInertia
  /// \param _velocityChange
  void updateVelocityChangeHDAccelerationType(
      const Eigen::Matrix6d& _artInertia,
      const Eigen::Vector6d& _velocityChange);

  // Documentation inherited
  virtual void updateForceID(const Eigen::Vector6d& _bodyForce) override;

  // Documentation inherited
  virtual void updateForceHD(const Eigen::Vector6d& _bodyForce) override;

  // Documentation inherited
  virtual void updateImpulseID(const Eigen::Vector6d& _bodyImpulse) override;

  // Documentation inherited
  virtual void updateImpulseHD(const Eigen::Vector6d& _bodyImpulse) override;

  // Documentation inherited
  virtual void updateConstrainedTermsFD(double _timeStep) override;

  // Documentation inherited
  virtual void updateConstrainedTermsHD(double _timeStep) override;

  /// \brief updateConstrainedTermsHDTorqueType
  /// \param _timeStep
  void updateConstrainedTermsHDTorqueType(double _timeStep);

  /// \brief updateConstrainedTermsHDAccelerationType
  /// \param _timeStep
  void updateConstrainedTermsHDAccelerationType(double _timeStep);

  /// \}

  //----------------------------------------------------------------------------
  // Recursive algorithms for equations of motion
  //----------------------------------------------------------------------------

  /// Add child's bias force to parent's one
  virtual void addChildBiasForceForInvMassMatrix(
      Eigen::Vector6d& _parentBiasForce,
      const Eigen::Matrix6d& _childArtInertia,
      const Eigen::Vector6d& _childBiasForce);

  /// Add child's bias force to parent's one
  virtual void addChildBiasForceForInvAugMassMatrix(
      Eigen::Vector6d& _parentBiasForce,
      const Eigen::Matrix6d& _childArtInertia,
      const Eigen::Vector6d& _childBiasForce);

  ///
  virtual void updateTotalForceForInvMassMatrix(
      const Eigen::Vector6d& _bodyForce);

  // Documentation inherited
  virtual void getInvMassMatrixSegment(Eigen::MatrixXd& _invMassMat,
                                       const size_t _col,
                                       const Eigen::Matrix6d& _artInertia,
                                       const Eigen::Vector6d& _spatialAcc);

  // Documentation inherited
  virtual void getInvAugMassMatrixSegment(Eigen::MatrixXd& _invMassMat,
                                          const size_t _col,
                                          const Eigen::Matrix6d& _artInertia,
                                          const Eigen::Vector6d& _spatialAcc);

  // Documentation inherited
  virtual void addInvMassMatrixSegmentTo(Eigen::Vector6d& _acc);

  // Documentation inherited
  virtual Eigen::VectorXd getSpatialToGeneralized(
      const Eigen::Vector6d& _spatial);

protected:
  // TODO(JS): Need?
  ///
  size_t mIndexInSkeleton;

  /// Input
  double mInput;

  //----------------------------------------------------------------------------
  // Configuration
  //----------------------------------------------------------------------------

  /// Position
  double mPosition;

  /// Lower limit of position
  double mPositionLowerLimit;

  /// Upper limit of position
  double mPositionUpperLimit;

  /// Derivatives w.r.t. an arbitrary scalr variable
  double mPositionDeriv;

  //----------------------------------------------------------------------------
  // Velocity
  //----------------------------------------------------------------------------

  /// Generalized velocity
  double mVelocity;

  /// Min value allowed.
  double mVelocityLowerLimit;

  /// Max value allowed.
  double mVelocityUpperLimit;

  /// Derivatives w.r.t. an arbitrary scalr variable
  double mVelocityDeriv;

  //----------------------------------------------------------------------------
  // Acceleration
  //----------------------------------------------------------------------------

  /// Generalized acceleration
  double mAcceleration;

  /// Min value allowed.
  double mAccelerationLowerLimit;

  /// upper limit of generalized acceleration
  double mAccelerationUpperLimit;

  /// Derivatives w.r.t. an arbitrary scalr variable
  double mAccelerationDeriv;

  //----------------------------------------------------------------------------
  // Force
  //----------------------------------------------------------------------------

  /// Generalized force
  double mForce;

  /// Min value allowed.
  double mForceLowerLimit;

  /// Max value allowed.
  double mForceUpperLimit;

  /// Derivatives w.r.t. an arbitrary scalr variable
  double mForceDeriv;

  //----------------------------------------------------------------------------
  // Impulse
  //----------------------------------------------------------------------------

  /// Change of generalized velocity
  double mVelocityChange;

  /// Generalized impulse
  double mImpulse;

  /// Generalized constraint impulse
  double mConstraintImpulse;

  //----------------------------------------------------------------------------
  // Spring and damper
  //----------------------------------------------------------------------------

  /// Joint spring stiffness
  double mSpringStiffness;

  /// Rest joint position for joint spring
  double mRestPosition;

  /// Joint damping coefficient
  double mDampingCoefficient;

  //----------------------------------------------------------------------------
  // For recursive dynamics algorithms
  //----------------------------------------------------------------------------

  /// Spatial Jacobian expressed in the child body frame
  Eigen::Vector6d mJacobian;

  /// Time derivative of spatial Jacobian expressed in the child body frame
  Eigen::Vector6d mJacobianDeriv;

  /// Inverse of projected articulated inertia
  double mInvProjArtInertia;

  /// Inverse of projected articulated inertia for implicit joint damping and
  /// spring forces
  double mInvProjArtInertiaImplicit;

  /// Total force projected on joint space
  double mTotalForce;

  /// Total impluse projected on joint space
  double mTotalImpulse;

  //----------------------------------------------------------------------------
  // For equations of motion
  //----------------------------------------------------------------------------

  ///
  double mInvM_a;

  ///
  double mInvMassMatrixSegment;

private:
};

}  // namespace dynamics
}  // namespace dart

#endif  // DART_DYNAMICS_SINGLEDOFJOINT_H_
