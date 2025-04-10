import java.io.FileOutputStream; 
import java.io.IOException; 
import java.nio.ByteBuffer; 
import java.nio.channels.DatagramChannel; 
import java.nio.channels.SelectionKey; 
import java.nio.channels.Selector; 
import java.nio.channels.WritableByteChannel; 
import java.net.InetSocketAddress; 
import java.util.Iterator; 

public class UDPFileServer { 

    private static final int PORT = 8081; 
    private static final int BUFFER_SIZE = 1024; 

    public static void main(String[] args) { 
        try { 
            DatagramChannel serverChannel = DatagramChannel.open(); 
            serverChannel.configureBlocking(false); 
            serverChannel.bind(new InetSocketAddress(PORT)); 

            Selector selector = Selector.open(); 
            serverChannel.register(selector, SelectionKey.OP_READ); 

            ByteBuffer buffer = ByteBuffer.allocate(BUFFER_SIZE); 
            WritableByteChannel fileChannel = new FileOutputStream("archivorecibido.txt").getChannel(); 

            while (true) { 
                selector.select(); 
                Iterator<SelectionKey> keys = selector.selectedKeys().iterator(); 

                while (keys.hasNext()) { 
                    SelectionKey key = keys.next(); 
                    keys.remove(); 

                    if (key.isReadable()) { 
                        buffer.clear(); 
                        DatagramChannel channel = (DatagramChannel) key.channel(); 
                        InetSocketAddress clientAddress = (InetSocketAddress) channel.receive(buffer); 
                        buffer.flip(); 
                        fileChannel.write(buffer); 
                        System.out.println("Archivo recibido desde: " + clientAddress); 
                    } 
                } 
            } 

        } catch (IOException e) { 
            e.printStackTrace(); 
        } 
    } 
}
