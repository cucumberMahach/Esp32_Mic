#include <Arduino.h>
#include <WiFi.h>
#include <WebSocketsServer.h>
#include <driver/i2s.h>

//====PINS====
#define I2S_WS 15
#define I2S_SD 13
#define I2S_SCK 2
//============

#define I2S_PORT I2S_NUM_0
#define SAMPLE_BITS 32

//====WIFI====
const char *ssid = "WIFI-SSID";
const char *password = "WIFI-PASS";
const char *hostName = "MIC-ST-1";
//============

int _num = -1; //Websocket client number, -1 = no client
const int bufLen = 512;
const int bufLenBytes = bufLen * 4; //Bytes to read from i2s
uint32_t buf[bufLen];

WebSocketsServer webSocket(81); //Port = 81

void webSocketEvent(byte num, WStype_t type, uint8_t *payload, size_t length)
{
  Serial.printf("[event][%d] WStype = ", num);
  Serial.print(type);
  Serial.printf("; size = %d\n", length);

  switch (type)
  {
  case WStype_ERROR:
    break;
  case WStype_DISCONNECTED:
    if (num == _num)
      _num = -1;
    break;
  case WStype_CONNECTED:
    break;
  case WStype_TEXT:
  {
    Serial.printf("TEXT: %s\n", (char *)payload);
    String str((char *)payload);
    if (str == "start")
    {
      _num = num;
    }
    else if (str == "stop")
    {
      _num = -1;
    }else if (str == "mem"){
      Serial.printf("Free heap: %u\n", ESP.getFreeHeap());
      webSocket.sendTXT(num, String(ESP.getFreeHeap()));
    }
  }
  case WStype_BIN:
  {
    break;
  }
  case WStype_FRAGMENT_TEXT_START:
    break;
  case WStype_FRAGMENT_BIN_START:
    break;
  case WStype_FRAGMENT:
    break;
  case WStype_FRAGMENT_FIN:
    break;
  case WStype_PING:
    break;
  case WStype_PONG:
    break;

  default:
    break;
  }
}

void setupWiFi()
{
  WiFi.mode(WIFI_STA);

  if (WiFi.setHostname(hostName))
  {
    Serial.println("Hostname changed");
  }
  else
  {
    Serial.println("Hostname changing error");
  }

  Serial.println("Connecting to WiFi");

  WiFi.begin(ssid, password);
  int ccount = 0;
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
    ccount++;
    if (ccount >= 20)
    {
      Serial.print(WiFi.status());
      WiFi.begin(ssid, password);
      ccount = 0;
    }
  }
  Serial.println();

  Serial.print("WiFi connected. Local IP: ");
  Serial.println(WiFi.localIP());

  WiFi.setAutoReconnect(true);

  Serial.println("Wifi done!");
  Serial.println();
}

void setupWebsocket()
{
  webSocket.begin();
  webSocket.onEvent(webSocketEvent);
}

esp_err_t i2s_install()
{
  const i2s_config_t i2s_config = {
      .mode = i2s_mode_t(I2S_MODE_MASTER | I2S_MODE_RX),
      .sample_rate = 44100,
      .bits_per_sample = i2s_bits_per_sample_t(SAMPLE_BITS),
      .channel_format = I2S_CHANNEL_FMT_ONLY_LEFT,
      .communication_format = i2s_comm_format_t(I2S_COMM_FORMAT_I2S | I2S_COMM_FORMAT_I2S_MSB),
      .intr_alloc_flags = ESP_INTR_FLAG_LEVEL1, // default interrupt priority
      .dma_buf_count = 4,
      .dma_buf_len = 1024,
      .use_apll = true
      };

  return i2s_driver_install(I2S_PORT, &i2s_config, 0, NULL);
}

esp_err_t i2s_setpin()
{
  const i2s_pin_config_t pin_config = {
      .bck_io_num = I2S_SCK,
      .ws_io_num = I2S_WS,
      .data_out_num = -1,
      .data_in_num = I2S_SD};

  return i2s_set_pin(I2S_PORT, &pin_config);
}

void setup()
{
  Serial.begin(115200);
  Serial.println();
  Serial.println("Started");

  setupWiFi();
  setupWebsocket();


  Serial.println("WiFi & websocket done!");

  Serial.println("Setup I2S ...");

  auto res = i2s_install();
  Serial.printf("i2s_install: %d\n", res);
  if (res != 0)
  {
    while (1)
      delay(99999);
  }

  res = i2s_setpin();
  Serial.printf("i2s_setpin: %d\n", res);
  if (res != 0)
  {
    while (1)
      delay(99999);
  }

  res = i2s_start(I2S_PORT);
  Serial.printf("i2s_start: %d\n", res);
  if (res != 0)
  {
    while (1)
      delay(99999);
  }

  Serial.println("Done!");
}

void loop()
{
  webSocket.loop();
  if (_num != -1)
  {
    size_t bytes_read = 0; //unused
    i2s_read(I2S_PORT, buf, bufLenBytes, &bytes_read, portMAX_DELAY); //Read from i2s
    webSocket.sendBIN(_num, (uint8_t*)buf, bufLenBytes); //Send to client
  }
}