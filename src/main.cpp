#include <Arduino.h>
#include <WiFi.h>
#include <WebServer.h>
#include <esp_camera.h>

WebServer server(80);

// Brochage de la caméra pour Seeed XIAO ESP32S3 Sense
#define PWDN_GPIO_NUM     -1
#define RESET_GPIO_NUM    -1
#define XCLK_GPIO_NUM      10
#define SIOD_GPIO_NUM      40
#define SIOC_GPIO_NUM      39

#define Y9_GPIO_NUM        48
#define Y8_GPIO_NUM        11
#define Y7_GPIO_NUM        12
#define Y6_GPIO_NUM        14
#define Y5_GPIO_NUM        16
#define Y4_GPIO_NUM        18
#define Y3_GPIO_NUM        17
#define Y2_GPIO_NUM        15
#define VSYNC_GPIO_NUM     38
#define HREF_GPIO_NUM      47
#define PCLK_GPIO_NUM      13

void setup() {
  Serial.begin(115200);

  // Initialisation de la caméra
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
  config.pin_sccb_sda = SIOD_GPIO_NUM;
  config.pin_sccb_scl = SIOC_GPIO_NUM;
  config.pin_pwdn = PWDN_GPIO_NUM;
  config.pin_reset = RESET_GPIO_NUM;
  config.xclk_freq_hz = 20000000;
  config.pixel_format = PIXFORMAT_JPEG;

  // Initialisation selon la résolution souhaitée
  config.frame_size = FRAMESIZE_VGA;
  config.jpeg_quality = 30;
  config.fb_count = 2;

  // Initialisation de la caméra
  esp_err_t err = esp_camera_init(&config);
  if (err != ESP_OK) {
    Serial.printf("Erreur d'initialisation de la caméra: 0x%x", err);
    return;
  }

  // Connexion au WiFi
  WiFi.begin(SSID_WIFI, PWD_WIFI);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi connecté");
  Serial.print("Adresse IP: ");
  delay(10000);
  Serial.println(WiFi.localIP());

  // Route pour le flux MJPEG
  server.on("/stream", HTTP_GET, [](){
    WiFiClient client = server.client();

    // En-têtes initiaux (pas de Connection: close)
    client.print("HTTP/1.1 200 OK\r\n");
    client.print("Content-Type: multipart/x-mixed-replace; boundary=frame\r\n");
    client.print("Cache-Control: no-cache\r\n");
    client.print("Pragma: no-cache\r\n");
    client.print("\r\n");

    while (client.connected()) {
      camera_fb_t *fb = esp_camera_fb_get();
      if (!fb) {
        Serial.println("Échec de la capture");
        break;
      }

      // En-tête de la partie avec Content-Length
      client.print("--frame\r\n");
      client.print("Content-Type: image/jpeg\r\n");
      client.print("Content-Length: " + String(fb->len) + "\r\n");
      client.print("\r\n");

      // Envoi des bytes JPEG bruts
      size_t written = client.write(fb->buf, fb->len);
      if (written != fb->len) {
        Serial.printf("Client write mismatch: %u/%u\n", (unsigned)written, (unsigned)fb->len);
        esp_camera_fb_return(fb);
        break;
      }

      client.print("\r\n");
      esp_camera_fb_return(fb);

      // Laisser le temps au réseau et vérifier la connexion
      delay(10);
      if (!client.connected()) {
        Serial.println("Client disconnected");
        break;
      }
    }

    if (client) client.stop();
  });

  server.begin();
}

void loop() {
  server.handleClient();
}
