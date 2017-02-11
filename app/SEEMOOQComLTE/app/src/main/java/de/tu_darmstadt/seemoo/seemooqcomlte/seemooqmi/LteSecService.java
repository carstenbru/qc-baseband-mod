/**
 * LTE security service implementation
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

import de.tu_darmstadt.seemoo.seemooqcomlte.R;

//TODO pycparser/hexag00n references in thesis

//TODO try if MAC is now correct length
//TODO try if MAC direction works
//TODO try to calculate MAC and en/de-cryption
//TODO try if mutils hooks work better..


//TODO capture again: MAC algorithms were wrong, MAC length was wrong
//TODO find and hook DSM functions and see if they are used..

//TODO generate key function
//TODO generate algo key input data


//TODO implement all message types, depending on registration

//TODO registration mode for listeners, register for all ORed
//TODO options in GUI, either in options or directly in fragment (currently 2nd options preferred)

//TODO last key caching (per purpose)
//TODO message caching (not so important, maybe not)

//TODO doc

public class LteSecService extends SeemooQmiService {
    public static final int LTE_SEC_SVC_ID = 0x4C544573; //"LTEs" in ASCII

    private List<LteSecListener> lteSecListeners = new LinkedList<LteSecListener>();

    /**
     * listener for LTE security updates
     */
    public interface LteSecListener extends EventListener {
        void newAlgorithmKey(NewAlgorithmKeyEvent e);
        void cipherCall(CryptoCallEvent e);
        void decipherCall(CryptoCallEvent e);
        void maciCall(CryptoCallEvent e);
    }

    /**
     * registration flags specifying what indications with which content should be sent
     */
    public enum RegistrationFlag {
        GENERATED_KEYS(1 << 0), /* generated keys */
        GENERATED_KEYS_INPUT((1 << 0) | (1 << 1)), /* include input data (in_key, in_string, in_string_length) for generated keys */

        GENERATED_ALGO_KEYS(1 << 2), /* generated algorithm keys (includes KeyUse, KeyAlgorithm)) */
        GENERATED_ALGO_KEYS_INPUT((1 << 2) | (1 << 3)), /* include input data (in_key)  */

        CIPHER_CALLS(1 << 4), /* cipher calls (includes used algorithm, bearer, count, msg_length) */
        CIPHER_CALLS_KEY((1 << 4) | (1 << 5)), /* include used key */
        CIPHER_CALLS_IN_MSG((1 << 4) | (1 << 6)), /* include input message */
        CIPHER_CALLS_OUT_MSG((1 << 4) | (1 << 7)), /* include ciphered message */

        DECIPHER_CALLS(1 << 8), /* decipher calls (includes used algorithm, bearer, count, msg_length) */
        DECIPHER_CALLS_KEY((1 << 8) | (1 << 9)), /* include used key */
        DECIPHER_CALLS_IN_MSG((1 << 8) | (1 << 10)), /* include input message */
        DECIPHER_CALLS_OUT_MSG((1 << 8) | (1 << 11)), /* include deciphered message */

        MACI_CALLS(1 << 12), /* MAC generation calls (includes used algorithm, bearer, count, msg_length, direction) */
        MACI_CALLS_KEY((1 << 12) | (1 << 13)), /* include used key */
        MACI_CALLS_IN_MSG((1 << 12) | (1 << 14)), /* include input message */
        MACI_CALLS_MAC((1 << 12) | (1 << 15)); /* include generated MAC */

        private int value;

        private RegistrationFlag(int value) {
            this.value = value;
        }

        public int getValue() {
            return value;
        }
        public boolean flagSet(int testVal) {
            return ((testVal & value) == value);
        }
    }

    public enum KeyUse {
        NAS_ENC(1),
        NAS_INT(2),
        RRC_ENC(3),
        RRC_INT(4),
        UP_ENC(5);

        private int value;
        private static KeyUse[] allValues = values();

        private KeyUse(int value) {
            this.value = value;
        }

        public int getValue() {
            return value;
        }

        public static KeyUse fromInt(int n) {
            return allValues[n-1];
        }
    }

    public enum KeyAlgorithm {
        UEA1(0, false),
        UEA2(1, false),
        EEA1(2, false),
        EEA2(3, false),
        EEA3(4, false),
        EEA0(7, false),

        UIA1(0, true),
        EIA1(1, true),
        EIA2(2, true),
        EIA3(3, true),
        EIA0(7, true);

        private int value;
        private boolean integrityAlgo;

        private KeyAlgorithm(int value, boolean integrityAlgo) {
            this.integrityAlgo = integrityAlgo;
            this.value = value;
        }

        public int getValue() {
            return value;
        }

        public boolean isIntegrityAlgo() {
            return integrityAlgo;
        }

        public static KeyAlgorithm fromInt(boolean integrityAlgo, int n) {
            for (KeyAlgorithm ka : KeyAlgorithm.values()) {
                if (ka.isIntegrityAlgo() == integrityAlgo) {
                    if (ka.getValue() == n) {
                        return ka;
                    }
                }
            }
            return null;
        }
    }

    /**
     * algorithm key update event
     */
    public class NewAlgorithmKeyEvent extends EventObject {
        private byte key[];
        private KeyUse keyUse;
        private KeyAlgorithm keyAlgorithm;

        public NewAlgorithmKeyEvent(Object source, byte[] key, KeyUse keyUse, KeyAlgorithm keyAlgorithm) {
            super(source);
            this.key = key;
            this.keyUse = keyUse;
            this.keyAlgorithm = keyAlgorithm;
        }

        public byte[] getKey() {
            return key;
        }

        public KeyUse getKeyUse() {
            return keyUse;
        }

        public KeyAlgorithm getKeyAlgorithm() {
            return keyAlgorithm;
        }
    }

    /**
     * crypto function call event (cipher, decipher, MAC)
     */
    public class CryptoCallEvent extends EventObject {
        private byte usedKey[];
        private KeyAlgorithm keyAlgorithm;
        private byte bearer;
        private long count;
        private int msgLength;
        private byte[] inMsg;
        private byte[] outMsg;
        private boolean directionDownlink;

        public CryptoCallEvent(Object source,
                               KeyAlgorithm keyAlgorithm,
                               byte bearer,
                               long count,
                               int msgLength,
                               byte[] usedKey,
                               byte[] inMsg,
                               byte[] outMsg,
                               boolean directionDownlink) {
            super(source);
            this.keyAlgorithm = keyAlgorithm;
            this.bearer = bearer;
            this.count = count;
            this.msgLength = msgLength;

            this.usedKey = usedKey;
            this.inMsg = inMsg;
            this.outMsg = outMsg;
            this.directionDownlink = directionDownlink;
        }

        public byte[] getUsedKey() {
            return usedKey;
        }

        public KeyAlgorithm getKeyAlgorithm() {
            return keyAlgorithm;
        }

        public byte getBearer() {
            return bearer;
        }

        public long getCount() {
            return count;
        }

        public int getMsgLength() {
            return msgLength;
        }

        public byte[] getInMsg() {
            return inMsg;
        }

        public byte[] getOutMsg() {
            return outMsg;
        }

        public boolean isDirectionDownlink() {
            return directionDownlink;
        }
    }

    public LteSecService(SeemooQmi seemooQmiP, Context appContextP) {
        super(seemooQmiP, appContextP);

        //add listener for service registration change responses
        seemooQmi.addPacketListener(LTE_SEC_SVC_ID, false, new SeemooQmi.PacketListener() {
            @Override
            public void packetReceived(SeemooQmi.PacketReceiveEvent e) {
                if (e.getDataLength() == 8) {
                    seemooQmi.notifyStatusListeners(appContext.getResources().getString(R.string.lte_sec_reg_success), 4);
                } else {
                    seemooQmi.notifyStatusListeners(appContext.getResources().getString(R.string.lte_sec_dereg_success), 4);
                }
            }
        });
        //add listener for LTE security indication messages
        seemooQmi.addPacketListener(LTE_SEC_SVC_ID, true, new SeemooQmi.PacketListener() {
            void readAndNotifyCryptoCall(RegistrationFlag type, byte[] data, int messageContent, RegistrationFlag keyFlag, RegistrationFlag inMsgFlag, RegistrationFlag outMsgFlag) {
                long count = SeemooQmi.readIntLittleEndian(data, 12);
                int msgLength = (data[10] + (data[11] << 8));

                byte[] usedKey = null;
                byte[] inMsg = null;
                byte[] outMsg = null;

                int pos = 16;
                if (keyFlag.flagSet(messageContent)) {
                    usedKey = Arrays.copyOfRange(data, pos, pos + 16);
                    pos += 16;
                }
                if (inMsgFlag.flagSet(messageContent)) {
                    inMsg = Arrays.copyOfRange(data, pos, pos + msgLength);
                    pos += msgLength;
                }
                if (outMsgFlag.flagSet(messageContent)) {
                    if (type == RegistrationFlag.MACI_CALLS) {
                        outMsg = new byte[4];
                        for (int i = 0; i < 4; i++) { //invert data to convert from little endian to big endian
                            outMsg[3 - i] = data[pos + i];
                        }
                        pos += 4;
                    } else {
                        outMsg = Arrays.copyOfRange(data, pos, pos + msgLength);
                        pos += msgLength;
                    }
                }
                boolean directionDownlink = (type == RegistrationFlag.DECIPHER_CALLS);
                if (type == RegistrationFlag.MACI_CALLS) {
                    directionDownlink = ((data[9] & 0x80) != 0);
                }

                notifyCryptoCall(type, data[8], (byte)(data[9] & 0x7F), count, msgLength, usedKey, inMsg, outMsg, directionDownlink);
            }

            @Override
            public void packetReceived(SeemooQmi.PacketReceiveEvent e) {
                byte[] data = e.getData();
                int messageContent = (int)SeemooQmi.readIntLittleEndian(data, 4);

                if (RegistrationFlag.GENERATED_ALGO_KEYS.flagSet(messageContent)) {
                    byte[] key = Arrays.copyOfRange(data, 8, 24);

                    notifyNewAlgorithmKey(key, data[24], data[25]);
                } else if (RegistrationFlag.CIPHER_CALLS.flagSet(messageContent)) {
                    readAndNotifyCryptoCall(RegistrationFlag.CIPHER_CALLS, data, messageContent, RegistrationFlag.CIPHER_CALLS_KEY, RegistrationFlag.CIPHER_CALLS_IN_MSG, RegistrationFlag.CIPHER_CALLS_OUT_MSG);
                } else if (RegistrationFlag.DECIPHER_CALLS.flagSet(messageContent)) {
                    readAndNotifyCryptoCall(RegistrationFlag.DECIPHER_CALLS, data, messageContent, RegistrationFlag.DECIPHER_CALLS_KEY, RegistrationFlag.DECIPHER_CALLS_IN_MSG, RegistrationFlag.DECIPHER_CALLS_OUT_MSG);
                } else if (RegistrationFlag.MACI_CALLS.flagSet(messageContent)) {
                    readAndNotifyCryptoCall(RegistrationFlag.MACI_CALLS, data, messageContent, RegistrationFlag.MACI_CALLS_KEY, RegistrationFlag.MACI_CALLS_IN_MSG, RegistrationFlag.MACI_CALLS_MAC);
                }
            }
        });

        //TODO depending on listeners!
        RegistrationFlag[] flags = {
                RegistrationFlag.GENERATED_ALGO_KEYS,
                RegistrationFlag.CIPHER_CALLS,
                RegistrationFlag.CIPHER_CALLS_KEY,
                RegistrationFlag.CIPHER_CALLS_IN_MSG,
                RegistrationFlag.CIPHER_CALLS_OUT_MSG,
                RegistrationFlag.DECIPHER_CALLS,
                RegistrationFlag.DECIPHER_CALLS_KEY,
                RegistrationFlag.DECIPHER_CALLS_IN_MSG,
                RegistrationFlag.DECIPHER_CALLS_OUT_MSG,
                RegistrationFlag.MACI_CALLS,
                RegistrationFlag.MACI_CALLS_KEY,
                RegistrationFlag.MACI_CALLS_IN_MSG,
                RegistrationFlag.MACI_CALLS_MAC
        };
        register(flags);
    }

    public void addListener(LteSecListener lteSecListener) {
        lteSecListeners.add(lteSecListener);
    }

    public void removeListener(LteSecListener lteSecListener) {
        lteSecListeners.remove(lteSecListener);
    }

    private void notifyNewAlgorithmKey(byte[] key, byte keyUse, byte keyAlgorithm) {
        for (LteSecListener ul : lteSecListeners) {
            KeyUse keyUseEnum = KeyUse.fromInt(keyUse);
            ul.newAlgorithmKey(new NewAlgorithmKeyEvent(this, key, keyUseEnum, KeyAlgorithm.fromInt(keyUseEnum.name().contains("INT"), keyAlgorithm)));
        }
    }

    private void notifyCryptoCall(
            RegistrationFlag type,
            byte keyAlgorithm,
            byte bearer,
            long count,
            int messageLength,
            byte[] usedKey,
            byte[] inMsg,
            byte[] outMsg,
            boolean directionDownlink) {
        for (LteSecListener ul : lteSecListeners) {
            CryptoCallEvent cce = new CryptoCallEvent(this,
                    KeyAlgorithm.fromInt(type == RegistrationFlag.MACI_CALLS, keyAlgorithm),
                    bearer,
                    count,
                    messageLength,
                    usedKey,
                    inMsg,
                    outMsg,
                    directionDownlink);
            switch (type) {
                case CIPHER_CALLS:
                    ul.cipherCall(cce);
                    break;
                case DECIPHER_CALLS:
                    ul.decipherCall(cce);
                    break;
                case MACI_CALLS:
                    ul.maciCall(cce);
                    break;
            }

        }
    }

    /**
     * register/deregister for indications in modem
     *
     * @param flags registration flags
     */
    public void register(RegistrationFlag[] flags) {
        int flagsInt = 0;
        if (flags != null) {
            for (RegistrationFlag rf : flags) {
                flagsInt |= rf.getValue();
            }
        }
        byte[] data = {
                (byte)(flagsInt & 0xFF),
                (byte)((flagsInt >> 8) & 0xFF),
                (byte)((flagsInt >> 16) & 0xFF),
                (byte)((flagsInt >> 24) & 0xFF)

        };
        seemooQmi.sendMessage(LTE_SEC_SVC_ID, true, data, 4);
    }

    @Override
    public void finalizeService() {
        register(null);
    }
}
