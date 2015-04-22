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

#ifndef DART_DYNAMICS_INVERSEKINEMATICS_H_
#define DART_DYNAMICS_INVERSEKINEMATICS_H_

#include <memory>

#include "dart/common/sub_ptr.h"
#include "dart/common/Signal.h"
#include "dart/optimizer/Solver.h"
#include "dart/optimizer/Function.h"
#include "dart/dynamics/SimpleFrame.h"
#include "dart/dynamics/Skeleton.h"
#include "dart/dynamics/BodyNode.h"

namespace dart {
namespace dynamics {

const double DefaultIKTolerance = 1e-6;
const double DefaultIKErrorClamp = 0.2;

/// The inverse kinematics class is templated to operate on either a BodyNode
/// or an EndEffector
template <class JacobianEntity>
class InverseKinematics : public common::Subject
{
public:

  // Qt Creator does not recognize this syntax, but it is valid as of C++11
  friend JacobianEntity;

  /// Method is a base class for different InverseKinematics methods
  class ErrorMethod : public common::Subject
  {
  public:

    ErrorMethod(InverseKinematics<JacobianEntity>* _ik,
                const std::string& _methodName);

    virtual ~ErrorMethod();

    virtual Eigen::Vector6d computeError() = 0;

    const Eigen::Vector6d& computeError(Eigen::Map<const Eigen::VectorXd>& _q);

    const std::string& getMethodName() const;

    void setBounds(
        const Eigen::Vector6d& _lower =
            Eigen::Vector6d::Constant(DefaultIKTolerance),
        const Eigen::Vector6d& _upper =
            Eigen::Vector6d::Constant(DefaultIKTolerance));

    void setBounds(const std::pair<Eigen::Vector6d, Eigen::Vector6d>& _bounds);

    const std::pair<Eigen::Vector6d, Eigen::Vector6d>& getBounds() const;

    void setAngularBounds(
        const Eigen::Vector3d& _lower =
            Eigen::Vector3d::Constant(DefaultIKTolerance),
        const Eigen::Vector3d& _upper =
            Eigen::Vector3d::Constant(DefaultIKTolerance));

    void setAngularBounds(
        const std::pair<Eigen::Vector3d, Eigen::Vector3d>& _bounds);

    std::pair<Eigen::Vector3d, Eigen::Vector3d> getAngularBounds() const;

    void setLinearBounds(
        const Eigen::Vector3d& _lower =
            Eigen::Vector3d::Constant(DefaultIKTolerance),
        const Eigen::Vector3d& _upper =
            Eigen::Vector3d::Constant(DefaultIKTolerance));

    void setLinearBounds(
        const std::pair<Eigen::Vector3d, Eigen::Vector3d>& _bounds);

    std::pair<Eigen::Vector3d, Eigen::Vector3d> getLinearBounds() const;

    void setErrorLengthClamp(double _clampSize = DefaultIKErrorClamp);

    double getErrorLengthClamp() const;

    void setErrorWeights(const Eigen::Vector6d& _weights);

    const Eigen::Vector6d& getErrorWeights() const;

    void setAngularErrorWeights(
        const Eigen::Vector3d& _weights = Eigen::Vector3d::Constant(0.2));

    Eigen::Vector3d getAngularErrorWeights() const;

    void setLinearErrorWeights(
        const Eigen::Vector3d& _weights = Eigen::Vector3d::Constant(1.0));

    Eigen::Vector3d getLinearErrorWeights() const;

    void clearCache();

  protected:

    common::sub_ptr< InverseKinematics<JacobianEntity> > mIK;

    std::string mMethodName;

    Eigen::VectorXd mLastConfig;

    Eigen::Vector6d mLastError;

    std::pair<Eigen::Vector6d, Eigen::Vector6d> mBounds;

    double mErrorLengthClamp;

    Eigen::Vector6d mErrorWeights;

  };

  class EulerAngleXYZMethod : public ErrorMethod
  {
  public:

    explicit EulerAngleXYZMethod(InverseKinematics<JacobianEntity>* _ik);

    virtual ~EulerAngleXYZMethod();

    virtual Eigen::Vector6d computeError() override;

    bool mComputeErrorFromCenter;
  };

  class GradientMethod : public common::Subject
  {
  public:

    GradientMethod(InverseKinematics<JacobianEntity>* _ik,
                   const std::string& _methodName);

    virtual ~GradientMethod();

    virtual void computeGradient(const Eigen::Vector6d& _error,
                                 Eigen::VectorXd& _grad) = 0;

    void computeGradient(Eigen::Map<const Eigen::VectorXd>& _q,
                         Eigen::Map<Eigen::VectorXd> _grad);

    const std::string& getMethodName() const;

    void clampGradient(Eigen::VectorXd& _grad) const;

    void setComponentWiseClamp(double _clamp = 0.2);

    double getComponentWiseClamp() const;

    void clearCache();

  protected:

    common::sub_ptr< InverseKinematics<JacobianEntity> > mIK;

    std::string mMethodName;

    Eigen::VectorXd mLastConfig;

    Eigen::VectorXd mLastGradient;

    double mComponentWiseClamp;

  };

  class JacobianDLS : public GradientMethod
  {
  public:

    explicit JacobianDLS(InverseKinematics<JacobianEntity>* _ik);

    virtual ~JacobianDLS();

    virtual void computeGradient(const Eigen::Vector6d& _error,
                                 Eigen::VectorXd& _grad) override;

    void setDampingCoefficient(double _damping = 0.05);

    double getDampingCoefficient() const;

  protected:

    double mDamping;
  };

  class JacobianTranspose : public GradientMethod
  {
  public:

    explicit JacobianTranspose(InverseKinematics<JacobianEntity>* _ik);

    virtual ~JacobianTranspose();

    virtual void computeGradient(const Eigen::Vector6d& _error,
                                 Eigen::VectorXd& _grad) override;
  };

  void setActive(bool _active=true);

  void setInactive();

  bool isActive() const;

  void setHierarchyLevel(size_t _level);

  size_t getHierarchyLevel() const;

  void useLinkage();

  void useWholeBody();

  void useDofs(const std::vector<size_t>& _dofs);

  const std::vector<size_t>& getDofs() const;

  const std::vector<int>& getDofMap() const;

  template <class IKErrorMethod>
  IKErrorMethod* setErrorMethod();

  ErrorMethod* getErrorMethod();

  const ErrorMethod* getErrorMethod() const;

  template <class IKGradientMethod>
  IKGradientMethod* setGradientMethod();

  GradientMethod* getGradientMethod();

  const GradientMethod* getGradientMethod() const;

  void setSolver(std::shared_ptr<optimizer::Solver> _newSolver);

  std::shared_ptr<optimizer::Solver> getSolver();

  std::shared_ptr<const optimizer::Solver> getSolver() const;

  void setTarget(std::shared_ptr<SimpleFrame> _newTarget);

  std::shared_ptr<SimpleFrame> getTarget();

  std::shared_ptr<const SimpleFrame> getTarget() const;

  JacobianEntity* getEntity();

  const JacobianEntity* getEntity() const;

  const math::Jacobian& computeJacobian() const;

  void clearCaches();

  virtual ~InverseKinematics();

protected:

  InverseKinematics(JacobianEntity* _entity);

  void resetTargetConnection();

  void resetEntityConnection();

  common::Connection mTargetConnection;

  common::Connection mEntityConnection;

  bool mActive;

  size_t mHierarchyLevel;

  std::vector<size_t> mDofs;

  std::vector<int> mDofMap;

  /// The method that this IK module will use to compute errors
  std::unique_ptr<ErrorMethod> mErrorMethod;

  /// The method that this IK module will use to compute gradients
  std::unique_ptr<GradientMethod> mGradientMethod;

  /// The solver that this IK module will use for iterative methods
  std::shared_ptr<optimizer::Solver> mSolver;


  std::shared_ptr<SimpleFrame> mTarget;


  sub_ptr<JacobianEntity> mEntity;


  mutable math::Jacobian mJacobian;
};

//==============================================================================
template <class JacobianEntity>
InverseKinematics<JacobianEntity>::ErrorMethod::ErrorMethod(
    InverseKinematics<JacobianEntity>* _ik,
    const std::string& _methodName)
  : mIK(_ik),
    mMethodName(_methodName),
    mLastConfig(0),
    mLastError(Eigen::Vector6d::Constant(std::nan(nullptr))),
    mBounds(Eigen::Vector6d::Constant(DefaultIKTolerance),
            Eigen::Vector6d::Constant(DefaultIKTolerance)),
    mErrorLengthClamp(DefaultIKErrorClamp)
{
  setAngularErrorWeights();
  setLinearErrorWeights();
}

//==============================================================================
template <class JacobianEntity>
InverseKinematics<JacobianEntity>::ErrorMethod::~ErrorMethod()
{
  // Do nothing
}

//==============================================================================
template <class JacobianEntity>
const Eigen::Vector6d& InverseKinematics<JacobianEntity>::
ErrorMethod::computeError(Eigen::Map<const Eigen::VectorXd>& _q)
{
  if(_q.size() != mIK->getDofs().size())
  {
    dterr << "[InverseKinematics::ErrorMethod::computeError] Mismatch between "
          << "configuration size [" << _q.size() << "] and the available "
          << "degrees of freedom [" << mIK->getDofs().size() <<"]."
          << "\nSkeleton name: " << mIK->getEntity()->getSkeleton()->getName()
          << "\nBody name: " << mIK->getEntity()->getName()
          << "\nMethod name: " << mMethodName << "\n";
    mLastError.setZero();
    return mLastError;
  }

  if(_q.size() == mLastConfig.size())
  {
    bool repeat = true;
    for(int i=0; i<mLastConfig.size(); ++i)
    {
      if(_q[i] != mLastConfig[i])
      {
        repeat = false;
        break;
      }
    }

    if(repeat)
      return mLastError;
  }

  dart::dynamics::Skeleton* skel = mIK->getEntity()->getSkeleton();
  skel->setPositionSegment(mIK->getDofs(), _q);
  mLastConfig = _q;

  mLastError = computeError();
  return mLastError;
}

//==============================================================================
template <class JacobianEntity>
const std::string& InverseKinematics<JacobianEntity>::
ErrorMethod::getMethodName() const
{
  return mMethodName;
}

//==============================================================================
template <class JacobianEntity>
void InverseKinematics<JacobianEntity>::ErrorMethod::setBounds(
    const Eigen::Vector6d& _lower, const Eigen::Vector6d& _upper)
{
  mBounds.first = _lower;
  mBounds.second = _upper;
}

//==============================================================================
template <class JacobianEntity>
void InverseKinematics<JacobianEntity>::ErrorMethod::setBounds(
    const std::pair<Eigen::Vector6d, Eigen::Vector6d>& _bounds)
{
  mBounds = _bounds;
}

//==============================================================================
template <class JacobianEntity>
const std::pair<Eigen::Vector6d, Eigen::Vector6d>&
InverseKinematics<JacobianEntity>::ErrorMethod::getBounds() const
{
  return mBounds;
}

//==============================================================================
template <class JacobianEntity>
void InverseKinematics<JacobianEntity>::ErrorMethod::setAngularBounds(
    const Eigen::Vector3d& _lower, const Eigen::Vector3d& _upper)
{
  mBounds.first.head<3>() = _lower;
  mBounds.second.head<3>() = _upper;
}

//==============================================================================
template <class JacobianEntity>
void InverseKinematics<JacobianEntity>::ErrorMethod::setAngularBounds(
    const std::pair<Eigen::Vector3d, Eigen::Vector3d>& _bounds)
{
  setAngularBounds(_bounds.first, _bounds.second);
}

//==============================================================================
template <class JacobianEntity>
std::pair<Eigen::Vector3d, Eigen::Vector3d>
InverseKinematics<JacobianEntity>::ErrorMethod::getAngularBounds() const
{
  return std::pair<Eigen::Vector3d, Eigen::Vector3d>(
        mBounds.first.head<3>(), mBounds.second.head<3>());
}

//==============================================================================
template <class JacobianEntity>
void InverseKinematics<JacobianEntity>::ErrorMethod::setLinearBounds(
    const Eigen::Vector3d& _lower, const Eigen::Vector3d& _upper)
{
  mBounds.first.tail<3>() = _lower;
  mBounds.second.tail<3>() = _upper;
}

//==============================================================================
template <class JacobianEntity>
void InverseKinematics<JacobianEntity>::ErrorMethod::setLinearBounds(
    const std::pair<Eigen::Vector3d, Eigen::Vector3d>& _bounds)
{
  setLinearBounds(_bounds.first, _bounds.second);
}

//==============================================================================
template <class JacobianEntity>
std::pair<Eigen::Vector3d, Eigen::Vector3d>
InverseKinematics<JacobianEntity>::ErrorMethod::getLinearBounds() const
{
  return std::pair<Eigen::Vector3d, Eigen::Vector3d>(
        mBounds.first.tail<3>(), mBounds.second.tail<3>());
}

//==============================================================================
template <class JacobianEntity>
void InverseKinematics<JacobianEntity>::
ErrorMethod::setErrorLengthClamp(double _clampSize)
{
  mErrorLengthClamp = _clampSize;
}

//==============================================================================
template <class JacobianEntity>
double InverseKinematics<JacobianEntity>::
ErrorMethod::getErrorLengthClamp() const
{
  return mErrorLengthClamp;
}

//==============================================================================
template <class JacobianEntity>
void InverseKinematics<JacobianEntity>::
ErrorMethod::setErrorWeights(const Eigen::Vector6d& _weights)
{
  mErrorWeights = _weights;
}

//==============================================================================
template <class JacobianEntity>
const Eigen::Vector6d&
InverseKinematics<JacobianEntity>::ErrorMethod::getErrorWeights() const
{
  return mErrorWeights;
}

//==============================================================================
template <class JacobianEntity>
void InverseKinematics<JacobianEntity>::
ErrorMethod::setAngularErrorWeights(const Eigen::Vector3d& _weights)
{
  mErrorWeights.head<3>() = _weights;
}

//==============================================================================
template <class JacobianEntity>
Eigen::Vector3d InverseKinematics<JacobianEntity>::
ErrorMethod::getAngularErrorWeights() const
{
  return mErrorWeights.head<3>();
}

//==============================================================================
template <class JacobianEntity>
void InverseKinematics<JacobianEntity>::
ErrorMethod::setLinearErrorWeights(const Eigen::Vector3d& _weights)
{
  mErrorWeights.tail<3>() = _weights;
}

//==============================================================================
template <class JacobianEntity>
Eigen::Vector3d InverseKinematics<JacobianEntity>::
ErrorMethod::getLinearErrorWeights() const
{
  return mErrorWeights.tail<3>();
}

//==============================================================================
template <class JacobianEntity>
void InverseKinematics<JacobianEntity>::ErrorMethod::clearCache()
{
  // This will force the error to be recomputed the next time computeError is called
  if(mLastConfig.size() > 0)
    mLastConfig[0] = std::nan(nullptr);
}

//==============================================================================
template <class JacobianEntity>
InverseKinematics<JacobianEntity>::EulerAngleXYZMethod::EulerAngleXYZMethod(
    InverseKinematics<JacobianEntity>* _ik)
  : ErrorMethod(_ik, "EulerAngleXYZ"),
    mComputeErrorFromCenter(true)
{
  // Do nothing
}

//==============================================================================
template <class JacobianEntity>
InverseKinematics<JacobianEntity>::EulerAngleXYZMethod::~EulerAngleXYZMethod()
{
  // Do nothing
}

//==============================================================================
template <class JacobianEntity>
Eigen::Vector6d
InverseKinematics<JacobianEntity>::EulerAngleXYZMethod::computeError()
{
  // This is a slightly modified implementation of the Berenson et al Task Space
  // Region method found in "Task Space Regions: A Framework for
  // Pose-Constrained Manipulation Planning".
  //
  // The modification is that we handle the computation of translational error
  // independently from the computation of rotational error, whereas the work of
  // Berenson computes them simultaneously using homogeneous transform matrices.
  // Handling them in terms of their homogeneous matrices results in less direct
  // paths while performing gradient descent, because there is a pseudo-error in
  // translation whenever rotational error exists.
  //
  // Also, we offer the option of always computing the error from the center of
  // the Task Space Region, which will often allow the end effector to enter the
  // valid constraint manifold faster than computing it from the edge of the
  // Task Space Region.

  // Use the target's transform with respect to its reference frame
  const Eigen::Isometry3d& targetTf =
      this->mIK->getTarget()->getRelativeTransform();
  // Use the actual transform with respect to the target's reference frame
  const Eigen::Isometry3d& actualTf =
      this->mIK->getEntity()->getTransform(
        this->mIK->getTarget()->getParentFrame());

  // ^ This scheme makes it so that the bounds are expressed in the reference
  // frame of the target

  Eigen::Vector3d p_error = actualTf.translation() - targetTf.translation();
  Eigen::Matrix3d R_error = actualTf.linear() * targetTf.linear().transpose();

  Eigen::Vector6d displacement;
  displacement.head<3>() = math::matrixToEulerXYZ(R_error);
  displacement.tail<3>() = p_error;

  Eigen::Vector6d error;
  const Eigen::Vector6d& min = this->mBounds.first;
  const Eigen::Vector6d& max = this->mBounds.second;
  for(int i=0; i<6; ++i)
  {
    if( displacement[i] < min[i] )
    {
      if(mComputeErrorFromCenter && !std::isinf(max[i]))
        error[i] = displacement[i] - (min[i]+max[i])/2.0;
      else
        error[i] = displacement[i] - min[i];
    }
    else if( max[i] < displacement[i] )
    {
      if(mComputeErrorFromCenter && !std::isinf(min[i]))
        error[i] = displacement[i] - (min[i]+max[i])/2.0;
      else
        error[i] = displacement[i] - max[i];
    }
    else
      error[i] = 0.0;
  }

  error = error.cwiseProduct(this->mErrorWeights);

  if(error.norm() > this->mErrorLengthClamp)
    error = error.normalized()*this->mErrorLengthClamp;

  if(!this->mIK->getTarget()->getParentFrame()->isWorld())
  {
    // Transform the error term into the world frame if it's not already
    const Eigen::Isometry3d& R =
        this->mIK->getTarget()->getParentFrame()->getWorldTransform();
    error = R.linear()*error;
  }

  return error;
}

//==============================================================================
template <class JacobianEntity>
InverseKinematics<JacobianEntity>::GradientMethod::GradientMethod(
    InverseKinematics<JacobianEntity>* _ik, const std::string& _methodName)
  : mIK(_ik),
    mMethodName(_methodName),
    mLastConfig(0),
    mLastGradient(0)
{
  setComponentWiseClamp();
}

//==============================================================================
template <class JacobianEntity>
InverseKinematics<JacobianEntity>::GradientMethod::~GradientMethod()
{
  // Do nothing
}

//==============================================================================
template <class JacobianEntity>
void InverseKinematics<JacobianEntity>::GradientMethod::computeGradient(
    Eigen::Map<const Eigen::VectorXd>& _q,
    Eigen::Map<Eigen::VectorXd> _grad)
{
  if(_q.size() != mIK->getDofs().size())
  {
    dterr << "[InverseKinematics::GradientMethod::computeGradient] Mismatch "
          << "between configuration size [" << _q.size() << "] and the "
          << "available degrees of freedom [" << mIK->getDofs().size() << "]."
          << "\nSkeleton name: " << mIK->getEntity()->getSkeleton()->getName()
          << "\nBody name: " << mIK->getEntity()->getName()
          << "\nMethod name: " << mMethodName << "\n";
    mLastGradient.resize(_q.size());
    mLastGradient.setZero();
    _grad = mLastGradient;
    return;
  }

  if(_q.size() == mLastConfig.size())
  {
    bool repeat = true;
    for(int i=0; i<mLastConfig.size(); ++i)
    {
      if(_q[i] != mLastConfig[i])
      {
        repeat = false;
        break;
      }
    }

    if(repeat)
      return mLastGradient;
  }

  Eigen::Vector6d error = mIK->getErrorMethod()->computeError(_q);
  dart::dynamics::Skeleton* skel = mIK->getEntity()->getSkeleton();
  skel->setPositionSegment(mIK->getDofs(), _q);
  computeGradient(error, _grad);
  mLastGradient = _grad;
}

//==============================================================================
template <class JacobianEntity>
const std::string& InverseKinematics<JacobianEntity>::
GradientMethod::getMethodName() const
{
  return mMethodName;
}

//==============================================================================
template <class JacobianEntity>
void InverseKinematics<JacobianEntity>::
GradientMethod::clampGradient(Eigen::VectorXd &_grad) const
{
  for(int i=0; i<_grad.size(); ++i)
  {
    if(std::abs(_grad[i]) > this->mComponentWiseClamp)
      _grad[i] = _grad[i] > 0 ?  this->mComponentWiseClamp :
                                -this->mComponentWiseClamp;
  }
}

//==============================================================================
template <class JacobianEntity>
void InverseKinematics<JacobianEntity>::
GradientMethod::setComponentWiseClamp(double _clamp)
{
  mComponentWiseClamp = std::abs(_clamp);
}

//==============================================================================
template <class JacobianEntity>
double InverseKinematics<JacobianEntity>::
GradientMethod::getComponentWiseClamp() const
{
  return mComponentWiseClamp;
}

//==============================================================================
template <class JacobianEntity>
void InverseKinematics<JacobianEntity>::GradientMethod::clearCache()
{
  // This will force the gradient to be recomputed the next time computeGradient is called
  if(mLastConfig.size() > 0)
    mLastConfig[0] = std::nan(nullptr);
}

//==============================================================================
template <class JacobianEntity>
InverseKinematics<JacobianEntity>::JacobianDLS::JacobianDLS(
    InverseKinematics<JacobianEntity>* _ik)
  : GradientMethod(_ik, "JacobianDLS")
{
  setDampingCoefficient();
}

//==============================================================================
template <class JacobianEntity>
InverseKinematics<JacobianEntity>::JacobianDLS::~JacobianDLS()
{
  // Do nothing
}

//==============================================================================
template <class JacobianEntity>
void InverseKinematics<JacobianEntity>::
JacobianDLS::computeGradient(const Eigen::Vector6d& _error,
                             Eigen::VectorXd& _grad)
{
  const math::Jacobian& J = this->mIK->computeJacobian();

  int rows = J.rows(), cols = J.cols();
  if(rows <= cols)
  {
    _grad = J.transpose()*(pow(mDamping,2)*Eigen::MatrixXd::Identity(rows, rows)
            + J*J.transpose() ).inverse() * _error;
  }
  else
  {
    _grad = ( pow(mDamping,2)*Eigen::MatrixXd::Identity(cols, cols) +
            J.transpose()*J).inverse() * J.transpose() * _error;
  }

  this->clampGradient(_grad);
}

//==============================================================================
template <class JacobianEntity>
void InverseKinematics<JacobianEntity>::
JacobianDLS::setDampingCoefficient(double _damping)
{
  mDamping = _damping;
}

//==============================================================================
template <class JacobianEntity>
double InverseKinematics<JacobianEntity>::
JacobianDLS::getDampingCoefficient() const
{
  return mDamping;
}

//==============================================================================
template <class JacobianEntity>
InverseKinematics<JacobianEntity>::
JacobianTranspose::JacobianTranspose(InverseKinematics<JacobianEntity>* _ik)
  : GradientMethod(_ik, "JacobianTranspose")
{
  // Do nothing
}

//==============================================================================
template <class JacobianEntity>
void InverseKinematics<JacobianEntity>::
JacobianTranspose::computeGradient(const Eigen::Vector6d& _error,
                                   Eigen::VectorXd& _grad)
{
  const math::Jacobian& J = this->mIK->computeJacobian();
  _grad = J.transpose() * _error;
  this->clampGradient(_grad);
}

//==============================================================================
template <class JacobianEntity>
void InverseKinematics<JacobianEntity>::setActive(bool _active)
{
  mActive = _active;
}

//==============================================================================
template <class JacobianEntity>
void InverseKinematics<JacobianEntity>::setInactive()
{
  mActive = false;
}

//==============================================================================
template <class JacobianEntity>
bool InverseKinematics<JacobianEntity>::isActive() const
{
  return mActive;
}

//==============================================================================
template <class JacobianEntity>
void InverseKinematics<JacobianEntity>::setHierarchyLevel(size_t _level)
{
  mHierarchyLevel = _level;
}

//==============================================================================
template <class JacobianEntity>
size_t InverseKinematics<JacobianEntity>::getHierarchyLevel() const
{
  return mHierarchyLevel;
}

//==============================================================================
template <class JacobianEntity>
void InverseKinematics<JacobianEntity>::useLinkage()
{
  mDofs.clear();

  if(mEntity->getNumDependentGenCoords() == 0)
  {
    useDofs(mDofs);
    return;
  }

  BodyNode* baseBn = mEntity->getParentBodyNode();

  // Attempt to get at least one degree of freedom
  while(baseBn &&
      baseBn->getNumDependentGenCoords() == mEntity->getNumDependentGenCoords())
  {
    baseBn = baseBn->getParentBodyNode();
  }

  // Keep moving upstream, as long as the parent has no other children
  while(baseBn && baseBn->getNumChildBodyNodes() == 1)
  {
    baseBn = baseBn->getParentBodyNode();
  }

  size_t start = 0;
  // Ignore any coordinates that the base BodyNode depends on
  if(baseBn)
    start = baseBn->getNumDependentGenCoords();

  for(size_t i=start; i<mEntity->getNumDependentGenCoords(); ++i)
    mDofs.push_back(mEntity->getDependentGenCoordIndex(i));

  useDofs(mDofs);
}

//==============================================================================
template <class JacobianEntity>
void InverseKinematics<JacobianEntity>::useWholeBody()
{
  useDofs(mEntity->getDependentGenCoordIndices());
}

//==============================================================================
template <class JacobianEntity>
void InverseKinematics<JacobianEntity>::useDofs(
    const std::vector<size_t>& _dofs)
{
  mDofs = _dofs;
  const std::vector<size_t>& entityDependencies =
      mEntity->getDependentGenCoordIndices();

  mDofMap.resize(entityDependencies.size());
  for(size_t i=0; i<mDofMap.size(); ++i)
  {
    mDofMap[i] = -1;
    for(size_t j=0; j<mDofs.size(); ++j)
    {
      if(mDofs[j] == entityDependencies[i])
      {
        mDofMap[i] = j;
      }
    }
  }
}

//==============================================================================
template <class JacobianEntity>
const std::vector<size_t>& InverseKinematics<JacobianEntity>::getDofs() const
{
  return mDofs;
}

//==============================================================================
template <class JacobianEntity>
const std::vector<int>& InverseKinematics<JacobianEntity>::getDofMap() const
{
  return mDofMap;
}

//==============================================================================
template <class JacobianEntity>
template <class IKErrorMethod>
IKErrorMethod* InverseKinematics<JacobianEntity>::setErrorMethod()
{
  IKErrorMethod* newMethod = new IKErrorMethod(this);
  mErrorMethod = std::move(std::unique_ptr<IKErrorMethod>(newMethod));
  return newMethod;
}

//==============================================================================
template <class JacobianEntity>
typename InverseKinematics<JacobianEntity>::ErrorMethod*
InverseKinematics<JacobianEntity>::getErrorMethod()
{
  return mErrorMethod.get();
}

//==============================================================================
template <class JacobianEntity>
const typename InverseKinematics<JacobianEntity>::ErrorMethod*
InverseKinematics<JacobianEntity>::getErrorMethod() const
{
  return mErrorMethod.get();
}

//==============================================================================
template <class JacobianEntity>
template <class IKGradientMethod>
IKGradientMethod* InverseKinematics<JacobianEntity>::setGradientMethod()
{
  IKGradientMethod* newMethod = new IKGradientMethod(this);
  mGradientMethod = std::move(std::unique_ptr<IKGradientMethod>(newMethod));
  return newMethod;
}

//==============================================================================
template <class JacobianEntity>
typename InverseKinematics<JacobianEntity>::GradientMethod*
InverseKinematics<JacobianEntity>::getGradientMethod()
{
  return mGradientMethod.get();
}

//==============================================================================
template <class JacobianEntity>
const typename InverseKinematics<JacobianEntity>::GradientMethod*
InverseKinematics<JacobianEntity>::getGradientMethod() const
{
  return mGradientMethod.get();
}

//==============================================================================
template <class JacobianEntity>
void InverseKinematics<JacobianEntity>::setSolver(
    std::shared_ptr<optimizer::Solver> _newSolver)
{
  mSolver = _newSolver;
}

//==============================================================================
template <class JacobianEntity>
std::shared_ptr<optimizer::Solver>
InverseKinematics<JacobianEntity>::getSolver()
{
  return mSolver;
}

//==============================================================================
template <class JacobianEntity>
std::shared_ptr<const optimizer::Solver>
InverseKinematics<JacobianEntity>::getSolver() const
{
  return mSolver;
}

//==============================================================================
template <class JacobianEntity>
void InverseKinematics<JacobianEntity>::setTarget(
    std::shared_ptr<SimpleFrame> _newTarget)
{
  if(nullptr == _newTarget)
  {
    _newTarget = SimpleFramePtr(
          new SimpleFrame(Frame::World(), mEntity->getName()+"_target",
                          mEntity->getWorldTransform()));
  }

  mTarget = _newTarget;
  resetTargetConnection();
}

//==============================================================================
template <class JacobianEntity>
std::shared_ptr<SimpleFrame>
InverseKinematics<JacobianEntity>::getTarget()
{
  return mTarget;
}

//==============================================================================
template <class JacobianEntity>
std::shared_ptr<const SimpleFrame>
InverseKinematics<JacobianEntity>::getTarget() const
{
  return mTarget;
}

//==============================================================================
template <class JacobianEntity>
JacobianEntity* InverseKinematics<JacobianEntity>::getEntity()
{
  return mEntity;
}

//==============================================================================
template <class JacobianEntity>
const JacobianEntity* InverseKinematics<JacobianEntity>::getEntity() const
{
  return mEntity;
}

//==============================================================================
template <class JacobianEntity>
const math::Jacobian& InverseKinematics<JacobianEntity>::computeJacobian() const
{
  const math::Jacobian& fullJacobian =
      this->mIK->getEntity()->getWorldJacobian();
  mJacobian.resize(6, this->mIK->getDofs().size());

  for(int i=0; i<this->mIK->getDofMap().size(); ++i)
  {
    int j = this->mIK->getDofMap()[i];
    if(j >= 0)
      mJacobian.block<6,1>(0,i) = fullJacobian.block<6,1>(0,j);
  }

  return mJacobian;
}

//==============================================================================
template <class JacobianEntity>
void InverseKinematics<JacobianEntity>::clearCaches()
{
  mErrorMethod->clearCaches();
  mGradientMethod->clearCaches();
}

//==============================================================================
template <class JacobianEntity>
InverseKinematics<JacobianEntity>::~InverseKinematics()
{
  mTargetConnection.disconnect();
  mEntityConnection.disconnect();
}

//==============================================================================
template <class JacobianEntity>
InverseKinematics<JacobianEntity>::InverseKinematics(JacobianEntity* _entity)
  : mActive(false),
    mHierarchyLevel(0),
    mEntity(_entity)
{
  // Create the default target for this IK module
  setTarget(nullptr);

  // The default error method is the one based on Task Space Regions
  setErrorMethod<EulerAngleXYZMethod>();

  // The default gradient method is damped least squares Jacobian
  setGradientMethod<JacobianDLS>();

  // Set up the cache clearing connections. These connections will ensure that
  // any time an outsider alters the transform of the object's transform or the
  // target's transform, our saved caches will be cleared so that the error and
  // gradient will be recomputed the next time they are polled.
  resetTargetConnection();
  resetEntityConnection();

  // By default, we use the linkage when performing IK
  useLinkage();

  // TODO(MXG): Instantiate mSolver using a default native solver
  // TODO(MXG): Write a default native solver
}

//==============================================================================
template <class JacobianEntity>
void InverseKinematics<JacobianEntity>::resetTargetConnection()
{
  mTargetConnection.disconnect();
  mTargetConnection = mTarget->onTransformUpdated.connect(
        [=](const Entity*)
        { this->clearCaches(); } );
  clearCaches();
}

//==============================================================================
template <class JacobianEntity>
void InverseKinematics<JacobianEntity>::resetEntityConnection()
{
  mEntityConnection.disconnect();
  mEntityConnection = mEntity->onTransformUpdated.connect(
        [=](const Entity*)
        { this->clearCaches(); } );
  clearCaches();
}

} // namespace dynamics
} // namespace dart

#endif // DART_DYNAMICS_INVERSEKINEMATICS_H_
