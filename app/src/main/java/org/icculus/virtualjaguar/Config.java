package org.icculus.virtualjaguar;

import java.io.File;
import java.io.FileNotFoundException;
import java.io.FileWriter;
import java.io.IOException;
import java.util.Scanner;

class Config {
    static final String NONE = "none";
    static boolean configLoaded = false;
    static String romImage = "";
    static int glFilterType = 0;
    static int enableDsp = 1;

    static void setDefaulValues() {

    }

    static void readConfig(String path) throws FileNotFoundException {
        File file = new File(path);
        Scanner sc = new Scanner(file).useDelimiter("[\n]");
        while (sc.hasNext()) {
            String str = sc.next() + "\n";
            if (str.startsWith("autostartROM") && str.contains("="))
                romImage = str.substring(str.indexOf("=") + 1, str.length()).trim();
            if (str.startsWith("glFilterType") && str.contains("=")) {
                glFilterType = getAsInt(str);
            }
            if (str.startsWith("DSPEnabled") && str.contains("=")) {
                enableDsp = getAsInt(str);
            }
        }
        sc.close();
    }

    static private int getAsInt(String str) {
        int result = 0;
        String tStr = str.substring(str.indexOf("=") + 1, str.length()).trim();
        try {
            result = Integer.parseInt(tStr);
        } catch (Exception ignore) {
        }
        return result;
    }

    static void writeConfig(String path) throws IOException {
        File file = new File(path);
        FileWriter fw = new FileWriter(file);

        fw.write("#\n");
        fw.write("# Virtual Jaguar configuration file\n");
        fw.write("#\n");
        fw.write("\n");
        fw.write("# Jaguar BIOS options: 1 - use, 0 - don't use\n");
        fw.write("\n");
        fw.write("useJaguarBIOS = 0\n");
        fw.write("\n");
        fw.write("# Jaguar ROM paths\n");
        fw.write("\n");
        fw.write("JagBootROM = ./bios/jagboot.rom\n");
        fw.write("CDBootROM = ./bios/jagcd.rom\n");
        fw.write("EEPROMs = ./eeproms\n");
        fw.write("ROMs = ./ROMs\n");
        fw.write("\n");
        fw.write("#OpenGL options: 1 - use OpenGL rendering, 0 - use old style rendering\n");
        fw.write("\n");
        fw.write("useOpenGL = 1\n");
        fw.write("\n");
        fw.write("#OpenGL filtering type: 1 - blurry, 0 - sharp\n");
        fw.write("\n");
        fw.write("glFilterType = " + glFilterType + "\n");
        fw.write("\n");
        fw.write("#Display options: 1 - fullscreen, 0 - windowed\n");
        fw.write("\n");
        fw.write("fullscreen = 1\n");
        fw.write("\n");
        fw.write("#NTSC / PAL options: 1 - NTSC, 0 - PAL\n");
        fw.write("\n");
        fw.write("hardwareTypeNTSC = 1\n");
        fw.write("\n");
        fw.write("#Render type (experimental): 0 - regular / OpenGL, 1 - TV style\n");
        fw.write("\n");
        fw.write("renderType = 0\n");
        fw.write("\n");
        fw.write("#DSP options: 1 - use, 0 - don 't use\n");
        fw.write("\n");
        fw.write("DSPEnabled = " + enableDsp + "\n");
        fw.write("\n");
        fw.write("#If DSP enabled, set whether or not to use the pipelined core: 1 - use, 0 - don 't use\n");
        fw.write("\n");
        fw.write("usePipelinedDSP = 0\n");
        fw.write("\n");
        fw.write("#FastBlitter options: 1 - use, 0 - don 't use\n");
        fw.write("\n");
        fw.write("useFastBlitter = 0\n");
        fw.write("\n");
        fw.write("#Joystick options: 1 - use joystick, 0 - don 't use\n");
        fw.write("\n");
        fw.write("useJoystick = 0\n");
        fw.write("\n");
        fw.write("#Joyport option: If joystick is enabled above, set the port (0 - 3) here\n");
        fw.write("\n");
        fw.write("joyport = 0\n");
        fw.write("\n");
        fw.write("#Jaguar joypad key assignments\n");
        fw.write("#Note: It would be nicer to be able to have a single left side to store all this in...\n");
        fw.write("#E.g.p1keys = 34, 32, 22, etc.instead of what we have here...\n");
        fw.write("\n");
        fw.write("p1k_up = 273	    	# SDLK_UP\n");
        fw.write("p1k_down = 274		# SDLK_DOWN\n");
        fw.write("p1k_left = 276		# SDLK_LEFT\n");
        fw.write("p1k_right = 275		# SDLK_RIGHT\n");
        fw.write("p1k_c = 122			# SDLK_z\n");
        fw.write("p1k_b = 120			# SDLK_x\n");
        fw.write("p1k_a = 99			# SDLK_c\n");
        fw.write("p1k_option = 39		# SDLK_QUOTE\n");
        fw.write("p1k_pause = 13		# SDLK_RETURN\n");
        fw.write("p1k_0 = 256			# SDLK_KP0\n");
        fw.write("p1k_1 = 257			# SDLK_KP1\n");
        fw.write("p1k_2 = 258			# SDLK_KP2\n");
        fw.write("p1k_3 = 259			# SDLK_KP3\n");
        fw.write("p1k_4 = 260			# SDLK_KP4\n");
        fw.write("p1k_5 = 261			# SDLK_KP5\n");
        fw.write("p1k_6 = 262			# SDLK_KP6\n");
        fw.write("p1k_7 = 263			# SDLK_KP7\n");
        fw.write("p1k_8 = 264			# SDLK_KP8\n");
        fw.write("p1k_9 = 265			# SDLK_KP9\n");
        fw.write("p1k_pound = 267		# SDLK_KP_DIVIDE\n");
        fw.write("p1k_star = 268		# SDLK_KP_MULTIPLY\n");
        fw.write("p2k_up = 273	    	# SDLK_UP\n");
        fw.write("p2k_down = 274		# SDLK_DOWN\n");
        fw.write("p2k_left = 276		# SDLK_LEFT\n");
        fw.write("p2k_right = 275		# SDLK_RIGHT\n");
        fw.write("p2k_c = 122			# SDLK_z\n");
        fw.write("p2k_b = 120			# SDLK_x\n");
        fw.write("p2k_a = 99			# SDLK_c\n");
        fw.write("p2k_option = 39		# SDLK_QUOTE\n");
        fw.write("p2k_pause = 13		# SDLK_RETURN\n");
        fw.write("p2k_0 = 256			# SDLK_KP0\n");
        fw.write("p2k_1 = 257			# SDLK_KP1\n");
        fw.write("p2k_2 = 258			# SDLK_KP2\n");
        fw.write("p2k_3 = 259			# SDLK_KP3\n");
        fw.write("p2k_4 = 260			# SDLK_KP4\n");
        fw.write("p2k_5 = 261			# SDLK_KP5\n");
        fw.write("p2k_6 = 262			# SDLK_KP6\n");
        fw.write("p2k_7 = 263			# SDLK_KP7\n");
        fw.write("p2k_8 = 264			# SDLK_KP8\n");
        fw.write("p2k_9 = 265			# SDLK_KP9\n");
        fw.write("p2k_pound = 267		# SDLK_KP_DIVIDE\n");
        fw.write("p2k_star = 268		# SDLK_KP_MULTIPLY\n");
        fw.write("\n");
        if (!romImage.isEmpty())
            fw.write("autostartROM = " + romImage + "\n");
        fw.write("\n");

        fw.close();
    }
}
