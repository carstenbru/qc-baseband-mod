/**
 * service for PDCCH dump messages
 *
 * @author Carsten Bruns (carst.bruns@gmx.de)
 */
package de.tu_darmstadt.seemoo.seemooqcomlte.seemooqmi;

import android.content.Context;

import java.util.EventListener;
import java.util.EventObject;
import java.util.LinkedList;
import java.util.List;

import de.tu_darmstadt.seemoo.seemooqcomlte.R;

public class PdcchDumpService extends SeemooQmiService {
    public static final int PDCCH_DUMP_SVC_ID = 0x43434864; //"CCHd" in ASCII
    public static final int PDCCH_CELL_INFO_SVC_ID = 0x43434869; //"CCHi" in ASCII

    private List<PdcchDumpListener> pdcchDumpListeners = new LinkedList<PdcchDumpListener>();

    private boolean isRegistered = false;
    private boolean isCellInfoRequest = false;

    /**
     * listener for new PDCCH dump messages
     */
    public interface PdcchDumpListener extends EventListener {
        public void newDumpData(PdcchDumpEvent e);
        public void newCellInfo(PdcchCellInfoEvent e);
    }

    /**
     * event for an incoming PDCCH dump message
     */
    public class PdcchDumpEvent extends EventObject {
        private byte[] data;
        private int dataLength;

        public PdcchDumpEvent(Object source, byte[] data, int dataLength) {
            super(source);
            this.data = data;
            this.dataLength = dataLength;
        }

        public byte[] getData() {
            return data;
        }
        public int getDataLength() { return dataLength; }
    }

    /**
     * event for an incoming PDCCH cell info message
     */
    public class PdcchCellInfoEvent extends EventObject {
        private byte[] data;
        private int dataLength;

        public PdcchCellInfoEvent(Object source, byte[] data, int dataLength) {
            super(source);
            this.data = data;
            this.dataLength = dataLength;
        }

        public byte[] getData() {
            return data;
        }
        public int getDataLength() { return dataLength; }
    }

    /**
     * constructor
     *
     * @param seemooQmiP instance of SeemooQmi
     * @param appContextP application context
     */
    public PdcchDumpService(SeemooQmi seemooQmiP, Context appContextP) {
        super(seemooQmiP, appContextP);

        //add listener for service registration change responses
        seemooQmi.addPacketListener(PDCCH_DUMP_SVC_ID, false, new SeemooQmi.PacketListener() {
            @Override
            public void packetReceived(SeemooQmi.PacketReceiveEvent e) {
                if (e.getDataLength() == 8) {
                    seemooQmi.notifyStatusListeners(appContext.getResources().getString(R.string.pdcch_dump_reg_success), 4);
                } else {
                    seemooQmi.notifyStatusListeners(appContext.getResources().getString(R.string.pdcch_dump_dereg_success), 4);
                }
            }
        });
        //add listener for dump data indication messages
        seemooQmi.addPacketListener(PDCCH_DUMP_SVC_ID, true, new SeemooQmi.PacketListener() {
            @Override
            public void packetReceived(SeemooQmi.PacketReceiveEvent e) {
                notifyPdcchDumpListenersDumpData(e.getData(), e.getDataLength());
            }
        });

        //add listener for service registration change responses
        seemooQmi.addPacketListener(PDCCH_CELL_INFO_SVC_ID, false, new SeemooQmi.PacketListener() {
            @Override
            public void packetReceived(SeemooQmi.PacketReceiveEvent e) {
                if (e.getDataLength() == 8) {
                    if (!isCellInfoRequest) {
                        seemooQmi.notifyStatusListeners(appContext.getResources().getString(R.string.pdcch_cell_info_reg_success), 4);
                    }
                } else {
                    seemooQmi.notifyStatusListeners(appContext.getResources().getString(R.string.pdcch_cell_info_dereg_success), 4);
                }
            }
        });
        //add listener for cell info indication messages
        seemooQmi.addPacketListener(PDCCH_CELL_INFO_SVC_ID, true, new SeemooQmi.PacketListener() {
            @Override
            public void packetReceived(SeemooQmi.PacketReceiveEvent e) {
                notifyPdcchDumpListenersCellInfo(e.getData(), e.getDataLength());
            }
        });
    }

    public void addListener(PdcchDumpListener pdcchDumpListener) {
        pdcchDumpListeners.add(pdcchDumpListener);
    }

    public void removeListener(PdcchDumpListener pdcchDumpListener) {
        pdcchDumpListeners.remove(pdcchDumpListener);
    }

    /**
     * send message to all listeners
     *
     * @param data data to send
     */
    private void notifyPdcchDumpListenersDumpData(byte[] data, int dataLength) {
        //send message to all listeners
        if (!pdcchDumpListeners.isEmpty()) {
            for (PdcchDumpListener dl : pdcchDumpListeners) {
                dl.newDumpData(new PdcchDumpEvent(this, data, dataLength));
            }
        }
    }

    /**
     * send message to all listeners
     *
     * @param data data to send
     */
    private void notifyPdcchDumpListenersCellInfo(byte[] data, int dataLength) {
        //send message to all listeners
        if (!pdcchDumpListeners.isEmpty()) {
            for (PdcchDumpListener dl : pdcchDumpListeners) {
                dl.newCellInfo(new PdcchCellInfoEvent(this, data, dataLength));
            }
        }
    }

    /**
     * register/deregister for indications in modem
     *
     * @param state new registration state
     */
    public void register(boolean state) {
        isRegistered = state;
        isCellInfoRequest = false;

        byte register[] = {(byte)(state ? 1 : 0)};
        seemooQmi.sendMessage(PDCCH_DUMP_SVC_ID, true, register, 1);
        seemooQmi.sendMessage(PDCCH_CELL_INFO_SVC_ID, true, register, 1);
    }

    /**
     * requests a cell info update
     */
    public void requestCellInfo() {
        if (isRegistered) {
            isCellInfoRequest = true;
            byte register[] = {(byte) (1)};
            seemooQmi.sendMessage(PDCCH_CELL_INFO_SVC_ID, true, register, 1);
        }
    }

    @Override
    public void finalizeService() {
        register(false);
    }
}
