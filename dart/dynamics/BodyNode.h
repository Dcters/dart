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

#ifndef DART_DYNAMICS_BODYNODE_H_
#define DART_DYNAMICS_BODYNODE_H_

#include <string>
#include <vector>

#include <Eigen/Dense>
#include <Eigen/StdVector>

#include "dart/config.h"
#include "dart/common/Deprecated.h"
#include "dart/common/sub_ptr.h"
#include "dart/common/Signal.h"
#include "dart/math/Geometry.h"

#include "dart/dynamics/Frame.h"
#include "dart/dynamics/Inertia.h"
#include "dart/dynamics/Skeleton.h"
#include "dart/dynamics/BodyNodePtr.h"
#include "dart/dynamics/Marker.h"

const double DART_DEFAULT_FRICTION_COEFF = 1.0;
const double DART_DEFAULT_RESTITUTION_COEFF = 0.0;

namespace dart {
namespace renderer {
class RenderInterface;
}  // namespace renderer
}  // namespace dart

namespace dart {
namespace dynamics {

class GenCoord;
class Skeleton;
class Joint;
class DegreeOfFreedom;
class Shape;
class Marker;

/// BodyNode class represents a single node of the skeleton.
///
/// BodyNode is a basic element of the skeleton. BodyNodes are hierarchically
/// connected and have a set of core functions for calculating derivatives.
///
/// BodyNode inherits Frame, and a parent Frame of a BodyNode is the parent
/// BodyNode of the BodyNode.
class BodyNode : public Frame, public SkeletonRefCountingBase
{
public:

  using ColShapeAddedSignal
      = common::Signal<void(const BodyNode*, ConstShapePtr _newColShape)>;

  using ColShapeRemovedSignal = ColShapeAddedSignal;

  using StructuralChangeSignal
      = common::Signal<void(const BodyNode*)>;

  struct UniqueProperties
  {
    /// Inertia information for the BodyNode
    Inertia mInertia;

    /// Array of collision shapes
    std::vector<ShapePtr> mColShapes;

    /// Indicates whether this node is collidable;
    bool mIsCollidable;

    /// Coefficient of friction
    double mFrictionCoeff;

    /// Coefficient of restitution
    double mRestitutionCoeff;

    /// Gravity will be applied if true
    bool mGravityMode;

    /// Properties of the Markers belonging to this BodyNode
    std::vector<Marker::Properties> mMarkerProperties;

    /// Constructor
    UniqueProperties(
        const Inertia& _inertia = Inertia(),
        const std::vector<ShapePtr>& _collisionShapes = std::vector<ShapePtr>(),
        bool _isCollidable = true,
        double _frictionCoeff = DART_DEFAULT_FRICTION_COEFF,
        double _restitutionCoeff = DART_DEFAULT_RESTITUTION_COEFF,
        bool _gravityMode = true);
  };

  /// Composition of Entity and BodyNode properties
  struct Properties : Entity::Properties, UniqueProperties
  {
    /// Composed constructor
    Properties(
        const Entity::Properties& _entityProperties = Entity::Properties("BodyNode"),
        const UniqueProperties& _bodyNodeProperties = UniqueProperties());
  };

  /// Destructor
  virtual ~BodyNode();

  /// Set the Properties of this BodyNode
  void setProperties(const Properties& _properties);

  /// Set the Properties of this BodyNode
  void setProperties(const UniqueProperties& _properties);

  /// Get the Properties of this BodyNode
  Properties getBodyNodeProperties() const;

  /// Copy the Properties of another BodyNode
  void copy(const BodyNode& _otherBodyNode);

  /// Copy the Properties of another BodyNode
  void copy(const BodyNode* _otherBodyNode);

  /// Same as copy(const BodyNode&)
  BodyNode& operator=(const BodyNode& _otherBodyNode);

  /// Set name. If the name is already taken, this will return an altered
  /// version which will be used by the Skeleton
  const std::string& setName(const std::string& _name);

  /// Return the name of the bodynode
  const std::string& getName() const;

  /// Set whether gravity affects this body
  /// \param[in] _gravityMode True to enable gravity
  void setGravityMode(bool _gravityMode);

  /// Return true if gravity mode is enabled
  bool getGravityMode() const;

  /// Return true if this body can collide with others bodies
  bool isCollidable() const;

  /// Set whether this body node will collide with others in the world
  /// \param[in] _isCollidable True to enable collisions
  void setCollidable(bool _isCollidable);

  /// Set the mass of the bodynode
  void setMass(double _mass);

  /// Return the mass of the bodynode
  double getMass() const;

  /// Set moment of inertia defined around the center of mass
  ///
  /// Principal moments of inertia (_Ixx, _Iyy, _Izz) must be positive or zero
  /// values.
  void setMomentOfInertia(
      double _Ixx, double _Iyy, double _Izz,
      double _Ixy = 0.0, double _Ixz = 0.0, double _Iyz = 0.0);

  /// Return moment of inertia defined around the center of mass
  void getMomentOfInertia(
      double& _Ixx, double& _Iyy, double& _Izz,
      double& _Ixy, double& _Ixz, double& _Iyz);

  /// Return spatial inertia
  const Eigen::Matrix6d& getSpatialInertia() const;

  /// Set the inertia data for this BodyNode
  void setInertia(const Inertia& _inertia);

  /// Get the inertia data for this BodyNode
  const Inertia& getInertia() const;

  /// Return the articulated body inertia
  const math::Inertia& getArticulatedInertia() const;

  /// Return the articulated body inertia for implicit joint damping and spring
  /// forces
  const math::Inertia& getArticulatedInertiaImplicit() const;

  /// Set center of mass expressed in body frame
  void setLocalCOM(const Eigen::Vector3d& _com);

  /// Return center of mass expressed in body frame
  const Eigen::Vector3d& getLocalCOM() const;

  /// Return center of mass expressed in world frame
  ///
  /// Deprecated in 4.4. Please use getCOM()
  DEPRECATED(4.4)
  Eigen::Vector3d getWorldCOM() const;

  /// Return velocity of center of mass expressed in world frame
  ///
  /// Deprecated in 4.4. Please use getCOMLinearVelocity() or
  /// getCOMSpatialVelocity()
  DEPRECATED(4.4)
  Eigen::Vector3d getWorldCOMVelocity() const;

  /// Return acceleration of center of mass expressed in world frame
  ///
  /// Deprecated in 4.4. Please use getCOMLinearAcceleration() or
  /// getCOMSpatialAcceleration()
  DEPRECATED(4.4)
  Eigen::Vector3d getWorldCOMAcceleration() const;

  /// Return the center of mass with respect to an arbitrary Frame
  Eigen::Vector3d getCOM(const Frame* _withRespectTo = Frame::World()) const;

  /// Return the linear velocity of the center of mass, expressed in terms of
  /// arbitrary Frames
  Eigen::Vector3d getCOMLinearVelocity(
                          const Frame* _relativeTo = Frame::World(),
                          const Frame* _inCoordinatesOf = Frame::World()) const;

  /// Return the spatial velocity of the center of mass, expressed in
  /// coordinates of this Frame and relative to the World Frame
  Eigen::Vector6d getCOMSpatialVelocity() const;

  /// Return the spatial velocity of the center of mass, expressed in terms of
  /// arbitrary Frames
  Eigen::Vector6d getCOMSpatialVelocity(const Frame* _relativeTo,
                                        const Frame* _inCoordinatesOf) const;

  /// Return the linear acceleration of the center of mass, expressed in terms
  /// of arbitary Frames
  Eigen::Vector3d getCOMLinearAcceleration(
                          const Frame* _relativeTo = Frame::World(),
                          const Frame* _inCoordinatesOf = Frame::World()) const;

  /// Return the acceleration of the center of mass expressed in coordinates of
  /// this BodyNode Frame and relative to the World Frame
  Eigen::Vector6d getCOMSpatialAcceleration() const;

  /// Return the spatial acceleration of the center of mass, expressed in terms
  /// of arbitrary Frames
  Eigen::Vector6d getCOMSpatialAcceleration(const Frame* _relativeTo,
                                            const Frame* _inCoordinatesOf) const;

  /// Set coefficient of friction in range of [0, ~]
  void setFrictionCoeff(double _coeff);

  /// Return frictional coefficient.
  double getFrictionCoeff() const;

  /// Set coefficient of restitution in range of [0, 1]
  void setRestitutionCoeff(double _coeff);

  /// Return coefficient of restitution
  double getRestitutionCoeff() const;

  //--------------------------------------------------------------------------
  // Structural Properties
  //--------------------------------------------------------------------------

  /// Add a collision Shape into the BodyNode
  void addCollisionShape(ShapePtr _shape);

  /// Remove a collision Shape from this BodyNode
  void removeCollisionShape(ShapePtr _shape);

  /// Remove all collision Shapes from this BodyNode
  void removeAllCollisionShapes();

  /// Return the number of collision shapes
  size_t getNumCollisionShapes() const;

  /// Return _index-th collision shape
  ShapePtr getCollisionShape(size_t _index);

  /// Return (const) _index-th collision shape
  ConstShapePtr getCollisionShape(size_t _index) const;

  /// Return the index of this BodyNode within its Skeleton
  size_t getIndexInSkeleton() const;

  /// Return the index of this BodyNode within its tree
  size_t getIndexInTree() const;

  /// Return the index of the tree that this BodyNode belongs to
  size_t getTreeIndex() const;

  /// Remove this BodyNode and all of its children (recursively) from their
  /// Skeleton. If a BodyNodePtr that references this BodyNode (or any of its
  /// children) still exists, the subtree will be moved into a new Skeleton
  /// with the given name. If the returned SkeletonPtr goes unused and no
  /// relevant BodyNodePtrs are held anywhere, then this BodyNode and all its
  /// children will be deleted.
  ///
  /// Note that this function is actually the same as split(), but given a
  /// different name for semantic reasons.
  SkeletonPtr remove(const std::string& _name = "temporary");

  /// Remove this BodyNode and all of its children (recursively) from their
  /// current parent BodyNode, and move them to another parent BodyNode. The new
  /// parent BodyNode can either be in a new Skeleton or the current one. If you
  /// pass in a nullptr, this BodyNode will become a new root BodyNode for its
  /// current Skeleton.
  ///
  /// Using this function will result in changes to the indexing of
  /// (potentially) all BodyNodes and Joints in the current Skeleton, even if
  /// the BodyNodes are kept within the same Skeleton.
  bool moveTo(BodyNode* _newParent);

  /// This is a version of moveTo(BodyNode*) that allows you to explicitly move
  /// this BodyNode into a different Skeleton. The key difference for this
  /// version of the function is that you can make this BodyNode a root node in
  /// a different Skeleton, which is not something that can be done by the other
  /// version.
  bool moveTo(const SkeletonPtr& _newSkeleton, BodyNode* _newParent);

  /// A version of moveTo(BodyNode*) that also changes the Joint type of the
  /// parent Joint of this BodyNode. This function returns the pointer to the
  /// newly created Joint. The original parent Joint will be deleted.
  ///
  /// This function can be used to change the Joint type of the parent Joint of
  /// this BodyNode, but note that the indexing of the BodyNodes and Joints in
  /// this Skeleton will still be changed, even if only the Joint type is
  /// changed.
  template <class JointType>
  JointType* moveTo(BodyNode* _newParent,
      const typename JointType::Properties& _joint =
      typename JointType::Properties());

  /// A version of moveTo(SkeletonPtr, BodyNode*) that also changes the Joint
  /// type of the parent Joint of this BodyNode. This function returns the
  /// pointer to the newly created Joint. The original Joint will be deleted.
  template <class JointType>
  JointType* moveTo(const SkeletonPtr& _newSkeleton, BodyNode* _newParent,
      const typename JointType::Properties& _joint =
          typename JointType::Properties());

  /// Remove this BodyNode and all of its children (recursively) from their
  /// current Skeleton and move them into a newly created Skeleton. The newly
  /// created Skeleton will have the same Skeleton::Properties as the current
  /// Skeleton, except it will use the specified name. The return value is a
  /// shared_ptr to the newly created Skeleton.
  ///
  /// Note that the parent Joint of this BodyNode will remain the same. If you
  /// want to change the Joint type of this BodyNode's parent Joint (for
  /// example, make it a FreeJoint), then use the templated split<JointType>()
  /// function.
  SkeletonPtr split(const std::string& _skeletonName);

  /// A version of split(const std::string&) that also changes the Joint type of
  /// the parent Joint of this BodyNode.
  template <class JointType>
  SkeletonPtr split(const std::string& _skeletonName,
      const typename JointType::Properties& _joint =
      typename JointType::Properties());

  /// Change the Joint type of this BodyNode's parent Joint.
  ///
  /// Note that this function will change the indexing of (potentially) all
  /// BodyNodes and Joints in the Skeleton.
  template <class JointType>
  JointType* changeParentJointType(
      const typename JointType::Properties& _joint =
      typename JointType::Properties());

  /// Create clones of this BodyNode and all of its children recursively (unless
  /// _recursive is set to false) and attach the clones to the specified
  /// BodyNode. The specified BodyNode can be in this Skeleton or a different
  /// Skeleton. Passing in nullptr will set the copy as a root node of the
  /// current Skeleton.
  ///
  /// The return value is a pair of pointers to the root of the newly created
  /// BodyNode tree.
  std::pair<Joint*, BodyNode*> copyTo(BodyNode* _newParent,
                                      bool _recursive=true);

  /// Create clones of this BodyNode and all of its children recursively (unless
  /// recursive is set to false) and attach the clones to the specified BodyNode
  /// of the specified Skeleton.
  ///
  /// The key differences between this function and the copyTo(BodyNode*)
  /// version is that this one allows the copied BodyNode to be const and allows
  /// you to copy it as a root node of another Skeleton.
  ///
  /// The return value is a pair of pointers to the root of the newly created
  /// BodyNode tree.
  std::pair<Joint*, BodyNode*> copyTo(const SkeletonPtr& _newSkeleton,
                                      BodyNode* _newParent,
                                      bool _recursive=true) const;

  /// A version of copyTo(BodyNode*) that also changes the Joint type of the
  /// parent Joint of this BodyNode.
  template <class JointType>
  std::pair<JointType*, BodyNode*> copyTo(
      BodyNode* _newParent,
      const typename JointType::Properties& _joint =
          typename JointType::Properties(),
      bool _recursive=true);

  /// A version of copyTo(Skeleton*,BodyNode*) that also changes the Joint type
  /// of the parent Joint of this BodyNode.
  template <class JointType>
  std::pair<JointType*, BodyNode*> copyTo(
      const SkeletonPtr& _newSkeleton, BodyNode* _newParent,
      const typename JointType::Properties& _joint =
          typename JointType::Properties(),
      bool _recursive=true) const;

  /// Create clones of this BodyNode and all of its children (recursively) and
  /// create a new Skeleton with the specified name to attach them to.
  SkeletonPtr copyAs(const std::string& _skeletonName,
                     bool _recursive=true) const;

  /// A version of copyAs(const std::string&) that also changes the Joint type
  /// of the root BodyNode.
  template <class JointType>
  SkeletonPtr copyAs(
      const std::string& _skeletonName,
      const typename JointType::Properties& _joint =
          typename JointType::Properties(),
      bool _recursive=true) const;

  /// Return the parent Joint of this BodyNode
  Joint* getParentJoint();

  /// Return the (const) parent Joint of this BodyNode
  const Joint* getParentJoint() const;

  /// Return the parent BodyNdoe of this BodyNode
  BodyNode* getParentBodyNode();

  /// Return the (const) parent BodyNode of this BodyNode
  const BodyNode* getParentBodyNode() const;

  /// Create a Joint and BodyNode pair as a child of this BodyNode
  template <class JointType, class NodeType = BodyNode>
  std::pair<JointType*, NodeType*> createChildJointAndBodyNodePair(
      const typename JointType::Properties& _jointProperties =
          typename JointType::Properties(),
      const typename NodeType::Properties& _bodyProperties =
          typename NodeType::Properties())
  {
    return getSkeleton()->createJointAndBodyNodePair<JointType, NodeType>(
          this, _jointProperties, _bodyProperties);
  }

  /// Return the number of child BodyNodes
  size_t getNumChildBodyNodes() const;

  /// Return the _index-th child BodyNode of this BodyNode
  BodyNode* getChildBodyNode(size_t _index);

  /// Return the (const) _index-th child BodyNode of this BodyNode
  const BodyNode* getChildBodyNode(size_t _index) const;

  /// Return the number of child Joints
  size_t getNumChildJoints() const;

  /// Return the _index-th child Joint of this BodyNode
  Joint* getChildJoint(size_t _index);

  /// Return the (const) _index-th child Joint of this BodyNode
  const Joint* getChildJoint(size_t _index) const;

  /// Add a marker into the bodynode
  void addMarker(Marker* _marker);

  /// Return the number of markers of the bodynode
  size_t getNumMarkers() const;

  /// Return _index-th marker of the bodynode
  Marker* getMarker(size_t _index);

  /// Return (const) _index-th marker of the bodynode
  const Marker* getMarker(size_t _index) const;

  /// Return true if _genCoordIndex-th generalized coordinate
  bool dependsOn(size_t _genCoordIndex) const;

  /// The number of the generalized coordinates by which this node is affected
  size_t getNumDependentGenCoords() const;

  /// Return a generalized coordinate index from the array index
  /// (< getNumDependentDofs)
  size_t getDependentGenCoordIndex(size_t _arrayIndex) const;

  /// Return a std::vector of generalized coordinate indices that this BodyNode
  /// depends on.
  const std::vector<size_t>& getDependentGenCoordIndices() const;

  /// Return a std::vector of DegreeOfFreedom pointers that this BodyNode
  /// depends on.
  const std::vector<DegreeOfFreedom*>& getDependentDofs();

  /// Return a std::vector of DegreeOfFreedom pointers that this BodyNode
  /// depends on.
  const std::vector<const DegreeOfFreedom*>& getDependentDofs() const;

  //--------------------------------------------------------------------------
  // Properties updated by dynamics (kinematics)
  //--------------------------------------------------------------------------

  /// Get the transform of this BodyNode with respect to its parent BodyNode,
  /// which is also its parent Frame.
  const Eigen::Isometry3d& getRelativeTransform() const;

  // Documentation inherited
  const Eigen::Vector6d& getRelativeSpatialVelocity() const;

  // Documentation inherited
  const Eigen::Vector6d& getRelativeSpatialAcceleration() const override;

  // Documentation inherited
  const Eigen::Vector6d& getPrimaryRelativeAcceleration() const override;

  /// Return the partial acceleration of this body
  const Eigen::Vector6d& getPartialAcceleration() const override;

  /// Return the generalized Jacobian targeting the origin of this BodyNode. The
  /// Jacobian is expressed in the Frame of this BodyNode.
  const math::Jacobian& getJacobian() const;

  /// A version of getJacobian() that lets you specify a coordinate Frame to
  /// express the Jacobian in.
  math::Jacobian getJacobian(const Frame* _inCoordinatesOf) const;

  /// Return the generalized Jacobian targeting an offset within the Frame of
  /// this BodyNode.
  math::Jacobian getJacobian(const Eigen::Vector3d& _offset) const;

  /// A version of getJacobian(const Eigen::Vector3d&) that lets you specify a
  /// coordinate Frame to express the Jacobian in.
  math::Jacobian getJacobian(const Eigen::Vector3d& _offset,
                             const Frame* _inCoordinatesOf) const;

  /// Return the generalized Jacobian targeting the origin of this BodyNode. The
  /// Jacobian is expressed in the World Frame.
  const math::Jacobian& getWorldJacobian() const;

  /// Return the generalized Jacobian targeting an offset in this BodyNode. The
  /// _offset is expected in coordinates of this BodyNode Frame. The Jacobian is
  /// expressed in the World Frame.
  math::Jacobian getWorldJacobian(const Eigen::Vector3d& _offset) const;

  /// Return the linear Jacobian targeting the origin of this BodyNode. You can
  /// specify a coordinate Frame to express the Jacobian in.
  math::LinearJacobian getLinearJacobian(
                          const Frame* _inCoordinatesOf = Frame::World()) const;

  /// Return the generalized Jacobian targeting an offset within the Frame of
  /// this BodyNode.
  math::LinearJacobian getLinearJacobian(const Eigen::Vector3d& _offset,
                          const Frame* _inCoordinatesOf = Frame::World()) const;

  /// Return the angular Jacobian targeting the origin of this BodyNode. You can
  /// specify a coordinate Frame to express the Jacobian in.
  math::AngularJacobian getAngularJacobian(
                          const Frame* _inCoordinatesOf = Frame::World()) const;

  /// Return the spatial time derivative of the generalized Jacobian targeting
  /// the origin of this BodyNode. The Jacobian is expressed in this BodyNode's
  /// coordinate Frame.
  ///
  /// NOTE: Since this is a spatial time derivative, it should be used with
  /// spatial vectors. If you are using classical linear and angular
  /// acceleration vectors, then use getJacobianClassicDeriv(),
  /// getLinearJacobianDeriv(), or getAngularJacobianDeriv() instead.
  const math::Jacobian& getJacobianSpatialDeriv() const;

  /// A version of getJacobianSpatialDeriv() that can return the Jacobian in
  /// coordinates of any Frame.
  ///
  /// NOTE: This Jacobian Derivative is only for use with spatial vectors. If
  /// you are using classical linear and angular vectors, then use
  /// getJacobianClassicDeriv(), getLinearJacobianDeriv(), or
  /// getAngularJacobianDeriv() instead.
  math::Jacobian getJacobianSpatialDeriv(const Frame* _inCoordinatesOf) const;

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
  math::Jacobian getJacobianSpatialDeriv(const Eigen::Vector3d& _offset) const;

  /// A version of getJacobianSpatialDeriv(const Eigen::Vector3d&) that allows
  /// an arbitrary coordinate Frame to be specified.
  math::Jacobian getJacobianSpatialDeriv(const Eigen::Vector3d& _offset,
                                         const Frame* _inCoordinatesOf) const;

  /// Return the classical time derivative of the generalized Jacobian targeting
  /// the origin of this BodyNode. The Jacobian is expressed in the World
  /// coordinate Frame.
  ///
  /// NOTE: Since this is a classical time derivative, it should be used with
  /// classical linear and angular vectors. If you are using spatial vectors,
  /// use getJacobianSpatialDeriv() instead.
  const math::Jacobian& getJacobianClassicDeriv() const;

  /// A version of getJacobianClassicDeriv() that can return the Jacobian in
  /// coordinates of any Frame.
  ///
  /// NOTE: Since this is a classical time derivative, it should be used with
  /// classical linear and angular vectors. If you are using spatial vectors,
  /// use getJacobianSpatialDeriv() instead.
  math::Jacobian getJacobianClassicDeriv(const Frame* _inCoordinatesOf) const;

  /// A version of getJacobianClassicDeriv() that can compute the Jacobian for
  /// an offset within the Frame of this BodyNode. The offset must be expressed
  /// in the coordinates of this BodyNode Frame.
  ///
  /// NOTE: Since this is a classical time derivative, it should be used with
  /// classical linear and angular vectors. If you are using spatial vectors,
  /// use getJacobianSpatialDeriv() instead.
  math::Jacobian getJacobianClassicDeriv(const Eigen::Vector3d& _offset,
                          const Frame* _inCoordinatesOf = Frame::World()) const;

  /// Return the linear Jacobian (classical) time derivative, in terms of any
  /// coordinate Frame.
  ///
  /// NOTE: Since this is a classical time derivative, it should be used with
  /// classical linear vectors. If you are using spatial vectors, use
  /// getJacobianSpatialDeriv() instead.
  math::LinearJacobian getLinearJacobianDeriv(
                          const Frame* _inCoordinatesOf = Frame::World()) const;

  /// A version of getLinearJacobianDeriv() that can compute the Jacobian for
  /// an offset within the Frame of this BodyNode. The offset must be expressed
  /// in coordinates of this BodyNode Frame.
  ///
  /// NOTE: Since this is a classical time derivative, it should be used with
  /// classical linear vectors. If you are using spatial vectors, use
  /// getJacobianSpatialDeriv() instead.
  math::LinearJacobian getLinearJacobianDeriv(
                          const Eigen::Vector3d& _offset,
                          const Frame* _inCoordinatesOf = Frame::World()) const;

  /// Return the angular Jacobian time derivative, in terms of any coordinate
  /// Frame.
  math::AngularJacobian getAngularJacobianDeriv(
                          const Frame* _inCoordinatesOf = Frame::World()) const;

  //----------------------------------------------------------------------------
  // Deprecated velocity, acceleration, and Jacobian functions
  //----------------------------------------------------------------------------

  /// Return the spatial velocity at the origin of the bodynode expressed in
  /// the body-fixed frame
  ///
  /// Deprecated in 4.4. Please use getSpatialVelocity() instead.
  DEPRECATED(4.4)
  const Eigen::Vector6d& getBodyVelocity() const;

  /// Return the linear velocity of the origin of the bodynode expressed in
  /// body-fixed frame
  ///
  /// Deprecated as of 4.4
  /// \sa getSpatialVelocity(), getLinearVelocity()
  DEPRECATED(4.4)
  Eigen::Vector3d getBodyLinearVelocity() const;

  /// Return the linear velocity of a point on the bodynode expressed in
  /// body-fixed frame
  /// \param[in] _offset Offset of the point from the origin of the bodynode
  /// frame. The offset is expressed in the body-fixed frame
  ///
  /// Deprecated as of 4.4
  /// \sa getSpatialVelocity(), getLinearVelocity()
  DEPRECATED(4.4)
  Eigen::Vector3d getBodyLinearVelocity(const Eigen::Vector3d& _offset) const;

  /// Return the angular velocity expressed in body-fixed frame
  ///
  /// Deprecated as of 4.4
  /// \sa getSpatialVelocity(), getAngularVelocity()
  DEPRECATED(4.4)
  Eigen::Vector3d getBodyAngularVelocity() const;

  /// Return the linear velocity of the origin of the bodynode expressed in
  /// world frame
  ///
  /// Deprecated as of 4.4
  /// \sa getSpatialVelocity(), getLinearVelocity()
  DEPRECATED(4.4)
  Eigen::Vector3d getWorldLinearVelocity() const;

  /// Return the linear velocity of a point on the bodynode expressed in world
  /// frame
  /// \param[in] _offset Offset of the point from the origin of the bodynode
  /// frame. The offset is expressed in the body-fixed frame
  ///
  /// Deprecated as of 4.4
  /// \sa getSpatialVelocity(), getLinearVelocity()
  DEPRECATED(4.4)
  Eigen::Vector3d getWorldLinearVelocity(const Eigen::Vector3d& _offset) const;

  /// Return the angular velocity expressed in world frame
  ///
  /// Deprecated as of 4.4
  /// \sa getSpatialVelocity(), getAngularVelocity()
  DEPRECATED(4.4)
  Eigen::Vector3d getWorldAngularVelocity() const;

  /// Return generalized acceleration at the origin of this body node where the
  /// acceleration is expressed in this body node frame
  ///
  /// Deprecated as of 4.4
  /// \sa getSpatialAcceleration()
  DEPRECATED(4.4)
  const Eigen::Vector6d& getBodyAcceleration() const;

  /// Return the linear acceleration of the origin of the bodynode expressed in
  /// body-fixed frame
  ///
  /// Deprecated as of 4.4
  /// \sa getSpatialAcceleration(), getLinearAcceleration()
  DEPRECATED(4.4)
  Eigen::Vector3d getBodyLinearAcceleration() const;

  /// Return the linear acceleration of a point on the bodynode expressed in
  /// body-fixed frame
  /// \param[in] _offset Offset of the point from the origin of the bodynode
  /// frame. The offset is expressed in the body-fixed frame
  ///
  /// Deprecated as of 4.4
  /// \sa getSpatialAcceleration(), getLinearAcceleration()
  DEPRECATED(4.4)
  Eigen::Vector3d getBodyLinearAcceleration(
      const Eigen::Vector3d& _offset) const;

  /// Return the angular acceleration expressed in body-fixed frame
  ///
  /// Deprecated as of 4.4
  /// \sa getSpatialAcceleration(), getAngularAcceleration()
  DEPRECATED(4.4)
  Eigen::Vector3d getBodyAngularAcceleration() const;

  /// Return the linear acceleration of the origin of the bodynode expressed in
  /// world frame
  ///
  /// Deprecated as of 4.4
  /// \sa getSpatialAcceleration(), getLinearAcceleration()
  DEPRECATED(4.4)
  Eigen::Vector3d getWorldLinearAcceleration() const;

  /// Return the linear acceleration of a point on the bodynode expressed in
  /// world frame
  /// \param[in] _offset Offset of the point from the origin of the bodynode
  /// frame. The offset is expressed in the body-fixed frame
  ///
  /// Deprecated as of 4.4
  /// \sa getSpatialAcceleration(), getLinearAcceleration()
  DEPRECATED(4.4)
  Eigen::Vector3d getWorldLinearAcceleration(
      const Eigen::Vector3d& _offset) const;

  /// Return the angular acceleration expressed in world frame
  ///
  /// Deprecated as of 4.4
  /// \sa getSpatialAcceleration(), getAngularAcceleration()
  DEPRECATED(4.4)
  Eigen::Vector3d getWorldAngularAcceleration() const;

  /// Return generalized Jacobian at the origin of this body node where the
  /// Jacobian is expressed in this body node frame
  ///
  /// Deprecated as of 4.4
  /// \sa getJacobian()
  DEPRECATED(4.4)
  const math::Jacobian& getBodyJacobian();

  /// Return the linear Jacobian of the origin of the bodynode expressed in
  /// body-fixed frame
  ///
  /// Deprecated as of 4.4
  /// \sa getJacobian(), getLinearJacobian()
  DEPRECATED(4.4)
  math::LinearJacobian getBodyLinearJacobian();

  /// Return the linear Jacobian of a point on the bodynode expressed in
  /// body-fixed frame
  /// \param[in] _offset Offset of the point from the origin of the bodynode
  /// frame. The offset is expressed in the body-fixed frame
  ///
  /// Deprecated as of 4.4
  /// \sa getJacobian(), getLinearJacobian()
  DEPRECATED(4.4)
  math::LinearJacobian getBodyLinearJacobian(const Eigen::Vector3d& _offset);

  /// Return the angular Jacobian expressed in body-fixed frame
  ///
  /// Deprecated as of 4.4
  /// \sa getJacobian(), getAngularJacobian()
  DEPRECATED(4.4)
  math::AngularJacobian getBodyAngularJacobian();

  /// Return the linear Jacobian of the origin of the bodynode expressed in
  /// world frame
  ///
  /// Deprecated as of 4.4
  /// \sa getJacobian(), getLinearJacobian()
  DEPRECATED(4.4)
  math::LinearJacobian getWorldLinearJacobian();

  /// Return the linear Jacobian of a point on the bodynode expressed in world
  /// frame
  /// \param[in] _offset Offset of the point from the origin of the bodynode
  /// frame. The offset is expressed in the body-fixed frame
  ///
  /// Deprecated as of 4.4
  /// \sa getJacobian(), getLinearJacobian()
  DEPRECATED(4.4)
  math::LinearJacobian getWorldLinearJacobian(const Eigen::Vector3d& _offset);

  /// Return the angular Jacobian expressed in world frame
  ///
  /// Deprecated as of 4.4
  /// \sa getJacobian(), getAngularJacobian()
  DEPRECATED(4.4)
  math::AngularJacobian getWorldAngularJacobian();

  /// Return time derivative of generalized Jacobian at the origin of this body
  /// node where the Jacobian is expressed in this body node frame
  ///
  /// Deprecated as of 4.4. Replaced by getJacobianSpatialDeriv()
  /// \sa getJacobianSpatialDeriv(), getJacobianClassicDeriv()
  DEPRECATED(4.4)
  const math::Jacobian& getBodyJacobianDeriv();

  /// Return the time derivative of the linear Jacobian of the origin of the
  /// bodynode expressed in body-fixed frame
  ///
  /// Deprecated as of 4.4
  /// \sa getJacobianSpatialDeriv(), getLinearJacobianClassicDeriv()
  DEPRECATED(4.4)
  math::LinearJacobian getBodyLinearJacobianDeriv();

  /// Return the time derivative of the linear Jacobian of a point on the
  /// bodynode expressed in body-fixed frame
  /// \param[in] _offset Offset of the point from the origin of the bodynode
  /// frame. The offset is expressed in the body-fixed frame
  ///
  /// Deprecated as of 4.4
  /// \sa getJacobianSpatialDeriv(), getLinearJacobianClassicDeriv()
  DEPRECATED(4.4)
  math::LinearJacobian getBodyLinearJacobianDeriv(
      const Eigen::Vector3d& _offset);

  /// Return the time derivative of the angular Jacobian expressed in body-fixed
  /// frame
  ///
  /// Deprecated as of 4.4
  /// \sa getJacobianSpatialDeriv(), getAngularJacobianClassicDeriv()
  DEPRECATED(4.4)
  math::AngularJacobian getBodyAngularJacobianDeriv();

  /// Return the time derivative of the linear Jacobian of the origin of the
  /// bodynode expressed in world frame
  ///
  /// Deprecated as of 4.4
  /// \sa getJacobianSpatialDeriv(), getLinearJacobianClassicDeriv()
  DEPRECATED(4.4)
  math::LinearJacobian getWorldLinearJacobianDeriv();

  /// Return the time derivative of the linear Jacobian of a point on the
  /// bodynode expressed in world frame
  /// \param[in] _offset Offset of the point from the origin of the bodynode
  /// frame. The offset is expressed in the body-fixed frame
  ///
  /// Deprecated as of 4.4
  /// \sa getJacobianSpatialDeriv(), getLinearJacobianClassicDeriv()
  DEPRECATED(4.4)
  math::LinearJacobian getWorldLinearJacobianDeriv(
      const Eigen::Vector3d& _offset);

  /// Return the angular Jacobian expressed in world frame
  ///
  /// Deprecated as of 4.4
  /// \sa getJacobianSpatialDeriv(), getAngularJacobianClassicDeriv()
  DEPRECATED(4.4)
  math::AngularJacobian getWorldAngularJacobianDeriv();

  // --- End of Deprecated functions -------------------------------------------

  /// Return the velocity change due to the constraint impulse
  const Eigen::Vector6d& getBodyVelocityChange() const;

  /// Set whether this body node is colliding with others. This is called by
  /// collision detector.
  /// \param[in] True if this body node is colliding.
  void setColliding(bool _isColliding);

  /// Return whether this body node is colliding with others
  /// \return True if this body node is colliding.
  bool isColliding();

  /// Add applying linear Cartesian forces to this node
  ///
  /// A force is defined by a point of application and a force vector. The
  /// last two parameters specify frames of the first two parameters.
  /// Coordinate transformations are applied when needed. The point of
  /// application and the force in local coordinates are stored in mContacts.
  /// When conversion is needed, make sure the transformations are avaialble.
  void addExtForce(const Eigen::Vector3d& _force,
                   const Eigen::Vector3d& _offset = Eigen::Vector3d::Zero(),
                   bool _isForceLocal = false,
                   bool _isOffsetLocal = true);

  /// Set Applying linear Cartesian forces to this node.
  void setExtForce(const Eigen::Vector3d& _force,
                   const Eigen::Vector3d& _offset = Eigen::Vector3d::Zero(),
                   bool _isForceLocal = false,
                   bool _isOffsetLocal = true);

  /// Add applying Cartesian torque to the node.
  ///
  /// The torque in local coordinates is accumulated in mExtTorqueBody.
  void addExtTorque(const Eigen::Vector3d& _torque, bool _isLocal = false);

  /// Set applying Cartesian torque to the node.
  ///
  /// The torque in local coordinates is accumulated in mExtTorqueBody.
  void setExtTorque(const Eigen::Vector3d& _torque, bool _isLocal = false);

  /// Clean up structures that store external forces: mContacts, mFext,
  /// mExtForceBody and mExtTorqueBody.
  ///
  /// Called by Skeleton::clearExternalForces.
  virtual void clearExternalForces();

  /// Clear out the generalized forces of the parent Joint and any other forces
  /// related to this BodyNode that are internal to the Skeleton. For example,
  /// the point mass forces for SoftBodyNodes.
  virtual void clearInternalForces();

  ///
  const Eigen::Vector6d& getExternalForceLocal() const;

  ///
  Eigen::Vector6d getExternalForceGlobal() const;

  /// Get spatial body force transmitted from the parent joint.
  ///
  /// The spatial body force is transmitted to this BodyNode from the parent
  /// body through the connecting joint. It is expressed in this BodyNode's
  /// frame.
  const Eigen::Vector6d& getBodyForce() const;

  //----------------------------------------------------------------------------
  // Constraints
  //   - Following functions are managed by constraint solver.
  //----------------------------------------------------------------------------

  /// Deprecated in 4.2. Please use isReactive().
  DEPRECATED(4.2)
  bool isImpulseReponsible() const;

  /// Return true if the body can react to force or constraint impulse.
  ///
  /// A body node is reactive if the skeleton is mobile and the number of
  /// dependent generalized coordinates is non zero.
  bool isReactive() const;

  /// Set constraint impulse
  /// \param[in] _constImp Spatial constraint impulse w.r.t. body frame
  void setConstraintImpulse(const Eigen::Vector6d& _constImp);

  /// Add constraint impulse
  /// \param[in] _constImp Spatial constraint impulse w.r.t. body frame
  void addConstraintImpulse(const Eigen::Vector6d& _constImp);

  /// Add constraint impulse
  void addConstraintImpulse(const Eigen::Vector3d& _constImp,
                            const Eigen::Vector3d& _offset,
                            bool _isImpulseLocal = false,
                            bool _isOffsetLocal = true);

  /// Clear constraint impulse
  virtual void clearConstraintImpulse();

  /// Return constraint impulse
  const Eigen::Vector6d& getConstraintImpulse() const;

  //----------------------------------------------------------------------------
  // Energies
  //----------------------------------------------------------------------------

  /// Return kinetic energy.
  virtual double getKineticEnergy() const;

  /// Return potential energy.
  virtual double getPotentialEnergy(const Eigen::Vector3d& _gravity) const;

  /// Return linear momentum.
  Eigen::Vector3d getLinearMomentum() const;

  /// Return angular momentum.
  Eigen::Vector3d getAngularMomentum(
      const Eigen::Vector3d& _pivot = Eigen::Vector3d::Zero());

  //----------------------------------------------------------------------------
  // Rendering
  //----------------------------------------------------------------------------

  /// Render the markers
  void drawMarkers(renderer::RenderInterface* _ri = NULL,
                   const Eigen::Vector4d& _color = Eigen::Vector4d::Ones(),
                   bool _useDefaultColor = true) const;

  //----------------------------------------------------------------------------
  // Notifications
  //----------------------------------------------------------------------------

  // Documentation inherited
  void notifyTransformUpdate() override;

  // Documentation inherited
  void notifyVelocityUpdate() override;

  // Documentation inherited
  void notifyAccelerationUpdate() override;

  /// Notify the Skeleton that the tree of this BodyNode needs an articulated
  /// inertia update
  void notifyArticulatedInertiaUpdate();

  /// Tell the Skeleton that the external forces need to be updated
  void notifyExternalForcesUpdate();

  /// Tell the Skeleton that the coriolis forces need to be update
  void notifyCoriolisUpdate();

  //----------------------------------------------------------------------------
  // Friendship
  //----------------------------------------------------------------------------

  friend class Skeleton;
  friend class Joint;
  friend class SoftBodyNode;
  friend class PointMass;

protected:

  /// Constructor called by Skeleton class
  BodyNode(BodyNode* _parentBodyNode, Joint* _parentJoint,
           const Properties& _properties);

  /// Create a clone of this BodyNode. This may only be called by the Skeleton
  /// class.
  virtual BodyNode* clone(BodyNode* _parentBodyNode, Joint* _parentJoint) const;

  /// Initialize the vector members with proper sizes.
  virtual void init(const SkeletonPtr& _skeleton);

  /// Add a child bodynode into the bodynode
  void addChildBodyNode(BodyNode* _body);

  //----------------------------------------------------------------------------
  /// \{ \name Recursive dynamics routines
  //----------------------------------------------------------------------------

  /// Separate generic child Entities from child BodyNodes for more efficient
  /// update notices
  void processNewEntity(Entity* _newChildEntity);

  /// Remove this Entity from mChildBodyNodes or mNonBodyNodeEntities
  void processRemovedEntity(Entity* _oldChildEntity);

  /// Update transformation
  virtual void updateTransform();

  /// Update spatial body velocity.
  virtual void updateVelocity();

  /// Update partial spatial body acceleration due to parent joint's velocity.
  virtual void updatePartialAcceleration() const;

  /// Update articulated body inertia for forward dynamics.
  /// \param[in] _timeStep Rquired for implicit joint stiffness and damping.
  virtual void updateArtInertia(double _timeStep) const;

  /// Update bias force associated with the articulated body inertia for forward
  /// dynamics.
  /// \param[in] _gravity Vector of gravitational acceleration
  /// \param[in] _timeStep Rquired for implicit joint stiffness and damping.
  virtual void updateBiasForce(const Eigen::Vector3d& _gravity,
                               double _timeStep);

  /// Update bias impulse associated with the articulated body inertia for
  /// impulse-based forward dynamics.
  virtual void updateBiasImpulse();

  /// Update spatial body acceleration with the partial spatial body
  /// acceleration for inverse dynamics.
  virtual void updateAccelerationID();

  /// Update spatial body acceleration for forward dynamics.
  virtual void updateAccelerationFD();

  /// Update spatical body velocity change for impluse-based forward dynamics.
  virtual void updateVelocityChangeFD();

  /// Update spatial body force for inverse dynamics.
  ///
  /// The spatial body force is transmitted to this BodyNode from the parent
  /// body through the connecting joint. It is expressed in this BodyNode's
  /// frame.
  virtual void updateTransmittedForceID(const Eigen::Vector3d& _gravity,
                                        bool _withExternalForces = false);

  /// Update spatial body force for forward dynamics.
  ///
  /// The spatial body force is transmitted to this BodyNode from the parent
  /// body through the connecting joint. It is expressed in this BodyNode's
  /// frame.
  virtual void updateTransmittedForceFD();

  /// Update spatial body force for impulse-based forward dynamics.
  ///
  /// The spatial body impulse is transmitted to this BodyNode from the parent
  /// body through the connecting joint. It is expressed in this BodyNode's
  /// frame.
  virtual void updateTransmittedImpulse();
  // TODO: Rename to updateTransmittedImpulseFD if impulse-based inverse
  // dynamics is implemented.

  /// Update the joint force for inverse dynamics.
  virtual void updateJointForceID(double _timeStep,
                                  double _withDampingForces,
                                  double _withSpringForces);

  /// Update the joint force for forward dynamics.
  virtual void updateJointForceFD(double _timeStep,
                                  double _withDampingForces,
                                  double _withSpringForces);

  /// Update the joint impulse for forward dynamics.
  virtual void updateJointImpulseFD();

  /// Update constrained terms due to the constraint impulses for foward
  /// dynamics.
  virtual void updateConstrainedTerms(double _timeStep);

  //- DEPRECATED ---------------------------------------------------------------

  /// Update spatial body acceleration with the partial spatial body
  /// acceleration.
  /// \warning Please use updateAccelerationID().
  DEPRECATED(4.3)
  virtual void updateAcceleration();

  /// Update the joint acceleration and body acceleration
  /// \warning Please use updateAccelerationFD().
  DEPRECATED(4.3)
  virtual void updateJointAndBodyAcceleration();

  /// Update joint velocity change for impulse-based forward dynamics algorithm
  /// \warning Please use updateVelocityChangeFD().
  DEPRECATED(4.3)
  virtual void updateJointVelocityChange();

  /// Update the spatial body force transmitted to this BodyNode from the
  /// parent body through the connecting joint. The spatial body force is
  /// expressed in this BodyNode's frame.
  /// \warning Please use updateTransmittedForceFD().
  DEPRECATED(4.3)
  virtual void updateTransmittedWrench();

  /// Update spatial body force. Inverse dynamics routine.
  ///
  /// The spatial body force is transmitted to this BodyNode from the parent
  /// body through the connecting joint. It is expressed in this BodyNode's
  /// frame.
  ///
  /// \warning Please use updateTransmittedForceID().
  DEPRECATED(4.3)
  virtual void updateBodyWrench(const Eigen::Vector3d& _gravity,
                                bool _withExternalForces = false);

  /// updateBodyImpForceFwdDyn
  /// \warning Please use updateTransmittedImpulse().
  DEPRECATED(4.3)
  virtual void updateBodyImpForceFwdDyn();

  /// Update the joint force
  /// \warning Please use updateJointForceID().
  DEPRECATED(4.3)
  virtual void updateGeneralizedForce(bool _withDampingForces = false);

  /// updateConstrainedJointAndBodyAcceleration
  /// \warning Deprecated. Please do not use.
  DEPRECATED(4.3)
  virtual void updateConstrainedJointAndBodyAcceleration(double _timeStep);

  /// updateConstrainedTransmittedForce
  /// \warning Deprecated. Please do not use.
  DEPRECATED(4.3)
  virtual void updateConstrainedTransmittedForce(double _timeStep);

  /// \}

  //----------------------------------------------------------------------------
  /// \{ \name Equations of motion related routines
  //----------------------------------------------------------------------------

  ///
  virtual void updateMassMatrix();
  virtual void aggregateMassMatrix(Eigen::MatrixXd& _MCol, size_t _col);
  virtual void aggregateAugMassMatrix(Eigen::MatrixXd& _MCol, size_t _col,
                                      double _timeStep);

  ///
  virtual void updateInvMassMatrix();
  virtual void updateInvAugMassMatrix();
  virtual void aggregateInvMassMatrix(Eigen::MatrixXd& _InvMCol, size_t _col);
  virtual void aggregateInvAugMassMatrix(Eigen::MatrixXd& _InvMCol, size_t _col,
                                         double _timeStep);

  ///
  virtual void aggregateCoriolisForceVector(Eigen::VectorXd& _C);

  ///
  virtual void aggregateGravityForceVector(Eigen::VectorXd& _g,
                                           const Eigen::Vector3d& _gravity);

  ///
  virtual void updateCombinedVector();
  virtual void aggregateCombinedVector(Eigen::VectorXd& _Cg,
                                       const Eigen::Vector3d& _gravity);

  /// Aggregate the external forces mFext in the generalized coordinates
  /// recursively
  virtual void aggregateExternalForces(Eigen::VectorXd& _Fext);

  ///
  virtual void aggregateSpatialToGeneralized(Eigen::VectorXd& _generalized,
                                             const Eigen::Vector6d& _spatial);

  /// Update body Jacobian. getBodyJacobian() calls this function if
  /// mIsBodyJacobianDirty is true.
  void updateBodyJacobian() const;

  /// Update the World Jacobian. The commonality of using the World Jacobian
  /// makes it worth caching.
  void updateWorldJacobian() const;

  /// Update spatial time derivative of body Jacobian.
  /// getJacobianSpatialTimeDeriv() calls this function if
  /// mIsBodyJacobianSpatialDerivDirty is true.
  void updateBodyJacobianSpatialDeriv() const;

  /// Update classic time derivative of body Jacobian.
  /// getJacobianClassicTimeDeriv() calls this function if
  /// mIsWorldJacobianClassicDerivDirty is true.
  void updateWorldJacobianClassicDeriv() const;

  /// \}

protected:

  //--------------------------------------------------------------------------
  // General properties
  //--------------------------------------------------------------------------

  /// A unique ID of this node globally.
  int mID;

  /// Counts the number of nodes globally.
  static size_t msBodyNodeCount;

  /// BodyNode-specific properties
  UniqueProperties mBodyP;

  /// Whether the node is currently in collision with another node.
  bool mIsColliding;

  //--------------------------------------------------------------------------
  // Structural Properties
  //--------------------------------------------------------------------------

  /// Index of this BodyNode in its Skeleton
  size_t mIndexInSkeleton;

  /// Index of this BodyNode in its Tree
  size_t mIndexInTree;

  /// Index of this BodyNode's tree
  size_t mTreeIndex;

  /// Parent joint
  Joint* mParentJoint;

  /// Parent body node
  BodyNode* mParentBodyNode;

  /// Array of child body nodes
  std::vector<BodyNode*> mChildBodyNodes;

  /// Array of child Entities that are not BodyNodes. Organizing them separately
  /// allows some performance optimizations.
  std::set<Entity*> mNonBodyNodeEntities;

  /// List of markers associated
  std::vector<Marker*> mMarkers;

  /// A increasingly sorted list of dependent dof indices.
  std::vector<size_t> mDependentGenCoordIndices;

  /// A version of mDependentGenCoordIndices that holds DegreeOfFreedom pointers
  /// instead of indices
  std::vector<DegreeOfFreedom*> mDependentDofs;

  /// Same as mDependentDofs, but holds const pointers
  std::vector<const DegreeOfFreedom*> mConstDependentDofs;

  //--------------------------------------------------------------------------
  // Dynamical Properties
  //--------------------------------------------------------------------------

  /// Body Jacobian
  ///
  /// Do not use directly! Use getJacobian() to access this quantity
  mutable math::Jacobian mBodyJacobian;

  /// Dirty flag for body Jacobian.
  mutable bool mIsBodyJacobianDirty;

  /// Cached World Jacobian
  ///
  /// Do not use directly! Use getJacobian() to access this quantity
  mutable math::Jacobian mWorldJacobian;

  /// Dirty flag for world Jacobian
  mutable bool mIsWorldJacobianDirty;

  /// Spatial time derivative of body Jacobian.
  ///
  /// Do not use directly! Use getJacobianSpatialDeriv() to access this quantity
  mutable math::Jacobian mBodyJacobianSpatialDeriv;

  /// Dirty flag for spatial time derivative of body Jacobian.
  mutable bool mIsBodyJacobianSpatialDerivDirty;

  /// Classic time derivative of Body Jacobian
  ///
  /// Do not use directly! Use getJacobianClassicDeriv() to access this quantity
  mutable math::Jacobian mWorldJacobianClassicDeriv;

  /// Dirty flag for the classic time derivative of the Jacobian
  mutable bool mIsWorldJacobianClassicDerivDirty;

  /// Partial spatial body acceleration due to parent joint's velocity
  ///
  /// Do not use directly! Use getPartialAcceleration() to access this quantity
  mutable Eigen::Vector6d mPartialAcceleration;
  // TODO(JS): Rename with more informative name

  /// Is the partial acceleration vector dirty
  mutable bool mIsPartialAccelerationDirty;

  /// Transmitted wrench from parent to the bodynode expressed in body-fixed
  /// frame
  Eigen::Vector6d mF;

  /// External spatial force
  Eigen::Vector6d mFext;

  /// Spatial gravity force
  Eigen::Vector6d mFgravity;

  /// Articulated body inertia
  ///
  /// Do not use directly! Use getArticulatedInertia() to access this quantity
  mutable math::Inertia mArtInertia;

  /// Articulated body inertia for implicit joint damping and spring forces
  ///
  /// DO not use directly! Use getArticulatedInertiaImplicit() to access this
  mutable math::Inertia mArtInertiaImplicit;

  /// Bias force
  Eigen::Vector6d mBiasForce;

  /// Cache data for combined vector of the system.
  Eigen::Vector6d mCg_dV;
  Eigen::Vector6d mCg_F;

  /// Cache data for gravity force vector of the system.
  Eigen::Vector6d mG_F;

  /// Cache data for external force vector of the system.
  Eigen::Vector6d mFext_F;

  /// Cache data for mass matrix of the system.
  Eigen::Vector6d mM_dV;
  Eigen::Vector6d mM_F;

  /// Cache data for inverse mass matrix of the system.
  Eigen::Vector6d mInvM_c;
  Eigen::Vector6d mInvM_U;

  /// Cache data for arbitrary spatial value
  Eigen::Vector6d mArbitrarySpatial;

  //------------------------- Impulse-based Dyanmics ---------------------------
  /// Velocity change due to to external impulsive force exerted on
  ///        bodies of the parent skeleton.
  Eigen::Vector6d mDelV;

  /// Impulsive bias force due to external impulsive force exerted on
  ///        bodies of the parent skeleton.
  Eigen::Vector6d mBiasImpulse;

  /// Constraint impulse: contact impulse, dynamic joint impulse
  Eigen::Vector6d mConstraintImpulse;

  // TODO(JS): rename with more informative one
  /// Generalized impulsive body force w.r.t. body frame.
  Eigen::Vector6d mImpF;

  /// Collision shape added signal
  ColShapeAddedSignal mColShapeAddedSignal;

  /// Collision shape removed signal
  ColShapeRemovedSignal mColShapeRemovedSignal;

  /// Structural change signal
  StructuralChangeSignal mStructuralChangeSignal;

public:
  // To get byte-aligned Eigen vectors
  EIGEN_MAKE_ALIGNED_OPERATOR_NEW

  //----------------------------------------------------------------------------
  /// \{ \name Slot registers
  //----------------------------------------------------------------------------

  /// Slot register for collision shape added signal
  common::SlotRegister<ColShapeAddedSignal> onColShapeAdded;

  /// Slot register for collision shape removed signal
  common::SlotRegister<ColShapeRemovedSignal> onColShapeRemoved;

  /// Raised when (1) parent BodyNode is changed, (2) moved between Skeletons,
  /// (3) parent Joint is changed
  mutable common::SlotRegister<StructuralChangeSignal> onStructuralChange;

  /// \}
};

//==============================================================================
template <class JointType>
JointType* BodyNode::moveTo(BodyNode* _newParent,
    const typename JointType::Properties& _joint)
{
  if(nullptr == _newParent)
    return getSkeleton()->moveBodyNodeTree<JointType>(
          this, getSkeleton(), nullptr, _joint);
  else
    return getSkeleton()->moveBodyNodeTree<JointType>(
          this, _newParent->getSkeleton(), _newParent, _joint);
}

//==============================================================================
template <class JointType>
JointType* BodyNode::moveTo(
    const SkeletonPtr& _newSkeleton, BodyNode* _newParent,
    const typename JointType::Properties& _joint)
{
  return getSkeleton()->moveBodyNodeTree<JointType>(
        this, _newSkeleton, _newParent, _joint);
}

//==============================================================================
template <class JointType>
SkeletonPtr BodyNode::split(const std::string& _skeletonName,
      const typename JointType::Properties& _joint)
{
  SkeletonPtr skel = Skeleton::create(getSkeleton()->getSkeletonProperties());
  skel->setName(_skeletonName);
  moveTo<JointType>(skel, nullptr, _joint);
  return skel;
}

//==============================================================================
template <class JointType>
JointType* BodyNode::changeParentJointType(
    const typename JointType::Properties& _joint)
{
  return moveTo<JointType>(getParentBodyNode(), _joint);
}

//==============================================================================
template <class JointType>
std::pair<JointType*, BodyNode*> BodyNode::copyTo(
    BodyNode* _newParent,
    const typename JointType::Properties& _joint,
    bool _recursive)
{
  if(nullptr == _newParent)
    return getSkeleton()->cloneBodyNodeTree<JointType>(
          this, getSkeleton(), nullptr, _joint, _recursive);
  else
    return getSkeleton()->cloneBodyNodeTree<JointType>(
          this, _newParent->getSkeleton(), _newParent, _joint, _recursive);
}

//==============================================================================
template <class JointType>
std::pair<JointType*, BodyNode*> BodyNode::copyTo(
    const SkeletonPtr& _newSkeleton, BodyNode* _newParent,
    const typename JointType::Properties& _joint,
    bool _recursive) const
{
  return getSkeleton()->cloneBodyNodeTree<JointType>(
        this, _newSkeleton, _newParent, _joint, _recursive);
}

//==============================================================================
template <class JointType>
SkeletonPtr BodyNode::copyAs(const std::string& _skeletonName,
    const typename JointType::Properties& _joint, bool _recursive) const
{
  SkeletonPtr skel = Skeleton::create(getSkeleton()->getSkeletonProperties());
  skel->setName(_skeletonName);
  copyTo<JointType>(skel, nullptr, _joint, _recursive);
  return skel;
}

}  // namespace dynamics
}  // namespace dart

#endif  // DART_DYNAMICS_BODYNODE_H_
