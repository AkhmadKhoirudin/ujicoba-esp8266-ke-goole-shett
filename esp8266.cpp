#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <MFRC522.h>

// Konfigurasi pin RFID dan buzzer
#define SS_PIN D8
#define RST_PIN D0
#define BUZZER_PIN D1

// Konfigurasi jaringan Wi-Fi
const char* ssid = "FAOZAN";
const char* password = "@Bhakti135";

// URL API Google Sheets
const char* serverURL = "https://script.google.com/macros/s/AKfycbzm9f13G7LkSwo8yUvMnh1mym0ZlMEJPaMgePDwAO8NWxGgRE2o3IYsugAhsT7hqao8ug/exec";

// Inisialisasi RFID reader
MFRC522 mfrc522(SS_PIN, RST_PIN);

// Fungsi untuk membunyikan buzzer
void buzz(int times) {
    for (int i = 0; i < times; i++) {
        digitalWrite(BUZZER_PIN, HIGH);
        delay(200);
        digitalWrite(BUZZER_PIN, LOW);
        delay(200);
    }
}

void setup() {
    // Inisialisasi komunikasi serial
    Serial.begin(115200);
    
    // Inisialisasi koneksi Wi-Fi
    WiFi.begin(ssid, password);
    Serial.println("Menghubungkan ke Wi-Fi...");
    
    // Tunggu sampai terhubung ke Wi-Fi
    while (WiFi.status() != WL_CONNECTED) {
        delay(1000);
        Serial.print(".");
    }
    Serial.println("\nTerhubung ke Wi-Fi!");
    
    // Inisialisasi modul RFID
    SPI.begin();
    mfrc522.PCD_Init();
    
    // Inisialisasi pin buzzer
    pinMode(BUZZER_PIN, OUTPUT);
}

void loop() {
    // Periksa apakah ada kartu RFID yang tersedia
    if (!mfrc522.PICC_IsNewCardPresent()) {
        return;
    }
    
    // Periksa apakah kartu dapat dibaca
    if (!mfrc522.PICC_ReadCardSerial()) {
        return;
    }
    
    // Baca UID kartu RFID
    String rfidUID = "";
    for (byte i = 0; i < mfrc522.uid.size; i++) {
        rfidUID += String(mfrc522.uid.uidByte[i], HEX);
    }
    
    // Cetak UID kartu ke Serial Monitor
    Serial.println("UID Kartu RFID: " + rfidUID);
    
    // Membuat objek WiFiClient
    WiFiClient client;
    
    // Membuat permintaan HTTP ke Google Sheets
    HTTPClient http;
    
    // Membuat URL lengkap dengan UID kartu sebagai parameter "name"
    String urlWithParams = String(serverURL) + "?name=" + rfidUID;
    
    // Inisialisasi permintaan HTTP dengan URL lengkap
    http.begin(client, urlWithParams);
    
    // Kirim permintaan GET
    int httpResponseCode = http.GET();
    
    if (httpResponseCode == HTTP_CODE_OK) {
        // Respons berhasil, cetak ke Serial Monitor
        String response = http.getString();
        Serial.println("Respons API: " + response);
        
        // Bunyikan buzzer sekali
        buzz(1);
    } else {
        // Jika terjadi kesalahan, cetak pesan error dan bunyikan buzzer tiga kali
        Serial.println("Error dalam koneksi API: " + String(httpResponseCode));
        Serial.println("Pesan error HTTP: " + http.errorToString(httpResponseCode));
        buzz(3);
    }
    
    // Tutup koneksi HTTP
    http.end();
    
    // Tunggu beberapa detik sebelum membaca kartu berikutnya
    delay(2000);
}
