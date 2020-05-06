//Version 0.03b

//Release notes 0.03 - 5 emergency circuts, auto on and off, manual on and off by web interface, states of the circuts,  clock, set up clock.

//Release notes 0.02 - 3 emergency circuts, auto on, manual on and off by Web interface, states of the circuts, clock, set up clock.

// 1. Circut test and switching on emergency light circut I, II, III
// 2. WEB interface to switch on emergency light
// 3. WEB interface to check the rest of the lighting circut
// 4. WEB interface to switch on and off the rest of the lighting circut
// 5. Real Time Clock
// 6. Auto switch
// 6. Log state into SD card
// 7. Display history

#include <SPI.h>
#include <Ethernet.h>
#include <TimeLord.h>
#include <OneWire.h>
#include <DS18B20.h>
#include <EEPROM.h>
#include <SD.h>
#include <C:\Users\gmroczkowski\Documents\Arduino\libraries\Timer\Timer.h>
#include <C:\Users\gmroczkowski\Documents\Arduino\libraries\Timer\Timer.cpp>
#include <C:\Users\gmroczkowski\Documents\Arduino\libraries\SRTC\RTC.h>
#include <C:\Users\gmroczkowski\Documents\Arduino\libraries\SRTC\RTC.cpp>

//Lighting circut definitióon
#define klatka_schodowa_sypialnia_pracownia_II 23 //F9,  1
#define kinkiety_Jagoda_Mikolaj_II 22             //F10, 2
#define salon_lazienka_dol_I 25                   //F11, 3
#define lazienka_gora_Jagoda_II 24                //F12, 4
#define garaz_wiatrolap_I 27                      //F13, 5
#define strych_II 26                              //F22, 6
#define spizarnia_kominek_III 28                  //F25, 7

//Emergency circut definition
#define strych 33
#define garaz 34
#define parter 35
#define pietro 36
#define spizarnia 37

//Initialize timer and RTC
Timer odliczanie;
RTC zegar;

//Auto switch to turn off emergency light if needed.
byte Auto = 1;

//Variables states for emergency lighting
byte garageOn = 0;
byte atticOn = 0;
byte PantryOn = 0;
byte UpstairsOn = 0;
byte DownstairsOn = 0;

//Variables states for lighting circut
byte F9_kssp = 0;
byte F10_kjm = 0;
byte F11_sld = 0;
byte F12_lgj = 0;
byte F13_gw = 0;
byte F22_s = 0;
byte F25_sk = 0;

//For the WEB server
String header;

// Current time
unsigned long currentTime = millis();
// Previous time
unsigned long previousTime = 0;
// Define timeout time in milliseconds (example: 2000ms = 2s)
const long timeoutTime = 2000;

//----------------From Ethernet server example:
// Enter a MAC address and IP address for your controller below.
// The IP address will be dependent on your local network:
byte mac[] = {
    0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xEA};
IPAddress ip(172, 26, 160, 19);

// Initialize the Ethernet server library
// with the IP address and port you want to use
// (port 80 is default for HTTP):
EthernetServer server(80);

void setup()
{

    // Define I/O
    pinMode(klatka_schodowa_sypialnia_pracownia_II, INPUT);
    pinMode(kinkiety_Jagoda_Mikolaj_II, INPUT);
    pinMode(salon_lazienka_dol_I, INPUT);
    pinMode(lazienka_gora_Jagoda_II, INPUT);
    pinMode(garaz_wiatrolap_I, INPUT);
    pinMode(strych_II, INPUT);
    pinMode(spizarnia_kominek_III, INPUT);

    pinMode(garaz, OUTPUT);
    pinMode(strych, OUTPUT);
    pinMode(parter, OUTPUT);
    pinMode(pietro, OUTPUT);
    pinMode(spizarnia, OUTPUT);

    // Set outputs to HIGH
    digitalWrite(garaz, HIGH);
    digitalWrite(strych, HIGH);
    digitalWrite(parter, HIGH);
    digitalWrite(pietro, HIGH);
    digitalWrite(spizarnia, HIGH);

    Serial.begin(19200); // open the serial port at 19200 bps:

    //start the Ethernet connection and the server :
    Ethernet.begin(mac, ip);
    //Check for Ethernet hardware present
    if (Ethernet.hardwareStatus() == EthernetNoHardware)
    {
        Serial.println("Ethernet shield was not found.  Sorry, can't run without hardware. :(");
        while (true)
        {
            delay(1); // do nothing, no point running without Ethernet hardware
        }
    }
    if (Ethernet.linkStatus() == LinkOFF)
    {
        Serial.println("Ethernet cable is not connected.");
    }

    // start the server
    server.begin();
    Serial.print("server is at ");
    Serial.println(Ethernet.localIP());
}

void loop()
{
    zegar.Flow();
    // listen for incoming clients
    //----------------From ethernet example:
    EthernetClient client = server.available();
    //----------------From RNT:
    if (client)
    {                                  // If a new client connects,
        Serial.println("New Client."); // print a message out in the serial port
        String currentLine = "";       // make a String to hold incoming data from the client
        currentTime = millis();
        previousTime = currentTime;
        while (client.connected() && currentTime - previousTime <= timeoutTime)
        { // loop while the client's connected
            currentTime = millis();
            if (client.available())
            {                           // if there's bytes to read from the client,
                char c = client.read(); // read a byte, then
                Serial.write(c);        // print it out the serial monitor
                header += c;
                if (c == '\n')
                { // if the byte is a newline character
                    // if the current line is blank, you got two newline characters in a row.
                    // that's the end of the client HTTP request, so send a response:
                    if (currentLine.length() == 0)
                    {
                        // HTTP headers always start with a response code (e.g. HTTP/1.1 200 OK)
                        // and a content-type so the client knows what's coming, then a blank line:
                        client.println(F("HTTP/1.1 200 OK"));
                        client.println(F("Content-type:text/html"));
                        client.println(F("Connection: close"));
                        //client.println("Refresh: 5");  // refresh the page automatically every 5 sec
                        client.println();

                        // Display the HTML web page
                        client.println(F("<!DOCTYPE html><html>"));
                        client.println(F("<head><meta name=\"viewport\" content=\"width=device-width, initial-scale=1\">"));
                        client.println(F("<link rel=\"icon\" href=\"data:,\">"));
                        // CSS to style the on/off buttons
                        // Feel free to change the background-color and font-size attributes to fit your preferences
                        client.println(F("<style>html { font-family: Helvetica; display: inline-block; margin: 0px auto; text-align: center;}"));
                        client.println(F(".button { background-color: #195B6A; border: none; color: white; padding: 16px 40px;"));
                        client.println(F("text-decoration: none; font-size: 30px; margin: 2px; cursor: pointer;}"));
                        client.println(F(".button2 {background-color: #77878A;}</style></head>"));
                        // Web Page Heading
                        client.println(F("<body><H1>Oswietlenie awaryjne Ozarow ver. 0.02b</H1>"));
                        client.println(F("<H3>Status:</H3>"));
                        client.println(F("<h5> Dzis: "));
                        client.println(zegar.getDate());
                        client.println(F(" "));
                        client.println(zegar.getWeekDay());
                        client.println(F(" godz.: "));
                        client.println(zegar.getTime());
                        client.println(F(" uptime: "));
                        client.println(odliczanie.upTime());
                        client.println(F("</h5>"));

                        //Auto switch off
                        if (header.indexOf("GET /0/off") >= 0)
                        {
                            Auto = 0;
                            Serial.println("Parter switch to off");
                        }

                        //Auto switch on
                        if (header.indexOf("GET /0/on") >= 0)
                        {
                            Auto = 1;
                            Serial.println("Parter switch to off");
                        }

                        //Downstairs off
                        if (header.indexOf("GET /1/off") >= 0)
                        {
                            DownstairsOn = 0;
                            Serial.println("Parter switch to off");
                        }

                        //Downstairs on
                        if (header.indexOf("GET /1/on") >= 0)
                        {
                            DownstairsOn = 1;
                            Serial.println("Parter switch to on");
                        }

                        //Upstairs off
                        if (header.indexOf("GET /2/off") >= 0)
                        {
                            UpstairsOn = 0;
                            Serial.println("Pietro switch to off");
                        }

                        //Upstairs on
                        if (header.indexOf("GET /2/on") >= 0)
                        {
                            UpstairsOn = 1;
                            Serial.println("Pietro switch to on");
                        }

                        //Pantry off
                        if (header.indexOf("GET /3/off") >= 0)
                        {
                            PantryOn = 0;
                            Serial.println("Spizarnia switch to off");
                        }

                        //Pantry on
                        if (header.indexOf("GET /3/on") >= 0)
                        {
                            PantryOn = 1;
                            Serial.println("Spizarnia switch to on");
                        }

                        //Garage off
                        if (header.indexOf("GET /4/off") >= 0)
                        {
                            garageOn = 0;
                            Serial.println("Garaz switch to off");
                        }

                        //Garage on
                        if (header.indexOf("GET /4/on") >= 0)
                        {
                            garageOn = 1;
                            Serial.println("Garaz switch to on");
                        }

                        //Attic off
                        if (header.indexOf("GET /5/off") >= 0)
                        {
                            atticOn = 0;
                            Serial.println("Strych switch to off");
                        }

                        //Attic on
                        if (header.indexOf("GET /5/on") >= 0)
                        {
                            atticOn = 1;
                            Serial.println("Strych switch to on");
                        }

                        //Setup clock
                        if (header.indexOf("GET /zegar/") >= 0)
                        { //Setup clock, clock format: \  

                            zegar.setYear((int)(header.substring(11, 15).toInt()));
                            zegar.setMonth((byte)(header.substring(16, 18).toInt()));
                            zegar.setMonthDay((byte)(header.substring(19, 21).toInt()));
                            zegar.setHour((byte)(header.substring(22, 24).toInt()));
                            zegar.setMinute((byte)(header.substring(25, 27).toInt()));
                            zegar.setSecond(55);
                            zegar.setWeekDay((byte)1);
                            Serial.println("Set up clock");
                        };

                        if (Auto == 1) //Turn on auto
                        {
                            client.println(F("<p><a href=\"/0/off\"><button class=\"button\">Auto</button></a>"));
                            Serial.println("Auto");
                        }
                        if (Auto == 0) //Turn off auto
                        {
                            client.println(F("<p><a href=\"/0/on\"><button class=\"button button2\">Manual</button></a>"));
                            Serial.println("Manual");
                        };
                        client.println(F("</p>"));

                        if (DownstairsOn == 1) //Turn on Downstairs circut
                        {
                            client.println(F("<p><a href=\"/1/off\"><button class=\"button\">Parter On</button></a>"));
                            Serial.println("Parter is on");
                        }
                        if (DownstairsOn == 0) //Turn off Downstairs circut
                        {
                            client.println(F("<p><a href=\"/1/on\"><button class=\"button button2\">Parter Off</button></a>"));
                            Serial.println("Parter is off");
                        };
                        client.println(F("</p>"));

                        if (UpstairsOn == 1) //Turn on Upstairs emergency circut
                        {
                            client.println(F("<p><a href=\"/2/off\"><button class=\"button\">Pietro On</button></a>"));
                            Serial.println("Pietro is on");
                        }
                        if (UpstairsOn == 0) //Turn off Upstairs emergency circut
                        {
                            client.println(F("<p><a href=\"/2/on\"><button class=\"button button2\">Pietro Off</button></a>"));
                            Serial.println("Parter is off");
                        };
                        client.println(F("</p>"));

                        if (PantryOn == 1) //Turn on Pantry Emergency circut
                        {
                            client.println(F("<p><a href=\"/3/off\"><button class=\"button\">Spizarnia On</button></a>"));
                            Serial.println("Spizarnia is on");
                        }
                        if (PantryOn == 0) //Turn off Pantry emergency circut
                        {
                            client.println(F("<p><a href=\"/3/on\"><button class=\"button button2\">Spizarnia Off</button></a>"));
                            Serial.println("Spizarnia is off");
                        };
                        client.println(F("</p>"));

                        if (garageOn == 1) //Turn on Garage emergency circut
                        {
                            client.println(F("<p><a href=\"/4/off\"><button class=\"button\">Garaz On</button></a>"));
                            Serial.println("Garaz is on");
                        }
                        if (garageOn == 0) //Turn off Garage emergency circut
                        {
                            client.println(F("<p><a href=\"/4/on\"><button class=\"button button2\">Garaz Off</button></a>"));
                            Serial.println("Garaz is off");
                        };
                        client.println(F("</p>"));

                        if (atticOn == 1) //Turn on Attic emergency circut
                        {
                            client.println(F("<p><a href=\"/5/off\"><button class=\"button\">Strych On</button></a>"));
                            Serial.println("Strych is on");
                        }
                        if (atticOn == 0) //Turn off Attic emergency circut
                        {
                            client.println(F("<p><a href=\"/5/on\"><button class=\"button button2\">Strych Off</button></a>"));
                            Serial.println("Strych is off");
                        };
                        client.println(F("</p>"));

                        client.println(F("<a href=\"/nothing\"><button class=\"button button2\">Refresh </button></a></p>"));

                        client.println(F("<H3>Stan obwodow:</H3>"));
                        client.println(F("<table><tr><th>Obwod</th><th>Stan</th></tr>"));

                        client.println(F("<tr><td>F9 Klatka schodowa, sypialnia, pracownia</td><td>"));
                        client.println(F9_kssp);
                        client.println(F("</td></tr>"));

                        client.println(F("<tr><td>F10 Kinkiety Jagoda, Mikolaj</td><td>"));
                        client.println(F10_kjm);
                        client.println(F("</td></tr>"));

                        client.println(F("<tr><td>F11 Salon, lazienka parter</td><td>"));
                        client.println(F11_sld);
                        client.println(F("</td></tr>"));

                        client.println(F("<tr><td>F12 Lazienka pietro, Jagoda</td><td>"));
                        client.println(F12_lgj);
                        client.println(F("</td></tr>"));

                        client.println(F("<tr><td>F13 Garaz, wiatrolap</td><td>"));
                        client.println(F13_gw);
                        client.println(F("</td></tr>"));

                        client.println(F("<tr><td>F22 Strych</td><td>"));
                        client.println(F22_s);
                        client.println(F("</td></tr>"));

                        client.println(F("<tr><td>F25 Spizarnia, kominek</td><td>"));
                        client.println(F25_sk);
                        client.println(F("</td></tr>"));

                        client.println(F("</table>"));

                        //-------------Koniec
                        client.println();
                        client.println(F("</body></html>")); //Nie wiem czemu, ale musialem zakomentowac, bo nie wyswietlal godziny :( - WCALE NIE! JUŻ DZIAŁA! :)
                        // The HTTP response ends with another blank line
                        client.println();
                        // Break out of the while loop
                        //Downstairs off

                        break;
                    }
                    else
                    { // if you got a newline, then clear currentLine
                        currentLine = "";
                    }
                }
                else if (c != '\r')
                {                     // if you got anything else but a carriage return character,
                    currentLine += c; // add it to the end of the currentLine
                }
            }
        }
        // Clear the header variable
        header = "";
        // Close the connection
        client.stop();
        //Serial.println("Client disconnected.");
        //Serial.println("");
    }

    //Check lighting circut state
    F9_kssp = !digitalRead(klatka_schodowa_sypialnia_pracownia_II);
    F10_kjm = !digitalRead(kinkiety_Jagoda_Mikolaj_II);
    F11_sld = !digitalRead(salon_lazienka_dol_I);
    F12_lgj = !digitalRead(lazienka_gora_Jagoda_II);
    F13_gw = !digitalRead(garaz_wiatrolap_I);
    F22_s = !digitalRead(strych_II);
    F25_sk = !digitalRead(spizarnia_kominek_III);

    //Check, if emergency lights is set to on
    if (Auto == 1)
    {
        //Turn off emergency circut if there are phase signal at the circut for upstairs
        if (F9_kssp == 1)
            UpstairsOn = 0;
        if (F10_kjm == 1)
            UpstairsOn = 0;
        if (F12_lgj == 1)
            UpstairsOn = 0;

        //Turn off emergency circut if there are phase signal at the circut for downstairs
        if (F11_sld == 1)
            DownstairsOn = 0;

        //Turn off emergency circut if there are phase signal at the circut for pantry
        if (F25_sk == 1)
            PantryOn = 0;

        //Turn off emergency circut if there are phase signal at the circut Garage
        if (F13_gw == 1)
            garageOn = 0;

        //Turn off emergency circut if there are phase signal at the circut for Attic
        if (F22_s == 1)
            atticOn = 0;

        //Turn on emergency circut if there are no phase signal at the circut for upstairs
        if (F9_kssp == 0)
            UpstairsOn = 1;
        if (F10_kjm == 0)
            UpstairsOn = 1;
        if (F12_lgj == 0)
            UpstairsOn = 1;

        //Turn on emergency circut if there are no phase signal at the circut for downstairs
        if (F11_sld == 0)
            DownstairsOn = 1;

        //Turn on emergency circut if there are no phase signal at the circut for pantry
        if (F25_sk == 0)
            PantryOn = 1;

        //Turn on emergency circut if there are no phase signal at the circut Garage
        if (F11_sld == 0)
            garageOn = 1;

        //Turn off emergency circut if there are no phase signal at the circut for Attic
        if (F22_s == 0)
            atticOn = 1;
    }

    //Check and execute Upstairs
    if (UpstairsOn == 1)
    {
        digitalWrite(pietro, LOW);
        //Serial.println("Pietro set to on");
    }
    if (UpstairsOn == 0)
    {
        digitalWrite(pietro, HIGH);
        //Serial.println("Pietro set to off");
    }
    //Check and execute Downstairs
    if (DownstairsOn == 1)
    {
        digitalWrite(parter, LOW);
        //Serial.println("Parter set to on");
    }
    if (DownstairsOn == 0)
    {
        digitalWrite(parter, HIGH);
        //Serial.println("Parter set to off");
    }
    //Check and execute Pantry
    if (PantryOn == 1)
    {
        digitalWrite(spizarnia, LOW);
        //Serial.println("Spizarnia set to on");
    }
    if (PantryOn == 0)
    {
        digitalWrite(spizarnia, HIGH);
        //Serial.println("Spizarnia set to off");
    }
    //Check and execute Garage
    if (garageOn == 1)
    {
        digitalWrite(garaz, LOW);
        //Serial.println("Garaz set to on");
    }
    if (garageOn == 0)
    {
        digitalWrite(garaz, HIGH);
        //Serial.println("Garaz set to off");
    }
    //Check and execute Attic
    if (atticOn == 1)
    {
        digitalWrite(strych, LOW);
        //Serial.println("Strych set to on");
    }
    if (atticOn == 0)
    {
        digitalWrite(strych, HIGH);
        //Serial.println("Strych set to off");
    }


}
