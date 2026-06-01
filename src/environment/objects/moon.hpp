#include "../generic.hpp"

class MoonEnvironment : public GenericEnvironment {
    public:
      MoonEnvironment()
          : GenericEnvironment("Moon", "Earth's only natural satellite", 1.62f,
                               3340.0f) {}
};