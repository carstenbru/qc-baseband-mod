/**
 * service for snprintf function hook messages
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

public class SnprintfService extends SeemooQmiService {
    public static final int SNPRINTF_SVC_ID = 0x736E7066; //"snpf" in ASCII

    private List<SnprintfListener> snprintfListeners = new LinkedList<SnprintfListener>();

    private List<String> oldMessages = new LinkedList<String>();
    private int maxOldMessagesSize = 65536;

    public void setMaxOldMessagesSize(int maxOldMessagesSize) {
        this.maxOldMessagesSize = maxOldMessagesSize;
    }

    /**
     * listener for new snprintf messages
     */
    public interface SnprintfListener extends EventListener {
        public void statusUpdate(SnprintfMessageEvent e);
    }

    /**
     * event for an incoming snprintf message
     */
    public class SnprintfMessageEvent extends EventObject {
        private String message;

        public SnprintfMessageEvent(Object source, String message) {
            super(source);
            this.message = message;
        }

        public String getMessage() {
            return message;
        }
    }

    /**
     * constructor
     *
     * @param seemooQmiP instance of SeemooQmi
     * @param appContextP application context
     */
    public SnprintfService(SeemooQmi seemooQmiP, Context appContextP) {
        super(seemooQmiP, appContextP);

        //add listener for service registration change responses
        seemooQmi.addPacketListener(SNPRINTF_SVC_ID, false, new SeemooQmi.PacketListener() {
            @Override
            public void packetReceived(SeemooQmi.PacketReceiveEvent e) {
                if (e.getDataLength() == 8) {
                    seemooQmi.notifyStatusListeners(appContext.getResources().getString(R.string.snprintf_reg_success), 4);
                } else {
                    seemooQmi.notifyStatusListeners(appContext.getResources().getString(R.string.snprintf_dereg_success), 4);
                }
            }
        });
        //add listener for snprintf call indication messages
        seemooQmi.addPacketListener(SNPRINTF_SVC_ID, true, new SeemooQmi.PacketListener() {
            @Override
            public void packetReceived(SeemooQmi.PacketReceiveEvent e) {
                byte[] data = e.getData();
                StringBuilder message = new StringBuilder();
                int writeDest = (int)SeemooQmi.readIntLittleEndian(data, 4); //TODO option to print write destination or not
                for (int i = 8; i < e.getDataLength(); i++) {
                    message.append((char)data[i]);
                }
                notifySnprintfListeners(message.toString());
            }
        });
    }

    public void addListener(SnprintfListener snprintfListener) {
        snprintfListeners.add(snprintfListener);
    }

    public void removeListener(SnprintfListener snprintfListener) {
        snprintfListeners.remove(snprintfListener);
    }

    /**
     * send message to all listeners
     *
     * @param message message to send
     */
    private void notifySnprintfListeners(String message) {
        //send message to all listeners
        if (!snprintfListeners.isEmpty()) {
            for (SnprintfListener sl : snprintfListeners) {
                sl.statusUpdate(new SnprintfMessageEvent(this, message + "\n"));
            }
        }
        //in addition store message internally
        oldMessages.add(message + "\n");
        int length = oldMessages.size();
        if (length > maxOldMessagesSize) {
            oldMessages = oldMessages.subList(length - maxOldMessagesSize/2, length);
        }
    }

    /**
     * register/deregister for indications in modem
     *
     * @param state new registration state
     */
    public void register(boolean state) {
        byte register[] = {(byte)(state ? 1 : 0)};
        seemooQmi.sendMessage(SNPRINTF_SVC_ID, true, register, 1);
    }

    /**
     * returns all old messages, i.e. all stored messages which arrived until now
     *
     * @return
     */
    public List<String> getCachedMessages() {
        return oldMessages;
    }

    @Override
    public void finalizeService() {
        register(false);
    }
}
