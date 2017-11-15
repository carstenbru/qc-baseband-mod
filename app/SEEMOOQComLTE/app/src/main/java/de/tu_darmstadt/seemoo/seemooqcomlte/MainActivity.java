/**
 * Main Activity of the App
 *
 * @author Carsten Bruns (carst.bruns@gmx.de)
 */
package de.tu_darmstadt.seemoo.seemooqcomlte;

import android.app.AlertDialog;
import android.content.Context;
import android.content.DialogInterface;
import android.content.Intent;
import android.content.SharedPreferences;
import android.graphics.Color;
import android.location.Location;
import android.location.LocationListener;
import android.location.LocationManager;
import android.net.Uri;
import android.os.Bundle;
import android.os.Handler;
import android.os.StrictMode;
import android.preference.Preference;
import android.preference.PreferenceFragment;
import android.preference.PreferenceManager;
import android.support.design.widget.TabLayout;
import android.support.design.widget.TextInputLayout;
import android.support.v4.app.Fragment;
import android.support.v4.app.FragmentManager;
import android.support.v4.app.FragmentPagerAdapter;
import android.support.v4.view.ViewPager;
import android.support.v7.app.AppCompatActivity;
import android.support.v7.widget.Toolbar;
import android.telephony.CellIdentityLte;
import android.telephony.CellInfo;
import android.telephony.CellInfoLte;
import android.telephony.CellSignalStrengthLte;
import android.telephony.TelephonyManager;
import android.text.Editable;
import android.text.TextWatcher;
import android.view.LayoutInflater;
import android.view.Menu;
import android.view.MenuItem;
import android.view.View;
import android.view.View.OnClickListener;
import android.view.ViewGroup;
import android.widget.AdapterView;
import android.widget.ArrayAdapter;
import android.widget.Button;
import android.widget.CheckBox;
import android.widget.CompoundButton;
import android.widget.EditText;
import android.widget.ImageButton;
import android.widget.LinearLayout;
import android.widget.RadioButton;
import android.widget.RadioGroup;
import android.widget.Spinner;
import android.widget.TextView;
import android.widget.Toast;

import com.github.mikephil.charting.charts.LineChart;
import com.github.mikephil.charting.components.Legend;
import com.github.mikephil.charting.components.XAxis;
import com.github.mikephil.charting.components.YAxis;
import com.github.mikephil.charting.data.Entry;
import com.github.mikephil.charting.data.LineData;
import com.github.mikephil.charting.data.LineDataSet;
import com.github.mikephil.charting.interfaces.datasets.ILineDataSet;

import java.io.DataOutputStream;
import java.io.File;
import java.io.FileOutputStream;
import java.io.IOException;
import java.nio.ByteBuffer;
import java.nio.ByteOrder;
import java.text.SimpleDateFormat;
import java.util.ArrayList;
import java.util.Calendar;
import java.util.Date;
import java.util.LinkedList;
import java.util.List;
import java.util.Set;
import java.util.regex.Pattern;

import de.tu_darmstadt.seemoo.seemooqcomlte.seemooqmi.ChannelEstimationService;
import de.tu_darmstadt.seemoo.seemooqcomlte.seemooqmi.ComplexInteger;
import de.tu_darmstadt.seemoo.seemooqcomlte.seemooqmi.FunctionCounterService;
import de.tu_darmstadt.seemoo.seemooqcomlte.seemooqmi.LteMacService;
import de.tu_darmstadt.seemoo.seemooqcomlte.seemooqmi.LteSecService;
import de.tu_darmstadt.seemoo.seemooqcomlte.seemooqmi.MemAccessService;
import de.tu_darmstadt.seemoo.seemooqcomlte.seemooqmi.PdcchDumpService;
import de.tu_darmstadt.seemoo.seemooqcomlte.seemooqmi.SeemooQmi;
import de.tu_darmstadt.seemoo.seemooqcomlte.seemooqmi.SnprintfService;


/* ------------ open improvements ------------ */
//TODO refactor MainActivity class: put code in multiple files, maybe put GUI fragments in seperate files? one for each?
//TODO snprintf show write destination, choose destinations to show from list of received ones (filter)
//->store snprintf messages in service WITH destination ID

//TODO small GUI fixes
// -move settings icon (too far right, too close to text
// -when changing to a not scrollable tab and title bar is removed, we cannot get it again
// -keyboard shows up when de-focusing edit texts in lte mac tab
// -app icon
// -settings: group header color
/* ------------------------------------------- */


public class MainActivity extends AppCompatActivity {
    private static SeemooQmi seemooQmi = null;
    private static FunctionCounterService functionCounterService = null;
    private static SnprintfService snprintfService = null;
    private static LteMacService lteMacService = null;
    private static LteSecService lteSecService = null;
    private static MemAccessService memAccessService = null;
    private static ChannelEstimationService channelEstimationService = null;
    private static PdcchDumpService pdcchDumpService = null;

    private static SharedPreferences sharedPreferences = null;
    private static SharedPreferences.OnSharedPreferenceChangeListener onSharedPrederencesChangeListener = null;
    private static Preference graphMaxPreference = null;

    private static int openTab = 0;

    private static StatusLogFragment statusLogFragment;
    private static FunctionCountersFragment functionCountersFragment;

    private static int statusMessageMask = 39;
    private static boolean funcCountersPoll = false;
    private static int funcCountersPollRate;
    private static Handler funcCountersPollHandler = new Handler();
    private static boolean snprintfDeregister;

    /**
     * runnable to read function counters automatically at a specified interval
     */
    private static Runnable funcCountersPollRunnable = new Runnable() {
        @Override
        public void run() {
            if (funcCountersPoll) {
                functionCounterService.sendFuncCountersRequest();
                if (!funcCountersPollHandler.hasMessages(0)) {
                    funcCountersPollHandler.postDelayed(this, funcCountersPollRate);
                }
            }
        }
    };

    public static String byteArrToHexString(byte[] byteArr, boolean pythonString, int seperation) {
        return byteArrToHexString(byteArr, pythonString, seperation, 0, byteArr.length);
    }


    /**
     * converts an array of bytes to a ASCII hex string
     *
     * @param byteArr      data array
     * @param pythonString true to print in a format readable by python, false for human readable
     * @param seperation   number of bytes after which a space should be inserted
     * @param startOffset  index of the first byte to convert
     * @param length       number of bytes to convert
     * @return ASCII hex string of the input data
     */
    public static String byteArrToHexString(byte[] byteArr, boolean pythonString, int seperation, int startOffset, int length) {
        StringBuilder sb = new StringBuilder();
        if (!pythonString) {
            int count = 0;
            for (int i = startOffset; i < startOffset + length; i++) {
                sb.append(String.format("%02X", byteArr[i]));
                count++;
                if ((seperation != 0) && ((count % seperation) == 0)) {
                    sb.append(" ");
                }
            }
        } else {
            for (int i = startOffset; i < startOffset + length; i++) {
                sb.append(String.format("\\x%02X", byteArr[i]));
            }
        }

        return sb.toString();
    }

    /**
     * sets the snprintf service buffer size from the value in the preferneces
     */
    private void snprintfSetBufferSizeFromSharedPrefs() {
        snprintfService.setMaxOldMessagesSize(Integer.parseInt(sharedPreferences.getString("snprintf_buffer_size", "65536")));
    }

    /**
     * sets the SeemooQMI poll rate from the value in the preferneces
     */
    private void seemooQmiSetPollRateFromSharedPrefs() {
        seemooQmi.setPollRate(Integer.parseInt(sharedPreferences.getString("poll_interval", "100")));
    }

    /**
     * sets the Seemoo QMI status messages buffer size from the value in the preferneces
     */
    private void seemooQmiSetStatusBufferSizeFromSharedPrefs() {
        seemooQmi.setMaxOldMessagesSize(Integer.parseInt(sharedPreferences.getString("status_buffer_size", "1024")));
    }

    /**
     * sets the snprintf registration option from the value in the preferneces,
     * registers or deregisters from the service depending on the setting and the current
     * GUI state
     */
    private void snprintfDeregisterFromSharedPrefs() {
        snprintfDeregister = sharedPreferences.getBoolean("snprintf_deregister", true);
        if (snprintfDeregister) {
            snprintfService.register(openTab == 3);
        } else {
            snprintfService.register(true);
        }

    }

    /**
     * sets the function counter service poll tyope and rate from the value in the preferneces,
     * starts polling if it should be started and was not before
     */
    private void funcCountersPollFromSharedPrefs() {
        funcCountersPollRate = Integer.parseInt(sharedPreferences.getString("func_counter_poll_interval", "200"));

        boolean funcCountersPollOld = funcCountersPoll;
        funcCountersPoll = sharedPreferences.getBoolean("func_counter_poll", true);
        if (openTab == 2) {
            if (!funcCountersPollOld && funcCountersPoll) {
                if (!funcCountersPollHandler.hasMessages(0)) {
                    funcCountersPollHandler.postDelayed(funcCountersPollRunnable, funcCountersPollRate);
                }
            }
            if (functionCountersFragment != null) {
                functionCountersFragment.setUserVisibleHint(true);
            }
        }
    }

    /**
     * configures the status log level from the value in the preferneces
     */
    private void statusLogLevelFromSharedPrefs() {
        int messageMask = 0;
        Set<String> options = (sharedPreferences.getStringSet("log_level", null));
        if (options != null) {
            for (String s : options) {
                try {
                    messageMask |= Integer.parseInt(s);
                } catch (Exception e) {

                }
            }
            statusMessageMask = messageMask;
        }
        seemooQmi.setOldStatusMessagesMask(messageMask);
        if (statusLogFragment != null) {
            statusLogFragment.reregisterStatusListener(messageMask);
        }
    }

    /**
     * registers a listener for changes on the SharedPrefernces to trigger reconfiguration
     * of the corresponding parameters
     */
    private void registerOnSharedPreferenceChangedLister() {
        if (onSharedPrederencesChangeListener == null) {
            onSharedPrederencesChangeListener = new SharedPreferences.OnSharedPreferenceChangeListener() {
                @Override
                public void onSharedPreferenceChanged(SharedPreferences sharedPreferences, String s) {
                    switch (s) {
                        case "poll_interval":
                            seemooQmiSetPollRateFromSharedPrefs();
                            return;
                        case "status_buffer_size":
                            seemooQmiSetStatusBufferSizeFromSharedPrefs();
                            return;
                        case "snprintf_buffer_size":
                            snprintfSetBufferSizeFromSharedPrefs();
                            return;
                        case "snprintf_deregister":
                            snprintfDeregisterFromSharedPrefs();
                            return;
                        case "func_counter_poll":
                        case "func_counter_poll_interval":
                            funcCountersPollFromSharedPrefs();
                            return;
                        case "log_level":
                            statusLogLevelFromSharedPrefs();
                            return;
                        case "csi_interval":
                            channelEstimationService.setInterval(Integer.parseInt(sharedPreferences.getString("csi_interval", "1")));
                            return;
                        case "csi_autoscale":
                            graphMaxPreference.setEnabled(!sharedPreferences.getBoolean("csi_autoscale", true));
                            return;
                    }
                }
            };
            sharedPreferences.registerOnSharedPreferenceChangeListener(onSharedPrederencesChangeListener);
        }
    }

    /**
     * initializes all parameters from the shared preferences
     */
    private void initParameters() {
        seemooQmiSetPollRateFromSharedPrefs();
        seemooQmiSetStatusBufferSizeFromSharedPrefs();
        snprintfSetBufferSizeFromSharedPrefs();
        snprintfDeregisterFromSharedPrefs();
        funcCountersPollFromSharedPrefs();
    }

    /**
     * creates the QMI services (including SeemooQmi which are not yet running
     */
    private void createServices() {
        seemooQmi = SeemooQmi.getInstance(getApplicationContext());
        statusLogLevelFromSharedPrefs();

        if (functionCounterService == null) {
            functionCounterService = new FunctionCounterService(seemooQmi, getApplicationContext());
        }

        if (snprintfService == null) {
            snprintfService = new SnprintfService(seemooQmi, getApplicationContext());
        }

        if (lteMacService == null) {
            lteMacService = new LteMacService(seemooQmi, getApplicationContext());
        }

        if (lteSecService == null) {
            lteSecService = new LteSecService(seemooQmi, getApplicationContext());
        }

        if (memAccessService == null) {
            memAccessService = new MemAccessService(seemooQmi, getApplicationContext());
        }

        if (channelEstimationService == null) {
            channelEstimationService = new ChannelEstimationService(seemooQmi, getApplicationContext());
            channelEstimationService.setInterval(Integer.parseInt(sharedPreferences.getString("csi_interval", "1")));
        }

        if (pdcchDumpService == null) {
            pdcchDumpService = new PdcchDumpService(seemooQmi, getApplicationContext());
        }
    }

    /**
     * stores a value in the default shared preferences
     *
     * @param key   name of the value
     * @param value actual value
     */
    private static void storeStringInSharedPrefs(String key, String value) {
        SharedPreferences.Editor editor = sharedPreferences.edit();
        editor.putString(key, value);
        editor.commit();
    }

    /**
     * stores a integer in the default shared preferences
     *
     * @param key   name of the value
     * @param value actual value
     */
    private static void storeIntInSharedPrefs(String key, int value) {
        SharedPreferences.Editor editor = sharedPreferences.edit();
        editor.putInt(key, value);
        editor.commit();
    }

    /**
     * stores a boolean in the default shared preferences
     *
     * @param key   name of the value
     * @param value actual value
     */
    private static void storeBooleanInSharedPrefs(String key, boolean value) {
        SharedPreferences.Editor editor = sharedPreferences.edit();
        editor.putBoolean(key, value);
        editor.commit();
    }

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);

        //set-up tabs
        SeemooPagerAdapter seemooPagerAdapter = new SeemooPagerAdapter(getSupportFragmentManager(), getApplicationContext());
        ViewPager viewPager = (ViewPager) findViewById(R.id.viewPager);
        viewPager.setAdapter(seemooPagerAdapter);
        viewPager.setOffscreenPageLimit(7);
        viewPager.setCurrentItem(openTab);
        viewPager.addOnPageChangeListener(new ViewPager.SimpleOnPageChangeListener() {
            @Override
            public void onPageSelected(int position) {
                super.onPageSelected(position);

                openTab = position;
            }
        });

        TabLayout tabLayout = (TabLayout) findViewById(R.id.tabs);
        tabLayout.setupWithViewPager(viewPager);

        Toolbar toolbar = (Toolbar) findViewById(R.id.toolbar);
        setSupportActionBar(toolbar);

        sharedPreferences = PreferenceManager.getDefaultSharedPreferences(this);

        //start services, initialize everything
        createServices();
        initParameters();
        registerOnSharedPreferenceChangedLister();

        seemooQmi.start();
        startService(new Intent(getBaseContext(), OnStopService.class));

        StrictMode.ThreadPolicy policy = new StrictMode.ThreadPolicy.Builder().permitAll().build();
        StrictMode.setThreadPolicy(policy);
    }


    @Override
    public boolean onCreateOptionsMenu(Menu menu) {
        getMenuInflater().inflate(R.menu.menu_main, menu);
        return true;
    }

    /**
     * fragment containing the settings screen
     */
    public static class MainSettingsFragment extends PreferenceFragment {
        private LinearLayout root;

        @Override
        public void onCreate(Bundle savedInstanceState) {
            super.onCreate(savedInstanceState);
            addPreferencesFromResource(R.xml.preferences);
        }

        @Override
        public void onActivityCreated(Bundle savedInstanceState) {
            super.onActivityCreated(savedInstanceState);

            //set frgaments background color,otherwise it is transparent!
            getView().setBackgroundColor(getResources().getColor(android.R.color.background_light));
            getView().setClickable(true);

            //add the toolbar with back button
            root = (LinearLayout) getView().findViewById(android.R.id.list).getParent().getParent().getParent();
            Toolbar bar = (Toolbar) getActivity().getLayoutInflater().inflate(R.layout.prefernce_toolbar, root, false);
            root.addView(bar, 0);
            bar.setNavigationOnClickListener(new View.OnClickListener() {
                @Override
                public void onClick(View v) {
                    getActivity().getFragmentManager().popBackStack();
                }
            });

            graphMaxPreference = findPreference("csi_graph_max");
            graphMaxPreference.setEnabled(!sharedPreferences.getBoolean("csi_autoscale", true));
        }

        @Override
        public void onDestroy() {
            super.onDestroy();

            //remove toolbar
            root.removeViewAt(0);
        }
    }

    @Override
    public boolean onOptionsItemSelected(MenuItem item) {
        int id = item.getItemId();

        //start settings screen
        if (id == R.id.action_settings) {
            getFragmentManager()
                    .beginTransaction()
                    .replace(android.R.id.content, new MainSettingsFragment())
                    .addToBackStack("settings")
                    .commit();
            return true;
        }

        return super.onOptionsItemSelected(item);
    }

    /**
     * pager adapter to handle tabs consisting of fragments for the different pages
     */
    public static class SeemooPagerAdapter extends FragmentPagerAdapter {
        private static int NUM_TABS = 8;

        private Context appContext;

        /**
         * Constructor
         *
         * @param fm         the FragmentManager
         * @param appContext application context
         */
        public SeemooPagerAdapter(FragmentManager fm, Context appContext) {
            super(fm);

            this.appContext = appContext;
        }

        @Override
        public Fragment getItem(int i) {
            switch (i) {
                case 0:
                    statusLogFragment = new StatusLogFragment();
                    return statusLogFragment;
                case 1:
                    return new PdcchDumpFragment();
                case 2:
                    functionCountersFragment = new FunctionCountersFragment();
                    return functionCountersFragment;
                case 3:
                    return new SnprintfFragment();
                case 4:
                    return new MemAccessFragment();
                case 5:
                    return new LteMacFragment();
                case 6:
                    return new LteSecFragment();
                case 7:
                    return new ChannelEstimationFragment();
                default:
                    return null;
            }
        }

        @Override
        public int getCount() {
            return NUM_TABS;
        }

        @Override
        public CharSequence getPageTitle(int position) {
            switch (position) {
                case 0:
                    return appContext.getResources().getString(R.string.tab_status_log);
                case 1:
                    return appContext.getResources().getString(R.string.tab_pdcch_dump);
                case 2:
                    return appContext.getResources().getString(R.string.tab_func_counters);
                case 3:
                    return appContext.getResources().getString(R.string.tab_snprintf);
                case 4:
                    return appContext.getResources().getString(R.string.tab_mem_access);
                case 5:
                    return appContext.getResources().getString(R.string.tab_lte_mac);
                case 6:
                    return appContext.getResources().getString(R.string.tab_lte_sec);
                case 7:
                    return appContext.getResources().getString(R.string.tab_channel_estimation);
                default:
                    return "error";
            }
        }
    }

    /**
     * Fragment showing status messages
     */
    public static class StatusLogFragment extends Fragment {
        private SeemooQmi.StatusListener statusListener;
        private int lastStatusLogPos = 0;


        private void addMsgToStatusLog(TextView statusLog, String msg, boolean replacesLast) {
            if (replacesLast && (lastStatusLogPos < statusLog.getText().length())) {
                CharSequence cs = statusLog.getText();
                cs = cs.subSequence(0, lastStatusLogPos);
                statusLog.setText(cs);
            }
            lastStatusLogPos = statusLog.getText().length();
            statusLog.append(msg);
            statusLog.append("\n");
        }

        @Override
        public View onCreateView(LayoutInflater inflater, ViewGroup container, Bundle savedInstanceState) {
            View rootView = inflater.inflate(R.layout.content_status, container, false);

            final TextView statusLog = (TextView) rootView.findViewById(R.id.statusLog);
            Button clearStatusLogButton = (Button) rootView.findViewById(R.id.statusLogClearButton);
            clearStatusLogButton.setOnClickListener(new OnClickListener() {
                @Override
                public void onClick(View view) {
                    statusLog.setText("");
                }
            });
            //put old messages in the log (received before creation of this fragment
            //or when activity was send to background)
            for (String m : seemooQmi.getOldStatusMessages()) {
                addMsgToStatusLog(statusLog, m, false);
            }
            statusListener = new SeemooQmi.StatusListener() {
                @Override
                public void statusUpdate(SeemooQmi.StatusUpdateEvent e) {
                    addMsgToStatusLog(statusLog, e.getStatus(), e.replacesLast());
                }
            };
            //register listner for incoming messages
            reregisterStatusListener(statusMessageMask);

            return rootView;
        }

        /**
         * reregisters (or registers) the StatusLister to SeemooQmi
         *
         * @param messageMask mask defining which message types should be send to the listener
         */
        public void reregisterStatusListener(int messageMask) {
            seemooQmi.removeStatusListener(statusListener);
            seemooQmi.addStatusListener(statusListener, messageMask);
        }

        @Override
        public void onDestroy() {
            super.onDestroy();

            seemooQmi.removeStatusListener(statusListener);
        }
    }

    /**
     * fragment for the PDCCH dump service
     */
    public static class PdcchDumpFragment extends Fragment {
        private static final int PDCCH_DATA_RECORD = 0;
        private static final int PDCCH_GPS_RECORD = 1;
        private static final int PDCCH_TIME_RECORD = 2;
        private static final int PDCCH_MAIN_CELL_INFO_RECORD = 3;
        private static final int PDCCH_ADD_CELL_INFO_RECORD = 4;

        private static final int PDCCH_MAIN_CELL_INFO_RECORD_VERSION = 0;
        private static final int PDCCH_GPS_RECORD_VERSION = 0;

        private PdcchDumpService.PdcchDumpListener pdcchDumpListener;

        private boolean pdcchDumpActive = false;
        private String pdcchDumpFilename;
        private DataOutputStream pdcchDumpStream;
        private int pddchDumpSubFile;
        private int pddchDumpSplitSize;

        private LocationManager locationManager;
        private LocationListener locationListener;

        private int timeRecordRate;
        private Handler timeRecordHandler = new Handler();

        private int cellInfoRecordRate;
        private Handler cellInfoRecordHandler = new Handler();

        private TelephonyManager telephonyManager;

        private float bandwidthMHz[] = { 1.4f, 3, 5, 10, 15, 20 };

        private boolean showCfiCounter;
        private int cfiCounters[] = { 0, 0, 0 };

        private Runnable timeRecordRunnable = new Runnable() {
            @Override
            public void run() {
                if (pdcchDumpActive) {
                    ByteBuffer buffer = ByteBuffer.allocate(8);
                    buffer.order(ByteOrder.LITTLE_ENDIAN);

                    Date currentTime = Calendar.getInstance().getTime();

                    buffer.putLong(currentTime.getTime());

                    byte[] time = buffer.array();;
                    writeRecord(PDCCH_TIME_RECORD, 0, time, 0, time.length);

                    if (!timeRecordHandler.hasMessages(0)) {
                        timeRecordHandler.postDelayed(this, timeRecordRate);
                    }
                }
            }
        };

        private Runnable cellInfoRecordRunnable = new Runnable() {
            @Override
            public void run() {
                if (pdcchDumpActive) {
                    pdcchDumpService.requestCellInfo();

                    if (!cellInfoRecordHandler.hasMessages(0)) {
                        cellInfoRecordHandler.postDelayed(this, cellInfoRecordRate);
                    }
                }
            }
        };

        private void setupFilenameEdit(View rootView) {
            EditText filenameEdit = (EditText) rootView.findViewById(R.id.pdcchDestFileName);
            filenameEdit.setText(sharedPreferences.getString("PdcchDestFilename", ""));
            filenameEdit.addTextChangedListener(new TextWatcher() {
                @Override
                public void onTextChanged(CharSequence s, int start, int before, int count) {
                }

                @Override
                public void beforeTextChanged(CharSequence s, int start, int count, int after) {
                }

                @Override
                public void afterTextChanged(Editable s) {
                    storeStringInSharedPrefs("PdcchDestFilename", s.toString());
                }
            });
        }

        private void setupGpsBox(View rootView) {
            CheckBox pdcchGps = (CheckBox) rootView.findViewById(R.id.pdcchGpsBox);
            pdcchGps.setChecked(sharedPreferences.getBoolean("PdcchGpsInclude", true));
            pdcchGps.setOnCheckedChangeListener(new CompoundButton.OnCheckedChangeListener() {
                @Override
                public void onCheckedChanged(CompoundButton compoundButton, boolean b) {
                    storeBooleanInSharedPrefs("PdcchGpsInclude", b);
                }
            });
        }

        /**
         * sets the states of all GUI elements according to the dumping state
         *
         * @param view      root view of the fragment
         * @param pdcchDumpActive dumping state
         */
        private void setPdcchDumpGuiState(View view, boolean pdcchDumpActive) {
            EditText filenameEdit = (EditText) view.findViewById(R.id.pdcchDestFileName);
            CheckBox pdcchDumpEnable = (CheckBox) view.findViewById(R.id.activatePdcchDump);
            CheckBox pdcchGps = (CheckBox) view.findViewById(R.id.pdcchGpsBox);

            filenameEdit.setEnabled(!pdcchDumpActive);
            pdcchGps.setEnabled(!pdcchDumpActive);
            pdcchDumpEnable.setChecked(pdcchDumpActive);
            this.pdcchDumpActive = pdcchDumpActive;
        }

        /**
         * configures the dump enable checkbox
         *
         * @param rootView root view of the fragment
         */
        private void setupPdcchDumpCheckbox(View rootView) {
            final CheckBox pdcchDumpEnable = (CheckBox) rootView.findViewById(R.id.activatePdcchDump);
            final CheckBox pdcchGps = (CheckBox) rootView.findViewById(R.id.pdcchGpsBox);
            pdcchDumpEnable.setOnCheckedChangeListener(new CompoundButton.OnCheckedChangeListener() {
                private void startDumping(CompoundButton compoundButton, File file) {
                    showCfiCounter = sharedPreferences.getBoolean("pdcch_cfi_counters", false);
                    for (int i = 0; i < 3; i++) {
                        cfiCounters[i] = 0;
                    }
                    try {
                        file.getParentFile().mkdirs();
                        file.createNewFile();
                        pdcchDumpStream = new DataOutputStream(new FileOutputStream(file));

                        setPdcchDumpGuiState(getView(), true);
                        pdcchDumpService.register(true);
                    } catch (IOException ioe) {
                        pdcchDumpEnable.setChecked(false);
                        Toast.makeText(compoundButton.getContext(), getResources().getString(R.string.fileError), Toast.LENGTH_SHORT).show();
                    }

                    timeRecordRate = Integer.parseInt(sharedPreferences.getString("pdcch_dump_time_interval", "5000"));
                    if (!timeRecordHandler.hasMessages(0)) {
                        timeRecordHandler.postDelayed(timeRecordRunnable, 0);
                    }

                    cellInfoRecordRate = Integer.parseInt(sharedPreferences.getString("pdcch_dump_cell_info_interval", "5000"));
                    if (!cellInfoRecordHandler.hasMessages(0)) {
                        cellInfoRecordHandler.postDelayed(cellInfoRecordRunnable, cellInfoRecordRate);
                    }

                    if (pdcchGps.isChecked()) {
                        //TODO check if GPS fix, if not ask if want to wait for fix
                        try {
                            int gpsInterval = Integer.parseInt(sharedPreferences.getString("pdcch_dump_gps_interval", "1000"));
                            locationManager.requestLocationUpdates(LocationManager.GPS_PROVIDER, gpsInterval, 0, locationListener);
                        } catch (SecurityException se) {
                        }
                    }
                }

                @Override
                public void onCheckedChanged(final CompoundButton compoundButton, boolean b) {
                    EditText filenameEdit = (EditText) getView().findViewById(R.id.pdcchDestFileName);

                    if (b) {
                        pdcchDumpFilename = filenameEdit.getText().toString();
                        if (pdcchDumpFilename.isEmpty()) {
                            pdcchDumpEnable.setChecked(false);
                            Toast.makeText(compoundButton.getContext(), getResources().getString(R.string.filenameEmpty), Toast.LENGTH_SHORT).show();
                        } else {
                            String filename = pdcchDumpFilename;
                            pddchDumpSubFile = 0;
                            if (sharedPreferences.getBoolean("pdcch_split_dump", true)) {
                                pddchDumpSplitSize = Integer.parseInt(sharedPreferences.getString("max_pdcch_file_size", "2000"));
                                filename = filename + "0";
                            } else {
                                pddchDumpSplitSize = 0;
                            }
                            final File file = new File(filename);
                            if (!file.exists()) {
                                startDumping(compoundButton, file);
                            } else {
                                AlertDialog.Builder alertDialog = new AlertDialog.Builder(getActivity());

                                alertDialog.setTitle("File already exists");
                                alertDialog.setMessage("The filename you entered already exists. Do you want to overwrite it?");

                                alertDialog.setPositiveButton("Yes", new DialogInterface.OnClickListener() {

                                    public void onClick(DialogInterface dialog, int which) {
                                        if (pddchDumpSplitSize != 0) {
                                            int i = 0;
                                            File delFile = file;
                                            while (delFile.exists()) {
                                                delFile.delete();
                                                i++;
                                                delFile = new File(pdcchDumpFilename + i);
                                            }
                                        }
                                        startDumping(compoundButton, file);
                                    }
                                });
                                alertDialog.setNegativeButton("No", new DialogInterface.OnClickListener() {

                                    @Override
                                    public void onClick(DialogInterface dialog, int which) {
                                        dialog.cancel();

                                        setPdcchDumpGuiState(getView(), false);
                                    }
                                });

                                alertDialog.show();
                            }
                        }

                    } else {
                        try {
                            if (pdcchDumpStream != null) {
                                pdcchDumpStream.close();
                            }
                        } catch (IOException ioe) {
                        }

                        //when un-checked stop PDCCH dumping
                        pdcchDumpService.register(false);
                        setPdcchDumpGuiState(getView(), false);

                        try {
                            locationManager.removeUpdates(locationListener);
                        } catch (SecurityException se) {
                        }
                    }
                }
            });
        }

        /**
         * writes a record header to the output stream
         *
         * @param recordType unique record type
         * @param recordVersion version of the record
         * @param data record data
         * @param offset start offset of record data to write
         * @param length number of bytes in this record (without header)
         */
        public void writeRecord(int recordType, int recordVersion, byte[] data, int offset, int length) { //TODO ensure that not different data appears interleaved in output...mutex..
            if (pddchDumpSplitSize != 0) { //file splitting active?
                if (pdcchDumpStream.size() + length + 8 > pddchDumpSplitSize * 1000 * 1000) { //file got too big, close it and create next one
                    try {
                        pdcchDumpStream.close();

                        pddchDumpSubFile++;
                        String filename = pdcchDumpFilename + pddchDumpSubFile;
                        File file = new File(filename);
                        file.createNewFile();
                        pdcchDumpStream = new DataOutputStream(new FileOutputStream(file));
                    } catch (IOException ioe) {
                    }
                }
            }

            byte[] header = new byte[8];
            SeemooQmi.writeIntLittleEndian(length + 8, header, 0);
            SeemooQmi.writeIntLittleEndian((recordType << 16) | recordVersion, header, 4);
            try {
                pdcchDumpStream.write(header);
                pdcchDumpStream.write(data, offset, length);
            } catch (IOException ioe) {
            }
        }

        @Override
        public View onCreateView(LayoutInflater inflater, ViewGroup container, Bundle savedInstanceState) {
            final View rootView = inflater.inflate(R.layout.content_pdcch_dump, container, false);

            setupFilenameEdit(rootView);
            setupGpsBox(rootView);
            setPdcchDumpGuiState(rootView, pdcchDumpActive);
            setupPdcchDumpCheckbox(rootView);

            final TextView pdcchCellInfoView = (TextView) rootView.findViewById(R.id.pdcchCellInfo);
            //new dump listener
            pdcchDumpListener = new PdcchDumpService.PdcchDumpListener() {
                @Override
                public void newDumpData(PdcchDumpService.PdcchDumpEvent e) {
                    long dumpRecordVersion = SeemooQmi.readIntLittleEndian(e.getData(), 4);
                    writeRecord(PDCCH_DATA_RECORD, (int)dumpRecordVersion, e.getData(), 8, e.getDataLength() - 8);

                    if (showCfiCounter) {
                        int cfiIndex = (e.getData()[11] >> 6) & 0x3;
                        cfiCounters[cfiIndex]++;
                    }
                }

                @Override
                public void newCellInfo(PdcchDumpService.PdcchCellInfoEvent e) {
                    ByteBuffer buffer = ByteBuffer.allocate(20);
                    buffer.order(ByteOrder.LITTLE_ENDIAN);

                    List<CellInfo> cellInfoList = telephonyManager.getAllCellInfo();
                    for (CellInfo cellInfo : cellInfoList)
                    {
                        if (cellInfo.isRegistered()) {
                            if (cellInfo instanceof CellInfoLte) {
                                CellInfoLte cellInfoLte = (CellInfoLte) cellInfo;
                                CellIdentityLte cellIdentityLte = cellInfoLte.getCellIdentity();
                                CellSignalStrengthLte cellSignalStrengthLte = cellInfoLte.getCellSignalStrength();

                                // [28:20] phy cell ID
                                // [19:10] MCC
                                // [9:0] MNC
                                buffer.putInt((cellIdentityLte.getPci() << 20) |
                                        (cellIdentityLte.getMcc() << 10) |
                                        cellIdentityLte.getMnc());
                                // 16bit TAC
                                buffer.putInt(cellIdentityLte.getTac());
                                // 28bit CID
                                buffer.putInt(cellIdentityLte.getCi());

                                // [20:16] TA (timing advance)
                                // [6:0] ASU
                                buffer.putInt((cellSignalStrengthLte.getTimingAdvance() << 16) |
                                        cellSignalStrengthLte.getAsuLevel());
                                // 32bit (?) dBm
                                buffer.putInt(cellSignalStrengthLte.getDbm());

                                byte[] data = buffer.array();
                                writeRecord(PDCCH_MAIN_CELL_INFO_RECORD, PDCCH_MAIN_CELL_INFO_RECORD_VERSION, data, 0, data.length);

                                //show cell info in GUI
                                StringBuilder sb = new StringBuilder();
                                Date currentTime = Calendar.getInstance().getTime();
                                SimpleDateFormat format = new SimpleDateFormat("HH:mm:ss");
                                sb.append("Last update: ").append(format.format(currentTime)).append("\n");
                                sb.append("MCC: ").append(cellIdentityLte.getMcc()).append("\n");
                                sb.append("MNC: ").append(cellIdentityLte.getMnc()).append("\n");
                                sb.append("TAC: ").append(cellIdentityLte.getTac()).append("\n");
                                sb.append("CID: ").append(cellIdentityLte.getCi()).append("\n");
                                sb.append("Physical cell ID: ").append(cellIdentityLte.getPci()).append("\n");
                                if (cellSignalStrengthLte.getTimingAdvance() != Integer.MAX_VALUE) {
                                    sb.append("Timing Advance: ").append(cellSignalStrengthLte.getTimingAdvance()).append("\n");
                                }
                                sb.append("Signal strength: ").append(cellSignalStrengthLte.getDbm()).append("dBm\n");
                                int bandwidthIdx = e.getData()[10] & 0x7;
                                sb.append("Bandwidth: ").append(String.format("%.1f", bandwidthMHz[bandwidthIdx])).append("MHz\n");
                                int earfcn = (int)(SeemooQmi.readIntLittleEndian(e.getData(), 272) & 0xFFFF);
                                sb.append("EARFCN: ").append(earfcn).append("\n");
                                if (showCfiCounter) {
                                    for (int i = 0; i < 3; i++) {
                                        sb.append("CFI").append(i+1).append(": ").append(cfiCounters[i]).append("\n");
                                    }
                                }
                                pdcchCellInfoView.setText(sb.toString());
                            }
                        }
                    }

                    long cellInfoRecordVersion = SeemooQmi.readIntLittleEndian(e.getData(), 4);
                    writeRecord(PDCCH_ADD_CELL_INFO_RECORD, (int)cellInfoRecordVersion, e.getData(), 8, e.getDataLength() - 8);
                }
            };
            pdcchDumpService.addListener(pdcchDumpListener);

            locationManager = (LocationManager)getContext().getSystemService(Context.LOCATION_SERVICE);
            locationListener = new LocationListener() {
                @Override
                public void onLocationChanged(Location location) {
                    ByteBuffer buffer = ByteBuffer.allocate(44);
                    buffer.order(ByteOrder.LITTLE_ENDIAN);

                    buffer.putDouble(location.getLatitude());
                    buffer.putDouble(location.getLongitude());
                    buffer.putDouble(location.getAltitude());
                    buffer.putFloat(location.getBearing());
                    buffer.putFloat(location.getAccuracy());
                    buffer.putFloat(location.getSpeed());
                    buffer.putLong(location.getTime());

                    byte[] data = buffer.array();
                    writeRecord(PDCCH_GPS_RECORD, PDCCH_GPS_RECORD_VERSION, data, 0, data.length);
                }

                @Override
                public void onStatusChanged(String s, int i, Bundle bundle) {

                }

                @Override
                public void onProviderEnabled(String s) {

                }

                @Override
                public void onProviderDisabled(String s) {

                }
            };

            telephonyManager = (TelephonyManager) getContext().getSystemService(Context.TELEPHONY_SERVICE);

            return rootView;
        }

        @Override
        public void onDestroy() {
            super.onDestroy();

            pdcchDumpService.removeListener(pdcchDumpListener);
            pdcchDumpService.register(false);
            try {
                locationManager.removeUpdates(locationListener);
            } catch (SecurityException se) {
            }
        }
    }

    /**
     * fragment showing the readings of the function counter service
     */
    public static class FunctionCountersFragment extends Fragment {
        FunctionCounterService.CounterUpdateListener counterUpdateListener;

        @Override
        public View onCreateView(LayoutInflater inflater, ViewGroup container, Bundle savedInstanceState) {
            final View rootView = inflater.inflate(R.layout.content_func_counters, container, false);

            //configure request button
            Button readFuncCountersButton = (Button) rootView.findViewById(R.id.readFuncCountersButton);
            readFuncCountersButton.setOnClickListener(new OnClickListener() {
                @Override
                public void onClick(View arg0) {
                    functionCounterService.sendFuncCountersRequest();
                }
            });
            readFuncCountersButton.setVisibility(funcCountersPoll ? View.GONE : View.VISIBLE);

            //update listener
            counterUpdateListener = new FunctionCounterService.CounterUpdateListener() {
                @Override
                public void counterUpdate(FunctionCounterService.CounterUpdateEvent e) {
                    TextView funcCountersView = (TextView) rootView.findViewById(R.id.funcCountersView);

                    StringBuilder stringBuilder = new StringBuilder();
                    stringBuilder.append("memcpy:\t\t").append(e.getMemcpyCounter()).append("\n");
                    stringBuilder.append("memset:\t\t").append(e.getMemsetCounter()).append("\n");
                    stringBuilder.append("snprintf:\t\t").append(e.getSnprintfCounter()).append("\n");
                    stringBuilder.append("qmi_ping_svc_ping_response:\t").append(e.getQmiPingSvcPingResponseCounter()).append("\n");
                    funcCountersView.setText(stringBuilder.toString());
                }
            };
            functionCounterService.addListener(counterUpdateListener);

            //send a first request to have an initial value
            functionCounterService.sendFuncCountersRequest();

            return rootView;
        }

        @Override
        public void setUserVisibleHint(boolean isVisibleToUser) {
            super.setUserVisibleHint(isVisibleToUser);

            if (isVisibleToUser) {
                //start polling
                if (!funcCountersPollHandler.hasMessages(0)) {
                    funcCountersPollHandler.postDelayed(funcCountersPollRunnable, funcCountersPollRate);
                }

                //set button visibility correctly when tab is opened
                if (getView() != null) {
                    Button readFuncCountersButton = (Button) getView().findViewById(R.id.readFuncCountersButton);
                    readFuncCountersButton.setVisibility(funcCountersPoll ? View.GONE : View.VISIBLE);
                }
            }
        }

        @Override
        public void onDestroy() {
            super.onDestroy();

            functionCounterService.removeListener(counterUpdateListener);
        }
    }

    /**
     * fragment showing snprintf messages
     */
    public static class SnprintfFragment extends Fragment {
        private boolean firstSetVisible = true;
        private SnprintfService.SnprintfListener snprintfListener;

        @Override
        public View onCreateView(LayoutInflater inflater, ViewGroup container, Bundle savedInstanceState) {
            View rootView = inflater.inflate(R.layout.content_snprintf, container, false);

            final TextView snprintfLog = (TextView) rootView.findViewById(R.id.snprintfLog);
            for (String m : snprintfService.getCachedMessages()) {
                snprintfLog.append(m);
            }
            snprintfListener = new SnprintfService.SnprintfListener() {
                @Override
                public void statusUpdate(SnprintfService.SnprintfMessageEvent e) {
                    snprintfLog.append(e.getMessage());
                }
            };
            snprintfService.addListener(snprintfListener);

            return rootView;
        }

        @Override
        public void setUserVisibleHint(boolean isVisibleToUser) {
            super.setUserVisibleHint(isVisibleToUser);

            //do not trigger this code at creation of the fragment..
            if (!firstSetVisible) {
                if (snprintfDeregister) {
                    //de(register) on tab changes
                    snprintfService.register(isVisibleToUser);
                }
            }
            firstSetVisible = false;
        }

        @Override
        public void onDestroy() {
            super.onDestroy();

            snprintfService.removeListener(snprintfListener);
        }
    }

    /**
     * fragment for GUI for LTE MAC
     */
    public static class LteMacFragment extends Fragment {
        private static final Pattern PARTIAl_IP_ADDRESS =
                Pattern.compile("^((25[0-5]|2[0-4][0-9]|[0-1][0-9]{2}|[1-9][0-9]|[0-9])\\.){0,3}" +
                        "((25[0-5]|2[0-4][0-9]|[0-1][0-9]{2}|[1-9][0-9]|[0-9])){0,1}$");

        private static final Pattern IP_ADDRESS = Pattern.compile(
                "^(([01]?\\d\\d?|2[0-4]\\d|25[0-5])\\.){3}([01]?\\d\\d?|2[0-4]\\d|25[0-5])$");

        private static boolean lteMacUdpActive = false;

        /**
         * sets the states of all GUI elements according to the UDP forwarding state
         *
         * @param view      root view of the fragment
         * @param udpActive UDP forwarding state
         */
        private void setLteMacUdpGuiState(View view, boolean udpActive) {
            EditText lteMacDestIp = (EditText) view.findViewById(R.id.destIp);
            EditText lteMacDestPort = (EditText) view.findViewById(R.id.destPort);
            CheckBox lteMacUdpEnable = (CheckBox) view.findViewById(R.id.activateUdp);

            lteMacDestIp.setEnabled(!udpActive);
            lteMacDestPort.setEnabled(!udpActive);
            lteMacUdpEnable.setChecked(udpActive);
            lteMacUdpActive = udpActive;
        }

        /**
         * configures the destination IP address edit field,
         * including a pattern to allow only valid IP addresses
         *
         * @param rootView root view of the fragment
         */
        private void setupDestIpEdit(View rootView) {
            EditText lteMacDestIp = (EditText) rootView.findViewById(R.id.destIp);
            lteMacDestIp.setText(sharedPreferences.getString("LteMacServiceIp", ""));
            lteMacDestIp.addTextChangedListener(new TextWatcher() {
                @Override
                public void onTextChanged(CharSequence s, int start, int before, int count) {
                }

                @Override
                public void beforeTextChanged(CharSequence s, int start, int count, int after) {
                }

                private String previousText = "";

                @Override
                public void afterTextChanged(Editable s) {
                    if (PARTIAl_IP_ADDRESS.matcher(s).matches()) {
                        previousText = s.toString();
                        storeStringInSharedPrefs("LteMacServiceIp", s.toString());
                    } else {
                        s.replace(0, s.length(), previousText);
                    }
                }
            });
        }

        /**
         * configures the destination port address edit field,
         * allowing only valid ports
         *
         * @param rootView root view of the fragment
         */
        private void setupDestPortEdit(View rootView) {
            EditText lteMacDestPort = (EditText) rootView.findViewById(R.id.destPort);
            lteMacDestPort.setText(sharedPreferences.getString("LteMacServiceUdpPort", ""));
            lteMacDestPort.addTextChangedListener(new TextWatcher() {
                @Override
                public void onTextChanged(CharSequence s, int start, int before, int count) {
                }

                @Override
                public void beforeTextChanged(CharSequence s, int start, int count, int after) {
                }

                private String previousText = "";

                @Override
                public void afterTextChanged(Editable s) {
                    Integer val;
                    try {
                        val = Integer.parseInt(s.toString());
                    } catch (Exception e) {
                        val = 0;
                    }
                    if ((val >= 0) && (val <= 65535) && ((s.length() == 0) || (s.charAt(0) != '0'))) {
                        previousText = s.toString();
                        storeStringInSharedPrefs("LteMacServiceUdpPort", s.toString());
                    } else {
                        s.replace(0, s.length(), previousText);
                    }
                }
            });
        }

        /**
         * configures the UDP enable checkbox
         *
         * @param rootView root view of the fragment
         */
        private void setupUdpCheckbox(View rootView) {
            final CheckBox lteMacUdpEnable = (CheckBox) rootView.findViewById(R.id.activateUdp);
            lteMacUdpEnable.setOnCheckedChangeListener(new CompoundButton.OnCheckedChangeListener() {
                @Override
                public void onCheckedChanged(CompoundButton compoundButton, boolean b) {
                    EditText lteMacDestIp = (EditText) getView().findViewById(R.id.destIp);
                    EditText lteMacDestPort = (EditText) getView().findViewById(R.id.destPort);


                    if (b) {
                        String ip = lteMacDestIp.getText().toString();
                        //check IP address and port, when port is not empty it is valid by input guarantees
                        if ((IP_ADDRESS.matcher(ip).matches()) && (lteMacDestPort.length() != 0)) {
                            setLteMacUdpGuiState(getView(), true);

                            Integer port = Integer.parseInt(lteMacDestPort.getText().toString());
                            lteMacService.register(true);
                            lteMacService.startSendPacketsOverUdp(ip, port);
                        } else { //on errors..
                            lteMacUdpEnable.setChecked(false);
                            Toast.makeText(compoundButton.getContext(), getResources().getString(R.string.ipPortInvalid), Toast.LENGTH_SHORT).show();
                        }
                    } else {
                        //when un-checked stop UDP forwarding
                        lteMacService.stopUdp();
                        lteMacService.register(false);

                        setLteMacUdpGuiState(getView(), false);
                    }
                }
            });
        }

        @Override
        public View onCreateView(LayoutInflater inflater, ViewGroup container, Bundle savedInstanceState) {
            View rootView = inflater.inflate(R.layout.content_lte_mac, container, false);

            setupDestIpEdit(rootView);
            setupDestPortEdit(rootView);
            setLteMacUdpGuiState(rootView, lteMacUdpActive);
            setupUdpCheckbox(rootView);

            return rootView;
        }
    }

    /**
     * fragment showing LTE security messages
     */
    public static class LteSecFragment extends Fragment {
        private LteSecService.LteSecListener lteSecListener;
        private CompoundButton.OnCheckedChangeListener optionChangedListener;
        boolean firstSetVisible = true;

        private void updateListenerSubsciption(boolean regActive) {
            lteSecService.removeListener(lteSecListener);
            if (regActive) {
                List<LteSecService.RegistrationFlag> flags = new LinkedList<LteSecService.RegistrationFlag>();

                boolean incKey = sharedPreferences.getBoolean("lte_sec_inc_key", true);
                boolean incInput = sharedPreferences.getBoolean("lte_sec_inc_input", true);
                boolean incOutput = sharedPreferences.getBoolean("lte_sec_inc_output", true);

                if (sharedPreferences.getBoolean("lte_sec_keys", true)) {
                    flags.add(LteSecService.RegistrationFlag.GENERATED_ALGO_KEYS);
                    if (incInput) {
                        flags.add(LteSecService.RegistrationFlag.GENERATED_ALGO_KEYS_INPUT);
                    }
                }
                if (sharedPreferences.getBoolean("lte_sec_master_key", true)) {
                    flags.add(LteSecService.RegistrationFlag.GENERATED_KEYS);
                    if (incInput) {
                        flags.add(LteSecService.RegistrationFlag.GENERATED_KEYS_INPUT);
                    }
                }
                if (sharedPreferences.getBoolean("lte_sec_cipher", true)) {
                    flags.add(LteSecService.RegistrationFlag.CIPHER_CALLS);
                    if (incKey) {
                        flags.add(LteSecService.RegistrationFlag.CIPHER_CALLS_KEY);
                    }
                    if (incInput) {
                        flags.add(LteSecService.RegistrationFlag.CIPHER_CALLS_IN_MSG);
                    }
                    if (incOutput) {
                        flags.add(LteSecService.RegistrationFlag.CIPHER_CALLS_OUT_MSG);
                    }
                }
                if (sharedPreferences.getBoolean("lte_sec_decipher", true)) {
                    flags.add(LteSecService.RegistrationFlag.DECIPHER_CALLS);
                    if (incKey) {
                        flags.add(LteSecService.RegistrationFlag.DECIPHER_CALLS_KEY);
                    }
                    if (incInput) {
                        flags.add(LteSecService.RegistrationFlag.DECIPHER_CALLS_IN_MSG);
                    }
                    if (incOutput) {
                        flags.add(LteSecService.RegistrationFlag.DECIPHER_CALLS_OUT_MSG);
                    }
                }
                if (sharedPreferences.getBoolean("lte_sec_mac", true)) {
                    flags.add(LteSecService.RegistrationFlag.MACI_CALLS);
                    if (incKey) {
                        flags.add(LteSecService.RegistrationFlag.MACI_CALLS_KEY);
                    }
                    if (incInput) {
                        flags.add(LteSecService.RegistrationFlag.MACI_CALLS_IN_MSG);
                    }
                    if (incOutput) {
                        flags.add(LteSecService.RegistrationFlag.MACI_CALLS_MAC);
                    }
                }
                lteSecService.addListener(lteSecListener, flags.toArray(new LteSecService.RegistrationFlag[flags.size()]));
            }
        }

        @Override
        public void setUserVisibleHint(boolean isVisibleToUser) {
            super.setUserVisibleHint(isVisibleToUser);

            //do not trigger this code at creation of the fragment..
            if (!firstSetVisible) {
                if (sharedPreferences.getBoolean("lte_sec_deregister", true)) {
                    //de(register) on tab changes
                    updateListenerSubsciption(isVisibleToUser);
                }
            }
            firstSetVisible = false;
        }

        @Override
        public View onCreateView(LayoutInflater inflater, ViewGroup container, Bundle savedInstanceState) {
            View rootView = inflater.inflate(R.layout.content_lte_sec, container, false);

            final TextView lteSecLog = (TextView) rootView.findViewById(R.id.lteSecLog);

            lteSecListener = new LteSecService.LteSecListener() {
                private String byteArrToHexStringCipher(byte[] data) {
                    if (sharedPreferences.getBoolean("lte_sec_python", false)) {
                        return byteArrToHexString(data, false, 0) + ", " + byteArrToHexString(data, true, 0);
                    } else {
                        return byteArrToHexString(data, false, 0);
                    }
                }

                @Override
                public void newKey(LteSecService.NewKeyEvent e) {
                    String key = byteArrToHexStringCipher(e.getKey());
                    String s = String.format("new master key: %s\n", byteArrToHexStringCipher(e.getKey()));
                    if (e.getInKey() != null) {
                        s = String.format("%sinput key: %s\n", s, byteArrToHexStringCipher(e.getInKey()));
                        s = String.format("%sinput string: %s\n", s, byteArrToHexStringCipher(e.getInString()));
                    }
                    s += "\n";
                    lteSecLog.append(s);
                    if (sharedPreferences.getBoolean("lte_sec_write_to_syso", false)) {
                        System.out.println(s);
                    }
                }

                @Override
                public void newAlgorithmKey(LteSecService.NewAlgorithmKeyEvent e) {
                    String key = byteArrToHexStringCipher(e.getKey());
                    String s = String.format("new algorithm key for %s, used algorithm %s: %s\n", e.getKeyUse().name(), e.getKeyAlgorithm().name(), key);
                    if (e.getInKey() != null) {
                        s = String.format("%sinput key: %s\n", s, byteArrToHexStringCipher(e.getInKey()));
                    }
                    s += "\n";
                    lteSecLog.append(s);
                    if (sharedPreferences.getBoolean("lte_sec_write_to_syso", false)) {
                        System.out.println(s);
                    }
                }

                private void writeCryptoCall(String type, LteSecService.CryptoCallEvent e) {
                    String s = String.format("%s: algorithm: %s, bearer: %d, count: %d, message bytes: %d\n", type, e.getKeyAlgorithm().name(), e.getBearer(), e.getCount(), e.getMsgLength());
                    if (e.getUsedKey() != null) {
                        s = String.format("%sused key: %s\n", s, byteArrToHexStringCipher(e.getUsedKey()));
                    }
                    if (e.getInMsg() != null) {
                        s = String.format("%sinput message: %s\n", s, byteArrToHexStringCipher(e.getInMsg()));
                    }
                    if (e.getOutMsg() != null) {
                        s = String.format("%soutput data: %s\n", s, byteArrToHexStringCipher(e.getOutMsg()));
                    }
                    s += "\n";

                    lteSecLog.append(s);
                    if (sharedPreferences.getBoolean("lte_sec_write_to_syso", false)) {
                        System.out.println(s);
                    }
                }

                @Override
                public void cipherCall(LteSecService.CryptoCallEvent e) {
                    writeCryptoCall("cipher call", e);
                }

                @Override
                public void decipherCall(LteSecService.CryptoCallEvent e) {
                    writeCryptoCall("decipher call", e);
                }

                @Override
                public void maciCall(LteSecService.CryptoCallEvent e) {
                    writeCryptoCall((e.isDirectionDownlink() ? "MAC-i call downlink" : "MAC-i call uplink"), e);
                }
            };

            optionChangedListener = new CompoundButton.OnCheckedChangeListener() {
                @Override
                public void onCheckedChanged(CompoundButton compoundButton, boolean b) {
                    String buttonId = getResources().getResourceName(compoundButton.getId());
                    buttonId = buttonId.substring(buttonId.lastIndexOf("/") + 1);
                    storeBooleanInSharedPrefs(buttonId, b);
                    updateListenerSubsciption(true);
                }
            };

            initOptionButton((CheckBox) rootView.findViewById(R.id.lte_sec_master_key));
            initOptionButton((CheckBox) rootView.findViewById(R.id.lte_sec_keys));
            initOptionButton((CheckBox) rootView.findViewById(R.id.lte_sec_cipher));
            initOptionButton((CheckBox) rootView.findViewById(R.id.lte_sec_decipher));
            initOptionButton((CheckBox) rootView.findViewById(R.id.lte_sec_mac));
            initOptionButton((CheckBox) rootView.findViewById(R.id.lte_sec_inc_key));
            initOptionButton((CheckBox) rootView.findViewById(R.id.lte_sec_inc_input));
            initOptionButton((CheckBox) rootView.findViewById(R.id.lte_sec_inc_output));

            ((Button) rootView.findViewById(R.id.lte_sec_clear)).setOnClickListener(new OnClickListener() {
                @Override
                public void onClick(View view) {
                    lteSecLog.setText("");
                }
            });

            if (!sharedPreferences.getBoolean("lte_sec_deregister", true)) {
                updateListenerSubsciption(true);
            }

            return rootView;
        }

        private void initOptionButton(CheckBox optionButton) {
            String buttonId = getResources().getResourceName(optionButton.getId());
            buttonId = buttonId.substring(buttonId.lastIndexOf("/") + 1);
            optionButton.setChecked(sharedPreferences.getBoolean(buttonId, true));
            optionButton.setOnCheckedChangeListener(optionChangedListener);
        }

        @Override
        public void onDestroy() {
            super.onDestroy();

            lteSecService.removeListener(lteSecListener);
        }
    }

    /**
     * fragment showing memory access service GUI
     */
    public static class MemAccessFragment extends Fragment {
        private MemAccessService.MemAccessListener memAccessListener;
        private MemDataFile lastData;
        private int bulkReadRemainingSegments = 0;
        private static final int MEM_WRITE_FILE_CHOOSE_INTENT_CODE = 47;

        private boolean memAddressInputValid(EditText memAddressInput) {
            return ((memAddressInput.getText().length() > 0) && (memAddressInput.getText().length() <= 8));
        }

        private boolean memLengthInputValid(EditText memLengthInput) {
            return (memLengthInput.getText().length() > 0);
        }

        private boolean memDataInputValid(EditText memDataInput) {
            return ((memDataInput.getText().length() > 0) && ((memDataInput.getText().length() % 2 == 0)));
        }

        private void setReadButtonState(Button memReadButton, EditText memAddressInput, EditText memLengthInput) {
            memReadButton.setEnabled(memAddressInputValid(memAddressInput) && memLengthInputValid(memLengthInput));
        }

        private void setWriteButtonState(Button memReadButton, EditText memAddressInput, EditText memDataInput) {
            memReadButton.setEnabled(memAddressInputValid(memAddressInput) && memDataInputValid(memDataInput));
        }

        public void onActivityResult(int requestCode, int resultCode, Intent data) {
            if ((requestCode == MEM_WRITE_FILE_CHOOSE_INTENT_CODE) && (data != null)) {
                Uri uri = data.getData();

                EditText memAddressInput = (EditText) getView().findViewById(R.id.memAddressInput);
                if (memAddressInputValid(memAddressInput)) {
                    long address = Long.parseLong(memAddressInput.getText().toString(), 16);

                    MemDataFile content = MemDataFile.createMemDataFile(getContext(), uri, (int) address);
                    if (content != null) {
                        memAccessService.writeMemory(content.getStartAddress(), content.getData(), 0);

                        Toast.makeText(getActivity(), String.format("Wrote memory file starting from address 0x%1$08X!", address), Toast.LENGTH_LONG).show();
                    } else {
                        Toast.makeText(getActivity(), "Error reading memory file!", Toast.LENGTH_LONG).show();
                    }
                } else {
                    Toast.makeText(getActivity(), "Start address invalid!", Toast.LENGTH_LONG).show();
                }
            }
            super.onActivityResult(requestCode, resultCode, data);
        }

        @Override
        public View onCreateView(LayoutInflater inflater, ViewGroup container, Bundle savedInstanceState) {
            View rootView = inflater.inflate(R.layout.content_mem_access, container, false);

            final TextView memAccessTerminal = (TextView) rootView.findViewById(R.id.memAccessTerminal);

            final RadioButton readSelect = (RadioButton) rootView.findViewById(R.id.readSelectButton);
            final RadioButton writeSelect = (RadioButton) rootView.findViewById(R.id.writeSelectButton);

            final LinearLayout readLayout = (LinearLayout) rootView.findViewById(R.id.readLayout);
            final LinearLayout writeLayout = (LinearLayout) rootView.findViewById(R.id.writeLayout);
            final Button writeFromFileButton = (Button) rootView.findViewById(R.id.mem_access_load_button);
            final ImageButton saveToFileButton = (ImageButton) rootView.findViewById(R.id.mem_access_save_button);
            writeLayout.setVisibility(View.GONE);
            writeFromFileButton.setVisibility(View.GONE);
            saveToFileButton.setVisibility(View.GONE);

            readSelect.setOnCheckedChangeListener(new CompoundButton.OnCheckedChangeListener() {
                @Override
                public void onCheckedChanged(CompoundButton compoundButton, boolean b) {
                    readLayout.setVisibility(!b ? View.GONE : View.VISIBLE);
                    writeLayout.setVisibility(b ? View.GONE : View.VISIBLE);
                    writeFromFileButton.setVisibility(b ? View.GONE : View.VISIBLE);
                    if (b) {
                        writeSelect.setChecked(false);
                    }
                }
            });
            writeSelect.setOnCheckedChangeListener(new CompoundButton.OnCheckedChangeListener() {
                @Override
                public void onCheckedChanged(CompoundButton compoundButton, boolean b) {
                    if (b) {
                        readSelect.setChecked(false);
                    }
                }
            });

            final Button memReadButton = (Button) rootView.findViewById(R.id.memReadButton);
            final Button memWriteButton = (Button) rootView.findViewById(R.id.memWriteButton);

            final EditText memAddressInput = (EditText) rootView.findViewById(R.id.memAddressInput);
            final EditText memLengthInput = (EditText) rootView.findViewById(R.id.memLengthInput);
            final EditText memDataInput = (EditText) rootView.findViewById(R.id.memDataInput);

            if (sharedPreferences.getBoolean("mem_access_keep_last_values", false)) {
                memAddressInput.setText(sharedPreferences.getString("memAddressInput", ""));
                memLengthInput.setText(sharedPreferences.getString("memLengthInput", ""));
                memDataInput.setText(sharedPreferences.getString("memDataInput", ""));
            } else {
                memAddressInput.setText("");
                memLengthInput.setText("");
                memDataInput.setText("");
            }

            setReadButtonState(memReadButton, memAddressInput, memLengthInput);
            setWriteButtonState(memWriteButton, memAddressInput, memDataInput);

            TextWatcher readWatcher = new TextWatcher() {
                @Override
                public void onTextChanged(CharSequence s, int start, int before, int count) {
                }

                @Override
                public void beforeTextChanged(CharSequence s, int start, int count, int after) {
                }

                @Override
                public void afterTextChanged(Editable s) {
                    setReadButtonState(memReadButton, memAddressInput, memLengthInput);
                    storeStringInSharedPrefs("memAddressInput", memAddressInput.getText().toString());
                    storeStringInSharedPrefs("memLengthInput", memLengthInput.getText().toString());
                }
            };

            TextWatcher writeWatcher = new TextWatcher() {
                @Override
                public void onTextChanged(CharSequence s, int start, int before, int count) {
                }

                @Override
                public void beforeTextChanged(CharSequence s, int start, int count, int after) {
                }

                @Override
                public void afterTextChanged(Editable s) {
                    setWriteButtonState(memWriteButton, memAddressInput, memDataInput);
                    storeStringInSharedPrefs("memAddressInput", memAddressInput.getText().toString());
                    storeStringInSharedPrefs("memDataInput", memDataInput.getText().toString());
                }
            };

            memAddressInput.addTextChangedListener(readWatcher);
            memAddressInput.addTextChangedListener(writeWatcher);
            memLengthInput.addTextChangedListener(readWatcher);
            memDataInput.addTextChangedListener(writeWatcher);

            memReadButton.setOnClickListener(new OnClickListener() {
                @Override
                public void onClick(View view) {
                    final long address = Long.parseLong(memAddressInput.getText().toString(), 16);
                    final int length = Integer.parseInt(memLengthInput.getText().toString());
                    if (length <= 8180) {
                        memAccessService.readMemory((int) address, length);
                    } else {
                        AlertDialog.Builder alertDialog = new AlertDialog.Builder(getActivity());

                        alertDialog.setTitle("Length too big");
                        alertDialog.setMessage("The length you entered is too big for a normal read. Do you want to do a bulk read (only directly to file) instead?");

                        alertDialog.setPositiveButton("Yes", new DialogInterface.OnClickListener() {

                            public void onClick(DialogInterface dialog, int which) {
                                saveToFileButton.setVisibility(View.GONE);
                                memAccessTerminal.setText("");
                                lastData = new MemDataFile((int) address, length);

                                dialog.dismiss();

                                bulkReadRemainingSegments = length / 2048;
                                for (int seg = 0; seg < bulkReadRemainingSegments; seg++) {
                                    memAccessService.readMemory((int) address + seg * 2048, 2048);
                                }
                                int dumped = bulkReadRemainingSegments * 2048;
                                if (dumped < length) {
                                    bulkReadRemainingSegments++;
                                    memAccessService.readMemory((int) address + dumped, length-dumped);
                                }
                            }
                        });

                        alertDialog.setNegativeButton("No", new DialogInterface.OnClickListener() {

                            @Override
                            public void onClick(DialogInterface dialog, int which) {
                                dialog.cancel();
                            }
                        });

                        alertDialog.show();
                    }
                }
            });

            memWriteButton.setOnClickListener(new OnClickListener() {
                @Override
                public void onClick(View view) {
                    long address = Long.parseLong(memAddressInput.getText().toString(), 16);
                    String s = memDataInput.getText().toString();
                    int numBytes = s.length() / 2;
                    byte[] data = new byte[numBytes];

                    for (int i = 0; i < numBytes; i++) {
                        byte b = (byte) Integer.parseInt(s.substring(i * 2, i * 2 + 2), 16);
                        data[i] = b;
                    }

                    memAccessService.writeMemory((int) address, data, 0);
                }
            });

            writeFromFileButton.setOnClickListener(new OnClickListener() {
                @Override
                public void onClick(View view) {
                    Intent intent = new Intent(Intent.ACTION_GET_CONTENT);
                    intent.setType("file/*");
                    startActivityForResult(intent, MEM_WRITE_FILE_CHOOSE_INTENT_CODE);
                }
            });

            saveToFileButton.setOnClickListener(new OnClickListener() {
                @Override
                public void onClick(View view) {
                    AlertDialog.Builder alertDialog = new AlertDialog.Builder(getActivity());
                    alertDialog.setTitle("Save memory content to file");

                    TextInputLayout til = new TextInputLayout(alertDialog.getContext());
                    final EditText input = new EditText(getContext());
                    LinearLayout.LayoutParams lp = new LinearLayout.LayoutParams(
                            LinearLayout.LayoutParams.MATCH_PARENT,
                            LinearLayout.LayoutParams.MATCH_PARENT);
                    input.setLayoutParams(lp);
                    input.setHint("enter filename");
                    input.setText(sharedPreferences.getString("mem_access_save_last_filename", ""));
                    til.addView(input);

                    LinearLayout dialogLayout = new LinearLayout(alertDialog.getContext());
                    dialogLayout.setLayoutParams(lp);
                    dialogLayout.setOrientation(LinearLayout.VERTICAL);


                    RadioButton rawBinary = new RadioButton(alertDialog.getContext());
                    rawBinary.setText("raw binary");
                    final RadioButton iHex = new RadioButton(alertDialog.getContext());
                    iHex.setText("ihex");
                    RadioGroup fileFormatSelect = new RadioGroup(alertDialog.getContext());
                    fileFormatSelect.setOrientation(RadioGroup.HORIZONTAL);
                    fileFormatSelect.addView(rawBinary);
                    fileFormatSelect.addView(iHex);
                    if (!sharedPreferences.getBoolean("mem_access_save_last_format_ihex", false)) {
                        fileFormatSelect.check(rawBinary.getId());
                    } else {
                        fileFormatSelect.check(iHex.getId());
                    }
                    fileFormatSelect.setLayoutParams(lp);

                    dialogLayout.addView(fileFormatSelect);
                    dialogLayout.addView(til);

                    alertDialog.setView(dialogLayout);

                    alertDialog.setPositiveButton("Save",
                            new DialogInterface.OnClickListener() {
                                public void onClick(DialogInterface dialog, int which) {
                                    boolean formatIHex = iHex.isChecked();

                                    String fileName = input.getText().toString();
                                    if (!fileName.contains(".")) {
                                        if (!formatIHex) {
                                            fileName += ".bin";
                                        } else {
                                            fileName += ".ihex";
                                        }
                                    }

                                    String dir = "/SEEMOOQComLTE/";
                                    File path = new File(System.getenv("EXTERNAL_STORAGE") + dir);

                                    boolean success = false;
                                    if (!formatIHex) {
                                        success = lastData.writeToBinFile(path, fileName);
                                    } else {
                                        success = lastData.writeToIHexFile(path, fileName);
                                    }
                                    if (success) {
                                        Toast.makeText(getActivity(), "Saved memory content to " + lastData.getFileName() + "!", Toast.LENGTH_LONG).show();
                                    } else {
                                        Toast.makeText(getActivity(), "Error saving memory file!", Toast.LENGTH_LONG).show();
                                    }


                                    storeStringInSharedPrefs("mem_access_save_last_filename", fileName);
                                    SharedPreferences.Editor editor = sharedPreferences.edit();
                                    editor.putBoolean("mem_access_save_last_format_ihex", iHex.isChecked());
                                    editor.commit();

                                    dialog.dismiss();
                                }
                            });
                    alertDialog.setNegativeButton("Cancel",
                            new DialogInterface.OnClickListener() {
                                public void onClick(DialogInterface dialog, int which) {
                                    dialog.cancel();
                                }
                            });

                    alertDialog.show();
                }
            });

            memAccessListener = new MemAccessService.MemAccessListener() {
                @Override
                public void memoryData(MemAccessService.MemoryReadEvent e) {
                    if (bulkReadRemainingSegments == 0) {
                        lastData = new MemDataFile(e.getStartAddress(), e.getData());
                        saveToFileButton.setVisibility(View.VISIBLE);

                        int separation;
                        try {
                            separation = Integer.parseInt(sharedPreferences.getString("mem_access_data_seperate", "4"));
                        } catch (Exception exc) {
                            separation = 4;
                        }
                        String dataString = "";
                        if (sharedPreferences.getBoolean("mem_access_include_address", true)) {
                            byte[] data = e.getData();

                            int bytesPerLine;
                            try {
                                bytesPerLine = Integer.parseInt(sharedPreferences.getString("mem_access_bytes_per_line", "4"));
                            } catch (Exception exc) {
                                bytesPerLine = 8;
                            }

                            StringBuilder sb = new StringBuilder();
                            for (int pos = 0; pos < data.length; pos += bytesPerLine) {
                                sb.append(String.format("0x%1$08X: ", e.getStartAddress() + pos));
                                sb.append(byteArrToHexString(data, false, separation, pos, bytesPerLine));
                                sb.append("\n");
                            }
                            dataString = sb.toString();
                        } else {
                            dataString = byteArrToHexString(e.getData(), false, separation);
                        }
                        String s = String.format("Memory read, start address: 0x%1$08X, length: %2$d bytes\n%3$s", e.getStartAddress(), e.getLength(), dataString);
                        memAccessTerminal.setText(s);
                        if (sharedPreferences.getBoolean("mem_access_write_to_syso", false)) {
                            System.out.println(s);
                        }
                    } else {
                        lastData.putData(e.getStartAddress(), e.getData());
                        bulkReadRemainingSegments--;
                        if (bulkReadRemainingSegments == 0) {
                            String s = String.format("Received all segments of bulk read!\nstart address: 0x%1$08X, length: %2$d bytes", lastData.getStartAddress(), lastData.getData().length);
                            memAccessTerminal.setText(s);
                            saveToFileButton.callOnClick();
                        } else {
                            String s = String.format("Received a segment for bulk read\nstart address: 0x%1$08X, length: %2$d bytes, remaining: %3$d\n", e.getStartAddress(), e.getLength(), bulkReadRemainingSegments);
                            memAccessTerminal.setText(memAccessTerminal.getText() + s);
                        }
                    }
                }

                @Override
                public void writeDone(MemAccessService.MemoryDataEvent e) {
                    String s = String.format("Memory write executed, start address: 0x%1$08X, length: %2$d bytes\n", e.getStartAddress(), e.getLength());
                    memAccessTerminal.setText(s);
                    saveToFileButton.setVisibility(View.GONE);
                    if (sharedPreferences.getBoolean("mem_access_write_to_syso", false)) {
                        System.out.println(s);
                    }
                }
            };
            memAccessService.addListener(memAccessListener);

            return rootView;
        }

        @Override
        public void onDestroy() {
            super.onDestroy();

            memAccessService.removeListener(memAccessListener);
        }
    }

    /**
     * fragment for showing channel estimation values
     */
    public static class ChannelEstimationFragment extends Fragment {
        private static final int MAX_RX_ANT = 2;
        private static final int MAX_TX_ANT = 4;

        private static final float SCALE_MAX_FACTOR = 1.2f;
        private static final float DOWNSCALE_THRESHOLD = 0.7f;
        private static final int LAST_MAX_MAX_SIZE = 40;

        private ChannelEstimationService.ChannelEstimationListener channelEstimationListener;

        private LineChart channelChart;
        private Spinner rxAntSel;
        private Spinner txAntSel;
        private TextView bandwidthView;
        private TextView yAxisLeftLabel;
        private TextView yAxisRightLabel;

        private int rxAntMode;
        private int txAntMode;
        private boolean complexModeSelected;
        private boolean drawAmplitude;
        private boolean drawPhase;

        float graphCurScaleMax;
        List<Float> lastMax;
        float curMax;

        private LineDataSet lineDataSetFromChannelMatrix(int type, String label, ComplexInteger[] channelMatrix, float colorAngle, boolean axisRight, boolean dashed, int numRBs) {
            List<Entry> entries = new ArrayList<Entry>();

            int pos = 0;
            boolean subcarrierIndex = sharedPreferences.getBoolean("csi_visualization_subcarrier_index", false);
            for (ComplexInteger sample : channelMatrix) {
                float sampleVal = 0.0f;
                switch (type) {
                    case 0:
                        sampleVal = (float) sample.abs();
                        curMax = Math.max(curMax, sampleVal);
                        break;
                    case 1:
                        sampleVal = (float) sample.angleInDegree();
                        break;
                    case 2:
                        sampleVal = sample.getReal();
                        curMax = Math.max(curMax, Math.abs(sampleVal));
                        break;
                    case 3:
                        sampleVal = sample.getImaginary();
                        curMax = Math.max(curMax, Math.abs(sampleVal));
                        break;
                }
                int xValue = 0;
                xValue = pos * numRBs / channelMatrix.length;
                if (subcarrierIndex) {
                    xValue *= 12;
                }
                entries.add(new Entry(xValue, sampleVal));
                pos++;
            }

            LineDataSet dataSet = new LineDataSet(entries, label);
            dataSet.setAxisDependency(axisRight ? YAxis.AxisDependency.RIGHT : YAxis.AxisDependency.LEFT);
            float[] hsv = {colorAngle, 1.0f, 1.0f};
            int color = Color.HSVToColor(hsv);
            dataSet.setColor(color);
            dataSet.setCircleColor(color);

            if (dashed) {
                dataSet.enableDashedLine(10.0f, 10.0f, 0);
            }

            return dataSet;
        }

        private float getColorAngle(int index, int numColors) {
            return 360 * (index / (float) numColors);
        }

        private void resetGraphAxisScaling() {
            graphCurScaleMax = 0.0f;
            lastMax = new LinkedList<Float>();
        }

        private void scaleGraphAxis() {
            if (sharedPreferences.getBoolean("csi_autoscale", true)) {
                if (curMax > graphCurScaleMax) {
                    graphCurScaleMax = curMax * SCALE_MAX_FACTOR;
                }

                if (sharedPreferences.getBoolean("csi_downscale_graph", true)) {
                    lastMax.add(curMax);
                    if (lastMax.size() > LAST_MAX_MAX_SIZE) {
                        lastMax.remove(0);
                    }

                    float max = 0;
                    for (float f : lastMax) {
                        max = Math.max(max, f);
                    }
                    if (max < graphCurScaleMax * DOWNSCALE_THRESHOLD) {
                        graphCurScaleMax = curMax * SCALE_MAX_FACTOR;
                        lastMax.clear();
                    }
                }
            } else {
                graphCurScaleMax = Integer.parseInt(sharedPreferences.getString("csi_graph_max", "1000"));
            }
        }

        private void drawChart(ChannelEstimationService.ChannelMatrices channelMatrices) {
            List<ILineDataSet> dataSets = new ArrayList<ILineDataSet>();

            boolean twoColors = sharedPreferences.getBoolean("csi_visualization_use_different_colors", true);
            if (!complexModeSelected) {
                if (!(drawAmplitude && drawPhase)) {
                    twoColors = false;
                }
            }

            int color = 0;
            int numRxAnt = (rxAntMode == 0) ? channelMatrices.getNumRxAnt() : 1;
            int numTxAnt = (txAntMode == 0) ? channelMatrices.getNumTxAnt() : 1;
            int numColors = numRxAnt * numTxAnt;
            if (twoColors) {
                numColors *= 2;
            }
            curMax = 0;
            for (int rxAnt = 1; rxAnt <= channelMatrices.getNumRxAnt(); rxAnt++) {
                if ((rxAntMode != 0) && (rxAntMode != rxAnt)) {
                    continue;
                }
                for (int txAnt = 1; txAnt <= channelMatrices.getNumRxAnt(); txAnt++) {
                    if ((txAntMode != 0) && (txAntMode != txAnt)) {
                        continue;
                    }

                    ComplexInteger[] curMatrix = channelMatrices.getChannelMatrix(rxAnt - 1, txAnt - 1);
                    int numRBs = channelMatrices.getBandwidthResourceBlocks();
                    if (complexModeSelected) {
                        String realLabel = String.format("Re Rx%d/Tx%d", rxAnt, txAnt);
                        dataSets.add(lineDataSetFromChannelMatrix(2, realLabel, curMatrix, getColorAngle(color, numColors), false, false, numRBs));
                        if (twoColors) {
                            color++;
                        }
                        String imgLabel = String.format("Im Rx%d/Tx%d", rxAnt, txAnt);
                        dataSets.add(lineDataSetFromChannelMatrix(3, imgLabel, curMatrix, getColorAngle(color, numColors), false, !twoColors, numRBs));
                    } else {
                        if (drawAmplitude) {
                            String label = String.format("Abs Rx%d/Tx%d", rxAnt, txAnt);
                            dataSets.add(lineDataSetFromChannelMatrix(0, label, curMatrix, getColorAngle(color, numColors), false, false, numRBs));
                            if (twoColors) {
                                color++;
                            }
                        }
                        if (drawPhase) {
                            String label = String.format("Phase Rx%d/Tx%d", rxAnt, txAnt);
                            dataSets.add(lineDataSetFromChannelMatrix(1, label, curMatrix, getColorAngle(color, numColors), drawAmplitude, !twoColors, numRBs));
                        }
                    }
                    color++;
                }
            }
            if (complexModeSelected || drawAmplitude) {
                scaleGraphAxis();
            }

            LineData lineData = new LineData(dataSets);
            channelChart.getAxisRight().setEnabled(false);
            yAxisRightLabel.setVisibility(View.GONE);
            yAxisLeftLabel.setText("Amplitude       ");
            channelChart.getAxisLeft().setAxisMaximum(graphCurScaleMax);

            if (!complexModeSelected) {
                channelChart.getAxisLeft().setAxisMinimum(0);
                if (drawPhase) {
                    if (drawAmplitude) {
                        channelChart.getAxisRight().setEnabled(true);
                        yAxisRightLabel.setText("Phase [degree]       ");
                        yAxisRightLabel.setVisibility(View.VISIBLE);
                    } else {
                        channelChart.getAxisLeft().setAxisMaximum(360);
                        yAxisLeftLabel.setText("Phase [degree]       ");
                    }
                }
            } else {
                channelChart.getAxisLeft().setAxisMinimum(-graphCurScaleMax);
            }

            channelChart.setData(lineData);

            XAxis xAxis = channelChart.getXAxis();
            xAxis.setPosition(XAxis.XAxisPosition.BOTTOM);

            if (sharedPreferences.getBoolean("csi_visualization_subcarrier_index", false)) {
                channelChart.getDescription().setText("Subcarrier");
            } else {
                channelChart.getDescription().setText("Resource Block");
            }
            channelChart.getDescription().setYOffset(-25);

            String descr = String.format("   Bandwidth: %s MHz", channelMatrices.getBandwidthString(false));
            bandwidthView.setText(descr);

            channelChart.invalidate();
        }

        private void reDrawChartWithLastData() {
            if (channelEstimationService.getLastChannelMatrices() != null) {
                drawChart(channelEstimationService.getLastChannelMatrices());
            }
        }

        private void configRxTxSelects(ChannelEstimationService.ChannelMatrices channelMatrices) {
            int rxAntCount = MAX_RX_ANT;
            int txAntCount = MAX_TX_ANT;
            if (channelMatrices != null) {
                rxAntCount = channelMatrices.getNumRxAnt();
                txAntCount = channelMatrices.getNumTxAnt();
            }

            if (rxAntCount + 1 != rxAntSel.getCount()) {
                configAntSpinner(rxAntSel, rxAntCount, "Rx");
                int selected = sharedPreferences.getInt("rx_ant_mode", 0);
                rxAntSel.setSelection((selected > rxAntSel.getCount() - 1) ? 0 : selected);
            }
            if (txAntCount + 1 != txAntSel.getCount()) {
                configAntSpinner(txAntSel, txAntCount, "Tx");
                int selected = sharedPreferences.getInt("tx_ant_mode", 0);
                txAntSel.setSelection((selected > txAntSel.getCount() - 1) ? 0 : selected);
            }
        }

        private void configAntSpinner(Spinner spinner, int maxVal, String type) {
            String options[] = new String[maxVal + 1];
            options[0] = String.format("%s all", type);
            for (int i = 1; i <= maxVal; i++) {
                options[i] = String.format("%s %d", type, i);
            }
            ArrayAdapter<String> adapter = new ArrayAdapter<String>(getContext(), android.R.layout.simple_spinner_item, options);
            adapter.setDropDownViewResource(android.R.layout.simple_spinner_dropdown_item);
            spinner.setAdapter(adapter);
        }

        @Override
        public View onCreateView(LayoutInflater inflater, ViewGroup container, Bundle savedInstanceState) {
            View rootView = inflater.inflate(R.layout.content_channel_estimation, container, false);

            rxAntSel = (Spinner) rootView.findViewById(R.id.rx_ant_select);
            rxAntSel.setOnItemSelectedListener(new AdapterView.OnItemSelectedListener() {
                @Override
                public void onItemSelected(AdapterView<?> adapterView, View view, int i, long l) {
                    rxAntMode = i;
                    storeIntInSharedPrefs("rx_ant_mode", rxAntMode);
                    reDrawChartWithLastData();
                }

                @Override
                public void onNothingSelected(AdapterView<?> adapterView) {

                }
            });

            txAntSel = (Spinner) rootView.findViewById(R.id.tx_ant_select);
            txAntSel.setOnItemSelectedListener(new AdapterView.OnItemSelectedListener() {
                @Override
                public void onItemSelected(AdapterView<?> adapterView, View view, int i, long l) {
                    txAntMode = i;
                    storeIntInSharedPrefs("tx_ant_mode", txAntMode);
                    reDrawChartWithLastData();
                }

                @Override
                public void onNothingSelected(AdapterView<?> adapterView) {

                }
            });
            configRxTxSelects(channelEstimationService.getLastChannelMatrices());

            final LinearLayout ampPhaseDetailOptions = (LinearLayout) rootView.findViewById(R.id.amp_phase_detail_options);
            final RadioButton complexMode = (RadioButton) rootView.findViewById(R.id.complex_val);
            RadioGroup visualizationMode = (RadioGroup) rootView.findViewById(R.id.visualization_mode);
            complexModeSelected = sharedPreferences.getBoolean("csi_visualization_mode_complex", false);
            if (complexModeSelected) {
                visualizationMode.check(complexMode.getId());
                ampPhaseDetailOptions.setVisibility(View.GONE);
            }
            visualizationMode.setOnCheckedChangeListener(new RadioGroup.OnCheckedChangeListener() {
                @Override
                public void onCheckedChanged(RadioGroup radioGroup, int i) {
                    complexModeSelected = (i == complexMode.getId());
                    storeBooleanInSharedPrefs("csi_visualization_mode_complex", complexModeSelected);
                    ampPhaseDetailOptions.setVisibility(complexModeSelected ? View.GONE : View.VISIBLE);
                    resetGraphAxisScaling();
                    reDrawChartWithLastData();
                }
            });

            drawAmplitude = sharedPreferences.getBoolean("csi_visualization_draw_amplitude", true);
            drawPhase = sharedPreferences.getBoolean("csi_visualization_draw_phase", false);
            final CheckBox amplitudeSelect = (CheckBox) rootView.findViewById(R.id.amplitude_select);
            amplitudeSelect.setChecked(drawAmplitude);
            final CheckBox phaseSelect = (CheckBox) rootView.findViewById(R.id.phase_select);
            phaseSelect.setChecked(drawPhase);
            amplitudeSelect.setOnCheckedChangeListener(new CompoundButton.OnCheckedChangeListener() {
                @Override
                public void onCheckedChanged(CompoundButton compoundButton, boolean b) {
                    drawAmplitude = b;
                    storeBooleanInSharedPrefs("csi_visualization_draw_amplitude", drawAmplitude);
                    if (!drawAmplitude && !drawPhase) {
                        phaseSelect.setChecked(true);
                    } else {
                        reDrawChartWithLastData();
                    }
                }
            });
            phaseSelect.setOnCheckedChangeListener(new CompoundButton.OnCheckedChangeListener() {
                @Override
                public void onCheckedChanged(CompoundButton compoundButton, boolean b) {
                    drawPhase = b;
                    storeBooleanInSharedPrefs("csi_visualization_draw_phase", drawPhase);
                    if (!drawAmplitude && !drawPhase) {
                        amplitudeSelect.setChecked(true);
                    } else {
                        reDrawChartWithLastData();
                    }
                }
            });

            ViewPager viewPager = (ViewPager) container.findViewById(R.id.viewPager);
            LinearLayout chEstLayout = (LinearLayout) rootView.findViewById(R.id.ch_est_layout);
            ViewGroup.LayoutParams lp = chEstLayout.getLayoutParams();
            lp.height = viewPager.getMeasuredHeight() - 250;
            chEstLayout.setLayoutParams(lp);

            channelChart = (LineChart) rootView.findViewById(R.id.channel_chart);
            channelChart.getAxisRight().setAxisMinimum(0);
            channelChart.getAxisRight().setAxisMaximum(360);
            channelChart.getAxisRight().setDrawGridLines(false);

            bandwidthView = (TextView) rootView.findViewById(R.id.bandwidthView);
            yAxisLeftLabel = (TextView) rootView.findViewById(R.id.yAxisLeftLabel);
            yAxisRightLabel = (TextView) rootView.findViewById(R.id.yAxisRightLabel);

            Legend l = channelChart.getLegend();
            l.setWordWrapEnabled(true);
            l.setMaxSizePercent(0.7f);

            channelEstimationListener = new ChannelEstimationService.ChannelEstimationListener() {
                @Override
                public void matricesReceived(ChannelEstimationService.ChannelMatrixEvent e) {
                    configRxTxSelects(e.getChannelMatrices());
                    drawChart(e.getChannelMatrices());
                }
            };
            channelEstimationService.addListener(channelEstimationListener);

            resetGraphAxisScaling();
            reDrawChartWithLastData();

            return rootView;
        }

        @Override
        public void onDestroy() {
            super.onDestroy();

            channelEstimationService.removeListener(channelEstimationListener);
        }
    }
}
