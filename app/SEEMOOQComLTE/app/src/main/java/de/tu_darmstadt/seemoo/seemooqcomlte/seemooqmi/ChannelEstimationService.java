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
    private int interval = 1;

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
        private int bandwidthIndex;
        private ComplexFixedPoint matrices[][][];

        private final int resourceBlocks[] = {0, 6, 15, 25, 50, 75, 100};
        private final float bandwidthMHz[] = {0, 1.4f, 3, 5, 10, 15, 20};

        public ChannelMatrices(int bandwidthIndex, int numRxAnt, int numTxAnt, int sampleCount) {
            bandwidthIndex++;
            this.bandwidthIndex = (bandwidthIndex < resourceBlocks.length) ? bandwidthIndex : 0;
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

        public int getBandwidthIndex() {
            return bandwidthIndex - 1;
        }

        public int getBandwidthResourceBlocks() {
            return resourceBlocks[bandwidthIndex];
        }

        public float getBandwidthMHz() {
            return  bandwidthMHz[bandwidthIndex];
        }

        public String getBandwidthString(boolean withUnit) {
            if (withUnit) {
                return String.format("%.01f MHz", getBandwidthMHz());
            } else {
                return String.format("%.01f", getBandwidthMHz());
            }
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
                int numRxAnt = (data[7] & 0x3) + 1;
                int numTxAnt = ((data[7] >> 2) & 0x7) + 1;
                int sysBandwidth = ((data[7] >> 5) & 0x7);

                int dataPos = 8;
                ChannelMatrices channelMatrices = new ChannelMatrices(sysBandwidth, numRxAnt, numTxAnt, numWhitenMatricesForCsf);
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

    public void setInterval(int interval) {
        this.interval = interval;
        if (!channelEstimationListeners.isEmpty()) {
            register(interval);
        }
    }

    public void addListener(ChannelEstimationListener channelEstimationListener) {
        if (channelEstimationListeners.isEmpty()) {
            register(interval); //register when we add the first listener
        }
        channelEstimationListeners.add(channelEstimationListener);
    }

    public void removeListener(ChannelEstimationListener channelEstimationListener) {
        channelEstimationListeners.remove(channelEstimationListener);
        if (channelEstimationListeners.isEmpty()) {
            register(interval); //deregister when we remove the last listener
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
     * @param interval interval of indications
     */
    private void register(int interval) {
        byte register[] = {(byte)((interval) & 0xFF), (byte)((interval >> 8) & 0xFF)};
        seemooQmi.sendMessage(CHANNEL_ESTIMATION_SVC_ID, true, register, register.length);
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
        register(0);
    }
}
