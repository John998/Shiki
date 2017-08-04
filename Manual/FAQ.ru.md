#### Общий FAQ:
- _Для чего нужен Shiki?_  
Если у вас Intel Ivy процессор или новее, воспроизведение iTunes DRM контента не работает, но ваш GPU поддерживает HDCP, вы можете попробовать исправить ситуацию с помощью Shiki.

- _Как отключить загрузку Shiki?_  
Добавьте `-shikioff` в аргументы загрузки ядра (boot-args). Также он не загружается, если вписан аргумент -x или -s.

- _Как активировать отладочную информацию в DEBUG версиях Shiki?_  
Добавьте `-shikidbg` в аргументы загрузки ядра.

- _С какими версиями OS X/macOS он работает?_  
Он работает, начиная от 10.9 и выше. Однако рекомендуется использовать 10.10 и новее.  
Обратите внимание, что Shiki работает только со сравнительно новыми версиями iTunes, например, 12.3.3 или новее.  
Если у вас возникли проблемы загрузки Shiki на 10.10 и новее, попробуйте добавить `-liluslow` или `-lilufast` в аргументы загрузки ядра.

- _Как установить Shiki?_  
Он должен быть загружен на начальной стадии запуска системы. Рекомендуется использовать (kext injection) в загрузчике. Команда `kextload` не сработает, а загрузка из /S/L/E или /L/E нестабильна.

- _Опасно ли его использовать?_  
Любой инструмент может быть опасным в неумелых руках. Shiki сейчас стабилен, но всё равно считается экспериментальным.

- _Изменяет ли Shiki файлы на диске?_  
Последняя версия Shiki никак не влияет на ваш диск. Тем не менее некоторые команды ввода-вывода (I/O) из-за особенностей логики API будут сообщать об изменении файлов в те моменты, когда Shiki загружен.

- _Нужно ли изменять параметры System Integrity Protection (rootless)?_  
Это не требуется.

- _Как проверить, подходит/готова ли моя конфигурация ПК к попытке использования кекста?_  
Существует список конфигураций с подпунктами, которые стоит проверить. Подробности указаны в разде "Проверка конфигурации".

- _Почему я должен правильно настроить VDA перед использованием Shiki?_  
Вы теоретически можете этого не делать, но работа системы может быть непредсказуема.

- _Какие видеокарты (GPU) поддерживаются с недочётами?_  
   - Видеокарты, использующие фреймбуферы Azul (например, HD 4400, HD4600), при использовании полноценного завода в режиме единственного GPU (без дискретной видеокарты) не могут проигрывать HD видео. Это связано с некорректно работающим HDCP на этих картах. Проблема не имеет отношения к Shiki, и для предотвращения зависаний требуется [IntelGraphicsFixup.kext](https://sourceforge.net/p/intelgraphicsfixup).

- _Мой компьютер забанили?_  
Если вы можете просматривать трейлеры, но купленные фильмы не проигрываются даже после авторизации компьютера, возможно, ваш MAC адрес сетевой карты забанили. Иногда это проходит через некоторое время само, иногда нужно войти и выйти или сбросить конфигурацию DRM. Если ничего не помогает, стоит изменить LAN MAC адрес. Смотрите подробности использования libHookMac во второй части FAQ. Если вам это помогло, стоит перепрошивать сетевую карту другим MAC адресом.

- _У Shiki открытый код?_  
Начиная с версии 2.0.0 код открыт.

- _Как скачать защищенный DRM трейлер для локального тестирования?_  
Включите трассировку Coremedia: 
`defaults write com.apple.coremedia cfbyteflume_trace 1`  
После, отфильтровав лог по ключевому слову 'iTunes', в консоли вы увидите URL ссылку.  
`... <<< CFByteFlume >>> FigCFHTTPCheckCacheValidator: Comparing dictUrl = http://.....m4v, url = http://......m4v`  
Вставьте её в свой браузер и вы сможете скачать файл: [пример](https://applelife.ru/threads/chernye-trejlery-itunes.42290/page-11#post-583709).

- _Воспроизведение DRM приводит к искажению картинки и артефактам через некоторое время, с чем это связано?_  
Судя по тестам это баг в декодере Apple DRM. Проблема якобы существует и на оборудовании от Apple. Обычно все нормально, но 1080p фильмы с большим битрейтом могут это провоцировать. Попробуйте перезагрузить компьютер, сбросить конфигурацию DRM, как это написанно ниже, проверьте управление питанием процессора.

- _Могу я использовать Shiki для проигрывания HTML5 видео в Netflix?_  
Нет, так как Netflix ограничила их 1080p контент очень малым количеством Mac моделей, исключая даже большинство Mac Pro.

- _Можно использовать Shiki для применения других изменений?_  
Это технически возможно и обсуждаемо.

#### FAQ по настройке системы:
- _Как мне проверить, что аппаратное декодирование видео работает?_  
Выполните собранную утилиту [VDADecoderChecker для 10.11](https://applelife.ru/threads/shiki-patcher-polzovatelskogo-urovnja.1349123/page-2#post-595056)/[VDADecoderChecker для 10.12](https://applelife.ru/threads/shiki-patcher-polzovatelskogo-urovnja.1349123/page-26#post-647746) (или соберите [сами](https://github.com/cylonbrain/VDADecoderCheck)) и проверьте вывод:  
`GVA info: Successfully connected to the Intel plugin, offline Gen75`  
`Hardware acceleration is fully supported` (Аппаратное ускорение полностью поддерживается)

- _Как мне проверить, что IMEI/IGPU устройства присутствуют в IOReg?_  
Выполните `ioreg | grep IMEI` в терминале и убедитесь в наличии вывода вроде:  
`    | |   +-o IMEI@16  <class IOPCIDevice, id 0x100000209, registered, matched, active, busy 0 (6 ms), retain 11>`

- _Как мне проверить platform-id встроенной видеокарты?_  
Выполните `ioreg -l | grep platform-id` в терминале и убедитесь в наличии вывода вроде:  
` | | | | "AAPL,ig-platform-id" = <04001204>`  
`04 00 12 04` в данном случае является текущим platform-id.

- _Как мне включить отладку AppleGVA?_  
Выполните:  
`defaults write com.apple.AppleGVA gvaDebug -boolean yes`  
`defaults write com.apple.AppleGVA enableSyslog -boolean yes`  

- _Как мне включить отладку FP?_  
Выполните:  
`defaults write com.apple.coremedia fp_trace 2`  

- _Как мне изменить настройки аппаратного декодирования (иногда нужно для некоторых ATI и старых NVIDIA)?_  
Выполните одну из следующих строк в терминале:  
`defaults write com.apple.AppleGVA forceNV -boolean yes` — включение NVIDIA декодера  
`defaults write com.apple.AppleGVA forceATI -boolean yes` — включение ATI декодера  
`defaults write com.apple.AppleGVA forceIntel -boolean yes` — включение Intel декодера  
`defaults write com.apple.AppleGVA forceSWDecoder -boolean yes` — включение программного декодера  
`defaults write com.apple.coremedia hardwareVideoDecoder disable` — выключение аппаратного декодера  
`defaults write com.apple.coremedia hardwareVideoDecoder force` — включение аппаратного декодера  
Обратите внимание, что данные параметры *не* нужны в большинстве случаев, и их использование сломает вам CL и VDA.

- _Как мне сбросить конфигурацию DRM (может потребоваться для исправления падений)?_  
Выполните следующие команды в терминале:  
`defaults delete com.apple.coremedia`  
`defaults delete com.apple.AppleGVA`  
`sudo rm -rf /Users/Shared/SC\ Info`  
`sudo defaults delete com.apple.coremedia`  
`sudo defaults delete com.apple.AppleGVA`  
Затем убедитесь, что подпись AppleGVA не повреждена (команда не должна ничего вывести):  
`codesign --no-strict --verify /System/Library/PrivateFrameworks/AppleGVA.framework`  
Если это не так, восстановите AppleGVA.framework из другой системы с правильными правами.  
Перезагрузитесь дважды.

- _Как мне изменить MAC адрес сетевой карты с помощью [libHookMac.dylib](https://github.com/vit9696/Shiki/raw/master/HookMac/libHookMac.dylib)?_  
   - Выключите SIP (System Integrity Protection);  
   - Запустите Терминал (указав свой случайный MAC адрес):  
   `DYLD_INSERT_LIBRARIES=/full/path/to/libHookMac.dylib MAC=00:11:22:33:44:55 /Applications/iTunes.app/Contents/MacOS/iTunes`;  
   - Вы увидите соответствующий вывод терминала если адрес был изменён.

- _Как я могу проверить, что мой dyld shared cache в порядке?_  
Выполните следующие команды и сравните их вывод:  
`DYLD_PREBIND_DEBUG=1 DYLD_SHARED_CACHE_DONT_VALIDATE=1 "/Applications/QuickTime Player.app/Contents/MacOS/QuickTime Player"`  
`DYLD_PREBIND_DEBUG=1 "/Applications/QuickTime Player.app/Contents/MacOS/QuickTime Player"`  
Если их вывод сильно отличается (команда без DYLD_SHARED_CACHE_DONT_VALIDATE=1 выводит много путей), то ваш shared cache повреждён.  
Для того, чтобы это исправить, отключите Shiki, перезагрузите компьютер и запустите команду `sudo update_dyld_shared_cache -force` в терминале.

- _Какие модели компьютеров поддерживают аппаратное ускорение?_  
Это можно узнать из `/System/Library/PrivateFrameworks/AppleGVA.framework/Info.plist`, если ваша Mac модель или ID платы присутствуют там, то эта модель поддерживает аппаратное декодирование видео. Вы должны выбрать ближайшую к вашей конфигурации ПК. Для примера iMac13,1 использует и IGPU, и дискретную видеокарту в паре, тогда как iMac13,3 имеет только IGPU. Если вы используете Mac модель не имеющую дискретной видеокарты, то VDA не будет работать, и вероятно вы получите ошибку от VDADecoderChecker. Используйте более близкую Mac модель или отредактируйте свойство `forceOfflineRenderer` в Info.plist выставив там значение NO для исправления.

- _Как включить онлайн декодер Intel, когда в AppleGVA указан оффлайн?_  
Добавьте `shikigva=1` в аргументы загрузки ядра.

- _Как можно сделать инжекцию IOVARendererID/IOVARendererSubID в некоторых видеокартах NVIDIA?_  
Веб драйвера NVIDIA могут не добавлять эти значения для карт на архитектуре Maxwell. Вы можете их добавить в кекст состоящий из одного plist (legacy) сами. Правильными значениями для VP4 графических процессоров являются:  
IOVARendererID → `<08 00 04 01>`  
IOVARendererSubID → `<03 00 00 00>`  
Для VP3 нужны другие цифры IOVARendererID → `<04 00 04 01>`.  
Спасибо igork, что заметил это.

- _Мне не удалось завести VDA декодер на моей видеокарте AMD, что можно попробовать еще?_  
Стоит выставить приоритет для встроенной графики Intel в BIOS и использовать platform-id с коннекторами (например, `<03 00 66 01>` для HD 4000 или `<03 00 22 0D>` для HD 4600). Кажется, это помогает с некоторыми видеокартами AMD (например, HD 7750).

- _На моей карте AMD работает декодер аппаратного ускорения, если включить его в терминале, но DRM декодирование до сих пор не работает, что делать?_  
Некоторые видеокарты AMD, например HD 7750, поддерживают декодирование аппаратным ускорением, но не могут декодировать DRM видео. Причина неизвестна. Используйте обычный режим Shiki.

- _Что делает утилита [BoardHash](https://github.com/vit9696/Shiki/raw/master/BoardHash/BoardHash)?_  
BoardHash может генерировать хэши board id моделей Mac по аналогии с теми, что присутствуют в CoreAUC.framework (функция _PsZXJ2EK7ifxrtgc). Например, для Mac-F221BEC8 (MacPro5,1) это 5f571162ce99350785007863627a096bfa11c81b.  
Кажется, это хэши маков со специальными правами HDCP. Например, известно, что с Mac моделью MacPro5,1 будут работать HD фильмы на HD4000 вне зависимости от состояния декодера.

- _Как отключить PAVP/HDCP на Intel Azul (HD 4400, HD 4600) и Skylake (HD 530)?_  
Для отключения HDCP/PAVP и предотвращения зависаний рекомендуется использовать [IntelGraphicsFixup.kext](https://sourceforge.net/p/intelgraphicsfixup).


#### Проверка конфигурации

- Проблему можно решить без Shiki, если у вас имеется:
   - AMD с рабочим DRM VDA (например, HD 7870, HD 6670, HD 7970);
   - NVIDIA c рабочим DRM VDA (должны поддерживаться карты 2xx серии и старее).

  Это решение плохо изучено, но работает у нескольких людей. Shiki.kext *не* нужен.
   - IGPU устройство выставлено в приоритете в BIOS и используется фреймбуфер с конекторами (например, значение AAPL,ig-platform-id `<03 00 66 01>`) или же IGPU устройство полностью отключено и отсутствует в IOReg (см. [D2EN register](https://applelife.ru/threads/chernye-trejlery-itunes.42290/page-14#post-584519)));
   - Используется принудительный выбор декодера NVIDIA или ATI в зависимости от установленной карты;
   - VDADecoderChecker сообщает об успешном подключении к декодеру VP3 (NVIDIA) или AMD;
   - Используется Mac модель с поддержкой аппаратного ускорения.
   
- _Решение с помощью кекста для независающих карт Intel и/или любой дискретной GPU:_  
   - IGPU устройство присутствует и используется фреймбуфер без конекторов (например, `<04 00 12 04>`, `<07 00 62 01>`), в работе соло используется фреймбуфер с коннекторами;
   - Дискретная GPU правильно настроена (IOVARenderID присутствует в IOReg);
   - VDADecoderChecker показывает, что Intel Offine (Online) VGA декодер работает;
   - IMEI устройство присутствует в IOReg;
   - Не используются принудительные настройки декодера.
   - Используется Mac модель, поддерживающая аппаратное ускорение.

- _Решение с помощью кекста для Intel Azul (HD 4400, HD 4600) без дискретной GPU:_  
   - IGPU устройство присутствует и используется фреймбуфер с конекторами (например, `<03 00 22 0d>`, `<00 00 16 0a>`), есть мнение, что если карту не переименовывать в IGPU, оставив GFX0, или обозвать произвольным именем, то виснуть она будет реже;
   - Драйвер или фреймбуфер пропатчены для отключения PAVP/HDCP;
   - Аппаратный декодер видео отключён через defaults (иначе после сна видео будет зависать);
   - IMEI устройство присутствует в IOReg;
   - Используется Mac модель, поддерживающая аппаратное ускорение.

- _Решение с помощью кекста для неподдерживающихся процессоров (например, Haswell Celeron E):_  
  Аппаратное декодирование видео ускорением не работает с этими процессорами вообще, а загружаться вам нужно с подменой ID процессора.  
  Отключите IGPU полностью или переименуйте имя на произвольное нестандартное (например, IGFX), установите Shiki, и всё должно заработать.
  Не в полной мере изучено, какие настройки нужны, но известно, что отключение аппаратного ускорения с помощью редактирования plist AppleGVA или использование модели MacPro5,1 иногда позволяет смотреть HD фильмы.  

_Спасибо: 07151129, Andrey1970, Сашко666, chrome, family1232009, garcon, iDark Soul, igork, lvs1974, m-dudarev, Mieze, Quadie, savvas, tatur_sn и другим._