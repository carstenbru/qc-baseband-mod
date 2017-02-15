/**
 * service for channel estimation messages
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

public class ChannelEstimationService extends SeemooQmiService {
    public static final int CHANNEL_ESTIMATION_SVC_ID = 0x43457374; //"CEst" in ASCII

    private ChannelMatrices lastChannelMatrices = null;

    private List<ChannelEstimationListener> channelEstimationListeners = new LinkedList<ChannelEstimationListener>();

    /**
     * listener for new channel estimation messages
     */
    public interface ChannelEstimationListener extends EventListener {
        public void matricesReceived(ChannelMatrixEvent e);
    }

    /**
     * event for an incoming channel matrices message
     */
    public class ChannelMatrixEvent extends EventObject {
        ChannelMatrices channelMatrices;

        public ChannelMatrixEvent(Object source, ChannelMatrices channelMatrices) {
            super(source);

            this.channelMatrices = channelMatrices;
        }

        public ChannelMatrices getChannelMatrices() {
            return channelMatrices;
        }
    }

    public class ChannelMatrices {
        ComplexFixedPoint matrices[][][];

        public ChannelMatrices(int numRxAnt, int numTxAnt, int sampleCount) {
            matrices = new ComplexFixedPoint[numRxAnt][numTxAnt][sampleCount];
        }

        public void setSample(int rxAnt, int txAnt, int sampleNumber, ComplexFixedPoint sampleData) {
            matrices[rxAnt][txAnt][sampleNumber] = sampleData;
        }

        public ComplexFixedPoint[] getChannelMatrix(int rxAnt, int txAnt) {
            return matrices[rxAnt][txAnt];
        }

        public ComplexFixedPoint getChannelMatrixSample(int rxAnt, int txAnt, int sample) {
            return matrices[rxAnt][txAnt][sample];
        }

        public int getNumRxAnt() {
            return matrices.length;
        }

        public int getNumTxAnt() {
            return matrices[0].length;
        }
    }

    /**
     * constructor
     *
     * @param seemooQmiP instance of SeemooQmi
     * @param appContextP application context
     */
    public ChannelEstimationService(SeemooQmi seemooQmiP, Context appContextP) {
        super(seemooQmiP, appContextP);

        //add listener for service registration change responses
        seemooQmi.addPacketListener(CHANNEL_ESTIMATION_SVC_ID, false, new SeemooQmi.PacketListener() {
            @Override
            public void packetReceived(SeemooQmi.PacketReceiveEvent e) {
                if (e.getDataLength() == 8) {
                    seemooQmi.notifyStatusListeners(appContext.getResources().getString(R.string.ch_est_reg_success), 4);
                } else {
                    seemooQmi.notifyStatusListeners(appContext.getResources().getString(R.string.ch_est_dereg_success), 4);
                }
            }
        });
        //add listener for channel estimation matrices indication messages
        seemooQmi.addPacketListener(CHANNEL_ESTIMATION_SVC_ID, true, new SeemooQmi.PacketListener() {
            @Override
            public void packetReceived(SeemooQmi.PacketReceiveEvent e) {
                byte[] data = e.getData();

                int numWhitenMatricesForCsf = data[6];
                int numRxAnt = data[7] & 0xF;
                int numTxAnt = (data[7] >> 4) & 0xF;

                int dataPos = 8;
                //TODO check order of Tx/Rx antennas!!
                ChannelMatrices channelMatrices = new ChannelMatrices(numRxAnt, numTxAnt, numWhitenMatricesForCsf);
                for (int txAnt = 0; txAnt < numTxAnt; txAnt++) {
                    for (int rxAnt = 0; rxAnt < numRxAnt; rxAnt++) {
                        for (int sample = 0; sample < numWhitenMatricesForCsf; sample++) {
                            int real = (data[dataPos] & 0xFF) | (data[dataPos + 1] << 8);
                            int imaginary = (data[dataPos + 2] & 0xFF) | (data[dataPos + 3] << 8);
                            channelMatrices.setSample(rxAnt, txAnt, sample, new ComplexFixedPoint(real, imaginary));
                            dataPos += 4;
                        }
                    }
                }

                notifyChannelEstimationListeners(channelMatrices);
            }
        });
    }

    public void addListener(ChannelEstimationListener channelEstimationListener) {
        if (channelEstimationListeners.isEmpty()) {
            register(true); //register when we add the first listener
        }
        channelEstimationListeners.add(channelEstimationListener);
    }

    public void removeListener(ChannelEstimationListener channelEstimationListener) {
        channelEstimationListeners.remove(channelEstimationListener);
        if (channelEstimationListeners.isEmpty()) {
            register(false); //deregister when we remove the last listener
        }
    }

    /**
     * send message to all listeners
     */
    private void notifyChannelEstimationListeners(ChannelMatrices channelMatrices) {
        //send message to all listeners
        if (!channelEstimationListeners.isEmpty()) {
            for (ChannelEstimationListener cel : channelEstimationListeners) {
                cel.matricesReceived(new ChannelMatrixEvent(this, channelMatrices));
            }
        }

        lastChannelMatrices = channelMatrices;
    }

    /**
     * register/deregister for indications in modem
     *
     * @param state new registration state
     */
    private void register(boolean state) {
        byte register[] = {(byte)(state ? 1 : 0)};
        seemooQmi.sendMessage(CHANNEL_ESTIMATION_SVC_ID, true, register, 1);
    }

    /**
     * gets the last received channel matrices estimation
     *
     * @return last channel matrices or null if none was received until now
     */
    public ChannelMatrices getLastChannelMatrices() {
        return lastChannelMatrices;
    }

    @Override
    public void finalizeService() {
        register(false);
    }
}
