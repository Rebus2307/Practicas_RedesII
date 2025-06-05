import java.io.IOException;
import java.net.ServerSocket;
import java.net.Socket;
import java.util.concurrent.ExecutorService;
import java.util.concurrent.Executors;

public class FTPServidor {
    private static final int PORT = 21;
    private static final int MAX_THREADS = 10;

    public static void main(String[] args) throws IOException {
        ServerSocket serverSocket = new ServerSocket(PORT);
        ExecutorService threadPool = Executors.newFixedThreadPool(MAX_THREADS);

        System.out.println("Servidor FTP iniciado en el puerto " + PORT);

        while (true) {
            Socket clientSocket = serverSocket.accept();
            threadPool.execute(new FTPHandler(clientSocket));
        }
    }
}
