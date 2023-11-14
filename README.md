# OtterTuner-MCU

# Getting Started

## Manual Toolchain Setup

Instructions are pulled from the [Espressif documentation](https://docs.espressif.com/projects/esp-idf/en/latest/esp32/get-started/linux-macos-setup.html).

### Installing requirements
For Linux users:

Ubuntu & Debian:
```shell
sudo apt-get install git wget flex bison gperf python3 python3-pip python3-venv cmake ninja-build ccache libffi-dev libssl-dev dfu-util libusb-1.0-0
```

For MacOS users:

using Homebrew:
```shell
brew install cmake ninja dfu-util
brew install ccache
```

### Installing and setting up ESP-IDF

Clone the ESP repo:
```shell
mkdir -p ~/esp
cd ~/esp
git clone --recursive https://github.com/espressif/esp-idf.git
```

Installing ESP toolchain
```shell
cd ~/esp/esp-idf
./install.sh esp32
```

setting up environment variables:
```shell
. $HOME/esp/esp-idf/export.sh
```

copy and paste alias into your shell profile (`.zshrc`, `.bashrc` or whatever else you use), then refresh your terminal session:
```shell
alias get_idf='. $HOME/esp/esp-idf/export.sh'
source ~/.zshrc
```

### Configuring the Project

Set the target with:
```shell
idf.py set-target esp32s3
idf.py menuconfig
```

### Building
Run
```shell
idf.py build
```

The output should look like:
```shell
$ idf.py build
Running cmake in directory /path/to/hello_world/build
Executing "cmake -G Ninja --warn-uninitialized /path/to/hello_world"...
Warn about uninitialized values.
-- Found Git: /usr/bin/git (found version "2.17.0")
-- Building empty aws_iot component due to configuration
-- Component names: ...
-- Component paths: ...

... (more lines of build system output)

[527/527] Generating hello_world.bin
esptool.py v2.3.1

Project build complete. To flash, run this command:
../../../components/esptool_py/esptool/esptool.py -p (PORT) -b 921600 write_flash --flash_mode dio --flash_size detect --flash_freq 40m 0x10000 build/hello_world.bin  build 0x1000 build/bootloader/bootloader.bin 0x8000 build/partition_table/partition-table.bin
or run 'idf.py -p PORT flash'
```

### Flashing onto the ESP32-S3
```shell
idf.py -p PORT flash
```

NOTE: You may need to press the RST button on the ESP32

### Monitoring the output
```shell
idf.py -p PORT monitor
```

To exit, use `Ctrl+]`

To combine building, flashing and monitoring, use:
```shell
idf.py -p PORT flash monitor
```