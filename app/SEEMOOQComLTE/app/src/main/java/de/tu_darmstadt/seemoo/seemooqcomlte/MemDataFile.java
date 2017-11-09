/**
 * Class to store and load memory data in various binary formats
 *
 * @author Carsten Bruns (carst.bruns@gmx.de)
 */

package de.tu_darmstadt.seemoo.seemooqcomlte;

import android.content.Context;
import android.net.Uri;

import java.io.BufferedReader;
import java.io.File;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.InputStream;
import java.io.InputStreamReader;
import java.util.Arrays;

public class MemDataFile {
    private int startAddress;
    private byte[] data;
    private String fileName;

    /**
     * constructor, used when data is known (e.g. to store read data later)
     *
     * @param startAddress first address in memory of the data bytes
     * @param data actual data bytes
     */
    public MemDataFile(int startAddress, byte[] data) {
        this.startAddress = startAddress;
        this.data = data;
    }

    /**
     * constructor, used when only length is known for now (e.g. bulk read)
     *
     * @param startAddress  first address in memory of the data bytes
     * @param length        final lenght of data
     */
    public MemDataFile(int startAddress, int length) {
        this.startAddress = startAddress;
        this.data = new byte[length];
    }

    /**
     * loads a MemDataFile from a stored .bin file
     *
     * @param context app context
     * @param uri file identifier
     * @param startAddress first address in memory of the data contained in the file
     * @return the loaded MemDataFile object
     */
    private static MemDataFile createMemDataFileBin(Context context, Uri uri, int startAddress) {
        try {
            InputStream iss = context.getContentResolver().openInputStream(uri);
            byte[] bytes = new byte[iss.available()];
            iss.read(bytes);

            return new MemDataFile(startAddress, bytes);
        } catch (IOException ioe) {
            return null;
        }
    }

    /**
     * loads a MemDataFile from a stored .ihex file
     *
     * @param context app context
     * @param uri file identifier
     * @return the loaded MemDataFile object
     */
    private static MemDataFile createMemDataFileIHex(Context context, Uri uri) {
        boolean startAddressHighSet = false;
        boolean startAddressLowSet = false;
        int startAddress = 0;

        try {
            InputStream iss = context.getContentResolver().openInputStream(uri);
            byte[] bytes = new byte[iss.available() / 2];

            BufferedReader br = new BufferedReader(new InputStreamReader(iss));

            int readBytes = 0;
            String line;
            boolean gotEndRecord = false;
            while (((line = br.readLine()) != null) && !gotEndRecord) { //parse ihex records
                line.trim();
                if (line.startsWith(":")) {
                    int type = Integer.parseInt(line.substring(7,9), 16);
                    switch (type) {
                        case 0: //normal data record
                            if (!startAddressLowSet) {
                                startAddress |= (Integer.parseInt(line.substring(3,7), 16) & 0xFFFF);
                                startAddressLowSet = true;
                            }
                            int length = Integer.parseInt(line.substring(1,3), 16);

                            for (int i = 0; i < length; i++) {
                                bytes[readBytes++] = (byte)Integer.parseInt(line.substring(9 + i*2, 11 + i*2), 16);
                            }
                            break;
                        case 1: //end record
                            gotEndRecord = true;
                            break;
                        case 2: //high address record
                            if (!startAddressHighSet) {
                                startAddress = Integer.parseInt(line.substring(9,13), 16) << 16;
                                startAddressHighSet = true;
                            }
                            break;
                    }
                }
             }

            return new MemDataFile(startAddress, Arrays.copyOfRange(bytes, 0, readBytes));
        } catch (IOException ioe) {
            return null;
        }
    }

    /**
     * loads a MemDataFile (in ihex or bin)
     *
     * @param context app context
     * @param uri file identifier
     * @param startAddress first address in memory of the data contained in the file
     * @return the loaded MemDataFile object
     */
    public static MemDataFile createMemDataFile(Context context, Uri uri, int startAddress) {
        String uriString  = uri.toString();
        if ((uriString.contains(".ihex")) || (uriString.contains(".hex"))) {
            return createMemDataFileIHex(context, uri);
        } else {
            return createMemDataFileBin(context, uri, startAddress);
        }
    }

    /**
     * writes the data contained in the object in an ihex format
     *
     * @param stream target stream
     * @throws IOException
     */
    private void writeIHexFile(FileOutputStream stream) throws IOException {
        long curAddr = (long)startAddress;
        long addrHighBase = 0;


        int dataPos = 0;
        while (dataPos < data.length) {
            if ((curAddr & (long) 0xFFFF0000) != addrHighBase) { //check if we have to write a new high address record
                int checksum = 4 + (int) ((curAddr >> 16) & 0xFF) + (int) ((curAddr >> 24) & 0xFF);
                checksum = ~(checksum) + 1;
                stream.write(String.format(":02000002%1$04X%2$02X\n", (curAddr >> 16) & 0xFFFF, checksum & 0xFF).getBytes());

                addrHighBase = curAddr & (long) 0xFFFF0000;
            }

            int remaining = data.length - dataPos;
            int writeSize = (remaining < 16) ? remaining : 16;

            stream.write(String.format(":%1$02X%2$04X00", writeSize & 0xFF, (curAddr & 0xFFFF)).getBytes());
            int checksum = writeSize + (int) ((curAddr) & 0xFF) + (int) ((curAddr >> 8) & 0xFF);;

            while (writeSize != 0) {
                stream.write(String.format("%1$02X", data[dataPos]).getBytes());
                checksum += data[dataPos];

                dataPos++;
                curAddr++;
                writeSize--;
            }

            checksum = ~(checksum) + 1;
            stream.write(String.format("%1$02X\n", checksum & 0xFF).getBytes());
        }

        stream.write(":00000001FF\n".getBytes()); //write end of file record
    }

    /**
     * writes the data contained in the object into a file
     *
     * @param path path of the destination file
     * @param fileName destination file name
     * @param iHex true to write in ihex format, false to write in bin format
     * @return true on success, false on errors
     */
    private boolean writeToFile(File path, String fileName, boolean iHex) {
        File file = new File(path, fileName);

        boolean success = true;
        try {
            path.mkdirs();
            file.createNewFile();
            FileOutputStream stream = new FileOutputStream(file);

            if (!iHex) {
                stream.write(data);
            } else {
                writeIHexFile(stream);
            }

            stream.close();
            this.fileName = file.toString();
        } catch (IOException ioe) {
            success = false;
        }

        return success;
    }

    /**
     * writes the data contained in the object into a file in bin format
     *
     * @param path path of the destination file
     * @param fileName destination file name
     * @return true on success, false on errors
     */
    public boolean writeToBinFile(File path, String fileName) {
        return writeToFile(path, fileName, false);
    }

    /**
     * writes the data contained in the object into a file in ihex format
     *
     * @param path path of the destination file
     * @param fileName destination file name
     * @return true on success, false on errors
     */
    public boolean writeToIHexFile(File path, String fileName) {
        return writeToFile(path, fileName, true);
    }

    /**
     * updates data in the object
     *
     * @param dataAddress first address (absolut) of the new data
     * @param newData  actual data bytes
     */
    public void putData(int dataAddress, byte[] newData) {
        if ((dataAddress >= startAddress) && (dataAddress + newData.length <= startAddress + data.length)) {
            System.arraycopy(newData, 0, data, dataAddress - startAddress, newData.length);
        }
    }

    public String getFileName() {
        return fileName;
    }

    public int getStartAddress() {
        return startAddress;
    }

    public byte[] getData() {
        return data;
    }
}
