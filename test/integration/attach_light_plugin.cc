/*
 * Copyright (C) 2012 Open Source Robotics Foundation
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
*/

#include "gazebo/test/ServerFixture.hh"
#include "gazebo/physics/physics.hh"
#include "gazebo/sensors/sensors.hh"
#include "gazebo/common/common.hh"
#include "gazebo/test/helper_physics_generator.hh"

using namespace gazebo;
class AttachLightTest : public ServerFixture,
                        public testing::WithParamInterface<const char*>
{
  /// \brief Test AttachLightPlugin by verifying light pose against link pose
  /// \param[in] _physicsEngine Name of physics engine
  public: void AttachLightPlugin(const std::string &_physicsEngine);
};


void AttachLightTest::AttachLightPlugin(const std::string &_physicsEngine)
{
  // Test plugin for attaching lights to links
  this->Load("worlds/attach_lights.world", true, _physicsEngine);

  physics::WorldPtr world = physics::get_world("default");
  ASSERT_TRUE(world != nullptr);

  // Get the double pendulum model
  physics::ModelPtr pendulumModel =
      world->GetModel("double_pendulum_with_lights");
  ASSERT_TRUE(pendulumModel != nullptr);

  // Get the links
  physics::LinkPtr upperLink =
      pendulumModel->GetLink("double_pendulum_with_base::upper_link");
  ASSERT_TRUE(upperLink != nullptr);
  physics::LinkPtr lowerLink =
      pendulumModel->GetLink("double_pendulum_with_base::lower_link");
  ASSERT_TRUE(lowerLink != nullptr);

  // Get the lights
  physics::LightPtr pointLight = world->Light("point");
  ASSERT_TRUE(pointLight != nullptr);
  physics::LightPtr pointLight2 = world->Light("point2");
  ASSERT_TRUE(pointLight2 != nullptr);
  physics::LightPtr spotLight = world->Light("spot");
  ASSERT_TRUE(spotLight != nullptr);

  // step the world
  world->Step(1);

  // Get the initial light pose offset relative to link
  ignition::math::Pose3d pointLightPose = pointLight->GetWorldPose().Ign() -
      upperLink->GetWorldPose().Ign();
  ignition::math::Pose3d pointLight2Pose = pointLight2->GetWorldPose().Ign() -
      upperLink->GetWorldPose().Ign();
  ignition::math::Pose3d spotLightPose = spotLight->GetWorldPose().Ign() -
      lowerLink->GetWorldPose().Ign();

  auto prevUpperLinkPose = upperLink->GetWorldPose().Ign();
  auto prevLowerLinkPose = lowerLink->GetWorldPose().Ign();

  // verify pose for 1 second
  for (unsigned int i = 0; i < 100; ++i)
  {
    ignition::math::Pose3d upperLinkPose = upperLink->GetWorldPose().Ign();
    ignition::math::Pose3d lowerLinkPose = lowerLink->GetWorldPose().Ign();

    // NOTE: there's potential race condition when verifying pose
    // using GetWorldPose
    EXPECT_EQ(pointLight->GetWorldPose(), pointLightPose + upperLinkPose);
    EXPECT_EQ(pointLight2->GetWorldPose(), pointLight2Pose + upperLinkPose);
    EXPECT_EQ(spotLight->GetWorldPose(), spotLightPose + lowerLinkPose);
    world->Step(10);
  }
}

TEST_P(AttachLightTest, AttachLightPlugin)
{
  AttachLightPlugin(GetParam());
}

INSTANTIATE_TEST_CASE_P(PhysicsEngines, AttachLightTest, PHYSICS_ENGINE_VALUES);

int main(int argc, char **argv)
{
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
