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
import com.github.mikephil.charting.components.Description;
import com.github.mikephil.charting.components.Legend;
import com.github.mikephil.charting.components.XAxis;
import com.github.mikephil.charting.components.YAxis;
import com.github.mikephil.charting.data.Entry;
import com.github.mikephil.charting.data.LineData;
import com.github.mikephil.charting.data.LineDataSet;
import com.github.mikephil.charting.interfaces.datasets.ILineDataSet;

import java.io.File;
import java.util.ArrayList;
import java.util.LinkedList;
import java.util.List;
import java.util.Set;
import java.util.regex.Pattern;

import de.tu_darmstadt.seemoo.seemooqcomlte.seemooqmi.AtCommandService;
import de.tu_darmstadt.seemoo.seemooqcomlte.seemooqmi.ChannelEstimationService;
import de.tu_darmstadt.seemoo.seemooqcomlte.seemooqmi.ComplexFixedPoint;
import de.tu_darmstadt.seemoo.seemooqcomlte.seemooqmi.FunctionCounterService;
import de.tu_darmstadt.seemoo.seemooqcomlte.seemooqmi.LteMacService;
import de.tu_darmstadt.seemoo.seemooqcomlte.seemooqmi.LteSecService;
import de.tu_darmstadt.seemoo.seemooqcomlte.seemooqmi.MemAccessService;
import de.tu_darmstadt.seemoo.seemooqcomlte.seemooqmi.SeemooQmi;
import de.tu_darmstadt.seemoo.seemooqcomlte.seemooqmi.SnprintfService;

//TODO do not always register for channel estimation and lte sec
//TODO refactor MainActivity class: put code in multiple files, maybe put GUI fragments in seperate files? one for each?


//TODO finish lte_sec
//TODO lte_sec output options
//TODO at_commands


//TODO nice GUI
// -move settings icon (too far right, too close to text
// -when changing to a not scrollable tab and title bar is removed we cannot get it again
// -keyboard shows up when de-focusing edit texts in lte mac tab
// -app icon
// -settings: group header color

//TODO snprintf show write destination, choose destinations to show from list of received ones (filter)
//->store snprintf messages in service WITH destination ID

//TODO TODOs

public class MainActivity extends AppCompatActivity {
    private static SeemooQmi seemooQmi = null;
    private static FunctionCounterService functionCounterService = null;
    private static SnprintfService snprintfService = null;
    private static LteMacService lteMacService = null;
    private static LteSecService lteSecService = null;
    private static AtCommandService atCommandService = null;
    private static MemAccessService memAccessService = null;
    private static ChannelEstimationService channelEstimationService = null;

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
            functionCounterService.sendFuncCountersRequest();
            if (funcCountersPoll) {
                funcCountersPollHandler.postDelayed(this, funcCountersPollRate);
            }
        }
    };

    public static String byteArrToHexString(byte[] byteArr, boolean pythonString, int seperation) {
        return  byteArrToHexString(byteArr, pythonString, seperation, 0, byteArr.length);
    }


    //TODO document (also other stuff!)
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
            snprintfService.register(openTab == 2);
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
        if (openTab == 1) {
            if (!funcCountersPollOld && funcCountersPoll) {
                funcCountersPollHandler.postDelayed(funcCountersPollRunnable, funcCountersPollRate);

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

        if (atCommandService == null) {
            atCommandService = new AtCommandService(seemooQmi, getApplicationContext());
        }

        if (memAccessService == null) {
            memAccessService = new MemAccessService(seemooQmi, getApplicationContext());
        }

        if (channelEstimationService == null) {
            channelEstimationService = new ChannelEstimationService(seemooQmi, getApplicationContext());
            channelEstimationService.setInterval(Integer.parseInt(sharedPreferences.getString("csi_interval", "1")));
        }
    }

    /**
     * stores a value in the default shared preferences
     *
     * @param key name of the value
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
     * @param key name of the value
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
     * @param key name of the value
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
         * @param fm the FragmentManager
         * @param appContext application context
         */
        public SeemooPagerAdapter(FragmentManager fm, Context appContext) {
            super(fm);

            this.appContext = appContext;
        }

        @Override //TODO re-order?
        public Fragment getItem(int i) {
            switch (i) {
                case 0:
                    statusLogFragment = new StatusLogFragment();
                    return statusLogFragment;
                case 1:
                    functionCountersFragment = new FunctionCountersFragment();
                    return functionCountersFragment;
                case 2:
                    return new SnprintfFragment();
                case 3:
                    return new LteMacFragment();
                case 4:
                    return new LteSecFragment();
                case 5:
                    return new AtCommandsFragment();
                case 6:
                    return new MemAccessFragment();
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

        @Override //TODO re-order?
        public CharSequence getPageTitle(int position) {
            switch (position) {
                case 0:
                    return appContext.getResources().getString(R.string.tab_status_log);
                case 1:
                    return appContext.getResources().getString(R.string.tab_func_counters);
                case 2:
                    return appContext.getResources().getString(R.string.tab_snprintf);
                case 3:
                    return appContext.getResources().getString(R.string.tab_lte_mac);
                case 4:
                    return appContext.getResources().getString(R.string.tab_lte_sec);
                case 5:
                    return appContext.getResources().getString(R.string.tab_at_commands);
                case 6:
                    return appContext.getResources().getString(R.string.tab_mem_access);
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

        @Override
        public View onCreateView(LayoutInflater inflater, ViewGroup container, Bundle savedInstanceState) {
            View rootView = inflater.inflate(R.layout.content_status, container, false);

            //put old messages in the log (received before creation of this fragment
            //or when activity was send to background)
            final TextView statusLog = (TextView) rootView.findViewById(R.id.statusLog);
            for (String m : seemooQmi.getOldStatusMessages()) {
                statusLog.append(m);
                statusLog.append("\n");
            }
            statusListener = new SeemooQmi.StatusListener() {
                @Override
                public void statusUpdate(SeemooQmi.StatusUpdateEvent e) {
                    statusLog.append(e.getStatus());
                    statusLog.append("\n");
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
                funcCountersPollHandler.postDelayed(funcCountersPollRunnable, funcCountersPollRate);

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
         * @param view root view of the fragment
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
                public void newAlgorithmKey(LteSecService.NewAlgorithmKeyEvent e) {
                    String key = byteArrToHexStringCipher(e.getKey());
                    String s = String.format("new algorithm key for %s, used algorithm %s: %s\n\n", e.getKeyUse().name(), e.getKeyAlgorithm().name(), key);
                    lteSecLog.append(s);
                    if (sharedPreferences.getBoolean("lte_sec_write_to_syso", false)) {
                        System.out.println(s);
                    }
                }

                private void writeCryptoCall(String type, LteSecService.CryptoCallEvent e) {
                    String s = String.format("%s: algorithm: %s, bearer: %d, count: %d, message bytes: %d\n", type, e.getKeyAlgorithm().name(), e.getBearer(), e.getCount(), e.getMsgLength());
                    //TODO change to use GUI settings
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
            //TODO depending on GUI settings
            //TODO options: generated keys, cipher, decipher, mac and include data: key, in, out
            LteSecService.RegistrationFlag[] flags = {
                    LteSecService.RegistrationFlag.GENERATED_ALGO_KEYS,
                    LteSecService.RegistrationFlag.CIPHER_CALLS,
                    LteSecService.RegistrationFlag.CIPHER_CALLS_KEY,
                    LteSecService.RegistrationFlag.CIPHER_CALLS_IN_MSG,
                    LteSecService.RegistrationFlag.CIPHER_CALLS_OUT_MSG,
                    LteSecService.RegistrationFlag.DECIPHER_CALLS,
                    LteSecService.RegistrationFlag.DECIPHER_CALLS_KEY,
                    LteSecService.RegistrationFlag.DECIPHER_CALLS_IN_MSG,
                    LteSecService.RegistrationFlag.DECIPHER_CALLS_OUT_MSG,
                    LteSecService.RegistrationFlag.MACI_CALLS,
                    LteSecService.RegistrationFlag.MACI_CALLS_KEY,
                    LteSecService.RegistrationFlag.MACI_CALLS_IN_MSG,
                    LteSecService.RegistrationFlag.MACI_CALLS_MAC
            };
            lteSecService.addListener(lteSecListener, flags);

            return rootView;
        }

        @Override
        public void onDestroy() {
            super.onDestroy();

            lteSecService.removeListener(lteSecListener);
        }
    }

    /**
     * fragment showing AT commands GUI
     */
    public static class AtCommandsFragment extends Fragment {
        private AtCommandService.AtCommandListener atCommandListener;

        @Override
        public View onCreateView(LayoutInflater inflater, ViewGroup container, Bundle savedInstanceState) {
            View rootView = inflater.inflate(R.layout.content_at_commands, container, false);

            final TextView atTerminal = (TextView) rootView.findViewById(R.id.atTerminal);
            for (String m : atCommandService.getCachedMessages()) {
                atTerminal.append(m);
            }

            final Button atSendButton = (Button) rootView.findViewById(R.id.atSendButton);
            atSendButton.setEnabled(false);
            final EditText atCommandInput = (EditText) rootView.findViewById(R.id.atCommandInput);
            atCommandInput.addTextChangedListener(new TextWatcher() {
                @Override
                public void beforeTextChanged(CharSequence charSequence, int i, int i1, int i2) {

                }

                @Override
                public void onTextChanged(CharSequence charSequence, int i, int i1, int i2) {

                }

                @Override
                public void afterTextChanged(Editable editable) {
                    atSendButton.setEnabled(editable.length() != 0);
                }
            });
            atSendButton.setOnClickListener(new OnClickListener() {
                @Override
                public void onClick(View view) {
                    String command = atCommandInput.getText().toString();
                    if (!command.isEmpty()) {
                        atCommandService.invokeCommand(command);
                        atCommandInput.setText("");
                    }
                }
            });

            atCommandListener = new AtCommandService.AtCommandListener() {
                @Override
                public void statusUpdate(AtCommandService.AtCommandEvent e) {
                    atTerminal.append(e.getMessage());
                }
            };
            atCommandService.addListener(atCommandListener);

            return rootView;
        }

        @Override
        public void onDestroy() {
            super.onDestroy();

            atCommandService.removeListener(atCommandListener);
        }
    }

    /**
     * fragment showing memory access service GUI
     */
    public static class MemAccessFragment extends Fragment {
        private MemAccessService.MemAccessListener memAccessListener;
        private MemDataFile lastData;
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

                    MemDataFile content = MemDataFile.createMemDataFile(getContext(), uri, (int)address);
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
                    long address = Long.parseLong(memAddressInput.getText().toString(), 16);
                    int length = Integer.parseInt(memLengthInput.getText().toString());
                    memAccessService.readMemory((int)address, length);
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
                        byte b = (byte)Integer.parseInt(s.substring(i*2, i*2+2), 16);
                        data[i] = b;
                    }

                    memAccessService.writeMemory((int)address, data, 0);
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
                                public void onClick(DialogInterface dialog,int which) {
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
    //TODO get channel freq and everything else to scale axis!
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

        private int rxAntMode;
        private int txAntMode;
        private boolean complexModeSelected;
        private boolean drawAmplitude;
        private boolean drawPhase;

        float graphCurScaleMax;
        List<Float> lastMax;
        float curMax;

        private LineDataSet lineDataSetFromChannelMatrix(int type, String label, ComplexFixedPoint[] channelMatrix, float colorAngle, boolean axisRight, boolean dashed) {
            List<Entry> entries = new ArrayList<Entry>();

            int pos = 0;
            for (ComplexFixedPoint sample : channelMatrix) {
                //TODO scale values
                float sampleVal = 0.0f;
                switch (type) {
                    case 0:
                        sampleVal = (float)sample.abs();
                        curMax = Math.max(curMax, sampleVal);
                        break;
                    case 1:
                        sampleVal = (float)sample.angleInDegree();
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
                entries.add(new Entry(pos++, sampleVal)); //TODO x-values
            }

            //TODO label axis
            LineDataSet dataSet = new LineDataSet(entries, label);
            dataSet.setAxisDependency(axisRight ? YAxis.AxisDependency.RIGHT : YAxis.AxisDependency.LEFT);
            float[] hsv = {colorAngle,1.0f,1.0f};
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

                    ComplexFixedPoint[] curMatrix = channelMatrices.getChannelMatrix(rxAnt - 1, txAnt - 1);
                    if (complexModeSelected) {
                        String realLabel = String.format("Re Rx%d/Tx%d", rxAnt, txAnt);
                        dataSets.add(lineDataSetFromChannelMatrix(2, realLabel, curMatrix, getColorAngle(color, numColors), false, false));
                        if (twoColors) {
                            color++;
                        }
                        String imgLabel = String.format("Im Rx%d/Tx%d", rxAnt, txAnt);
                        dataSets.add(lineDataSetFromChannelMatrix(3, imgLabel, curMatrix, getColorAngle(color, numColors), false, !twoColors));
                    } else {
                        if (drawAmplitude) {
                            String label = String.format("Abs Rx%d/Tx%d", rxAnt, txAnt);
                            dataSets.add(lineDataSetFromChannelMatrix(0, label, curMatrix, getColorAngle(color, numColors), false, false));
                            if (twoColors) {
                                color++;
                            }
                        }
                        if (drawPhase) {
                            String label = String.format("Phase Rx%d/Tx%d", rxAnt, txAnt);
                            dataSets.add(lineDataSetFromChannelMatrix(1, label, curMatrix, getColorAngle(color, numColors), drawAmplitude, !twoColors));
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
            channelChart.getAxisLeft().setAxisMaximum(graphCurScaleMax);

            if (!complexModeSelected) {
                channelChart.getAxisLeft().setAxisMinimum(0);
                if (drawPhase) {
                    if (drawAmplitude) {
                        channelChart.getAxisRight().setEnabled(true);
                    } else {
                        channelChart.getAxisLeft().setAxisMaximum(360);
                    }
                }
            } else {
                channelChart.getAxisLeft().setAxisMinimum(-graphCurScaleMax);
            }

            Legend l = channelChart.getLegend();
            if (complexModeSelected) {
                l.setMaxSizePercent(1.0f);
            } else {
                l.setMaxSizePercent(0.7f);
            }

            channelChart.setData(lineData);

            XAxis xAxis = channelChart.getXAxis();
            xAxis.setPosition(XAxis.XAxisPosition.BOTTOM);
            //TODO axis titles..

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
                rxAntSel.setSelection((selected > rxAntSel.getCount()-1) ? 0: selected);
            }
            if (txAntCount + 1 != txAntSel.getCount()) {
                configAntSpinner(txAntSel, txAntCount, "Tx");
                int selected = sharedPreferences.getInt("tx_ant_mode", 0);
                txAntSel.setSelection((selected > txAntSel.getCount()-1) ? 0: selected);
            }
        }

        private void configAntSpinner(Spinner spinner, int maxVal, String type) {
            String options[] = new String[maxVal+1];
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
                    if (!drawAmplitude && ! drawPhase) {
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
                    if (!drawAmplitude && ! drawPhase) {
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
            Description descr = new Description();
            descr.setText("");
            channelChart.setDescription(descr);
            channelChart.getAxisRight().setAxisMinimum(0);
            channelChart.getAxisRight().setAxisMaximum(360);
            channelChart.getAxisRight().setDrawGridLines(false);

            Legend l = channelChart.getLegend();
            l.setWordWrapEnabled(true);

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
