package org.icculus.virtualjaguar;

import java.io.File;
import java.io.FileNotFoundException;
import java.io.FileWriter;
import java.io.IOException;
import java.util.Scanner;

class Config {
    static final String NONE = "none";
    static boolean configLoaded = false;

    static void setDefaulValues() {

    }

    static void readConfig(String path) throws FileNotFoundException {
        File file = new File(path);
        Scanner sc = new Scanner(file).useDelimiter("[\n]");
        while (sc.hasNext()) {
            String str = sc.next() + "\n";
        }
        sc.close();
    }

    static void writeConfig(String path) throws IOException {
        File file = new File(path);
        FileWriter fw = new FileWriter(file);
        fw.write("log: bochsout.txt\n");
        fw.close();
    }
}
