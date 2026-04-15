#include <DHT.h> // Isı nem
#include <DHT_U.h> //Isı nem 2

#include <SoftwareSerial.h>


#define DHTPIN 8
DHT dht(DHTPIN, DHT11);



#define LEDCARBON 2
#define LEDNATURAL 7
#define BUTTON 4
#define CARBON A0
#define NATURALGAS A4
#define BUZZER 9

SoftwareSerial esp(10, 11); // RX, TX

// WiFi Ayarları
const char* ssid = "Birikiüçdörtbeşaltı";
const char* password = "Bmbol446";
const char* host = "maker.ifttt.com";
const int httpPort = 80;

// Değişkenler
unsigned long lastDebounceTime = 0;
const unsigned long debounceDelay = 50; // Debounce süresi (ms)

bool mailgonderildimi = false;
bool isimail = false;
bool nemmail = false;

int carbonmax = 450;
int naturalgasmax = 250;

bool systemState = false;  // Sistem açık/kapalı durumu
bool buttonPressed = false;
unsigned long buttonPressTime = 0;




void setup() {
  Serial.begin(9600);

  pinMode(LEDCARBON, OUTPUT);
  pinMode(LEDNATURAL, OUTPUT);
  pinMode(BUTTON, INPUT_PULLUP);
  pinMode(BUZZER, OUTPUT);
  pinMode(CARBON, INPUT);
  pinMode(NATURALGAS, INPUT);


  esp.begin(115200);

  // Buton Girişi

  //ISI NEM BAŞLANGIÇ
  dht.begin(); //Iıs nem başlangıç

  Serial.begin(9600);
  esp.begin(115200);
  /*esp.println("AT");
    Serial.println("AT YOLLANDI");
    while (!esp.find("OK")) {
    esp.println("AT");
    Serial.println("AT YOLLANDI");
    }
    Serial.println("OK ALINDI");

    esp.println("AT+CWMODE=1");
    Serial.println("ATCWMODE YOLLANDI");
    while (!esp.find("OK")) {
    Serial.println("ATCWMODE YOLLANDI");
    esp.println("AT+CWMODE=1");
    }
    Serial.println("OK ALINDI");
    Serial.println("Ağa bağlanıyor...");
    String connectCmd = "AT+CWJAP=\"" + String(ssid) + "\",\"" + String(password) + "\"";
    if (sendCommand(connectCmd, "OK", 10000)) {
    Serial.println("Ağa bağlandı");
    tone(BUZZER, 800);
    delay(200);
    noTone(BUZZER);

    } else {
    Serial.println("Ağa bağlanamadı!");
    for (int i; i < 4; i++) {
      tone(BUZZER, 800);
      delay(400);
      noTone(BUZZER);
      delay(400);
    }
    }*/

  initializeESP(); //BURAYI AÇ ------------------------------------------------------------------------------------------------------------ 





  Serial.println("Gaz Alarm Sistemi Başlatıldı");
}

unsigned long timerefresh = 0;

void loop() {

  //if(millis()-timerefresh >= 300000){
  //mailgonderildimi=false;
  //isimail=false;
  //nemmail=false;
  //timerefresh=millis();

  //}








  // Buton durum kontrolü
  handleButton();

  // Sistem açıksa gaz seviyelerini kontrol et
  if (systemState) {
    isinem();

    checkGasLevels();
  }
}

void handleButton() {
  // Buton basıldığında
  if (digitalRead(BUTTON) == LOW && !buttonPressed) {
    buttonPressed = true;
    buttonPressTime = millis();
  }

  // Buton bırakıldığında
  if (digitalRead(BUTTON) == HIGH) {
    buttonPressed = false;


  }

  // 3 saniye basılı tutma kontrolü
  if (buttonPressed && (millis() - buttonPressTime >= 3000)) {
    systemState = !systemState; // Sistem durumunu tersine çevir
    Serial.print("BASILDI2");

    tone(BUZZER, 800);
    delay(500);
    noTone(BUZZER);




    // LED'leri güncelle
    digitalWrite(LEDCARBON, systemState ? HIGH : LOW);
    digitalWrite(LEDNATURAL, systemState ? HIGH : LOW);
    delay(500);
    digitalWrite(LEDCARBON, systemState ? HIGH : LOW);
    digitalWrite(LEDNATURAL, systemState ? HIGH : LOW);

    Serial.print("Sistem ");

  }
}

void checkGasLevels() {
  static unsigned long lastReadTime = 0;
  bool naturalGasAlarm = false;    // Değişkenler EN ÜSTTE tanımlandı
  bool carbonAlarm = false;        // Değişkenler EN ÜSTTE tanımlandı

  if (millis() - lastReadTime >= 500) {
    int carbonValue = analogRead(CARBON);
    int naturalGasValue = analogRead(NATURALGAS);

    Serial.print("Karbon: ");
    Serial.print(carbonValue);
    Serial.print(" | Doğalgaz: ");
    Serial.println(naturalGasValue);

    // Carbon alarm kontrolü
    if (carbonValue >= carbonmax) {
      carbonAlarm = true;
    } else {
      carbonAlarm = false;
    }

    // Doğalgaz alarm kontrolü (HATA DÜZELTİLDİ)
    if (naturalGasValue >= naturalgasmax) {
      naturalGasAlarm = true;
    } else {                      // Süslü parantez eklendi
      naturalGasAlarm = false;
    }

    // Alarm kontrolü (Bu blok, sensör okuma IF'inin İÇİNDE olmalı)
    if (carbonAlarm || naturalGasAlarm) {
      tone(BUZZER, 800);
      if (carbonAlarm) {
        digitalWrite(LEDCARBON, 1);
        //Mail Gönderme
        if (mailgonderildimi == true) {
          lastDebounceTime = millis();
          Serial.println("İstek gönderiliyor...");
          sendEmailRequest("/trigger/carbonalarm/with/key/S1p_vYCx1Yeqa6rmAjWZ0V_jcDXijacRoIjyS0bi3C");
          mailgonderildimi = false;
        }

      }
      else {
        digitalWrite(LEDCARBON, 0);
      }
      if (naturalGasAlarm) {
        digitalWrite(LEDNATURAL, 1);
        //Mail Gönderme

        if (mailgonderildimi == false) {
          lastDebounceTime = millis();
          Serial.println("İstek gönderiliyor...");
          sendEmailRequest("/trigger/gazmail/with/key/S1p_vYCx1Yeqa6rmAjWZ0V_jcDXijacRoIjyS0bi3C");
          mailgonderildimi = true;
        }

      }
      else {
        digitalWrite(LEDNATURAL, 0);
      }
    } else {
      noTone(BUZZER);
      digitalWrite(LEDNATURAL, LOW);
      digitalWrite(LEDCARBON, LOW);
    }

    lastReadTime = millis();
  } // IF bloğu burada kapatıldı
} // Fonksiyon burada kapatıldı





//---------------------------------------------------------------------------------------------------------------------------------------
void initializeESP() {

  /*esp.println("AT");
    Serial.println("AT YOLLANDI");
    while (!esp.find("OK")) {
    esp.println("AT");
    Serial.println("AT YOLLANDI");
    }
    esp.println("AT+CWMODE=1");
    Serial.println("AT YOLLANDI");
    while (!esp.find("OK")) {
    esp.println("AT+CWMODE=1");
    Serial.println("ATCWMODE YOLLANDI");
    }*/

  sendCommand("AT", "OK", 10000);
  sendCommand("AT+CWMODE=1", "OK", 10000);


  Serial.println("Ağa bağlanıyor...");
  String connectCmd = "AT+CWJAP=\"" + String(ssid) + "\",\"" + String(password) + "\"";
  if (sendCommand(connectCmd, "OK", 10000)) {
    Serial.println("Ağa bağlandı");
    tone(BUZZER, 800);
    delay(200);
    noTone(BUZZER);

  } else {
    Serial.println("Ağa bağlanamadı!");
    for (int i; i < 4; i++) {
      tone(BUZZER, 800);
      delay(400);
      noTone(BUZZER);
      delay(400);
    }
  }
}

void sendEmailRequest(String URL1) {
  // TCP bağlantısı kur
  String tcpCommand = "AT+CIPSTART=\"TCP\",\"" + String(host) + "\",80";
  esp.println(tcpCommand);
  while (!esp.find("OK")) {
    esp.println(tcpCommand);
    Serial.println("Ağa Bağlanıyor");





  }








  // GET isteğini hazırla
  String url = URL1;
  String request = "GET " + url + " HTTP/1.1\r\n"
                   "Host: " + String(host) + "\r\n"
                   "Connection: close\r\n\r\n";

  // Veri gönder
  String sendCmd = "AT+CIPSEND=" + String(request.length());
  if (!sendCommand(sendCmd, ">", 10000)) {
    Serial.println("Gönderim başlatılamadı");
    return;
  }

  esp.print(request);
  Serial.println("İstek gönderildi");

  // Yanıtı oku
  if (waitForResponse("200 OK", 5000)) {
    Serial.println("E-posta gönderimi başarılı!");
  } else {
    Serial.println("Sunucu yanıtı alınamadı");
  }

  // Bağlantıyı kapat
  sendCommand("AT+CIPCLOSE", "CLOSED", 2000);
  Serial.println("Bağlantı kapatıldı");
}

// Yardımcı Fonksiyonlar
bool waitForResponse(const String & target, int timeout) {
  unsigned long start = millis();
  String response = "";

  while (millis() - start < timeout) {
    while (esp.available()) {
      char c = esp.read();
      response += c;
      if (response.indexOf(target) != -1) {
        Serial.println("[ESP] " + response);
        return true;
      }
    }
  }
  Serial.println("[Timeout] " + response);
  return false;
}

bool sendCommand(const String & cmd, const String & expected, int timeout) {
  Serial.println("[Gönderilen] " + cmd);
  esp.println(cmd);
  esp.println(cmd);

  esp.println(cmd);

  esp.println(cmd);

  esp.println(cmd);

  esp.println(cmd);

  esp.println(cmd);

  esp.println(cmd);

  esp.println(cmd);
  esp.println(cmd);

  esp.println(cmd);

  return waitForResponse(expected, timeout);
}



unsigned long isinemtime = 0;
void isinem () {
  if (millis() - isinemtime >= 2000) {
    //----------------------------------------------------DHT11-------------------------------------------------------

    float hum = dht.readHumidity();
    float temp = dht.readTemperature();
    Serial.print("Isı:");
    Serial.print(temp);
    Serial.print("|  Nem:");
    Serial.println(hum);


    if (temp >= 45) {
      //if (isimail == false) {
      //lastDebounceTime = millis();
      //Serial.println("/trigger/tempmail/with/key/S1p_vYCx1Yeqa6rmAjWZ0V_jcDXijacRoIjyS0bi3C");
      //isimail = true;
      //}
    }

    if (hum >= 80) {
      //if (nemmail == false) {
      //lastDebounceTime = millis();
      //Serial.println("/trigger/hummail/with/key/S1p_vYCx1Yeqa6rmAjWZ0V_jcDXijacRoIjyS0bi3C");
      //nemmail = true;

      //}

    }










  }



  isinemtime = millis();


}
