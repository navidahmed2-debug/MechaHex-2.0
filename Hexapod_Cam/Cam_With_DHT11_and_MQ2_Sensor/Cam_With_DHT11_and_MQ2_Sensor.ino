#include "esp_camera.h"
#include <WiFi.h>
#include "esp_timer.h"
#include "img_converters.h"
#include "Arduino.h"
#include "fb_gfx.h"
#include "soc/soc.h"           // disable brownout problems
#include "soc/rtc_cntl_reg.h"  // disable brownout problems
#include "esp_http_server.h"

#define sensor_t adafruit_sensor_t
#include <Adafruit_Sensor.h>
#undef sensor_t

#include <DHT.h>

#define DHTPIN 2           // GPIO pin where DHT11 data pin is connected
#define DHTTYPE DHT11      // Define the type of DHT sensor
DHT dht(DHTPIN, DHTTYPE);  // Initialize DHT sensor

#define MQ2_PIN 14
float gasLevel = 0.0;

// Replace with your network credentials
const char *ssid = "axion";
const char *password = "12345678";

#define PART_BOUNDARY "123456789000000000000987654321"

#define CAMERA_MODEL_AI_THINKER

#if defined(CAMERA_MODEL_AI_THINKER)
#define PWDN_GPIO_NUM 32
#define RESET_GPIO_NUM -1
#define XCLK_GPIO_NUM 0
#define SIOD_GPIO_NUM 26
#define SIOC_GPIO_NUM 27

#define Y9_GPIO_NUM 35
#define Y8_GPIO_NUM 34
#define Y7_GPIO_NUM 39
#define Y6_GPIO_NUM 36
#define Y5_GPIO_NUM 21
#define Y4_GPIO_NUM 19
#define Y3_GPIO_NUM 18
#define Y2_GPIO_NUM 5
#define VSYNC_GPIO_NUM 25
#define HREF_GPIO_NUM 23
#define PCLK_GPIO_NUM 22
#define FLASH_GPIO_NUM 4  // Flash LED pin

#else
#error "Camera model not selected"
#endif

static const char *_STREAM_CONTENT_TYPE = "multipart/x-mixed-replace;boundary=" PART_BOUNDARY;
static const char *_STREAM_BOUNDARY = "\r\n--" PART_BOUNDARY "\r\n";
static const char *_STREAM_PART = "Content-Type: image/jpeg\r\nContent-Length: %u\r\n\r\n";

httpd_handle_t camera_httpd = NULL;
httpd_handle_t stream_httpd = NULL;

bool ledState = false;
float temperature = 0.0;
float humidity = 0.0;

static const char PROGMEM INDEX_HTML[] = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
  <meta charset="UTF-8">
  <title>Camera with Sensor Data</title>
  <meta name="viewport" content="width=device-width, initial-scale=1">
  <style>
    body { 
      font-family: Arial, sans-serif; 
      background-color: #333; 
      color: #eee; 
      margin: 0; 
      padding-top: 20px; 
      display: flex; 
      flex-direction: column; 
      align-items: center;
    }
    h1 { 
      font-size: 1.5em; 
      color: #00ff00; 
      margin: 10px 0; 
      text-transform: uppercase;
    }
    .data-table {
      display: table;
      width: 100%;
      max-width: 400px;
      margin-top: 15px;
      background-color: #444; 
      border-radius: 10px; 
      padding: 15px; 
    }
    .data-row {
      display: table-row;
    }
    .data-cell {
      display: table-cell;
      padding: 8px;
      text-align: center;
      border-bottom: 1px solid #555;
    }
    .data-cell:last-child { border-bottom: none; }
    button { 
      padding: 12px 20px; 
      font-size: 16px; 
      border: none; 
      background-color: #007bff; 
      color: #fff; 
      cursor: pointer; 
      border-radius: 5px; 
      margin-top: 15px;
      width: 100%;
      max-width: 400px;
      transition: background-color 0.3s ease;
    }
    button:hover { background-color: #0056b3; }
    img { 
      width: 100%; 
      max-width: 400px; 
      border-radius: 10px; 
      margin-top: 15px;
      transform: rotate(180deg); /* Flip the image vertically */
    }
  </style>
</head>
<body>
  <h1>Camera with Sensor Data</h1>
  
  <img src="" id="photo">
  
  <button onclick="toggleFlashlight()">Toggle Flashlight</button>
  
  <div class="data-table">
    <div class="data-row">
      <div class="data-cell"><strong>🌡️ Temperature:</strong> <span id="temperature">--</span> °C</div>
    </div>
    <div class="data-row">
      <div class="data-cell"><strong>💧 Humidity:</strong> <span id="humidity">--</span> %</div>
    </div>
    <div class="data-row">
      <div class="data-cell"><strong>♨️ Gas Level:</strong> <span id="gasLevel">--</span> ppm</div>
    </div>
  </div>

  <script>
    function updateData() {
      fetch('/data').then(response => response.json()).then(data => {
        document.getElementById("temperature").innerText = data.temperature;
        document.getElementById("humidity").innerText = data.humidity;
        document.getElementById("gasLevel").innerText = data.gasLevel;
      });
    }
    function toggleFlashlight() {
      fetch("/flash");
    }
    window.onload = function() {
      document.getElementById("photo").src = window.location.href.slice(0, -1) + ":81/stream";
      setInterval(updateData, 5000); // Update sensor data every 5 seconds
    };
  </script>
</body>
</html>
)rawliteral";

static esp_err_t index_handler(httpd_req_t *req) {
  httpd_resp_set_type(req, "text/html");
  return httpd_resp_send(req, (const char *)INDEX_HTML, strlen(INDEX_HTML));
}

static esp_err_t data_handler(httpd_req_t *req) {
  char json_response[100];
  snprintf(json_response, sizeof(json_response), "{\"temperature\":%.1f,\"humidity\":%.1f,\"gasLevel\":%.2f}", temperature, humidity, gasLevel);
  httpd_resp_set_type(req, "application/json");
  return httpd_resp_send(req, json_response, strlen(json_response));
}

static esp_err_t stream_handler(httpd_req_t *req) {
  camera_fb_t *fb = NULL;
  esp_err_t res = ESP_OK;
  size_t _jpg_buf_len = 0;
  uint8_t *_jpg_buf = NULL;
  char *part_buf[64];

  res = httpd_resp_set_type(req, _STREAM_CONTENT_TYPE);
  if (res != ESP_OK) {
    return res;
  }

  while (true) {
    fb = esp_camera_fb_get();
    if (!fb) {
      Serial.println("Camera capture failed");
      res = ESP_FAIL;
    } else {
      if (fb->width > 400) {
        if (fb->format != PIXFORMAT_JPEG) {
          bool jpeg_converted = frame2jpg(fb, 80, &_jpg_buf, &_jpg_buf_len);
          esp_camera_fb_return(fb);
          fb = NULL;
          if (!jpeg_converted) {
            Serial.println("JPEG compression failed");
            res = ESP_FAIL;
          }
        } else {
          _jpg_buf_len = fb->len;
          _jpg_buf = fb->buf;
        }
      }
    }
    if (res == ESP_OK) {
      size_t hlen = snprintf((char *)part_buf, 64, _STREAM_PART, _jpg_buf_len);
      res = httpd_resp_send_chunk(req, (const char *)part_buf, hlen);
    }
    if (res == ESP_OK) {
      res = httpd_resp_send_chunk(req, (const char *)_jpg_buf, _jpg_buf_len);
    }
    if (res == ESP_OK) {
      res = httpd_resp_send_chunk(req, _STREAM_BOUNDARY, strlen(_STREAM_BOUNDARY));
    }
    if (fb) {
      esp_camera_fb_return(fb);
      fb = NULL;
      _jpg_buf = NULL;
    } else if (_jpg_buf) {
      free(_jpg_buf);
      _jpg_buf = NULL;
    }
    if (res != ESP_OK) {
      break;
    }
  }
  return res;
}

static esp_err_t flash_handler(httpd_req_t *req) {
  ledState = !ledState;
  digitalWrite(FLASH_GPIO_NUM, ledState);
  return httpd_resp_send(req, "Flash toggled", HTTPD_RESP_USE_STRLEN);
}

void startCameraServer() {
  httpd_config_t config = HTTPD_DEFAULT_CONFIG();
  config.server_port = 80;

  httpd_uri_t index_uri = {
    .uri = "/",
    .method = HTTP_GET,
    .handler = index_handler,
    .user_ctx = NULL
  };

  httpd_uri_t data_uri = {
    .uri = "/data",
    .method = HTTP_GET,
    .handler = data_handler,
    .user_ctx = NULL
  };

  httpd_uri_t stream_uri = {
    .uri = "/stream",
    .method = HTTP_GET,
    .handler = stream_handler,
    .user_ctx = NULL
  };

  httpd_uri_t flash_uri = {
    .uri = "/flash",
    .method = HTTP_GET,
    .handler = flash_handler,
    .user_ctx = NULL
  };

  if (httpd_start(&camera_httpd, &config) == ESP_OK) {
    httpd_register_uri_handler(camera_httpd, &index_uri);
    httpd_register_uri_handler(camera_httpd, &flash_uri);
    httpd_register_uri_handler(camera_httpd, &data_uri);
  }

  config.server_port += 1;
  config.ctrl_port += 1;

  if (httpd_start(&stream_httpd, &config) == ESP_OK) {
    httpd_register_uri_handler(stream_httpd, &stream_uri);
  }
}

void setup() {
  WRITE_PERI_REG(RTC_CNTL_BROWN_OUT_REG, 0);  //disable brownout detector

  Serial.begin(115200);
  Serial.setDebugOutput(false);

  dht.begin();  // Initialize DHT sensor
  pinMode(MQ2_PIN, INPUT);

  camera_config_t config;
  config.ledc_channel = LEDC_CHANNEL_0;
  config.ledc_timer = LEDC_TIMER_0;
  config.pin_d0 = Y2_GPIO_NUM;
  config.pin_d1 = Y3_GPIO_NUM;
  config.pin_d2 = Y4_GPIO_NUM;
  config.pin_d3 = Y5_GPIO_NUM;
  config.pin_d4 = Y6_GPIO_NUM;
  config.pin_d5 = Y7_GPIO_NUM;
  config.pin_d6 = Y8_GPIO_NUM;
  config.pin_d7 = Y9_GPIO_NUM;
  config.pin_xclk = XCLK_GPIO_NUM;
  config.pin_pclk = PCLK_GPIO_NUM;
  config.pin_vsync = VSYNC_GPIO_NUM;
  config.pin_href = HREF_GPIO_NUM;
  config.pin_sscb_sda = SIOD_GPIO_NUM;
  config.pin_sscb_scl = SIOC_GPIO_NUM;
  config.pin_pwdn = PWDN_GPIO_NUM;
  config.pin_reset = RESET_GPIO_NUM;
  config.xclk_freq_hz = 20000000;
  config.pixel_format = PIXFORMAT_JPEG;

  if (psramFound()) {
    config.frame_size = FRAMESIZE_VGA;
    config.jpeg_quality = 10;
    config.fb_count = 2;
  } else {
    config.frame_size = FRAMESIZE_SVGA;
    config.jpeg_quality = 12;
    config.fb_count = 1;
  }

  esp_err_t err = esp_camera_init(&config);
  if (err != ESP_OK) {
    Serial.printf("Camera init failed with error 0x%x", err);
    return;
  }

  pinMode(FLASH_GPIO_NUM, OUTPUT);
  digitalWrite(FLASH_GPIO_NUM, LOW);

  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.print("Camera Stream Ready! Go to: http://");
  Serial.println(WiFi.localIP());

  startCameraServer();
}

void loop() {
  temperature = dht.readTemperature();
  humidity = dht.readHumidity();
  gasLevel = analogRead(MQ2_PIN);

  if (isnan(temperature) || isnan(humidity)) {
    Serial.println("Failed to read from DHT sensor!");
  } else {
    Serial.printf("Temperature: %.1f °C, Humidity: %.1f %%, Gas Level: %.2f ppm\n", temperature, humidity, gasLevel);
  }

  delay(5000);  // Update sensor readings every 5 seconds
}