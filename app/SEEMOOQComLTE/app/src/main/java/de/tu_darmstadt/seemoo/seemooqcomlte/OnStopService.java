package de.tu_darmstadt.seemoo.seemooqcomlte;

import android.app.Service;
import android.content.Intent;
import android.os.IBinder;

import de.tu_darmstadt.seemoo.seemooqcomlte.seemooqmi.SeemooQmi;

public class OnStopService extends Service {
    @Override
    public IBinder onBind(Intent intent) {
        return null;
    }

    @Override
    public int onStartCommand(Intent intent, int flags, int startId) {
        return START_NOT_STICKY;
    }

    public void onTaskRemoved(Intent rootIntent) {
        SeemooQmi seemooQmi = SeemooQmi.getInstance(getApplicationContext());
        seemooQmi.finalizeAllServices();

        stopSelf();
    }
}