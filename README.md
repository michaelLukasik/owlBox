# owlBox
🐧 Code for the the owl box 🐧 

Inside the owl enclosure is a PIR motion sensor, set to trigger at motion inside of the box (realistically, it is sensitive enough that it will trigger if an animal even passes near the entrance). The PIR wakes the ESP32 from DeepSleep and triggers Telegram and Google Sheets API's and logs enviornment data along with a time stamp. There is a seperate module that goes inside that activates upon motion inside the enclosure as well. This module controlls a small landscaping light that alerts people to the presence of the Owls without disturbing them. Origninally, this was going to go outside on the enclosure, however the relay clicking is absurdbly loud and would likely spook the little guys. 

This was the best photo I was able to get of him just after nesting season last spring. He is bonded with a larger female who doesn't come by as often. 

![IMG_20230901_122449](https://github.com/michaelLukasik/owlBox/assets/138163589/30053887-dec5-45c2-8240-092e2bcee013)


A demo will follow once the light box is prettier. Construction on the enclosure and the indoor module ended on 9/6/2023. I still need to test battery life of both halves of the project as whatever the battery life is, it will likely get drastically reduced when cold weather comes around. Insullation for the battery may need to be provided. 

