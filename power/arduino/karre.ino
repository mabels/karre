
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
      digitalWrite(port, HIGH); //pull up on
      state = 0;
    }
    int16_t read() {
      state = analogRead(port);
      return state;
    }
    int16_t currentValue() {
      return state;
    }
};

template <int8_t port>
class DigitalWrite {
  private:
    int8_t state;
  public:
    void setup() {
      pinMode(port, OUTPUT);
      state = LOW;
    }

    int8_t write(int8_t _state) {
      state = _state;
      digitalWrite(port, state);
      return state;
    }
    int8_t currentValue() {
      return state;
    }
};


class Delegate
{
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

class CarApp {
  private:
    const int8_t NEXT=42;
    const int8_t DONE=0;
    Timer t;

    AnalogRead<A1> voltage_car;
    AnalogRead<A2> voltage_220;
    AnalogRead<A3> voltage_vbat;

    DigitalWrite<1> relay_car_220_loader;
    DigitalWrite<2> relay_inverter_net;
    DigitalWrite<3> relay_vbat;
    DigitalWrite<4> relay_car;

    int16_t times;

    Adafruit_MCP9808 tempsensor;
    float temperatur;

    int8_t  two_times;

    int16_t readMCP9808() {
//      tempsensor.shutdown_wake(0);
//      temperatur = tempsensor.readTempC();
//      tempsensor.shutdown_wake(1);
    }

    Delegate::Ptr delegateCheckPower;
    Delegate::Ptr delegateCheckTemperatur;
    Delegate::Ptr delegateSendState;

  public:
    CarApp() {
      delegateCheckPower = Delegate::instance<CarApp, &CarApp::checkPower>(this);
      delegateCheckTemperatur = Delegate::instance<CarApp, &CarApp::checkTemperatur>(this);
      delegateSendState = Delegate::instance<CarApp, &CarApp::sendState>(this);
    }

    int setup() {
      voltage_car.setup();
      voltage_220.setup();
      voltage_vbat.setup();

      relay_inverter_net.setup();
      relay_car_220_loader.setup();
      relay_vbat.setup();
      relay_car.setup();

      tempsensor.begin();
      //Serial.println("jo-3");
      t.every(1000, delegateSendState, this);
      //delay(1000);
      //Serial.println("jo-4");
      sendState();
    }
    int loop() {
      t.update();
    }
    void sendState() {
      two_times = 0;

      t.after(900, delegateCheckTemperatur, this);
      t.after(200, delegateCheckPower, this);
      Serial.print(F("{"));
      Serial.print(F("instance:"));Serial.print((long)this);
      Serial.print(F(",times:"));Serial.print(++times);
      Serial.print(F(",millis:"));Serial.print(millis());
      Serial.print(F(",temperatur:"));Serial.print(temperatur);
      Serial.print(F(",voltage_220:"));Serial.print(voltage_220.currentValue());
      Serial.print(F(",voltage_car:"));Serial.print(voltage_car.currentValue());
      Serial.print(F(",voltage_vbat:"));Serial.print(voltage_vbat.currentValue());
      Serial.print(F(",relay_car_220_loader:"));Serial.print(relay_car_220_loader.currentValue());
      Serial.print(F(",relay_vbat:"));Serial.print(relay_inverter_net.currentValue());
      Serial.print(F(",relay_car:"));Serial.print(relay_car.currentValue());
      Serial.print(F(",relay_inverter:"));Serial.print(relay_inverter_net.currentValue());
      Serial.print(F("}\n"));
    }

    void checkPower() {
      if (two_times++ < 1) {
        t.after(400, delegateCheckPower, this);
      }
      runVoltage220() && readVoltageCar() && readVoltageVBat();
    }

    void checkTemperatur() {
      temperatur = readMCP9808();
      if (temperatur < 1000) {
        // pwm off
      } else if (temperatur < 2000) {
        // pwm 40%
      } else if (temperatur < 3000) {
        // pwm 80%
      } else if (temperatur < 3000) {
        // pwm 100%
      } else if (temperatur > 3200) {
        // poweroff
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

//Timer t;
void loop()
{
  app.loop();
//  t.update();
}

//int cnt = 0;
//void jo() {
//  Serial.println((int)&app);
//}
void setup()
{
  Serial.begin(9600);

//  t.every(1000, jo);

//  delay(2000);
//  Serial.println("jo-1");
  app.setup();
}
