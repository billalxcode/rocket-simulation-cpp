class BaseEnvironment {
    private:
      const char* name;
      const char* description;
      const float gravity = 0.0f;
      const float density = 0.0f;

    public:
      BaseEnvironment(const char* name, const char* description = nullptr,
                      float gravity = 0.0f, float density = 0.0f)
          : name(name), description(description), gravity(gravity),
            density(density) {}
      virtual ~BaseEnvironment() = default;
      virtual const char* getName() const { return name; };
      virtual const char* getDescription() const {
            return description ? description : "No description available.";
      }
      virtual float getGravity() const { return gravity; }
      virtual float getDensity() const { return density; }
};