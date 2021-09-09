Rasperry Pi Raw I²S I/O Virtual Codec 
=====================================

This project implements a simple suite of kernel modules and a device tree
overlay to enable the Raspberry Pi I²S (PCM) audio interface to be used for
both input and output with a virtual codec.

The mainline Raspberry Pi project provides integrations with specific boards
and codecs such as the HiFiBerry DAC. The `hifiberry-dac` device tree overlay
can be used as a substitude for raw I²S output, however input is not supported
by this codec.

Other supported codecs support I²S input, output and duplex streaming, but
these are more complex devices which have I²C control interfaces. Without the
real codec being present, the drivers for these devices will not register them
as available.

This project borrows heavily from the HiFiBerry DAC integration, but adds
support for input.

The project will be useful to anyone who wishes to experiment with I²S
signalling.

Installation
-----------

1. Install prerequisites:

    ```
     $ sudo apt install make gcc linux-headers device-tree-compiler
    ```

2. Build the modules and overlay:

    ```
     $ make
    ```

3. Install the modules and overlay:

    ```
     $ sudo make install
    ```

4. Enable device tree overlay.

    Open the boot configuration file in a text editor:

    ```
     $ sudo editor /boot/config.txt
    ```

    Find the line that has `dtparam=audio=on`. Disable it by putting a `#` in front.

    Then add `dtoverlay=rpi-raw-i2s-io` on the next line.

    If you have enabled any other audio-related overlays, remove them.

5. Reboot:

    ```
     $ sudo reboot
    ```

6. Check the virtual audio device is registered:

    ```
     $ aplay -l
    **** List of PLAYBACK Hardware Devices ****
    card 0: sndrpirawi2scod [snd_rpi_raw_i2s_codec], device 0: Raspberry Pi Raw I2S Codec HiFi rpi-raw-i2s-hifi-0 [Raspberry Pi Raw I2S Codec HiFi rpi-raw-i2s-hifi-0]
      Subdevices: 1/1
      Subdevice #0: subdevice #0
    ```

    ```
     $ arecord -l
    **** List of CAPTURE Hardware Devices ****
    card 0: sndrpirawi2scod [snd_rpi_raw_i2s_codec], device 0: Raspberry Pi Raw I2S Codec HiFi rpi-raw-i2s-hifi-0 [Raspberry Pi Raw I2S Codec HiFi rpi-raw-i2s-hifi-0]
      Subdevices: 1/1
      Subdevice #0: subdevice #0
    ```

Connections
-----------

By default the I²S connections on the Rasperry Pi pin-header are as follows:

Pin | Name   | Description
--- | ------ | ------------------------------------------------------
12  | `CLK`  | Clock. Single pulse for each data bit.
35  | `FS`   | Frame Select. Indicates the channel of each data word.
38  | `DIN`  | Audio data in.
40  | `DOUT` | Audio data out.

Test
----

To test the driver is working correctly, connect `DIN` and `DOUT` with a
jumper lead. This will loop the output data back to the input.

Start recording in the background:

```
 $ arecord -r 48000 -t wav test.wav &
```

Now generate some output audio:

```
 $ speaker-test -r 48000 -c2 -twav -l1
```

Now kill recording session:

```
 $ killall arecord
```

Load up the audio file in an application such as Audacity. You should have
a bit-for-bit perfect reproduction of the played audio.
