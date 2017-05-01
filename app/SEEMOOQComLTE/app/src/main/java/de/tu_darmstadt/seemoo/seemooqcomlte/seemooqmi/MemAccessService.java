/**
 * memory access (read/write) service implementation
 *
 * @author Carsten Bruns (carst.bruns@gmx.de)
 */
package de.tu_darmstadt.seemoo.seemooqcomlte.seemooqmi;

import android.content.Context;

import java.util.Arrays;
import java.util.EventListener;
import java.util.EventObject;
import java.util.LinkedList;
import java.util.List;

public class MemAccessService extends SeemooQmiService {
    public static final int MEM_ACCESS_READ_SVC_ID = 0x4D454D66; //"MEMr" in ASCII
    public static final int MEM_ACCESS_WRITE_SVC_ID = 0x4D454D77; //"MEMw" in ASCII

    private List<MemAccessListener> memAccessListeners = new LinkedList<MemAccessListener>();

    /**
     * listener for incoming memory read packages for this service
     */
    private SeemooQmi.PacketListener readListener = new SeemooQmi.PacketListener() {
        @Override
        public void packetReceived(SeemooQmi.PacketReceiveEvent e) {
            byte[] data = e.getData();
            int startAddress = (int)SeemooQmi.readIntLittleEndian(data, 4);
            int length = (int)SeemooQmi.readIntLittleEndian(data, 8);
            byte[] memData = Arrays.copyOfRange(data, 12, 12 + length);

            notifyListenersRead(startAddress, length, memData);
        }
    };

    /**
     * listener for incoming memory write packages for this service
     */
    private SeemooQmi.PacketListener writeListener = new SeemooQmi.PacketListener() {
        @Override
        public void packetReceived(SeemooQmi.PacketReceiveEvent e) {
            byte[] data = e.getData();
            int startAddress = (int)SeemooQmi.readIntLittleEndian(data, 4);
            int length = (int)SeemooQmi.readIntLittleEndian(data, 8);

            notifyListenersWrite(startAddress, length);
        }
    };

    /**
     * listener for memory read results
     */
    public interface MemAccessListener extends EventListener {
        void memoryData(MemoryReadEvent e);
        void writeDone(MemoryDataEvent e);
    }

    /**
     * memory data event
     */
    public class MemoryDataEvent extends EventObject {
        private int startAddress;
        private int length;

        public MemoryDataEvent(Object source, int startAddress, int length) {
            super(source);

            this.startAddress = startAddress;
            this.length = length;
        }

        public int getStartAddress() {
            return startAddress;
        }

        public int getLength() {
            return length;
        }
    }

    /**
     * memory read event
     */
    public class MemoryReadEvent extends  MemoryDataEvent {
        private byte[] data;

        public MemoryReadEvent(Object source, int startAddress, int length, byte[] data) {
            super(source, startAddress, length);

            this.data = data;
        }

        public byte[] getData() {
            return data;
        }
    }

    public MemAccessService(SeemooQmi seemooQmiP, Context appContextP) {
        super(seemooQmiP, appContextP);
    }

    public void addListener(MemAccessListener memAccessListener) {
        if (memAccessListeners.isEmpty()) {
            //if this is the first listener attached to us, register at seemooQmi to receive messages
            seemooQmi.addPacketListener(MEM_ACCESS_READ_SVC_ID, false, readListener);
            seemooQmi.addPacketListener(MEM_ACCESS_WRITE_SVC_ID, false, writeListener);
        }
        memAccessListeners.add(memAccessListener);
    }

    public void removeListener(MemAccessListener memAccessListener) {
        memAccessListeners.remove(memAccessListener);
        if (memAccessListeners.isEmpty()) {
            //if this is the last listener attached to us, deregister at seemooQmi
            seemooQmi.removePacketListener(MEM_ACCESS_READ_SVC_ID, false, readListener);
            seemooQmi.removePacketListener(MEM_ACCESS_WRITE_SVC_ID, false, writeListener);
        }
    }

    private void notifyListenersRead(int startAddress, int length, byte[] data) {
        for (MemAccessListener mrl : memAccessListeners) {
            mrl.memoryData(new MemoryReadEvent(this, startAddress, length, data));
        }
    }

    private void notifyListenersWrite(int startAddress, int length) {
        for (MemAccessListener mrl : memAccessListeners) {
            mrl.writeDone(new MemoryDataEvent(this, startAddress, length));
        }
    }

    /**
     * sends a QMI message to request for a memory read
     *
     * @param startAddress first address to read
     * @param length length of data to read in bytes, maximal 8180
     */
    public void readMemory(int startAddress, int length) {
        byte[] data = new byte[8];
        SeemooQmi.writeIntLittleEndian(startAddress, data, 0);
        SeemooQmi.writeIntLittleEndian(length, data, 4);
        seemooQmi.sendMessage(MEM_ACCESS_READ_SVC_ID, false, data, 8);
    }

    /**
     * sends a QMI message to request for a memory write
     *
     * @param startAddress first address to write
     * @param data data to write
     * @param length length of data to write in bytes, maximal 8180, if 0 the data array length is used instead
     */
    public void writeMemory(int startAddress, byte[] data, int length) {
        if (length == 0) {
            length = data.length;
        }
        if (length > (SeemooQmi.MAX_PACKAGE_LENGTH - 12)) {
            length = SeemooQmi.MAX_PACKAGE_LENGTH - 12;
        }

        byte[] dataQmi = new byte[8 + length];
        SeemooQmi.writeIntLittleEndian(startAddress, dataQmi, 0);
        SeemooQmi.writeIntLittleEndian(length, dataQmi, 4);
        System.arraycopy(data, 0, dataQmi, 8, length);

        seemooQmi.sendMessage(MEM_ACCESS_WRITE_SVC_ID, false, dataQmi, 8 + length);
    }

    @Override
    public void finalizeService() {

    }
}
