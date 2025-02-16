# BLE-YC01_ESP32
I was looking for a way to Access my BLE-YC01 Pool-Monitor without the Yinmik-App.
I'm sharing this mainly because it took me quite a while to gather all the information needed. So don't laugh about my programming style :-)
Most of the conversation stuff was done by the HomeAssistant and some other communities (great thanks to them!), 
but I wanted a standalone solution since there's no HomeAssistant near my Swimmingpool.

Why I did that? For 2 reasons:
 - to extend the signal range by using WiFi instead of BLE
 - to be able to enter the data into InfluxDB in order to get some graphics out of it
   that might help me to completely understand the relation of the different values

Just another hint: It's fairly easy to get the Bytes from the device by using gatttool:
  gatttool -b C0:00:00:XX:XX:XX --char-read -a 0x000e
but you'll get something like
  ff a1 fe fa fc 09 ff 03 fd 01 fe ef ff c7 fd b8 fa b8 2f fc 
from that and have to do some wired stuff to get the desired values from that.
So I ended up asking Bing-AI to convert the Python-Skript from jdeath (https://github.com/jdeath/BLE-YC01/blob/main/custom_components/ble_yc01/BLE_YC01/parser.py  --  thanks again!) 
to Arduino code.

If you have Ideas on how to improve this, feel free to contact me. Otherwise just have fun with it.
