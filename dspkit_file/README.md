# ssy130-files-dspkit


## Contents

SSY130 projects associated files, and reference code-base for the DSP-KIT.
See user_guide.pdf for DSP-KIT installation and use guidelines.

The easiest way to import the files to your computer is using VSCode "Clone Git Repository..." functionality if you have git installed.

## Note for use with Windows
**Before** connecting the dspkit to the USB-port on your computer you must download and install this [driver](https://www.st.com/en/development-tools/stsw-link009.html). 
You install the driver by running `stlink_winusb_install.bat` in "Command prompt" with Administrator rights. 

Windows 10: Opening the Command Prompt as Administrator
1. Press the Windows Start button at the bottom left.
2. Type in "Command Prompt".
3. Right click on Command Prompt and click "Run as administrator".
4. Click Yes if the Windows 10 User Account Control prompt is displayed.
5. The Command Prompt should appear.
6. Change directory to where you unpacked the software
7. Start the `stlink_winusb_install.bat` script.

## Note for use with Linux
The following commands needs to be issued in a Linux environment (tested on a Debian system)
```
sudo apt install python3-venv
sudo apt -y install stlink-tools
sudo apt install curl
sudo curl -fsSL https://raw.githubusercontent.com/platformio/platformio-core/develop/platformio/assets/system/99-platformio-udev.rules | sudo tee /etc/udev/rules.d/99-platformio-udev.rules > /dev/null~
sudo service udev restart
```
