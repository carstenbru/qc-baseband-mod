/**
 * LTE MAC frames service
 *
 * @author Carsten Bruns (carst.bruns@gmx.de)
 */
package de.tu_darmstadt.seemoo.seemooqcomlte.seemooqmi;

import android.content.Context;
import android.telephony.TelephonyManager;

import java.io.ByteArrayOutputStream;
import java.io.IOException;
import java.net.DatagramPacket;
import java.net.DatagramSocket;
import java.net.InetAddress;
import java.net.SocketException;
import java.net.UnknownHostException;

import de.tu_darmstadt.seemoo.seemooqcomlte.R;


public class LteMacService extends SeemooQmiService {
    public static final int LTE_MAC_DL_SVC_ID = 0x4D414364; //"MACd" in ASCII
    public static final int LTE_MAC_UL_SVC_ID = 0x4D414375; //"MACu" in ASCII
    public static final int LTE_MAC_UL_RACH_SVC_ID = 0x4D414372; //"MACr" in ASCII

    private static final String MAC_LTE_START_STRING = "mac-lte";

    private static final int FDD_RADIO = 1;
    private static final int TDD_RADIO = 2;

    private static final int DIRECTION_UPLINK = 0;
    private static final int DIRECTION_DOWNLINK = 1;
    private static final int DIRECTION_UPLINK_RACH = 2;

    private static final int MAC_LTE_PAYLOAD_TAG = 0x01;
    private static final int MAC_LTE_RNTI_TAG = 0x02;
    private static final int MAC_LTE_UEID_TAG = 0x03;
    private static final int MAC_LTE_FRAME_SUBFRAME_TAG = 0x04;
    private static final int MAC_LTE_PREDEFINED_DATA_TAG = 0x05;
    private static final int MAC_LTE_RETX_TAG = 0x06;
    private static final int MAC_LTE_CRC_STATUS_TAG = 0x07;
    private static final int MAC_LTE_EXT_BSR_SIZES_TAG = 0x08;
    private static final int MAC_LTE_SEND_PREAMBLE_TAG = 0x09;
    private static final int MAC_LTE_CARRIER_ID_TAG = 0x0A;
    private static final int MAC_LTE_PHY_TAG = 0x0B;
    private static final int MAC_LTE_SIMULT_PUCCH_PUSCH_PCELL_TAG = 0x0C;
    private static final int MAC_LTE_SIMULT_PUCCH_PUSCH_PSCELL_TAG = 0x0D;
    private static final int MAC_LTE_CE_MODE_TAG = 0x0E;
    private static final int MAC_LTE_NB_MODE_TAG = 0x0F;
    private static final int MAC_LTE_N_UL_RB_TAG = 0x10;

    //timescale of modem: 1ms equals 19200 ticks
    private static final int MODEM_TIMESCALE_1MS = 19200;
    //offset to shift subframe intervals to encounter timer inaccuracy (not really neccessary)
    private static final int UL_TIMING_SECURE_OFFSET = 64;

    private boolean udpEnable = false;
    private String udpIpAdress;
    private int udpPort;
    private DatagramSocket datagramSocket;

    //UE Id used to identify this phone in wireshark, we use the last 4 digits of of IMSI
    private int wiresharkUeid;

    /** variables used to synchronize uplink frame numbers
     *  synchronization is done on RAR repsonse (RACH msg2) and RRCConnection request (RACH msg3) in uplink
     *  which have a fixed delay of 6 subframes
     **/
    private boolean syncUplinkSystemFrame = false; //flag to start synchronization
    private int uplinkSystemFrameSync = 0; //offset to subtract from uplink time to calculate system time
    private int lastDownlinkFrameTimeMs; //system time (frame number in ms) of the last downlink frame

    /**
     * constructor
     *
     * @param seemooQmiP  SeemooQmi instance
     * @param appContextP application context
     */
    public LteMacService(SeemooQmi seemooQmiP, Context appContextP) {
        super(seemooQmiP, appContextP);

        String deviceId = ((TelephonyManager)appContext.getSystemService(Context.TELEPHONY_SERVICE)).getSubscriberId();
        if (deviceId != null) {
            wiresharkUeid = Integer.parseInt(deviceId.substring(deviceId.length() - 4, deviceId.length()));
        } else {
            wiresharkUeid = 0;
        }

        try {
            datagramSocket = new DatagramSocket();
        } catch (SocketException se) {
            datagramSocket = null;
            //TODO
        }

        //add listener for service registration change responses
        seemooQmi.addPacketListener(LTE_MAC_DL_SVC_ID, false, new SeemooQmi.PacketListener() {
            @Override
            public void packetReceived(SeemooQmi.PacketReceiveEvent e) {
                if (e.getDataLength() == 8) {
                    seemooQmi.notifyStatusListeners(appContext.getResources().getString(R.string.ltemac_reg_success), 4);
                } else {
                    seemooQmi.notifyStatusListeners(appContext.getResources().getString(R.string.ltemac_dereg_success), 4);
                }
            }
        });
        //add listener for snprintf call indication messages
        seemooQmi.addPacketListener(LTE_MAC_DL_SVC_ID, true, new SeemooQmi.PacketListener() {
            @Override
            public void packetReceived(SeemooQmi.PacketReceiveEvent e) {
                if (udpEnable) {
                    sendPacketOverUdp(e.getData(), e.getDataLength(), DIRECTION_DOWNLINK);
                }
            }
        });
        seemooQmi.addPacketListener(LTE_MAC_UL_SVC_ID, true, new SeemooQmi.PacketListener() {
            @Override
            public void packetReceived(SeemooQmi.PacketReceiveEvent e) {
                if (udpEnable) {
                    sendPacketOverUdp(e.getData(), e.getDataLength(), DIRECTION_UPLINK);
                }
            }
        });

        seemooQmi.addPacketListener(LTE_MAC_UL_RACH_SVC_ID, true, new SeemooQmi.PacketListener() {
            @Override
            public void packetReceived(SeemooQmi.PacketReceiveEvent e) {
                if (udpEnable) {
                    sendPacketOverUdp(e.getData(), e.getDataLength(), DIRECTION_UPLINK_RACH);
                }
            }
        });
    }

    /**
     * translates a RNTI type value from the values used by Qualcomm to the ones used by Wireshark
     *
     * @param rntiModem RNTI type value received from modem
     * @return RNTI tyoe value to use for Wireshark
     */
    private int translateRnti(int rntiModem) {
        //see lte/api/lte_as.h for codes
        int decodeTable[] = {3, 5, 1, 2, 3, 4, -1, -1, -1};
        int res = ((rntiModem >= 0) && (rntiModem < decodeTable.length)) ? decodeTable[rntiModem] : -1;
        if (res < 0) {
            //unknown RNTI type value
            res = 0;
            seemooQmi.notifyStatusListeners(String.format(appContext.getResources().getString(R.string.ltemac_rnti_error), rntiModem), 1);
        }
        return res;
    }

    /**
     * class for metadata containing additional information for LTE MAC downlink messages
     */
    private class LteMacDLMetadata {
        //TODO decode other information of metadata
        private int rntiType;
        private int frameNumber;

        LteMacDLMetadata(byte[] data) {
            rntiType = translateRnti(data[11] & 0x7F);
            frameNumber = (data[8] & 0xFF) + ((data[9] & 0xFF) << 8);
        }

        public int getRntiType() {
            return rntiType;
        }

        public  int getFrameNumber() {
            return frameNumber;
        }
    }

    /**
     * writes constant (i.e. not depending on incoming packages) metadata tags
     *
     * @param dataStream write destination byte stream
     */
    private void writeConstantMetadata(ByteArrayOutputStream dataStream) {
        dataStream.write(MAC_LTE_UEID_TAG);
        dataStream.write((byte)(wiresharkUeid >> 8));
        dataStream.write((byte)wiresharkUeid);
    }

    int last_dl_freme_num; //TODO remove
    /**
     * send a LTE MAC frame over UDP
     *
     * @param data          MAC frame PDU
     * @param dataLength    length of data
     * @param dataDirection direction of the data packet (DIRECTION_UPLINK or DIRECTION_DOWNLINK)
     */
    private void sendPacketOverUdp(byte[] data, int dataLength, int dataDirection) {
        try {
            ByteArrayOutputStream dataStream = new ByteArrayOutputStream();

            //start string so that Wireshark recognizes it as LTE MAC frame
            dataStream.write(MAC_LTE_START_STRING.getBytes());
            //mandatory metainformation
            dataStream.write(FDD_RADIO);
            dataStream.write(dataDirection & 1);

            if (dataDirection == DIRECTION_DOWNLINK) {
                LteMacDLMetadata metadata = new LteMacDLMetadata(data);

                if (syncUplinkSystemFrame) { //when in uplink time synchronization..
                    //store system time (in ms) of the downlink frame
                    long frameNumber = metadata.getFrameNumber();
                    lastDownlinkFrameTimeMs = (int)(frameNumber >> 4) * 10 + (int)(frameNumber & 0xF);
                }

                dataStream.write(metadata.getRntiType());

                dataStream.write(MAC_LTE_FRAME_SUBFRAME_TAG);
                dataStream.write((byte)(metadata.getFrameNumber() >> 8));
                dataStream.write((byte)metadata.getFrameNumber());
                writeConstantMetadata(dataStream);
                last_dl_freme_num = metadata.getFrameNumber();

                //include PDU
                dataStream.write(MAC_LTE_PAYLOAD_TAG);
                dataStream.write(data, 12, dataLength - 12);
            } else if (dataDirection == DIRECTION_UPLINK) {
                long frameTime = SeemooQmi.readIntLittleEndian(data, 4);

                if (syncUplinkSystemFrame) { //when in uplink time synchronization..
                    //calculate time offset: shift by one interval to be sure to still calculate right value even with LTE timing advance (max 667,66us)
                    uplinkSystemFrameSync = (int)(frameTime - MODEM_TIMESCALE_1MS + UL_TIMING_SECURE_OFFSET);
                    //shift by current system time
                    uplinkSystemFrameSync -= ((lastDownlinkFrameTimeMs + 6) * MODEM_TIMESCALE_1MS);
                    syncUplinkSystemFrame = false;
                }

                int frameTimeMs = (int)(frameTime - uplinkSystemFrameSync) / MODEM_TIMESCALE_1MS; //calculate time value: shift interval and scale
                frameTimeMs = (((frameTimeMs % 10240) + 10240) % 10240); //limit system frame number to 1024 (modulo)
                int frameTimeWireshark = ((frameTimeMs / 10) << 4) + (frameTimeMs % 10); //convert to wireshark representation

                dataStream.write(3);
                writeConstantMetadata(dataStream);

                dataStream.write(MAC_LTE_FRAME_SUBFRAME_TAG);
                dataStream.write((byte)(frameTimeWireshark >> 8));
                dataStream.write((byte)frameTimeWireshark);
                writeConstantMetadata(dataStream);

                //include PDU
                dataStream.write(MAC_LTE_PAYLOAD_TAG);
                dataStream.write(data, 12, dataLength - 12);
            } else {
                syncUplinkSystemFrame = true;

                dataStream.write(2); //RACH
                dataStream.write(MAC_LTE_SEND_PREAMBLE_TAG);
                dataStream.write(data[4]);
                dataStream.write(data[5] + 1);
                writeConstantMetadata(dataStream);
                dataStream.write(MAC_LTE_PAYLOAD_TAG);
            }

            InetAddress dest = InetAddress.getByName(udpIpAdress);
            DatagramPacket p = new DatagramPacket(dataStream.toByteArray(), dataStream.size(), dest, udpPort);
            datagramSocket.send(p);
        } catch (SocketException se) { //TODO maybe deactivate on exception as it will be the same next time
            //TODO
        } catch (UnknownHostException uhe) {
            //TODO
        } catch (IOException ioe) {
            //TODO
        }
    }

    /**
     * register/deregister for indications in modem
     *
     * @param state new registration state
     */
    public void register(boolean state) {
        byte register[] = {(byte) (state ? 1 : 0)};
        seemooQmi.sendMessage(LTE_MAC_DL_SVC_ID, true, register, 1);
    }

    /**
     * enable forwarding of incoming LTE MAC frames over UDP
     *
     * @param udpIpAdress UDP destination IP address
     * @param udpPort     UDP destination port
     */
    public void startSendPacketsOverUdp(String udpIpAdress, int udpPort) {
        this.udpIpAdress = udpIpAdress;
        this.udpPort = udpPort;
        this.udpEnable = true;
    }

    /**
     * stop forwarding LTE MAC frames over UDP
     */
    public void stopUdp() {
        this.udpEnable = false;
    }

    @Override
    public void finalizeService() {
        stopUdp();
        register(false);
    }
}
