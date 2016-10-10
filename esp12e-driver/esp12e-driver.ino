
/* Automated Watering System
 * Copyright (C) 2016 Amitesh Singh <singh.amitesh@gmail.com>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library;
 * if not, see <http://www.gnu.org/licenses/>.
 */
 
#include <Arduino.h>

#include <ESP8266WiFi.h>
#include <ESP8266WiFiMulti.h>
#include <ESP8266HTTPClient.h>

#include <Ticker.h>

#define DEBUG

Ticker tick;

static const char *host = "192.168.0.101";
static const uint16_t port = 8080;
static const uint8_t portMotor = 5; // GPIO_5
static const char *ssid = "AMIDUINO";
static const char *ssidPasswd = "password";

#define MOTOR_RUNNING_TIME 60 * 2 // 2 mins
#define MOISTURE_THRESHOLD 600
#define MOISTURE_THRESHOLD_LIMIT 800

ESP8266WiFiMulti WifiMulti;

enum {
     MOTOR_STOP = 0,
     MOTOR_RUNNING = 1,
     MOTOR_PRE_STOP = 2,
};

enum {
     JOB_SEND_EMAIL_START = 1,
     JOB_SEND_EMAIL_FINISH,
     JOB_GET_MOISTURE_THRESHOLD,
     JOB_GET_MOTOR_RUNNING_TIME,
     JOB_GET_MOISTURE_THRESHOLD_LIMIT,
};

uint8_t isMotorRunning = MOTOR_STOP;

void setup()
{
#ifdef DEBUG
   Serial.begin(115200);
   Serial.println();
   Serial.println();
   Serial.println();
   Serial.println();
#endif

   WifiMulti.addAP(ssid, ssidPasswd);

   if (WifiMulti.run() != WL_CONNECTED)
     {
#ifdef DEBUG
        Serial.println("connecting....");
#endif
        delay(1000);
     }

#ifdef DEBUG
   Serial.println("Connected to wifi router");
#endif

   pinMode(portMotor, OUTPUT);
}

static void _stop_motor_cb()
{
#ifdef DEBUG
   Serial.println("Stopping the motor");
#endif
   //stops the motor
   digitalWrite(portMotor, 0);
   isMotorRunning = MOTOR_PRE_STOP; // this is when motor is about to stop
}

static String wateringJob(int taskId, uint16_t adcValue = 0)
{
   String ret = "";
   if (WifiMulti.run() == WL_CONNECTED)
     {
        HTTPClient http;
        char buf[30];

        if (taskId == JOB_SEND_EMAIL_START)
          {
             sprintf(buf, "/wateringJob?task=%d&adcvalue=%d", taskId, adcValue);
          }
        else
          sprintf(buf, "/wateringJob?task=%d", taskId);

        http.begin(host, port, buf);

        delay(1000);
        int httpCode = http.GET();
        delay(12000);
        if (httpCode > 0)
          {
             if (httpCode == HTTP_CODE_OK)
               {
                  ret = http.getString();
#ifdef DEBUG
                  Serial.println(ret);
#endif
               }
          }
#ifdef DEBUG
        else
          Serial.println("[HTTP]: Failed to connect");
#endif
        http.end();
     }
     return ret;
}

static uint16_t getMoistureThreshold(int8_t taskId)
{
   String val = wateringJob(taskId);

#ifdef DEBUG
   Serial.println("threshold value is: ");
   Serial.println(val);
#endif
   if (val != "")
   {
      return uint16_t(val.toInt());
   }
   else
      return taskId == 3 ? MOISTURE_THRESHOLD : MOISTURE_THRESHOLD_LIMIT;
}

static uint16_t getMotorRunningTime()
{
   String val = wateringJob(JOB_GET_MOTOR_RUNNING_TIME);

#ifdef DEBUG
   Serial.println("motorRunningtime:");
   Serial.println(val);
#endif

   if (val != "")
   {
      return uint16_t (val.toInt());
   }
   else
      return MOTOR_RUNNING_TIME;
}

void loop()
{
   if (isMotorRunning == MOTOR_STOP)
     {
        uint16_t adcValue = analogRead(A0);
        delay(100);
#ifdef DEBUG
        Serial.println("Adc Reading");
        Serial.print(adcValue);
#endif

        if (adcValue > getMoistureThreshold(JOB_GET_MOISTURE_THRESHOLD)
            && adcValue < getMoistureThreshold(JOB_GET_MOISTURE_THRESHOLD_LIMIT))
          {
             isMotorRunning = MOTOR_RUNNING;
             digitalWrite(portMotor, 1);

             //running motor for 2 mins.
             tick.once(getMotorRunningTime(), _stop_motor_cb);

             //send email
             wateringJob(JOB_SEND_EMAIL_START, adcValue);
          }
        else
          {
             //sleep for few mins.. TODO: check how to do deep sleep in esp
             // Deep sleep, make sure GPIO_16 is connected to RST
             // ESP.deepSleep(60 * 100000); // sleep for 60 seconds
          }
     }
   else if (isMotorRunning == MOTOR_PRE_STOP) //motor is about to stop.
     {
        wateringJob(JOB_SEND_EMAIL_FINISH);
        isMotorRunning = MOTOR_STOP; //finally motor is stopped
     }

   delay(10000);
}
