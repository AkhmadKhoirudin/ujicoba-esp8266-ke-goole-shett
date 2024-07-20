// Inisialisasi Google Sheet
var ss = SpreadsheetApp.openById('11kPvXw5ScU8NM-IXy6NEIt5kEfkIOjw8_HDy02v8qmY');
var sheet = ss.getSheetByName('Sheet1');

// Ubah zona waktu ke Asia/Jakarta
var timezone = "Asia/Jakarta";

// Fungsi untuk menangani permintaan HTTP GET
function doGet(e) {
    // Log permintaan yang diterima
    Logger.log(JSON.stringify(e));

    // Periksa parameter yang diterima
    if (!e || !e.parameter || !e.parameter.name) {
        return ContentService.createTextOutput("Parameter tidak lengkap atau tidak valid.");
    }

    // Ambil waktu saat ini dalam format yang diinginkan
    var currDate = new Date();
    var currTime = Utilities.formatDate(currDate, timezone, 'HH:mm:ss');
    var currDateStr = Utilities.formatDate(currDate, timezone, 'yyyy-MM-dd');

    // Ambil parameter `name`
    var name = stripQuotes(e.parameter.name);

    // Cek apakah ada entri dengan name yang sama pada tanggal yang sama
    var dataRange = sheet.getDataRange().getValues();
    for (var i = 1; i < dataRange.length; i++) {
        var rowDate = Utilities.formatDate(new Date(dataRange[i][0]), timezone, 'yyyy-MM-dd');
        var rowName = dataRange[i][2];
        if (rowDate == currDateStr && rowName == name) {
            return ContentService.createTextOutput("Data dengan nama yang sama pada tanggal yang sama sudah ada.");
        }
    }

    // Tambahkan data ke Google Sheet
    var nextRow = sheet.getLastRow() + 1;
    sheet.getRange("A" + nextRow).setValue(currDate);
    sheet.getRange("B" + nextRow).setValue(currTime);
    sheet.getRange("C" + nextRow).setValue(name);

    // Tambahkan rumus ke kolom D yang terkait dengan data di kolom C
    var formulaD = `=VLOOKUP(C${nextRow}; Sheet2!$A:F; 6; FALSE)`;
    sheet.getRange("F" + nextRow).setFormula(formulaD);

    // Tambahkan rumus ke kolom E untuk mengirim pesan
    var formulaE = `=HYPERLINK("https://wa.me/" & VLOOKUP(C${nextRow};Sheet2!$A:$D;4;FALSE)&"?text="& ENCODEURL("Absensi berhasil untuk " &VLOOKUP(C${nextRow};Sheet2!$A:$D;2;FALSE)& ".");"Kirim Pesan")`;
    sheet.getRange("E" + nextRow).setFormula(formulaE);

    // Tunggu sebentar untuk memastikan formula di kolom F sudah dihitung
    SpreadsheetApp.flush();

    // Ambil email dari kolom F
    var email = sheet.getRange("F" + nextRow).getValue();
    
    // Kirim email pemberitahuan
    var subject = "Pemberitahuan Absensi Baru";
    var body = `Absensi berhasil dicatat.\n\nNama: ${name}\nTanggal: ${currDateStr}\nWaktu: ${currTime}`;
    
    if (email) {
        MailApp.sendEmail(email, subject, body);
    } else {
        Logger.log("Email tidak ditemukan untuk nama: " + name);
    }

    // Kembalikan respons ke perangkat
    return ContentService.createTextOutput("Data berhasil disimpan ke Google Sheet.");
}

// Fungsi untuk menangani permintaan HTTP POST (tidak digunakan dalam skrip Anda)
function doPost(e) {
    if (!e.parameter || !e.parameter.value) {
        return ContentService.createTextOutput("Parameter tidak lengkap atau tidak valid.");
    }

    // Tambahkan data dari parameter `value` ke Google Sheet
    var range = sheet.getRange('A2');
    range.setValue(e.parameter.value);

    // Kembalikan respons ke perangkat
    return ContentService.createTextOutput("Data berhasil disimpan ke Google Sheet.");
}

// Fungsi untuk menghapus tanda kutip dari string
function stripQuotes(value) {
    return value.toString().replace(/^["']|['"]$/g, "");
}

// Fungsi tambahan, jika diperlukan, untuk pengujian atau penggunaan lainnya
function myFunction() {
    // Anda bisa menambahkan kode untuk pengujian atau fungsi tambahan di sini
}
