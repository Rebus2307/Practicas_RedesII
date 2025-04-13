import java.io.FileInputStream; 
import java.io.IOException; 
import java.nio.ByteBuffer; 
import java.nio.channels.DatagramChannel; 
import java.nio.channels.ReadableByteChannel; 
import java.net.InetSocketAddress; 

public class UDPFileClient { 

    private static final int PORT = 8081; 
    private static final int BUFFER_SIZE = 1024; 

    public static void main(String[] args) { 
        if (args.length != 1) { 
            System.out.println("Usage: java UDPFileClient <file-path>"); 
            return; 
        } 

        String filePath = args[0]; 

        try { 
            DatagramChannel clientChannel = DatagramChannel.open(); 
            clientChannel.configureBlocking(false); 
            InetSocketAddress serverAddress = new InetSocketAddress("localhost", PORT); 

            ByteBuffer buffer = ByteBuffer.allocate(BUFFER_SIZE); 
            ReadableByteChannel fileChannel = new FileInputStream(filePath).getChannel(); 

            while (fileChannel.read(buffer) > 0) { 
                buffer.flip(); 
                clientChannel.send(buffer, serverAddress); 
                buffer.clear(); 
            } 

            fileChannel.close(); 
            clientChannel.close(); 
            System.out.println("Archivo enviado al servidor."); 

        } catch (IOException e) { 
            e.printStackTrace(); 
        } 
    } 
}
