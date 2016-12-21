package de.tu_darmstadt.seemoo.seemooqcomlte.seemooqmi;

import android.content.Context;

import de.tu_darmstadt.seemoo.seemooqcomlte.R;

public abstract class SeemooQmiService {
    protected SeemooQmi seemooQmi;
    protected Context appContext;

    SeemooQmiService(SeemooQmi seemooQmi, Context appContext) {
        this.seemooQmi = seemooQmi;
        this.appContext = appContext;
        seemooQmi.addService(this);

        seemooQmi.notifyStatusListeners(appContext.getResources().getString(R.string.qmi_svc_start) + " " + getClass().getSimpleName(), 32);
    }

    public abstract void finalizeService();
}
