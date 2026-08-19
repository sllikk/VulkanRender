#pragma once
#include  "utils.hpp"

struct ObjectConstant {

  glm::mat4 worldTransform{};

};

struct FrameContext {

  FrameContext(const FrameContext& other) = delete;
  FrameContext& operator=(const FrameContext& other) = delete;

  DeletionQueue deletion_queue;


};

