#pragma once
#include <memory>
#include <unordered_map>
#include <string>
#include "Scene.h"

class SceneManager {
public:
  void registerScene(const std::string& name, std::unique_ptr<Scene> scene);
  void setActive(const std::string& name);
  Scene* active();

private:
  std::unordered_map<std::string, std::unique_ptr<Scene>> scenes_;
  Scene* current_ = nullptr;
};
