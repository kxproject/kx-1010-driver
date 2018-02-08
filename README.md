kx-1010-driver
===============

This is an alternative audio driver for Windows for E-mu / Creative Labs 1010 PCI/PCIe audio devices.
It supports sampling rates up to 192kHz.
This driver is NOT kX-compatible, and does not support custom DSP microcode, effects etc., unless they are compiled into the driver.

ASIO component is not available for this driver, but generic ASIO4ALL drivers should work. It should be possible to implement faster ASIO DLLs by accessing private API exposed by the driver into user space.

This is WaveRT-based driver, that's why it is trivial to implement WaveRT-compliant ASIO client DLL.

This driver requires no extra build tools, just use Windows DDK (earlier msbuild-based version for e.g. Windows 7.1)
The code is EXPERIMENTAL, use on your own risk.

You will need to create a simple .INF file for the driver to install properly. Please review Microsoft documentation for details on portcls/WaveRT drivers.
