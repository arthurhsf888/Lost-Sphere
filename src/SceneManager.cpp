#include "SceneManager.h"
#include <stdexcept>

void SceneManager::registerScene(const std::string& name, std::unique_ptr<Scene> scene) {
  scenes_[name] = std::move(scene);
}

void SceneManager::setActive(const std::string& name) {
  auto it = scenes_.find(name);
  if (it == scenes_.end()) throw std::runtime_error("Cena não registrada: " + name);
  current_ = it->second.get();
}

Scene* SceneManager::active() {
  return current_;
}
