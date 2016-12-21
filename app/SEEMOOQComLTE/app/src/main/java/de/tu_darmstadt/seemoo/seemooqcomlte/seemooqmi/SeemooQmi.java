/**
 * Seemoo QMI device interface
 *
 * use this class to receive and send QMI packets
 *
 * @author Carsten Bruns (carst.bruns@gmx.de)
 */
package de.tu_darmstadt.seemoo.seemooqcomlte.seemooqmi;

import android.content.Context;
import android.os.Handler;

import java.io.BufferedReader;
import java.io.File;
import java.io.FileInputStream;
import java.io.FileOutputStream;
import java.io.InputStream;
import java.io.InputStreamReader;
import java.util.EventListener;
import java.util.EventObject;
import java.util.HashMap;
import java.util.LinkedList;
import java.util.List;
import java.util.Map;

import de.tu_darmstadt.seemoo.seemooqcomlte.R;

public class SeemooQmi {
    private static SeemooQmi singletonInstance = null;

    private static final String STATUS_FILE = "/dev/seemoo_qmi_status";
    private static final String DATA_FILE = "/dev/seemoo_qmi_data";
    private static final int MAX_PACKAGE_LENGTH = 8192;

    private Map<Integer, List<PacketListener>> packetListeners = new HashMap<Integer, List<PacketListener>>();
    private Map<StatusListener, Integer> statusListeners = new HashMap<StatusListener, Integer>();

    private Handler pollHandler = new Handler();
    private int pollRate = 100;
    private boolean stopPolling = false;

    private Context appContext;

    private List<String> oldStatusMessages = new LinkedList<String>();
    private int oldStatusMessagesMask;
    private int maxOldMessagesSize = 1024;

    private List<SeemooQmiService> services = new LinkedList<SeemooQmiService>();

    /**
     * runnable to handle polling od messages from the kernel driver
     */
    private Runnable pollRunnable = new Runnable() {
        @Override
        public void run() {
            readStatus();
            if (!packetListeners.isEmpty()) {
                readMessages();
            }
            if (!stopPolling) {
                pollHandler.postDelayed(this, pollRate);
            }
        }
    };

    public void setMaxOldMessagesSize(int maxOldMessagesSize) {
        this.maxOldMessagesSize = maxOldMessagesSize;
    }

    public void setOldStatusMessagesMask(int oldStatusMessagesMask) {
        this.oldStatusMessagesMask = oldStatusMessagesMask;
    }

    /**
     * interface for a listener for status messages
     */
    public interface StatusListener extends EventListener {
        void statusUpdate(StatusUpdateEvent e);
    }

    /**
     * interface for a listener for incoming QMI packages
     */
    public interface PacketListener extends EventListener {
        void packetReceived(PacketReceiveEvent e);
    }

    /**
     * new status message event
     */
    public class StatusUpdateEvent extends EventObject {
        private String status;

        public StatusUpdateEvent(Object source, String status) {
            super(source);
            this.status = status;
        }

        public String getStatus() {
            return status;
        }
    }

    /**
     * QMI packet receive event
     */
    public class PacketReceiveEvent extends EventObject {
        private byte data[];
        private int dataLength;

        public PacketReceiveEvent(Object source, byte[] data, int dataLength) {
            super(source);
            this.data = data;
            this.dataLength = dataLength;
        }

        public int getDataLength() {
            return dataLength;
        }

        public byte[] getData() {
            return data;
        }
    }

    /**
     * reads an integer (32bit unsigned) from a byte array as little endian
     * (byte order e.g. on hexagon used in modem)
     *
     * @param data byte array
     * @param offset position of the first byte to read in the data array
     * @return the read value, as long as java has no unsigned integer
     */
    public static long readIntLittleEndian(byte data[], int offset) {
        return (data[offset] & 0xFF)
                + ((data[offset + 1] & 0xFF) << 8)
                + ((data[offset + 2] & 0xFF) << 16)
                + ((data[offset + 3] & 0xFF) << 24);
    }

    /**
     * gets an instance of the SeemooQmi
     * this class is a singleton, only one instance will ever exist
     *
     * @param appContext application context
     * @return the SeemooQmi instance
     */
    public static SeemooQmi getInstance(Context appContext) {
        if (singletonInstance == null) {
            singletonInstance = new SeemooQmi(appContext);
        }
        return singletonInstance;
    }

    /**
     * private constructor, use getInstance instead!
     *
     * @param appContext application context
     */
    private SeemooQmi(Context appContext) {
        this.appContext = appContext;
    }

    public void start() {
        File statusFile = new File(STATUS_FILE);
        File dataFile = new File(DATA_FILE);
        if (statusFile.exists() && dataFile.exists()) {
            notifyStatusListeners(appContext.getResources().getString(R.string.qmi_dev_success), 2);
            pollHandler.postDelayed(pollRunnable, pollRate);
        } else {
            notifyStatusListeners(appContext.getResources().getString(R.string.qmi_dev_error), 1);
        }
    }

    public void setPollRate(int pollRate) {
        this.pollRate = pollRate;
    }

    /**
     * adds a new packet listener
     *
     * @param svcId id of the service
     * @param indications true if we want to listen for indications, false for normal services
     * @param listener the listener to add
     */
    public void addPacketListener(int svcId, boolean indications, PacketListener listener) {
        if (indications) {
            svcId |= (1 << 31);
        }
        if (packetListeners.containsKey(svcId)) {
            packetListeners.get(svcId).add(listener);
        } else {
            List<PacketListener> listeners = new LinkedList<PacketListener>();
            listeners.add(listener);
            packetListeners.put(svcId, listeners);
        }
    }

    /**
     * removes a packet listener
     *
     * @param svcId id of the service
     * @param indications true if we want to listen for indications, false for normal services
     * @param listener the listener to add
     */
    public void removePacketListener(int svcId, boolean indications, PacketListener listener) {
        if (indications) {
            svcId |= (1 << 31);
        }
        if (packetListeners.containsKey(svcId)) {
            packetListeners.get(svcId).remove(listener);
        }
    }

    /**
     * send a message to all (matching) packet listeners
     *
     * @param svcId id of the destination service
     * @param data payload data to send
     * @param dataLength length of the data
     */
    private void notifyPacketListeners(int svcId, byte[] data, int dataLength) {
        if (packetListeners.containsKey(svcId)) {
            for (PacketListener pl : packetListeners.get(svcId)) {
                pl.packetReceived(new PacketReceiveEvent(this, data, dataLength));
            }
        }
    }

    /**
     * adds a status listener
     *
     * @param listener listener to add
     * @param messageMask mask of status messages which should be send to the listener
     */
    public void addStatusListener(StatusListener listener, int messageMask) {
        statusListeners.put(listener, messageMask);
    }

    /**
     * removes a status listener
     *
     * @param listener the listener to remove
     */
    public void removeStatusListener(StatusListener listener) {
        statusListeners.remove(listener);
    }

    /**
     * send a message to all (matching) status listeners
     *
     * @param message the message to send
     * @param levelMask log level mask
     */
    public void notifyStatusListeners(String message, int levelMask) {
        //send to matching listeners
        if (!statusListeners.isEmpty()) {
            for (Map.Entry<StatusListener, Integer> entry : statusListeners.entrySet()) {
                if ((entry.getValue() & levelMask) != 0) {
                    entry.getKey().statusUpdate(new StatusUpdateEvent(this, message + "\n"));
                }
            }
        }
        //write to old messages storage
        if ((oldStatusMessagesMask & levelMask) != 0) {
            oldStatusMessages.add(message + "\n");
            int length = oldStatusMessages.size();
            if (length > maxOldMessagesSize) {
                oldStatusMessages = oldStatusMessages.subList(length - maxOldMessagesSize / 2, length);
            }
        }
    }

    /**
     * return all status messages received until now that match the mask defined with
     * setOldStatusMessagesMask()
     *
     * @return
     */
    public List<String> getOldStatusMessages() {
        return oldStatusMessages;
    }

    /**
     * reads status messages from the device
     */
    private void readStatus() {
        try {
            File file = new File(STATUS_FILE);
            InputStream inputStream = new FileInputStream(file);

            if (inputStream != null) {
                InputStreamReader inputStreamReader = new InputStreamReader(inputStream);
                BufferedReader bufferedReader = new BufferedReader(inputStreamReader);
                String receiveString = "";
                StringBuilder stringBuilder = new StringBuilder();

                while ((receiveString = bufferedReader.readLine()) != null) {
                    stringBuilder.append(receiveString).append("\n");
                }

                inputStream.close();
                if (stringBuilder.length() > 0) {
                    notifyStatusListeners(stringBuilder.substring(0, stringBuilder.length()-1), 2);
                }
            }
        } catch (Exception e) {
            notifyStatusListeners(e.toString(), 1);
        }
    }

    /**
     * reads QMI packets from the device
     */
    private void readMessages() {
        int read = 1;
        do {
            File file = new File(DATA_FILE);
            byte[] data = new byte[MAX_PACKAGE_LENGTH];
            try {
                FileInputStream fis = new FileInputStream(file);
                read = fis.read(data);
                if (read > 0) {
                    int svcId = (int)readIntLittleEndian(data, 0);
                    notifyStatusListeners(String.format(appContext.getResources().getString(R.string.packet_receive), svcId, read), 8);
                    notifyPacketListeners(svcId, data, read);
                }
            } catch (Exception e) {
                notifyStatusListeners(appContext.getResources().getString(R.string.error_msg_read) + e.toString(), 1);
            }
        } while (read > 0);
    }

    /**
     * sends a QMI packet to the device
     *
     * @param svc service ID
     * @param indicationRegister true if the message is an indication register message, otherwise false
     * @param payloadData payload bytes
     * @param payloadLength payload length
     */
    public void sendMessage(int svc, boolean indicationRegister, byte[] payloadData, int payloadLength) {
        byte data[] = new byte[payloadLength + 4];
        //put service ID
        data[0] = (byte)(svc & 0xFF);
        data[1] = (byte)((svc >> 8) & 0xFF);
        data[2] = (byte)((svc >> 16) & 0xFF);
        data[3] = (byte)((svc >> 24) & 0xFF);
        if (indicationRegister) {
            //set highest bit one to tell kerne module we want to register indications
            data[3] |= 0x80;
        }

        //copy payload
        System.arraycopy(payloadData, 0, data, 4, payloadLength);

        notifyStatusListeners(String.format(appContext.getResources().getString(R.string.packet_send), readIntLittleEndian(data, 0), payloadLength + 4), 16);
        try {
            FileOutputStream fos = new FileOutputStream(DATA_FILE);
            fos.write(data);
            fos.close();
        } catch (Exception e) {
            notifyStatusListeners(appContext.getResources().getString(R.string.error_msg_send) + e.toString(), 1);
        }
    }

    /**
     * add a service to the SeemooQmi, this is neccessary to finalize it at app exit
     *
     * @param seemooQmiService service to add
     */
    public void addService(SeemooQmiService seemooQmiService) {
        services.add(seemooQmiService);
    }

    /**
     * finalize all service, i.e. give them the chance to clean-up (e.g. de-register in modem)
     */
    public void finalizeAllServices() {
        for (SeemooQmiService service : services) {
            service.finalizeService();
        }

        stopPolling = true;
        //read messages a last time to get QMI response from request in finalizeService methods
        //otherwise these would appear at the next App start
        readMessages();
    }
}
