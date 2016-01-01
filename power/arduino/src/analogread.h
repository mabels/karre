template <int8_t port>
class AnalogRead {
  private:
    int16_t state;
  public:
    void setup() {
      pinMode(port, INPUT);
      //digitalWrite(port, HIGH); //pull up on
      state = 0;
    }
    int16_t read() {
      state = analogRead(port);
      return state;
    }
    int16_t value() const {
      return state;
    }
};
