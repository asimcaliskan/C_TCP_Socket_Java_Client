import java.io.*;
import java.net.*;
import java.util.ArrayList;
import java.util.List; 
import java.awt.*;
import javax.swing.JTextArea;
class sclient
{
    private String host = null;
    private int port = -999;
    private Socket socket = null;
    private String channel = null;
    private BufferedReader inStream = null;
    private PrintWriter outStream = null;
    private List<String> outFrameList;
    public static void main(String[] args)
    {
        //FRAME DECLARATION<
	    Frame frame = new Frame("JAVA CLIENT");  
	    JTextArea textField;  
	    textField = new JTextArea("THE VIDEO STREAM IS STARTING");  
	    textField.setFont(new Font("MONOSPACED", Font.BOLD, 20));
	    textField.setBounds(50, 50, 900, 500);  
	    frame.add(textField);  
	    frame.setSize(1000, 600);  
	    frame.setLayout(null);  
	    frame.setVisible(true); 
		//FRAME DECLARATION>

        sclient client = new sclient();
        client.host = args[0];
        client.port = new Integer(args[1]).intValue();
        client.channel = args[2];
        client.setUp();
        client.outFrameList = new ArrayList<String>();
        boolean exit = true;
        while(exit)
        {
            client.sendData();
            exit = client.receiveData(client, textField);
        }
        client.cleanUp(frame);
    } 

    public void setUp(){
        System.out.println("SETUP");

        try
        {
            socket = new Socket(host, port);
            inStream = new BufferedReader(new InputStreamReader(socket.getInputStream()));
            outStream = new PrintWriter(socket.getOutputStream(), true);
        }
        catch(UnknownHostException e)
        {
            System.err.println("Cannot find host called: " + host);
            e.printStackTrace();
            System.exit(-1);
        }
        catch(IOException e)
        {
            System.err.println("Could not establish connection for " + host);
            e.printStackTrace();
            System.exit(-1);
        }
    } 

    private void sendData()
    {
        //System.out.println("SEND");
        outStream.println(channel);
    }

    public boolean receiveData(sclient client, JTextArea textField)
    {
        //System.out.println("RECEIVE");
        boolean returnValue = false;
        try
        {
            String inn = inStream.readLine();
            while(inn == null){
                inn = inStream.readLine();
            }
            try
            {
                Thread.sleep(50);
            }
            catch(InterruptedException ex)
            {
                Thread.currentThread().interrupt();
            }
            //System.out.println(inn);
            client.outFrameList.add(inn);
            if(client.outFrameList.size() == 14){
            	String canvasOut = "";
                for (String string : client.outFrameList) 
                {
			        canvasOut += string + "\n";
		        }
                System.out.println(canvasOut);
                textField.setText(canvasOut);
                client.outFrameList.clear();
            }
            if(inn.equals("exit")) returnValue = false;
            else                   returnValue = true;
        }
        catch(IOException e)
        {
        System.err.println("Conversation error with host " + host);
        e.printStackTrace();
        }
        return returnValue;
    } 

    public void cleanUp(Frame frame)
    {
        System.out.println("CLEAN_UP");
        try
        {
            if (inStream != null)
            inStream.close();
            if (socket != null)
            socket.close();
            frame.dispose();
        } 
        catch(IOException e)
        {
            System.err.println("Error in cleanup");
            e.printStackTrace();
            System.exit(-1);
        }
    } 
}