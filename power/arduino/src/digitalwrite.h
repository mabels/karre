
template <int8_t port>
class DigitalWrite {
  private:
    int8_t state;
  public:
    DigitalWrite& setup() {
      pinMode(port, OUTPUT);
      return *this;
    }

    void high() {
      write(HIGH);
    }
    void low() {
      write(LOW);
    }

    int8_t write(int8_t _state) {
      Serial.print(F("State:"));Serial.print(port);Serial.println(_state);
      state = _state;
      digitalWrite(port, state);
      return state;
    }
    int8_t value() const {
      return state;
    }
};
