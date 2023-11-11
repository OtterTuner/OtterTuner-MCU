# OtterTuner-MCU

# Getting Started

## Manual Toolchain Setup

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

