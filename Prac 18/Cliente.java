import java.io.*;
import java.net.*;

public class Cliente {
    private static final String SERVER_ADDRESS = "127.0.0.1";
    private static final int SERVER_PORT = 1234;

    public static void main(String[] args) {
        try (Socket socket = new Socket(SERVER_ADDRESS, SERVER_PORT);
             BufferedReader input = new BufferedReader(new InputStreamReader(socket.getInputStream()));
             PrintWriter output = new PrintWriter(socket.getOutputStream(), true);
             BufferedReader console = new BufferedReader(new InputStreamReader(System.in))) {

            System.out.println("Conectado al servidor. Escribe mensajes:");

            String mensaje;
            while ((mensaje = console.readLine()) != null) {
                output.println(mensaje);
                System.out.println("Servidor responde: " + input.readLine());
            }
        } catch (IOException e) {
            e.printStackTrace();
        }
    }
}
