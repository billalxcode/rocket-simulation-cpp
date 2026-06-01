#include "base.hpp"

class GenericEnvironment : public BaseEnvironment {
    public:
      GenericEnvironment(const char* name, const char* description = nullptr,
                         double gravity = 0.0)
          : BaseEnvironment(name, description, gravity) {};
      ~GenericEnvironment() override = default;
};