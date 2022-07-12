SYSTEM_MODE(MANUAL);
SYSTEM_THREAD(ENABLED);

#include "oled-wing-adafruit.h"
OledWingAdafruit display;

const size_t UART_TX_BUF_SIZE = 20;

void onDataReceived(const uint8_t *data, size_t len, const BlePeerDevice &peer, void *context);
void oledSetup();

const BleUuid serviceUuid("6E400001-B5A3-F393-E0A9-E50E24DCCA9E");
const BleUuid rxUuid("6E400002-B5A3-F393-E0A9-E50E24DCCA9E");
const BleUuid txUuid("6E400003-B5A3-F393-E0A9-E50E24DCCA9E");
BleCharacteristic txCharacteristic("tx", BleCharacteristicProperty::NOTIFY, txUuid, serviceUuid);
BleCharacteristic rxCharacteristic("rx", BleCharacteristicProperty::WRITE_WO_RSP, rxUuid, serviceUuid, onDataReceived, NULL);

int mytime = 0;

void oledSetup()
{
  display.clearDisplay();
  display.setCursor(0, 0);
  display.setTextColor(WHITE);
  display.setTextSize(1);
}

void onDataReceived(const uint8_t *data, size_t len, const BlePeerDevice &peer, void *context)
{
  for (size_t i = 0; i < len; i++)
  {
    Serial.write(data[i]);
  }
  if (String(data[0]).equals("49"))
  { // i could probably ditch string conversion but this works and honestly I'm tired of doing this lab lo
    oledSetup();
    display.println("lurg");
    display.display();
  }
  else if (String(data[0]).equals("48"))
  {
    BLE.disconnect();
  }
}

void setup()
{
  BLE.addCharacteristic(txCharacteristic);
  BLE.addCharacteristic(rxCharacteristic);

  BleAdvertisingData data;
  data.appendServiceUUID(serviceUuid);
  data.appendLocalName("WRYYY!!"); // max 8 characters
  BLE.advertise(&data);

  BLE.on();
  display.setup();
  display.clearDisplay();

  mytime = millis();

  uint8_t txBuf[10];                    // an array of 10 bytes we can send
  txCharacteristic.setValue(txBuf, 10); // here we are sending all 10 bytes
}

void loop()
{
  RGB.control(true);
  display.loop();

  if (millis() - 1000 >= mytime)
  {
    if (BLE.connected())
    {
      RGB.color(RGB_COLOR_BLUE);
      uint8_t txBuf[UART_TX_BUF_SIZE];
      String message = "Hello!\n";
      message.toCharArray((char *)txBuf, message.length() + 1);
      txCharacteristic.setValue(txBuf, message.length() + 1);
    }
    else
    {
      RGB.color(RGB_COLOR_YELLOW);
      oledSetup();
      display.print("disconnected");
      display.display();
    }
    mytime = millis();
  }
}