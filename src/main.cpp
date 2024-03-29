#include <Arduino.h>
#include <Base64.h>
#include "soc/soc.h"           // Disable brownour problems
#include "soc/rtc_cntl_reg.h"  // Disable brownour problems
#include "esp_camera.h"
#include "WiFi.h"
#include <HTTPClient.h>

#define PWDN_GPIO_NUM     32
#define RESET_GPIO_NUM    -1
#define XCLK_GPIO_NUM      0
#define SIOD_GPIO_NUM     26
#define SIOC_GPIO_NUM     27

#define Y9_GPIO_NUM       35
#define Y8_GPIO_NUM       34
#define Y7_GPIO_NUM       39
#define Y6_GPIO_NUM       36
#define Y5_GPIO_NUM       21
#define Y4_GPIO_NUM       19
#define Y3_GPIO_NUM       18
#define Y2_GPIO_NUM        5
#define VSYNC_GPIO_NUM    25
#define HREF_GPIO_NUM     23
#define PCLK_GPIO_NUM     22

#define MOTOR_DER_F 14
#define MOTOR_DER_A 15
#define MOTOR_IZQ_F 12
#define MOTOR_IZQ_A 13

#define CANAL_DER_F 8
#define CANAL_DER_A 0
#define CANAL_IZQ_F 1
#define CANAL_IZQ_A 2
#define CANAL_LED 3

#define RESOLUCION 8
#define FRECUENCIA 1000

#define VELOCIDAD 200

enum Motor {DER,IZQ};

void initWifi(){
  const char* ssid = "Estudiantes";
  const char* password = "educar_2018";
  
  Serial.begin(115200);
  WiFi.begin(ssid, password);

  Serial.print("Conectando...");
  while (WiFi.status() != WL_CONNECTED) { //Check for the connection
    delay(500);
    Serial.print(".");
  }

  Serial.print("Conectado con éxito, mi IP es: ");
  Serial.println(WiFi.localIP());
}

void initCamera() {
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
  config.grab_mode = CAMERA_GRAB_LATEST ;
  
  
  // WARNING!!! PSRAM IC required for UXGA resolution and high JPEG quality
  //            Ensure ESP32 Wrover Module or other board with PSRAM is selected
  //            Partial images will be transmitted if image exceeds buffer size
  //   
  // if PSRAM IC present, init with UXGA resolution and higher JPEG quality
  //                      for larger pre-allocated frame buffer.
  if(psramFound()){  
    config.frame_size = FRAMESIZE_UXGA;
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
    ESP.restart();
  }

  
  sensor_t * s = esp_camera_sensor_get();
  // initial sensors are flipped vertically and colors are a bit saturated
  if (s->id.PID == OV3660_PID) {
    s->set_vflip(s, 1); // flip it back
    s->set_brightness(s, 1); // up the brightness just a bit
    s->set_saturation(s, -2); // lower the saturation
  }
  // drop down frame size for higher initial frame rate
  s->set_framesize(s, FRAMESIZE_QVGA);    
  pinMode(4, OUTPUT);
}

void initMotor(){

  ledcSetup(CANAL_DER_F,FRECUENCIA,RESOLUCION);
  ledcSetup(CANAL_DER_A,FRECUENCIA,RESOLUCION);
  ledcSetup(CANAL_IZQ_F,FRECUENCIA,RESOLUCION);
  ledcSetup(CANAL_IZQ_A,FRECUENCIA,RESOLUCION);

  ledcAttachPin(MOTOR_DER_F,CANAL_DER_F);  
  ledcAttachPin(MOTOR_DER_A,CANAL_DER_A);
  ledcAttachPin(MOTOR_IZQ_F,CANAL_IZQ_F);  
  ledcAttachPin(MOTOR_IZQ_A,CANAL_IZQ_A);
  }
void mover_adelante(int lado, int vel){
 
  
  if(lado == DER){
    ledcWrite(CANAL_DER_F,vel);
    ledcWrite(CANAL_DER_A,0);
  }
  else{
   ledcWrite(CANAL_IZQ_F,vel);
   ledcWrite(CANAL_IZQ_A,0);
  }
  //digitalWrite(PIN_2,1);
  //digitalWrite(MOTOR_DER_F,0);
}

void mover_atras(int lado, int vel){
 
 if(lado == DER){
    ledcWrite(CANAL_DER_F,0);
    ledcWrite(CANAL_DER_A,vel);
  }
  else{
   ledcWrite(CANAL_IZQ_F,0);
   ledcWrite(CANAL_IZQ_A,vel);
  }
}



void avanzar(){
  mover_adelante(DER,VELOCIDAD);
  mover_adelante(IZQ,VELOCIDAD);
}


void doblar_derecha(int tiempo = 0){
  mover_adelante(IZQ,VELOCIDAD);
  mover_atras(DER,VELOCIDAD);
  delay(tiempo);
}

void doblar_izquierda(int tiempo = 0){
  mover_adelante(DER,VELOCIDAD);
  mover_atras(IZQ,VELOCIDAD);
  delay(tiempo);
}

void retroceder(){
  mover_atras(IZQ,VELOCIDAD);
  mover_atras(DER,VELOCIDAD);
}

void stop(){
  mover_adelante(IZQ,0);
  mover_atras(DER,0);

}

String get_enocde_image() {
  digitalWrite(4,1);
  Serial.println("encode image");
  camera_fb_t * fb = NULL;
  
  fb = esp_camera_fb_get();  
  
  if(!fb) {
    Serial.println("Camera capture failed");
    return "Camera capture failed";
  }  
  digitalWrite(4,0);
  char *input = (char *)fb->buf;
  char output[Base64.encodedLength(3)];
  String imageFile;
  for (int i=0;i<fb->len;i++) {
    Base64.encode(output, (input++), 3);
    if (i%3==0) imageFile += String(output);
  }
  
  //Serial.println(imageFile);

  esp_camera_fb_return(fb);
  return imageFile;
}

void request(){
  // Serial.println("requestaaaaaaaaaaaaaaaa");
  HTTPClient http;
  String datos_a_enviar = get_enocde_image();
  
  http.begin("http://10.0.10.75:8000/");        //Indicamos el destino
  http.addHeader("Content-Type", "plain-text"); //Preparamos el header text/plain si solo vamos a enviar texto plano sin un paradigma llave:valor.

  int codigo_respuesta = http.sendRequest("GET",(uint8_t *) datos_a_enviar.c_str(), datos_a_enviar.length());   //Enviamos el post pasándole, los datos que queremos enviar. (esta función nos devuelve un código que guardamos en un int)
  
  if(codigo_respuesta>0){
    Serial.println("Código HTTP ► " + String(codigo_respuesta));   //Print return code

    if(codigo_respuesta == 200){
      String cuerpo_respuesta = http.getString();
      Serial.println("El servidor respondio:");
      Serial.println(cuerpo_respuesta);
    }
    }else{

     Serial.print("Error codigo: ");
     Serial.println(codigo_respuesta);

    }

    http.end();  //libero recursos
}
void setup() {
  // put your setup code here, to run once:
  // initWifi();
//  
  initCamera();
  Serial.begin(115200);
 
  digitalWrite(4,1);
  initMotor();

  Serial.println("Start");
  
  
}

void loop() {
  // put your main code here, to run repeatedly:
  Serial.println("loop");
  mover_adelante(DER,VELOCIDAD);
  mover_adelante(IZQ,VELOCIDAD);
  digitalWrite(4,0);
  delay(3000);
  mover_adelante(IZQ,0);
  mover_adelante(DER,0);
  digitalWrite(4,1);
  delay(3000);
  
  //request();
  //delay(5000);
}

