package com.witech;

import org.eclipse.paho.client.mqttv3.MqttException;

import java.sql.*;

public class Database {
    private static final String url = "jdbc:oracle:thin:@//127.0.0.1:49161/XE" ;
    private static Connection DBconn = null;
    private static Statement State;

    private static int ERR_Database = 3;
    static{
        try{
            Class.forName("oracle.jdbc.OracleDriver");
        }
        catch (ClassNotFoundException e){
            System.out.println(e.getMessage());
            e.printStackTrace();
        }
        try{
            DBconn = DriverManager.getConnection(url,"wi_eye","wi_tech");
            State = DBconn.createStatement();
            System.out.println("WiEye Database connected!");
        }
        catch (SQLException e){
            System.out.println(e.getMessage());
            e.printStackTrace();
//            System.exit(ERR_Database);
        }
    }
    public static ResultSet Query(String Command){
        ResultSet Re = null;
        try{
            Re = State.executeQuery(Command);
        }catch (SQLException e){
            e.printStackTrace();
        }
        return Re;
    }

    public static synchronized int Update(String Command){
        int Re = 0;
        try{
            Re = State.executeUpdate(Command);
        }catch (SQLException e){
            e.printStackTrace();
        }
        return Re;
    }
//    public static void main(String[] args) throws MqttException {
////        final String Topic = "8765WiEye00007AB3";
////        String Command = "SELECT * FROM \"Worker\"";
////        System.out.println(Database.DBconn);
////        ResultSet Re = Database.Query(Command);
//    }

}
