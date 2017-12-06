Highspeedlogger Tools:
=====================

pprz_download:
-------------

Download files from an SD-card written in RAW data format using the Paparazzi SD Direct logger. For linux, SD-card drive name can be found by calling ```ls /dev``` before and after inserting the SD-card. Common names for the SD-card drive include ```/dev/sdb``` and ```/dev/mmcblk0```. Su is required.

```
sudo ./pprz_download <DRIVE>
```

download:
--------

Download files from an SD-card written in RAW data by the *highspeedlogger* hardware.

convert:
-------

Convert *highspeedlogger* to csv


format:
------

Lowlevel RAW Format for any SD card
