# * Automated Watering System
# * Copyright (C) 2016 Amitesh Singh <singh.amitesh@gmail.com>
# *
# * This library is free software; you can redistribute it and/or
# * modify it under the terms of the GNU Lesser General Public
# * License as published by the Free Software Foundation; either
# * version 2.1 of the License, or (at your option) any later version.
# *
# * This library is distributed in the hope that it will be useful,
# * but WITHOUT ANY WARRANTY; without even the implied warranty of
# * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
# * Lesser General Public License for more details.
# *
# * You should have received a copy of the GNU Lesser General Public
# * License along with this library;
# * if not, see <http://www.gnu.org/licenses/>.
# *

import cherrypy

import urllib
import time

import smtplib
from email.MIMEMultipart import MIMEMultipart
from email.MIMEText import MIMEText
from email.MIMEBase import MIMEBase
from email import encoders

class Watering(object):
    wateringCount = 0

    def __init__(self):
        self.timelog = ""
        self.motorRunningTime = "60"
        self.moistureThreshold = "500"
        self.moistureLimit = "900"
        self.adcValue = ""

    @cherrypy.expose
    def index(self):
        return """
        <html>
        <head>
        <style>
          h1 {
               color: grey;
          }
          .button {
                display: inline-block;
                border-radius: 4px;
                background-color: #f4511e;
                border: none;
                color: #FFFFFF;
                text-align: center;
                font-size: 28px;
                padding: 20px;
                width: 200px;
                transition: all 0.5s;
                cursor: pointer;
                margin: 5px;
            }

            .button span {
                cursor: pointer;
                display: inline-block;
                position: relative;
                transition: 0.5s;
            }

            .button span:after {
               content: ">";
               position: absolute;
               opacity: 0;
               top: 0;
               right: -20px;
               transition: 0.5s;
            }

            .button:hover span {
               padding-right: 25px;
            }

            .button:hover span:after {
               opacity: 1;
               right: 0;
            }

        </style>
        </head>
        <body>
        <h1> Watering system - (C) 2016 Amitesh Singh
        </h1>
        <p>
        This watering system does the watering of 15 plants in my balcony.
        </p>
        <h3>
        TODOs
        </h3>
        <p>
            1. add photos here. <br>
            2. mention this url in email body. <br>
            3. make the rpi accessible publicly. <br>
            4. <strike> mention when the last job is finished in showStatus() </strike> <br>
            5. Add email button which sends the last job history. <br>
            6. Store all jobs in DB. learn about it. <br>
        </p>

        <form method="GET" action="showStatus">
        <button type="submit" class="button" style="vertical-align:middle"><span>Check Status!</span> </button>
        </form>

        <form method = "GET" action="sendStatusEmail">
        <button type = "submit" class="button" style="vertical-align:middle"><span>Send Report</span> </button>

        </form>
        </body>
        </html>
        """

    @cherrypy.expose
    def sendStatusEmail(self):
        return "yet to implement.. TODOs"
    @cherrypy.expose
    def showStatus(self):
       return """
       <html>
       <head>
        <style>
        p {
          color: red;
        }
        </style>
        </head>
       <body>
       <p>
       Watering: %d <br>
       Last job finished at: <b>%s</b> <br>
       Motor running time: <b>%s seconds.</b> <br>
       Moisture threshold: <b>%s - %s </b>(on the scale of 0 to 1023, 1024 - when sensor is in AIR,
       0 - when sensor is connected via copper wire.) <br>
       </p>
       </body>
       </html>
       """ % (Watering.wateringCount, self.timelog, self.motorRunningTime, self.moistureThreshold, self.moistureLimit)

    def sendEmail(self, task):
        timeformat = "%H:%M:%S"
        dateformat = "%d/%m/%Y"

        self.timelog = time.strftime(dateformat) + " " + time.strftime(timeformat)

        fromaddr = "singh.amitesh@gmail.com"
        toaddr = ["singh.amitesh@gmail.com", "pooja.ieccs@gmail.com"]

        msg = MIMEMultipart()
        msg['From'] = fromaddr
        msg['To'] = ", ".join(toaddr)
        if task == "1":
            msg['Subject'] = "Watering Log: Watering Job has started - " + self.timelog
        else:
            msg['Subject'] = "Watering Log: Watering Job has ended successfully - " + self.timelog

        body = "Time :" + self.timelog + "\n Last ADC value: " + self.adcValue

        msg.attach(MIMEText(body, 'plain'))

        #filename = "photo.jpg"
        #attachment = open("/tmp/photo.jpg", "rb")

        #part = MIMEBase('application', 'octet-stream')
        #part.set_payload((attachment).read())
        #encoders.encode_base64(part)
        #part.add_header('Content-Disposition', "attachment; filename= %s" % filename)

        #msg.attach(part)

        server = smtplib.SMTP('smtp.gmail.com', 587)
        server.starttls()
        server.login(fromaddr, "password")
        text = msg.as_string()
        server.sendmail(fromaddr, toaddr, text)
        server.quit()

    # task = 1 -- waterting started
    # task 2 - watering stopped
    @cherrypy.expose
    def wateringJob(self, task="1", adcvalue="None"):
        if task == "1": # watering job has started.
            self.adcValue = adcvalue
            print "watering has started"
        elif task == "2": # watering job done.
            Watering.wateringCount += 1
            print "watering is done and stoppped"
        elif task == "3": # moisture  threshold.
            return self.moistureThreshold # TODO: add this value into DB, should be way to set this value
        elif task == "4": # motor running time
            return self.motorRunningTime
        elif task == "5": # moisture Limit
            return self.moistureLimit

        self.sendEmail(task)
        return task

if __name__ == "__main__":
    # This will run cherrypy on public interface. by default its bind to localhost:8080
    cherrypy.config.update({'server.socket_host': '0.0.0.0'})
    cherrypy.quickstart(Watering())
