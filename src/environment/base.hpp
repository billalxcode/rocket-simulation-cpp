class BaseEnvironment {
    private:
      const char* name;
      const char* description;
      const double gravity = 0.0;

    public:
      BaseEnvironment(const char* name, const char* description = nullptr,
                      double gravity = 0.0)
          : name(name), description(description), gravity(gravity) {}
      virtual ~BaseEnvironment() = default;
      virtual const char* getName() const { return name; };
      virtual const char* getDescription() const {
            return description ? description : "No description available.";
      }
      virtual double getGravity() const { return gravity; }
};