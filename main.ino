#include <Arduino_FreeRTOS.h>
#include <LiquidCrystal_I2C.h>
#include <SPI.h>
#include <Wire.h>
#include <math.h>
#include <semphr.h>
#include <stdio.h>
#define _USE_MATH_DEFINES
#define Btn1 7
#define left_led 13
#define right_led 12
#define back_led_1 11
#define back_led_2 10
#define front_led 9
#define front_left_led 4
#define front_right_led 6
#define joy_butten 2
#define photo_resistor 8
#define joy_x A0
#define joy_y A1
#define trigPin A2
#define echoPin A3
#define onepulse_size 3
#define radius 0.3
#define buzzer 5
#define main_led_mode_num 5
float perimeter = 2 * radius * M_PI;
float speed = 0,prev_speed=0;
TaskHandle_t handle0;
TaskHandle_t handle1;
TaskHandle_t handle2;
TaskHandle_t handle3;
TaskHandle_t handle4;
bool left_light = false;
bool right_light = false;
bool left_light_on = false;
bool right_light_on = false;
bool main_light_on = false;
bool prev_turn_on = false;
bool slowing_down = false;
long duration, interval;
float distance = 20;
int turn_animate_frame = 0;
int timer_counter = 0;
int sonic_buffer[2] = {0};
int sonic_index = 0;
int breath = 0;
int breath_up_down = 0;
int main_led_mode = 0;
bool result = false;
bool prev_result = false;
// bool measure_enable=true;
int sonic_onepulse[onepulse_size];
int onepulse_index = 0;
void JoyTask(void *pvParameters);
void ledTask(void *pvParameters);
void ultraSonicTask(void *pvParameters);
void TimerTask(void *pvParameters);
LiquidCrystal_I2C lcd(0x3F, 16, 2);

void setup()
{
  // put your setup code here, to run once:
  lcd.init();
  lcd.backlight();
  lcd.clear();

  pinMode(Btn1, INPUT);
  pinMode(left_led, OUTPUT);
  pinMode(right_led, OUTPUT);
  pinMode(joy_butten, INPUT_PULLUP);
  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);
  pinMode(photo_resistor, INPUT);
  pinMode(buzzer, OUTPUT);
  xTaskCreate(JoyTask, "Task0", 128, NULL, 1, &handle0);
  xTaskCreate(ledTask, "Task1", 128, NULL, 1, &handle1);
  xTaskCreate(ultraSonicTask, "Task2", 128, NULL, 2, &handle2);
  xTaskCreate(TimerTask, "Task3", 64, NULL, 1, &handle3);
  vTaskStartScheduler();
  Serial.begin(9600);
}

void TimerTask(void *pvParameters)
{ //0.1second
  for (;;)
  {
    timer_counter++;
    vTaskDelay(100 / portTICK_PERIOD_MS);
  }
}
void ultraSonicTask(void *pvParameters)
{
  for (;;)
  {
    digitalWrite(trigPin, LOW); // Clears the trigPin
    delayMicroseconds(2);
    /* Sets the trigPin on HIGH state for 10 ms */
    digitalWrite(trigPin, HIGH);
    delayMicroseconds(10);
    digitalWrite(trigPin, LOW);
    /* Reads Echo pin, returns sound travel time in ms */
    duration = pulseIn(echoPin, HIGH);
    /* Calculating the distance */
    distance = duration * 0.034 / 2;
    sonic_onepulse[onepulse_index++] = (distance < 7);
    onepulse_index %= onepulse_size;
    // for (int i = 0; i < onepulse_size;i++){
    //   Serial.print(sonic_onepulse[i]);
    // }
    // Serial.print('\n');
    int true_count = 0;
    int false_count = 0;
    for (int i = 0; i < onepulse_size; i++)
    {
      if (sonic_onepulse[i] == true)
      {
        true_count++;
      }
      else
      {
        false_count++;
      }
    }
    // Serial.println(true_count);
    // Serial.println(false_count);
    if (true_count == onepulse_size || false_count == onepulse_size)
    {
      prev_result = result;
      if (true_count == onepulse_size)
      {
        result = true;
      }
      else
      {
        result = false;
      }
      if (result != prev_result)
      {
        // sonic_buffer[1] = timer_counter;
        prev_speed = speed;
        float duration = (float)timer_counter * 0.1; //(second)
        if (duration > 0)
        {
          speed = perimeter * 0.5 / duration; //  speed(m/s)
        }
        else
        {
          speed = perimeter * 0.5 / 0.05;
        }
        if(prev_speed>speed){
          slowing_down = true;
        }else{
          slowing_down = false;
        }
        
        // Serial.print(speed / 1000 * 3600);
        // Serial.println("km/h");
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print(speed / 1000 * 3600*1.5);
        lcd.setCursor(5, 0);
        lcd.print("km/h");
        // lcd.setCursor(0, 1);
        // lcd.print(duration);
        timer_counter = 0;
      }
    }

    if (timer_counter>50){
      speed = 0;
      slowing_down = false;
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print(speed);
      lcd.setCursor(5, 0);
      lcd.print("km/h");
    }
    if (right_light){
      if (right_light_on){
        lcd.setCursor(5, 1);
        lcd.print("-");
        vTaskDelay(100 / portTICK_PERIOD_MS);
        lcd.setCursor(4, 1);
        lcd.print("-");
        vTaskDelay(100 / portTICK_PERIOD_MS);
        lcd.setCursor(3, 1);
        lcd.print("-");
        vTaskDelay(100 / portTICK_PERIOD_MS);
        lcd.setCursor(2, 1);
        lcd.print("<");
        vTaskDelay(100 / portTICK_PERIOD_MS);
        lcd.setCursor(1, 1);
        lcd.print("<");
        vTaskDelay(100 / portTICK_PERIOD_MS);
        lcd.setCursor(0, 1);
        lcd.print("<");
        vTaskDelay(100 / portTICK_PERIOD_MS);
      }else{
        lcd.setCursor(0, 1);
        lcd.print("      ");
      }
    }
    if(left_light){
      if(left_light_on){
        lcd.setCursor(7, 1);
        lcd.print("-");
        vTaskDelay(100 / portTICK_PERIOD_MS);
        lcd.setCursor(8, 1);
        lcd.print("-");
        vTaskDelay(100 / portTICK_PERIOD_MS);
        lcd.setCursor(9, 1);
        lcd.print("-");
        vTaskDelay(100 / portTICK_PERIOD_MS);
        lcd.setCursor(10, 1);
        lcd.print(">");
        vTaskDelay(100 / portTICK_PERIOD_MS);
        lcd.setCursor(11, 1);
        lcd.print(">");
        vTaskDelay(100 / portTICK_PERIOD_MS);
        lcd.setCursor(12, 1);
        lcd.print(">");
        vTaskDelay(100 / portTICK_PERIOD_MS);
      }else{
        lcd.setCursor(0, 7);
        lcd.print("      ");
      }
    }
    // prev_sonic_time = millis();
    //    if(distance<20){
    //      Serial.println(distance);
    //    }
    vTaskDelay(50 / portTICK_PERIOD_MS);
  }
}
void ledTask(void *pvParameters)
{
  for (;;)
  {

    if (left_light)
    {
      if (left_light_on)
      {
        // tone(buzzer, 750);
        digitalWrite(left_led, HIGH);
        digitalWrite(front_left_led, HIGH);
        vTaskDelay(250 / portTICK_PERIOD_MS);
        left_light_on = false;
      }
      else
      {
        // noTone(buzzer);
        digitalWrite(left_led, LOW);
        digitalWrite(front_left_led, LOW);
        vTaskDelay(250 / portTICK_PERIOD_MS);
        left_light_on = true;
      }
    }
    else
    {
      // noTone(buzzer);
      digitalWrite(front_left_led, LOW);
      digitalWrite(left_led, LOW);
    }
    if (right_light)
    {
      if (right_light_on)
      {

        digitalWrite(right_led, HIGH);
        digitalWrite(front_right_led, HIGH);
        vTaskDelay(250 / portTICK_PERIOD_MS);
        right_light_on = false;
      }
      else
      {
        // noTone(buzzer);
        digitalWrite(front_right_led, LOW);
        digitalWrite(right_led, LOW);
        vTaskDelay(250 / portTICK_PERIOD_MS);
        right_light_on = true;
      }
    }
    else
    {
      // noTone(buzzer);
      digitalWrite(right_led, LOW);
      digitalWrite(front_right_led, LOW);
    }

    if (right_light_on || left_light_on)
    {
      tone(buzzer, 750);
      noTone(buzzer);
    }
    if(slowing_down){
      digitalWrite(back_led_1, HIGH);
      digitalWrite(back_led_2, HIGH);
      digitalWrite(front_led, HIGH);
      vTaskDelay(100 / portTICK_PERIOD_MS);
      digitalWrite(back_led_1, LOW);
      digitalWrite(back_led_2, LOW);
      digitalWrite(front_led, LOW);
    }else{
      if (!digitalRead(photo_resistor) || main_light_on)
      {
        if (main_led_mode == 0)
        {
          digitalWrite(back_led_1, HIGH);
          digitalWrite(back_led_2, HIGH);
          digitalWrite(front_led, HIGH);
        }
        else if (main_led_mode==1)
        {
          digitalWrite(back_led_1, HIGH);
          digitalWrite(back_led_2, HIGH);
          digitalWrite(front_led, HIGH);
        }else if(main_led_mode==2){
          analogWrite(back_led_1, breath);
          analogWrite(back_led_2, breath);
          analogWrite(front_led, breath);
          if (!breath_up_down){
            if (breath<80){
              breath += 4;
            }else{
              breath += 30;
            }
          }else{
            if(breath>80){
              breath -= 30;
            }else{
              breath -= 4;
            }
            
          }
          if(breath>255){
            breath = 255;
            breath_up_down = 1;
          }
          if(breath<0){
            breath = 0;
            breath_up_down = 0;
          }
        }
        else if (main_led_mode==3)
        {
          digitalWrite(front_led, HIGH);
          digitalWrite(back_led_1, HIGH);
          vTaskDelay(30 / portTICK_PERIOD_MS);
          digitalWrite(back_led_1, LOW);
          digitalWrite(back_led_2, HIGH);
          vTaskDelay(30 / portTICK_PERIOD_MS);
          digitalWrite(back_led_2, LOW);
        }
        else if (main_led_mode == 4){
          digitalWrite(front_led, HIGH);
          digitalWrite(back_led_1, HIGH);
          digitalWrite(back_led_2, HIGH);
          vTaskDelay(500 / portTICK_PERIOD_MS);
          digitalWrite(back_led_1, LOW);
          digitalWrite(back_led_2, LOW);
          digitalWrite(front_led, LOW);
          vTaskDelay(500 / portTICK_PERIOD_MS);
        }
      }
      else
      {
        digitalWrite(back_led_1, LOW);
        digitalWrite(back_led_2, LOW);
        digitalWrite(front_led, LOW);
      }
    }

    vTaskDelay(10);
  }
}
void JoyTask(void *pvParameters)
{
  for (;;)
  {
    int x_val = analogRead(joy_x);
    int y_val = analogRead(joy_y);
    int btn_val = digitalRead(joy_butten);
    if (x_val < 10)
    {
      left_light = true;
      left_light_on = true;
      right_light = false;
      right_light_on = false;
    }
    else if (x_val > 1000)
    {
      left_light = false;
      left_light_on = false;
      right_light = true;
      right_light_on = true;
    }
    if (btn_val == 0)
    {
      left_light = false;
      right_light = false;
      left_light_on = false;
      right_light_on = false;
    }
    if (y_val > 1000)
    {
      if (!prev_turn_on)
      {
        prev_turn_on = true;
        main_led_mode++;
        main_led_mode %= main_led_mode_num;
        if (main_led_mode == 0){
          main_light_on = false;
        }else{
          main_light_on = true;
        }
        breath = 0;
        breath_up_down = 0;
      }
    }
    else
    {
      prev_turn_on = false;
    }
    if (y_val < 20)
    {
      tone(buzzer, 500);
    }
    else
    {
      noTone(buzzer);
    }
    vTaskDelay(10);
  }
}

void loop()
{
}
