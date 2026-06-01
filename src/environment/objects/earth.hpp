#include "../generic.hpp"

class EarthEnvironment : public GenericEnvironment {
    public:
      EarthEnvironment()
          : GenericEnvironment("Earth", "The third planet from the Sun",
                               9.807) {}
};