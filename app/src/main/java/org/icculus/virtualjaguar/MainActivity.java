package org.icculus.virtualjaguar;

import android.Manifest;
import android.app.Activity;
import android.content.Intent;
import android.content.SharedPreferences;
import android.content.pm.PackageManager;
import android.os.Bundle;
import android.os.Environment;
import android.support.annotation.NonNull;
import android.support.v4.app.ActivityCompat;
import android.view.Menu;
import android.view.MenuInflater;
import android.view.MenuItem;
import android.view.View;
import android.widget.Button;
import android.widget.CheckBox;
import android.widget.CompoundButton;
import android.widget.RadioButton;
import android.widget.TextView;
import android.widget.Toast;

import java.io.File;
import java.io.FileNotFoundException;
import java.io.IOException;

public class MainActivity extends Activity {
    private String appPath;
    private String configPath;
    private SharedPreferences sPref;
    final String SAVED_PATH = "saved_path";
    private final int REQUEST_EXTERNAL_STORAGE = 1;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);

        appPath = Environment.getExternalStorageDirectory().getAbsolutePath() +
                "/Android/data/" + getPackageName() + "/files/";
        configPath = appPath + "vj.cfg";

        if (!Config.configLoaded)
            Config.setDefaulValues();

        verifyStoragePermissions();
        updateUI();
    }

    @Override
    public boolean onCreateOptionsMenu(Menu menu) {
        MenuInflater inflater = getMenuInflater();
        inflater.inflate(R.menu.menu, menu);
        return true;
    }

    @Override
    public boolean onOptionsItemSelected(MenuItem item) {
        int id = item.getItemId();
        switch (id) {
            case R.id.start:
                if (!Config.romImage.isEmpty())
                    start();
                else
                    Toast.makeText(MainActivity.this, getString(R.string.please_select_rom_image),
                            Toast.LENGTH_SHORT).show();
                break;
        }
        return super.onOptionsItemSelected(item);
    }

    private void start() {
        saveConfig();

        // run bochs app
        //ComponentName cn = new ComponentName("net.sourceforge.bochs", "net.sourceforge.bochs.MainActivity");
        Intent intent = new Intent(this, SDLActivity.class);
        //intent.setComponent(cn);
        startActivity(intent);
    }

    private void saveConfig() {
        try {
            Config.writeConfig(configPath);
        } catch (IOException e) {
            Toast.makeText(MainActivity.this, getString(R.string.config_not_saved), Toast.LENGTH_SHORT).show();
        }
        Toast.makeText(MainActivity.this, getString(R.string.config_saved), Toast.LENGTH_SHORT).show();
    }

    static String getFileName(String path) {
        String result;
        if (path.contains("/"))
            result = path.substring(path.lastIndexOf("/") + 1, path.length());
        else
            result = path;
        return result;
    }

    private void checkConfig() {
        if (!Config.configLoaded) {
            Config.configLoaded = true;
            try {
                Config.readConfig(configPath);
            } catch (FileNotFoundException e) {
                Toast.makeText(MainActivity.this, getString(R.string.config_not_found),
                        Toast.LENGTH_SHORT).show();
                return;
            }
            Toast.makeText(MainActivity.this, getString(R.string.config_loaded),
                    Toast.LENGTH_SHORT).show();
        }
    }

    private boolean createDirIfNotExists() {
        boolean ret = true;

        File file = new File(appPath);
        if (!file.exists()) {
            if (!file.mkdirs()) {
                ret = false;
            }
        }
        return ret;
    }

    @Override
    public void onRequestPermissionsResult(int requestCode, @NonNull String[] permissions,
                                           @NonNull int[] grantResults) {
        switch (requestCode) {
            case REQUEST_EXTERNAL_STORAGE:
                if (grantResults.length != 0 && grantResults[0] == PackageManager.PERMISSION_GRANTED) {
                    // Permission Granted
                    createDirIfNotExists();
                    checkConfig();
                } else {
                    Toast.makeText(this, "Permission Denied", Toast.LENGTH_LONG).show();
                }
                break;
            default:
                super.onRequestPermissionsResult(requestCode, permissions, grantResults);
        }
    }

    public void verifyStoragePermissions() {
        if (ActivityCompat.checkSelfPermission(this,
                Manifest.permission.WRITE_EXTERNAL_STORAGE) != PackageManager.PERMISSION_GRANTED) {

            String[] PERMISSIONS_STORAGE = new String[]{Manifest.permission.READ_EXTERNAL_STORAGE,
                    Manifest.permission.WRITE_EXTERNAL_STORAGE};

            ActivityCompat.requestPermissions(
                    this,
                    PERMISSIONS_STORAGE,
                    REQUEST_EXTERNAL_STORAGE
            );
            return;
        }
        createDirIfNotExists();
        checkConfig();
    }

    private void saveLastPath(String filePath) {
        String dirPath;
        if (filePath.contains("/"))
            dirPath = filePath.substring(0, filePath.lastIndexOf('/'));
        else
            dirPath = filePath;
        sPref = getPreferences(MODE_PRIVATE);
        SharedPreferences.Editor ed = sPref.edit();
        ed.putString(SAVED_PATH, dirPath);
        ed.apply();
    }

    private String getLastPath() {
        sPref = getPreferences(MODE_PRIVATE);
        return sPref.getString(SAVED_PATH, null);
    }

    private void updateUI() {
        romSelection();
        glFilterTypeselection();
        dspSelection();
        fastBlitterSelection();
    }

    private void romSelection() {
        final TextView selectedRomTv = (TextView) findViewById(R.id.selectedRomTextView);
        Button selectRomBtn = (Button) findViewById(R.id.selectRomBtn);
        if (!Config.romImage.isEmpty())
            selectedRomTv.setText(Config.romImage);
        else
            selectedRomTv.setText(getString(R.string.none));
        selectRomBtn.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View view) {
                // Set up extension
                String extension[] = {".jag", ".j64", ".abs", ".rom", ".bin"};
                FileChooser filechooser = new FileChooser(MainActivity.this, getLastPath(), extension);
                filechooser.setFileListener(new FileChooser.FileSelectedListener() {
                    @Override
                    public void fileSelected(File file) {
                        String filename = file.getAbsolutePath();
                        saveLastPath(file.getPath());
                        Config.romImage = filename;
                        selectedRomTv.setText(Config.romImage);
                    }
                });
                filechooser.showDialog();
            }
        });
    }

    private void glFilterTypeselection() {
        RadioButton sharpRb = (RadioButton) findViewById(R.id.glFilterTypeSharp);
        RadioButton blurryRb = (RadioButton) findViewById(R.id.glFilterTypeBlurry);
        sharpRb.setChecked(Config.glFilterType == 0);
        blurryRb.setChecked(Config.glFilterType == 1);
        sharpRb.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View view) {
                Config.glFilterType = 0;
            }
        });
        blurryRb.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View view) {
                Config.glFilterType = 1;
            }
        });
    }

    private void dspSelection() {
        CheckBox enableDspCb = (CheckBox) findViewById(R.id.enableDspCheckbox);
        final CheckBox usePipelinedDSPCb = (CheckBox) findViewById(R.id.usePipelinedDSPCheckbox);
        enableDspCb.setChecked(Config.enableDsp == 1);
        usePipelinedDSPCb.setChecked(Config.usePipelinedDSP == 1);
        usePipelinedDSPCb.setEnabled(Config.enableDsp == 1);
        enableDspCb.setOnCheckedChangeListener(new CompoundButton.OnCheckedChangeListener() {
            @Override
            public void onCheckedChanged(CompoundButton compoundButton, boolean b) {
                Config.enableDsp = b ? 1 : 0;
                usePipelinedDSPCb.setEnabled(Config.enableDsp == 1);
            }
        });
        usePipelinedDSPCb.setOnCheckedChangeListener(new CompoundButton.OnCheckedChangeListener() {
            @Override
            public void onCheckedChanged(CompoundButton compoundButton, boolean b) {
                Config.usePipelinedDSP = b ? 1 : 0;
            }
        });
    }

    private void fastBlitterSelection() {
        CheckBox useFastBlitterCb = (CheckBox) findViewById(R.id.useFastBlitterCheckbox);
        useFastBlitterCb.setChecked(Config.useFastBlitter == 1);
        useFastBlitterCb.setOnCheckedChangeListener(new CompoundButton.OnCheckedChangeListener() {
            @Override
            public void onCheckedChanged(CompoundButton compoundButton, boolean b) {
                Config.useFastBlitter = b ? 1 : 0;
            }
        });
    }

}
