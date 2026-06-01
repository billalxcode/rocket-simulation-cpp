#include "base.hpp"

class GenericEnvironment : public BaseEnvironment {
    public:
      GenericEnvironment(const char* name, const char* description = nullptr,
                         float gravity = 0.0f, float density = 0.0f)
          : BaseEnvironment(name, description, gravity, density) {};
      ~GenericEnvironment() override = default;
};