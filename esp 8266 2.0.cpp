#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <WiFiClientSecure.h>
#include <MFRC522.h>

// Informasi jaringan Wi-Fi
const char* ssid = "FAOZAN"; // Nama jaringan Wi-Fi Anda
const char* password = "@Bhakti135"; // Kata sandi jaringan Wi-Fi Anda

// URL awal untuk mengirim UID RFID
const char* baseURL = "https://script.google.com/macros/s/AKfycbzm9f13G7LkSwo8yUvMnh1mym0ZlMEJPaMgePDwAO8NWxGgRE2o3IYsugAhsT7hqao8ug/exec?name=";

// Pin untuk buzzer
const int buzzerPin = D1;

// Pin untuk RFID
const uint8_t SS_PIN = D8;
const uint8_t RST_PIN = D0;
MFRC522 mfrc522(SS_PIN, RST_PIN);

// Objek WiFiClientSecure dan HTTPClient
WiFiClientSecure client;
HTTPClient http;

void setup() {
    // Inisialisasi serial untuk debugging
    Serial.begin(115200);

    // Menghubungkan ke jaringan Wi-Fi
    Serial.println("Menghubungkan ke jaringan Wi-Fi...");
    WiFi.begin(ssid, password);

    // Tunggu hingga terhubung ke jaringan Wi-Fi
    while (WiFi.status() != WL_CONNECTED) {
        delay(100);
        Serial.print(".");
    }
    Serial.println("\nTerhubung ke jaringan Wi-Fi!");

    // Konfigurasi client untuk menerima semua sertifikat
    client.setInsecure();

    // Inisialisasi SPI dan RFID
    SPI.begin();
    mfrc522.PCD_Init();

    // Inisialisasi buzzer
    pinMode(buzzerPin, OUTPUT);
}

void loop() {
    // Cek apakah ada kartu RFID baru
    if (!mfrc522.PICC_IsNewCardPresent()) {
        return;
    }

    // Cek apakah kartu dapat dibaca
    if (!mfrc522.PICC_ReadCardSerial()) {
        return;
    }

    // Baca UID kartu RFID
    String rfidUID = "";
    for (byte i = 0; i < mfrc522.uid.size; i++) {
        rfidUID += String(mfrc522.uid.uidByte[i], HEX);
    }

    // Cetak UID RFID yang dibaca
    Serial.print("rfidUID: ");
    Serial.println(rfidUID);

    // Tambahkan UID RFID ke URL awal
    String fullURL = String(baseURL) + rfidUID;

    // Inisialisasi timer untuk mengukur waktu pengiriman
    unsigned long startTime = millis();

    // Kirim permintaan HTTP GET menggunakan `HTTPClient` dan `WiFiClientSecure`
    http.begin(client, fullURL);
    int httpResponseCode = http.GET();

    // Akhiri koneksi HTTP
    http.end();

    // Catat waktu yang dibutuhkan untuk pengiriman data
    unsigned long endTime = millis();
    unsigned long timeTaken = endTime - startTime;

    Serial.print("Waktu pengiriman data: ");
    Serial.println(timeTaken);

    // Periksa kode respons HTTP
    if (httpResponseCode > 0) {
        // Jika permintaan berhasil, cetak respons dari server
        Serial.println("\nKoneksi HTTPS berhasil!");
        Serial.println("Respons dari server:");
        Serial.println(http.getString());

        // Bunyikan buzzer sekali
        buzzOnce();
    } else {
        // Jika permintaan gagal, cetak kode respons
        Serial.print("\nGagal mengakses server. Kode respons: ");
        Serial.println(httpResponseCode);

        // Bunyikan buzzer tiga kali
        buzzThreeTimes();
    }
}

void buzzOnce() {
    // Bunyikan buzzer sekali
    digitalWrite(buzzerPin, HIGH);
    delay(200);
    digitalWrite(buzzerPin, LOW);
}

void buzzThreeTimes() {
    // Bunyikan buzzer tiga kali
    for (int i = 0; i < 3; i++) {
        digitalWrite(buzzerPin, HIGH);
        delay(200);
        digitalWrite(buzzerPin, LOW);
        delay(200);
    }
}
