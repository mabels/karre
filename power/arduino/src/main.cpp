
#include "Timer.h"

#include "Wire.h"
#include "Adafruit_MCP9808.h"

#include "analogread.h"
#include "analogwrite.h"

#include "digitalwrite.h"

#include "delegate.h"

/* This port correponds to the "-W 0x20,-" command line option. */
#define special_output_port (*((volatile char *)0x20))
/* This port correponds to the "-R 0x22,-" command line option. */
#define special_input_port  (*((volatile char *)0x22))
/* Poll the specified string out the debug port. */
void debug_puts(const char *str) {
  for(const char *c = str; *c; ++c) {
    special_output_port = *c;
  }
}

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
      if (!ok) {
        ok = tempsensor.begin();
        if (!ok) {
          return -4711;
        }
      }
      tempsensor.awake();
      currentValue = tempsensor.readTempC();
      tempsensor.shutdown();
      return currentValue;
    }
};

class CarApp {
  private:
    const static int8_t NEXT=42;
    const static int8_t DONE=0;
    Timer t;

    AnalogRead<A0> voltage_220;
    AnalogRead<A1> voltage_car;
    AnalogRead<A2> voltage_vbat;
    AnalogRead<A3> voltage_a3;
    AnalogRead<A4> voltage_a4;
    AnalogRead<A5> voltage_a5;
    AnalogRead<A6> voltage_a6;
    // 4 unklar
    // 5 unklar
    // 6 unklar
    // 7 unklar
    AnalogRead<A7> voltage_myself;

    AnalogWrite<6> fan_speed;
    // 0 scheint nicht connected zu sein
    // 1 scheint nicht connected zu sein
    // 2 dickes relay unklar was
    // 3 dickes relay unklar was
    // 4 dickes relay unklar was
    // 5 finder!!!!
    // 6 luefter
    // nix
    DigitalWrite<2> relay_car_220_loader;
    DigitalWrite<3> relay_inverter_net;
    DigitalWrite<4> relay_car;
    DigitalWrite<5> relay_finder_220;

    DigitalWrite<0> relay_bistabil_on;
    DigitalWrite<1> relay_bistabil_off;



    int16_t times;

    int8_t  two_times;

    TempSensor tempsensor;
    Delegate::Ptr delegateCheckPower;
    Delegate::Ptr delegateCheckTemperatur;
    Delegate::Ptr delegateSendState;
    // Delegate::Ptr delegateToggleTest;


  public:
    CarApp() {
      delegateCheckPower = Delegate::instance<CarApp, &CarApp::checkPower>(this);
      delegateCheckTemperatur = Delegate::instance<CarApp, &CarApp::checkTemperatur>(this);
      delegateSendState = Delegate::instance<CarApp, &CarApp::sendState>(this);
      // delegateToggleTest = Delegate::instance<CarApp, &CarApp::toggleTest>(this);
    }

    // void toggleTest() {
    //   Serial.print(F("toggleTest:"));Serial.println(relay_car_220_loader.value());
    //   if (relay_car_220_loader.value()) {
    //     relay_car_220_loader.low();
    //   } else {
    //     relay_car_220_loader.high();
    //   }
    // }

    void setup() {
      Serial.begin(9600);
      voltage_car.setup();
      voltage_220.setup();
      voltage_vbat.setup();
      voltage_a4.setup();
      voltage_a5.setup();
      voltage_a6.setup();
      voltage_myself.setup();

      fan_speed.setup();

      //relay_inverter_net.setup().high();
      // relay_bistabil_on.setup().low();
      // delay(200);
      // relay_bistabil_on.setup().high();
      // delay(100);
      // relay_bistabil_off.setup().low();
      // delay(200);
      // relay_bistabil_off.setup().high();

      // relay_car_220_loader low + relay_inverter_net == car power
      relay_car_220_loader.setup().high();
      relay_inverter_net.setup().high();
      relay_car.setup().high();

      //.high();
      //relay_vbat.setup().high();
      //relay_car.setup().high();

      //Serial.println("jo-3");
      t.every(1000, delegateSendState, this);

      //t.every(5000, delegateToggleTest, this);
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
      t.after(800, delegateCheckPower, this);
      Serial.print(F("{"));
      Serial.print(F("instance:"));Serial.print((long)this);
      Serial.print(F(",times:"));Serial.print(++times);
      Serial.print(F(",millis:"));Serial.print(millis());
      Serial.print(F(",fanSpeed:"));Serial.print(fan_speed.value());
      Serial.print(F(",tempSensorOk:"));Serial.print(tempsensor.isOk());
      Serial.print(F(",temperatur:"));Serial.println(tempsensor.value());
      Serial.print(F(",voltage_220:"));Serial.print(voltage_220.value()*0.0370716);
      Serial.print(F(",voltage_car:"));Serial.print(voltage_car.value()*0.0370716);
      Serial.print(F(",voltage_vbat:"));Serial.print(voltage_vbat.value()*0.0370716);
      Serial.print(F(",voltage_a4:"));Serial.print(voltage_a4.value());
      Serial.print(F(",voltage_a5:"));Serial.print(voltage_a5.value());
      Serial.print(F(",voltage_a6:"));Serial.print(voltage_a6.value());
      Serial.print(F(",voltage_myself:"));Serial.println(voltage_myself.value());
      Serial.print(F(",relay_car_220_loader:"));Serial.print(relay_car_220_loader.value());
      //Serial.print(F(",relay_vbat:"));Serial.print(relay_inverter_net.value());
      Serial.print(F(",relay_car:"));Serial.print(relay_car.value());
      Serial.print(F(",relay_inverter:"));Serial.print(relay_inverter_net.value());
      Serial.print(F("}\r\n"));
    }

    void checkPower() {
      // if (two_times++ < 1) {
      //   t.after(400, delegateCheckPower, this);
      // }
      readVoltage220() && readVoltageCar() && readVoltageVBat() && readVoltageMySelf();
    }

    void powerOff() {
      // Relais Output VBAT off
      // Relais Output PASSTHRUE off
      // LadeStrecke auf 220V
    }

    void checkTemperatur() {
      tempsensor.read();
      if (!tempsensor.isOk()) {
        // fullspeed without sensor
        fan_speed.write(255);
        //tempsensor.begin();
        return;
      }
      if (tempsensor.value() < 38) {
        // pwm off
        fan_speed.write(0);
      } else if (tempsensor.value() < 35) {
        // pwm 40%
        fan_speed.write(255);
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

    int readVoltage220() {
      int16_t val = voltage_220.read();
      if (val <= 2000) return NEXT;
      // relay_car_220_loader.write(LOW);
      // relay_inverter_net.write(LOW);
      // relay_car.write(LOW);
      //relay_vbat.write(LOW);
      return DONE;
    }

    int readVoltageMySelf() {
      int16_t val = voltage_myself.read();
      if (val <= 2000) return NEXT;
      relay_car_220_loader.write(HIGH);
      relay_inverter_net.write(LOW);
      relay_car.write(LOW);
      //relay_vbat.write(LOW);
      return DONE;
    }


    int readVoltageCar() {
      int16_t val = voltage_car.read();
      if (val <= 2000) return NEXT;
      relay_car_220_loader.write(LOW);
      relay_inverter_net.write(HIGH);
      //relay_vbat.write(LOW);
      relay_car.write(HIGH);
      return DONE;
    }

    int readVoltageVBat() {
      int16_t val = voltage_vbat.read();
      if (val <= 2000) {
        // // poweroff ?;
        // relay_car_220_loader.write(LOW);
        // relay_inverter_net.write(LOW);
        // //relay_vbat.write(LOW);
        // relay_car.write(LOW);
        return NEXT;
      }
      relay_car_220_loader.write(LOW);
      relay_inverter_net.write(HIGH);
      //relay_vbat.write(HIGH);
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

// template <int8_t T>
// class AnalogReadObserve {
// private:
//   AnalogRead<T> port;
// public:
//   Delegate::Ptr upperAction;
//   Delegate::Ptr lowerAction;
//
//   const int16_t triggerValueUpper;
//   const int16_t upperRecheckTimeMsec;
//   int8_t upperRecheckTriggerCount;
//
//
//   const int16_t triggerValueLower;
//   const int16_t lowerRecheckTimeMsec;
//   int8_t lowerRecheckTriggerCount;
//
//   const int16_t checkTimeMsec;
//   int     lastCheckMillis;
//   int16_t checkcount;
//   boolean  enabled;
//
//   int start() {
//     lastCheckMillis = 0;
//     upperRecheckTriggerCount = 0;
//     lowerRecheckTriggerCount = 0;
//     checkcount = 0;
//     enabled = true;
//     return 7;
//   }
//
//   int stop() { enabled = false; return 9; }
//   void loop() { loop(millis()); }
//   void loop(int millis) {
//     if (!enabled) { return; }
//     if (checkcount < 0 && -1*lowerRecheckTriggerCount < checkcount) {
//         // lowerTriggerCheck
//     } if (checkcount < 0 && -1*lowerRecheckTriggerCount < checkcount) {
//         // upperTriggerCheck
//     } else if (lastCheckMillis == 0 || (millis-lastCheckMillis) >= checkTimeMsec) {
//         int16_t value = port.value();
//     }
//   }
// };

// Tests
//MockAnalogPort<19> portMock;
// class EventTarget {
// public:
//   int checkLower = 0;
//   void lower(void *) {
//     ++checkLower;
//   }
//   int checkUpper = 0;
//   void upper(void *) {
//     ++checkUpper;
//   }
// }
// EventTarget et;
// AnalogReadObserve<19,555,250,4,222,500,3,1000> aro;
// aro.lowerAction = Delegate::instance<EventTarget, &EventTarget::lower>(this);
// aro.upperAction = Delegate::instance<EventTarget, &EventTarget::upper>(this);
//
// // lowerTrigger
// int millis = 10000;
// portMock.setValue(200);
// for(; millis < 10010; ++millis) {
//   aro.loop(millis);
//   assert(et.checkLower == 0 && et.checkUpper == 0);
// }
// for(millis = 10250; millis < 10260; ++millis) {
//   aro.loop(millis);
//   assert(et.checkLower == 0 && et.checkUpper == 0);
// }
// for(millis = 10500; millis < 10510; ++millis) {
//   aro.loop(millis);
//   assert(et.checkLower == 0 && et.checkUpper == 0);
// }


//int cnt = 0;
//void jo() {
//  Serial.println((int)&app);
//}
//TempSensor ts;
void setup()
{

  //
  // Serial.print(A2);
  // pinMode(A2,INPUT);
  // while (true) {
  //   Serial.print(">>>");
  //   Serial.println(ts.read());
  //   delay(1000);
  // }
  /*
  Serial.begin(9600);
  for (int8_t i = 0; i < 6 ; ++i) {
    pinMode(2+i, OUTPUT);
    digitalWrite(2+i, HIGH);
  }
  for (int8_t i = 4; i < 6 ; ++i) {
    pinMode(2+i, OUTPUT);
    digitalWrite(2+i, LOW);
  }
  Serial.print(A0);
  pinMode(A0,INPUT);
    delay(1000);
  Serial.print(A1);
  pinMode(A1,INPUT);
    delay(1000);
  Serial.print(A2);
  pinMode(A2,INPUT);
    delay(1000);
  Serial.print(A3);
  pinMode(A3,INPUT);
    delay(1000);
  Serial.print(A4);
  pinMode(A4,INPUT);
    delay(1000);
  Serial.print(A5);
  pinMode(A5,INPUT);
    delay(1000);
  Serial.print(A6);
  pinMode(A6,INPUT);
    delay(1000);
  Serial.print(A7);
  pinMode(A7,INPUT);
    delay(1000);
  while (1) {
    Serial.print(">>>");
    Serial.println(ts.read());
    delay(1000);
  }

  /*
  digitalWrite(8, HIGH);
  delay(200);
  digitalWrite(8, LOW);
  */
  /*
  analogWrite(6, 250);
  delay(1000);
  digitalWrite(6, HIGH);
  delay(1000);
  digitalWrite(6, LOW);
  */
  /*
  while (1) {
    for (int8_t i = 3; i <= 3 ; ++i) {
      delay(2000);
      digitalWrite(2+i, LOW);
      delay(1000);
      digitalWrite(2+i, HIGH);
    }
  }
  */
  app.setup();
}
