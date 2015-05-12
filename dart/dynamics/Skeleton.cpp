/*
 * Copyright (c) 2011-2015, Georgia Tech Research Corporation
 * All rights reserved.
 *
 * Author(s): Sehoon Ha <sehoon.ha@gmail.com>,
 *            Jeongseok Lee <jslee02@gmail.com>
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

#include "dart/dynamics/Skeleton.h"

#include <algorithm>
#include <queue>
#include <string>
#include <vector>

#include "dart/common/Console.h"
#include "dart/math/Geometry.h"
#include "dart/math/Helpers.h"
#include "dart/dynamics/BodyNode.h"
#include "dart/dynamics/DegreeOfFreedom.h"
#include "dart/dynamics/Joint.h"
#include "dart/dynamics/Marker.h"
#include "dart/dynamics/PointMass.h"
#include "dart/dynamics/SoftBodyNode.h"

namespace dart {
namespace dynamics {

//==============================================================================
static bool checkIndexArrayValidity(const Skeleton* skel,
                                    const std::vector<size_t>& _indices,
                                    const std::string& _fname)
{
  size_t dofs = skel->getNumDofs();
  for(size_t i=0; i<_indices.size(); ++i)
  {
    if(_indices[i] >= dofs)
    {
      if(dofs > 0)
      {
        dterr << "[Skeleton::" << _fname << "] Invalid entry (" << i << ") in "
              << "_indices array: " << _indices[i] << ". Value must be less than "
              << dofs << " for the Skeleton named [" << skel->getName() << "] ("
              << skel << ")\n";
      }
      else
      {
        dterr << "[Skeleton::" << _fname << "] The Skeleton named ["
              << skel->getName() << "] (" << skel << ") is empty, but _indices "
              << "has entries in it. Nothing will be set!\n";
      }

      return false;
    }
  }
  return true;
}

//==============================================================================
static bool checkIndexArrayAgreement(const Skeleton* skel,
                                     const std::vector<size_t>& _indices,
                                     const Eigen::VectorXd& _values,
                                     const std::string& _fname,
                                     const std::string& _vname)
{
  if( static_cast<int>(_indices.size()) != _values.size() )
  {
    dterr << "[Skeleton::" << _fname << "] Mismatch between _indices size ("
          << _indices.size() << ") and " << _vname << " size ("
          << _values.size() << ") for Skeleton named [" << skel->getName()
          << "] (" << skel << "). Nothing will be set!\n";
    assert(false);
    return false;
  }

  return checkIndexArrayValidity(skel, _indices, _fname);
}

//==============================================================================
Skeleton::Properties::Properties(
    const std::string& _name,
    bool _isMobile,
    const Eigen::Vector3d& _gravity,
    double _timeStep,
    bool _enabledSelfCollisionCheck,
    bool _enableAdjacentBodyCheck)
  : mName(_name),
    mIsMobile(_isMobile),
    mGravity(_gravity),
    mTimeStep(_timeStep),
    mEnabledSelfCollisionCheck(_enabledSelfCollisionCheck),
    mEnabledAdjacentBodyCheck(_enableAdjacentBodyCheck)
{
  // Do nothing
}

//==============================================================================
std::shared_ptr<Skeleton> Skeleton::create(const std::string& _name)
{
  return create(Properties(_name));
}

//==============================================================================
std::shared_ptr<Skeleton> Skeleton::create(const Properties& _properties)
{
  std::shared_ptr<Skeleton> skel(new Skeleton(_properties));
  skel->setPtr(skel);
  return skel;
}

//==============================================================================
std::shared_ptr<Skeleton> Skeleton::getPtr()
{
  return mPtr.lock();
}

//==============================================================================
std::shared_ptr<const Skeleton> Skeleton::getPtr() const
{
  return mPtr.lock();
}

//==============================================================================
Skeleton::~Skeleton()
{
  for (BodyNode* bn : mBodyNodes)
  {
    if(bn->getParentJoint())
      delete bn->getParentJoint();

    delete bn;
  }
}

//==============================================================================
SkeletonPtr Skeleton::clone() const
{
  SkeletonPtr skelClone = Skeleton::create(getName());

  for(size_t i=0; i<getNumBodyNodes(); ++i)
  {
    // Create a clone of the parent Joint
    Joint* joint = getJoint(i)->clone();

    // Identify the original parent BodyNode
    const BodyNode* originalParent = getBodyNode(i)->getParentBodyNode();

    // Grab the parent BodyNode clone (using its name, which is guaranteed to be
    // unique), or use nullptr if this is a root BodyNode
    BodyNode* parentClone = originalParent == nullptr? nullptr :
          skelClone->getBodyNode(originalParent->getName());

    if( (nullptr != originalParent) && (nullptr == parentClone) )
    {
      dterr << "[Skeleton::clone] Failed to find a clone of BodyNode named ["
            << originalParent->getName() << "] which is needed as the parent "
            << "of the BodyNode named [" << getBodyNode(i)->getName()
            << "] and should already have been created. Please report this as "
            << "a bug!\n";
    }

    skelClone->registerBodyNode(getBodyNode(i)->clone(parentClone, joint));
  }

  skelClone->setProperties(getSkeletonProperties());

  return skelClone;
}

//==============================================================================
void Skeleton::setProperties(const Properties& _properties)
{
  setName(_properties.mName);
  setMobile(_properties.mIsMobile);
  setGravity(_properties.mGravity);
  setTimeStep(_properties.mTimeStep);

  if(_properties.mEnabledSelfCollisionCheck)
    enableSelfCollision(_properties.mEnabledAdjacentBodyCheck);
  else
    disableSelfCollision();
}

//==============================================================================
const Skeleton::Properties& Skeleton::getSkeletonProperties() const
{
  return mSkeletonP;
}

//==============================================================================
const std::string& Skeleton::setName(const std::string& _name)
{
  if(_name == mSkeletonP.mName)
    return mSkeletonP.mName;

  const std::string oldName = mSkeletonP.mName;
  mSkeletonP.mName = _name;

  mNameMgrForBodyNodes.setManagerName(
        "Skeleton::BodyNode | "+mSkeletonP.mName);
  mNameMgrForSoftBodyNodes.setManagerName(
        "Skeleton::SoftBodyNode | "+mSkeletonP.mName);
  mNameMgrForJoints.setManagerName(
        "Skeleton::Joint | "+mSkeletonP.mName);
  mNameMgrForDofs.setManagerName(
        "Skeleton::DegreeOfFreedom | "+mSkeletonP.mName);
  mNameMgrForMarkers.setManagerName(
        "Skeleton::Marker | "+mSkeletonP.mName);

  mNameChangedSignal.raise(mPtr.lock(), oldName, mSkeletonP.mName);

  return mSkeletonP.mName;
}

//==============================================================================
const std::string& Skeleton::getName() const
{
  return mSkeletonP.mName;
}

//==============================================================================
const std::string& Skeleton::addEntryToBodyNodeNameMgr(BodyNode* _newNode)
{
  _newNode->mEntityP.mName =
      mNameMgrForBodyNodes.issueNewNameAndAdd(_newNode->getName(), _newNode);

  return _newNode->mEntityP.mName;
}

//==============================================================================
const std::string& Skeleton::addEntryToJointNameMgr(Joint* _newJoint)
{
  _newJoint->mJointP.mName =
      mNameMgrForJoints.issueNewNameAndAdd(_newJoint->getName(), _newJoint);
  _newJoint->updateDegreeOfFreedomNames();

  return _newJoint->mJointP.mName;
}

//==============================================================================
void Skeleton::addEntryToSoftBodyNodeNameMgr(SoftBodyNode* _newNode)
{
  // Note: This doesn't need the same checks as BodyNode and Joint, because
  // its name has already been resolved against all the BodyNodes, which includes
  // all SoftBodyNodes.
  mNameMgrForSoftBodyNodes.addName(_newNode->getName(), _newNode);
}

//==============================================================================
void Skeleton::addMarkersOfBodyNode(BodyNode* _node)
{
  for (size_t i=0; i<_node->getNumMarkers(); ++i)
    addEntryToMarkerNameMgr(_node->getMarker(i));
}

//==============================================================================
void Skeleton::removeMarkersOfBodyNode(BodyNode* _node)
{
  for (size_t i=0; i<_node->getNumMarkers(); ++i)
    mNameMgrForMarkers.removeName(_node->getMarker(i)->getName());
}

//==============================================================================
const std::string& Skeleton::addEntryToMarkerNameMgr(Marker* _newMarker)
{
  _newMarker->mProperties.mName = mNameMgrForMarkers.issueNewNameAndAdd(
      _newMarker->getName(), _newMarker);
  return _newMarker->mProperties.mName;
}

//==============================================================================
void Skeleton::enableSelfCollision(bool _enableAdjecentBodyCheck)
{
  mSkeletonP.mEnabledSelfCollisionCheck = true;
  mSkeletonP.mEnabledAdjacentBodyCheck = _enableAdjecentBodyCheck;
}

//==============================================================================
void Skeleton::disableSelfCollision()
{
  mSkeletonP.mEnabledSelfCollisionCheck = false;
  mSkeletonP.mEnabledAdjacentBodyCheck = false;
}

//==============================================================================
bool Skeleton::isEnabledSelfCollisionCheck() const
{
  return mSkeletonP.mEnabledSelfCollisionCheck;
}

//==============================================================================
bool Skeleton::isEnabledAdjacentBodyCheck() const
{
  return mSkeletonP.mEnabledAdjacentBodyCheck;
}

//==============================================================================
void Skeleton::setMobile(bool _isMobile)
{
  mSkeletonP.mIsMobile = _isMobile;
}

//==============================================================================
bool Skeleton::isMobile() const
{
  return mSkeletonP.mIsMobile;
}

//==============================================================================
void Skeleton::setTimeStep(double _timeStep)
{
  assert(_timeStep > 0.0);
  mSkeletonP.mTimeStep = _timeStep;
  notifyArticulatedInertiaUpdate();
}

//==============================================================================
double Skeleton::getTimeStep() const
{
  return mSkeletonP.mTimeStep;
}

//==============================================================================
void Skeleton::setGravity(const Eigen::Vector3d& _gravity)
{
  mSkeletonP.mGravity = _gravity;
  mIsGravityForcesDirty = true;
  mIsCoriolisAndGravityForcesDirty = true;
}

//==============================================================================
const Eigen::Vector3d& Skeleton::getGravity() const
{
  return mSkeletonP.mGravity;
}

//==============================================================================
void Skeleton::addBodyNode(BodyNode* _body)
{
  assert(_body && _body->getParentJoint());

  mBodyNodes.push_back(_body);
  addEntryToBodyNodeNameMgr(_body);
  addMarkersOfBodyNode(_body);
  _body->mSkeleton = mPtr;
  registerJoint(_body->getParentJoint());

  SoftBodyNode* softBodyNode = dynamic_cast<SoftBodyNode*>(_body);
  if (softBodyNode)
  {
    mSoftBodyNodes.push_back(softBodyNode);
    addEntryToSoftBodyNodeNameMgr(softBodyNode);
  }
}

//==============================================================================
size_t Skeleton::getNumBodyNodes() const
{
  return mBodyNodes.size();
}

//==============================================================================
size_t Skeleton::getNumRigidBodyNodes() const
{
  return mBodyNodes.size() - mSoftBodyNodes.size();
}

//==============================================================================
size_t Skeleton::getNumSoftBodyNodes() const
{
  return mSoftBodyNodes.size();
}

//==============================================================================
template<typename T>
static T getVectorObjectIfAvailable(size_t _idx, const std::vector<T>& _vec)
{
  // TODO: Should we have an out-of-bounds assertion or throw here?
  if (_idx < _vec.size())
    return _vec[_idx];

  return nullptr;
}

//==============================================================================
BodyNode* Skeleton::getRootBodyNode(size_t _treeIdx)
{
  return getVectorObjectIfAvailable(_treeIdx, mRootBodyNodes);
}

//==============================================================================
const BodyNode* Skeleton::getRootBodyNode(size_t _treeIdx) const
{
  return getVectorObjectIfAvailable(_treeIdx, mRootBodyNodes);
}

//==============================================================================
BodyNode* Skeleton::getBodyNode(size_t _idx)
{
  return getVectorObjectIfAvailable<BodyNode*>(_idx, mBodyNodes);
}

//==============================================================================
const BodyNode* Skeleton::getBodyNode(size_t _idx) const
{
  return getVectorObjectIfAvailable<BodyNode*>(_idx, mBodyNodes);
}

//==============================================================================
SoftBodyNode* Skeleton::getSoftBodyNode(size_t _idx)
{
  return getVectorObjectIfAvailable<SoftBodyNode*>(_idx, mSoftBodyNodes);
}

//==============================================================================
const SoftBodyNode* Skeleton::getSoftBodyNode(size_t _idx) const
{
  return getVectorObjectIfAvailable<SoftBodyNode*>(_idx, mSoftBodyNodes);
}

//==============================================================================
BodyNode* Skeleton::getBodyNode(const std::string& _name)
{
  return mNameMgrForBodyNodes.getObject(_name);
}

//==============================================================================
const BodyNode* Skeleton::getBodyNode(const std::string& _name) const
{
  return mNameMgrForBodyNodes.getObject(_name);
}

//==============================================================================
SoftBodyNode* Skeleton::getSoftBodyNode(const std::string& _name)
{
  return mNameMgrForSoftBodyNodes.getObject(_name);
}

const SoftBodyNode* Skeleton::getSoftBodyNode(const std::string& _name) const
{
  return mNameMgrForSoftBodyNodes.getObject(_name);
}

//==============================================================================
size_t Skeleton::getNumJoints() const
{
  // The number of joints and body nodes are identical
  return getNumBodyNodes();
}

//==============================================================================
Joint* Skeleton::getJoint(size_t _idx)
{
  BodyNode* bn = getVectorObjectIfAvailable<BodyNode*>(_idx, mBodyNodes);
  if (bn)
    return bn->getParentJoint();

  return nullptr;
}

//==============================================================================
const Joint* Skeleton::getJoint(size_t _idx) const
{
  return const_cast<Skeleton*>(this)->getJoint(_idx);
}

//==============================================================================
Joint* Skeleton::getJoint(const std::string& _name)
{
  return mNameMgrForJoints.getObject(_name);
}

//==============================================================================
const Joint* Skeleton::getJoint(const std::string& _name) const
{
  return mNameMgrForJoints.getObject(_name);
}

//==============================================================================
size_t Skeleton::getNumDofs() const
{
  return mDofs.size();
}

//==============================================================================
DegreeOfFreedom* Skeleton::getDof(size_t _idx)
{
  return getVectorObjectIfAvailable<DegreeOfFreedom*>(_idx, mDofs);
}

//==============================================================================
const DegreeOfFreedom* Skeleton::getDof(size_t _idx) const
{
  return getVectorObjectIfAvailable<DegreeOfFreedom*>(_idx, mDofs);
}

//==============================================================================
DegreeOfFreedom* Skeleton::getDof(const std::string& _name)
{
  return mNameMgrForDofs.getObject(_name);
}

//==============================================================================
const DegreeOfFreedom* Skeleton::getDof(const std::string& _name) const
{
  return mNameMgrForDofs.getObject(_name);
}

//==============================================================================
Marker* Skeleton::getMarker(const std::string& _name)
{
  return mNameMgrForMarkers.getObject(_name);
}

//==============================================================================
const Marker* Skeleton::getMarker(const std::string& _name) const
{
  return const_cast<Skeleton*>(this)->getMarker(_name);
}

//==============================================================================
void Skeleton::init(double _timeStep, const Eigen::Vector3d& _gravity)
{
  // Set timestep and gravity
  setTimeStep(_timeStep);
  setGravity(_gravity);

  // Get root bodynodes that don't have parent bodynode
  std::vector<BodyNode*> rootBodyNodes;
  for (size_t i = 0; i < mBodyNodes.size(); ++i)
  {
    if (mBodyNodes[i]->getParentBodyNode() == nullptr)
      rootBodyNodes.push_back(mBodyNodes[i]);
  }

  // Rearrange the list of body nodes with BFS (Breadth First Search)
  std::queue<BodyNode*> queue;
  mBodyNodes.clear();
  mNameMgrForBodyNodes.clear();
  mNameMgrForJoints.clear();
  mSoftBodyNodes.clear();
  mNameMgrForSoftBodyNodes.clear();
  mDofs.clear();
  mNameMgrForDofs.clear();
  mNameMgrForMarkers.clear();
  for (size_t i = 0; i < rootBodyNodes.size(); ++i)
  {
    queue.push(rootBodyNodes[i]);

    while (!queue.empty())
    {
      BodyNode* itBodyNode = queue.front();
      queue.pop();
      registerBodyNode(itBodyNode);
      for (size_t j = 0; j < itBodyNode->getNumChildBodyNodes(); ++j)
        queue.push(itBodyNode->getChildBodyNode(j));
    }
  }

  ///////////////////////////////////////////////////////////////////////////

  // Set dimension of dynamics quantities
  updateCacheDimensions();

  // Clear external/internal force
  clearExternalForces();
  resetForces();

  // Calculate mass
  updateTotalMass();
}

//==============================================================================
size_t Skeleton::getDof() const
{
  return getNumDofs();
}

//==============================================================================
GenCoordInfo Skeleton::getGenCoordInfo(size_t _index) const
{
  assert(_index < getNumDofs());

  return mGenCoordInfos[_index];
}

//==============================================================================
template <void (DegreeOfFreedom::*setValue)(double _value)>
static void setValuesFromVector(Skeleton* skel,
                                const std::vector<size_t>& _indices,
                                const Eigen::VectorXd& _values,
                                const std::string& _fname,
                                const std::string& _vname)
{
  if(!checkIndexArrayAgreement(skel, _indices, _values, _fname, _vname))
    return;

  for (size_t i=0; i<_indices.size(); ++i)
  {
    (skel->getDof(_indices[i])->*setValue)(_values[i]);
  }
}

//==============================================================================
template <void (DegreeOfFreedom::*setValue)(double _value)>
static void setAllValuesFromVector(Skeleton* skel,
                                   const Eigen::VectorXd& _values,
                                   const std::string& _fname,
                                   const std::string& _vname)
{
  size_t nDofs = skel->getNumDofs();
  if( _values.size() != static_cast<int>(skel->getNumDofs()) )
  {
    dterr << "[Skeleton::" << _fname << "] Invalid number of entries ("
          << _values.size() << ") in " << _vname << " for Skeleton named ["
          << skel->getName() << "] (" << skel << ") . Must be equal to ("
          << skel->getNumDofs() << "). Nothing will be set!\n";
    assert(false);
    return;
  }

  for(size_t i=0; i < nDofs; ++i)
  {
    (skel->getDof(i)->*setValue)(_values[i]);
  }
}

//==============================================================================
template <double (DegreeOfFreedom::*getValue)() const>
static Eigen::VectorXd getValuesFromVector(
    const Skeleton* skel, const std::vector<size_t>& _indices,
    const std::string& _fname)
{
  Eigen::VectorXd q(_indices.size());

  for(size_t i=0; i<_indices.size(); ++i)
  {
    const DegreeOfFreedom* dof = skel->getDof(_indices[i]);
    if(dof)
      q[i] = (dof->*getValue)();
    else
    {
      q[i] = 0.0;
      dterr << "[Skeleton::" << _fname << "] Requesting invalid index ("
            << _indices[i] << ") for Skeleton named [" << skel->getName()
            << "] (" << skel << "). Setting value to zero.\n";
      assert(false);
    }
  }

  return q;
}

//==============================================================================
template <double (DegreeOfFreedom::*getValue)() const>
static Eigen::VectorXd getValuesFromAllDofs(
    const Skeleton* skel)
{
  size_t nDofs = skel->getNumDofs();
  Eigen::VectorXd values(nDofs);

  for(size_t i=0; i<nDofs; ++i)
    values[i] = (skel->getDof(i)->*getValue)();

  return values;
}

//==============================================================================
template <void (DegreeOfFreedom::*apply)()>
static void applyToAllDofs(Skeleton* skel)
{
  size_t nDofs = skel->getNumDofs();
  for(size_t i=0; i<nDofs; ++i)
    (skel->getDof(i)->*apply)();
}

//==============================================================================
template <void (DegreeOfFreedom::*setValue)(double _value)>
static void setValueFromIndex(Skeleton* skel, size_t _index, double _value,
                              const std::string& _fname)
{
  if(_index >= skel->getNumDofs())
  {
    if(skel->getNumDofs() > 0)
      dterr << "[Skeleton::" << _fname << "] Out of bounds index (" << _index
            << ") for Skeleton named [" << skel->getName() << "] (" << skel
            << "). Must be less than " << skel->getNumDofs() << "!\n";
    else
      dterr << "[Skeleton::" << _fname << "] Index (" << _index << ") cannot "
            << "be used on Skeleton [" << skel->getName() << "] (" << skel
            << ") because it is empty!\n";
    assert(false);
    return;
  }

  (skel->getDof(_index)->*setValue)(_value);
}

//==============================================================================
template <double (DegreeOfFreedom::*getValue)() const>
static double getValueFromIndex(const Skeleton* skel, size_t _index,
                                const std::string& _fname)
{
  if(_index >= skel->getNumDofs())
  {
    if(skel->getNumDofs() > 0)
      dterr << "[Skeleton::" << _fname << "] Out of bounds index (" << _index
            << ") for Skeleton named [" << skel->getName() << "] (" << skel
            << "). Must be less than " << skel->getNumDofs() << "!\n";
    else
      dterr << "[Skeleton::" << _fname << "] Index (" << _index << ") cannot "
            << "be requested for Skeleton [" << skel->getName() << "] (" << skel
            << ") because it is empty!\n";
    assert(false);
    return 0;
  }

  return (skel->getDof(_index)->*getValue)();
}

//==============================================================================
void Skeleton::setCommand(size_t _index, double _command)
{
  setValueFromIndex<&DegreeOfFreedom::setCommand>(
        this, _index, _command, "setCommand");
}

//==============================================================================
double Skeleton::getCommand(size_t _index) const
{
  return getValueFromIndex<&DegreeOfFreedom::getCommand>(
        this, _index, "getCommand");
}

//==============================================================================
void Skeleton::setCommands(const Eigen::VectorXd& _commands)
{
  setAllValuesFromVector<&DegreeOfFreedom::setCommand>(
        this, _commands, "setCommands", "_commands");
}

//==============================================================================
void Skeleton::setCommands(const std::vector<size_t>& _indices,
                           const Eigen::VectorXd& _commands)
{
  setValuesFromVector<&DegreeOfFreedom::setCommand>(
        this, _indices, _commands, "setCommands", "_commands");
}

//==============================================================================
Eigen::VectorXd Skeleton::getCommands() const
{
  return getValuesFromAllDofs<&DegreeOfFreedom::getCommand>(this);
}

//==============================================================================
Eigen::VectorXd Skeleton::getCommands(const std::vector<size_t>& _indices) const
{
  return getValuesFromVector<&DegreeOfFreedom::getCommand>(
        this, _indices, "getCommands");
}

//==============================================================================
void Skeleton::resetCommands()
{
  applyToAllDofs<&DegreeOfFreedom::resetCommand>(this);
}

//==============================================================================
void Skeleton::setPosition(size_t _index, double _position)
{
  setValueFromIndex<&DegreeOfFreedom::setPosition>(
        this, _index, _position, "setPosition");
}

//==============================================================================
double Skeleton::getPosition(size_t _index) const
{
  return getValueFromIndex<&DegreeOfFreedom::getPosition>(
        this, _index, "getPosition");
}

//==============================================================================
void Skeleton::setPositions(const Eigen::VectorXd& _positions)
{
  setAllValuesFromVector<&DegreeOfFreedom::setPosition>(
        this, _positions, "setPositions", "_positions");
}

//==============================================================================
void Skeleton::setPositions(const std::vector<size_t>& _indices,
                            const Eigen::VectorXd& _positions)
{
  setValuesFromVector<&DegreeOfFreedom::setPosition>(
        this, _indices, _positions, "setPositions", "_positions");
}

//==============================================================================
Eigen::VectorXd Skeleton::getPositions() const
{
  return getValuesFromAllDofs<&DegreeOfFreedom::getPosition>(this);
}

//==============================================================================
Eigen::VectorXd Skeleton::getPositions(const std::vector<size_t>& _indices) const
{
  return getValuesFromVector<&DegreeOfFreedom::getPosition>(
        this, _indices, "getPositions");
}

//==============================================================================
Eigen::VectorXd Skeleton::getPositionSegment(
    const std::vector<size_t>& _indices) const
{
  return getValuesFromVector<&DegreeOfFreedom::getPosition>(
        this, _indices, "getPositionSegment");
}

//==============================================================================
void Skeleton::setPositionSegment(const std::vector<size_t>& _indices,
                                  const Eigen::VectorXd& _positions)
{
  setPositions(_indices, _positions);
}

//==============================================================================
void Skeleton::resetPositions()
{
  applyToAllDofs<&DegreeOfFreedom::resetPosition>(this);
}

//==============================================================================
void Skeleton::setPositionLowerLimit(size_t _index, double _position)
{
  setValueFromIndex<&DegreeOfFreedom::setPositionLowerLimit>(
        this, _index, _position, "setPositionLowerLimit");
}

//==============================================================================
double Skeleton::getPositionLowerLimit(size_t _index) const
{
  return getValueFromIndex<&DegreeOfFreedom::getPositionLowerLimit>(
        this, _index, "getPositionLowerLimit");
}

//==============================================================================
void Skeleton::setPositionUpperLimit(size_t _index, double _position)
{
  setValueFromIndex<&DegreeOfFreedom::setPositionUpperLimit>(
        this, _index, _position, "setPositionUpperLimit");
}

//==============================================================================
double Skeleton::getPositionUpperLimit(size_t _index) const
{
  return getValueFromIndex<&DegreeOfFreedom::getPositionUpperLimit>(
        this, _index, "getPositionUpperLimit");
}

//==============================================================================
void Skeleton::setVelocity(size_t _index, double _velocity)
{
  setValueFromIndex<&DegreeOfFreedom::setVelocity>(
        this, _index, _velocity, "setVelocity");
}

//==============================================================================
double Skeleton::getVelocity(size_t _index) const
{
  return getValueFromIndex<&DegreeOfFreedom::getVelocity>(
        this, _index, "getVelocity");
}

//==============================================================================
void Skeleton::setVelocities(const Eigen::VectorXd& _velocities)
{
  setAllValuesFromVector<&DegreeOfFreedom::setVelocity>(
        this, _velocities, "setVelocities", "_velocities");
}

//==============================================================================
void Skeleton::setVelocities(const std::vector<size_t>& _indices,
                             const Eigen::VectorXd& _velocities)
{
  setValuesFromVector<&DegreeOfFreedom::setVelocity>(
        this, _indices, _velocities, "setVelocities", "_velocities");
}

//==============================================================================
void Skeleton::setVelocitySegment(const std::vector<size_t>& _indices,
                                  const Eigen::VectorXd& _velocities)
{
  setVelocities(_indices, _velocities);
}

//==============================================================================
Eigen::VectorXd Skeleton::getVelocities() const
{
  return getValuesFromAllDofs<&DegreeOfFreedom::getVelocity>(this);
}

//==============================================================================
Eigen::VectorXd Skeleton::getVelocities(const std::vector<size_t>& _indices) const
{
  return getValuesFromVector<&DegreeOfFreedom::getVelocity>(
        this, _indices, "getVelocities");
}

//==============================================================================
Eigen::VectorXd Skeleton::getVelocitySegment(const std::vector<size_t>& _id) const
{
  return getVelocities(_id);
}

//==============================================================================
void Skeleton::resetVelocities()
{
  applyToAllDofs<&DegreeOfFreedom::resetVelocity>(this);
}

//==============================================================================
void Skeleton::setVelocityLowerLimit(size_t _index, double _velocity)
{
  setValueFromIndex<&DegreeOfFreedom::setVelocityLowerLimit>(
        this, _index, _velocity, "setVelocityLowerLimit");
}

//==============================================================================
double Skeleton::getVelocityLowerLimit(size_t _index)
{
  return getValueFromIndex<&DegreeOfFreedom::getVelocityLowerLimit>(
        this, _index, "getVelocityLowerLimit");
}

//==============================================================================
void Skeleton::setVelocityUpperLimit(size_t _index, double _velocity)
{
  setValueFromIndex<&DegreeOfFreedom::setVelocityUpperLimit>(
        this, _index, _velocity, "setVelocityUpperLimit");
}

//==============================================================================
double Skeleton::getVelocityUpperLimit(size_t _index)
{
  return getValueFromIndex<&DegreeOfFreedom::getVelocityUpperLimit>(
        this, _index, "getVelocityUpperLimit");
}

//==============================================================================
void Skeleton::setAcceleration(size_t _index, double _acceleration)
{
  setValueFromIndex<&DegreeOfFreedom::setAcceleration>(
        this, _index, _acceleration, "setAcceleration");
}

//==============================================================================
double Skeleton::getAcceleration(size_t _index) const
{
  return getValueFromIndex<&DegreeOfFreedom::getAcceleration>(
        this, _index, "getAcceleration");
}

//==============================================================================
void Skeleton::setAccelerations(const Eigen::VectorXd& _accelerations)
{
  setAllValuesFromVector<&DegreeOfFreedom::setAcceleration>(
        this, _accelerations, "setAccelerations", "_accelerations");
}

//==============================================================================
void Skeleton::setAccelerations(const std::vector<size_t>& _indices,
                                const Eigen::VectorXd& _accelerations)
{
  setValuesFromVector<&DegreeOfFreedom::setAcceleration>(
        this, _indices, _accelerations, "setAccelerations", "_accelerations");
}

//==============================================================================
void Skeleton::setAccelerationSegment(const std::vector<size_t>& _indices,
                                      const Eigen::VectorXd& _accelerations)
{
  setAccelerations(_indices, _accelerations);
}

//==============================================================================
Eigen::VectorXd Skeleton::getAccelerations() const
{
  return getValuesFromAllDofs<&DegreeOfFreedom::getAcceleration>(this);
}

//==============================================================================
Eigen::VectorXd Skeleton::getAccelerations(
    const std::vector<size_t>& _indices) const
{
  return getValuesFromVector<&DegreeOfFreedom::getAcceleration>(
        this, _indices, "getAccelerations");
}

//==============================================================================
Eigen::VectorXd Skeleton::getAccelerationSegment(
    const std::vector<size_t>& _indices) const
{
  return getAccelerations(_indices);
}

//==============================================================================
void Skeleton::resetAccelerations()
{
  applyToAllDofs<&DegreeOfFreedom::resetAcceleration>(this);
}

//==============================================================================
void Skeleton::setAccelerationLowerLimit(size_t _index, double _acceleration)
{
  setValueFromIndex<&DegreeOfFreedom::setAccelerationLowerLimit>(
        this, _index, _acceleration, "setAccelerationLowerLimit");
}

//==============================================================================
double Skeleton::getAccelerationLowerLimit(size_t _index) const
{
  return getValueFromIndex<&DegreeOfFreedom::getAccelerationLowerLimit>(
        this, _index, "getAccelerationLowerLimit");
}

//==============================================================================
void Skeleton::setAccelerationUpperLimit(size_t _index, double _acceleration)
{
  setValueFromIndex<&DegreeOfFreedom::setAccelerationUpperLimit>(
        this, _index, _acceleration, "setAccelerationUpperLimit");
}

//==============================================================================
double Skeleton::getAccelerationUpperLimit(size_t _index) const
{
  return getValueFromIndex<&DegreeOfFreedom::getAccelerationUpperLimit>(
        this, _index, "getAccelerationUpperLimit");
}

//==============================================================================
void Skeleton::setForce(size_t _index, double _force)
{
  setValueFromIndex<&DegreeOfFreedom::setForce>(
        this, _index, _force, "setForce");
}

//==============================================================================
double Skeleton::getForce(size_t _index) const
{
  return getValueFromIndex<&DegreeOfFreedom::getForce>(
        this, _index, "getForce");
}

//==============================================================================
void Skeleton::setForces(const Eigen::VectorXd& _forces)
{
  setAllValuesFromVector<&DegreeOfFreedom::setForce>(
        this, _forces, "setForces", "_forces");
}

//==============================================================================
void Skeleton::setForces(const std::vector<size_t>& _indices,
                         const Eigen::VectorXd& _forces)
{
  setValuesFromVector<&DegreeOfFreedom::setForce>(
        this, _indices, _forces, "setForces", "_forces");
}

//==============================================================================
Eigen::VectorXd Skeleton::getForces() const
{
  return getValuesFromAllDofs<&DegreeOfFreedom::getForce>(this);
}

//==============================================================================
Eigen::VectorXd Skeleton::getForces(const std::vector<size_t>& _indices) const
{
  return getValuesFromVector<&DegreeOfFreedom::getForce>(
        this, _indices, "getForces");
}

//==============================================================================
void Skeleton::resetForces()
{
  applyToAllDofs<&DegreeOfFreedom::resetForce>(this);

  // TODO(JS): Find better place
  for (size_t i = 0; i < mSoftBodyNodes.size(); ++i)
  {
    for (size_t j = 0; j < mSoftBodyNodes[i]->getNumPointMasses(); ++j)
      mSoftBodyNodes[i]->getPointMass(j)->resetForces();
  }
}

//==============================================================================
void Skeleton::setForceLowerLimit(size_t _index, double _force)
{
  setValueFromIndex<&DegreeOfFreedom::setForceLowerLimit>(
        this, _index, _force, "setForceLowerLimit");
}

//==============================================================================
double Skeleton::getForceLowerLimit(size_t _index) const
{
  return getValueFromIndex<&DegreeOfFreedom::getForceLowerLimit>(
        this, _index, "getForceLowerLimit");
}

//==============================================================================
void Skeleton::setForceUpperLimit(size_t _index, double _force)
{
  setValueFromIndex<&DegreeOfFreedom::setForceUpperLimit>(
        this, _index, _force, "setForceUpperLimit");
}

//==============================================================================
double Skeleton::getForceUpperLimit(size_t _index) const
{
  return getValueFromIndex<&DegreeOfFreedom::getForceUpperLimit>(
        this, _index, "getForceUpperLimit");
}

//==============================================================================
Eigen::VectorXd Skeleton::getVelocityChanges() const
{
  const size_t dof = getNumDofs();
  Eigen::VectorXd velChange(dof);

  size_t index = 0;
  for (size_t i = 0; i < dof; ++i)
  {
    Joint* joint      = mDofs[i]->getJoint();
    size_t localIndex = mDofs[i]->getIndexInJoint();

    velChange[index++] = joint->getVelocityChange(localIndex);
  }

  assert(index == dof);

  return velChange;
}

//==============================================================================
void Skeleton::setConstraintImpulses(const Eigen::VectorXd& _impulses)
{
  setJointConstraintImpulses(_impulses);
}

//==============================================================================
void Skeleton::setJointConstraintImpulses(const Eigen::VectorXd& _impulses)
{
  const size_t dof = getNumDofs();

  size_t index = 0;
  for (size_t i = 0; i < dof; ++i)
  {
    Joint* joint      = mDofs[i]->getJoint();
    size_t localIndex = mDofs[i]->getIndexInJoint();

    joint->setConstraintImpulse(localIndex, _impulses[index++]);
  }

  assert(index == dof);
}

//==============================================================================
Eigen::VectorXd Skeleton::getConstraintImpulses() const
{
  return getJointConstraintImpulses();
}

//==============================================================================
Eigen::VectorXd Skeleton::getJointConstraintImpulses() const
{
  const size_t dof = getNumDofs();
  Eigen::VectorXd impulse(dof);

  size_t index = 0;
  for (size_t i = 0; i < dof; ++i)
  {
    Joint* joint      = mDofs[i]->getJoint();
    size_t localIndex = mDofs[i]->getIndexInJoint();

    impulse[index++] = joint->getConstraintImpulse(localIndex);
  }

  assert(index == dof);

  return impulse;
}

//==============================================================================
void Skeleton::setState(const Eigen::VectorXd& _state)
{
  assert(_state.size() % 2 == 0);

  size_t index = 0;
  size_t dof = 0;
  size_t halfSize = _state.size() / 2;
  Joint* joint;

  for (size_t i = 0; i < mBodyNodes.size(); ++i)
  {
    joint = mBodyNodes[i]->getParentJoint();

    dof = joint->getNumDofs();

    if (dof)
    {
      joint->setPositions(_state.segment(index, dof));
      joint->setVelocities(_state.segment(index + halfSize, dof));

      index += dof;
    }
  }
}

//==============================================================================
Eigen::VectorXd Skeleton::getState() const
{
  Eigen::VectorXd state(2 * getNumDofs());

  state << getPositions(), getVelocities();

  return state;
}

//==============================================================================
void Skeleton::integratePositions(double _dt)
{
  for (size_t i = 0; i < mBodyNodes.size(); ++i)
    mBodyNodes[i]->getParentJoint()->integratePositions(_dt);

  for (size_t i = 0; i < mSoftBodyNodes.size(); ++i)
  {
    for (size_t j = 0; j < mSoftBodyNodes[i]->getNumPointMasses(); ++j)
      mSoftBodyNodes[i]->getPointMass(j)->integratePositions(_dt);
  }
}

//==============================================================================
void Skeleton::integrateVelocities(double _dt)
{
  for (size_t i = 0; i < mBodyNodes.size(); ++i)
    mBodyNodes[i]->getParentJoint()->integrateVelocities(_dt);

  for (size_t i = 0; i < mSoftBodyNodes.size(); ++i)
  {
    for (size_t j = 0; j < mSoftBodyNodes[i]->getNumPointMasses(); ++j)
      mSoftBodyNodes[i]->getPointMass(j)->integrateVelocities(_dt);
  }
}

//==============================================================================
void Skeleton::computeForwardKinematics(bool _updateTransforms,
                                        bool _updateVels,
                                        bool _updateAccs)
{
  if (_updateTransforms)
  {
    for (std::vector<BodyNode*>::iterator it = mBodyNodes.begin();
         it != mBodyNodes.end(); ++it)
    {
      (*it)->updateTransform();
    }
  }

  if (_updateVels)
  {
    for (std::vector<BodyNode*>::iterator it = mBodyNodes.begin();
         it != mBodyNodes.end(); ++it)
    {
      (*it)->updateVelocity();
      (*it)->updatePartialAcceleration();
    }
  }

  if (_updateAccs)
  {
    for (std::vector<BodyNode*>::iterator it = mBodyNodes.begin();
         it != mBodyNodes.end(); ++it)
    {
      (*it)->updateAccelerationID();
    }
  }
}

//==============================================================================
static bool isValidBodyNode(const Skeleton* _skeleton,
                            const BodyNode* _bodyNode,
                            const std::string& _jacobianType)
{
  if (nullptr == _bodyNode)
  {
    dtwarn << "[Skeleton::getJacobian] Invalid BodyNode pointer, 'nullptr'. "
           << "Returning zero Jacobian." << std::endl;
    return false;
  }

  // The given BodyNode should be in the Skeleton.
  if (_bodyNode->getSkeleton().get() != _skeleton)
  {
    dtwarn << "[Skeleton::getJacobian] Attempting to get a "
           << _jacobianType << " of a BodyNode '"
           << _bodyNode->getName() << "' that is not in this Skeleton '"
           << _skeleton->getName() << ". Returning zero Jacobian." << std::endl;
    return false;
  }

  return true;
}

//==============================================================================
template <typename JacobianType>
void assignJacobian(JacobianType& _J,
                    const BodyNode* _bodyNode,
                    const JacobianType& _JBodyNode)
{
  // Assign the BodyNode's Jacobian to the result Jacobian.
  size_t localIndex = 0;
  const auto& indices = _bodyNode->getDependentGenCoordIndices();
  for (const auto& index : indices)
  {
    // Each index should be less than the number of dofs of this Skeleton.
    assert(index < _bodyNode->getSkeleton()->getNumDofs());

    _J.col(index) = _JBodyNode.col(localIndex++);
  }
}

//==============================================================================
math::Jacobian Skeleton::getJacobian(const BodyNode* _bodyNode) const
{
  math::Jacobian J = math::Jacobian::Zero(6, getNumDofs());

  // If _bodyNode is nullptr or not in this Skeleton, return zero Jacobian
  if (!isValidBodyNode(this, _bodyNode, "spatial Jacobian"))
    return J;

  // Get the spatial Jacobian of the targeting BodyNode
  const math::Jacobian JBodyNode = _bodyNode->getJacobian();

  // Assign the BodyNode's Jacobian to the full-sized Jacobian
  assignJacobian<math::Jacobian>(J, _bodyNode, JBodyNode);

  return J;
}

//==============================================================================
math::Jacobian Skeleton::getJacobian(const BodyNode* _bodyNode,
                                     const Frame* _inCoordinatesOf) const
{
  math::Jacobian J = math::Jacobian::Zero(6, getNumDofs());

  // If _bodyNode is nullptr or not in this Skeleton, return zero Jacobian
  if (!isValidBodyNode(this, _bodyNode, "spatial Jacobian"))
    return J;

  // Get the spatial Jacobian of the targeting BodyNode
  const math::Jacobian JBodyNode = _bodyNode->getJacobian(_inCoordinatesOf);

  // Assign the BodyNode's Jacobian to the full-sized Jacobian
  assignJacobian<math::Jacobian>(J, _bodyNode, JBodyNode);

  return J;
}

//==============================================================================
math::Jacobian Skeleton::getJacobian(const BodyNode* _bodyNode,
                                     const Eigen::Vector3d& _localOffset) const
{
  math::Jacobian J = math::Jacobian::Zero(6, getNumDofs());

  // If _bodyNode is nullptr or not in this Skeleton, return zero Jacobian
  if (!isValidBodyNode(this, _bodyNode, "spatial Jacobian"))
    return J;

  // Get the spatial Jacobian of the targeting BodyNode
  const math::Jacobian JBodyNode = _bodyNode->getJacobian(_localOffset);

  // Assign the BodyNode's Jacobian to the full-sized Jacobian
  assignJacobian<math::Jacobian>(J, _bodyNode, JBodyNode);

  return J;
}

//==============================================================================
math::Jacobian Skeleton::getJacobian(const BodyNode* _bodyNode,
                                     const Eigen::Vector3d& _localOffset,
                                     const Frame* _inCoordinatesOf) const
{
  math::Jacobian J = math::Jacobian::Zero(6, getNumDofs());

  // If _bodyNode is nullptr or not in this Skeleton, return zero Jacobian
  if (!isValidBodyNode(this, _bodyNode, "spatial Jacobian"))
    return J;

  // Get the spatial Jacobian of the targeting BodyNode
  const math::Jacobian JBodyNode = _bodyNode->getJacobian(_localOffset,
                                                          _inCoordinatesOf);

  // Assign the BodyNode's Jacobian to the full-sized Jacobian
  assignJacobian<math::Jacobian>(J, _bodyNode, JBodyNode);

  return J;
}

//==============================================================================
math::Jacobian Skeleton::getWorldJacobian(const BodyNode* _bodyNode) const
{
  math::Jacobian J = math::Jacobian::Zero(6, getNumDofs());

  // If _bodyNode is nullptr or not in this Skeleton, return zero Jacobian
  if (!isValidBodyNode(this, _bodyNode, "spatial Jacobian"))
    return J;

  // Get the spatial Jacobian of the targeting BodyNode
  const math::Jacobian JBodyNode = _bodyNode->getWorldJacobian();

  // Assign the BodyNode's Jacobian to the full-sized Jacobian
  assignJacobian<math::Jacobian>(J, _bodyNode, JBodyNode);

  return J;
}

//==============================================================================
math::Jacobian Skeleton::getWorldJacobian(
    const BodyNode* _bodyNode,
    const Eigen::Vector3d& _localOffset) const
{
  math::Jacobian J = math::Jacobian::Zero(6, getNumDofs());

  // If _bodyNode is nullptr or not in this Skeleton, return zero Jacobian
  if (!isValidBodyNode(this, _bodyNode, "spatial Jacobian"))
    return J;

  // Get the spatial Jacobian of the targeting BodyNode
  const math::Jacobian JBodyNode = _bodyNode->getWorldJacobian(_localOffset);

  // Assign the BodyNode's Jacobian to the full-sized Jacobian
  assignJacobian<math::Jacobian>(J, _bodyNode, JBodyNode);

  return J;
}

//==============================================================================
math::LinearJacobian Skeleton::getLinearJacobian(
    const BodyNode* _bodyNode,
    const Frame* _inCoordinatesOf) const
{
  math::LinearJacobian Jv = math::LinearJacobian::Zero(3, getNumDofs());

  // If _bodyNode is nullptr or not in this Skeleton, return zero Jacobian
  if (!isValidBodyNode(this, _bodyNode, "linear Jacobian"))
    return Jv;

  // Get the linear Jacobian of the targeting BodyNode
  math::LinearJacobian JvBodyNode
      = _bodyNode->getLinearJacobian(_inCoordinatesOf);

  // Assign the BodyNode's Jacobian to the full-sized Jacobian
  assignJacobian<math::LinearJacobian>(Jv, _bodyNode, JvBodyNode);

  return Jv;
}

//==============================================================================
math::LinearJacobian Skeleton::getLinearJacobian(
    const BodyNode* _bodyNode,
    const Eigen::Vector3d& _localOffset,
    const Frame* _inCoordinatesOf) const
{
  math::LinearJacobian Jv = math::LinearJacobian::Zero(3, getNumDofs());

  // If _bodyNode is nullptr or not in this Skeleton, return zero Jacobian
  if (!isValidBodyNode(this, _bodyNode, "linear Jacobian"))
    return Jv;

  // Get the linear Jacobian of the targeting BodyNode
  math::LinearJacobian JvBodyNode
      = _bodyNode->getLinearJacobian(_localOffset, _inCoordinatesOf);

  // Assign the BodyNode's Jacobian to the full-sized Jacobian
  assignJacobian<math::LinearJacobian>(Jv, _bodyNode, JvBodyNode);

  return Jv;
}

//==============================================================================
math::AngularJacobian Skeleton::getAngularJacobian(
    const BodyNode* _bodyNode,
    const Frame* _inCoordinatesOf) const
{
  math::AngularJacobian Jw = math::AngularJacobian::Zero(3, getNumDofs());

  // If _bodyNode is nullptr or not in this Skeleton, return zero Jacobian
  if (!isValidBodyNode(this, _bodyNode, "angular Jacobian"))
    return Jw;

  // Get the angular Jacobian of the targeting BodyNode
  math::AngularJacobian JwBodyNode
      = _bodyNode->getAngularJacobian(_inCoordinatesOf);

  // Assign the BodyNode's Jacobian to the full-sized Jacobian
  assignJacobian<math::AngularJacobian>(Jw, _bodyNode, JwBodyNode);

  return Jw;
}

//==============================================================================
math::Jacobian Skeleton::getJacobianSpatialDeriv(
    const BodyNode* _bodyNode) const
{
  math::Jacobian dJ = math::Jacobian::Zero(6, getNumDofs());

  // If _bodyNode is nullptr or not in this Skeleton, return zero Jacobian
  if (!isValidBodyNode(this, _bodyNode, "spatial Jacobian time derivative"))
    return dJ;

  // Get the spatial Jacobian time derivative of the targeting BodyNode
  math::Jacobian dJBodyNode = _bodyNode->getJacobianSpatialDeriv();

  // Assign the BodyNode's Jacobian to the full-sized Jacobian
  assignJacobian<math::Jacobian>(dJ, _bodyNode, dJBodyNode);

  return dJ;
}

//==============================================================================
math::Jacobian Skeleton::getJacobianSpatialDeriv(
    const BodyNode* _bodyNode,
    const Frame* _inCoordinatesOf) const
{
  math::Jacobian dJ = math::Jacobian::Zero(6, getNumDofs());

  // If _bodyNode is nullptr or not in this Skeleton, return zero Jacobian
  if (!isValidBodyNode(this, _bodyNode, "spatial Jacobian time derivative"))
    return dJ;

  // Get the spatial Jacobian time derivative of the targeting BodyNode
  math::Jacobian dJBodyNode
      = _bodyNode->getJacobianSpatialDeriv(_inCoordinatesOf);

  // Assign the BodyNode's Jacobian to the full-sized Jacobian
  assignJacobian<math::Jacobian>(dJ, _bodyNode, dJBodyNode);

  return dJ;
}

//==============================================================================
math::Jacobian Skeleton::getJacobianSpatialDeriv(
    const BodyNode* _bodyNode,
    const Eigen::Vector3d& _localOffset) const
{
  math::Jacobian dJ = math::Jacobian::Zero(6, getNumDofs());

  // If _bodyNode is nullptr or not in this Skeleton, return zero Jacobian
  if (!isValidBodyNode(this, _bodyNode, "spatial Jacobian time derivative"))
    return dJ;

  // Get the spatial Jacobian time derivative of the targeting BodyNode
  math::Jacobian dJBodyNode
      = _bodyNode->getJacobianSpatialDeriv(_localOffset);

  // Assign the BodyNode's Jacobian to the full-sized Jacobian
  assignJacobian<math::Jacobian>(dJ, _bodyNode, dJBodyNode);

  return dJ;
}

//==============================================================================
math::Jacobian Skeleton::getJacobianSpatialDeriv(
    const BodyNode* _bodyNode,
    const Eigen::Vector3d& _localOffset,
    const Frame* _inCoordinatesOf) const
{
  math::Jacobian dJ = math::Jacobian::Zero(6, getNumDofs());

  // If _bodyNode is nullptr or not in this Skeleton, return zero Jacobian
  if (!isValidBodyNode(this, _bodyNode, "spatial Jacobian time derivative"))
    return dJ;

  // Get the spatial Jacobian time derivative of the targeting BodyNode
  math::Jacobian dJBodyNode
      = _bodyNode->getJacobianSpatialDeriv(_localOffset, _inCoordinatesOf);

  // Assign the BodyNode's Jacobian to the full-sized Jacobian
  assignJacobian<math::Jacobian>(dJ, _bodyNode, dJBodyNode);

  return dJ;
}

//==============================================================================
math::Jacobian Skeleton::getJacobianClassicDeriv(
    const BodyNode* _bodyNode) const
{
  math::Jacobian dJ = math::Jacobian::Zero(3, getNumDofs());

  // If _bodyNode is nullptr or not in this Skeleton, return zero Jacobian
  if (!isValidBodyNode(this, _bodyNode,
                       "spatial Jacobian (classical) time derivative"))
    return dJ;

  // Get the spatial Jacobian time derivative of the targeting BodyNode
  math::Jacobian dJBodyNode = _bodyNode->getJacobianClassicDeriv();

  // Assign the BodyNode's Jacobian to the full-sized Jacobian
  assignJacobian<math::Jacobian>(dJ, _bodyNode, dJBodyNode);

  return dJ;
}

//==============================================================================
math::Jacobian Skeleton::getJacobianClassicDeriv(
    const BodyNode* _bodyNode,
    const Frame* _inCoordinatesOf) const
{
  math::Jacobian dJ = math::Jacobian::Zero(3, getNumDofs());

  // If _bodyNode is nullptr or not in this Skeleton, return zero Jacobian
  if (!isValidBodyNode(this, _bodyNode,
                       "spatial Jacobian (classical) time derivative"))
    return dJ;

  // Get the spatial Jacobian time derivative of the targeting BodyNode
  math::Jacobian dJBodyNode
      = _bodyNode->getJacobianClassicDeriv(_inCoordinatesOf);

  // Assign the BodyNode's Jacobian to the full-sized Jacobian
  assignJacobian<math::Jacobian>(dJ, _bodyNode, dJBodyNode);

  return dJ;
}

//==============================================================================
math::Jacobian Skeleton::getJacobianClassicDeriv(
    const BodyNode* _bodyNode,
    const Eigen::Vector3d& _localOffset,
    const Frame* _inCoordinatesOf) const
{
  math::Jacobian dJ = math::Jacobian::Zero(3, getNumDofs());

  // If _bodyNode is nullptr or not in this Skeleton, return zero Jacobian
  if (!isValidBodyNode(this, _bodyNode,
                       "spatial Jacobian (classical) time derivative"))
    return dJ;

  // Get the spatial Jacobian time derivative of the targeting BodyNode
  math::Jacobian dJBodyNode
      = _bodyNode->getJacobianClassicDeriv(_localOffset, _inCoordinatesOf);

  // Assign the BodyNode's Jacobian to the full-sized Jacobian
  assignJacobian<math::Jacobian>(dJ, _bodyNode, dJBodyNode);

  return dJ;
}

//==============================================================================
math::LinearJacobian Skeleton::getLinearJacobianDeriv(
    const BodyNode* _bodyNode,
    const Frame* _inCoordinatesOf) const
{
  math::LinearJacobian dJv = math::LinearJacobian::Zero(3, getNumDofs());

  // If _bodyNode is nullptr or not in this Skeleton, return zero Jacobian
  if (!isValidBodyNode(this, _bodyNode, "linear Jacobian time derivative"))
    return dJv;

  // Get the linear Jacobian time derivative of the targeting BodyNode
  math::LinearJacobian JvBodyNode
      = _bodyNode->getLinearJacobianDeriv(_inCoordinatesOf);

  // Assign the BodyNode's Jacobian to the full-sized Jacobian
  assignJacobian<math::LinearJacobian>(dJv, _bodyNode, JvBodyNode);

  return dJv;
}

//==============================================================================
math::LinearJacobian Skeleton::getLinearJacobianDeriv(
    const BodyNode* _bodyNode,
    const Eigen::Vector3d& _localOffset,
    const Frame* _inCoordinatesOf) const
{
  math::LinearJacobian dJv = math::LinearJacobian::Zero(3, getNumDofs());

  // If _bodyNode is nullptr or not in this Skeleton, return zero Jacobian
  if (!isValidBodyNode(this, _bodyNode, "linear Jacobian time derivative"))
    return dJv;

  // Get the linear Jacobian time derivative of the targeting BodyNode
  math::LinearJacobian JvBodyNode
      = _bodyNode->getLinearJacobianDeriv(_localOffset, _inCoordinatesOf);

  // Assign the BodyNode's Jacobian to the full-sized Jacobian
  assignJacobian<math::LinearJacobian>(dJv, _bodyNode, JvBodyNode);

  return dJv;
}

//==============================================================================
math::AngularJacobian Skeleton::getAngularJacobianDeriv(
    const BodyNode* _bodyNode, const Frame* _inCoordinatesOf) const
{
  math::AngularJacobian dJw = math::AngularJacobian::Zero(3, getNumDofs());

  // If _bodyNode is nullptr or not in this Skeleton, return zero Jacobian
  if (!isValidBodyNode(this, _bodyNode, "angular Jacobian time derivative"))
    return dJw;

  // Get the angular Jacobian time derivative of the targeting BodyNode
  math::AngularJacobian JwBodyNode
      = _bodyNode->getAngularJacobianDeriv(_inCoordinatesOf);

  // Assign the BodyNode's Jacobian to the full-sized Jacobian
  assignJacobian<math::AngularJacobian>(dJw, _bodyNode, JwBodyNode);

  return dJw;
}

//==============================================================================
double Skeleton::getMass() const
{
  return mTotalMass;
}

//==============================================================================
const Eigen::MatrixXd& Skeleton::getMassMatrix() const
{
  if (mIsMassMatrixDirty)
    updateMassMatrix();
  return mM;
}

//==============================================================================
const Eigen::MatrixXd& Skeleton::getAugMassMatrix() const
{
  if (mIsAugMassMatrixDirty)
    updateAugMassMatrix();

  return mAugM;
}

//==============================================================================
const Eigen::MatrixXd& Skeleton::getInvMassMatrix() const
{
  if (mIsInvMassMatrixDirty)
    updateInvMassMatrix();

  return mInvM;
}

//==============================================================================
const Eigen::MatrixXd& Skeleton::getInvAugMassMatrix() const
{
  if (mIsInvAugMassMatrixDirty)
    updateInvAugMassMatrix();

  return mInvAugM;
}

//==============================================================================
const Eigen::VectorXd& Skeleton::getCoriolisForces() const
{
  if (mIsCoriolisForcesDirty)
    updateCoriolisForces();

  return mCvec;
}

//==============================================================================
const Eigen::VectorXd& Skeleton::getGravityForces() const
{
  if (mIsGravityForcesDirty)
    updateGravityForces();

  return mG;
}

//==============================================================================
const Eigen::VectorXd& Skeleton::getCoriolisAndGravityForces() const
{
  if (mIsCoriolisAndGravityForcesDirty)
    updateCoriolisAndGravityForces();

  return mCg;
}

//==============================================================================
const Eigen::VectorXd& Skeleton::getExternalForces() const
{
  if (mIsExternalForcesDirty)
    updateExternalForces();

  return mFext;
}

//==============================================================================
const Eigen::VectorXd& Skeleton::getConstraintForces() const
{
  const size_t dof = getNumDofs();
  mFc = Eigen::VectorXd::Zero(dof);

  // Body constraint impulses
  for (std::vector<BodyNode*>::const_reverse_iterator it = mBodyNodes.rbegin();
       it != mBodyNodes.rend(); ++it)
  {
    (*it)->aggregateSpatialToGeneralized(&mFc, (*it)->getConstraintImpulse());
  }

  // Joint constraint impulses
  size_t index = 0;
  for (size_t i = 0; i < dof; ++i)
  {
    Joint* joint      = mDofs[i]->getJoint();
    size_t localIndex = mDofs[i]->getIndexInJoint();

    mFc[index++] += joint->getConstraintImpulse(localIndex);
  }
  assert(index == dof);

  // Get force by devide impulse by time step
  mFc = mFc / mSkeletonP.mTimeStep;

  return mFc;
}

//==============================================================================
const Eigen::VectorXd& Skeleton::getCoriolisForceVector() const
{
  return getCoriolisForces();
}

//==============================================================================
const Eigen::VectorXd& Skeleton::getGravityForceVector() const
{
  return getGravityForces();
}

//==============================================================================
const Eigen::VectorXd& Skeleton::getCombinedVector() const
{
  return getCoriolisAndGravityForces();
}

//==============================================================================
const Eigen::VectorXd& Skeleton::getExternalForceVector() const
{
  return getExternalForces();
}

//==============================================================================
//const Eigen::VectorXd& Skeleton::getDampingForceVector() {
//  if (mIsDampingForceVectorDirty)
//    updateDampingForceVector();
//  return mFd;
//}

//==============================================================================
const Eigen::VectorXd& Skeleton::getConstraintForceVector()
{
  return getConstraintForces();
}

//==============================================================================
void Skeleton::draw(renderer::RenderInterface* _ri, const Eigen::Vector4d& _color,
                    bool _useDefaultColor) const
{
  getRootBodyNode()->draw(_ri, _color, _useDefaultColor);
}

//==============================================================================
void Skeleton::drawMarkers(renderer::RenderInterface* _ri,
                           const Eigen::Vector4d& _color,
                           bool _useDefaultColor) const
{
  getRootBodyNode()->drawMarkers(_ri, _color, _useDefaultColor);
}

//==============================================================================
Skeleton::Skeleton(const Properties& _properties)
  : mTotalMass(0.0),
    mIsArticulatedInertiaDirty(true),
    mIsMassMatrixDirty(true),
    mIsAugMassMatrixDirty(true),
    mIsInvMassMatrixDirty(true),
    mIsInvAugMassMatrixDirty(true),
    mIsCoriolisForcesDirty(true),
    mIsGravityForcesDirty(true),
    mIsCoriolisAndGravityForcesDirty(true),
    mIsExternalForcesDirty(true),
    mIsDampingForcesDirty(true),
    mIsImpulseApplied(false),
    mUnionSize(1)
{
  setProperties(_properties);
}

//==============================================================================
void Skeleton::setPtr(std::shared_ptr<Skeleton> _ptr)
{
  mPtr = _ptr;
  resetUnion();
}

//==============================================================================
void Skeleton::registerBodyNode(BodyNode* _newBodyNode)
{
#ifndef NDEBUG  // Debug mode
  std::vector<BodyNode*>::iterator repeat =
      std::find(mBodyNodes.begin(), mBodyNodes.end(), _newBodyNode);
  if(repeat != mBodyNodes.end())
  {
    dterr << "[Skeleton::registerBodyNode] Attempting to double-register the "
          << "BodyNode named [" << _newBodyNode->getName() << "] in the "
          << "Skeleton named [" << getName() << "]. Please report this as a "
          << "bug!\n";
    return;
  }
#endif // -------- Debug mode

  mBodyNodes.push_back(_newBodyNode);
  if(nullptr == _newBodyNode->getParentBodyNode())
  {
    mRootBodyNodes.push_back(_newBodyNode);
  }
  _newBodyNode->mSkeleton = getPtr();
  _newBodyNode->mIndexInSkeleton = mBodyNodes.size()-1;
  addEntryToBodyNodeNameMgr(_newBodyNode);
  registerJoint(_newBodyNode->getParentJoint());

  SoftBodyNode* softBodyNode = dynamic_cast<SoftBodyNode*>(_newBodyNode);
  if (softBodyNode)
  {
    mSoftBodyNodes.push_back(softBodyNode);
    addEntryToSoftBodyNodeNameMgr(softBodyNode);
  }

  _newBodyNode->init(getPtr());

  updateTotalMass();
  updateCacheDimensions();
  // TODO(MXG): Decide if the following are necessary
//  clearExternalForces();
//  resetForces();

#ifndef NDEBUG // Debug mode
  for(size_t i=0; i<mBodyNodes.size(); ++i)
  {
    if(mBodyNodes[i]->mIndexInSkeleton != i)
    {
      dterr << "[Skeleton::registerBodyNode] BodyNode named ["
            << mBodyNodes[i]->getName() << "] in Skeleton [" << getName()
            << "] is mistaken about its index ( " << i << " : "
            << mBodyNodes[i]->mIndexInSkeleton << " ). Please report this as "
            << "a bug!\n";
      assert(false);
    }
  }
#endif // ------- Debug mode

  _newBodyNode->mStructuralChangeSignal.raise(_newBodyNode);
}

//==============================================================================
void Skeleton::registerJoint(Joint* _newJoint)
{
  if (nullptr == _newJoint)
  {
    dterr << "[Skeleton::registerJoint] Error: Attempting to add a nullptr "
             "Joint to the Skeleton named [" << mSkeletonP.mName << "]. Report "
             "this as a bug!\n";
    assert(false);
    return;
  }

  addEntryToJointNameMgr(_newJoint);
  _newJoint->registerDofs();

  const size_t startDof = getNumDofs();
  for(size_t i = 0; i < _newJoint->getNumDofs(); ++i)
  {
    mDofs.push_back(_newJoint->getDof(i));
    _newJoint->setIndexInSkeleton(i, startDof + i);
  }
}

//==============================================================================
void Skeleton::unregisterBodyNode(BodyNode* _oldBodyNode)
{
  mNameMgrForBodyNodes.removeName(_oldBodyNode->getName());

  size_t index = _oldBodyNode->getIndex();
  assert(mBodyNodes[index] == _oldBodyNode);
  mBodyNodes.erase(mBodyNodes.begin()+index);
  for(size_t i=index; i < mBodyNodes.size(); ++i)
  {
    BodyNode* bn = mBodyNodes[i];
    bn->mIndexInSkeleton = i;
  }

  SoftBodyNode* soft = dynamic_cast<SoftBodyNode*>(_oldBodyNode);
  if(soft)
  {
    mNameMgrForSoftBodyNodes.removeName(soft->getName());

    mSoftBodyNodes.erase(std::remove(mSoftBodyNodes.begin(),
                                     mSoftBodyNodes.end(), soft),
                         mSoftBodyNodes.end());
  }

  unregisterJoint(_oldBodyNode->getParentJoint());
}

//==============================================================================
void Skeleton::unregisterJoint(Joint* _oldJoint)
{
  if (nullptr == _oldJoint)
  {
    dterr << "[Skeleton::unregisterJoint] Attempting to unregister nullptr "
          << "Joint from Skeleton named [" << getName() << "]. Report this as "
          << "a bug!\n";
    assert(false);
    return;
  }

  mNameMgrForJoints.removeName(_oldJoint->getName());

  size_t firstDofIndex = (size_t)(-1);
  for (size_t i = 0; i < _oldJoint->getNumDofs(); ++i)
  {
    DegreeOfFreedom* dof = _oldJoint->getDof(i);
    mNameMgrForDofs.removeObject(dof);

    firstDofIndex = std::min(firstDofIndex, dof->getIndexInSkeleton());
    mDofs.erase(std::remove(mDofs.begin(), mDofs.end(), dof), mDofs.end());
  }

  for (size_t i = firstDofIndex; i < mDofs.size(); ++i)
  {
    DegreeOfFreedom* dof = mDofs[i];
    dof->mIndexInSkeleton = i;
  }
}

//==============================================================================
void Skeleton::moveBodyNodeTree(Joint* _parentJoint, BodyNode* _bodyNode,
                                std::shared_ptr<Skeleton> _newSkeleton,
                                BodyNode* _parentNode)
{
  if(nullptr == _bodyNode)
  {
    dterr << "[Skeleton::moveBodyNodeTree] Skeleton named [" << getName()
          << "] (" << this << ") is attempting to move a nullptr BodyNode. "
          << "Please report this as a bug!\n";
    assert(false);
    return;
  }

  if(this != _bodyNode->getSkeleton().get())
  {
    dterr << "[Skeleton::moveBodyNodeTree] Skeleton named [" << getName()
          << "] (" << this << ") is attempting to move a BodyNode named ["
          << _bodyNode->getName() << "] even though it belongs to another "
          << "Skeleton [" << _bodyNode->getSkeleton()->getName() << "] ("
          << _bodyNode->getSkeleton() << "). Please report this as a bug!\n";
    assert(false);
    return;
  }

  if( (nullptr == _parentJoint)
      && (_bodyNode->getParentBodyNode() == _parentNode)
      && (this == _newSkeleton.get()) )
  {
    // Short-circuit if the BodyNode is already in the requested place, and its
    // Joint does not need to be changed
    return;
  }

  if(_bodyNode == _parentNode)
  {
    dterr << "[Skeleton::moveBodyNodeTree] Attempting to move BodyNode named ["
          << _bodyNode->getName() << "] (" << _bodyNode << ") to be its own "
          << "parent. This is not permitted!\n";
    return;
  }

  if(_parentNode && _parentNode->descendsFrom(_bodyNode))
  {
    dterr << "[Skeleton::moveBodyNodeTree] Attempting to move BodyNode named ["
          << _bodyNode->getName() << "] of Skeleton [" << getName() << "] ("
          << this << ") to be a child of BodyNode [" << _parentNode->getName()
          << "] in Skeleton [" << _newSkeleton->getName() << "] ("
          << _newSkeleton << "), but that would create a closed kinematic "
          << "chain, which is not permitted! Nothing will be moved.\n";
    return;
  }

  if(nullptr == _newSkeleton)
  {
    if(nullptr == _parentNode)
    {
      dterr << "[Skeleton::moveBodyNodeTree] Attempting to move a BodyNode "
            << "tree starting from [" << _bodyNode->getName() << "] in "
            << "Skeleton [" << getName() << "] into a nullptr Skeleton. This "
            << "is not permitted!\n";
      return;
    }

    _newSkeleton = _parentNode->getSkeleton();
  }

  if(_parentNode && _newSkeleton != _parentNode->getSkeleton())
  {
    dterr << "[Skeleton::moveBodyNodeTree] Mismatch between the specified "
          << "Skeleton [" << _newSkeleton->getName() << "] (" << _newSkeleton
          << ") and the specified new parent BodyNode ["
          << _parentNode->getName() << "] whose actual Skeleton is named ["
          << _parentNode->getSkeleton()->getName() << "] ("
          << _parentNode->getSkeleton() << ") while attempting to move a "
          << "BodyNode tree starting from [" << _bodyNode->getName() << "] in "
          << "Skeleton [" << getName() << "] (" << this << ")\n";
    return;
  }

  std::vector<BodyNode*> tree = extractBodyNodeTree(_bodyNode);

  Joint* originalParent = _bodyNode->getParentJoint();
  if(originalParent != _parentJoint)
  {
    _bodyNode->mParentJoint = _parentJoint;
    _parentJoint->mChildBodyNode = _bodyNode;
    delete originalParent;
  }

  if(_parentNode != _bodyNode->getParentBodyNode())
  {
    _bodyNode->mParentBodyNode = _parentNode;
    if(_parentNode)
    {
      _parentNode->mChildBodyNodes.push_back(_bodyNode);
      _bodyNode->changeParentFrame(_parentNode);
    }
    else
    {
      _bodyNode->changeParentFrame(Frame::World());
    }
  }
  _newSkeleton->receiveBodyNodeTree(tree);
}

//==============================================================================
std::pair<Joint*, BodyNode*> Skeleton::cloneBodyNodeTree(
    Joint* _parentJoint, const BodyNode* _bodyNode,
    std::shared_ptr<Skeleton> _newSkeleton, BodyNode* _parentNode) const
{
  std::pair<Joint*, BodyNode*> root(nullptr, nullptr);
  std::vector<const BodyNode*> tree = constructBodyNodeTree(_bodyNode);

  std::map<std::string, BodyNode*> nameMap;
  std::vector<BodyNode*> clones;
  clones.reserve(tree.size());

  for(size_t i=0; i<tree.size(); ++i)
  {
    const BodyNode* original = tree[i];
    // If this is the root of the tree, and the user has requested a change in
    // its parent Joint, use the specified parent Joint instead of created a
    // clone
    Joint* joint = (i==0 && _parentJoint != nullptr) ? _parentJoint :
        original->getParentJoint()->clone();

    BodyNode* newParent = i==0 ? _parentNode :
        nameMap[original->getParentBodyNode()->getName()];

    BodyNode* clone = original->clone(newParent, joint);
    clones.push_back(clone);
    nameMap[clone->getName()] = clone;

    if(0==i)
    {
      root.first = joint;
      root.second = clone;
    }
  }

  _newSkeleton->receiveBodyNodeTree(clones);
  return root;
}

//==============================================================================
template <typename BodyNodeT>
static void recursiveConstructBodyNodeTree(
    std::vector<BodyNodeT*>& tree, BodyNodeT* _currentBodyNode)
{
  tree.push_back(_currentBodyNode);
  for(size_t i=0; i<_currentBodyNode->getNumChildBodyNodes(); ++i)
    recursiveConstructBodyNodeTree(tree, _currentBodyNode->getChildBodyNode(i));
}

//==============================================================================
std::vector<const BodyNode*> Skeleton::constructBodyNodeTree(
    const BodyNode* _bodyNode) const
{
  std::vector<const BodyNode*> tree;
  recursiveConstructBodyNodeTree<const BodyNode>(tree, _bodyNode);

  return tree;
}

//==============================================================================
std::vector<BodyNode*> Skeleton::constructBodyNodeTree(BodyNode *_bodyNode)
{
  std::vector<BodyNode*> tree;
  recursiveConstructBodyNodeTree<BodyNode>(tree, _bodyNode);

  return tree;
}

//==============================================================================
std::vector<BodyNode*> Skeleton::extractBodyNodeTree(BodyNode* _bodyNode)
{
  std::vector<BodyNode*> tree = constructBodyNodeTree(_bodyNode);

  // Go backwards to minimize the number of shifts needed
  std::vector<BodyNode*>::reverse_iterator rit;
  for(rit = tree.rbegin(); rit != tree.rend(); ++rit)
    unregisterBodyNode(*rit);

  for(size_t i=0; i<mBodyNodes.size(); ++i)
    mBodyNodes[i]->init(getPtr());

  return tree;
}

//==============================================================================
void Skeleton::receiveBodyNodeTree(const std::vector<BodyNode*>& _tree)
{
  for(BodyNode* bn : _tree)
    registerBodyNode(bn);
}

//==============================================================================
void Skeleton::notifyArticulatedInertiaUpdate()
{
  if(mIsArticulatedInertiaDirty)
    return;

  mIsArticulatedInertiaDirty = true;
  mIsMassMatrixDirty = true;
  mIsAugMassMatrixDirty = true;
  mIsInvMassMatrixDirty = true;
  mIsInvAugMassMatrixDirty = true;
  mIsCoriolisForcesDirty = true;
  mIsGravityForcesDirty = true;
  mIsCoriolisAndGravityForcesDirty = true;
}

//==============================================================================
void Skeleton::updateTotalMass()
{
  mTotalMass = 0.0;
  for(size_t i=0; i<getNumBodyNodes(); ++i)
    mTotalMass += getBodyNode(i)->getMass();
}

//==============================================================================
void Skeleton::updateCacheDimensions()
{
  size_t dof = getNumDofs();
  mM    = Eigen::MatrixXd::Zero(dof, dof);
  mAugM = Eigen::MatrixXd::Zero(dof, dof);
  mInvM = Eigen::MatrixXd::Zero(dof, dof);
  mInvAugM = Eigen::MatrixXd::Zero(dof, dof);
  mCvec = Eigen::VectorXd::Zero(dof);
  mG    = Eigen::VectorXd::Zero(dof);
  mCg   = Eigen::VectorXd::Zero(dof);
  mFext = Eigen::VectorXd::Zero(dof);
  mFc   = Eigen::VectorXd::Zero(dof);
  mFd   = Eigen::VectorXd::Zero(dof);

  notifyArticulatedInertiaUpdate();
}

//==============================================================================
void Skeleton::updateArticulatedInertia() const
{
  for (std::vector<BodyNode*>::const_reverse_iterator it = mBodyNodes.rbegin();
       it != mBodyNodes.rend(); ++it)
  {
    (*it)->updateArtInertia(mSkeletonP.mTimeStep);
  }

  mIsArticulatedInertiaDirty = false;
}

//==============================================================================
void Skeleton::updateMassMatrix() const
{
  if (getNumDofs() == 0)
    return;

  assert(static_cast<size_t>(mM.cols()) == getNumDofs()
         && static_cast<size_t>(mM.rows()) == getNumDofs());

  mM.setZero();

  // Backup the original internal force
  Eigen::VectorXd originalGenAcceleration = getAccelerations();

  size_t dof = getNumDofs();
  Eigen::VectorXd e = Eigen::VectorXd::Zero(dof);
  for (size_t j = 0; j < dof; ++j)
  {
    e[j] = 1.0;
    // This const_cast is okay, because we will return the accelerations to
    // their original values at the end of the function
    const_cast<Skeleton*>(this)->setAccelerations(e);

    // Prepare cache data
    for (std::vector<BodyNode*>::const_iterator it = mBodyNodes.begin();
         it != mBodyNodes.end(); ++it)
    {
      (*it)->updateMassMatrix();
    }

    // Mass matrix
    for (std::vector<BodyNode*>::const_reverse_iterator it = mBodyNodes.rbegin();
         it != mBodyNodes.rend(); ++it)
    {
      (*it)->aggregateMassMatrix(&mM, j);
      size_t localDof = (*it)->mParentJoint->getNumDofs();
      if (localDof > 0)
      {
        size_t iStart = (*it)->mParentJoint->getIndexInSkeleton(0);

        if (iStart + localDof < j)
          break;
      }
    }

    e[j] = 0.0;
  }
  mM.triangularView<Eigen::StrictlyUpper>() = mM.transpose();

  // Restore the original generalized accelerations
  const_cast<Skeleton*>(this)->setAccelerations(originalGenAcceleration);

  mIsMassMatrixDirty = false;
}

//==============================================================================
void Skeleton::updateAugMassMatrix() const
{
  if (getNumDofs() == 0)
    return;

  assert(static_cast<size_t>(mAugM.cols()) == getNumDofs()
         && static_cast<size_t>(mAugM.rows()) == getNumDofs());

  mAugM.setZero();

  // Backup the origianl internal force
  Eigen::VectorXd originalGenAcceleration = getAccelerations();

  int dof = getNumDofs();
  Eigen::VectorXd e = Eigen::VectorXd::Zero(dof);
  for (int j = 0; j < dof; ++j)
  {
    e[j] = 1.0;
    // This const_cast is okay, because we will return the accelerations to
    // their original values at the end of the function
    const_cast<Skeleton*>(this)->setAccelerations(e);

    // Prepare cache data
    for (std::vector<BodyNode*>::const_iterator it = mBodyNodes.begin();
         it != mBodyNodes.end(); ++it)
    {
      (*it)->updateMassMatrix();
    }

    // Mass matrix
    //    for (std::vector<BodyNode*>::iterator it = mBodyNodes.begin();
    //         it != mBodyNodes.end(); ++it)
    for (int i = mBodyNodes.size() - 1; i > -1 ; --i)
    {
      mBodyNodes[i]->aggregateAugMassMatrix(&mAugM, j, mSkeletonP.mTimeStep);
      int localDof = mBodyNodes[i]->mParentJoint->getNumDofs();
      if (localDof > 0)
      {
        int iStart =
            mBodyNodes[i]->mParentJoint->getIndexInSkeleton(0);
        if (iStart + localDof < j)
          break;
      }
    }

    e[j] = 0.0;
  }
  mAugM.triangularView<Eigen::StrictlyUpper>() = mAugM.transpose();

  // Restore the origianl internal force
  const_cast<Skeleton*>(this)->setAccelerations(originalGenAcceleration);

  mIsAugMassMatrixDirty = false;
}

//==============================================================================
void Skeleton::updateInvMassMatrix() const
{
  if (getNumDofs() == 0)
    return;

  assert(static_cast<size_t>(mInvM.cols()) == getNumDofs()
         && static_cast<size_t>(mInvM.rows()) == getNumDofs());

  // We don't need to set mInvM as zero matrix as long as the below is correct
  // mInvM.setZero();

  // Backup the origianl internal force
  Eigen::VectorXd originalInternalForce = getForces();

  // Note: we do not need to update articulated inertias here, because they will
  // be updated when BodyNode::updateInvMassMatrix() calls
  // BodyNode::getArticulatedInertia()

  int dof = getNumDofs();
  Eigen::VectorXd e = Eigen::VectorXd::Zero(dof);
  for (int j = 0; j < dof; ++j)
  {
    e[j] = 1.0;
    // This const_cast is okay, because we set the forces back to their original
    // values at the end of this function
    const_cast<Skeleton*>(this)->setForces(e);

    // Prepare cache data
    for (std::vector<BodyNode*>::const_reverse_iterator it = mBodyNodes.rbegin();
         it != mBodyNodes.rend(); ++it)
    {
      (*it)->updateInvMassMatrix();
    }

    // Inverse of mass matrix
    //    for (std::vector<BodyNode*>::iterator it = mBodyNodes.begin();
    //         it != mBodyNodes.end(); ++it)
    for (size_t i = 0; i < mBodyNodes.size(); ++i)
    {
      mBodyNodes[i]->aggregateInvMassMatrix(&mInvM, j);
      int localDof = mBodyNodes[i]->mParentJoint->getNumDofs();
      if (localDof > 0)
      {
        int iStart =
            mBodyNodes[i]->mParentJoint->getIndexInSkeleton(0);
        if (iStart + localDof > j)
          break;
      }
    }

    e[j] = 0.0;
  }
  mInvM.triangularView<Eigen::StrictlyLower>() = mInvM.transpose();

  // Restore the origianl internal force
  const_cast<Skeleton*>(this)->setForces(originalInternalForce);

  mIsInvMassMatrixDirty = false;
}

//==============================================================================
void Skeleton::updateInvAugMassMatrix() const
{
  if (getNumDofs() == 0)
    return;

  assert(static_cast<size_t>(mInvAugM.cols()) == getNumDofs()
         && static_cast<size_t>(mInvAugM.rows()) == getNumDofs());

  // We don't need to set mInvM as zero matrix as long as the below is correct
  // mInvM.setZero();

  // Backup the origianl internal force
  Eigen::VectorXd originalInternalForce = getForces();

  int dof = getNumDofs();
  Eigen::VectorXd e = Eigen::VectorXd::Zero(dof);
  for (int j = 0; j < dof; ++j)
  {
    e[j] = 1.0;
    // This const_cast is okay, because we set the forces back to their original
    // values at the end
    const_cast<Skeleton*>(this)->setForces(e);

    // Prepare cache data
    for (std::vector<BodyNode*>::const_reverse_iterator it = mBodyNodes.rbegin();
         it != mBodyNodes.rend(); ++it)
    {
      (*it)->updateInvAugMassMatrix();
    }

    // Inverse of mass matrix
    //    for (std::vector<BodyNode*>::iterator it = mBodyNodes.begin();
    //         it != mBodyNodes.end(); ++it)
    for (size_t i = 0; i < mBodyNodes.size(); ++i)
    {
      mBodyNodes[i]->aggregateInvAugMassMatrix(&mInvAugM, j, mSkeletonP.mTimeStep);
      int localDof = mBodyNodes[i]->mParentJoint->getNumDofs();
      if (localDof > 0)
      {
        int iStart =
            mBodyNodes[i]->mParentJoint->getIndexInSkeleton(0);
        if (iStart + localDof > j)
          break;
      }
    }

    e[j] = 0.0;
  }
  mInvAugM.triangularView<Eigen::StrictlyLower>() = mInvAugM.transpose();

  // Restore the origianl internal force
  const_cast<Skeleton*>(this)->setForces(originalInternalForce);

  mIsInvAugMassMatrixDirty = false;
}

//==============================================================================
void Skeleton::updateCoriolisForceVector()
{
  updateCoriolisForces();
}

//==============================================================================
void Skeleton::updateCoriolisForces() const
{
  if (getNumDofs() == 0)
    return;

  assert(static_cast<size_t>(mCvec.size()) == getNumDofs());

  mCvec.setZero();

  for (std::vector<BodyNode*>::const_iterator it = mBodyNodes.begin();
       it != mBodyNodes.end(); ++it)
  {
    (*it)->updateCombinedVector();
  }

  for (std::vector<BodyNode*>::const_reverse_iterator it = mBodyNodes.rbegin();
       it != mBodyNodes.rend(); ++it)
  {
    (*it)->aggregateCoriolisForceVector(&mCvec);
  }

  mIsCoriolisForcesDirty = false;
}

//==============================================================================
void Skeleton::updateGravityForceVector()
{
  updateGravityForces();
}

//==============================================================================
void Skeleton::updateGravityForces() const
{
  if (getNumDofs() == 0)
    return;

  assert(static_cast<size_t>(mG.size()) == getNumDofs());

  // Calcualtion mass matrix, M
  mG.setZero();
  for (std::vector<BodyNode*>::const_reverse_iterator it = mBodyNodes.rbegin();
       it != mBodyNodes.rend(); ++it)
  {
    (*it)->aggregateGravityForceVector(&mG, mSkeletonP.mGravity);
  }

  mIsGravityForcesDirty = false;
}

//==============================================================================
void Skeleton::updateCombinedVector()
{
  updateCoriolisAndGravityForces();
}

//==============================================================================
void Skeleton::updateCoriolisAndGravityForces() const
{
  if (getNumDofs() == 0)
    return;

  assert(static_cast<size_t>(mCg.size()) == getNumDofs());

  mCg.setZero();
  for (std::vector<BodyNode*>::const_iterator it = mBodyNodes.begin();
       it != mBodyNodes.end(); ++it)
  {
    (*it)->updateCombinedVector();
  }

  for (std::vector<BodyNode*>::const_reverse_iterator it = mBodyNodes.rbegin();
       it != mBodyNodes.rend(); ++it)
  {
    (*it)->aggregateCombinedVector(&mCg, mSkeletonP.mGravity);
  }

  mIsCoriolisAndGravityForcesDirty = false;
}

//==============================================================================
void Skeleton::updateExternalForceVector()
{
  updateExternalForces();
}

//==============================================================================
void Skeleton::updateExternalForces() const
{
  if (getNumDofs() == 0)
    return;

  assert(static_cast<size_t>(mFext.size()) == getNumDofs());

  // Clear external force.
  mFext.setZero();

  for (std::vector<BodyNode*>::const_reverse_iterator itr = mBodyNodes.rbegin();
       itr != mBodyNodes.rend(); ++itr)
  {
    (*itr)->aggregateExternalForces(&mFext);
  }

  // TODO(JS): Not implemented yet
//  for (std::vector<SoftBodyNode*>::iterator it = mSoftBodyNodes.begin();
//       it != mSoftBodyNodes.end(); ++it)
//  {
//    double kv = (*it)->getVertexSpringStiffness();
//    double ke = (*it)->getEdgeSpringStiffness();

//    for (int i = 0; i < (*it)->getNumPointMasses(); ++i)
//    {
//      PointMass* pm = (*it)->getPointMass(i);
//      int nN = pm->getNumConnectedPointMasses();

//      // Vertex restoring force
//      Eigen::Vector3d Fext = -(kv + nN * ke) * pm->getPositions()
//                             - (mTimeStep * (kv + nN*ke)) * pm->getVelocities();

//      // Edge restoring force
//      for (int j = 0; j < nN; ++j)
//      {
//        Fext += ke * (pm->getConnectedPointMass(j)->getPositions()
//                      + mTimeStep
//                        * pm->getConnectedPointMass(j)->getVelocities());
//      }

//      // Assign
//      int iStart = pm->getIndexInSkeleton(0);
//      mFext.segment<3>(iStart) = Fext;
//    }
//  }

  mIsExternalForcesDirty = false;
}

//==============================================================================
//void Skeleton::updateDampingForceVector() {
//  assert(mFd.size() == getNumDofs());
//  assert(getNumDofs() > 0);

//  // Clear external force.
//  mFd.setZero();

//  for (std::vector<BodyNode*>::iterator itr = mBodyNodes.begin();
//       itr != mBodyNodes.end(); ++itr) {
//    Eigen::VectorXd jointDampingForce =
//        (*itr)->getParentJoint()->getDampingForces();
//    for (int i = 0; i < jointDampingForce.size(); i++) {
//      mFd((*itr)->getParentJoint()->getIndexInSkeleton(0)) =
//          jointDampingForce(i);
//    }
//  }

//  for (std::vector<SoftBodyNode*>::iterator it = mSoftBodyNodes.begin();
//       it != mSoftBodyNodes.end(); ++it) {
//    for (int i = 0; i < (*it)->getNumPointMasses(); ++i) {
//      PointMass* pm = (*it)->getPointMass(i);
//      int iStart = pm->getGenCoord(0)->getIndexInSkeleton();
//      mFd.segment<3>(iStart) = -(*it)->getDampingCoefficient() * pm->getVelocities();
//    }
//  }
//}

//==============================================================================
void Skeleton::computeForwardDynamics()
{
  //
//  computeForwardDynamicsRecursionPartA(); // No longer needed with auto-update

  //
  computeForwardDynamicsRecursionPartB();
}

//==============================================================================
void Skeleton::computeForwardDynamicsRecursionPartA()
{
  // Update body transformations, velocities, and partial acceleration due to
  // parent joint's velocity
  for (std::vector<BodyNode*>::iterator it = mBodyNodes.begin();
       it != mBodyNodes.end(); ++it)
  {
    (*it)->updateTransform();
    (*it)->updateVelocity();
    (*it)->updatePartialAcceleration();
  }

  mIsArticulatedInertiaDirty = true;
  mIsMassMatrixDirty = true;
  mIsAugMassMatrixDirty = true;
  mIsInvMassMatrixDirty = true;
  mIsInvAugMassMatrixDirty = true;
  mIsCoriolisForcesDirty = true;
  mIsGravityForcesDirty = true;
  mIsCoriolisAndGravityForcesDirty = true;
  mIsExternalForcesDirty = true;
//  mIsDampingForceVectorDirty = true;

  for (std::vector<BodyNode*>::iterator it = mBodyNodes.begin();
       it != mBodyNodes.end(); ++it)
  {
    (*it)->mIsBodyJacobianDirty = true;
    (*it)->mIsWorldJacobianDirty = true;
    (*it)->mIsBodyJacobianSpatialDerivDirty = true;
    (*it)->mIsWorldJacobianClassicDerivDirty = true;
  }
}

//==============================================================================
void Skeleton::computeForwardDynamicsRecursionPartB()
{
  // Note: Articulated Inertias will be updated automatically when
  // getArtInertiaImplicit() is called in BodyNode::updateBiasForce()

  for (auto it = mBodyNodes.rbegin(); it != mBodyNodes.rend(); ++it)
    (*it)->updateBiasForce(mSkeletonP.mGravity, mSkeletonP.mTimeStep);

  // Forward recursion
  for (auto& bodyNode : mBodyNodes)
  {
    bodyNode->updateAccelerationFD();
    bodyNode->updateTransmittedForceFD();
    bodyNode->updateJointForceFD(mSkeletonP.mTimeStep, true, true);
  }
}

//==============================================================================
void Skeleton::computeInverseDynamics(bool _withExternalForces,
                                      bool _withDampingForces)
{
  //
  computeInverseDynamicsRecursionB(_withExternalForces, _withDampingForces);
}

//==============================================================================
void Skeleton::computeInverseDynamicsRecursionA()
{
  for (std::vector<BodyNode*>::iterator it = mBodyNodes.begin();
       it != mBodyNodes.end(); ++it)
  {
    (*it)->updateTransform();
    (*it)->updateVelocity();
    (*it)->updatePartialAcceleration();
    (*it)->updateAccelerationID();
  }

  mIsArticulatedInertiaDirty = true;
  mIsMassMatrixDirty = true;
  mIsAugMassMatrixDirty = true;
  mIsInvMassMatrixDirty = true;
  mIsInvAugMassMatrixDirty = true;
  mIsCoriolisForcesDirty = true;
  mIsGravityForcesDirty = true;
  mIsCoriolisAndGravityForcesDirty = true;
  mIsExternalForcesDirty = true;
//  mIsDampingForceVectorDirty = true;

  for (std::vector<BodyNode*>::iterator it = mBodyNodes.begin();
       it != mBodyNodes.end(); ++it)
  {
    (*it)->mIsBodyJacobianDirty = true;
    (*it)->mIsWorldJacobianDirty = true;
    (*it)->mIsBodyJacobianSpatialDerivDirty = true;
    (*it)->mIsWorldJacobianClassicDerivDirty = true;
  }
}

//==============================================================================
void Skeleton::computeInverseDynamicsRecursionB(bool _withExternalForces,
                                                bool _withDampingForces,
                                                bool _withSpringForces)
{
  // Skip immobile or 0-dof skeleton
  if (getNumDofs() == 0)
    return;

  // Backward recursion
  for (auto it = mBodyNodes.rbegin(); it != mBodyNodes.rend(); ++it)
  {
    (*it)->updateTransmittedForceID(mSkeletonP.mGravity, _withExternalForces);
    (*it)->updateJointForceID(mSkeletonP.mTimeStep,
                              _withDampingForces,
                              _withSpringForces);
  }
}

//==============================================================================
void Skeleton::clearExternalForces()
{
  for (auto& bodyNode : mBodyNodes)
    bodyNode->clearExternalForces();
}

//==============================================================================
void Skeleton::clearConstraintImpulses()
{
  for (auto& bodyNode : mBodyNodes)
    bodyNode->clearConstraintImpulse();
}

//==============================================================================
void Skeleton::updateBiasImpulse(BodyNode* _bodyNode)
{
  // Assertions
  assert(_bodyNode != nullptr);
  assert(getNumDofs() > 0);

  // This skeleton should contains _bodyNode
  assert(std::find(mBodyNodes.begin(), mBodyNodes.end(), _bodyNode)
         != mBodyNodes.end());

#ifndef NDEBUG
  // All the constraint impulse should be zero
  for (size_t i = 0; i < mBodyNodes.size(); ++i)
    assert(mBodyNodes[i]->mConstraintImpulse == Eigen::Vector6d::Zero());
#endif

  // Prepare cache data
  BodyNode* it = _bodyNode;
  while (it != nullptr)
  {
    it->updateBiasImpulse();
    it = it->getParentBodyNode();
  }
}

//==============================================================================
void Skeleton::updateBiasImpulse(BodyNode* _bodyNode,
                                 const Eigen::Vector6d& _imp)
{
  // Assertions
  assert(_bodyNode != nullptr);
  assert(getNumDofs() > 0);

  // This skeleton should contain _bodyNode
  assert(std::find(mBodyNodes.begin(), mBodyNodes.end(), _bodyNode)
         != mBodyNodes.end());

#ifndef NDEBUG
  // All the constraint impulse should be zero
  for (size_t i = 0; i < mBodyNodes.size(); ++i)
    assert(mBodyNodes[i]->mConstraintImpulse == Eigen::Vector6d::Zero());
#endif

  // Set impulse to _bodyNode
  _bodyNode->mConstraintImpulse = _imp;

  // Prepare cache data
  BodyNode* it = _bodyNode;
  while (it != nullptr)
  {
    it->updateBiasImpulse();
    it = it->getParentBodyNode();
  }

  _bodyNode->mConstraintImpulse.setZero();
}

//==============================================================================
void Skeleton::updateBiasImpulse(BodyNode* _bodyNode1,
                                 const Eigen::Vector6d& _imp1,
                                 BodyNode* _bodyNode2,
                                 const Eigen::Vector6d& _imp2)
{
  // Assertions
  assert(_bodyNode1 != nullptr);
  assert(_bodyNode2 != nullptr);
  assert(getNumDofs() > 0);

  // This skeleton should contain _bodyNode
  assert(std::find(mBodyNodes.begin(), mBodyNodes.end(), _bodyNode1)
         != mBodyNodes.end());
  assert(std::find(mBodyNodes.begin(), mBodyNodes.end(), _bodyNode2)
         != mBodyNodes.end());

#ifndef NDEBUG
  // All the constraint impulse should be zero
  for (size_t i = 0; i < mBodyNodes.size(); ++i)
    assert(mBodyNodes[i]->mConstraintImpulse == Eigen::Vector6d::Zero());
#endif

  // Set impulse to _bodyNode
  _bodyNode1->mConstraintImpulse = _imp1;
  _bodyNode2->mConstraintImpulse = _imp2;

  // Find which body is placed later in the list of body nodes in this skeleton
  std::vector<BodyNode*>::reverse_iterator it1
      = std::find(mBodyNodes.rbegin(), mBodyNodes.rend(), _bodyNode1);
  std::vector<BodyNode*>::reverse_iterator it2
      = std::find(mBodyNodes.rbegin(), mBodyNodes.rend(), _bodyNode2);

  std::vector<BodyNode*>::reverse_iterator it = std::min(it1, it2);

  // Prepare cache data
  for (; it != mBodyNodes.rend(); ++it)
    (*it)->updateBiasImpulse();

  _bodyNode1->mConstraintImpulse.setZero();
  _bodyNode2->mConstraintImpulse.setZero();
}

//==============================================================================
void Skeleton::updateBiasImpulse(SoftBodyNode* _softBodyNode,
                                 PointMass* _pointMass,
                                 const Eigen::Vector3d& _imp)
{
  // Assertions
  assert(_softBodyNode != nullptr);
  assert(getNumDofs() > 0);

  // This skeleton should contain _bodyNode
  assert(std::find(mSoftBodyNodes.begin(), mSoftBodyNodes.end(), _softBodyNode)
         != mSoftBodyNodes.end());

#ifndef NDEBUG
  // All the constraint impulse should be zero
  for (size_t i = 0; i < mBodyNodes.size(); ++i)
    assert(mBodyNodes[i]->mConstraintImpulse == Eigen::Vector6d::Zero());
#endif

  // Set impulse to _bodyNode
  Eigen::Vector3d oldConstraintImpulse =_pointMass->getConstraintImpulses();
  _pointMass->setConstraintImpulse(_imp, true);

  // Prepare cache data
  BodyNode* it = _softBodyNode;
  while (it != nullptr)
  {
    it->updateBiasImpulse();
    it = it->getParentBodyNode();
  }

  // TODO(JS): Do we need to backup and restore the original value?
  _pointMass->setConstraintImpulse(oldConstraintImpulse);
}

//==============================================================================
void Skeleton::updateVelocityChange()
{
  for (auto& bodyNode : mBodyNodes)
    bodyNode->updateVelocityChangeFD();
}

//==============================================================================
void Skeleton::setImpulseApplied(bool _val)
{
  mIsImpulseApplied = _val;
}

//==============================================================================
bool Skeleton::isImpulseApplied() const
{
  return mIsImpulseApplied;
}

//==============================================================================
void Skeleton::computeImpulseForwardDynamics()
{
  // Skip immobile or 0-dof skeleton
  if (!isMobile() || getNumDofs() == 0)
    return;

  // Note: we do not need to update articulated inertias here, because they will
  // be updated when BodyNode::updateBiasImpulse() calls
  // BodyNode::getArticulatedInertia()

  // Backward recursion
  for (auto it = mBodyNodes.rbegin(); it != mBodyNodes.rend(); ++it)
    (*it)->updateBiasImpulse();

  // Forward recursion
  for (auto& bodyNode : mBodyNodes)
  {
    bodyNode->updateVelocityChangeFD();
    bodyNode->updateTransmittedImpulse();
    bodyNode->updateJointImpulseFD();
    bodyNode->updateConstrainedTerms(mSkeletonP.mTimeStep);
  }
}

//==============================================================================
void Skeleton::setConstraintForceVector(const Eigen::VectorXd& _Fc)
{
  mFc = _Fc;
}

//==============================================================================
double Skeleton::getKineticEnergy() const
{
  double KE = 0.0;

  for (std::vector<BodyNode*>::const_iterator it = mBodyNodes.begin();
       it != mBodyNodes.end(); ++it)
  {
    KE += (*it)->getKineticEnergy();
  }

  assert(KE >= 0.0 && "Kinetic energy should be positive value.");
  return KE;
}

//==============================================================================
double Skeleton::getPotentialEnergy() const
{
  double PE = 0.0;

  for (std::vector<BodyNode*>::const_iterator it = mBodyNodes.begin();
       it != mBodyNodes.end(); ++it)
  {
    PE += (*it)->getPotentialEnergy(mSkeletonP.mGravity);
    PE += (*it)->getParentJoint()->getPotentialEnergy();
  }

  return PE;
}

//==============================================================================
Eigen::Vector3d Skeleton::getCOM(const Frame* _withRespectTo) const
{
  Eigen::Vector3d com(0.0, 0.0, 0.0);

  const size_t numBodies = getNumBodyNodes();
  for (size_t i = 0; i < numBodies; ++i)
  {
    const BodyNode* bodyNode = getBodyNode(i);
    com += bodyNode->getMass() * bodyNode->getCOM(_withRespectTo);
  }

  assert(mTotalMass != 0.0);
  return com / mTotalMass;
}

//==============================================================================

// Templated function for computing different kinds of COM properties, like
// velocities and accelerations
template <
    typename PropertyType,
    PropertyType (BodyNode::*getPropertyFn)(const Frame*, const Frame*) const>
PropertyType getCOMPropertyTemplate(const Skeleton* _skel,
                                    const Frame* _relativeTo,
                                    const Frame* _inCoordinatesOf)
{
  PropertyType result(PropertyType::Zero());

  const size_t numBodies = _skel->getNumBodyNodes();
  for (size_t i = 0; i < numBodies; ++i)
  {
    const BodyNode* bodyNode = _skel->getBodyNode(i);
    result += bodyNode->getMass()
              * (bodyNode->*getPropertyFn)(_relativeTo, _inCoordinatesOf);
  }

  assert(_skel->getMass() != 0.0);
  return result / _skel->getMass();
}

//==============================================================================
Eigen::Vector6d Skeleton::getCOMSpatialVelocity(const Frame* _relativeTo,
    const Frame* _inCoordinatesOf) const
{
  return getCOMPropertyTemplate<Eigen::Vector6d,
      &BodyNode::getCOMSpatialVelocity>(this, _relativeTo, _inCoordinatesOf);
}

//==============================================================================
Eigen::Vector3d Skeleton::getCOMLinearVelocity(const Frame* _relativeTo,
    const Frame* _inCoordinatesOf) const
{
  return getCOMPropertyTemplate<Eigen::Vector3d,
      &BodyNode::getCOMLinearVelocity>(this, _relativeTo, _inCoordinatesOf);
}

//==============================================================================
Eigen::Vector6d Skeleton::getCOMSpatialAcceleration(const Frame* _relativeTo,
    const Frame* _inCoordinatesOf) const
{
  return getCOMPropertyTemplate<Eigen::Vector6d,
      &BodyNode::getCOMSpatialAcceleration>(this, _relativeTo, _inCoordinatesOf);
}

//==============================================================================
Eigen::Vector3d Skeleton::getCOMLinearAcceleration(const Frame* _relativeTo,
    const Frame* _inCoordinatesOf) const
{
  return getCOMPropertyTemplate<Eigen::Vector3d,
      &BodyNode::getCOMLinearAcceleration>(this, _relativeTo, _inCoordinatesOf);
}

//==============================================================================

// Templated function for computing different kinds of COM Jacobians and their
// derivatives
template <
    typename JacType, // JacType is the type of Jacobian we're computing
    JacType (BodyNode::*getJacFn)(const Eigen::Vector3d&, const Frame*) const>
JacType getCOMJacobianTemplate(const Skeleton* _skel,
                               const Frame* _inCoordinatesOf)
{
  // Initialize the Jacobian to zero
  JacType J = JacType::Zero(JacType::RowsAtCompileTime, _skel->getNumDofs());

  // Iterate through each of the Skeleton's BodyNodes
  const size_t numBodies = _skel->getNumBodyNodes();
  for (size_t i = 0; i < numBodies; ++i)
  {
    const BodyNode* bn = _skel->getBodyNode(i);

    // (bn->*getJacFn) is a function pointer to the function that gives us the
    // kind of Jacobian we want from the BodyNodes. Calling it will give us the
    // relevant Jacobian for this BodyNode
    JacType bnJ = bn->getMass() * (bn->*getJacFn)(bn->getLocalCOM(),
                                                  _inCoordinatesOf);

    // For each column in the Jacobian of this BodyNode, we add it to the
    // appropriate column of the overall BodyNode
    for (size_t j=0, end=bn->getNumDependentGenCoords(); j < end; ++j)
    {
      size_t idx = bn->getDependentGenCoordIndex(j);
      J.col(idx) += bnJ.col(j);
    }
  }

  assert(_skel->getMass() != 0.0);
  return J / _skel->getMass();
}

//==============================================================================
math::Jacobian Skeleton::getCOMJacobian(const Frame* _inCoordinatesOf) const
{
  return getCOMJacobianTemplate<math::Jacobian, &BodyNode::getJacobian>(
        this, _inCoordinatesOf);
}

//==============================================================================
math::LinearJacobian Skeleton::getCOMLinearJacobian(
    const Frame* _inCoordinatesOf) const
{
  return getCOMJacobianTemplate<math::LinearJacobian,
           &BodyNode::getLinearJacobian>(this, _inCoordinatesOf);
}

//==============================================================================
math::Jacobian Skeleton::getCOMJacobianSpatialDeriv(
    const Frame* _inCoordinatesOf) const
{
  return getCOMJacobianTemplate<math::Jacobian,
      &BodyNode::getJacobianSpatialDeriv>(this, _inCoordinatesOf);
}

//==============================================================================
math::LinearJacobian Skeleton::getCOMLinearJacobianDeriv(
    const Frame* _inCoordinatesOf) const
{
  return getCOMJacobianTemplate<math::LinearJacobian,
      &BodyNode::getLinearJacobianDeriv>(this, _inCoordinatesOf);
}

//==============================================================================
Eigen::Vector3d Skeleton::getWorldCOM()
{
  return getCOM(Frame::World());
}

//==============================================================================
Eigen::Vector3d Skeleton::getWorldCOMVelocity()
{
  return getCOMLinearVelocity();
}

//==============================================================================
Eigen::Vector3d Skeleton::getWorldCOMAcceleration()
{
  return getCOMLinearAcceleration();
}

//==============================================================================
Eigen::MatrixXd Skeleton::getWorldCOMJacobian()
{
  return getCOMLinearJacobian();
}

//==============================================================================
Eigen::MatrixXd Skeleton::getWorldCOMJacobianTimeDeriv()
{
  return getCOMLinearJacobianDeriv();
}

}  // namespace dynamics
}  // namespace dart
