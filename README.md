# Modification of LTE Firmwares on smartphones

The modems integrated into today's smartphone are black boxes, only the manufacturers know their internals and have access to the code running on the integrated processor. This project contains a patching framework allowing to modify firmwares of recent Qualcomm modems in the high level C programming language. By this means, it gets possible to modify the modem's behavior and to add features. It also opens the modem as a research platform for recent mobile network technologies, removing the need for expensive special equipment in many research projects.

In addition, this project contains example applications ranging from debugging tools up to LTE MAC layer sniffing, security key extraction and access to channel estimates of the physical layer.

For more details, please read the related master thesis located in the "thesis" subfolder.

## WARNING

Our software may damage your hardware and may void your hardware's warranty! You use our tools at your own risk and responsibility! If you don't like these terms, don't use it!

## Directories

* **app**: Android application including a GUI for all example patches
* **fw_patching**: Firmware patching framework
    *  **base_fw**: Base firmware images and wrapper headers
    *  **projects**: Modem patch projects
        * **all_app**: Metaproject to combine all patches used by the app
        * **channel_estimation**: Access to LTE channel estimation matrices
        *  **common**: Code used by multiple projects (QMI communication)
        *  **func_counter_snprintf**: Counting of calls of standard C functions, forwarding of snprintf outputs
        *  **mem_access**: Direct access (read & write) to the modem memory
        *  **lte_mac**: LTE MAC Layer messages sniffing
        *  **lte_sec**: LTE security: key extraction, cryptography functions
  *  **scripts**: Scripts implementing the framework's functionality
* **kernel**: Android kernel with patches and additional modules
* **thesis**: Master thesis during which this code was developed
* **tools**: Miscellaneous additional tools/scripts

## Dependencies

* Hexagon SDK (compiler & linker): https://developer.qualcomm.com/software/hexagon-dsp-sdk/tools
* pycparser: https://github.com/eliben/pycparser
* pycparserext: https://github.com/inducer/pycparserext
* hexag00n (hexagondisasm): https://github.com/programa-stic/hexag00n/tree/master/hexagondisasm

## Contact

* Carsten Bruns <carst.bruns@gmx.de>
* [Matthias Schulz](https://seemoo.tu-darmstadt.de/mschulz) <mschulz@seemoo.tu-darmstadt.de>
