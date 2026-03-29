# ULUBEY_ASTRO Teknik Rapor - Mission Logic ve Interface Katmanı

## 1. Amaç ve Kapsam

Bu rapor, proje içindeki görev mantığı (Logic) ve donanım soyutlama (Interfaces) katmanlarının teknik çalışma prensibini açıklar. Hedef, kodun hangi koşulda hangi adımı çalıştırdığını, FreeRTOS ile nasıl zamanlandığını ve hata durumlarında nasıl güvenli davrandığını netleştirmektir.

İncelenen temel dosyalar:
- [ULUBEY_ASTRO/Logic/mission_manager.c](../ULUBEY_ASTRO/Logic/mission_manager.c)
- [ULUBEY_ASTRO/Logic/mission_manager.h](../ULUBEY_ASTRO/Logic/mission_manager.h)
- [ULUBEY_ASTRO/Interfaces/satellite_if.h](../ULUBEY_ASTRO/Interfaces/satellite_if.h)
- [ULUBEY_ASTRO/Interfaces/satellite_if.c](../ULUBEY_ASTRO/Interfaces/satellite_if.c)
- [ULUBEY_ASTRO/Core/Src/freertos.c](../ULUBEY_ASTRO/Core/Src/freertos.c)
- [ULUBEY_ASTRO/Core/Src/main.c](../ULUBEY_ASTRO/Core/Src/main.c)
- [ULUBEY_ASTRO/Core/Inc/FreeRTOSConfig.h](../ULUBEY_ASTRO/Core/Inc/FreeRTOSConfig.h)

## 2. Mimari Yapı

Yazılım iki ana katmanla ayrılmıştır:

1. Interfaces katmanı
- Donanımla etkileşen fonksiyon protokollerini tanımlar.
- Üst katmanın (Logic) doğrudan HAL veya MCU bağımlı kod yazmasını engeller.
- Bu aşamada fonksiyonlar placeholder davranışı sergiler ve log yazar.

2. Logic katmanı
- Görev kararlarını verir.
- Konum kontrolü, ADCS mod seçimi, stabilizasyon bekleme ve payload tetikleme adımlarını yönetir.
- FreeRTOS task olarak çalışır.

Bu ayrım sayesinde görev mantığı donanımdan ayrıştırılmış olur ve test/simülasyon aşamaları kolaylaşır.

## 3. Interface Katmanı Teknik Özeti

Interface API, [ULUBEY_ASTRO/Interfaces/satellite_if.h](../ULUBEY_ASTRO/Interfaces/satellite_if.h) içinde tanımlıdır.

Sağlanan fonksiyonlar:
- IF_ADCS_GetStatus: ADCS stabil durumunu okur.
- IF_ADCS_SetMode: ADCS modunu ayarlar. 0 Idle/Sleep, 1 Targeting.
- IF_GPS_GetLocation: Enlem ve boylam değerlerini doldurur.
- IF_Camera_Capture: Kamera çekimini tetikler.
- IF_Radio_Transmit: Radyo iletimini tetikler.

Mevcut placeholder uygulaması [ULUBEY_ASTRO/Interfaces/satellite_if.c](../ULUBEY_ASTRO/Interfaces/satellite_if.c) dosyasındadır:
- GPS her zaman Bursa koordinatı döner: lat=40.5, lon=29.0
- ADCS durum fonksiyonu STABLE döner.
- Diğer fonksiyonlar eylemi UART log formatında bildirir.

## 4. Mission Task İş Akışı

Ana görev fonksiyonu:
- vTaskMissionManager
- Konum: [ULUBEY_ASTRO/Logic/mission_manager.c](../ULUBEY_ASTRO/Logic/mission_manager.c)

Döngü davranışı:
1. GPS verisi okunur.
2. Enlem 40.0 ile 41.0 arasında ise hedef bölge kabul edilir.
3. ADCS targeting moda çekilir.
4. ADCS stabilizasyonu polling ile beklenir.
5. Stabil ise kamera ve radyo adımları tetiklenir.
6. Hedef dışı durumda ADCS idle moda alınıp döngü beklemeye girer.

## 5. Timeout ve Fail-Safe Davranışı

Kilitlenmeyi önlemek için stabilizasyon bekleme aşamasında sınırlı deneme vardır:
- Maksimum deneme: 50
- Her deneme arası gecikme: 200 ms
- Toplam timeout: yaklaşık 10 saniye

Hesap:
- 50 x 200 ms = 10000 ms = 10 s

Timeout sonunda:
- Hata logu basılır: [ERROR]: ADCS Timeout!
- ADCS idle moda alınır.
- Görev continue ile ana döngü başına döner.

Bu yaklaşım, ADCS hiç stabil olamazsa bile taskın sonsuz beklemede kalmasını engeller.

## 6. FreeRTOS Entegrasyonu

Task oluşturma ve scheduler bağlantısı iki noktada yapılır:

1. Thread tanimi ve oluşturma
- [ULUBEY_ASTRO/Core/Src/freertos.c](../ULUBEY_ASTRO/Core/Src/freertos.c)
- missionMgr adlı thread osThreadNew ile oluşturulur.
- stack_size: 256 * 4 byte
- priority: osPriorityAboveNormal
- Thread oluşmazsa Error_Handler çağrılır.

2. Başlatma akışı
- [ULUBEY_ASTRO/Core/Src/main.c](../ULUBEY_ASTRO/Core/Src/main.c)
- osKernelInitialize sonrasında MX_FREERTOS_Init çağrılır.
- Sonrasında osKernelStart ile scheduler devreye alınır.

## 7. Zamanlama ve Tick Bilgisi

FreeRTOS konfigürasyonunda tick hızı:
- configTICK_RATE_HZ = 1000
- Konum: [ULUBEY_ASTRO/Core/Inc/FreeRTOSConfig.h](../ULUBEY_ASTRO/Core/Inc/FreeRTOSConfig.h)

Buna göre:
- osDelay(200) yaklaşık 200 ms
- osDelay(1000) yaklaşık 1 s

Task periyodu:
- Normal döngü sonunda 1 saniye bekleme vardır.
- Hedef bölge içinde ADCS polling aşaması ek süre üretebilir.

## 8. Loglama ve İzlenebilirlik

Log formatı katman bazlı etiketlenmiştir:
- Logic: [LOGIC][MISSION] ...
- Interface: [IF][ADCS], [IF][GPS], [IF][CAM], [IF][RADIO]
- Hata: [ERROR] ...

Bu yapı, test ve simülasyon sırasında olay sırasını takip etmeyi kolaylaştırır.

## 10. Sonuç

Kod tabanı, görev mantığı ile donanım etkileşimini ayrı katmanlarda tutan temiz bir mimariye sahiptir. Mission task, hedef bölge kararını verir, ADCS stabilizasyonunu denetler, timeout ile fail-safe uygular ve payload adımlarını kontrollü şekilde tetikler. Bu yapı hem test/simülasyon hem de gerçek donanıma geçiş için uygun bir temel sunar.
