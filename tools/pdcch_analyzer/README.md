# PDCCH dump analyzer

In this directory you find the code to analyze PDCCH dumps created by the Android App also available in this repository, together with the corresponding **pdcch_dump** modem firmware patch.

## WARNING

Our software may damage your hardware and may void your hardware's warranty! You use our tools at your own risk and responsibility! If you don't like these terms, don't use it!

## Directories

* **dump_analyze**: Tool performing the actual analysis of DCIs, outputs results as *CSV*
* **dump_decode**: Decoding tool to convert raw dumps into pre-decoded ones
* **dump_record_print**: Tool printing information about all records found in dumps
* **pdcch_analyzer_lib**: Library used by the tools, implements dump format reading and PDCCH decoder
* **plotting**: Gnuplot scripts for data visualization

## Data collection

This analyzer code operates on dumps collected with the Android App found in this repository. In order to collect dumps, you have to build and install on your device:
* modem firmware with *pdcch_dump* patch
* custom kernel including the *seemoo_qmi_client* kernel module
* Android App

In this readme, we focus on how to build and use the analyzer code. For a description of how to build and install the patched modem firmware, kernel module and Android App, refer to the general readme in the root directory of this repository.

Once all these components are installed, you can start and control dumping of the PDCCH from the GUI of the App.

## Build instructions

There are two options to build the tools:
* Each tool folder (*dump_analyze*, *dump_decode*, *dump_record_print*) and the library (*pdcch_analyzer_lib*) contains an Eclipse project. Therefore, you can import them in this IDE and build from there.
* Each tool also contains two subfolders *Debug* and *Release* with makefiles such that it can be build using *make*. Therefore, simply go to the right configuration subfolder of the tool you want to build and type `make all`. Before building the tools, you have to build the library (*pdcch_analyzer_lib*) in the same way, in the same configuration (Debug/Release) as you want to build the tool.

## Usage

Each of the tools can operate either on "raw" dumps as they are recoreded by the App or on pre-decoded files, which contains only DCIs and not the complete PDCCH buffer values anymore. In most cases, expecially when multiple analyses are expected, it is recommended to run the slow decoding step only once, i.e. first convert the "raw" dump into a pre-decoded one by decoding it with the *dump_decode* tool. The resulting output file can then be used as input to the other tools.
A common tool flow would thus be (with example filenames used again in the individual tool descriptions):
1. Collect dump with the App (and patched modem firmware) -> pdcch.bin
2. Decode dump with *dump_decode* -> pdcch_dec.bin
3. Run analysis on pre-decoded dump with *dump_analyze* -> data.csv
4. Use gnuplot to visualize the result -> graph.svg

### dump_decode
Decodes "raw" dumps and outputs the result (DCI data and metadata) to a new file.
Usage:
```
./dump_decode [-nocompress] DUMP_FILE_PATH DUMP_NAME [OUTPUT_PATH] OUTPUT_BASE_NAME
        -nocompress: if present, the output will not be gzip compressed
        DUMP_FILE_PATH: path to the input dump file
        DUMP_NAME: name of the input file (without .binX file extension)
        OUTPUT_PATH (optional): path to the output dump file, if not provided, DUMP_FILE_PATH is used
        OUTPUT_BASE_NAME: desired name of the output file (without .binX file extension)
```
Example:
```
./pdcch_decode ./dumps/ pdcch pdcch_dec
```

### dump_analyze
Runs different analysis on the dump (DCI data) and outputs the results as *CSV*. 
Usage:
```
./dump_analyze DUMP_FILE_PATH DUMP_NAME
	DUMP_FILE_PATH: path to the input dump file
	DUMP_NAME: name of the input file (without .binX file extension)
	CFG_FILE: analysis configuration file
```
Example:
```
./dump_analyze ./dumps/ pdcch_dec ../config/default.awc
```
The configuration file specifies the analyses which should be performed and how the tool should output the results. To understand this better, please have a look at an example configuration file provided in the *config* subfolder.
In general, you can define different *analyzers* with parameters here and connect them to *writers* which store the results to files under certain conditions, e.g. the average of the data of the 10 subframes of an LTE frame.

### dump_record_print
Prints information about all records in a dump file, mainly for debugging.
Usage:
```
./dump_record_print DUMP_FILE_PATH DUMP_NAME
	DUMP_FILE_PATH: path to the input dump file
	DUMP_NAME: name of the input file (without .binX file extension)
```
Example:
```
./dump_record_print ./dumps/ pdcch_dec
```

### Data plotting
The dump processing tools convert the data to *CSV* files. These can easily be plotted using common tools, e.g. gnuplot. The *plotting* subfolder contains some example gnuplot scripts for this, their usage is documented in the header of each script. This step highly depends on your aim and on which analyses where performed in the analysis step before.

## Contact

* Carsten Bruns <carst.bruns@gmx.de>
* [Matthias Schulz](https://seemoo.tu-darmstadt.de/mschulz) <mschulz@seemoo.tu-darmstadt.de>
* [Arash Asadi](https://www.seemoo.tu-darmstadt.de/team/arash-asadi) <aasadi@seemoo.tu-darmstadt.de>

