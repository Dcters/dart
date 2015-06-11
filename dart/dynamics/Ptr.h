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

#ifndef DART_DYNAMICS_PTR_H_
#define DART_DYNAMICS_PTR_H_

#include "dart/dynamics/detail/BodyNodePtr.h"
#include "dart/dynamics/detail/JointPtr.h"
#include "dart/dynamics/detail/DegreeOfFreedomPtr.h"

// This file is a lightweight means of providing the smart pointers which are
// commonly used within the dart::dynamics namespace. It is 'lightweight' in the
// sense that it does not depend on any types being fully defined, making this
// header suitable for inclusion in other headers which might only want access
// to the smart pointers without needing fully defined classes.

namespace dart {
namespace dynamics {

// -- Standard shared/weak pointers --
// Define a typedef for const and non-const version of shared_ptr and weak_ptr
// for the class X
#define DART_DYNAMICS_MAKE_SHARED_WEAK( X )                 \
  typedef std::shared_ptr< X >      X ## Ptr;               \
  typedef std::shared_ptr< const X > Const ## X ## Ptr;     \
  typedef std::weak_ptr< X >        Weak ## X ## Ptr;       \
  typedef std::weak_ptr< const X >   WeakConst ## X ## Ptr;

// Skeleton smart pointers
class Skeleton;
DART_DYNAMICS_MAKE_SHARED_WEAK(Skeleton)
// These pointers will take the form of:
// std::shared_ptr<Skeleton>        --> SkeletonPtr
// std::shared_ptr<const Skeleton>  --> ConstSkeletonPtr
// std::weak_ptr<Skeleton>          --> WeakSkeletonPtr
// std::weak_ptr<const Skeleton>    --> WeakConstSkeletonPtr

// MetaSkeleton smart pointers
class MetaSkeleton;
DART_DYNAMICS_MAKE_SHARED_WEAK(MetaSkeleton)

// ReferentialSkeleton smart pointers
class ReferentialSkeleton;
DART_DYNAMICS_MAKE_SHARED_WEAK(ReferentialSkeleton)

class Group;
DART_DYNAMICS_MAKE_SHARED_WEAK(Group)

class Linkage;
DART_DYNAMICS_MAKE_SHARED_WEAK(Linkage)

class Branch;
DART_DYNAMICS_MAKE_SHARED_WEAK(Branch)

class Chain;
DART_DYNAMICS_MAKE_SHARED_WEAK(Chain)

// Shape smart pointers
class Shape;
DART_DYNAMICS_MAKE_SHARED_WEAK(Shape)

// -- Custom BodyNode smart pointers --
#define DART_DYNAMICS_MAKE_BODYNODEPTR( X )                         \
  typedef TemplateBodyNodePtr< X >          X ## Ptr;               \
  typedef TemplateBodyNodePtr< const X >     Const ## X ## Ptr;     \
  typedef TemplateWeakBodyNodePtr< X >      Weak ## X ## Ptr;       \
  typedef TemplateWeakBodyNodePtr< const X > WeakConst ## X ## Ptr;

// BodyNode smart pointers
class BodyNode;
DART_DYNAMICS_MAKE_BODYNODEPTR(BodyNode)

// SoftBodyNode smart pointers
class SoftBodyNode;
DART_DYNAMICS_MAKE_BODYNODEPTR(SoftBodyNode)

// -- BodyNode dependent smart pointers --
#define DART_DYNAMICS_MAKE_BN_DEPENDENT_PTR( X )                                          \
  typedef Template ## X ## Ptr < X , BodyNode >                   X ## Ptr;               \
  typedef Template ## X ## Ptr < const X , const BodyNode >       Const ## X ## Ptr;      \
  typedef TemplateWeak ## X ## Ptr < X , BodyNode >               Weak ## X ## Ptr;       \
  typedef TemplateWeak ## X ## Ptr < const X , const BodyNode >   WeakConst ## X ## Ptr;


// Joint smart pointers
class Joint;
DART_DYNAMICS_MAKE_BN_DEPENDENT_PTR(Joint)

// DegreeOfFreedom smart pointers
class DegreeOfFreedom;
DART_DYNAMICS_MAKE_BN_DEPENDENT_PTR(DegreeOfFreedom)

} // namespace dynamics
} // namespace dart

#endif // DART_DYNAMICS_PTR_H_
