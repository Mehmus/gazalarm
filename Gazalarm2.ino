#include <SoftwareSerial.h>
#define LEDCARBON 2
#define LEDNATURAL 7
#define BUTTON 4
#define CARBON A0
#define NATURALGAS A4
#define BUZZER 9

SoftwareSerial esp(10, 11); // RX, TX

// WiFi Ayarları
const char* ssid = "HUAWEI P20 lite";
const char* password = "hepsikucuk";
const char* host = "maker.ifttt.com";
const int httpPort = 80;

// Değişkenler
unsigned long lastDebounceTime = 0;
const unsigned long debounceDelay = 50; // Debounce süresi (ms)
int carbonmax = 500;
int naturalgasmax = 300;

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

  //initializeESP(); //BURAYI AÇ ------------------------------------------------------------------------------------------------------------
  



  Serial.println("Gaz Alarm Sistemi Başlatıldı");
}

void loop() {
  // Buton durum kontrolü
  handleButton();

  // Sistem açıksa gaz seviyelerini kontrol et
  if (systemState) {
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

    //Maşl Gönderme
    //lastDebounceTime = millis();
    //Serial.println("İstek gönderiliyor...");
    //sendEmailRequest();


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
      digitalWrite(LEDCARBON, carbonAlarm ? HIGH : LOW);
      digitalWrite(LEDNATURAL, naturalGasAlarm ? HIGH : LOW);
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
  sendCommand("AT", "OK", 2000);
  sendCommand("AT+CWMODE=1", "OK", 2000);

  Serial.println("Ağa bağlanıyor...");
  String connectCmd = "AT+CWJAP=\"" + String(ssid) + "\",\"" + String(password) + "\"";
  if (sendCommand(connectCmd, "OK", 10000)) {
    Serial.println("Ağa bağlandı");
  } else {
    Serial.println("Ağa bağlanamadı!");
  }
}

void sendEmailRequest() {
  // TCP bağlantısı kur
  String tcpCommand = "AT+CIPSTART=\"TCP\",\"" + String(host) + "\",80";
  if (!sendCommand(tcpCommand, "OK", 5000)) {
    Serial.println("IFTTT sunucusuna bağlanılamadı");
    return;
  }

  // GET isteğini hazırla
  String url = "/trigger/sendmail/with/key/mBdp6CjBwBnYhG3-fQftWACC3DqZDxmdhqQe_EAxiu9";
  String request = "GET " + url + " HTTP/1.1\r\n"
                   "Host: " + String(host) + "\r\n"
                   "Connection: close\r\n\r\n";

  // Veri gönder
  String sendCmd = "AT+CIPSEND=" + String(request.length());
  if (!sendCommand(sendCmd, ">", 3000)) {
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
  return waitForResponse(expected, timeout);
}
