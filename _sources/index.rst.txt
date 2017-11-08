
High-Speed SD Logger
====================

.. toctree::
   :maxdepth: 2

   sw/README


.. image:: https://travis-ci.org/tudelft/highspeedlogger.png?branch=master
   :target: https://travis-ci.org/tudelft/highspeedlogger
   :alt: Travis Build

The High-Speed SD Logger Project consists of 2 parts: 

 - a) dedicated hardware project using an *STM32F4* with a custom 2 layer PCB + the firmware and required tools
 - b) tools to read raw data from an **SD** card connected directly to a https://github.com/paparazzi/paparazzi autopilot

.. note:: Do not mix up download and download_pprz

Folders:

 - hw -> Hardware of a)
 - sw -> Logger Firmware of a)
 - tools -> PC tools for download and conversion of a) and b)

Can for instance be used together with paparazzi-autopilot logging module:

a) 
https://github.com/paparazzi/paparazzi/blob/master/conf/modules/logger_spi_link.xml

b) 
https://github.com/paparazzi/paparazzi/blob/master/conf/modules/logger_sd_spi_direct.xml


Build with::

   make

	"A Quote"

.. _GitHub: https://github.com/tudelft/highspeedlogger
