template <int8_t port>
class AnalogWrite {
  private:
    int16_t currentValue;
  public:
    void setup() {
      pinMode(port, OUTPUT);
      currentValue = 0;
    }
    int16_t write(int16_t v) {
      currentValue = v;
      analogWrite(port, currentValue);
      return currentValue;
    }
    int16_t value() const {
      return currentValue;
    }
};
