package com.witech; /**
 * Created by Administrator on 17-2-10.
 */

import org.eclipse.paho.client.mqttv3.MqttClient;
import org.eclipse.paho.client.mqttv3.MqttConnectOptions;
import org.eclipse.paho.client.mqttv3.MqttDeliveryToken;
import org.eclipse.paho.client.mqttv3.MqttException;
import org.eclipse.paho.client.mqttv3.MqttMessage;
import org.eclipse.paho.client.mqttv3.MqttPersistenceException;
import org.eclipse.paho.client.mqttv3.MqttTopic;
import org.eclipse.paho.client.mqttv3.persist.MemoryPersistence;

import java.text.DecimalFormat;

/**
 *
 * Title:Server
 * Description: 服务器向多个客户端推送主题，即不同客户端可向服务器订阅相同主题
 * @author admin
 * 2017年2月10日下午17:41:10
 */
public class PublishMQTT extends Thread{
    //定义一个主题
    private final String Topic;
    //定义MQTT的ID，可以在MQTT服务配置中指定
    private final String ClientName;

    private MqttClient client;
    private MqttTopic topic11;
    public MqttTopic LogTopic;
    private MqttMessage message;
    private final String ComFile = "/home/jennygroup/Matlab/Com_WiEye";
    private int Count;

    /**
     * 构造函数
     * @throws MqttException
     */
    public PublishMQTT() throws MqttException {
        // MemoryPersistence设置clientid的保存形式，默认为以内存保存
        Topic = MQTT.LogTOPIC;
        ClientName = MQTT.GenerateClientName();
        client = new MqttClient(MQTT.HOST, ClientName, new MemoryPersistence());
        connect();
    }
    public PublishMQTT(final String topic) throws MqttException{
        Topic = topic;
        DecimalFormat format = new DecimalFormat("0000");
        ClientName = format.format(Math.floor(Math.random() * 10000)) + "WiEye_00007AB7";
        client = new MqttClient(MQTT.HOST, ClientName, new MemoryPersistence());
        connect();
        this.start();
    }
    public void run(){

    }

    /**
     *  用来连接服务器
     */
    private void connect() {
        MqttConnectOptions options = new MqttConnectOptions();
        options.setCleanSession(false);
//        options.setUserName(userName);
//        options.setPassword(passWord.toCharArray());
        // 设置超时时间
        options.setConnectionTimeout(10);
        // 设置会话心跳时间
        options.setKeepAliveInterval(20);
        try {
//            client.setCallback(new PushCallback(ClientName, Topic));
            client.connect(options);

            topic11 = client.getTopic(Topic);
            LogTopic = client.getTopic(MQTT.LogTOPIC);
        } catch (Exception e) {
            e.printStackTrace();
        }
    }

    /**
     *
     * @param topic
     * @param message
     * @throws MqttPersistenceException
     * @throws MqttException
     */
    public void publish(MqttTopic topic , MqttMessage message) throws MqttPersistenceException,
            MqttException {
        MqttDeliveryToken token = topic.publish(message);
        token.waitForCompletion();
        System.out.println("message is published completely! "
                + token.isComplete());
    }

    /**
     *  启动入口
     * @param args
     * @throws MqttException
     */
    public static void main(String[] args) throws MqttException {
        PublishMQTT server = new PublishMQTT("WiEye00007AB7");
//        Database DB = new Database();
//        server.message = new MqttMessage();
//        server.message.setQos(1);
//        server.message.setRetained(true);
//        server.message.setPayload("00007AB731417".getBytes());
//        server.publish(server.LogTopic , server.message);
//        System.out.println(server.message.isRetained() + "------retained状态");
        try {
            Thread.sleep(1000);
        }catch (Exception e){
            e.printStackTrace();
        }
        server.message = new MqttMessage();
        server.message.setQos(1);
        server.message.setRetained(true);
        server.message.setPayload("FEF238E700000000_4003".getBytes());
        server.publish(server.topic11 , server.message);
    }
}
