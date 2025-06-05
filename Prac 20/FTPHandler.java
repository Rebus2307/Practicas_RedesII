import java.io.*;
import java.net.Socket;

public class FTPHandler implements Runnable {
    private Socket clientSocket;

    public FTPHandler(Socket socket) {
        this.clientSocket = socket;
    }

    @Override
    public void run() {
        try {
            InputStream input = clientSocket.getInputStream();
            OutputStream output = clientSocket.getOutputStream();
            BufferedReader reader = new BufferedReader(new InputStreamReader(input));
            PrintWriter writer = new PrintWriter(output, true);

            String command = reader.readLine();
            System.out.println("Comando recibido: " + command);

            if (command.startsWith("PUT")) {
                String fileName = command.split(" ")[1];
                writer.println("READY");
                receiveFile(fileName, input, reader, writer);
            }

            clientSocket.close();
        } catch (IOException e) {
            e.printStackTrace();
        }
    }

    private void receiveFile(String fileName, InputStream input, BufferedReader reader, PrintWriter writer) {
        try {
            File file = new File("servidor_" + fileName);
            FileOutputStream fileOutput = new FileOutputStream(file);

            byte[] buffer = new byte[4096];
            int bytesRead;

            while ((bytesRead = input.read(buffer)) != -1) {
                fileOutput.write(buffer, 0, bytesRead);
                if (reader.ready() && "END".equals(reader.readLine())) {
                    break;
                }
            }

            fileOutput.close();
            System.out.println("Archivo " + fileName + " recibido con éxito.");
            writer.println("Archivo " + fileName + " subido correctamente.");
        } catch (IOException e) {
            e.printStackTrace();
            writer.println("Error al recibir el archivo " + fileName);
        }
    }
}
