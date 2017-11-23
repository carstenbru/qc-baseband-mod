# Modification of LTE Firmwares on smartphones

The modems integrated into today's smartphone are black boxes, only the manufacturers know their internals and have access to the code running on the integrated processor. This project contains a patching framework allowing to modify firmwares of recent Qualcomm modems in the high level C programming language. By this means, it gets possible to modify the modem's behavior and to add features. It also opens the modem as a research platform for recent mobile network technologies, removing the need for expensive special equipment in many research projects.

In addition, this project contains example applications ranging from debugging tools up to LTE MAC layer sniffing, security key extraction and access to channel estimates of the physical layer.

For more details, please read the related master thesis located in the "thesis" subfolder.

## WARNING

Our software may damage your hardware and may void your hardware's warranty! You use our tools at your own risk and responsibility! If you don't like these terms, don't use it!

## Supported phones

* Asus PadFone Infinity 2 (A86)
* Xiaomi Mi4 LTE (CU)

## Directories

* **app**: Android application including a GUI for all example patches
* **fw_patching**: Firmware patching framework
    *  **base_fw**: Base firmware images and wrapper headers
    *  **projects**: Modem patch projects
        *  **all_app**: Metaproject to combine all patches used by the app
        *  **channel_estimation**: Access to LTE channel estimation matrices
        *  **common**: Code used by multiple projects (QMI communication)
        *  **func_counter_snprintf**: Counting of calls of standard C functions, forwarding of snprintf outputs
        *  **lte_mac**: LTE MAC Layer messages sniffing
        *  **lte_sec**: LTE security: key extraction, cryptography functions
        *  **mem_access**: Direct access (read & write) to the modem memory
        *  **pdcch_dump**: LTE PDCCH dumping, see also *tools/pdcch_analyzer* directory
        *  **version_string**: Only modification of the firmware version string, no code patches
  *  **scripts**: Scripts implementing the framework's functionality
* **kernel**: Android kernel with patches and additional modules
* **thesis**: Master thesis during which this code was developed
* **tools**: Miscellaneous additional tools/scripts
    *  **pdcch_analyzer**: PDCCH dump analysis tool (C++)

## Dependencies
Our framework requires a set of other tools to work:
* Hexagon SDK (compiler & linker): https://developer.qualcomm.com/software/hexagon-dsp-sdk/tools
* pycparser: https://github.com/eliben/pycparser
* pycparserext: https://github.com/inducer/pycparserext
* hexag00n (hexagondisasm): https://github.com/programa-stic/hexag00n/tree/master/hexagondisasm
* Basic Android tools: *adb*, *fastboot*
* Other tools: *git*, *python*, *mtools*, *rsync*, *sed*, *patch*, *python-setuptools*

## Build instructions
The main component of this project is the modem firmware patching. However, the example projects use an Android application to interact with the user and to display data. In order to enable communication between the Modem and the Android application, a kernel module is neccessary and, therefore, also a custom Android kernel for the target phone needs to be build.

### Install dependencies
1. Install linux and Android tools : `sudo apt-get install adb fastboot git python mtools rsync sed patch python-setuptools`
2. Download and install the Hexagon SDK from https://developer.qualcomm.com/download/hexagon/hexagon-sdk-linux.bin
    * The framework expects the Hexagon tools on the environment executable PATH, thus you need to add it there (adapt the path!): `export PATH=$PATH:~/Qualcomm/HEXAGON_Tools/6.4.06/gnu/bin/:~/Qualcomm/HEXAGON_Tools/6.4.06/qc/bin/`
    * We recommend to also add this line to your *~/.bashrc* file to make it permanently
3. Download *pycparser* and *pycparserext* somewhere to your computer and install them:
    ```
    git clone https://github.com/eliben/pycparser.git
    cd pyparser
    sudo python setup.py install
    cd ..
    git clone https://github.com/inducer/pycparserext.git
    cd pyparserext
    sudo python setup.py install
    ```
4. Install *hexag00n*:
    ```
    git clone https://github.com/programa-stic/hexag00n.git
    cd hexag00n
    sudo python setup.py install
    ```

### Modem firmware
1. Go to the folder of the patch project in *fw_patching/projects*, e.g. `cd fw_patching/projects/channel_estimation`
2. In case you want to compile for another target than the default (Xiaomi Mi4) adapt the *BASE_FW* variable in the projects Makefile accordingly
3. Start the build: `make`
4. If everything went well, a patched modem firmware (*image/patched_modem.img*) is generated and can be flashed to the target device
    * For the *Xiaomi Mi4*, you can use fastboot to do so. To start the device in *fastboot* mode, turn it off and press the *Volume_Down* and power buttons simultaneously
    * Now actually flash the modem firmware: `fastboot flash modem+modem1 image/patched_modem.img`

### Android Kernel
A kernel prepared with all needed patches is located in the *kernel* subdirectory. To build it do the following in a terminal in the repositories root directory:
1. Go to the correct kernel subfolder, e.g. `cd kernel/xiaomi_mi4`
2. Setup the build environment `source setup_env.sh`
3. Start the build `make`. Grab a coffee as this will take some time.
4. The kernel is now ready to be installed, you can use fastboot to do so:
    * If you want to install it permanently: `fastboot flash boot+boot1 boot.img`
    * If you just want to boot it once: `fastboot boot boot.img`

### Android App
The Android App is provided as an *Android Studio* project, simply open, build and install it.

## Contact

* Carsten Bruns <carst.bruns@gmx.de>
* [Matthias Schulz](https://seemoo.tu-darmstadt.de/mschulz) <mschulz@seemoo.tu-darmstadt.de>
