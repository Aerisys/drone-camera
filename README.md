# Drone - Camera

## Installation
Clone the project
Use platformio in vscode and open the project<br><br>

Create a file named platformio.ini.local<br>
Copy this lines in this new file:
```
[env:seeed_xiao_esp32s3]
build_flags = 
    -DSSID_WIFI=\"SSID\"
    -DPWD_WIFI=\"PASSWORD\"
```

Replace SSID and PASSWORD with your Wi Fi crendentials<br>
Build it to the camera<br><br>

The IP adresse is print in the console, connect to it with endpoint /stream like:<br>
http://192.168.0.1/stream<br>