
#include "Timer.h"

#include "Wire.h"
#include "Adafruit_MCP9808.h"

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
      state = _state;
      digitalWrite(port, state);
      return state;
    }
    int8_t value() const {
      return state;
    }
};


class Delegate {
public:
    typedef void (*Ptr)(void *);

    template <class T, void (T::*TMethod)(void)>
    static Ptr instance(T* object_ptr) {
        return static_cast<Ptr>(&method_stub<T, TMethod>);
    }
private:
    template <class T, void (T::*TMethod)(void)>
    static void method_stub(void* object_ptr) {
        T* p = static_cast<T*>(object_ptr);
        return (p->*TMethod)();
    }
};

class TempSensor {
  private:
    Adafruit_MCP9808 tempsensor;
    boolean ok;
    float currentValue;
  public:
    boolean isOk() const {
      return ok;
    }
    float value() const {
      return currentValue;
    }
    TempSensor() : ok(false), currentValue(0) { }
    float read() {
      tempsensor.awake();
      currentValue = tempsensor.readTempC();
      tempsensor.shutdown();
      return currentValue;
    }
    void begin() {
      ok = tempsensor.begin();
    }
};

class CarApp {
  private:
    const static int8_t NEXT=42;
    const static int8_t DONE=0;
    Timer t;

    AnalogRead<A1> voltage_car;
    AnalogRead<A2> voltage_220;
    AnalogRead<A3> voltage_vbat;

    AnalogWrite<A4> fan_speed;

    DigitalWrite<4> relay_car_220_loader;
    DigitalWrite<5> relay_inverter_net;
    DigitalWrite<6> relay_vbat;
    DigitalWrite<7> relay_car;

    int16_t times;

    int8_t  two_times;

    TempSensor tempsensor;
    Delegate::Ptr delegateCheckPower;
    Delegate::Ptr delegateCheckTemperatur;
    Delegate::Ptr delegateSendState;

  public:
    CarApp() {
      delegateCheckPower = Delegate::instance<CarApp, &CarApp::checkPower>(this);
      delegateCheckTemperatur = Delegate::instance<CarApp, &CarApp::checkTemperatur>(this);
      delegateSendState = Delegate::instance<CarApp, &CarApp::sendState>(this);
    }

    void setup() {
      voltage_car.setup();
      voltage_220.setup();
      voltage_vbat.setup();

      fan_speed.setup();

      relay_inverter_net.setup().high();
      relay_car_220_loader.setup().high();
      relay_vbat.setup().high();
      relay_car.setup().high();

      tempsensor.begin();
      //Serial.println("jo-3");
      t.every(1000, delegateSendState, this);
      //delay(1000);
      //Serial.println("jo-4");
      sendState();
    }
    void loop() {
      t.update();
    }
    void sendState() {
      two_times = 0;

      t.after(900, delegateCheckTemperatur, this);
      //t.after(200, delegateCheckPower, this);
      Serial.print(F("{"));
      Serial.print(F("instance:"));Serial.print((long)this);
      Serial.print(F(",times:"));Serial.print(++times);
      Serial.print(F(",millis:"));Serial.print(millis());
      Serial.print(F(",fanSpeed:"));Serial.print(fan_speed.value());
      Serial.print(F(",tempSensorOk:"));Serial.print(tempsensor.isOk());
      Serial.print(F(",temperatur:"));Serial.print(tempsensor.value());
      Serial.print(F(",voltage_220:"));Serial.print(voltage_220.value());
      Serial.print(F(",voltage_car:"));Serial.print(voltage_car.value());
      Serial.print(F(",voltage_vbat:"));Serial.print(voltage_vbat.value());
      Serial.print(F(",relay_car_220_loader:"));Serial.print(relay_car_220_loader.value());
      Serial.print(F(",relay_vbat:"));Serial.print(relay_inverter_net.value());
      Serial.print(F(",relay_car:"));Serial.print(relay_car.value());
      Serial.print(F(",relay_inverter:"));Serial.print(relay_inverter_net.value());
      Serial.print(F("}\r\n"));
    }

    void checkPower() {
      if (two_times++ < 1) {
        t.after(400, delegateCheckPower, this);
      }
      runVoltage220() && readVoltageCar() && readVoltageVBat();
    }

    void powerOff() {
      // Relais Output VBAT off
      // Relais Output PASSTHRUE off
      // LadeStrecke auf 220V
    }

    void checkTemperatur() {
      if (!tempsensor.isOk()) {
        // fullspeed without sensor
        fan_speed.write(255);
        tempsensor.begin();
        return;
      }
      tempsensor.read();
      if (tempsensor.value() < 30) {
        // pwm off
        fan_speed.write(0);
      } else if (tempsensor.value() < 35) {
        // pwm 40%
        fan_speed.write(80);
      } else if (tempsensor.value() < 40) {
        // pwm 80%
        fan_speed.write(160);
      } else if (tempsensor.value() < 50) {
        // pwm 100%
        fan_speed.write(255);
      } else if (tempsensor.value() >= 50) {
        // poweroff
        powerOff();
      }
    }

    int runVoltage220() {
      int16_t val = voltage_220.read();
      if (val <= 2000) return NEXT;
      relay_car_220_loader.write(HIGH);
      relay_inverter_net.write(LOW);
      relay_car.write(LOW);
      relay_vbat.write(LOW);
      return DONE;
    }

    int readVoltageCar() {
      int16_t val = voltage_car.read();
      if (val <= 2000) return NEXT;
      relay_car_220_loader.write(LOW);
      relay_inverter_net.write(HIGH);
      relay_vbat.write(LOW);
      relay_car.write(HIGH);
      return DONE;
    }

    int readVoltageVBat() {
      int16_t val = voltage_vbat.read();
      if (val <= 2000) {
        // poweroff ?;
        relay_car_220_loader.write(LOW);
        relay_inverter_net.write(LOW);
        relay_vbat.write(LOW);
        relay_car.write(LOW);
        return NEXT;
      }
      relay_car_220_loader.write(LOW);
      relay_inverter_net.write(HIGH);
      relay_vbat.write(HIGH);
      relay_car.write(LOW);
      return DONE;
    }
};

CarApp app;

//Adafruit_MCP9808 tempsensor = Adafruit_MCP9808();
//Timer t;
void loop()
{
  app.loop();

}

//int cnt = 0;
//void jo() {
//  Serial.println((int)&app);
//}
void setup()
{
  Serial.begin(9600);
  app.setup();
}
