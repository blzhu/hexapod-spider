#include <ArduinoJson.h>
#include "FS.h"
//#include <LittleFS.h>
#include <Wire.h>
#include <Adafruit_PWMServoDriver.h>
#include <Adafruit_NeoPixel.h>
#include <WiFi.h>
#include <ESPmDNS.h>  
#include <WiFiUdp.h>
#include <ArduinoOTA.h>
#include <Ps3Controller.h>
#include "esp32-hal-dac.h"
#include "SPIFFS.h"

#define NEED_DISPLAY 1
#if NEED_DISPLAY
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#endif

//define value

#define SERVO_FREQ 300
#define PCA_FREQ 25000000
#define MIN_US 600
#define CENTER_US 1500
#define MAX_US 2400
#define ANGLE_SCALE 100
#define SPEAK_PIN 26
#define LED_PIN 25
#define CURRENT_PIN 34//电流检测
#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels
#define SCREEN_ADDRESS 0x3C ///< See datasheet for Address; 0x3D for 128x64, 0x3C for 128x32
#define zoomRate 100.0
#define num_avg 10
//#define LOGO_HEIGHT   16
//#define LOGO_WIDTH    16
//static const unsigned char PROGMEM logo_bmp[] =
//{ 0b00000000, 0b11000000,
//  0b00000001, 0b11000000,
//  0b00000001, 0b11000000,
//  0b00000011, 0b11100000,
//  0b11110011, 0b11100000,
//  0b11111110, 0b11111000,
//  0b01111110, 0b11111111,
//  0b00110011, 0b10011111,
//  0b00011111, 0b11111100,
//  0b00001101, 0b01110000,
//  0b00011011, 0b10100000,
//  0b00111111, 0b11100000,
//  0b00111111, 0b11110000,
//  0b01111100, 0b11110000,
//  0b01110000, 0b01110000,
//  0b00000000, 0b00110000 };
  
//variable
Adafruit_PWMServoDriver pwm1 = Adafruit_PWMServoDriver(0x41);
Adafruit_PWMServoDriver pwm2 = Adafruit_PWMServoDriver();
Adafruit_NeoPixel strip = Adafruit_NeoPixel(8, LED_PIN, NEO_GRB + NEO_KHZ800);
#if NEED_DISPLAY
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire);
#endif
int BeepVolume = 200;
int BeepWarning = 0;//响的次数
int muteBeep = 0;
WiFiUDP Udp;
unsigned int localUdpPort = 80;  // local port to listen on
char incomingPacket[255];  // buffer for incoming packets
char sendingPacket[255];
String cmdStr;
int OTAInited = 0;
int UDPStarted = 0;
const char* ssid = "zz";
const char* password = "13758118012";
const char* ssid_config = "SpiderBot";
int intvalServo = 10;
int intvalCmd = 20;
int servoThreadMode = 0;//0位置模式，1为Trot模式
/////////////////////////////////
//六条腿逆时针排列，第一腿是左前腿，最后腿是右前腿
//每条腿三个关节，从身体到足端为1，2，3
//左边腿的舵机pin为0-15，右边为16-31,根据PCB和安装的位置设定

const int P2=16;
const int Legs_pin[6][3]={{8,9,10},{4,3,2},{7,6,5},{5+P2,6+P2,7+P2},{2+P2,3+P2,4+P2},{10+P2,9+P2,8+P2}};
//坐标设置，头部向前为y正，向右为x正，向下为z正
//左边中间腿Legs_pin[1]为逆解计算腿，其他腿根据这个计算结果做坐标变换
//从身体到足端腿上的三段长度，分别为25，42，92，测量有误差
const float La = 25;
const float Lb = 42;
const float Lc = 92;
//身体的长宽是四边腿的原点形成的长方形，中间的宽是两边中间腿的两个原点的距离
const float body_width = 73;
const float body_length = 131;
const float body_center_width = 99;
///////
volatile float curPostion[6][3];
volatile float expPostion[6][3];
volatile float curAngle[6][3];
volatile float expAngle[6][3];
volatile float curCurrent;//当前电流
volatile float maxCurrent;
float legOffset[6][3]={0};
float initPos[6][3];
/////////////////////////////////

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  Serial.println("Start robot !!!!!");
  pinMode(LED_PIN, OUTPUT);
  pinMode(SPEAK_PIN, OUTPUT);
  pinMode(CURRENT_PIN,INPUT);
  analogSetWidth(12);
  pwmInit();
  psInit();
  readConfig();
  displayInit();
  ledRingInit();
  if(0)
    initOTA();
  CreateFreeRTOS();
  BeepWarning = 2;
//  MakeBeep(1);
//  initPostion();
}

void loop() {
  // put your main code here, to run repeatedly:
//  rainbowCycle(20);
//  testdrawline();
}

void pwmInit()
{
  pwm1.begin();
  pwm1.setOscillatorFrequency(PCA_FREQ);
  pwm1.setPWMFreq(SERVO_FREQ); 
  pwm2.begin();
  pwm2.setOscillatorFrequency(PCA_FREQ);
  pwm2.setPWMFreq(SERVO_FREQ);   
}

void setAngle(int n,double angle)
{ 
   int pulse = 0;
   if(angle<0)
      pulse = MIN_US;
   else if(angle>180)
      pulse = MAX_US;
   else 
      pulse = map(angle*ANGLE_SCALE,0,180*ANGLE_SCALE,MIN_US,MAX_US);
//  Serial.printf("leg =%d , angle = %f\n",n,angle);
  if(n>=16)
    pwm2.writeMicroseconds(n-16,pulse);
  else
    pwm1.writeMicroseconds(n,pulse);   
}

void setAngleForAll(double angle)
{
  for(int i=0;i<31;i++)
  {
    setAngle(i,angle);
  }
}

void setAngleForLeft(double angle)
{
  for(int i=0;i<16;i++)
  {
    setAngle(i,angle);
  }
}

void setAngleForRight(double angle)
{
  for(int i=16;i<32;i++)
  {
    setAngle(i,angle);
  }
}

void MakeBeep(int nCount)
{
//  if(muteBeep)return; 
  if(nCount<=0)
    return;
  for(int i = 0;i<nCount;i++){
    dacWrite(SPEAK_PIN,BeepVolume); 
    delay(100);
    dacWrite(SPEAK_PIN,0); 
    delay(100);
  }
}



void displayInit()
{
#if NEED_DISPLAY
  if(!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) {
    Serial.println(F("SSD1306 allocation failed"));
    return;
  }  
  Serial.println("OLED begun");
  // Show initial display buffer contents on the screen --
  // the library initializes this with an Adafruit splash screen.
  display.display();
  delay(200); // Pause for 2 seconds

  // Clear the buffer
  display.clearDisplay();
  display.setTextSize(2); // Draw 2X-scale text
  display.setTextColor(SSD1306_WHITE);

//  // Draw a single pixel in white
//  display.drawPixel(10, 10, SSD1306_WHITE);
//
//  // Show the display buffer on the screen. You MUST call display() after
//  // drawing commands to make them visible on screen!
//  display.display();
#endif
}
#if NEED_DISPLAY
void testdrawrect(void) {
  display.clearDisplay();

  for(int16_t i=0; i<display.height()/2; i+=2) {
    display.drawRect(i, i, display.width()-2*i, display.height()-2*i, SSD1306_WHITE);
    display.display(); // Update screen with each newly-drawn rectangle
    vTaskDelay(1);
  }
}
void testdrawline() {
  int16_t i;

  display.clearDisplay(); // Clear display buffer

  for(i=0; i<display.width(); i+=4) {
    display.drawLine(0, 0, i, display.height()-1, SSD1306_WHITE);
    display.display(); // Update screen with each newly-drawn line
    vTaskDelay(1);
  }
  for(i=0; i<display.height(); i+=4) {
    display.drawLine(0, 0, display.width()-1, i, SSD1306_WHITE);
    display.display();
    vTaskDelay(1);
  }
  vTaskDelay(250);

  display.clearDisplay();

  for(i=0; i<display.width(); i+=4) {
    display.drawLine(0, display.height()-1, i, 0, SSD1306_WHITE);
    display.display();
    vTaskDelay(1);
  }
  for(i=display.height()-1; i>=0; i-=4) {
    display.drawLine(0, display.height()-1, display.width()-1, i, SSD1306_WHITE);
    display.display();
    vTaskDelay(1);
  }
  vTaskDelay(250);

  display.clearDisplay();

  for(i=display.width()-1; i>=0; i-=4) {
    display.drawLine(display.width()-1, display.height()-1, i, 0, SSD1306_WHITE);
    display.display();
    vTaskDelay(1);
  }
  for(i=display.height()-1; i>=0; i-=4) {
    display.drawLine(display.width()-1, display.height()-1, 0, i, SSD1306_WHITE);
    display.display();
    delay(1);
  }
  vTaskDelay(250);

  display.clearDisplay();

  for(i=0; i<display.height(); i+=4) {
    display.drawLine(display.width()-1, 0, 0, i, SSD1306_WHITE);
    display.display();
    vTaskDelay(1);
  }
  for(i=0; i<display.width(); i+=4) {
    display.drawLine(display.width()-1, 0, i, display.height()-1, SSD1306_WHITE);
    display.display();
  }

//  vTaskDelay(2000); // Pause for 2 seconds
}
#endif
void CreateFreeRTOS()
{
    xTaskCreatePinnedToCore(
    ServoThread
    ,  "ServoThread"
    ,  10240  // Stack size
    ,  NULL
    ,  2  // Priority
    ,  NULL ,1);
   
    xTaskCreatePinnedToCore(
    CMDThread
    ,  "CMDThread"
    ,  10240  // Stack size
    ,  NULL
    ,  2  // Priority
    ,  NULL ,0);  
    
    xTaskCreatePinnedToCore(
    ActionThread
    ,  "ActionThread"
    ,  4096  // Stack size
    ,  NULL
    ,  2  // Priority
    ,  NULL ,0);
    
    
    xTaskCreatePinnedToCore(
    UtilityThread
    ,  "UtilityThread"
    ,  4096  // Stack size
    ,  NULL
    ,  2  // Priority
    ,  NULL ,0);   

    xTaskCreatePinnedToCore(
    LedRingThread
    ,  "LedRingThread"
    ,  1024  // Stack size
    ,  NULL
    ,  2  // Priority
    ,  NULL ,0);   
    
    xTaskCreatePinnedToCore(
    DisplayThread
    ,  "DisplayThread"
    ,  1024  // Stack size
    ,  NULL
    ,  2  // Priority
    ,  NULL ,0);     
}
void ServoThread(void *pvParameters) 
{
  (void) pvParameters;
  for (;;)
  {
    timer_process_servo();
    vTaskDelay(intvalServo);
  }
  vTaskDelete(NULL);
}

void CMDThread(void *pvParameters) 
{
  (void) pvParameters;
  for (;;)
  {
    timer_process_cmd();
    vTaskDelay(intvalCmd);
  }
  vTaskDelete(NULL);
}

void ActionThread(void *pvParameters) 
{
  (void) pvParameters;
  for (;;)
  {
    timer_process_action();
    vTaskDelay(20);
  }
  vTaskDelete(NULL);
}

void UtilityThread(void *pvParameters) 
{
  (void) pvParameters;
  for (;;)
  {
    timer_process_utility();
    vTaskDelay(20);
  }
  vTaskDelete(NULL);
}
void LedRingThread(void *pvParameters) 
{
  (void) pvParameters;
  for (;;)
  {
    timer_process_ledRing();
    vTaskDelay(20);
  }
  vTaskDelete(NULL); 
}

void DisplayThread(void *pvParameters) 
{
  (void) pvParameters;
  for (;;)
  {
    timer_process_display();
    vTaskDelay(100);
  }
  vTaskDelete(NULL); 
}

void timer_process_servo()
{
  if(servoThreadMode == 0)//位置模式
    check_expect_postion();
  else if(servoThreadMode == 1)//trot模式
    check_cycle_trot();
}
void timer_process_cmd()
{
//  check_UdpCmd();
  checkPS();
//  check_SerialCmd();
}
void timer_process_utility()
{
  static int utilCount = 0;
  if(utilCount%5 == 0){
    checkBeep();
    checkOTA();
    checkCurrent();
  }  
  utilCount++;
}

void timer_process_action()
{
    if(cmdStr.length()==0)
      return;
//    Serial.println(cmdStr.c_str());
    String cmd = cmdStr;
    cmdStr = "";
}

void timer_process_ledRing()
{
  if(isSetting())return;
  rainbowCycle(5);
}

void timer_process_display()
{
#if NEED_DISPLAY
//  Serial.println("OLED begun");
//  display.clearDisplay();
//
//  display.setCursor(20, 20);
//  display.println(curCurrent);
//  display.display();      // Show initial text
#endif
}

void checkBeep()
{
  if(muteBeep)
    return;  
  static int i = 0;
    if(BeepWarning)
    {

      if(i%2==0)
      {
//        dacWrite(SPEAK_PIN,BeepVolume); 
        digitalWrite(SPEAK_PIN,HIGH); 
      }
      else{
        digitalWrite(SPEAK_PIN,LOW); 
        BeepWarning--;
      }
      i++;
    }
    else
    {
      i = 0;
//      dacWrite(SPEAK_PIN,0); 
      digitalWrite(SPEAK_PIN,LOW); 
    }
   
}

void initOTA()
{
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  while (WiFi.waitForConnectResult() != WL_CONNECTED) {
    Serial.println("Connection Failed! Rebooting...");
    delay(5000);
    ESP.restart();
  }
  ArduinoOTA
    .onStart([]() {
      String type;
      if (ArduinoOTA.getCommand() == U_FLASH)
        type = "sketch";
      else // U_SPIFFS
        type = "filesystem";

      // NOTE: if updating SPIFFS this would be the place to unmount SPIFFS using SPIFFS.end()
      Serial.println("Start updating " + type);
    })
    .onEnd([]() {
      Serial.println("\nEnd");
    })
    .onProgress([](unsigned int progress, unsigned int total) {
      Serial.printf("Progress: %u%%\r", (progress / (total / 100)));
    })
    .onError([](ota_error_t error) {
      Serial.printf("Error[%u]: ", error);
      if (error == OTA_AUTH_ERROR) Serial.println("Auth Failed");
      else if (error == OTA_BEGIN_ERROR) Serial.println("Begin Failed");
      else if (error == OTA_CONNECT_ERROR) Serial.println("Connect Failed");
      else if (error == OTA_RECEIVE_ERROR) Serial.println("Receive Failed");
      else if (error == OTA_END_ERROR) Serial.println("End Failed");
    });

  ArduinoOTA.begin();
  OTAInited = 1;
  Serial.println("Ready");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());  
}
void checkOTA()
{
  if(OTAInited)
    ArduinoOTA.handle();
}

void checkCurrent()
{
  int sum = 0;
  int value = 0;
  int mapValue = 0;
  
  for(int i=0;i<20;i++)
  {
    value = analogRead(CURRENT_PIN);
    sum=sum+value;
    vTaskDelay(1);
  }
  value = sum/20;
  mapValue = map(value,0,4096,0,3300);
  curCurrent = abs(mapValue-3300/2)/44.0;
  if(maxCurrent<curCurrent)maxCurrent = curCurrent;
//  Serial.printf("value = %d,mv = %d,cur = %f,max = %f\n",value,mapValue,curCurrent,maxCurrent);

}
