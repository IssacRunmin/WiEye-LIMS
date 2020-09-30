package com.witech;

import java.text.DecimalFormat;

public class MQTT {
    public static final String DevID = "7AB3";
    public static final String HOST = "tcp://localhost:1883";
    public static final String LogTOPIC = "WiEyeClient";
    public static final String userName = "admin";
    public static final String passWord = "password";
    private static final DecimalFormat format = new DecimalFormat("00000000");
    public static final String GenerateClientName(){
        return format.format(Math.floor(Math.random() * 100000000)) + "Sever" + MQTT.DevID;
    }
}
