#include <Arduino.h>
#include <HardwareSerial.h>
#include <HomeSpan.h>

#define BUF_SIZE 256
#define TX_PIN  16
#define RX_PIN  17

struct Projector : Service::Television {
  HardwareSerial *ESP32Serial;

  SpanCharacteristic *active; 
  byte buffer[BUF_SIZE];

  Projector() : Service::Television() {
    new Characteristic::ConfiguredName("Projector");
    ESP32Serial = new HardwareSerial(1);
    ESP32Serial->begin(9600, SERIAL_8N1, RX_PIN, TX_PIN);
    active = new Characteristic::Active(this->getPower());
  } 

  boolean update() {
    if(active->updated()) {
      this->setPower(active->getNewVal());
    }
    return true;
  }

  void setPower(uint8_t powerMode) {
    if (powerMode) {
      this->serialSend("PWR ON\r");
    } else {
      this->serialSend("PWR OFF\r");
    }
  }

  uint32_t getPower() {
    serialSend("PWR?\r");
    delay(1000);
    if (serialReceive()) {
      if (buffer[0] == 'O' && buffer[1] == 'N') {
        return 1;
      }
    }
    return 0;
  }

  int serialReceive() {
    int nBytes = 0;
    memset(&buffer[0], 0, BUF_SIZE);
    while (ESP32Serial->available() > 0) {
      this->buffer[nBytes++] = (byte)ESP32Serial->read();
      if (nBytes > BUF_SIZE) {
        break;
      }
    }
    this->buffer[nBytes] = 0x0;
    LOG0("Received from projector :%s\n", this->buffer);
    return nBytes;
  }

  void serialSend(const char *msg) {
    uint32_t len = strlen(msg);
    LOG0("Send to projector :%s\n", msg);
    ESP32Serial->write(msg, len);
  }
};

void setup() {
  Serial.begin(115200);
  homeSpan.setQRID("SPTV");
  homeSpan.setPairingCode("04277826");
  homeSpan.begin(Category::Television, "Projector Dongle");

  new SpanAccessory();

  new Projector(); 

}

void loop() {
  homeSpan.poll();
}

