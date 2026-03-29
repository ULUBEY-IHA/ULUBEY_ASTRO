# ULUBEY_ASTRO Teknik Rapor - Mission Logic ve Interface Katmani

## 1. Amac ve Kapsam

Bu rapor, proje icindeki gorev mantigi (Logic) ve donanim soyutlama (Interfaces) katmanlarinin teknik calisma prensibini aciklar. Hedef, kodun hangi kosulda hangi adimi calistirdigini, FreeRTOS ile nasil zamanlandigini ve hata durumlarinda nasil guvenli davrandigini netlestirmektir.

Incelenen temel dosyalar:
- [ULUBEY_ASTRO/Logic/mission_manager.c](../ULUBEY_ASTRO/Logic/mission_manager.c)
- [ULUBEY_ASTRO/Logic/mission_manager.h](../ULUBEY_ASTRO/Logic/mission_manager.h)
- [ULUBEY_ASTRO/Interfaces/satellite_if.h](../ULUBEY_ASTRO/Interfaces/satellite_if.h)
- [ULUBEY_ASTRO/Interfaces/satellite_if.c](../ULUBEY_ASTRO/Interfaces/satellite_if.c)
- [ULUBEY_ASTRO/Core/Src/freertos.c](../ULUBEY_ASTRO/Core/Src/freertos.c)
- [ULUBEY_ASTRO/Core/Src/main.c](../ULUBEY_ASTRO/Core/Src/main.c)
- [ULUBEY_ASTRO/Core/Inc/FreeRTOSConfig.h](../ULUBEY_ASTRO/Core/Inc/FreeRTOSConfig.h)

## 2. Mimari Yapi

Yazilim iki ana katmanla ayrilmistir:

1. Interfaces katmani
- Donanimla etkilesen fonksiyon protokollerini tanimlar.
- Ust katmanin (Logic) dogrudan HAL veya MCU bagimli kod yazmasini engeller.
- Bu asamada fonksiyonlar placeholder davranis sergiler ve log yazar.

2. Logic katmani
- Gorev kararlarini verir.
- Konum kontrolu, ADCS mod secimi, stabilizasyon bekleme ve payload tetikleme adimlarini yonetir.
- FreeRTOS task olarak calisir.

Bu ayrim sayesinde gorev mantigi donanimdan ayristirilmis olur ve test/simulasyon asamalari kolaylasir.

## 3. Interface Katmani Teknik Ozeti

Interface API, [ULUBEY_ASTRO/Interfaces/satellite_if.h](../ULUBEY_ASTRO/Interfaces/satellite_if.h) icinde tanimlidir.

Saglanan fonksiyonlar:
- IF_ADCS_GetStatus: ADCS stabil durumunu okur.
- IF_ADCS_SetMode: ADCS modunu ayarlar. 0 Idle/Sleep, 1 Targeting.
- IF_GPS_GetLocation: Enlem ve boylam degerlerini doldurur.
- IF_Camera_Capture: Kamera cekimini tetikler.
- IF_Radio_Transmit: Radyo iletimini tetikler.

Mevcut placeholder uygulamasi [ULUBEY_ASTRO/Interfaces/satellite_if.c](../ULUBEY_ASTRO/Interfaces/satellite_if.c) dosyasindadir:
- GPS her zaman Bursa koordinati doner: lat=40.5, lon=29.0
- ADCS durum fonksiyonu STABLE doner.
- Diger fonksiyonlar eylemi UART log formatinda bildirir.

## 4. Mission Task Is Akisi

Ana gorev fonksiyonu:
- vTaskMissionManager
- Konum: [ULUBEY_ASTRO/Logic/mission_manager.c](../ULUBEY_ASTRO/Logic/mission_manager.c)

Dongu davranisi:
1. GPS verisi okunur.
2. Enlem 40.0 ile 41.0 arasinda ise hedef bolge kabul edilir.
3. ADCS targeting moda cekilir.
4. ADCS stabilizasyonu polling ile beklenir.
5. Stabil ise kamera ve radyo adimlari tetiklenir.
6. Hedef disi durumda ADCS idle moda alinip dongu beklemeye girer.

## 5. Timeout ve Fail-Safe Davranisi

Kilitlenmeyi onlemek icin stabilizasyon bekleme asamasinda sinirli deneme vardir:
- Maksimum deneme: 50
- Her deneme arasi gecikme: 200 ms
- Toplam timeout: yaklasik 10 saniye

Hesap:
- 50 x 200 ms = 10000 ms = 10 s

Timeout sonunda:
- Hata logu basilir: [ERROR]: ADCS Timeout!
- ADCS idle moda alinir.
- Gorev continue ile ana dongu basina doner.

Bu yaklasim, ADCS hic stabil olamazsa bile taskin sonsuz beklemede kalmasini engeller.

## 6. FreeRTOS Entegrasyonu

Task olusturma ve scheduler baglantisi iki noktada yapilir:

1. Thread tanimi ve olusturma
- [ULUBEY_ASTRO/Core/Src/freertos.c](../ULUBEY_ASTRO/Core/Src/freertos.c)
- missionMgr adli thread osThreadNew ile olusturulur.
- stack_size: 256 * 4 byte
- priority: osPriorityAboveNormal
- Thread olusmazsa Error_Handler cagrilir.

2. Baslatma akisi
- [ULUBEY_ASTRO/Core/Src/main.c](../ULUBEY_ASTRO/Core/Src/main.c)
- osKernelInitialize sonrasinda MX_FREERTOS_Init cagrilir.
- Sonrasinda osKernelStart ile scheduler devreye alinir.

## 7. Zamanlama ve Tick Bilgisi

FreeRTOS konfigurasyonunda tick hizi:
- configTICK_RATE_HZ = 1000
- Konum: [ULUBEY_ASTRO/Core/Inc/FreeRTOSConfig.h](../ULUBEY_ASTRO/Core/Inc/FreeRTOSConfig.h)

Buna gore:
- osDelay(200) yaklasik 200 ms
- osDelay(1000) yaklasik 1 s

Task periyodu:
- Normal dongu sonunda 1 saniye bekleme vardir.
- Hedef bolge icinde ADCS polling asamasi ek sure uretebilir.

## 8. Loglama ve Izlenebilirlik

Log formati katman bazli etiketlenmistir:
- Logic: [LOGIC][MISSION] ...
- Interface: [IF][ADCS], [IF][GPS], [IF][CAM], [IF][RADIO]
- Hata: [ERROR] ...

Bu yapi, test ve simulasyon sirasinda olay sirasini takip etmeyi kolaylastirir.

## 9. Teknik Sinirlar ve Iyilestirme Onerileri

Mevcut durum (placeholder asamasi):
- ADCS her zaman stabil dondugu icin timeout yolu normalde tetiklenmez.
- GPS sabit deger dondugu icin sistem surekli hedef bolge icinde calisir.

Bir sonraki teknik adimlar:
1. Interface katmanini gercek sensor/alt surucu verileri ile beslemek.
2. Polling yerine event tabanli yapilar (task notification, queue, event flags) kullanmak.
3. Mission task icin stack watermark ve CPU kullanim olcumu eklemek.
4. Loglari zaman damgasi ile zenginlestirmek.

## 10. Sonuc

Kod tabani, gorev mantigi ile donanim etkilesimini ayri katmanlarda tutan temiz bir mimariya sahiptir. Mission task, hedef bolge kararini verir, ADCS stabilizasyonunu denetler, timeout ile fail-safe uygular ve payload adimlarini kontrollu sekilde tetikler. Bu yapi hem test/simulasyon hem de gercek donanima gecis icin uygun bir temel sunar.
