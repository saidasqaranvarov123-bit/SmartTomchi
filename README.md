# Smart Tomchi

**Arduino Nano uchun tuproq namligi tahlili, 16×2 LCD ekran va ixtiyoriy avtomatik sug'orish dasturi.**

Ekranda `SMART TOMCHI` yozuvi bilan ishga tushish animatsiyasi, namlik foizi va to'lish chizig'i, harorat/havo namligi, tahlil, sug'orish holati hamda ishlash vaqti almashib turadi. Barcha ekranlar USB Serial Monitor oynasida ham ko'rinadi.

## Hozirgi holat

Kod klassik **Nano / ATmega328P** uchun yozilgan. Qurilma suratidagi datchik modeli va pin ulanishlari tasdiqlanmagan. Jadvaldagi pinlar **namunaviy ulanish**, suratdan aniqlangan ulanish emas.

Boshlang'ich sozlama **DEMO + USB ekran**: tashqi GPIO chiqishlari faollashmaydi, namlik va harorat qiymatlari namoyish uchun yaratiladi. Har bir ma'lumot sahifasida `DEMO` belgisi bor. Haqiqiy LCD va datchiklarni yoqishdan oldin `Config.h` ni mavjud yig'ilmaga moslash kerak.

Bu loyiha apparatda sinovdan o'tkazilmagan. Kompilyatsiya va AVR simulyatoridagi sinovlar natijalari [TEST_RESULTS.md](TEST_RESULTS.md) da berilgan.

## Imkoniyatlar

- 4 soniyalik boshlanish animatsiyasi va 4 soniyada almashadigan beshta sahifa.
- Analog tuproq namligi datchigi uchun 9 o'qishdan median filtr.
- Quruq va nam holatlar bo'yicha ikki nuqtali kalibrlash; qarama-qarshi yo'nalishdagi analog datchiklarni ham qo'llaydi.
- EEPROM xotirasiga faqat `SAVE` buyrug'ida yozish; versiya va CRC bilan tekshirish.
- Ixtiyoriy DHT11 yoki DHT22 orqali harorat va havo namligi.
- Ixtiyoriy uchta LED, nasos relesi va suv sathi kaliti.
- Quruqlik chegarasi, sug'orishni to'xtatish chegarasi va qayta yoqish tanaffusi.
- Noto'g'ri/eski tuproq o'qishi, suv yo'qligi yoki vaqt chegarasida nasosni o'chirish.
- Pin takrorlanishi va Nano uchun yaroqsiz chiqish pinlarini aniqlash.
- Qatorlarni keshlash: LCD faqat o'zgargan qatorni yangilaydi; siklda `lcd.clear()` ishlatilmaydi.
- Doimiy `String` obyektlari va katta dinamik buferlar ishlatilmaydi.

## Ekranlar

Quyidagi sonlar faqat ko'rinish namunasi:

```text
+----------------+
|SMART TOMCHI   /|
|DEMO yuklanmoqda|
+----------------+

+----------------+
|DEMO Namlik: 65%|
|[######    ] OK |
+----------------+

+----------------+
|DEMO Havo       |
|T:24.0C H:62%   |
+----------------+

+----------------+
|DEMO Tahlil     |
|ME'YOR: holat OK|
+----------------+

+----------------+
|DEMO Sug'orish  |
|DEMO: nasos OFF |
+----------------+
```

Haqiqiy LCD da chiziq maxsus belgilar bilan qisman ham to'ladi. USB nusxasida bu belgilar `#` bilan ifodalanadi. LCD mosligi uchun interfeys matnlari ASCII harflarida yozilgan.

## Tez boshlash: USB namoyish

1. [Arduino IDE](https://www.arduino.cc/en/software/) ni o'rnating.
2. Boards Manager orqali **Arduino AVR Boards 1.8.6** paketini o'rnating.
3. Loyiha papkasining nomi `SmartTomchi` bo'lsin. `SmartTomchi.ino` va barcha `.h` fayllar shu papkada qolsin. GitHub ZIP ni ochganda `SmartTomchi-main` papkasini `SmartTomchi` deb qayta nomlang.
4. `SmartTomchi.ino` ni Arduino IDE orqali oching.
5. Platani ma'lumot uzata oladigan USB kabel bilan ulang. Sinovda nasos quvvatini uzing.
6. **Tools → Board → Arduino AVR Boards → Arduino Nano** va tegishli **Port** ni tanlang.
7. Processor sifatida **ATmega328P** ni tanlang. Avval **Verify**, keyin **Upload** ni bosing.
8. **Serial Monitor → 9600 baud → Newline** ni tanlang. Sahifalar USB oynasida almashadi.

Yuklash avvalgi dasturni almashtiradi. Kerak bo'lsa, avvalgi manba kodini alohida saqlang. Eski bootloaderli Nano uchun **ATmega328P (Old Bootloader)** talab qilinishi mumkin.

## LCD ni ishlatish

Qo'llanadigan ekran: **parallel HD44780 mos 16×2 LCD**, 4-bit ulanish. Orqasida I2C adapter bo'lsa, bu drayver unga mos emas. Qurilma suratida ekran old tomondan ko'ringani uchun uning interfeysi hali tasdiqlanmagan.

Library Manager orqali **LiquidCrystal 1.0.7** kutubxonasini o'rnating. `Config.h` dagi olti LCD pinini haqiqiy ulanishga moslang. Quyidagilar faqat namuna:

| LCD belgisi | Nano namunaviy pini |
| --- | --- |
| RS | D12 |
| E / Enable | D11 |
| D4 | D5 |
| D5 | D4 |
| D6 | D3 |
| D7 | D2 |
| R/W | GND |
| VSS | GND |
| VDD | Mos 5V ta'minot |
| VO | Kontrast potensiometrining o'rta chiqishi |

LCD quvvati, yoritgichi va kontrast ulanishini uning moduliga mos tekshiring. D0/D1 USB aloqa uchun band. LCD D0–D3 ma'lumot pinlari 4-bit rejimda ishlatilmaydi.

Ulanishlar tasdiqlangach, `Config.h` dagi mavjud qiymatlarni o'zgartiring:

```cpp
#define ST_DEMO_MODE 1
#define ST_PIN_MAPPING_CONFIRMED 1
#define ST_LCD_ENABLED 1
```

Qayta yuklang: haqiqiy ekranda ham namoyish sahifalari chiqadi. Qora kataklar yoki ko'rinmas matn bo'lsa, avval VO kontrastini va RS/E/D4–D7 ulanishini tekshiring.

## Haqiqiy datchiklar

Analog chiqishli tuproq datchigining **AO** chiqishini `cfg::soilPin` ga moslang. Faqat raqamli DO chiqishidan foiz o'lchash qo'llanmaydi. `cfg::soilPin = A0` — namuna. Datchik quvvati/chiqish kuchlanishi Nano kirishiga mos bo'lishi kerak.

```cpp
#define ST_DEMO_MODE 0
#define ST_PIN_MAPPING_CONFIRMED 1
#define ST_LCD_ENABLED 1
#define ST_DHT_MODEL 0
#define ST_RELAY_ENABLED 0
```

Kalibrlashgacha ekranda `KALIBRLASH KERAK` va xom `RAW` qiymati ko'rinadi. Taxminiy 800/350 chegaralari haqiqiy o'lchov sifatida avtomatik qabul qilinmaydi.

### Harorat va havo namligi

Faqat datchik haqiqatan DHT11 yoki DHT22 bo'lsa, `ST_DHT_MODEL` ni `11` yoki `22` qiling. `cfg::dhtPin` ni DATA simiga moslang; namunada D7. Library Manager orqali quyidagilarni o'rnating:

- **DHT sensor library 1.4.6** — Adafruit.
- **Adafruit Unified Sensor 1.1.15**.

DHT yo'q bo'lsa `0` qoldiring: ekran `DHT ulanmagan` deb ko'rsatadi. Suratdagi metall zond avtomatik ravishda DHT deb hisoblanmaydi. DS18B20, termopara yoki boshqa zond uchun uning modeli aniqlangach alohida drayver kerak.

Tuproq namligi va havo namligi ikki xil ko'rsatkich. DHT xatosi havo sahifasida ko'rsatiladi; sug'orish qarori tuproq namligiga asoslanadi.

## Kalibrlash

Serial Monitor: **9600 baud**, qator oxiri **Newline** yoki **Both NL & CR**. Katta/kichik harflar bir xil qabul qilinadi; buyruqlarni quyidagidek bo'shliqlar bilan yozing.

1. Datchikni tanlangan quruq tuproq sharoitiga qo'ying. `RAW` ko'rsatkichi barqarorlashishini kuting; o'qish har 2.5 soniyada yangilanadi.
2. `CAL DRY` yuboring.
3. Datchikni tanlangan nam tuproq sharoitiga qo'ying, elektron qismini suvga botirmang. Yangi `RAW` qiymati barqarorlashishini kuting.
4. `CAL WET` yuboring. Ikki qiymat orasida kamida 100 RAW farq bo'lishi kerak.
5. `SAVE` yuboring. Sozlama EEPROM ning 0–9 baytlarida saqlanadi.
6. `STATUS` bilan tekshiring. Qayta yoqilganda tekshirilgan kalibrlash qayta o'qiladi.

Bu foiz **ikki tayanch holat orasidagi nisbiy namlik indeksi**, laboratoriyada o'lchangan hajmiy suv foizi emas. Datchik yoki tuproq o'zgarsa qayta kalibrlang. Kalibrlash buyrug'i avtomatik sug'orishni o'chiradi.

## Sug'orish va LED lar

Rele/nasos ulanishi tasdiqlanmaguncha `ST_RELAY_ENABLED = 0` qolsin. Nasos Arduino pinidan bevosita quvvatlanmaydi; uning quvvatiga mos boshqaruv moduli va ta'minot kerak. Relayning faollashish darajasini `cfg::relayActiveLow` bilan moslang.

Namunaviy qo'shimcha pinlar:

| Vazifa | Pin | Shart |
| --- | --- | --- |
| Nasos relesi IN | D6 | `ST_RELAY_ENABLED = 1` |
| Qizil LED | D8 | `ST_LEDS_ENABLED = 1`, mos ketma-ket rezistor |
| Yashil LED | D9 | `ST_LEDS_ENABLED = 1`, mos ketma-ket rezistor |
| Ko'k LED | D10 | `ST_LEDS_ENABLED = 1`, mos ketma-ket rezistor |
| Suv sathi kaliti | A1 | `ST_TANK_ENABLED = 1`, kalit GND ga yopilsa suv bor |

Suv sathi kirishi `INPUT_PULLUP`: HIGH bo'lsa suv yo'q yoki sim uzilgan deb qabul qilinadi. Suv sathi kaliti bo'lmasa tizim suv borligini tekshira olmaydi; Serial holatida `WATER=NA` chiqadi.

Haqiqiy rejim, tekshirilgan pinlar, rele, kalibrlangan va yangi tuproq o'qishi tayyor bo'lgach **`AUTO ON`** yuboring. Qurilma yoqilganda AUTO har doim o'chirilgan bo'ladi. DEMO rejimida AUTO va rele yoqilmaydi.

Namunaviy chegaralar `Config.h` da:

| Parametr | Qiymat |
| --- | --- |
| Sug'orishni boshlash | Namlik ≤30% |
| Sug'orishni to'xtatish | Namlik ≥55% |
| Juda nam deb ko'rsatish | Namlik ≥80% |
| Bir ishga tushishda vaqt chegarasi | 10 soniya |
| Qayta yoqishdan oldingi tanaffus | 30 soniya, birinchi ishga tushishda ham |
| O'qishning eskirish vaqti | 6.5 soniya |

Bu qiymatlar ekin va qurilma uchun universal norma emas; o'zingizning tuproq, datchik va nasosingiz bilan sozlang. 10 soniya chegarasiga yetish **fault** hisoblanadi: nasos to'xtaydi va `RESET` + yangi `AUTO ON` gacha qayta yoqilmaydi. `RESET` o'zi nasosni ishga tushirmaydi.

Qizil LED: quruqlik; xatoda miltillaydi. Yashil: namlik me'yorda. Ko'k: namlik yuqori yoki nasos ishlayapti. LED lar DEMO rejimida ham namunaviy holatni ko'rsatishi mumkin; rele faollashmaydi.

## USB buyruqlari

| Buyruq | Natija |
| --- | --- |
| `HELP` | Buyruqlar ro'yxati |
| `STATUS` | Rejim, RAW, foiz, harorat, havo namligi, kalibrlash, suv va nasos holati |
| `RAW` | Oxirgi xom o'qish; DEMO bo'lsa belgilanadi |
| `CAL DRY` | Quruq tayanch nuqta |
| `CAL WET` | Nam tayanch nuqta |
| `SAVE` | Kalibrlashni tekshirib EEPROM ga yozish |
| `AUTO ON` | Shartlar tayyor bo'lsa avtomatik sug'orishni yoqish |
| `AUTO OFF` | Sug'orishni o'chirish |
| `RESET` | Vaqt faultini tozalash; AUTO o'chirilgan qoladi |

## Fayllar

```text
SmartTomchi/
  SmartTomchi.ino       Asosiy dastur
  Config.h             Pinlar, rejimlar, chegaralar
  Logic.h              Kalibrlash va boshqaruv shartlari
  LogicChecks.h        Kompilyatsiya vaqtidagi chegaraviy sinovlar
  Calibration.h        EEPROM va CRC
  Hardware.h           Pinlarni tekshirish
  Sensors.h            Analog namlik, DHT, DEMO
  Controller.h         Rele va LED boshqaruvi
  Screen.h             LCD va USB ekran nusxasi
  Console.h            USB buyruqlari
  tests/               AVR simulyatsiya sinovlari
  .github/workflows/   GitHub avtomatik tekshiruvi
```

## Ishlab chiquvchi tekshiruvi

Arduino CLI 1.6.0, Python 3 va Node.js 22 bilan tekshirildi. Node/Python faqat ishlab chiqishdagi sinov uchun; qurilmaning ishlashiga kerak emas.

```text
arduino-cli core update-index
arduino-cli core install arduino:avr@1.8.6
arduino-cli lib install "LiquidCrystal@1.0.7" "DHT sensor library@1.4.6" "Adafruit Unified Sensor@1.1.15"
npm ci --ignore-scripts
python tests/run_checks.py
```

Sinovlar beshta apparat konfiguratsiyasini va alohida test dasturini kompilyatsiya qiladi. Haqiqiy hosil bo'lgan AVR `.hex` dasturlari `avr8js` da bajariladi: 27 ta boshqaruv/EEPROM/ADC tekshiruvi, beshta sahifa, parallel LCD signallarini HD44780 dekoderi bilan tekshirish. Kompilyatsiya vaqtida yana 20 ta chegaraviy shart tekshiriladi.

Analog kirish ochiq qolsa tasodifiy o'qish paydo bo'lishi mumkin. Dastur 0/1023 atrofidagi xatolarni rad etadi, lekin uzilgan har bir analog simni ishonchli aniqlashni kafolatlamaydi. Simulyator namlikning fizik aniqligi, rele elektr ulanishi yoki nasosning suv oqimini tasdiqlamaydi.

## GitHub ga joylash

1. GitHub da `SmartTomchi` nomli bo'sh repository yarating.
2. Tayyor ZIP ni kompyuteringizda oching.
3. **Add file → Upload files** orqali `SmartTomchi` ichidagi fayl va papkalarni repository ildiziga yuklang. ZIP faylining o'zini kod o'rniga yuklamang.
4. `.github` va `.gitignore` yashirin fayl/papkalari ham yuklanganini tekshiring. `.build` va `node_modules` ni yuklamang.
5. O'zgarishlarni saqlang. GitHub **Actions** bo'limida avtomatik tekshiruv boshlanadi.

Loyiha ichida telefon raqami, token yoki Wi-Fi paroli yo'q. Tashqi xizmat yoki OpenAI API talab qilinmaydi.

## Rasmiy manbalar

- [Arduino dasturini yuklab olish](https://www.arduino.cc/en/software/)
- [Nano protsessor va bootloader tanlovi](https://support.arduino.cc/hc/en-us/articles/4401874304274-Select-the-right-processor-for-Arduino-Nano)
- [Arduino LiquidCrystal ulanish namunasi](https://github.com/arduino-libraries/LiquidCrystal/blob/master/examples/HelloWorld/HelloWorld.ino)
- [Adafruit DHT kutubxonasi](https://github.com/adafruit/DHT-sensor-library)
- [avr8js simulyatori](https://github.com/wokwi/avr8js)
- [GitHub ga fayllarni yuklash](https://docs.github.com/en/repositories/working-with-files/managing-files/adding-a-file-to-a-repository)
