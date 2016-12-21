/**
 * LTE MAC frames service
 *
 * @author Carsten Bruns (carst.bruns@gmx.de)
 */
package de.tu_darmstadt.seemoo.seemooqcomlte.seemooqmi;

import android.content.Context;

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
    //TODO handle uplink packets in the same way as donwlink packets, only change flag
    public static final int LTE_MAC_UL_SVC_ID = 0x4D414375; //"MACu" in ASCII

    private static final String MAC_LTE_START_STRING = "mac-lte";

    private static final int FDD_RADIO = 1;
    private static final int TDD_RADIO = 2;

    private static final int DIRECTION_UPLINK = 0;
    private static final int DIRECTION_DOWNLINK = 1;

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

    private boolean udpEnable = false;
    private String udpIpAdress;
    private int udpPort;

    /**
     * constructor
     *
     * @param seemooQmiP SeemooQmi instance
     * @param appContextP application context
     */
    public LteMacService(SeemooQmi seemooQmiP, Context appContextP) {
        super(seemooQmiP, appContextP);

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
                    sendPacketOverUdp(e.getData(), e.getDataLength());
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
        int decodeTable[] = {3,5,1,2,-1,4,-1,-1,-1};
        int res = ((rntiModem >= 0) && (rntiModem < decodeTable.length)) ? decodeTable[rntiModem] : -1;
        if (res < 0) {
            //unknown RNTI type value
            res = 0;
            seemooQmi.notifyStatusListeners(String.format(appContext.getResources().getString(R.string.ltemac_rnti_error), rntiModem), 1);
        }
        return res;
    }

    /**
     * class for metadata containing additional information for LTE MAC messages
     */
    private class LteMacMetadata {
        //TODO decode other information of metadata
        private int rntiType;

        LteMacMetadata(byte[] data) {
            rntiType = translateRnti(data[11] & 0x7F);
        }

        public int getRntiType() {
            return rntiType;
        }
    }

    /**
     * send a LTE MAC frame over UDP
     *
     * @param data MAC frame PDU
     * @param dataLength length of data
     */
    private void sendPacketOverUdp(byte[] data, int dataLength) {
        try {
            ByteArrayOutputStream dataStream = new ByteArrayOutputStream();

            LteMacMetadata metadata = new LteMacMetadata(data);

            //start string so that Wireshark recognizes it as LTE MAC frame
            dataStream.write(MAC_LTE_START_STRING.getBytes());
            //mandatory metainformation
            dataStream.write(FDD_RADIO); //TODO
            dataStream.write(DIRECTION_DOWNLINK); //TODO
            dataStream.write(metadata.getRntiType());

            //TODO include optional data fields in UDP LTE-MAC payload header

            //include PDU
            dataStream.write(MAC_LTE_PAYLOAD_TAG);
            dataStream.write(data, 12, dataLength - 12);

            DatagramSocket ds = new DatagramSocket();
            InetAddress local = InetAddress.getByName(udpIpAdress);
            DatagramPacket p = new DatagramPacket(dataStream.toByteArray(), dataStream.size(), local, udpPort);
            ds.send(p);
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
        byte register[] = {(byte)(state ? 1 : 0)};
        seemooQmi.sendMessage(LTE_MAC_DL_SVC_ID, true, register, 1);
    }

    /**
     * enable forwarding of incoming LTE MAC frames over UDP
     *
     * @param udpIpAdress UDP destination IP address
     * @param udpPort UDP destination port
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
