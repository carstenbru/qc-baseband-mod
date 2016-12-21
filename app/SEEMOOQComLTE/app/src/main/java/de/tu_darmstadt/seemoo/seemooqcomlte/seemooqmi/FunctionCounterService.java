package de.tu_darmstadt.seemoo.seemooqcomlte.seemooqmi;

import android.content.Context;

import java.util.EventListener;
import java.util.EventObject;
import java.util.LinkedList;
import java.util.List;

public class FunctionCounterService extends SeemooQmiService {
    public static final int FUNC_COUNTER_SVC_ID = 0x4675436F; //"FuCo" in ASCII

    private List<CounterUpdateListener> counterUpdateListeners = new LinkedList<CounterUpdateListener>();

    private SeemooQmi.PacketListener packetListener = new SeemooQmi.PacketListener() {
        @Override
        public void packetReceived(SeemooQmi.PacketReceiveEvent e) {
            byte[] data = e.getData();
            long funcCounters[] = new long[4];
            for (int i = 0; i < 4; i++) {
                funcCounters[i] = SeemooQmi.readIntLittleEndian(data, 4 + 4 * i);
            }
            notifyUpdateListeners(funcCounters);
        }
    };

    public interface CounterUpdateListener extends EventListener {
        void counterUpdate(CounterUpdateEvent e);
    }

    public class CounterUpdateEvent extends EventObject {
        private long funcCounters[];

        public CounterUpdateEvent(Object source, long[] funcCounters) {
            super(source);
            this.funcCounters = funcCounters;
        }

        public long getQmiPingSvcPingResponseCounter() {
            return funcCounters[0];
        }

        public long getMemcpyCounter() {
            return funcCounters[1];
        }

        public long getMemsetCounter() {
            return funcCounters[2];
        }

        public long getSnprintfCounter() {
            return funcCounters[3];
        }
    }

    public FunctionCounterService(SeemooQmi seemooQmi, Context appContext) {
        super(seemooQmi, appContext);
    }

    public void addListener(CounterUpdateListener counterUpdateListener) {
        if (counterUpdateListeners.isEmpty()) {
            seemooQmi.addPacketListener(FUNC_COUNTER_SVC_ID, false, packetListener);
        }
        counterUpdateListeners.add(counterUpdateListener);
    }

    public void removeListener(CounterUpdateListener counterUpdateListener) {
        counterUpdateListeners.remove(counterUpdateListener);
        if (counterUpdateListeners.isEmpty()) {
            seemooQmi.removePacketListener(FUNC_COUNTER_SVC_ID, false, packetListener);
        }
    }

    private void notifyUpdateListeners(long[] funcCounters) {
        for (CounterUpdateListener ul : counterUpdateListeners) {
            ul.counterUpdate(new CounterUpdateEvent(this, funcCounters));
        }
    }

    public void sendFuncCountersRequest() {
        seemooQmi.sendMessage(FUNC_COUNTER_SVC_ID, false, new byte[0], 0);
    }

    @Override
    public void finalizeService() {

    }
}
