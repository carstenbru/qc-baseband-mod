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

    public interface StatusListener extends EventListener {
        void statusUpdate(StatusUpdateEvent e);
    }

    public interface PacketListener extends EventListener {
        void packetReceived(PacketReceiveEvent e);
    }

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

    public static long readIntLittleEndian(byte data[], int offset) {
        return (data[offset] & 0xFF)
                + ((data[offset + 1] & 0xFF) << 8)
                + ((data[offset + 2] & 0xFF) << 16)
                + ((data[offset + 3] & 0xFF) << 24);
    }

    public static SeemooQmi getInstance(Context appContext) {
        if (singletonInstance == null) {
            singletonInstance = new SeemooQmi(appContext);
        }
        return singletonInstance;
    }

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

    public void removePacketListener(int svcId, boolean indications, PacketListener listener) {
        if (indications) {
            svcId |= (1 << 31);
        }
        if (packetListeners.containsKey(svcId)) {
            packetListeners.get(svcId).remove(listener);
        }
    }

    private void notifyPacketListeners(int svcId, byte[] data, int dataLength) {
        if (packetListeners.containsKey(svcId)) {
            for (PacketListener pl : packetListeners.get(svcId)) {
                pl.packetReceived(new PacketReceiveEvent(this, data, dataLength));
            }
        }
    }

    public void addStatusListener(StatusListener listener, int messageMask) {
        statusListeners.put(listener, messageMask);
    }

    public void removeStatusListener(StatusListener listener) {
        statusListeners.remove(listener);
    }

    public void notifyStatusListeners(String message, int levelMask) {
        if (!statusListeners.isEmpty()) {
            for (Map.Entry<StatusListener, Integer> entry : statusListeners.entrySet()) {
                if ((entry.getValue() & levelMask) != 0) {
                    entry.getKey().statusUpdate(new StatusUpdateEvent(this, message + "\n"));
                }
            }
        }
        if ((oldStatusMessagesMask & levelMask) != 0) {
            oldStatusMessages.add(message + "\n");
            int length = oldStatusMessages.size();
            if (length > maxOldMessagesSize) {
                oldStatusMessages = oldStatusMessages.subList(length - maxOldMessagesSize / 2, length);
            }
        }
    }

    public List<String> getOldStatusMessages() {
        return oldStatusMessages;
    }

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

    public void sendMessage(int svc, boolean indicationRegister, byte[] payloadData, int payloadLength) {
        byte data[] = new byte[payloadLength + 4];
        data[0] = (byte)(svc & 0xFF);
        data[1] = (byte)((svc >> 8) & 0xFF);
        data[2] = (byte)((svc >> 16) & 0xFF);
        data[3] = (byte)((svc >> 24) & 0xFF);
        if (indicationRegister) {
            data[3] |= 0x80;
        }

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

    public void addService(SeemooQmiService seemooQmiService) {
        services.add(seemooQmiService);
    }

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
