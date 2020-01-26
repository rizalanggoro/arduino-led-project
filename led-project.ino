
/*
 * 0 static       ==> 255
 * 1 breathing    ==> 0-255-0
 * 2 color cycle  ==> rgb
 * 3 strobing     ==> 0-255 --> 0
 */

int blue = 12;
int red = 11;
int green = 10;

int leds[] = {red, green, blue};

int mode = 0;

long previousMillis = 0;

// -->
int staticRedLevel = 0;
int staticGreenLevel = 0;
int staticBlueLevel = 0;

// -->
int breathingLedPin = leds[0];
int breathingInterval = 5;

// -->
int colorCycleInterval = 5;

// -->
int strobingInterval = 5;
int strobingLedPin = leds[0];

void setup() {
  Serial.begin(9600);
  pinMode(leds[0], OUTPUT);
  pinMode(leds[1], OUTPUT);
  pinMode(leds[2], OUTPUT);
}

void loop() {
  parseBluetooth();
}

void parseBluetooth() {
  if (Serial.available() > 0) {
    String data = Serial.readString();
    if (contains(data, "breathing")) {
      // breathing;pin:0,interval:5
      String pin = data.substring(data.indexOf(";") + 1, data.indexOf(","));
      pin = pin.substring(pin.indexOf(":") + 1, pin.length());

      String interval = data.substring(data.indexOf(",") + 1, data.length());
      interval = interval.substring(interval.indexOf(":") + 1, interval.length());

      Serial.println(interval);
      mode = 1;
      breathingLedPin = leds[pin.toInt()];
      breathingInterval = interval.toInt();
      resetLed();
      callback(true);
    } else if (contains(data, "static")) {
      // static;red:38,green:50,,blue:56
      String red = data.substring(data.indexOf(";") + 1, data.indexOf(","));
      red = red.substring(red.indexOf(":") + 1, red.length());

      String green = data.substring(data.indexOf(",") + 1, data.indexOf(",,"));
      green = green.substring(green.indexOf(":") + 1, green.length());

      String blue = data.substring(data.indexOf(",,") + 1, data.length());
      blue= blue.substring(blue.indexOf(":") + 1, blue.length());

      mode = 0;
      resetLed();
      staticRedLevel = red.toInt();
      staticGreenLevel = green.toInt();
      staticBlueLevel = blue.toInt();
      callback(true);
    } else if (contains(data, "colorCycle")) {
      // colorCycle;interval:5
      String interval = data.substring(data.indexOf(";") + 1, data.length());
      interval = interval.substring(interval.indexOf(":") + 1, interval.length());

      mode = 2;
      resetLed();
      colorCycleInterval = interval.toInt();
      callback(true);
    } else if (contains(data, "strobing")) {
      // strobing;pin:0,interval:5
      String pin = data.substring(data.indexOf(";") + 1, data.indexOf(","));
      pin = pin.substring(pin.indexOf(":") + 1, pin.length());

      String interval = data.substring(data.indexOf(",") + 1, data.length());
      interval = interval.substring(interval.indexOf(":") + 1, interval.length());

      mode = 3;
      strobingLedPin = leds[pin.toInt()];
      strobingInterval = interval.toInt();
      resetLed();
      callback(true);
    } else {
      callback(false);
    }
  }
  parseMode();
}

void callback(bool success) {
  Serial.println(success ? "1" : "0");
}

bool contains(String data, String a) {
  return data.indexOf(a) >= 0;
}

void resetLed() {
  // after received data
  analogWrite(leds[0], 0);
  analogWrite(leds[1], 0);
  analogWrite(leds[2], 0);
  //breathingInterval = 5;
}

void parseMode() {
  switch(mode) {
    case 0 :
    // static
    staticLed(); //-->
    break;

    case 1 :
    // breathing
    breathingLed(); //-->
    break;

    case 2 :
    // color cycle
    colorCycleLed(); //-->
    break;

    case 3 :
    // strobing
    strobingLed();
    break;

    case 4 :
    rgb(255, 255, 255);
    break;
  }
}

void staticLed() {
  analogWrite(leds[0], staticRedLevel);
  analogWrite(leds[1], staticGreenLevel);
  analogWrite(leds[2], staticBlueLevel);
}

bool breathingIsUp = true;
int breathingLevelUp = 0;
int breathingLevelDown = 0;
void breathingLed() {
  long currentMillis = millis();
  if (currentMillis - previousMillis >= breathingInterval) {
    previousMillis = currentMillis;
    if (breathingIsUp) {
      if (breathingLevelUp < 255) {
        breathingLevelUp++;
        analogWrite(breathingLedPin, breathingLevelUp);
      } else if (breathingLevelUp == 255) {
        breathingIsUp = !breathingIsUp;
        breathingLevelUp = 0;
      }
    } else {
      if (breathingLevelDown < 255) {
        breathingLevelDown++;
        analogWrite(breathingLedPin, 255 - breathingLevelDown);
      } else if (breathingLevelDown == 255) {
        breathingIsUp = !breathingIsUp;
        breathingLevelDown = 0;
      }
    }
  }
}

int colorCycleCurrentLedPin = -1;
bool colorCycleIsUp = true;
int colorCycleLevelUp = 0;
int colorCycleLevelDown = 0;

void colorCycleLed() {
  cc(leds[1], true);
  cc(leds[0], true);
  cc(leds[1], false);
  cc(leds[2], true);
  cc(leds[0], false);
  cc(leds[2], false);
}

void cc(int ledPin, bool isUp) {
  long currentMillis = millis();
  if (colorCycleCurrentLedPin == -1) {
    colorCycleCurrentLedPin = ledPin;
    colorCycleIsUp = isUp;
  }
  if (currentMillis - previousMillis >= colorCycleInterval) {
    if (ledPin == colorCycleCurrentLedPin && colorCycleIsUp == isUp) {
      previousMillis = currentMillis;
      if (isUp) {
        if (colorCycleLevelUp < 255) {
          colorCycleLevelUp++;
          analogWrite(colorCycleCurrentLedPin, colorCycleLevelUp);
        } else if (colorCycleLevelUp == 255) {
          colorCycleLevelUp = 0;
          colorCycleCurrentLedPin = -1;
        }
      } else {
        if (colorCycleLevelDown < 255) {
          colorCycleLevelDown++;
          analogWrite(colorCycleCurrentLedPin, 255 - colorCycleLevelDown);
        } else if (colorCycleLevelDown == 255) {
          colorCycleLevelDown = 0;
          colorCycleCurrentLedPin = -1;
        }
      }
    }
  }
}

int strobingLevelUp = 0;
void strobingLed() {
  long currentMillis = millis();
  if (currentMillis - previousMillis >= strobingInterval) {
    previousMillis = currentMillis;
    if (strobingLevelUp < 255) {
      strobingLevelUp++;
      analogWrite(strobingLedPin, strobingLevelUp);
    } else if (strobingLevelUp == 255) {
      strobingLevelUp = 0;
    }
  }
}

void rgb(int r, int g, int b) {
  analogWrite(leds[0], r);
  analogWrite(leds[1], g);
  analogWrite(leds[2], b);
}
