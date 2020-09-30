package com.witech; /**
 *
 * Description:
 * @author admin
 * 2017年2月10日下午18:04:07
 */

import org.eclipse.paho.client.mqttv3.IMqttDeliveryToken;
import org.eclipse.paho.client.mqttv3.MqttCallback;
import org.eclipse.paho.client.mqttv3.MqttMessage;

import java.sql.ResultSet;
import java.util.Date;

/**
 * 发布消息的回调类
 *
 * 必须实现MqttCallback的接口并实现对应的相关接口方法CallBack 类将实现 MqttCallBack。
 * 每个客户机标识都需要一个回调实例。在此示例中，构造函数传递客户机标识以另存为实例数据。
 * 在回调中，将它用来标识已经启动了该回调的哪个实例。
 * 必须在回调类中实现三个方法：
 *
 *  public void messageArrived(MqttTopic topic, MqttMessage message)接收已经预订的发布。
 *
 *  public void connectionLost(Throwable cause)在断开连接时调用。
 *
 *  public void deliveryComplete(MqttDeliveryToken token))
 *  接收到已经发布的 QoS 1 或 QoS 2 消息的传递令牌时调用。
 *  由 MqttClient.connect 激活此回调。
 *
 */
public class PushCallback implements MqttCallback {
    private final String ClientName;
    private final String Topic;
    private String PubNum = "";

    public PushCallback(){
//        this();
        ClientName = "";
        Topic = "";
    }
    public PushCallback(final String client, final String topic){
        ClientName = client;
        Topic = topic;
    }
    public void connectionLost(Throwable cause) {
        // 连接丢失后，一般在这里面进行重连
        System.out.println("连接断开，可以做重连");
    }

    public void deliveryComplete(IMqttDeliveryToken token) {
        System.out.println("deliveryComplete---------" + token.isComplete());
    }

    public void messageArrived(String topic, MqttMessage message) throws Exception {
        // subscribe后得到的消息会执行到这里面

        if (topic.compareTo(Topic) != 0) return;
        System.out.println("接收消息主题 : " + topic);
        System.out.println("接收消息Qos : " + message.getQos());
        System.out.println("接收消息内容 : " + new String(message.getPayload()));
        if (topic.compareTo(MQTT.LogTOPIC) == 0 ){
            if (message.getPayload().length < 8)
                return;
            final String InTopic = new String(message.getPayload());
            String Devid = InTopic.substring(0,8);
            String Command = "SELECT * FROM \"Device\" WHERE \"DeviceID\" = '" + Devid +
                    "'";// AND 'DevPassword'='" + InTopic.substring(8) +"'";
            System.out.println(Command);
            ResultSet Re = Database.Query(Command);
            if (!Re.next()){
                System.out.println("Invalid Device ID/ Payload!");
            }
            else {
                System.out.println("Device #" + Devid + " Log in!");
                new RecvMQTT("WiEye" + Devid+"_Out");
            }
        }
        else{
            String Payload = new String(message.getPayload());
            if (Payload.length() < 8) return;
            if (Payload.substring(17,19).compareTo(PubNum) != 0){
                String Command;
                String ACState;

                System.out.println("PublishNum: " + PubNum);
                Command = "SELECT \"ID\",\"DepID\" FROM \"Worker\" WHERE \"CardID\" = '" + Payload.substring(0,16) + "'";
                ResultSet Re = Database.Query(Command);
                if(!Re.next()){
                    System.out.println("Did not found card ID");
                }
                else{
                    PubNum = Payload.substring(19,21);
                    ACState = Payload.substring(17,18);
                    Command = "INSERT INTO \"Attendance\" VALUES(1," + Re.getString("ID")
                            + ", sysdate, 0,1,'1'," + ACState + ")";
                    System.out.println(Command);
                    Database.Update(Command);
                }
            }
        }
    }
}