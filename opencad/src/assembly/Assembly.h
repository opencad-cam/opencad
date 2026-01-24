#pragma once
#include <vector>
#include <memory>
#include "Component.h"
#include "AssemblyConstraint.h"

namespace opencad {
namespace assembly {

class Assembly {
public:
    Assembly() = default;
    ~Assembly() = default;

    void addComponent(std::shared_ptr<Component> component);
    void removeComponent(std::shared_ptr<Component> component);
    const std::vector<std::shared_ptr<Component>>& getComponents() const;

    void addConstraint(std::shared_ptr<AssemblyConstraint> constraint);
    const std::vector<std::shared_ptr<AssemblyConstraint>>& getConstraints() const;

    bool solve();

private:
    std::vector<std::shared_ptr<Component>> m_components;
    std::vector<std::shared_ptr<AssemblyConstraint>> m_constraints;
};

} // namespace assembly
} // namespace opencad
