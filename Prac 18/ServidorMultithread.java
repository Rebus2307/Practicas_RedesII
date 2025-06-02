import java.io.*;
import java.net.*;

public class ServidorMultithread {
    private static final int PORT = 1234;

    public static void main(String[] args) {
        try (ServerSocket serverSocket = new ServerSocket(PORT)) {
            System.out.println("Servidor en espera de conexiones...");

            while (true) {
                Socket clienteSocket = serverSocket.accept();
                System.out.println("Cliente conectado: " + clienteSocket.getInetAddress());
                ClientHandler handler = new ClientHandler(clienteSocket);
                new Thread(handler).start();
            }
        } catch (IOException e) {
            e.printStackTrace();
        }
    }
}

class ClientHandler implements Runnable {
    private Socket clienteSocket;
    private BufferedReader input;
    private PrintWriter output;

    public ClientHandler(Socket socket) {
        this.clienteSocket = socket;
    }

    public void run() {
        try {
            input = new BufferedReader(new InputStreamReader(clienteSocket.getInputStream()));
            output = new PrintWriter(clienteSocket.getOutputStream(), true);

            String mensaje;
            while ((mensaje = input.readLine()) != null) {
                System.out.println("Cliente dice: " + mensaje);
                output.println("Servidor: " + mensaje);
            }
        } catch (IOException e) {
            e.printStackTrace();
        } finally {
            try {
                clienteSocket.close();
            } catch (IOException e) {
                e.printStackTrace();
            }
        }
    }
}
