# Tekshiruv natijalari

Sana: 2026-11-25. Haqiqiy qurilma ulanmagan; quyidagilar mahalliy kompilyator va simulyator natijalari.

Vositalar: Arduino CLI 1.6.0, Arduino AVR Boards 1.8.6, LiquidCrystal 1.0.7, DHT sensor library 1.4.6, Adafruit Unified Sensor 1.1.15, avr8js 0.21.1.

| Konfiguratsiya | Flash / 30720 bayt | Statik RAM / 2048 bayt | Natija |
| --- | --- | --- | --- |
| USB DEMO, tashqi pinlar o'chirilgan | 9158 | 372 | O'tdi |
| Parallel LCD DEMO | 10444 | 408 | O'tdi |
| Real, LCD + DHT22 + rele + LED + suv sathi | 14328 | 422 | O'tdi |
| Real, LCD + DHT11, eski bootloader, barcha qo'shimchalar | 14326 | 422 | O'tdi |
| Real, LCD/DHT/relesiz tuproq datchigi | 10570 | 366 | O'tdi |
| Boshqaruv test dasturi | 6430 | 210 | O'tdi |

RAM sonlari statik foydalanishni bildiradi; ish paytidagi stekni o'z ichiga olmaydi. Arduino AVR core kutubxonasidagi `new.cpp` fayli ayrim yangi buildlarda ishlatilmagan parametr haqida ogohlantirish beradi; loyiha manba fayllari bo'yicha ogohlantirish qolmagan.

## Bajarilgan sinovlar

- 20 ta compile-time tekshiruv: ikki yo'nalishdagi namlik xaritasi, chegaralash, noto'g'ri kalibrlash, `millis()` aylanishi, gisterezis va boshlash/to'xtatish shartlari.
- 27 ta bajariladigan tekshiruv: pinlar, EEPROM saqlash/qayta o'qish/CRC buzilishi, rele boshlang'ich OFF holati, AUTO talablari, tanaffus, quruqlikda boshlash, namlikda to'xtash, timeout va fault, RESET, eski o'qish, suv yo'qligi, datchik xatosi, bloklangan apparat, haqiqiy ADC kodi va foiz hisoblash.
- Asosiy USB DEMO dasturining 24 soniyasi simulyatsiya qilindi: 26 ta ekran kadri, barcha beshta sahifa, 16 belgilik qatorlar va tashqi GPIO lar o'chirilganligi tekshirildi.
- Parallel LCD DEMO dasturining 24 soniyasi simulyatsiya qilindi: 28 ta ekran kadri, 56 qator. Haqiqiy GPIO signallari HD44780 dekoderiga berilib, ekranga yozilgan belgilar USB nusxasi bilan solishtirildi.

Tekshiruvni qayta bajarish: README dagi ishlab chiquvchi buyruqlari. GitHub Actions konfiguratsiyasi qo'shilgan, ammo GitHub da hali bajarilmagan.

## Apparatda qolgan ish

Suratdagi datchik modeli, LCD interfeysi, real pinlar, quvvat va nasos ulanishlarini tekshirish kerak. Shundan so'ng haqiqiy tuproqda ikki nuqtali kalibrlash va nasos bilan sinov bajariladi. Kompilyatsiya va simulyatsiya bularning o'rnini bosa olmaydi.
