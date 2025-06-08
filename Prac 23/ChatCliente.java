import java.io.*;
import java.net.*;
import java.util.Scanner;

public class ChatCliente {
    private static final String SERVER_ADDRESS = "127.0.0.1";
    private static final int SERVER_PORT = 12345;

    public static void main(String[] args) {
        try (Socket socket = new Socket(SERVER_ADDRESS, SERVER_PORT);
             PrintWriter out = new PrintWriter(socket.getOutputStream(), true);
             BufferedReader in = new BufferedReader(new InputStreamReader(socket.getInputStream()));
             Scanner scanner = new Scanner(System.in)) {

            Thread receiveMessages = new Thread(() -> {
                try {
                    String message;
                    while ((message = in.readLine()) != null) {
                        System.out.println("Mensaje recibido: " + message);
                    }
                } catch (IOException e) {
                    e.printStackTrace();
                }
            });
            receiveMessages.start();

            System.out.println("Conectado al chat. Escribe tu mensaje:");
            while (true) {
                String messageToSend = scanner.nextLine();
                out.println(messageToSend);
            }
        } catch (IOException e) {
            e.printStackTrace();
        }
    }
}
