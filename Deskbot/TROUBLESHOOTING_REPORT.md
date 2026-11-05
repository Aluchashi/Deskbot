 DeskBot Troubleshooting Report

**তারিখ:** ৫ নভেম্বর ২০২৬  
**পরিধি:** Wi-Fi ও সময় sync, world clock, dashboard lag, weather API, এবং HTTP/HTTPS নিরাপত্তা। Source code পর্যালোচনা করা হয়েছে; device-এ hardware test করা হয়নি।

## পাওয়া সমস্যা ও পরিবর্তন

| বিষয় | কারণ / ফলাফল | অবস্থা |
|---|---|---|
| World clock ভুল | Germany-কে UTC+4 ধরে নেওয়া হয়েছিল, যদিও Germany-র standard time UTC+1 এবং summer-এ UTC+2। | ঠিক করা হয়েছে: সঠিক base offset ও ইউরোপের summer-time পরিবর্তন যোগ করা হয়েছে। |
| Internet ছাড়া ভুল clock | ESP32 আগে NTP sync করে চালু থাকলে internet বিচ্ছিন্ন হওয়ার পরও নিজের clock চালাতে পারে। কিন্তু power cycle-এর পর internet/NTP না পেলে clock set হয় না; আগের কোড তখনও epoch-ভিত্তিক ভুল সময় দেখাত। | ঠিক করা হয়েছে: valid time না থাকলে world-clock card-এ `NO SYNC` দেখাবে। |
| বিভিন্ন Wi-Fi-তে connect হলেও internet না পাওয়া | আগের firmware-এ নির্দিষ্ট `192.168.137.x` address ছিল। | আগের পরিবর্তনে DHCP করা হয়েছে। |
| NTP sync-এ আটকে থাকা | আগের display-তে sync ব্যর্থতার timeout/error ছিল না। | আগের পরিবর্তনে timeout message ও retry যোগ হয়েছে। |
| Weather API `401` ও dashboard lag | `401` মানে OpenWeatherMap request অনুমোদন করেনি—সাধারণত API key ভুল/অকার্যকর। `weatherLoaded` false থাকায় আগের code বারবার request করত; request চলাকালে ESP32 web server-ও অপেক্ষা করত। | আগের পরিবর্তনে `401`-এর পর পুনরাবৃত্ত request বন্ধ, UI-তে error দেখানো, এবং UI state দ্রুত বদলানোর ব্যবস্থা হয়েছে। সঠিক API key এখনও সেট করতে হবে। |

## HTTPS ও নিরাপত্তা findings

1. **Dashboard এবং API-তে HTTPS নেই।** Browser থেকে পাঠানো control ও response একই Wi-Fi-তে network traffic দেখা সম্ভব হলে plaintext-এ দেখা যেতে পারে।
2. **Dashboard/API-তে user authentication নেই।** একই LAN-এর অন্য device-ও page, alarm, notification বা Pomodoro endpoint call করতে পারে।
3. **Setup access point-এর password স্থির:** `12345678`; এটি OLED-এও দেখানো হয়। কাছাকাছি কেউ setup mode-এ ঢুকলে saved Wi-Fi/API configuration বদলাতে পারে।
4. **Weather API key plaintext HTTP URL-এ পাঠানো হয়।** Network traffic বা intermediary log-এ key প্রকাশ হওয়ার ঝুঁকি থাকে।

এগুলো code-এ নিশ্চিত পাওয়া গেছে; এগুলো **এখনও ঠিক করা হয়নি**। HTTPS যোগ করতে server-side TLS certificate ও phone trust setup লাগবে। LAN-only prototype-এ আপাতত trusted private Wi-Fi ব্যবহার করা উচিত; shared/guest/public Wi-Fi-তে dashboard ব্যবহার ঝুঁকিপূর্ণ।

## পরবর্তী করণীয়

1. সঠিক OpenWeatherMap API key Setup Portal-এ দিয়ে save/reboot করুন।
2. সংশোধিত firmware ESP32-তে upload করুন।
3. NTP sync হওয়ার পর internet বন্ধ করে world clock চালু থাকে কি না দেখুন; power cycle করে offline চালালে `NO SYNC` আসা প্রত্যাশিত, যতক্ষণ না external RTC বা internet sync যোগ করা হয়।
4. Device-টি shared network-এ ব্যবহারের আগে HTTPS, authentication, আর unique setup password যোগ করুন। Weather API endpoint-ও certificate-validated HTTPS-এ নিন।

## যাচাইয়ের সীমা

Source review-তে Germany winter/summer offsets, Singapore offset, এবং unsynced-clock fallback-এর logic পর্যালোচনা করা হয়েছে। এই workspace-এ `arduino-cli` পাওয়া যায়নি, তাই firmware compile/upload এবং বাস্তব device/network behavior যাচাই করা সম্ভব হয়নি।

Germany-র summer-time পরিবর্তনের তারিখ EU Directive 2000/84/EC অনুযায়ী: মার্চের শেষ রবিবারে শুরু, অক্টোবরের শেষ রবিবারে শেষ—দুটিই 01:00 UTC-তে। [Official EUR-Lex directive](https://eur-lex.europa.eu/legal-content/EN/TXT/?uri=CELEX%3A32000L0084)
