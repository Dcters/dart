/*
 * Copyright (c) 2015-2016, Georgia Tech Research Corporation
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

#ifndef DART_COMMON_DETAIL_COMPOSITE_HPP_
#define DART_COMMON_DETAIL_COMPOSITE_HPP_

#include "dart/common/Composite.hpp"

#define DART_COMMON_CHECK_ILLEGAL_ASPECT_ERASE( Func, T, ReturnType )\
  if(requiresAspect< T >())\
  {\
    dterr << "[Composite::" #Func << "] Illegal request to remove required "\
          << "Aspect [" << typeid(T).name() << "]!\n";\
    assert(false);\
    return ReturnType ;\
  }

namespace dart {
namespace common {

//==============================================================================
template <class T>
bool Composite::has() const
{
  return (get<T>() != nullptr);
}

//==============================================================================
template <class T>
T* Composite::get()
{
  AspectMap::iterator it = mAspectMap.find( typeid(T) );
  if(mAspectMap.end() == it)
    return nullptr;

  return static_cast<T*>(it->second.get());
}

//==============================================================================
template <class T>
const T* Composite::get() const
{
  return const_cast<Composite*>(this)->get<T>();
}

//==============================================================================
template <class T>
void Composite::set(const T* aspect)
{
  _set(typeid(T), aspect);
}

//==============================================================================
template <class T>
void Composite::set(std::unique_ptr<T>&& aspect)
{
  _set(typeid(T), std::move(aspect));
}

//==============================================================================
template <class T, typename ...Args>
T* Composite::createAspect(Args&&... args)
{
  T* aspect = new T(this, std::forward<Args>(args)...);
  mAspectMap[typeid(T)] = std::unique_ptr<T>(aspect);
  addToComposite(aspect);

  return aspect;
}

//==============================================================================
template <class T>
void Composite::removeAspect()
{
  AspectMap::iterator it = mAspectMap.find( typeid(T) );
  DART_COMMON_CHECK_ILLEGAL_ASPECT_ERASE(removeAspect, T, DART_BLANK)
  if(mAspectMap.end() != it)
  {
    removeFromComposite(it->second.get());
    it->second = nullptr;
  }
}

//==============================================================================
template <class T>
std::unique_ptr<T> Composite::releaseAspect()
{
  std::unique_ptr<T> extraction = nullptr;
  AspectMap::iterator it = mAspectMap.find( typeid(T) );
  DART_COMMON_CHECK_ILLEGAL_ASPECT_ERASE(releaseAspect, T, nullptr)
  if(mAspectMap.end() != it)
  {
    removeFromComposite(it->second.get());
    extraction = std::unique_ptr<T>(static_cast<T*>(it->second.release()));
  }

  return extraction;
}

//==============================================================================
template <class T>
constexpr bool Composite::isSpecializedFor()
{
  return false;
}

//==============================================================================
template <class T>
bool Composite::requiresAspect() const
{
  return (mRequiredAspects.find(typeid(T)) != mRequiredAspects.end());
}

//==============================================================================
template <class T>
void createAspects(T* /*comp*/)
{
  // Do nothing
}

//==============================================================================
template <class T, class NextAspect, class... Aspects>
void createAspects(T* comp)
{
  comp->template createAspect<NextAspect>();

  createAspects<T, Aspects...>(comp);
}

} // namespace common
} // namespace dart

//==============================================================================
// Create non-template alternatives to Composite functions
#define DART_BAKE_SPECIALIZED_ASPECT_IRREGULAR( TypeName, AspectName )\
  inline bool has ## AspectName () const\
  {\
    return this->template has<TypeName>();\
  }\
\
  inline TypeName * get ## AspectName ()\
  {\
    return this->template get<TypeName>();\
  }\
\
  inline const TypeName* get ## AspectName () const\
  {\
    return this->template get<TypeName>();\
  }\
\
  inline TypeName * get ## AspectName (const bool createIfNull)\
  {\
    TypeName* aspect = get ## AspectName();\
\
    if (createIfNull && nullptr == aspect)\
      return create ## AspectName();\
\
    return aspect;\
  }\
\
  inline void set ## AspectName (const TypeName * aspect)\
  {\
    this->template set<TypeName>(aspect);\
  }\
\
  inline void set ## AspectName (std::unique_ptr< TypeName >&& aspect)\
  {\
    this->template set<TypeName>(std::move(aspect));\
  }\
\
  template <typename ...Args>\
  inline TypeName * create ## AspectName (Args&&... args)\
  {\
    return this->template createAspect<TypeName>(std::forward<Args>(args)...);\
  }\
\
  inline void erase ## AspectName ()\
  {\
    this->template removeAspect<TypeName>();\
  }\
\
  inline std::unique_ptr< TypeName > release ## AspectName ()\
  {\
    return this->template releaseAspect<TypeName>();\
  }

//==============================================================================
#define DART_BAKE_SPECIALIZED_ASPECT(AspectName)\
  DART_BAKE_SPECIALIZED_ASPECT_IRREGULAR(AspectName, AspectName);

#endif // DART_COMMON_DETAIL_COMPOSITE_HPP_
