/*
 * Copyright (c) 2011-2019, The DART development contributors
 * All rights reserved.
 *
 * The list of contributors can be found at:
 *   https://github.com/dartsim/dart/blob/master/LICENSE
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

#include <dart/dart.hpp>
#include <pybind11/pybind11.h>
#include "eigen_geometry_pybind.h"
#include "eigen_pybind.h"

namespace dart {
namespace python {

void ShapeFrame(pybind11::module& m)
{
  ::pybind11::class_<
      dart::dynamics::ShapeFrame,
      // dart::common::EmbedPropertiesOnTopOf<
      //     dart::dynamics::ShapeFrame,
      //     dart::dynamics::detail::ShapeFrameProperties,
      //     dart::common::SpecializedForAspect<
      //         dart::dynamics::VisualAspect,
      //         dart::dynamics::CollisionAspect,
      //         dart::dynamics::DynamicsAspect> >,
      dart::dynamics::Frame,
      std::shared_ptr<dart::dynamics::ShapeFrame>>(m, "ShapeFrame")
      .def(
          "setProperties",
          +[](dart::dynamics::ShapeFrame* self,
              const dart::dynamics::ShapeFrame::UniqueProperties& properties) {
            self->setProperties(properties);
          },
          ::pybind11::arg("properties"))
      .def(
          "setAspectProperties",
          +[](dart::dynamics::ShapeFrame* self,
              const dart::common::EmbedPropertiesOnTopOf<
                  dart::dynamics::ShapeFrame,
                  dart::dynamics::detail::ShapeFrameProperties,
                  dart::common::SpecializedForAspect<
                      dart::dynamics::VisualAspect,
                      dart::dynamics::CollisionAspect,
                      dart::dynamics::DynamicsAspect>>::AspectProperties&
                  properties) { self->setAspectProperties(properties); },
          ::pybind11::arg("properties"))
      .def(
          "setShape",
          +[](dart::dynamics::ShapeFrame* self,
              const dart::dynamics::ShapePtr& shape) { self->setShape(shape); },
          ::pybind11::arg("shape"))
      .def(
          "getShape",
          +[](dart::dynamics::ShapeFrame* self) -> dart::dynamics::ShapePtr {
            return self->getShape();
          })
      .def(
          "getShape",
          +[](const dart::dynamics::ShapeFrame* self)
              -> dart::dynamics::ConstShapePtr { return self->getShape(); })
      .def(
          "hasVisualAspect",
          +[](const dart::dynamics::ShapeFrame* self) -> bool {
            return self->hasVisualAspect();
          })
      .def(
          "getVisualAspect",
          +[](dart::dynamics::ShapeFrame* self)
              -> dart::dynamics::VisualAspect* {
            return self->getVisualAspect();
          },
          pybind11::return_value_policy::reference_internal)
      .def(
          "getVisualAspect",
          +[](dart::dynamics::ShapeFrame* self,
              bool createIfNull) -> dart::dynamics::VisualAspect* {
            return self->getVisualAspect(createIfNull);
          },
          pybind11::return_value_policy::reference_internal,
          ::pybind11::arg("createIfNull"))
      .def(
          "setVisualAspect",
          +[](dart::dynamics::ShapeFrame* self,
              const dart::dynamics::VisualAspect* aspect) {
            self->setVisualAspect(aspect);
          },
          ::pybind11::arg("aspect"))
      .def(
          "createVisualAspect",
          +[](dart::dynamics::ShapeFrame* self)
              -> dart::dynamics::VisualAspect* {
            return self->createVisualAspect();
          },
          pybind11::return_value_policy::reference_internal)
      .def(
          "removeVisualAspect",
          +[](dart::dynamics::ShapeFrame* self) { self->removeVisualAspect(); })
      .def(
          "releaseVisualAspect",
          +[](dart::dynamics::ShapeFrame* self)
              -> std::unique_ptr<dart::dynamics::VisualAspect> {
            return self->releaseVisualAspect();
          })
      .def(
          "hasCollisionAspect",
          +[](const dart::dynamics::ShapeFrame* self) -> bool {
            return self->hasCollisionAspect();
          })
      .def(
          "getCollisionAspect",
          +[](dart::dynamics::ShapeFrame* self)
              -> dart::dynamics::CollisionAspect* {
            return self->getCollisionAspect();
          },
          pybind11::return_value_policy::reference_internal)
      .def(
          "getCollisionAspect",
          +[](dart::dynamics::ShapeFrame* self,
              bool createIfNull) -> dart::dynamics::CollisionAspect* {
            return self->getCollisionAspect(createIfNull);
          },
          pybind11::return_value_policy::reference_internal,
          ::pybind11::arg("createIfNull"))
      .def(
          "setCollisionAspect",
          +[](dart::dynamics::ShapeFrame* self,
              const dart::dynamics::CollisionAspect* aspect) {
            self->setCollisionAspect(aspect);
          },
          ::pybind11::arg("aspect"))
      .def(
          "createCollisionAspect",
          +[](dart::dynamics::ShapeFrame* self)
              -> dart::dynamics::CollisionAspect* {
            return self->createCollisionAspect();
          },
          pybind11::return_value_policy::reference_internal)
      .def(
          "setCollisionAspect",
          +[](dart::dynamics::ShapeFrame* self,
              const dart::dynamics::CollisionAspect* aspect) {
            self->setCollisionAspect(aspect);
          },
          ::pybind11::arg("aspect"))
      .def(
          "removeCollisionAspect",
          +[](dart::dynamics::ShapeFrame* self) {
            self->removeCollisionAspect();
          })
      .def(
          "releaseCollisionAspect",
          +[](dart::dynamics::ShapeFrame* self)
              -> std::unique_ptr<dart::dynamics::CollisionAspect> {
            return self->releaseCollisionAspect();
          })
      .def(
          "hasDynamicsAspect",
          +[](const dart::dynamics::ShapeFrame* self) -> bool {
            return self->hasDynamicsAspect();
          })
      .def(
          "getDynamicsAspect",
          +[](dart::dynamics::ShapeFrame* self)
              -> dart::dynamics::DynamicsAspect* {
            return self->getDynamicsAspect();
          },
          pybind11::return_value_policy::reference_internal)
      .def(
          "getDynamicsAspect",
          +[](dart::dynamics::ShapeFrame* self,
              bool createIfNull) -> dart::dynamics::DynamicsAspect* {
            return self->getDynamicsAspect(createIfNull);
          },
          pybind11::return_value_policy::reference_internal,
          ::pybind11::arg("createIfNull"))
      .def(
          "setDynamicsAspect",
          +[](dart::dynamics::ShapeFrame* self,
              const dart::dynamics::DynamicsAspect* aspect) {
            self->setDynamicsAspect(aspect);
          },
          ::pybind11::arg("aspect"))
      .def(
          "createDynamicsAspect",
          +[](dart::dynamics::ShapeFrame* self)
              -> dart::dynamics::DynamicsAspect* {
            return self->createDynamicsAspect();
          },
          pybind11::return_value_policy::reference_internal)
      .def(
          "setDynamicsAspect",
          +[](dart::dynamics::ShapeFrame* self,
              const dart::dynamics::DynamicsAspect* aspect) {
            self->setDynamicsAspect(aspect);
          },
          ::pybind11::arg("aspect"))
      .def(
          "removeDynamicsAspect",
          +[](dart::dynamics::ShapeFrame* self) {
            self->removeDynamicsAspect();
          })
      .def(
          "releaseDynamicsAspect",
          +[](dart::dynamics::ShapeFrame* self)
              -> std::unique_ptr<dart::dynamics::DynamicsAspect> {
            return self->releaseDynamicsAspect();
          })
      .def("isShapeNode", +[](const dart::dynamics::ShapeFrame* self) -> bool {
        return self->isShapeNode();
      });

  ::pybind11::class_<dart::dynamics::VisualAspect>(m, "VisualAspect")
      .def(::pybind11::init<>())
      .def(
          ::pybind11::init<
              const dart::common::detail::AspectWithVersionedProperties<
                  dart::common::CompositeTrackingAspect<
                      dart::dynamics::ShapeFrame>,
                  dart::dynamics::VisualAspect,
                  dart::dynamics::detail::VisualAspectProperties,
                  dart::dynamics::ShapeFrame,
                  &dart::common::detail::NoOp>::PropertiesData&>(),
          ::pybind11::arg("properties"))
      .def(
          "setRGBA",
          +[](dart::dynamics::VisualAspect* self,
              const Eigen::Vector4d& color) { self->setRGBA(color); },
          ::pybind11::arg("color"))
      .def(
          "setHidden",
          +[](dart::dynamics::VisualAspect* self, const bool& value) {
            self->setHidden(value);
          },
          ::pybind11::arg("value"))
      .def(
          "setShadowed",
          +[](dart::dynamics::VisualAspect* self, const bool& value) {
            self->setShadowed(value);
          },
          ::pybind11::arg("value"))
      .def(
          "setColor",
          +[](dart::dynamics::VisualAspect* self,
              const Eigen::Vector3d& color) { self->setColor(color); },
          ::pybind11::arg("color"))
      .def(
          "setColor",
          +[](dart::dynamics::VisualAspect* self,
              const Eigen::Vector4d& color) { self->setColor(color); },
          ::pybind11::arg("color"))
      .def(
          "setRGB",
          +[](dart::dynamics::VisualAspect* self, const Eigen::Vector3d& rgb) {
            self->setRGB(rgb);
          },
          ::pybind11::arg("rgb"))
      .def(
          "setAlpha",
          +[](dart::dynamics::VisualAspect* self, const double alpha) {
            self->setAlpha(alpha);
          },
          ::pybind11::arg("alpha"))
      .def(
          "getColor",
          +[](const dart::dynamics::VisualAspect* self) -> Eigen::Vector3d {
            return self->getColor();
          })
      .def(
          "getRGB",
          +[](const dart::dynamics::VisualAspect* self) -> Eigen::Vector3d {
            return self->getRGB();
          })
      .def(
          "getAlpha",
          +[](const dart::dynamics::VisualAspect* self) -> double {
            return self->getAlpha();
          })
      .def("hide", +[](dart::dynamics::VisualAspect* self) { self->hide(); })
      .def("show", +[](dart::dynamics::VisualAspect* self) { self->show(); })
      .def("isHidden", +[](const dart::dynamics::VisualAspect* self) -> bool {
        return self->isHidden();
      });

  ::pybind11::class_<dart::dynamics::CollisionAspect>(m, "CollisionAspect")
      .def(::pybind11::init<>())
      .def(
          ::pybind11::init<
              const dart::common::detail::AspectWithVersionedProperties<
                  dart::common::CompositeTrackingAspect<
                      dart::dynamics::ShapeFrame>,
                  dart::dynamics::CollisionAspect,
                  dart::dynamics::detail::CollisionAspectProperties,
                  dart::dynamics::ShapeFrame,
                  &dart::common::detail::NoOp>::PropertiesData&>(),
          ::pybind11::arg("properties"))
      .def(
          "setCollidable",
          +[](dart::dynamics::CollisionAspect* self, const bool& value) {
            self->setCollidable(value);
          },
          ::pybind11::arg("value"))
      .def(
          "isCollidable",
          +[](const dart::dynamics::CollisionAspect* self) -> bool {
            return self->isCollidable();
          });

  ::pybind11::class_<dart::dynamics::DynamicsAspect>(m, "DynamicsAspect")
      .def(::pybind11::init<>())
      .def(
          ::pybind11::init<
              const dart::common::detail::AspectWithVersionedProperties<
                  dart::common::CompositeTrackingAspect<
                      dart::dynamics::ShapeFrame>,
                  dart::dynamics::DynamicsAspect,
                  dart::dynamics::detail::DynamicsAspectProperties,
                  dart::dynamics::ShapeFrame,
                  &dart::common::detail::NoOp>::PropertiesData&>(),
          ::pybind11::arg("properties"))
      .def(
          "setFrictionCoeff",
          +[](dart::dynamics::DynamicsAspect* self, const double& value) {
            self->setFrictionCoeff(value);
          },
          ::pybind11::arg("value"))
      .def(
          "setRestitutionCoeff",
          +[](dart::dynamics::DynamicsAspect* self, const double& value) {
            self->setRestitutionCoeff(value);
          },
          ::pybind11::arg("value"));
}

} // namespace python
} // namespace dart
